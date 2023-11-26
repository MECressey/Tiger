#include <afx.h>

#include <stdlib.h>
#include <search.h>
#include <limits.h>
#include <stdio.h>
//#include <cstdio>
#include <sys\types.h>
#include <sys\stat.h>
#include <wchar.h>
//#include <string.h>
#include <iostream>
#include <string>
#include <fstream>
#include "tigerrec.hpp"
#include "tigerlib.h"
#include "range.hpp"

#include "doblock.h"
//#include "dopoly.h"
#include "doaddr.h"
#include "donames.h"
#include "dogtpoly.h"
//#include "dohist.h"
#include "tgrnames.h"
#include "namelook.h"
//#include "distname.h"
#include "tgrtypes.h"
#include "tigerdb.hpp"
#include "trendlin.h"
#include "doline.h"
//#include "DAC.HPP"
#include <map>
#include <array>
#include <vector>
#include <algorithm>
#include <assert.h>
#include "HASHTABL.HPP"
#include "TopoTools.h"
#include "TString.h"

#define TO_TIGERDB
#define DO_REAT

const int RTSQ =	0;
const int TIGER_92	= 5;
const int TIGER_94	= 21;

static short GetStateFips( const TCHAR * );
static short GetCountyFips( const TCHAR * );
/*
struct DirLineId
{
	long tlid;
	signed char dir;
};
*/
struct i_rnum
{
   long rnum;
   long fpos;
};

static long fsize( const char * );
static int __cdecl compare( const void *, const void * );
static long DoShapeIndex( const TCHAR *, i_rnum ** );
static int BinarySearch( long rnum, i_rnum *, long, long * );
static TigerDB::Classification MapCFCC( const char *cfcc );
static double CalcArea(/*TigerDB::Chain* line*/XY_t pts[], int nPts,
	const double& xcom, const double& ycom, double* xcg, double* ycg, int dir);
//static bool isPolygonMatch(ObjHandle &poly, std::vector<DirLineId>& polyLines, int start, int end);

static XY_t points[15000];

#define RT1_EXT				0
#define RT2_EXT				1
#define RT3_EXT				2
#define RT4_EXT				3
#define RT5_EXT				4
#define RT6_EXT				5
#define RT7_EXT				6
#define RT8_EXT				7
#define RT9_EXT				8
#define RTA_EXT				9
#define RTC_EXT				10
#define RTH_EXT				11
#define RTI_EXT				12
#define RTP_EXT				13
#define RTR_EXT				14
#define RTS_EXT				15
#define RTZ_EXT				16

static const TCHAR *tigerExts[] = 
{
	_T(".RT1"),
	_T(".RT2"),
	_T(".RT3"),
	_T(".RT4"),
	_T(".RT5"),
	_T(".RT6"),
	_T(".RT7"),
	_T(".RT8"),
	_T(".RT9"),
	_T(".RTA"),
	_T(".RTC"),
	_T(".RTH"),
	_T(".RTI"),
	_T(".RTP"),
	_T(".RTR"),
	_T(".RTS"),
	_T(".RTZ"),
};

class DbHash : public DbHashAccess {
public:
	long tlid;
	int is_equal(DbObject* dbo) { return this->tlid == ((TigerDB::Chain*)dbo)->userId/*GetTLID()*/; }
	long int hashKey(int nBits) { return HashTable::HashDK(nBits, tlid); }
};

