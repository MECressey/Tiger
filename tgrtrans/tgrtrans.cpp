#include <afx.h>

#include <stdlib.h>
#include <search.h>
#include <limits.h>
#include <stdio.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <wchar.h>
#include "tigerrec.hpp"
#include "tigerlib.h"
#include "range.hpp"

#include "doblock.h"
//#include "dopoly.h"
#include "doaddr.h"
#include "donames.h"
//#include "dogtpoly.h"
//#include "dohist.h"
#include "tgrnames.h"
#include "namelook.h"
//#include "distname.h"
#include "tgrtypes.h"
#include "tigerdb.hpp"
#include "trendlin.h"
#include "doline.h"

#include "TString.h"

#define TO_TIGERDB
#define DO_REAT

const int RTSQ =	0;
const int TIGER_92	= 5;
const int TIGER_94	= 21;

static short GetStateFips( const TCHAR * );
static short GetCountyFips( const TCHAR * );


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

static XY_t points[10000];

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

int main( int argc, char *argv[] )
{
  char buffer[512];
  long nLines = 0;
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
  	   *inFile2 = 0;
  short countyFips;
	int		stateFips;
  i_rnum *index = 0;
  long nIndex;
  bool doLines = true,
  	   doNames = true,
		   doBlocks = true,
		   doPolys = true,
		   doAddress = true,
		   doGTpoly = true,
		   doHistory = true,
		   doZips	= false,
			 doTigerDB = true/*false*/;
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
		strcpy( buffer, argv[ 1 ] );

		if( argc > 2 && ( argv[ 2 ][ 0 ] == '/' || argv[ 2 ][ 0 ] == '-' ) )
		{
	    doHistory = doGTpoly = doLines = doNames = doBlocks = doPolys = doAddress = false;
			int i = 1;
			while( argv[ 2 ][ i ] != '\0' )
			{
	      switch( argv[ 2 ][ i ] )
			  {
			  default :
			    printf( "* TgrTrans - invalid argument\n" );
				goto USAGE_ERROR;

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
		db.SetSynchronousMode(TRUE);
#ifdef TO_TIGERDB
		TigerDB tDB(&db);
#endif
		CString rootName;

		TgrNames tgrNames( &db );
		NameLook nLook( &db );
//		DistNames distName( &db );

	  length = ::strlen( buffer );
	  ::fputs( buffer, stdout );

	  if( buffer[ length - 1 ] == '\n' )
			buffer[ length - 1 ] = '\0';
    
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
#ifdef TO_TIGERDB
      if( doTigerDB && (error = tDB.Open( TString(argv[3]), 1 )) != 0 )
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
				if( ! ((rec1.blkl != rec1.blkr || rec1.blkls != rec1.blkrs ||
							 rec1.ctbnal != rec1.ctbnar || rec1.countyl != rec1.countyr ||
							 rec1.statel != rec1.stater) ||
							(rec1.side != '\0') ||
							(rec1.cfcc[0] == 'A' || rec1.cfcc[0] == 'B' || rec1.cfcc[0] == 'F' || rec1.cfcc[0] == 'H') ))
					continue;

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

					if( ( error = tDB.NewObject( DB_TIGER_LINE, dbo/*, id*/ ) ) != 0 )
					{
						fprintf( stderr,  "**dbOM.newObject failed\n" );
						goto CLEAN_UP;
					}

					TigerDB::Chain *line = (TigerDB::Chain *)dbo.Lock();

					line->Init();

			 		line->Set( (unsigned)nPoints, points );
	//			 	line->Set( id );
			 		line->SetCode( cCode );
					line->SetName( names, nNames );
					line->write();
	#ifdef _DEBUG
					fprintf( stdout, "TLID: %ld (%ld)\n", rec1.tlid, line->dbAddress() );
	#endif
					fflush( stdout );
					dbo.Unlock();
			
					tDB.Add( dbo );
					tDB.TrBegin();
					tDB.TrEnd();
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

	    printf( "\nNumber of lines: %ld%\n", nLines );

	    if( csvLines ) fclose( csvLines );
	    if( csvBlocks ) fclose( csvBlocks );
	    if( csvBlocks2 ) fclose( csvBlocks2 );
	    if( csvPoly ) fclose( csvPoly );
	    if( inFile2 ) fclose( inFile2 );
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
#ifdef DO_LATER
//
//	Process History file - Tiger 94
//
	  if( doHistory )
	  {
	  	FILE *csvFile = 0;

			rt1Name = rootName + tigerExts[RTH_EXT];
//	    rt1Name.SetAt( length - 1, 'H' );
	    if( ( inFile1 = fopen( rt1Name, "r" ) ) != 0 )
	    {
			  TigerRecH rec;

			  fName = baseName + "H.csv";
			  if( ( csvFile = fopen( fName, "w" ) ) == 0 )
			  {
			    printf( "* Cannot open %s\n", (const char *)fName );
			    fclose( inFile1 );
			    goto ERR_RETURN;
			  }

			  while( rec.GetNextRec( inFile1 ) > 0 )
			  {
			    DoHistory( csvFile, rec );
			  }
			  fclose( inFile1 );
	    }
			if( csvFile )
			  fclose( csvFile );
	  }
//
//	Process record "I" - GT/Poly-Chain link
//
	  if( doGTpoly )
	  {
			FILE *csvFile = 0;
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