int main( int argc, char *argv[] )
{
  char buffer[512];
  long nLines = 0,
		   nLinesSkipped = 0;
  CString rt1Name,
  				baseName,
  			  fName;
  int dirIdx,
      dotIdx,
      length;
  XY_t sNode,
		   eNode;
  FILE *csvLines = 0,
  	   *csvBlocks = 0,
  	   *csvBlocks2 = 0,
  	   *csvNames = 0,
  	   *csvAddr = 0,
	   *csvPoly = 0,
  	   *inFile1 = 0,
  	   *inFile2 = 0,
	   *mappingFile = 0;
  short countyFips;
	int		stateFips;
  i_rnum *index = 0;
  long nIndex;
	bool doCreate = false,
			 doLines = true,
  	   doNames = true,
		   doBlocks = true,
		   doPolys = true,
		   doAddress = true,
		   doGTpoly = true,
		   doHistory = true,
		   doZips	= false,
			 doTigerDB = true/*false*/,
			 doTopo = false;
  int version = 0;
	int error;
	double tlFilter = 0.0001;
	int doTrendLine = 0/* 1 */;

	if( argc <= 1 )
	{
		fputs( "Enter tiger name: ", stdout );
		fgets( buffer, sizeof( buffer ) - 1, stdin );
	}
	else
	{
		strcpy(buffer, argv[1]);

		if( argc > 2 && ( argv[ 2 ][ 0 ] == '/' || argv[ 2 ][ 0 ] == '-' ) )
		{
			doHistory = doGTpoly = doLines = doNames = doBlocks = doPolys = doAddress = doCreate = doTopo = false;
			int i = 1;
			while( argv[2][i] != '\0' )
			{
	      switch( argv[2][i] )
			  {
			  default :
			    printf( "* TgrTrans - invalid argument\n" );
				goto USAGE_ERROR;

				case 'C':
				case 'c':
					doCreate = true;
					break;

				case 'I' :
			  case 'i' :
			    doGTpoly = true;
					break;

			  case 'H' :
			  case 'h' :
			    doHistory = true;
					break;

			  case 'p' :
			  case 'P' :
			    doPolys = true;
					break;

			  case 'L' :
			  case 'l' :
			    doLines = true;
					break;

			  case 'B' :
			  case 'b' :
			    doBlocks = true;
					break;

			  case 'A' :
			  case 'a' :
			    doAddress = true;
					break;

			  case 'N' :
			  case 'n' :
			    doNames = true;
					break;

				case 'T':
				case 't':
					doTopo = true;
					break;

			  case 'Z' :
			  case 'z' :
			    doZips = true;
					break;
			  }
			  i++;
			}
		}
	}

	try
	{
		CDatabase db;
		db.SetQueryTimeout(60 * 10);
		db.Open( _T("TigerBase"), FALSE, TRUE, _T("ODBC;"), FALSE );
//		db.SetSynchronousMode(TRUE);
#ifdef TO_TIGERDB
		TigerDB tDB(&db);
		//DAC dac;
#endif
		CString rootName;

		TgrNames tgrNames( &db );
		NameLook nLook( &db );
//		DistNames distName( &db );

	  length = ::strlen( buffer );
	  ::fputs( buffer, stdout );

	  if( buffer[ length - 1 ] == '\n' )
			buffer[ length - 1 ] = '\0';
    
		if (doCreate)
		{
			char input[80];
			double xmin,  // 204
				xmax,
				ymax,	//520,
				ymin;

			/*printf("\nDatabase name? ");
			if (gets(input) == NULL)
				return 0;*/

			char name[80];
			strcpy(name, argv[3]);
			strcat(name, ".gdb");
			/* Waldo  Range - Xmin: -69.854694, Ymin: 44.202643, XMax: -68.792301, YMax: 44.754642
			printf("\nX Min? ");
			std::getline(std::cin, input);
			//input << std::cin;*/
			sscanf(argv[4], "%lf", &xmin);

			printf("X Min = %f\n", xmin);

			/*printf("\nY Min? ");
			std::getline(std::cin, input);*/
			sscanf(argv[5], "%lf", &ymin);
			printf("Y Min = %f\n", ymin);

			/*printf("\nX Max? ");
			std::getline(std::cin, input);*/
			sscanf(argv[6], "%lf", &xmax);
			printf("X Max = %f\n", xmax);

			/*printf("\nY Max? ");
			std::getline(std::cin, input);*/
			sscanf(argv[7], "%lf", &ymax);
			printf("Y Max = %f\n", ymax);
			TString tName(argv[3]);
			error = tDB.dacCreate(tName, 1 << 16);
			if ((error = tDB.Create(tName, xmin, ymin, xmax, ymax)) != 0)
				return -1;

			printf("Database %s created\n", name);

			return 0;
		}

		if (doTopo)
		{
			if (doTigerDB && (error = tDB.Open(TString(argv[3]), 1)) != 0)
			{
				fprintf(stderr, "* Cannot open Tiger DB: %s\n", argv[3]);
				goto CLEAN_UP;
			}

			const Range2D range = tDB.GetRange();
			int err = TopoTools::BuildTopology(tDB, range);
			err = tDB.Close();
			return 0;
		}

	  rt1Name = (const char *)&buffer[ 0 ];
//		rt1Name += tigerExts[RT1_EXT];
	  if( ( inFile1 = ::fopen( TString(rt1Name), "r" ) ) == NULL )
	  {
			printf( "* TgrTrans - cannot open required Tiger record 1 file" );
	    goto ERR_RETURN;
	  }

	  dirIdx = rt1Name.Find( _T("TGR") )/*rt1Name.ReverseFind( 'r' ) + 1*/;
	  dotIdx = rt1Name.ReverseFind( '.' );
	  rootName = rt1Name.Left(dotIdx);
		printf( "\nRoot name: %s\n", (const char*)rootName);
//	  rootName = rt1Name.Mid( dirIdx, dotIdx - dirIdx )/*rt1Name.Left(dotIdx)*/;

	  baseName = rootName/*rt1Name.Mid( dirIdx, dotIdx - dirIdx )*/;
	  countyFips = GetCountyFips( baseName );
	  stateFips = GetStateFips( baseName );

		printf( "\nState: %d, County: %d\n", stateFips, countyFips);

		if( doNames )
		{
			tgrNames.m_strFilter = _T("(STATE = ?) AND (COUNTY = ?) AND (NAME = ?)");
			tgrNames.m_nParams = 3;
			tgrNames.stateFips = stateFips;
			tgrNames.countyFips = countyFips;
			tgrNames.qName = _T("");
			tgrNames.Open( CRecordset::forwardOnly, 0, CRecordset::readOnly );
			while (!tgrNames.IsEOF())
				tgrNames.MoveNext();
		}
//
//	Pre-process the .f52 file
//
	  length = rt1Name.GetLength();

	  if( doLines )
	  {
		  fName = baseName + "Map.tab";
		  mappingFile = ::fopen(TString(fName), "w");
			//error = tDB.dacCreate(TString(argv[3]), 1 << 16);
			//error = dac.create(TString("C:\\Work\\Waldo2006.dac"), 1 << 16);
			//short s;
			//error = dac.open(TString("C:\\Work\\Waldo2006.dac"), 0, 1, &s);
#ifdef TO_TIGERDB
      if( doTigerDB && (error = tDB.Open( TString(argv[3]), 1)) != 0 )
			{
				fprintf( stderr, "* Cannot open Tiger DB: %s\n", argv[ 3 ] );
				goto CLEAN_UP;
			}
#endif
			rt1Name = rootName + tigerExts[RT2_EXT];
/*			rt1Name.SetAt( length - 1, '2' );*/
			nIndex = DoShapeIndex( rt1Name, &index );
			inFile2 = ::fopen( TString(rt1Name), "r" );
			fName = baseName + _T("l.tab");
			csvLines = fopen( TString(fName), "w" );

			nLook.stateFips = stateFips;
			nLook.countyFips = countyFips;
			nLook.tlid = 0;
			nLook.Open( CRecordset::forwardOnly, 0, CRecordset::readOnly );
			while( ! nLook.IsEOF() )
				nLook.MoveNext();
#ifdef SAVE_FOR_NOW
			distName.nameParam = "";
			distName.Open( CRecordset::forwardOnly, 0, CRecordset::readOnly );
			while( ! distName.IsEOF() )
				distName.MoveNext();
#endif
	  }

	  if( doBlocks )
	  {
	    fName = baseName + "b.tab";
	    csvBlocks = ::fopen( TString(fName), "w" );
/*  Don't open this file
	    fName = baseName + "b2.csv";
	    csvBlocks2 = ::fopen( TString(fName), "w" );
*/
	  }

	  if( doAddress )
	  {
			fName = baseName + "a.csv";
			csvAddr = ::fopen( TString(fName), "w" );
	  }

	  if( doNames )
	  {
			fName = baseName + "n.tab";
			if ((csvNames = ::fopen( TString(fName), "w" )) == 0)
				printf("** Cannot create file %s\n", (const char*)fName);
	  }

	  if( doPolys )
	  {
			fName = baseName + "p.csv";
			csvPoly = ::fopen( TString(fName), "w" );
	  }
//
//	Process the .F51 file
//
	  if( doLines || doBlocks || doAddress || doNames || doPolys )
	  {
			TigerRec1 rec1;
			while( rec1.GetNextRec( inFile1 ) > 0 )
			{
			  int count = 0;
			  unsigned nPoints = 2;

			  version = rec1.version;

			  nLines++;
				if( doNames && 
						( rec1.fn.fedirp[0] != '\0' || rec1.fn.fename[0] != '\0' ||
						  rec1.fn.fetype[0] != '\0' || rec1.fn.fedirs[0] != '\0' ) )
				{ 
					tgrNames.qName = rec1.fn.fename;
					tgrNames.Requery();
					int nFound = 0;
					CString foundType;
					long foundFeat = -1;
					while( ! tgrNames.IsEOF() )
					{
						tgrNames.m_dirp.TrimRight();
						tgrNames.m_dirs.TrimRight();
						tgrNames.m_type.TrimRight();
						nFound++;
						if( tgrNames.m_dirp == rec1.fn.fedirp && tgrNames.m_type == rec1.fn.fetype &&
								tgrNames.m_dirs == rec1.fn.fedirs )
						{
/*
							printf( "N: %s, T: %s, F: %ld\n", (const char *)TString(tgrNames.m_name),
							 (const char *)TString(tgrNames.m_type), tgrNames.m_feat );
*/
							DoNames( csvNames, stateFips, countyFips, tgrNames.m_feat, rec1.tlid, RTSQ, rec1.fn );
							while (!tgrNames.IsEOF())
								tgrNames.MoveNext();
							goto NEXT_LINE;
						}
						foundType = tgrNames.m_type;
						foundFeat = tgrNames.m_feat;
						tgrNames.MoveNext();
					}
					printf( "%ld\t%d\t%ld\t%ld\t%s\t%s\t%s\t%ld\n", nFound, stateFips, countyFips,
							foundFeat, (const char*)foundType, (const char*)rec1.fn.fename, (const char*)rec1.fn.fetype, rec1.tlid);
				}
NEXT_LINE :
	      if( doBlocks )
					DoBlocks( csvBlocks, csvBlocks2, rec1 );
			  if( doAddress )
					DoAddress( csvAddr, rec1.tlid, 0, rec1.ar );
#ifdef DO_LATER
			  if( doPolys ) DoPolygons( csvPoly, rec1 );
#endif
			  if( ! doLines ) continue;
//
//	Get points now so we can do MBR
//
				TigerDB::Classification cCode = MapCFCC( rec1.cfcc );

//	Process only the desired lines:
//	1) If line is a border
//	2) If line defines a census block
//	3) If line is a road, water or political boundary
				if (!((rec1.blkl != rec1.blkr || rec1.blkls != rec1.blkrs ||
					rec1.ctbnal != rec1.ctbnar || rec1.countyl != rec1.countyr ||
					rec1.statel != rec1.stater) ||
					(rec1.side != '\0') ||
					(rec1.cfcc[0] == 'A' || rec1.cfcc[0] == 'B' || rec1.cfcc[0] == 'F' || rec1.cfcc[0] == 'H')))
				{
					nLinesSkipped++;
					continue;
				}


				TigerDB::Name names[5];
				int nNames = 0;
//
//	If there is no name in the record 1 file, do not bother to look at all
				if( rec1.fn.fedirp[0] != '\0' || rec1.fn.fename[0] != '\0' ||
						rec1.fn.fetype[0] != '\0' || rec1.fn.fedirs[0] != '\0' )
				{
					struct ReatRec
					{
						const TCHAR *name;
						TigerDB::Classification baseCode;
					};

					static const ReatRec ReatRecs[] =
					{
						{_T("I -"),					TigerDB::ROAD_PrimaryLimitedAccess},
						{_T("I- "),					TigerDB::ROAD_PrimaryLimitedAccess},
						{_T("US Hwy "),			TigerDB::ROAD_PrimaryUnlimitedAccess},
						{_T("State Hwy "),	TigerDB::ROAD_SecondaryAndConnecting}
					};
//
//	REAT the line if it's a Road type
					if( rec1.cfcc[0] == 'A' )
					{
						int pos;

						CString name = rec1.fn.fename[0];
						for( int k = sizeof(ReatRecs)/sizeof(ReatRecs[0]); --k >= 0; )
						{
							if( (pos = name.Find(ReatRecs[k].name)) != -1)
							{
								if( cCode > ReatRecs[k].baseCode ||
										cCode == TigerDB::ROAD_MajorCategoryUnknown )
								{
									cCode = ReatRecs[k].baseCode;
									printf( "  Reated line %ld to %d\n", rec1.tlid, cCode);
									break;
								}
							}
						}
					}
#ifdef DO_REAT
					nLook.tlid = rec1.tlid;
					nLook.Requery();
					while( ! nLook.IsEOF() )
					{
//
//	REAT the line if it's a Road type
						if( rec1.cfcc[0] == 'A' )
						{
							int pos;

							for( int k = sizeof(ReatRecs)/sizeof(ReatRecs[0]); --k >= 0; )
							{
								if( (pos = nLook.m_name.Find(ReatRecs[k].name)) != -1)
								{
									if( cCode > ReatRecs[k].baseCode ||
											cCode == TigerDB::ROAD_MajorCategoryUnknown )
									{
										cCode = ReatRecs[k].baseCode;
										printf( "  Reated line %ld to %d\n", rec1.tlid, cCode);
										break;
									}
								}
							}
						}
						::_tcscpy(names[nNames].name, nLook.m_name);
						::_tcscpy( names[nNames].type, nLook.m_type.TrimRight() );
						::_tcscpy( names[nNames].prefix, nLook.m_dirp.TrimRight());
						::_tcscpy( names[nNames].suffix, nLook.m_dirs.TrimRight());

#ifdef SAVE_FOR_NOW
						distName.nameParam = nLook.m_name;
						distName.Requery();
						if( distName.IsEOF() )
						{
							fprintf( stderr, "* Cannot find name %s id\n", (const char *)nLook.m_name );
		 					names[ nNames ].nameId = 0;
						}
						else
						{
		 					names[ nNames ].nameId = distName.m_id;
						}

						if( nLook.m_type.IsEmpty() ||
							( code = TigerGetTypeCode( nLook.m_type ) ) < 0 )
							names[ nNames ].typeCode = 0;
						else
						{
							names[ nNames ].typeCode = code;
						}

						if( nLook.m_dirp.IsEmpty() ||
							( code = TigerGetPrefixCode( nLook.m_dirp ) ) < 0 )
							names[ nNames ].prefixCode = 0;
						else
						{
							names[ nNames ].prefixCode = code;
						}

						names[ nNames ].suffixCode = 0;
#endif
						nNames++;
						nLook.MoveNext();
					}
#endif
				}

			  long offset;
			  if( BinarySearch( rec1.tlid, index, nIndex, &offset ) )
			  {
			    TigerRec2 rec2;
			    long tlid = 0;

			    fseek( inFile2, index[ offset ].fpos, SEEK_SET );
			    nPoints += rec2.GetPoints( inFile2, &tlid, &points[ 1 ],
			      sizeof(points) / sizeof(points[0]));
			  }

			  if( ( rec1.blkl != rec1.blkr || rec1.blkls != rec1.blkrs ||
							rec1.ctbnal != rec1.ctbnar || rec1.countyl != rec1.countyr ||
							rec1.statel != rec1.stater ) && cCode == TigerDB::NotClassified )
				{
					fprintf( stderr, "TgrTrans: line %ld defines Census Block but no code\n",
							rec1.tlid );
				}

			  rec1.GetFromGeoPoint( &points[0] );
			  rec1.GetToGeoPoint( &points[nPoints - 1] );

				if( doTrendLine )
				{
					nPoints = TrendLine( points, nPoints, tlFilter );
				}

				if (doTigerDB)
 				{
#ifdef TO_TIGERDB
					ObjHandle dbo;

					if( ( error = tDB.NewObject( DB_GEO_LINE/*DB_TIGER_LINE*/, dbo/*, id*/)) != 0)
					{
						fprintf( stderr,  "**dbOM.newObject failed\n" );
						goto CLEAN_UP;
					}

					TigerDB::Chain *line = (TigerDB::Chain *)dbo.Lock();

					line->Init();
					line->line_coor.init();

/*					if ((error = line->Set((unsigned)nPoints, points)) != 0)
					{
						fprintf(stderr, "**line->Set() failed\n");
						goto CLEAN_UP;
					}*/
	//			 	line->Set( id );
/**/					Range2D box;
					box.Init();
					for (unsigned i = 0; i < nPoints; i++)
					{
						box.Add(points[i]);
					}

					line->SetMBR(box);
/**/
					line->userCode = cCode;
			 		//line->SetCode( cCode );
					line->SetName( names, nNames );
					//line->SetTLID(rec1.tlid);
					line->userId = rec1.tlid;
					if( (error = line->write()) != 0)
					{
						fprintf(stderr, "**line->write() failed\n");
						goto CLEAN_UP;
					}
//					line->Set((unsigned)nPoints, points);
					fprintf(mappingFile, "%ld\t%ld\n", rec1.tlid, line->dbAddress() );
	//#endif
					fflush( stdout );
					if ((error = tDB.dacInsert(line, line)) != 0)
					{
						fprintf(stderr, "**tDB.dacInsert() failed\n");
						goto CLEAN_UP;
					}
					//error = dac.insert(line, line);
					dbo.Unlock();
			
					if ((error = tDB.Add( dbo )) != 0)
					{
						fprintf(stderr, "**tDB.Add() failed\n");
						goto CLEAN_UP;
					}
					error = line->Set((unsigned)nPoints, points);
					tDB.TrBegin();
					if ((error = tDB.TrEnd()) != 0)
					{
						fprintf(stderr, "**tDB.TrEnd() failed\n");
						goto CLEAN_UP;
					}
					//error = dac.trend();
#endif
				}
				else
				{
//	Calculate MBR
//
					Range2D range;

					range.Envelope( points[0] );
					range.Envelope( points[nPoints - 1] );
					if( nPoints > 2 )
					{
						long i = nPoints - 2;
						while( --i >= 0 )
						{
							range.Envelope( points[i] );
						}
					}
					TigerParams params;

					params.fips = countyFips;
					params.code = cCode;
					params.border = rec1.side != '\0';

					WriteLine(csvLines, 0, rec1.tlid, points[0], points[nPoints - 1], range, params, nPoints,
									&points[1]);
				}
	    }

	    if( index != 0 )
	      delete [] index;

	    printf( "\nNumber of lines proccessed: %ld, skipped: %ld\n", nLines, nLinesSkipped );

	    if( csvLines ) fclose( csvLines );
	    if( csvBlocks ) fclose( csvBlocks );
	    if( csvBlocks2 ) fclose( csvBlocks2 );
	    if( csvPoly ) fclose( csvPoly );
	    if( inFile2 ) fclose( inFile2 );
			if (mappingFile) fclose(mappingFile);
	  }

	  fclose( inFile1 );

//
//	Process extra names
//
	  if( doNames )
	  {
	    length = rt1Name.GetLength();
			rt1Name = rootName + tigerExts[RT4_EXT];
//	    rt1Name.SetAt( length - 1, '4' );
	    if( ( inFile1 = ::fopen( TString(rt1Name), "r" ) ) != 0 )
	    {
//			  rt1Name.SetAt( length - 1, '5' );

//			  if( ( inFile2 = fopen( rt1Name, "r" ) ) != 0 )
			  {
			    TigerRec4 rec4;
				  TigerRec5 rec5;

			    while( rec4.GetNextRec( inFile1 ) > 0 )
			    {
					  for( int i = 0; i < rec4.nFeatures; i++ )
					  {
//					    fseek( inFile2, ( rec4.feats[ i ] - 1) * 54, SEEK_SET );
//					    rec5.GetNextRec( inFile2 );
							DoNames( csvNames, stateFips, countyFips, rec4.feats[ i ],
								rec4.tlid, RTSQ + i + 1, rec5.fn );
//					    DoNames( csvNames, rec4.tlid, RTSQ + i + 1, rec5.fn );
					  }
			    }
//			    fclose( inFile2 );
			  }
//			  else
//			    printf( "Cannot open %s file\n", (const char *)rt1Name );

			  fclose( inFile1 );
	    }
	    fclose( csvNames );
	  }
//
//	Process extra addresses
//
	  if( doAddress )
	  {
 			rt1Name = rootName + tigerExts[RT6_EXT];
//	    rt1Name.SetAt( length - 1, _T('6') );
	    if( ( inFile1 = ::fopen( TString(rt1Name), "r" ) ) != 0 )
	    {
			  TigerRec6 rec;

			  while( rec.GetNextRec( inFile1 ) > 0 )
			  {
			    DoAddress( csvAddr, rec.tlid, rec.rtsq, rec.ar );
			  }
			  fclose( inFile1 );
	    }
	    fclose( csvAddr );
	  }

//
//	Process record "I" - GT/Poly-Chain link
//
	  if( doGTpoly )
	  {
			FILE *csvFile = 0;
			error = tDB.Open(TString(argv[3]), 1);

//
//	Load record "8" - Landmark links into memory
//
			rt1Name = rootName + tigerExts[RT8_EXT];
			if ((inFile1 = fopen(rt1Name, "r")) == 0)
			{
				printf("Cannot open %s\n", (const char*)rt1Name);
				goto ERR_RETURN;
			}

			struct LMLink {
				char cenid[6];
				long polyid;
			};
			std::map<int, LMLink> llMap;
			TigerRec8 rec8;
			int count = 0;
			while (rec8.GetNextRec(inFile1) > 0)
			{
				LMLink ll;
				memcpy(ll.cenid, rec8.cenid, sizeof(rec8.cenid));
				ll.cenid[5] = '\0';
				ll.polyid = rec8.polyid;
				llMap.insert({ rec8.land, ll });
				count++;
				//DoLMlink(csvFile, rec8);
			}
			fclose(inFile1);
//
// Load GT polygons
			rt1Name = rootName + tigerExts[RTI_EXT];
			if ((inFile1 = fopen(rt1Name, "r")) == 0)
			{
				printf("Cannot open %s\n", (const char*)rt1Name);
				goto ERR_RETURN;
			}
			struct GTPoly {
				long tlid;
				char cenidl[6];
				long polyidl;
				char cenidr[6];
				long polyidr;
			};
			std::vector<GTPoly> gtPolys;
			count = 0;
			TigerRecI recI;
			while (recI.GetNextRec(inFile1) > 0)
			{
				GTPoly gtp;
				gtp.tlid = recI.tlid;
				memcpy(gtp.cenidl, recI.cenidl, sizeof(recI.cenidl));
				gtp.cenidl[5] = '\0';
				gtp.polyidl = recI.polyidl;
				memcpy(gtp.cenidr, recI.cenidr, sizeof(recI.cenidr));
				gtp.cenidr[5] = '\0';
				gtp.polyidr = recI.polyidr;
				gtPolys.push_back(gtp);
				count++;
				//DoGTpoly(csvFile, recI);
			}
			//
			//	Process record "P" - 
			//
			rt1Name = rootName + tigerExts[RTP_EXT];
			if ((inFile1 = fopen(rt1Name, "r")) == 0)
			{
				printf("Cannot open %s\n", (const char*)rt1Name);
				goto ERR_RETURN;
			}
			struct GeoPoint
			{
				long lat,
					lon;
			};
			std::map<std::string, GeoPoint> centroidMap;
			TigerRecP recP;
			while (recP.GetNextRec(inFile1) > 0)
			{
				GeoPoint gp;
				gp.lat = recP.polyLat;
				gp.lon = recP.polyLong;
				std::string s = recP.cenid + std::to_string(recP.polyid);

				centroidMap.insert({ s, gp });
				printf("PolyID: %d, Lat: %d, Long: %d\n", recP.polyid, recP.polyLat, recP.polyLong);
			}
			::fclose(inFile1);
//
//	Process record "7" - Landmark links
//
			rt1Name = rootName + tigerExts[RT7_EXT];
			if ((inFile1 = fopen(rt1Name, "r")) == 0)
			{
				printf("Cannot open %s\n", (const char*)rt1Name);
				goto ERR_RETURN;
			}

			int nPolysFound = 0;
			// Process Landmark polygons
			TigerRec7 rec7;
			while (rec7.GetNextRec(inFile1) > 0)
			{
				std::vector<GeoDB::DirLineId> dirLineIds;
				if (rec7.lalong != -1 || rec7.lalat != -1)  // Point Landmarks
					continue;
				if (rec7.cfcc[0] != 'H')  // Temporary
					continue;
				std::map<int, LMLink>::iterator it = llMap.find(rec7.land);
				if (it == llMap.end())
					continue;
				count = 0;

				// Search for the GT poly records
				LMLink ll = it->second;
				for (std::vector<GTPoly>::iterator it2 = gtPolys.begin(); it2 != gtPolys.end(); ++it2)
				{
					GeoDB::DirLineId dl;
					if (strcmp(it2->cenidl, ll.cenid) == 0 && it2->polyidl == ll.polyid)
					{
						dl.id = it2->tlid;
						dl.dir = -1;
						dirLineIds.push_back(dl);
						count++;
					}
					else if (strcmp(it2->cenidr, ll.cenid) == 0 && it2->polyidr == ll.polyid)
					{
						dl.id = it2->tlid;
						dl.dir = 1;
						dirLineIds.push_back(dl);
						count++;
					}
				}
				/*printf("Count = %ld\n", count);
				if (count == 1)
					continue;*/
				printf("Processing: %d, name: %s\n", rec7.land, rec7.laname);
				int err;
				struct PolySegment
				{
					double area;
					Range2D mbr;
					int start, end;
					double xcg, ycg;
				};
				std::vector<GeoDB::DirLineId> orderedLines;
				std::vector<PolySegment> polySegs;

				int start = 0;
				DbHash dbHash;
				while (dirLineIds.size() > 0)
				{
					GeoDB::DirLineId dl = dirLineIds[0];
					orderedLines.push_back(dl);
					dirLineIds[0] = dirLineIds.back();// [dirLineIds.size() - 1] ;
					dirLineIds.pop_back();
					//
	//	Go through & calculate area, centroid & MBR
	//
					XY_t sPt,
						ePt;
					ObjHandle oh;

					dbHash.tlid = dl.id;
					err = tDB.dacSearch(DB_GEO_LINE, &dbHash, oh);
					assert(err == 0);
					TigerDB::Chain* line = (TigerDB::Chain*)oh.Lock();
					Range2D mbr;
					mbr = line->GetMBR();
					line->GetNodes(&sPt, &ePt);
					line->Get(points);
					int nPts = line->GetNumPts();
					oh.Unlock();
					XY_t lastPt,
						startPt;
					if (dl.dir > 0)
						lastPt = ePt;
					else
					{
						lastPt = sPt;
						sPt = ePt;
					}

					double xcg = 0.0,
						ycg = 0.0,
						xcom = sPt.x,
						ycom = sPt.y,
						rval = 0.0;

					double area = CalcArea(points, nPts, xcom, ycom, &xcg, &ycg, 1);
					if (dl.dir > 0)
						rval += area;
					else
						rval -= area;
					/*if (dl.dir < 0)
						rval += CalcArea(points, nPts, xcom, ycom, &xcg, &ycg, -1);
					else
						rval += CalcArea(points, nPts, xcom, ycom, &xcg, &ycg, 1);*/
					int nSegs = 1;
					while (!(lastPt == sPt))
					{
						int i;
						bool found = false;
						for (i = 0; i < dirLineIds.size(); i++)
						{
							dl = dirLineIds[i];
							dbHash.tlid = dl.id;
							err = tDB.dacSearch(DB_TIGER_LINE, &dbHash, oh);
							TigerDB::Chain* line = (TigerDB::Chain*)oh.Lock();
							XY_t sNode, eNode;
							line->GetNodes(&sNode, &eNode);
							mbr.Envelope(line->GetMBR());
							line->Get(points);
							nPts = line->GetNumPts();
							oh.Unlock();
							if (dl.dir < 0)
							{
								XY_t temp = eNode;
								eNode = sNode;
								sNode = temp;
							}
							if (sNode == lastPt)
							{
								double area = CalcArea(points, nPts, xcom, ycom, &xcg, &ycg, 1);
								if (dl.dir > 0)
									rval += area;
								else
									rval -= area;
								/*if (dl.dir < 0)
								{
									rval += CalcArea(points, nPts, xcom, ycom, &xcg, &ycg, -1);
								}
								else
								{
									rval += CalcArea(points, nPts, xcom, ycom, &xcg, &ycg, 1);
								}*/
								lastPt = eNode;
								orderedLines.push_back(dl);
								dirLineIds[i] = dirLineIds.back();// [dirLineIds.size() - 1] ;
								dirLineIds.pop_back();
								nSegs += 1;
								found = true;
								break;
							}
						}
						if (!found)
						{
							printf("** Processing %d, did not find a closed loop.  Directed Edges left: %d\n", rec7.land, dirLineIds.size());
							nSegs = 0;
							break;
						}
						//assert(i < dirLineIds.size());
					}
					if (nSegs == 0)  // Error condition!
					{
						break;
					}
					PolySegment ps;
					if (rval < 0.0)
						rval = -rval;
					ps.area = rval;
					ps.mbr = mbr;
					{
						double areai = 1.0 / rval;
						if ((xcg = (xcg * areai + xcom) * (1.0 / 3.0)) <= 0.0)
							/*fprintf(stderr, "Centroid X: %lf\n", xcg)*/;
						if ((ycg = (ycg * areai + ycom) * (1.0 / 3.0)) <= 0.0)
							/*fprintf(stderr, "Centroid Y: %lf\n", ycg)*/;
					}
					ps.ycg = (ycg/* + 0.5*/);
					ps.xcg = (xcg/* + 0.5*/);
					ps.start = start;
					ps.end = orderedLines.size();
					start += nSegs;
					polySegs.push_back(ps);
				}
				
				struct greater_than_key
				{
					inline bool operator() (const PolySegment& struct1, const PolySegment& struct2)
					{
						return (struct1.area > struct2.area);
					}
				};

				std::sort(polySegs.begin(), polySegs.end(), greater_than_key());  // Put the largest polygon first
				/*//int last = 0;
				for (int i = 0; i < polySegs.size(); i++)
				{
					printf("Polyseg - Area: %.6f, Count: %d\n", polySegs[i].area, polySegs[i].end - polySegs[i].start);
					//last = polySegs[i].end;
				}*/
				ObjHandle po;
				GeoDB::Poly* poly = 0;
				for (int j = 0; j < polySegs.size(); j++)
				{
					PolySegment ps = polySegs[j];
					/**/

					if (j == 0)
					{
						// Search and see if we find the matching polygon already constructed
						ObjHandle so;
						GeoDB::Search ss;

						XY_t centroid;
						centroid.x = ps.xcg;
						centroid.y = ps.ycg;

						Range2D range;
						range.Envelope(centroid);

						tDB.Init(/*range*/ps.mbr, &ss);
						while (tDB.GetNext(&ss, &so) == 0)
						{
							GeoDB::SpatialObj* spatialObj = (GeoDB::SpatialObj*)so.Lock();
							GeoDB::SpatialClass sc = spatialObj->IsA();
							if (sc == GeoDB::AREA)
							{
								nPolysFound++;
								//int nPts = TigerDB::Polygon::GetPts(so, points);
								GeoDB::Poly* poly = (GeoDB::Poly*)spatialObj;
								//printf("Poly: %ld with classification: %d", spatialObj->dbAddress(), poly->GetCode());
								int match = poly->matchPoly(so, orderedLines, ps.start, ps.end);
								if (match > 0)
								//if (isPolygonMatch(so, orderedLines, ps.start, ps.end))
								{
									printf("* Poly: %ld matches %s (%d)\n", poly->dbAddress(), rec7.laname, match);
								}
								//printf("\n");
							}
							so.Unlock();
						}
						break; // Don't create the Polygon now
						if ((err = tDB.NewObject(GeoDB::DB_POLY, po)) != 0)
						{
							fprintf(stderr, "**dbOM.newObject failed\n");
						}

						poly = (GeoDB::Poly*)po.Lock();
						poly->SetCode(MapCFCC(rec7.cfcc));
						poly->SetArea(ps.area);
						poly->SetMBR(ps.mbr);

						poly->write();
						//po.Unlock();
						tDB.Add(po);
					}
					for (int i = ps.end; --i >= ps.start; )
						//	for (int i = 0; i < lineCount; i++)
					{
						GeoDB::DirLineId& lineId = orderedLines[i];
						dbHash.tlid = lineId.id;
						ObjHandle eh;
						err = tDB.dacSearch(DB_GEO_LINE, &dbHash, eh);
						assert(err == 0);
						if (j > 0)  // Changing direction of edge in an island
							lineId.dir = -lineId.dir;
						err = poly->AddEdge(eh, lineId.dir);
					}
					break;  // Islands don't work now
				}
				if (poly != 0)
				{
					po.Unlock();

					tDB.TrBegin();
					tDB.TrEnd();
				}
				/**/

				//DoLMarea(csvFile, rec7);
			}
			printf("Number of Hydro polygons found: %d\n", nPolysFound);
/*
			rt1Name = rootName + tigerExts[RTI_EXT];
//	    rt1Name.SetAt( length - 1, 'i' );
	    if( ( inFile1 = fopen( rt1Name, "r" ) ) == 0 )
	    {
			  printf( "Cannot open %s\n", (const char *)rt1Name );
			  goto ERR_RETURN;
	    }

	    fName = baseName + "gt.csv";
	    if( ( csvFile = fopen( fName, "w" ) ) == 0 )
			{
			  printf( "Cannot open %s\n", (const char *)fName );
			  fclose( inFile1 );
			  goto ERR_RETURN;
			}

	    TigerRecI recI;
			while( recI.GetNextRec( inFile1 ) > 0 )
			{
			  DoGTpoly( csvFile, recI );
			}

			fclose( inFile1 );
			fclose( csvFile );
//
//	Process record "8" - Landmark links
//
			rt1Name = rootName + tigerExts[RT8_EXT];
//	    rt1Name.SetAt( length - 1, '8' );
	    if( ( inFile1 = fopen( rt1Name, "r" ) ) == 0 )
	    {
			  printf( "Cannot open %s\n", (const char *)rt1Name );
			  goto ERR_RETURN;
	    }

	    fName = baseName + "ll.csv";
	    if( ( csvFile = fopen( fName, "w" ) ) == 0 )
			{
			  printf( "Cannot open %s\n", (const char *)fName );
			  fclose( inFile1 );
			  goto ERR_RETURN;
			}

			TigerRec8 rec8;
			while( rec8.GetNextRec( inFile1 ) > 0 )
			{
			  DoLMlink( csvFile, rec8 );
			}

			fclose( inFile1 );
			fclose( csvFile );
//
//	Process record "7" - Landmark links
//
			rt1Name = rootName + tigerExts[RT7_EXT];
//	    rt1Name.SetAt( length - 1, '7' );
	    if( ( inFile1 = fopen( rt1Name, "r" ) ) == 0 )
	    {
		  printf( "Cannot open %s\n", (const char *)rt1Name );
		  goto ERR_RETURN;
	    }

	    fName = baseName + "lm.csv";		// landmark area & line features
	    if( ( csvFile = fopen( fName, "w" ) ) == 0 )
		{
		  printf( "Cannot open %s\n", (const char *)fName );
		  fclose( inFile1 );
		  goto ERR_RETURN;
		}

	    fName = baseName + "lp.csv";			// landmark points
	    if( ( inFile2 = fopen( fName, "w" ) ) == 0 )
			{
			  printf( "Cannot open %s\n", (const char *)fName );
			  fclose( inFile1 );
			  fclose( csvFile );
			  goto ERR_RETURN;
			}

			TigerRec7 rec7;
			while( rec7.GetNextRec( inFile1 ) > 0 )
			{
			  if( rec7.lalong == -1 && rec7.lalat == -1 )
			    DoLMarea( csvFile, rec7 );
			  else
			    DoLMpoint( inFile2, rec7 );
			}

			fclose( inFile1 );
			fclose( inFile2 );
			fclose( csvFile );
*/
	  }
#ifdef DO_LATER
		//
		//	Process History file - Tiger 94
		//
		if (doHistory)
		{
			FILE* csvFile = 0;

			rt1Name = rootName + tigerExts[RTH_EXT];
			//	    rt1Name.SetAt( length - 1, 'H' );
			if ((inFile1 = fopen(rt1Name, "r")) != 0)
			{
				TigerRecH rec;

				fName = baseName + "H.csv";
				if ((csvFile = fopen(fName, "w")) == 0)
				{
					printf("* Cannot open %s\n", (const char*)fName);
					fclose(inFile1);
					goto ERR_RETURN;
				}

				while (rec.GetNextRec(inFile1) > 0)
				{
					DoHistory(csvFile, rec);
				}
				fclose(inFile1);
			}
			if (csvFile)
				fclose(csvFile);
		}
	  
		if( doZips || version == TIGER_94 )
	  {
	  	FILE *csvFile = 0;
			TigerRecZ rec;

			rt1Name = rootName + tigerExts[RTZ_EXT];
//	    rt1Name.SetAt( length - 1, 'Z' );
	    if( ( inFile1 = fopen( rt1Name, "r" ) ) == 0 )
	    {
		  printf( "Cannot open %s\n", (const char *)rt1Name );
		  goto ERR_RETURN;
	    }

	    fName = baseName + "Z.csv";		// zip+4 file
	    if( ( csvFile = fopen( fName, "w" ) ) == 0 )
			{
			  printf( "Cannot open %s\n", (const char *)fName );
			  fclose( inFile1 );
			  goto ERR_RETURN;
			}

			while( rec.GetNextRec( inFile1 ) > 0 )
			{
			  fprintf( csvFile, "%ld,%d,%d,%d\n", rec.tlid, rec.rtsq, rec.zip4L, rec.zip4R );
			}

			fclose( inFile1 );
			fclose( csvFile );
	  }
#endif
CLEAN_UP :
		if( tgrNames.IsOpen() )
			tgrNames.Close();
		if( nLook.IsOpen() )
			nLook.Close();
#ifdef SAVE_FOR_NOW
		if( distName.IsOpen() )
			distName.Close();
#endif
		if( db.IsOpen() )
			db.Close();

#ifdef TO_TIGERDB
		if( tDB.IsOpen() )
			tDB.Close();
#endif
	  return( 0 );
	}
	catch( CDBException *e )
	{
		printf( "* %s\n", (const char *)TString(e->m_strError) );
		e->Delete();
	}
	catch( ... )
	{
		printf( " *Unknown exception\n" );
	} 

USAGE_ERROR :
  printf( "TgrTrans <*.*1> [/LIHPBANZ[0|1|2]]\n" );

ERR_RETURN :
  return( -1 );
}

static TigerDB::Classification MapCFCC( const char *cfcc )
{
  int cfccNum = atoi( &cfcc[1] );
  TigerDB::Classification code = TigerDB::NotClassified;

  switch( cfcc[0] )
  {
	default :
	  fprintf( stderr,  "*MapCFCC - invalid CFCC Code: %c\n", cfcc[ 0 ] );
	  break;

  case 'P':  // Provisional Code - added in Census 2000
  case 'A' :
		switch( cfccNum )
		{
			default :
			  fprintf( stderr,  "*MapCFCC - Unknown CFCC A: %d\n", cfccNum );
				break;

			case 0 :
			case 1 :
			case 2 :
			case 3 :
			case 4 :
			case 5 :
			case 6 :
			case 7 :
			case 8 :
			case 10 :
				code = TigerDB::ROAD_MajorCategoryUnknown;
				break;

			case 11	:
			case 12 :
			case 13 :
			case 14 :
			case 15 :
			case 16 :
			case 17 :
			case 18 :
				code = TigerDB::ROAD_PrimaryLimitedAccess;
				break;

			case 21 :
			case 22 :
			case 23 :
			case 24 :
			case 25 :
			case 26 :
			case 27 :
			case 28 :
				code = TigerDB::ROAD_PrimaryUnlimitedAccess;
				break;

			case 31 :
			case 32 :
			case 33 :
			case 34 :
			case 35 :
			case 36 :
			case 37 :
			case 38 :
				code = TigerDB::ROAD_SecondaryAndConnecting;
				break;

			case 41 :
			case 42 :
			case 43 :
			case 44 :
			case 45 :
			case 46 :
			case 47 :
			case 48 :
				code = TigerDB::ROAD_LocalNeighborhoodAndRural;
				break;

			case 51 :
			case 52 :
			case 53 :
				code = TigerDB::ROAD_VehicularTrail;
				break;

			case 60 :
				code = TigerDB::ROAD_SpecialCharacteristics;
				break;

			case 61 :
				code = TigerDB::ROAD_Cul_de_sac;
				break;

			case 62 :
				code = TigerDB::ROAD_TrafficCircle;
				break;

			case 63 :
				code = TigerDB::ROAD_AccessRamp;
				break;

			case 64 :
				code = TigerDB::ROAD_ServiceDrive;
				break;

			case 65 :
				code = TigerDB::ROAD_FerryCrossing;
				break;

			case 71 :
			case 72 :
			case 73 :
			case 74 :
				code = TigerDB::ROAD_OtherThoroughfare;
				break;
		}
	  break;

	case 'B' :
		switch( cfccNum )
		{
			default :
			  fprintf( stderr,  "*MapCFCC - Unknown CFCC B: %d\n", cfccNum );
				break;

		case 1 :
		case 2 :
		case 3 :
			code = TigerDB::RR_MajorCategoryUnknown;
			break;

		case 11 :
		case 12 :
		case 13 :
			code = TigerDB::RR_MainLine;
			break;

		case 21 :
		case 22 :
		case 23 :
			code = TigerDB::RR_Spur;
			break;

		case 31 :
		case 32 :
		case 33 :
			code = TigerDB::RR_Yard;
			break;

		case 40 :
			code = TigerDB::RR_SpecialCharacteristics;
			break;

		case 50 :
		case 51 :
		case 52 :
			code = TigerDB::RR_OtherThoroughfare;
			break;
		}
	  break;
				  
	case 'C' :
		switch( cfccNum )
		{
			default :
			  fprintf( stderr,  "*MapCFCC - Unknown CFCC C: %d\n", cfccNum );
				break;

			case 0 :
				code = TigerDB::MGT_CategoryUnknown;
				break;

			case 10 :
				code = TigerDB::MGT_Pipeline;
				break;

			case 20 :
				code = TigerDB::MGT_PowerLine;
				break;

			case 30 :
			case 31 :
				code = TigerDB::MGT_SpecialCharacteristics;
				break;
		}
	  break;

	case 'D' :
		switch( cfccNum )
		{
			default :
			  fprintf( stderr,  "*MapCFCC - Unknown CFCC D: %d\n", cfccNum );
				break;

			case 0 :
				code = TigerDB::LM_CategoryUnknown;
				break;

			case 10 :
				code = TigerDB::LM_MilitaryInstallation;
				break;

			case 20 :
				code = TigerDB::LM_MultihouseholdOrTransientQuarters;
				break;

			case 21 :
			case 22 :
			case 26 :
				code = TigerDB::LM_ApartmentBuildingOrBoardingHouse;
				break;

			case 23 :
				code = TigerDB::LM_MobileHomePark;
				break;

			case 24 :
				code = TigerDB::LM_Marina;
				break;

			case 25 :
				code = TigerDB::LM_CrewOfVessel;
				break;

			case 27 :
				code = TigerDB::LM_HotelOrMotel;
				break;

			case 28 :
				code = TigerDB::LM_Campground;
				break;

			case 29 :
				code = TigerDB::LM_ShelterOrMission;
				break;

			case 30 :
				code = TigerDB::LM_CustodialFacility;
				break;
			case 31 :
				code = TigerDB::LM_Hospital;
				break;
			case 32 :
				code = TigerDB::LM_HalfwayHouse;
				break;

			case 33 :
				code = TigerDB::LM_NursingHome;
				break;

			case 34 :
				code = TigerDB::LM_CountyHome;
				break;

			case 35 :
				code = TigerDB::LM_Orphanage;
				break;

			case 36 :
				code = TigerDB::LM_Jail;
				break;

			case 37 :
				code = TigerDB::LM_FederalOrStatePrison;
				break;

			case 40 :
				code = TigerDB::LM_EducationalOrReligiousInstitution;
				break;
			case 41 :
				code = TigerDB::LM_SororityOrFraternity;
				break;
			case 42 :
				code = TigerDB::LM_ConventOrMonastery;
				break;
			case 43 :
				code = TigerDB::LM_EducationalInstitution;
				break;
			case 44 :
				code = TigerDB::LM_ReligiousInstitution;
				break;

			case 50 :
				code = TigerDB::LM_TransportationTerminal;
				break;
			case 51 :
				code = TigerDB::LM_Airport;
				break;
			case 52 :
				code = TigerDB::LM_TrainStation;
				break;
			case 53 :
				code = TigerDB::LM_BusTerminal;
				break;
			case 54 :
				code = TigerDB::LM_MarineTerminal;
				break;
			case 55 :
				code = TigerDB::LM_SeaplaneAnchorage;
				break;

			case 60 :
				code = TigerDB::LM_Employmentcenter;
				break;
			case 61 :
				code = TigerDB::LM_ShoppingOrRetailCenter;
				break;
			case 62 :
				code = TigerDB::LM_IndustrialBuildingOrPark;
				break;
			case 63 :
				code = TigerDB::LM_OfficebuildingOrPark;
				break;
			case 64 :
				code = TigerDB::LM_AmusementCenter;
				break;
			case 65 :
				code = TigerDB::LM_GovernmentCenter;
				break;
			case 66 :
				code = TigerDB::LM_OtherEmploymentCenter;
				break;

			case 70 :
				code = TigerDB::LM_Tower;
				break;
			case 71 :
				code = TigerDB::LM_LookoutTower;
				break;

			case 80 :
				code = TigerDB::LM_OpenSpace;
				break;
			case 81 :
				code = TigerDB::LM_GolfCourse;
				break;
			case 82 :
				code = TigerDB::LM_Cemetery;
				break;
			case 83 :
				code = TigerDB::LM_NationalParkOrForest;
				break;
			case 84 :
				code = TigerDB::LM_OtherFederalLand;
				break;
			case 85 :
				code = TigerDB::LM_StateOrLocalPark_Forest;
				break;

			case 90 :
				code = TigerDB::LM_SpecialPurpose;
				break;

			case 91 :
				code = TigerDB::LM_POBox_ZipCode;
				break;

			case 92 :
				code = TigerDB::LM_Urbanizacion;
				break;
		}
	  break;
				  
	case 'E' :
		switch( cfccNum )
		{
			default :
			  fprintf( stderr,  "*MapCFCC - Unknown CFCC E: %d\n", cfccNum );
				break;

			case 0 :
				code = TigerDB::PF_CategoryUnknown;
				break;

			case 10 :
				code = TigerDB::PF_Fenceline;
				break;

			case 20 :
				code = TigerDB::PF_TopographicFeature;
				break;
			case 21 :
				code = TigerDB::PF_RidgeLine;
				break;
			case 22 :
				code = TigerDB::PF_MountainPeak;
				break;
			case 23 :
				code = TigerDB::PF_Island;
				break;
		}
	  break;
				  
	case 'F' :
		switch( cfccNum )
		{
			default :
			  fprintf( stderr,  "*MapCFCC - Unknown CFCC F: %d\n", cfccNum );
				break;

			case 0 :
				code = TigerDB::NVF_BoundaryClassificationUnknown;
				break;

			case 10 :
			case 11 :
			case 12 :
			case 13 :
			case 14 :
			case 15 :
			case 16 :
				code = TigerDB::NVF_LegalOrAdministrativeBoundary;
				break;

			case 20 :
			case 21 :
			case 22 :
			case 23 :
			case 30 :		// Point-to-point line???
				code = TigerDB::NVF_ClosureExtension;
				break;

			case 24 :
				code = TigerDB::NVF_SeparationLine;
				break;
			case 25 :
				code = TigerDB::NVF_Centerline;
				break;

			case 40 :
				code = TigerDB::NVF_PropertyLine;
				break;

			case 50 :
				code = TigerDB::NVF_ZIPCodeBoundary;
				break;

			case 60 :		// Map edge
				break;

			case 70 :
			case 71 :
			case 72 :
			case 73 :
			case 74 :
				code = TigerDB::NVF_StatisticalBoundary;
				break;

			case 80 :
			case 81 :
			case 82 :
			case 83 :
			case 84 :
			case 85 :
				code = TigerDB::NVF_OtherTabulationBoundary;
				break;
		}
	  break;
				  
	case 'H' :
		switch( cfccNum )
		{
			default :
			  fprintf( stderr,  "*MapCFCC - Unknown CFCC H: %d\n", cfccNum );
				break;

			case 0 :
				code = TigerDB::HYDRO_ClassificationUnknown;
				break;
			case 1 :
				code = TigerDB::HYDRO_PerennialShoreline;
				break;
			case 2 :
				code = TigerDB::HYDRO_IntermittentShoreline;
				break;
			
			case 10 :
			case 11 :
				code = TigerDB::HYDRO_PerennialStream;
				break;
			case 12 :
				code = TigerDB::HYDRO_IntermittentStream;
				break;
			case 13 :
				code = TigerDB::HYDRO_BraidedStream;
				break;

			case 20 :
			case 21 :
				code = TigerDB::HYDRO_PerennialCanalDitchOrAqueduct;
				break;
			case 22 :
				code = TigerDB::HYDRO_IntermittentCanalDitchOrAqueduct;
				break;

			case 30 :
			case 31 :
				code = TigerDB::HYDRO_PerennialLakeOrPond;
				break;
			case 32 :
				code = TigerDB::HYDRO_IntermittentLakeOrPond;
				break;

			case 40 :
			case 41 :
				code = TigerDB::HYDRO_PerennialReservoir;
				break;
			case 42 :
				code = TigerDB::HYDRO_IntermittentReservoir;
				break;

			case 50 :
			case 51 :
				code = TigerDB::HYDRO_BayEstuaryGulfOrSound;
				break;
			case 53 :
				code = TigerDB::HYDRO_SeaOrOcean;
				break;

			case 60 :
				code = TigerDB::HYDRO_GravelPitOrQuarry;
				break;

			case 70 :
			case 74 :				// Census water boundary - separates inland from coastal
				code = TigerDB::NVF_WaterAreaDefinitionBoundary;
				break;

			case 71 :
				code = TigerDB::NVF_USGSClosureLine;
				break;

			case 72 :
				code = TigerDB::NVF_CensusWaterCenterLine;
				break;

			case 73 :
				code = TigerDB::NVF_CensusWaterBoundary12Mile;
				break;

			case 75 :
				code = TigerDB::NVF_CensusWaterBoundary3Mile;
				break;

			case 80 :
				break;

			case 81 :
				code = TigerDB::HYDRO_Glacier;
				break;
		}
	  break;

	case 'X' :
		break;
  }
  
  return( code );
}

static short GetStateFips( const TCHAR *string )
{
	wchar_t state[3];

  const char *pos/* = (TCHAR *)string + 5*/;

  if( ( pos = strstr(string, "TGR")) == 0)
    return( 0 );

	state[ 0 ] = pos[3]/*string[3]*/;
	state[ 1 ] = pos[4]/*string[4]*/;
	state[ 2 ] = '\0';

  return( (short)_wtoi( state ) );
}

static short GetCountyFips( const TCHAR *string )
{
  const char *pos/* = (TCHAR *)string + 5*/;

  if( ( pos = ::strstr( string, "TGR") ) == 0 )
    return( 0 );
  
	pos += 5;

  return( (short)atoi( pos ) );
}

static long fsize( const TCHAR *fname )
{
  long size;
  struct stat fs;

  size = 0;
  if( *fname && ! stat( TString(fname), &fs ))
		size = fs.st_size;

  return( size );
}

static int __cdecl compare( const void *rec1, const void *rec2 )
{
  if( ((i_rnum *)rec1)->rnum < ((i_rnum *)rec2)->rnum )
    return( -1 );

  if( ((i_rnum *)rec1)->rnum > ((i_rnum *)rec2)->rnum )
    return( 1 );

  return( 0 );
}

static long DoShapeIndex( const TCHAR *fname, i_rnum **array )
{
  long size = fsize( fname );
  long nRecs = size / 210;
  char record[ 256 ];
  long tlid = 0;
  long fpos = 0;

  i_rnum *index = new i_rnum[ nRecs ];

  FILE *file = ::fopen( TString(fname), "r" );
  long count = 0;
  while( --nRecs >= 0 )
  {
		fpos = ftell( file );

    if( ::fgets( record, sizeof( record ) - 1, file ) == NULL )
		{
		  puts( "Error reading F52 file" );
		  break;
		}
		record[ 15 ] = '\0';

		long rec_num = atol( &record[ 5 ] );
		if( rec_num != tlid )
		{
		  index[ count ].rnum = rec_num;	  
		  index[ count ].fpos = fpos;	  
		  count++;
		  tlid = rec_num;
		}

//	fpos += 210;
  }

  fclose( file );

  qsort( index, count, sizeof( i_rnum ), compare );

  *array = index;

  return( count );
}

//	bin_search
//	Perform the actual recursive binary search on 'idx' with 'cnt' records
//	for record number 'rnum' as described by BinarySearch().
//	Returns: TRUE if 'rnum' is found in the index.
static int BinarySearch( long rnum, i_rnum *idx, long cnt, long *pos )
{
  long fpos;
  int ret = 0;

  if( cnt )
  {
		long half = cnt / 2;

		if( idx[ half ].rnum == rnum)
    {
      *pos = half;
      ret = 1;
    }
    else if( idx[ half ].rnum > rnum )
    {
      if( ret = BinarySearch( rnum, idx, half, &fpos ) )
        *pos = fpos;
    }
    else
    {
      if( ret = BinarySearch( rnum, &idx[ half + 1 ], cnt - half - 1, &fpos ) )
        *pos = half + fpos + 1;
    }
  }

  return( ret );
}

inline void DoCalc(
	const XY_t& pt,
	const double& xcom, const double& ycom,
	double& xold, double& yold,
	double* xcg, double* ycg, double& area
)
{
	double x = (double)pt.x,
		y = (double)pt.y,
		aretri = (xcom - x) * (yold - ycom) + (xold - xcom) * (y - ycom);

	*xcg += aretri * (x + xold);
	*ycg += aretri * (y + yold);
	area += aretri;
	xold = x;
	yold = y;
}

static double CalcArea(XY_t pts[], int nPts,
	const double& xcom, const double& ycom, double* xcg, double* ycg, int dir)
{
	double area = 0.0;

	if (dir > 0)
	{
		double xold = pts[ 0 ].x,
			yold = pts[ 0].y;

		if (nPts > 2)
		{
			for (int i = 1; i < nPts; i++)
			{
				double x = pts[i].x,
					y = pts[i].y,
					aretri = (xcom - x) * (yold - ycom) + (xold - xcom) * (y - ycom);

				*xcg += aretri * (x + xold);
				*ycg += aretri * (y + yold);
				area += aretri;
				xold = x;
				yold = y;
			}
		}
		else
			DoCalc(pts[nPts - 1], xcom, ycom, xold, yold, xcg, ycg, area);
	}
	else
	{
		--nPts;
		double xold = pts[ nPts ].x,
			yold = pts[ nPts ].y;

		if (nPts > 2)
		{
			for (int i = nPts - 1; --i >= 0; )
			{
				double x = pts[i].x,
					y = pts[i].y,
					aretri = (xcom - x) * (yold - ycom) + (xold - xcom) * (y - ycom);

				*xcg += aretri * (x + xold);
				*ycg += aretri * (y + yold);
				area += aretri;
				xold = x;
				yold = y;
			}
		}
		else
			DoCalc(pts[0], xcom, ycom, xold, yold, xcg, ycg, area);
	}

	return(area);
}
/*
static bool isPolygonMatch(ObjHandle &poly, std::vector<DirLineId> &polyLines, int start, int end)
{
	int err;
	ObjHandle epl = poly;
	int total = 0,
		nFound = 0;
	while ((err = POLY_EDGE.getNext(epl)) == 0)
	{
		ObjHandle eo = epl;
		err = EDGE_POLY.getNext(eo);
		TigerDB::Chain* edge = (TigerDB::Chain*)eo.Lock();
		long tlid = edge->GetTLID();
		eo.Unlock();

		for (int i = end; --i >= start; )
		{
			DirLineId& lineId = polyLines[i];
			if (tlid == lineId.tlid)
				nFound++;
		}
		total++;
	}

	return (nFound > total / 2);
//	return false;
}
*/