//
//	TgrTrans.cpp - is a program that converts Tiger/Line files to a GeoDB.  The Tiger/Line files ended in 2006
//	and the Census Bureau started using ESRI shapefiles as the data exchange format.
//  Copyright(C) 2024 Michael E. Cressey
//
//	This program is free software : you can redistribute it and /or modify it under the terms of the
//	GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or
//	any later version.
//
//	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
//	implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License along with this program.
//  If not, see https://www.gnu.org/licenses/
//
/*
* The 1st program argument is the Tiger/Line file name
* The 2nd argument is the translation flag:
* 	'C' | 'c' - creates a GeoDB.  It is followed by 4 floating point numbers indicating the MBR of the database
*		'I' | 'i' - creates Landmark (GT) polygons.  Uses the 7, 8, I and P record type files
*		'H' | 'h' - processes the history file.
*		'p' | 'P' : reads the .F51 file and creates a .csv file that needs to be loaded into a SQL Server
*		'L' | 'l' : creates Tiger chains.  Processes the .RT1 and .RT2 files.  The 3rd argument is the TigerDB name
*		'B' | 'b' : creates a file with Census block boundaries (*b.tab) that is loaded into a SQL Server
*		'A' | 'a' : processes the RT6 file for addresses
*		'N' | 'n' : processes extra Tiger/Line names from the RT4 files and creates a *n.tab file that is loaded into a SQL Server
*		'G' | 'g' : processes the GNIS names file in argument 2.  It creates a POINT object and sets the userCode to the Feature Class.  Set the userId too
*		'T' | 't' : builds topology in a GeoDB given a range of data
*		'Z' | 'z' : process zipcodes.
*/
#include <afx.h>

#include <stdlib.h>
#include <search.h>
#include <limits.h>
#include <stdio.h>
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
#include "doaddr.h"
#include "donames.h"
#include "dogtpoly.h"
#include "tgrnames.h"
#include "namelook.h"
#include "tgrtypes.h"
#include "tigerdb.hpp"
#include "trendlin.h"
#include "doline.h"

#include <map>
#include <array>
#include <vector>
#include <algorithm>
#include <assert.h>
#include "HASHTABL.HPP"
#include "TopoTools.h"
#include "TString.h"
#include "gnis.h"

using namespace NodeEdgePoly;

#define TO_TIGERDB
#define DO_REAT
//#define SET_POLY_NAMES

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
static double CalcArea(/*TigerDB::Chain* line*/XY_t pts[], int nPts,
	const double& xcom, const double& ycom, double* xcg, double* ycg, int dir);
static int FindIdInList(long edgeUseId, GeoDB::dir_t dir, std::vector<GeoDB::DirLineId>& lineIds);
static bool chainEqual(ObjHandle& oh, unsigned nPts, XY_t pts[], TigerDB::Classification cCode);

static XY_t points[15000];

// Tiger extension types
const int RT1_EXT = 0;
const int RT2_EXT = 1;
const int RT3_EXT = 2;
const int RT4_EXT = 3;
const int RT5_EXT = 4;
const int RT6_EXT = 5;
const int RT7_EXT = 6;
const int RT8_EXT = 7;
const int RT9_EXT = 8;
const int RTA_EXT = 9;
const int RTC_EXT = 10;
const int RTH_EXT = 11;
const int RTI_EXT = 12;
const int RTP_EXT = 13;
const int RTR_EXT = 14;
const int RTS_EXT = 15;
const int RTZ_EXT = 16;

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

// This method is used to emulate the string split() method found in other languages such as Java, Python, etc.
void splitString(std::string str, char splitter, std::vector<std::string> &result) {
	std::string current = "";
	for (int i = 0; i < str.size(); i++) {
		if (str[i] == splitter) {
			if (current != "") {
				result.push_back(current);
				current = "";
			}
			continue;
		}
		current += str[i];
	}
	if (current.size() != 0)
		result.push_back(current);
}

int main( int argc, char *argv[] )
{
  char buffer[512];
  long nLines = 0,
		   nLinesSkipped = 0;
  CString rt1Name,
  				baseName,
  			  fName;
  int length;
  XY_t sNode,
		   eNode;
  FILE *csvLines = 0,
  	   *tabBlocks = 0,
  	   *tabBlocks2 = 0,
  	   *tabNames = 0,
  	   *csvAddr = 0,
			 *csvPoly = 0,
  	   *inFile1 = 0,
  	   *inFile2 = 0,
			 *equivFile = 0;
  short countyFips = 0;
	int		stateFips = 0;
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
			 doTigerDB = true,
			 doTopo = false,
			 doGNIS = false;
  int version = 0;
	int error;
	double tlFilter = 0.0001;
	bool doTrendLine = false;

	if( argc <= 2 )
	{
		goto USAGE_ERROR;
	}

	strcpy(buffer, argv[1]);
	if( argc > 2 && ( argv[2][0] == '/' || argv[2][0] == '-' ))
	{
		doHistory = doGTpoly = doLines = doNames = doBlocks = doPolys = doAddress = doCreate = doTopo = doGNIS = false;
		int i = 1;
		while( argv[2][i] != '\0' )		// Can chain options
		{
	    switch( argv[2][i] )
			{
			default :
			  printf( "* TgrTrans - invalid argument\n" );
				goto USAGE_ERROR;

			case 'A':
			case 'a':
				doAddress = true;
				break;

			case 'B':
			case 'b':
				doBlocks = true;
				break;

			case 'C':
			case 'c':
				if (argc < 8)
					goto USAGE_ERROR;
				doCreate = true;
				break;

			case 'G':
			case 'g':
				doGNIS = true;
				break;

			case 'H' :
			case 'h' :
			  doHistory = true;
				break;

			case 'I' :
			case 'i' :
			  doGTpoly = true;
				break;

			case 'L' :
			case 'l' :
			  doLines = true;
				break;

			case 'N' :
			case 'n' :
			  doNames = true;
				break;

			case 'p' :
			case 'P' :
			  doPolys = true;
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
	else
	{
		goto USAGE_ERROR;
	}

	try
	{
		CDatabase db;
		db.SetQueryTimeout(60 * 10);
		db.Open( _T("TigerBase"), FALSE, TRUE, _T("ODBC;"), FALSE );
#ifdef TO_TIGERDB
		TigerDB tDB(&db);
#endif
		CString rootName;

		TgrNames tgrNames(&db);
		NameLook nLook(&db);

	  length = ::strlen(buffer);
	  ::fputs(buffer, stdout);

	  if( buffer[length - 1] == '\n' )
			buffer[length - 1] = '\0';
    
		// Create a GeoDB
		if (doCreate)
		{
			//char input[80];
			double xmin,  // 204
				xmax,
				ymax,	//520,
				ymin;

			char name[80];
			strcpy(name, argv[3]);
			strcat(name, ".gdb");
			/* Waldo  Range - Xmin: -69.854694, Ymin: 44.202643, XMax: -68.792301, YMax: 44.754642 */
			sscanf(argv[4], "%lf", &xmin);
			printf("\nX Min = % f\n", xmin);

			sscanf(argv[5], "%lf", &ymin);
			printf("Y Min = %f\n", ymin);

			sscanf(argv[6], "%lf", &xmax);
			printf("X Max = %f\n", xmax);

			sscanf(argv[7], "%lf", &ymax);
			printf("Y Max = %f\n", ymax);

			TString tName(argv[3]);
			if ((error = tDB.dacCreate(tName, 1 << 16)) != 0 ||
				(error = tDB.Create(tName, xmin, ymin, xmax, ymax)) != 0)
			{
				printf("** Error creating DAC or GDB file: %d\n", error);
				return -1;
			}

			printf("Database %s created\n", name);

			return 0;
		}
		
		// Build topology
		if (doTopo)
		{
			std::string version;
			if (doTigerDB && (error = tDB.Open(TString(argv[3]), version, 1)) != 0)
			{
				fprintf(stderr, "* Cannot open Tiger DB: %s\n", argv[3]);
				goto CLEAN_UP;
			}

			TopoTools::TopoStats stats;
			const Range2D range = tDB.getRange();
			int err = TopoTools::buildTopology(tDB, range, &stats);
			if (err != 0)
				fprintf(stderr, "TopoTools::buildTopology() failed: %ld\n", err);
			printf("  Stats - nEdges: %ld, nNodes: %ld\n", stats.nEdgesRead, stats.nNodesCreated);
			err = tDB.Close();
			return 0;
		}

	  rt1Name = (const char *)&buffer[ 0 ];
//		rt1Name += tigerExts[RT1_EXT];
	  if( (inFile1 = ::fopen( TString(rt1Name), "r" )) == NULL)
	  {
			printf( "* TgrTrans - cannot open required Tiger record 1 file" );
	    goto ERR_RETURN;
	  }

	  int dirIdx = rt1Name.Find( _T("TGR") )/*rt1Name.ReverseFind( 'r' ) + 1*/;
	  int dotIdx = rt1Name.ReverseFind( '.' );
	  rootName = rt1Name.Left(dotIdx);
		printf( "\nRoot name: %s\n", (const char*)rootName);
//	  rootName = rt1Name.Mid( dirIdx, dotIdx - dirIdx )/*rt1Name.Left(dotIdx)*/;

	  baseName = rootName/*rt1Name.Mid( dirIdx, dotIdx - dirIdx )*/;
	  countyFips = GetCountyFips(baseName);
	  stateFips = GetStateFips(baseName);

		printf( "\nState: %d, County: %d\n", stateFips, countyFips);

		// Setup for processing names
		if( doNames )
		{
			tgrNames.m_strFilter = _T("(STATE = ?) AND (COUNTY = ?) AND (NAME = ?)");
			tgrNames.m_nParams = 3;
			tgrNames.stateFips = stateFips;
			tgrNames.countyFips = countyFips;
		}
//
//	Pre-process the .f52 file
//
	  length = rt1Name.GetLength();

		// Setup for processing lines (or chains)
			tgrNames.qName = _T("");
			tgrNames.Open( CRecordset::forwardOnly, 0, CRecordset::readOnly );
			while (!tgrNames.IsEOF())
				tgrNames.MoveNext();
	  if( doLines )
	  {
		  fName.Format(_T("%d_Equiv.tab"), stateFips);
		  equivFile = ::fopen(TString(fName), "w+");
#if defined(TO_TIGERDB)
			std::string version;
      if( doTigerDB && (error = tDB.Open( TString(argv[3]), version, 1)) != 0 )
			{
				fprintf( stderr, "* Cannot open Tiger DB: %s\n", argv[ 3 ] );
				goto CLEAN_UP;
			}
#endif
			rt1Name = rootName + tigerExts[RT2_EXT];
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
#if defined(SAVE_FOR_NOW)
			distName.nameParam = "";
			distName.Open( CRecordset::forwardOnly, 0, CRecordset::readOnly );
			while( ! distName.IsEOF() )
				distName.MoveNext();
#endif
	  }

	  if( doBlocks )
	  {
	    fName = baseName + "b.tab";
	    if ((tabBlocks = ::fopen( TString(fName), "w" )) == 0)
				printf("** Cannot create block file %s\n", (const char*)fName);
/*  Don't open this file
	    fName = baseName + "b2.csv";
	    tabBlocks2 = ::fopen( TString(fName), "w" );
*/
	  }

	  if( doAddress )
	  {
			fName = baseName + "a.csv";
			if ((csvAddr = ::fopen( TString(fName), "w" )) == 0)
				printf("** Cannot create address file %s\n", (const char*)fName);
	  }

	  if( doNames )
	  {
			fName = baseName + "n.tab";
			if ((tabNames = ::fopen( TString(fName), "w" )) == 0)
				printf("** Cannot create file %s\n", (const char*)fName);
	  }

	  if( doPolys )
	  {
			fName = baseName + "p.csv";
			csvPoly = ::fopen( TString(fName), "w" );
	  }
//
//	Process the Tiger Record 1 file
//
	  if( doLines || doBlocks || doAddress || doNames || doPolys )
	  {
			Range2D dataRange;
			TigerRec1 rec1;
			while( rec1.GetNextRec( inFile1 ) > 0 )
			{
			  int count = 0;
			  unsigned nPoints = 2;

			  version = rec1.version;

			  nLines++;
				if( doNames && 
						(rec1.fn.fedirp[0] != '\0' || rec1.fn.fename[0] != '\0' ||
						  rec1.fn.fetype[0] != '\0' || rec1.fn.fedirs[0] != '\0' ))
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
							DoNames( tabNames, stateFips, countyFips, tgrNames.m_feat, rec1.tlid, RTSQ, rec1.fn );
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
					DoBlocks(tabBlocks, tabBlocks2, rec1, countyFips);
			  if( doAddress )
					DoAddress( csvAddr, rec1.tlid, 0, rec1.ar );
#if defined(DO_LATER)
			  if( doPolys ) DoPolygons( csvPoly, rec1 );
#endif
			  if( ! doLines ) continue;
//
//	Get points now so we can do MBR
//
				TigerDB::Classification cCode = MapCFCC(rec1.cfcc);

//	Process only the desired lines:
//	1) If line is a border
//	2) If line defines a census block
//	3) If line is a road, water or political boundary
				if (!((rec1.blkl != rec1.blkr || rec1.blkls != rec1.blkrs ||
					     rec1.ctbnal != rec1.ctbnar || rec1.countyl != rec1.countyr ||
					     rec1.statel != rec1.stater) ||
					(rec1.side != '\0') ||
					(rec1.cfcc[0] == 'A' || rec1.cfcc[0] == 'B' || rec1.cfcc[0] == 'F' || rec1.cfcc[0] == 'H' || rec1.cfcc[0] == 'C'
							|| rec1.cfcc[0] == 'P' || rec1.cfcc[0] == 'D')))
				{
					nLinesSkipped++;
					printf("  * Skipped %ld with FC: %s\n", rec1.tlid, rec1.cfcc);
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
//	REAT the line if it's a Road type.  REATing is changing the road type based on the road name
					if( rec1.cfcc[0] == 'A' )  // Roads
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
#if defined(DO_REAT)
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
			  if( BinarySearch(rec1.tlid, index, nIndex, &offset ))
			  {
			    TigerRec2 rec2;
			    long tlid = 0;

			    fseek(inFile2, index[ offset ].fpos, SEEK_SET);
			    nPoints += rec2.GetPoints( inFile2, &tlid, &points[ 1 ],
			      sizeof(points) / sizeof(points[0]));
			  }

			  if( (rec1.blkl != rec1.blkr || rec1.blkls != rec1.blkrs ||
							rec1.ctbnal != rec1.ctbnar || rec1.countyl != rec1.countyr ||
							rec1.statel != rec1.stater ) && cCode == TigerDB::NotClassified)
				{
					fprintf(stderr, "TgrTrans: line %ld defines Census Block but no code\n", rec1.tlid);
				}

			  rec1.GetFromGeoPoint(&points[0]);
			  rec1.GetToGeoPoint(&points[nPoints - 1]);

				if (doTrendLine)  // Thin line is specified
				{
					nPoints = TrendLine( points, nPoints, tlFilter );
				}

				Range2D mbr;  // Calculate and set the MBR of the chain
				mbr.Init();
				for (unsigned i = 0; i < nPoints; i++)
					mbr.Add(points[i]);

				dataRange.Envelope(mbr);  // Keep the entire data range
				if (doTigerDB)
 				{
//#if defined(TO_TIGERDB)
					// Check state boundary lines to see if the line already exists in the database (from another county)
					if (rec1.countyl != rec1.countyr)
					{
						ObjHandle fo;

						DbObject::Id equivId = 0;
//#if defined(SAVE_FOR_NOW)
						GeoDB::Edge::Hash dbHash;
						dbHash.id = rec1.tlid;
						if ((error = tDB.dacSearch(DB_EDGE, &dbHash, fo)) == 0)	// Sometimes the chains share a common TLID between counties
						{
							bool equiv = chainEqual(fo, nPoints, points, cCode);
							assert(equiv);

							equivId = rec1.tlid;
						}
						else  // search spatially for the boundary chain
						{
							GeoDB::Search ss;
							GeoDB::searchClasses_t searchFilter;
							searchFilter.set(DB_EDGE);
							tDB.InitSearch(&ss, mbr, searchFilter);
							while ((error = tDB.getNext(&ss, &fo)) == 0)
							{
								GeoDB::Edge* edge = (GeoDB::Edge*)fo.Lock();
								GeoDB::SpatialClass sc = edge->IsA();
								long userId = edge->userId;
								fo.Unlock();
								if (sc == GeoDB::LINE)
								{
									bool equiv = chainEqual(fo, nPoints, points, cCode);
									if (equiv)
									{
										equivId = edge->userId;
										fprintf(equivFile, "%ld\t%ld\n", rec1.tlid, userId);
									}
								}

								if (equivId != 0)
								{
									break;
								}
							}
						}
//#endif
						if (equivId != 0)
						{
							printf("* %ld (%d) is equivalent to %ld\n", rec1.tlid, stateFips, equivId);
							nLinesSkipped += 1;
							continue;
						}
					}

					ObjHandle dbo;
					// Create a new object in memory and set some of it's data.  An Edge (Chain) is a variable length object, so we
					// want to create it first before we allocate disk space for it.
					if ((error = tDB.NewObject(DB_EDGE, dbo)) != 0)
					{
						fprintf( stderr,  "**dbOM.newObject failed\n" );
						goto CLEAN_UP;
					}

					TigerDB::Chain *line = (TigerDB::Chain *)dbo.Lock();

					//line->Init();
					//line->line_coor.init();

					line->setMBR(mbr);

					line->userCode = cCode;
					line->SetName(names, nNames);	// Don't store the actual names in the Chain (retrieve from the SQL database)
					line->userId = rec1.tlid;			// TLID is unique
					if ((error = line->Write()) != 0)	// We call Write() to assign an object space on the disk
					{
						fprintf(stderr, "**DbOM::Write() failed\n");
						goto CLEAN_UP;
					}
//					line->Set((unsigned)nPoints, points);
					//fprintf(mappingFile, "%ld\t%ld\n", rec1.tlid, line->dbAddress() );
					fflush(stdout);

					if ((error = tDB.dacInsert(line, line)) != 0)  // Insert the hashed object in the DAC
					{
						fprintf(stderr, "**tDB.dacInsert() failed\n");
						goto CLEAN_UP;
					}
					dbo.Unlock();
			
					if ((error = tDB.addToSpatialTree( dbo )) != 0)
					{
						fprintf(stderr, "**tDB.Add() failed\n");
						goto CLEAN_UP;
					}
					error = line->Set((unsigned)nPoints, points);

					tDB.TrBegin();		// Do a transaction which writes all the records to the database for the chain
					if ((error = tDB.TrEnd()) != 0)
					{
						fprintf(stderr, "**tDB.TrEnd() failed\n");
						goto CLEAN_UP;
					}
//#endif
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

					WriteLine(csvLines, 0, rec1.tlid, points[0], points[nPoints - 1], range, params, nPoints, &points[1]);
				}
	    }

	    if( index != 0 )
	      delete [] index;

	    printf("\nNumber of lines proccessed: %ld, skipped: %ld\n", nLines, nLinesSkipped );
			fprintf(stderr, "Data Range: %f %f %f %f\n", dataRange.x.min, dataRange.y.min, dataRange.x.max, dataRange.y.max);

	    if( csvLines ) fclose( csvLines );
	    if( tabBlocks ) fclose( tabBlocks );
	    if( tabBlocks2 ) fclose( tabBlocks2 );
	    if( csvPoly ) fclose( csvPoly );
	    if( inFile2 ) fclose( inFile2 );
			if (equivFile) fclose(equivFile);
	  }

	  fclose( inFile1 );

//
//	Process extra names
//
	  if( doNames )
	  {
	    length = rt1Name.GetLength();
			rt1Name = rootName + tigerExts[RT4_EXT];
	    if( (inFile1 = ::fopen( TString(rt1Name), "r" )) != 0)
	    {
			  TigerRec4 rec4;
				TigerRec5 rec5;

			  while( rec4.GetNextRec( inFile1 ) > 0 )
			  {
					for( int i = 0; i < rec4.nFeatures; i++ )
					{
//					    fseek( inFile2, ( rec4.feats[ i ] - 1) * 54, SEEK_SET );
//					    rec5.GetNextRec( inFile2 );
						DoNames( tabNames, stateFips, countyFips, rec4.feats[ i ],
							rec4.tlid, RTSQ + i + 1, rec5.fn );
//					    DoNames( tabNames, rec4.tlid, RTSQ + i + 1, rec5.fn );
					}
			  }
//			    fclose( inFile2 );

			  fclose( inFile1 );
	    }
	    fclose( tabNames );
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
			printf("PROCESSING LANDMARK POLYGONS\n");
			std::string version;
			error = tDB.Open(TString(argv[3]), version, 1);

//
//	Load record "8" - Landmark links into memory
//
			rt1Name = rootName + tigerExts[RT8_EXT];
			if ((inFile1 = fopen(rt1Name, "r")) == 0)
			{
				printf("* Cannot open %s\n", (const char*)rt1Name);
				goto ERR_RETURN;
			}

			struct LMLink {
				char cenid[6];
				long polyid;
			};
			std::multimap<int, LMLink> llMap;
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
				printf("* Cannot open %s\n", (const char*)rt1Name);
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
				printf("* Cannot open %s\n", (const char*)rt1Name);
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
				//printf("PolyID: %d, Lat: %d, Long: %d\n", recP.polyid, recP.polyLat, recP.polyLong);
			}
			::fclose(inFile1);
//
//	Process record "7" - Landmark links
//
			rt1Name = rootName + tigerExts[RT7_EXT];
			if ((inFile1 = fopen(rt1Name, "r")) == 0)
			{
				printf("* Cannot open %s\n", (const char*)rt1Name);
				goto ERR_RETURN;
			}

			int nPolysFound = 0;
			// Process Landmark polygons
			int nLandmarkPolys = 0;
			TigerRec7 rec7;
			while (rec7.GetNextRec(inFile1) > 0)
			{
				std::vector<GeoDB::DirLineId> dirLineIds;
				if (rec7.lalong != -1 || rec7.lalat != -1)  // Point Landmarks
					continue;
				if (rec7.cfcc[0] != 'D')  // Temporary - only process landmark
					continue;
				int cfccNum = atoi(&rec7.cfcc[1]);
				if (cfccNum != 85)				// Temp - only process parks
					continue;
				std::multimap<int, LMLink>::iterator it = llMap.find(rec7.land);
				if (it == llMap.end())
					continue;
				auto itr1 = llMap.lower_bound(rec7.land);
				auto itr2 = llMap.upper_bound(rec7.land);
				int linkCount = 0;
				count = 0;
				printf("  GTPolys: ");
				while (itr1 != itr2)
				{
					linkCount++;
					// Search for the GT poly records
					LMLink ll = itr1->second;
					printf("%d, ", ll.polyid);
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
					itr1++;
				}
				printf(" %d, Total edges: %d\n", linkCount, count);
#ifdef THIS_DIDNT_WORK
				// Check to see if duplicate edge in different direction
				for (int i = 0; i < dirLineIds.size(); i++)
				{
					for (int j = i + 1; j < dirLineIds.size(); j++)
					{
						if (dirLineIds[i].id == dirLineIds[j].id)
						{

							assert(dirLineIds[i].dir != dirLineIds[j].dir);
							printf("** Duplicate Edge %d in LandMark poly: %d\n", dirLineIds[i].id, rec7.land);
							dirLineIds[i] = dirLineIds.back();
							dirLineIds.pop_back();
							dirLineIds[i] = dirLineIds.back();
							dirLineIds.pop_back();
							if (i > 0)
								--i;
							if (j > 1)
							--j;
							break;
						}
					}
				}
#endif
				/*printf("Count = %ld\n", count);
				if (count == 1)
					continue;*/
				printf("Processing: %d, name: %s (%s)\n", rec7.land, rec7.laname, rec7.cfcc);
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
				GeoDB::Edge::Hash dbHash;
				while (dirLineIds.size() > 0)
				{
					ObjHandle nh;
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

					long lastEdgeUserId = dl.id;

					dbHash.id = dl.id;
					err = tDB.dacSearch(DB_EDGE, &dbHash, oh);
					assert(err == 0);
					GeoDB::Edge* line = (GeoDB::Edge*)oh.Lock();
					unsigned char lastEdgeUserCode = line->userCode;
					Range2D mbr;
					mbr = line->getMBR();
					line->getNodes(&sPt, &ePt);
					line->Get(points);
					int nPts = line->getNumPts();
					oh.Unlock();
					XY_t lastPt,
						startPt;
					signed char zLevel;
					if (dl.dir > 0)
					{
						lastPt = ePt;
						err = line->getNode(nh, dl.dir, &zLevel);
					}
					else
					{
						lastPt = sPt;
						sPt = ePt;
						err = line->getNode(nh, 0, &zLevel);
					}
					assert(err == 0);

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
					while (lastPt != sPt)
					{
						//int i;
						ObjHandle eh;
						GeoDB::dir_t outDir,
							saveDir;
						double angle;
						bool found = false;
						signed char zLevel;
						int saveListPos = -1;
						int pos = -1,
							savePos = -1;
						ObjHandle nextEdge;
						ObjHandle nodeLink = nh;
						while ((err = GeoDB::Node::getNextDirectedEdge(nodeLink, eh, &outDir, &angle, &zLevel)) == 0)
						{
							pos += 1;
							GeoDB::Edge* line = (GeoDB::Edge*)eh.Lock();
							long id = line->userId;

							if (id == lastEdgeUserId)
								found = true;
							else
							{
								int foundPos = FindIdInList(id, (outDir > 0) ? -1 : 1, dirLineIds);

								if (foundPos >= 0)
								{
									if (saveListPos < 0 || line->userCode == lastEdgeUserCode)
									{
										saveListPos = foundPos;
										saveDir = outDir;
										savePos = pos;
										nextEdge = eh;
									}
								}
							}
							eh.Unlock();
						}
						if (saveListPos < 0)  // Didn't find a connecting edge.  Ignore this edge and continue;
						{
							assert(found);
							found = false;
							printf("** Found an edge (%ld) that didn't connect to any other edge in list: %d\n", lastEdgeUserId, dirLineIds.size());
							goto PROCESS_POLYSEGS;  // Stop processing list (TEMP)
						}
						GeoDB::Edge *edge = (GeoDB::Edge*)nextEdge.Lock();
						lastEdgeUserId = edge->userId;
						XY_t startPt,
							endPt;
						edge->getNodes(&startPt, &endPt);
						long userId = edge->userId;
						edge->Get(points);
						nPts = edge->getNumPts();

						if (saveDir > 0)
							assert(endPt == lastPt);
						else
							assert(startPt == lastPt);
						dl = dirLineIds[saveListPos];
						if (dl.dir < 0)
							dl.dir = 0;
						assert(saveDir != dl.dir);
						if (dl.dir > 0)
							lastPt = endPt;
						else
							lastPt = startPt;
						//signed char zLevel;
						err = edge->getNode(nh, dl.dir > 0 ? dl.dir : 0, &zLevel);
						assert(err == 0);
						mbr.Envelope(edge->getMBR());
						//GeoPoint::Envelope( &min, &max, currLine->min, currLine->max );

						if (dl.dir <= 0)
						{
							rval += CalcArea(points, nPts, xcom, ycom, &xcg, &ycg, -1);
						}
						else
						{
							rval += CalcArea(points, nPts, xcom, ycom, &xcg, &ycg, 1);
						}
						nextEdge.Unlock();

						orderedLines.push_back(dl);
						dirLineIds[saveListPos] = dirLineIds.back();// [dirLineIds.size() - 1] ;
						dirLineIds.pop_back();
						nSegs += 1;
						found = true;
#ifdef OLD_WAY
						for (i = 0; i < dirLineIds.size(); i++)
						{
							dl = dirLineIds[i];
							dbHash.tlid = dl.id;
							err = tDB.dacSearch(DB_EDGE, &dbHash, oh);
							assert(err == 0);
							GeoDB::Edge* line = (GeoDB::Edge*)oh.Lock();
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
#endif
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
						assert(nSegs > 0);
						break;
					}
					PolySegment ps;
/*					if (rval < 0.0)
						rval = -rval;*/
					ps.area = -rval;
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
				PROCESS_POLYSEGS:
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
				bool buildPoly = false;
				TigerDB::Polygon* poly = 0;
				for (int j = 0; j < polySegs.size(); j++)
				{
					PolySegment ps = polySegs[j];
					/**/
//					if (ps.area <= 0)  // Islands don't work
//						break;

//					if (j == 0)
					{
						// Search and see if we find the matching polygon already constructed
#ifdef SET_POLY_NAMES
						ObjHandle so;
						GeoDB::Search ss;

						XY_t centroid;
						centroid.x = ps.xcg;
						centroid.y = ps.ycg;

						//Range2D range;
						//range.Envelope(centroid);

						tDB.Init(/*range*/ps.mbr, &ss);
						while (tDB.GetNext(&ss, &so) == 0)
						{
							GeoDB::SpatialObj* spatialObj = (GeoDB::SpatialObj*)so.Lock();
							GeoDB::SpatialClass sc = spatialObj->IsA();
							if (sc == GeoDB::AREA)
							{
								nPolysFound++;
								//int nPts = TigerDB::Polygon::GetPts(so, points);
								TigerDB::Polygon* poly = (TigerDB::Polygon*)spatialObj;
								//printf("Poly: %ld with classification: %d", spatialObj->dbAddress(), poly->GetCode());
								int match = poly->matchPoly(so, orderedLines, ps.start, ps.end);
								if (match > 0)
								//if (isPolygonMatch(so, orderedLines, ps.start, ps.end))
								{
									std::string name(rec7.laname);
									poly->SetName(name);
									err = poly->write();
									assert(err == 0);
									if ((err = tDB.TrBegin()) == 0)
										err = tDB.TrEnd();
									assert(err == 0);
									printf("* Poly: %ld matches %s (%d)\n", poly->dbAddress(), rec7.laname, match);
								}
								//printf("\n");
							}
							so.Unlock();
						}

						break; // Don't create the Polygon now
#endif
						if ((err = tDB.NewDbObject(DB_POLY, po)) != 0)
						{
							fprintf(stdout, "**dbOM.NewDbObject failed\n");
						}
						nLandmarkPolys++;
						poly = (TigerDB::Polygon*)po.Lock();
						poly->userCode = MapCFCC(rec7.cfcc);
						poly->setArea(ps.area);
						poly->setMBR(ps.mbr);
						std::string name(rec7.laname);
						if (ps.area >= 0.0)
							poly->SetName(name);
						else
							poly->SetName("Island");
						XY_t centroid;
						centroid.x = ps.xcg;
						centroid.y = ps.ycg;
						poly->setCentroid(centroid);
						//poly->write();
						//po.Unlock();
						err = tDB.addToSpatialTree(po);
						assert(err == 0);
					}
					for (int i = ps.start; i < ps.end; i++)  // counter-clock wise
					//for (int i = ps.end; --i >= ps.start; )
					{
						GeoDB::DirLineId& lineId = orderedLines[i];
						dbHash.id = lineId.id;
						ObjHandle eh;
						err = tDB.dacSearch(DB_EDGE, &dbHash, eh);
						assert(err == 0);
						if (lineId.dir > 0)
							lineId.dir = 0;
						else
							lineId.dir = 1;
						/*if (lineId.dir < 0)
							lineId.dir = 0;*/
						err = poly->addEdge(eh, lineId.dir);
						assert(err == 0);
					}
					fprintf(stdout, " Polygon %ld (%ld) created\n", poly->dbAddress(), poly->userCode);
					po.Unlock();
					buildPoly = true;
					//break;  // Islands don't work now
				}
				if (buildPoly)
				{
					//po.Unlock();

					err = tDB.TrBegin();
					err = tDB.TrEnd();
					assert(err == 0);
				}
				/**/

				//DoLMarea(csvFile, rec7);
			}
			printf("Number of Landmark polygons: %d\n", nLandmarkPolys);
			fflush(stdout);
#ifdef SET_POLY_NAMES
			printf("Number of Hydro polygons found: %d\n", nPolysFound);
#endif
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

		if (doGNIS)
		{
			FILE *gnisFile = 0,
				   *nameFile = 0;
			bool doAll = false;

			if (argc < 5)
			{
				printf("Invalid number of arguments %d\n", argc);
				goto USAGE_ERROR;
			}

			int argLen = ::strlen(argv[4]);
			if (argLen % 3 != 0)
			{
				printf("County FIPS are specified in multiples of 3 characters: %d\n", argLen);
				goto ERR_RETURN;
			}

			std::vector<std::string> countyList;
			if (argLen == 3 && (::strcmp(argv[4], "ALL") == 0 || ::strcmp(argv[4], "all") == 0))
				doAll = true;
			else
			{
				int nCountyFips = argLen / 3;
				int startPos = 0;
				for (int i = 0; i < nCountyFips; i++)
				{
					char saveChar = argv[4][startPos + 3];
					argv[4][startPos + 3] = '\0';
					std::string fips(&argv[4][startPos]);
					countyList.push_back(fips);
					argv[4][startPos + 3] = saveChar;
					startPos += 3;
				}
			}

			std::string version;
			error = tDB.Open(TString(argv[3]), version, 1);
			if (error != 0)
			{
				printf("Cannot open GDB %s\n", argv[3]);
				goto ERR_RETURN;
			}
			//
			//	Open GNIS point file
			//
			if ((gnisFile = ::fopen(argv[1], "r")) == 0)
			{
				tDB.Close();
				printf("Cannot open %s\n", argv[1]);
				goto ERR_RETURN;
			}

			char fName[128];
			sprintf(fName, "GNISDomesticNames-%s.tab", argv[4]);
			if ((nameFile = ::fopen(fName, "w")) == 0)
			{
				tDB.Close();
				::fclose(gnisFile);
				printf("Cannot open output file: %s\n", fName);
				goto ERR_RETURN;
			}

			fprintf(nameFile, "feature_id\tstate_fips\tcounty_fips\tfeature_name\n");

			char record[350];
			::fgets(record, sizeof(record) - 1, gnisFile);  // Read header record
			int count = 1;
			while (! ::feof(gnisFile) && ::fgets(record, sizeof(record) - 1, gnisFile) != NULL)
			{
				std::vector<std::string> split;
				splitString(record, '|', split);
				if (!doAll && std::find(countyList.begin(), countyList.end(), split[6]) == countyList.end())
					continue;			// Skip counties not in list
				count++;
				fprintf(nameFile, "%s\t%s\t%s\t%s\n", split[0].c_str(), split[4].c_str(), split[6].c_str(), split[1].c_str());
				int size = split.size();
				int id = atoi(split[0].c_str());
				double lat = atof(split[12].c_str());
				double lon = atof(split[13].c_str());
				TigerDB::GNISFeatures fc = MapFeatureClassToCode(split[2]);
//#if defined(TEMP)
				ObjHandle po;
				if ((error = tDB.NewDbObject(DB_POINT, po)) != 0)
				{
					fprintf(stderr, "**dbOM.NewDbObject failed: %ld\n", error);
				}

				GeoDB::Point *point = (GeoDB::Point*)po.Lock();
				point->userCode = fc;
				point->userId = id;
				XY_t pt;
				pt.y = lat;
				pt.x = lon;
				point->Set(pt);
				Range2D mbr;
				mbr.Add(pt);
				point->setMBR(mbr);
				po.Unlock();

				//po.Unlock();
				error = tDB.addToSpatialTree(po);
				assert(error == 0);

				if ((error = tDB.TrBegin()) == 0)
					error = tDB.TrEnd();
				assert(error == 0);
//#endif
			}

			printf("Read %ld lines from %s\n", count, argv[1]);
			::fclose(gnisFile);
			::fclose(nameFile);
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
	return -1;

USAGE_ERROR :
	printf("Usage: TgrTrans...\n");
	printf("  <TGR*.RT1> /a <.gdb filename> : Reads addresses from the .RT1 and .RT6 files and creates an <TGR*a.csv> address file to load into an RDMS\n");
	printf("  <TGR*.RT1> /b <.gdb filename> : Creates a <TGR*b.tab> block file to be loaded into RDMS\n");
	printf("  <TGR*.RT1> /c <.gdb filename> <xmin> <ymin> <xmax> <ymax> : Creates a new GDB file\n");
	printf("  <DomesticNames_*.txt> /g <dbname> <ALL|CountyFipsList> : Creates TigerDB::GNISFeature (point) features and a <GNISDomesticNames-*.tab> file to load into an RDMS\n");
	printf("  <TGR*.RT1> /h <.gdb filename> : Not supported currently\n");
	printf("  <TGR*.RT1> /i <.gdb filename> : Creates Tiger Landmark polygons from the <TGR*.RT7>, <TGR*.RT8>, <TGR*.RTI>, <TGR*.RTP> files\n");
	printf("  <TGR*.RT1> /l <.gdb filename> : Creates Tiger lines from the <TGR*.RT1> file.\n");
	printf("  <TGR*.RT1> /n <.gdb filename> : Creates a <TGR*n.tab> file which is loaded into an RDMS and processed.  It also connects distinct line names from an RDMS when creating edges\n");
	printf("  <TGR*.RT1> /p <.gdb filename> : Creates a <TGR*p.csv> file. Has been replaced for a new program called PolyMake\n");
	printf("  <TGR*.RT1> /t <.gdb filename> : Builds topology (Nodes and directed edges) in the entire .gdb file\n");
	printf("  <TGR*.RT1> /z <.gdb filename> : Builds zip files.  Not currently implemented\n");


ERR_RETURN :
  return( -1 );
}

// Census Feature Code Class - based on 2006 classification codes
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
  case 'A' :	// Road
		switch( cfccNum )
		{
			default :
			  fprintf( stderr,  "*MapCFCC - Unknown CFCC A: %d\n", cfccNum );
				break;

			case 0 :  // Road feature; classification unknown or not elsewhere classified
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
      
      // A1 roads
			case 11	: // Primary road with limited access or interstate highway, unseparated
			case 12 : // unseparated, in tunnel
			case 13 : // unseparated, underpassing
			case 14 : // unseparated, with rail line in center
			case 15 : // separated
			case 16 : // separated, in tunnel
			case 17 : // separated, underpassing
			case 18 : // separated, with rail line in center
      case 19 : // bridge
				code = TigerDB::ROAD_PrimaryLimitedAccess;
				break;
      
      // A2 roads
			case 21 : // Primary road without limited access, US highways, unseparated  // 
			case 22 : // unseparated, in tunnel
			case 23 : // unseparated, underpassing
			case 24 : // with rail line in center
			case 25 : // separated
			case 26 : // separated, in tunnel
			case 27 : // separated, underpassing
			case 28 : // separated, with rail line in center
      case 29 : // bridge
				code = TigerDB::ROAD_PrimaryUnlimitedAccess;
				break;
      
      // A3 roads
			case 31 : // Secondary and connecting road, state and county highways, unseparated
			case 32 :
			case 33 :
			case 34 :
			case 35 :
			case 36 :
			case 37 :
			case 38 :
      case 39 : 
				code = TigerDB::ROAD_SecondaryAndConnecting;
				break;

      // A4 roads
			case 41 : // Local, neighborhood, and rural road, city street, unseparated
			case 42 :
			case 43 :
			case 44 :
			case 45 :
			case 46 :
			case 47 :
			case 48 :
      case 49 : 
				code = TigerDB::ROAD_LocalNeighborhoodAndRural;
				break;

      // A5 roads
			case 51 : // Vehicular trail, road passable only by 4WD vehicle, unseparated
			case 52 :
			case 53 :
				code = TigerDB::ROAD_VehicularTrail;
				break;

      // A6 roads
			case 60 : // Special road feature, major category used when the minor category could not be determined
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
        
      case 66 : // Gated barrier to travel
      case 67 : // Toll booth barrier to travel
        code = TigerDB::ROAD_BarrierToTravel;
        break;

      // A7 roads
      case 70 : // Other thoroughfare, major category used when the minor category could not be determined
			case 71 : // Walkway or trail for pedestrians, usually unnamed
			case 72 : // Stairway, stepped road for pedestrians, usually unnamed
			case 73 : // Alley, road for service vehicles, usually unnamed, located at the rear of buildings and property
			case 74 : // Private road or drive for service vehicles, usually privately owned and unnamed.
				code = TigerDB::ROAD_OtherThoroughfare;
				break;
      case 75 : // Internal U.S. Census Bureau use
        code = TigerDB::ROAD_InternalUSCensusBureau;
        break;
		}
	  break;

	case 'B' :	// Railroad
		switch( cfccNum )
		{
			default :
			  fprintf( stderr,  "*MapCFCC - Unknown CFCC B: %d\n", cfccNum );
				break;
    case 0 :  // Railroad feature; classification unknown or not elsewhere classified
		case 1 :
		case 2 :
		case 3 :
			code = TigerDB::RR_MajorCategoryUnknown;
			break;

		case 11 : // Railroad main track, not in tunnel or underpassing
		case 12 :
		case 13 :
    case 14 : // Abandoned/inactive rail line with tracks present
    case 15 : // Abandoned rail line with grade, but no tracks
    case 16 : // Abandoned rail line with track and grade information unknown
    case 19 : // Railroad main track, bridge
			code = TigerDB::RR_MainLine;
			break;

		case 21 : // Railroad spur track, not in tunnel or underpassing
		case 22 :
		case 23 :
    case 29 :
			code = TigerDB::RR_Spur;
			break;

		case 31 : // Railroad yard track, not in tunnel or underpassing
		case 32 :
		case 33 :
    case 39 : // Railroad yard track, bridge
			code = TigerDB::RR_Yard;
			break;

		case 40 : // Railroad ferry crossing, the representation of a route over water used by ships carrying train cars to connecting railroads on opposite shores
			code = TigerDB::RR_FerryCrossing;
			break;

		case 50 : // Other rail line; major category used alone when the minor category could not be determined
		case 51 :
		case 52 :
			code = TigerDB::RR_OtherThoroughfare;
			break;
		}
	  break;
				  
	case 'C' :	// Miscellaneous Ground Transportation
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

			case 30 : // Other ground transportation that is not a pipeline or a power transmission
				code = TigerDB::RR_FerryCrossing;
				break;

			case 31 : // Aerial tramway, monorail, or ski lift
				code = TigerDB::MGT_AerialTramway;
				break;
      case 32 : // Pier/dock a platform built out from the shore into the water and supported by piles
				code = TigerDB::MGT_PierDock;
				break;
		}
	  break;

	case 'D' :	// Landmark
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
			case 21 : // Apartment building or complex
			case 22 : // Rooming or boarding house
			case 26 : // Housing facility for workers
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
			case 27 : // Hotel, motel, resort, spa, hostel, YMCA, or YWCA
				code = TigerDB::LM_HotelOrMotel;
				break;
			case 28 :
				code = TigerDB::LM_Campground;
				break;
			case 29 :
				code = TigerDB::LM_ShelterOrMission;
				break;

			case 30 : // Custodial facility; major category used alone when the minor category could not be determined
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
      case 45 : // Museum including visitor center, cultural center, or tourist attraction
				code = TigerDB::LM_Museum;
				break;
      case 46 : // Community Center
				code = TigerDB::LM_CommunityCenter;
				break;
      case 47 : // Library
				code = TigerDB::LM_Library;
				break;

			// Transportation terminal
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
      case 56 : // Airport Intermodel Transportation Hub/Terminal site that allows switching of differing modes of transportation
				code = TigerDB::LM_AirportIntermodelTransportationHub;
				break;
      case 57 : // Airport-Statistical Representation used as part of urban area delineation
				code = TigerDB::LM_AirportStatisticalRepresentation;
				break;
      case 58 : // Park and ride facility/parking lot
				code = TigerDB::LM_ParkAndRide;
				break;

      // Employment Center
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
      case 67 : // Convention center
				code = TigerDB::LM_ConventionCenter;
				break;

      // Towers, Monuments, and Other Vertical Structures
			case 70 :
				code = TigerDB::LM_Tower;
				break;
			case 71 :
				code = TigerDB::LM_LookoutTower;
				break;
      case 72 : // Transmission tower including cell, radio, and TV
				code = TigerDB::LM_TransmissionTower;
				break;
      case 73 : // Water tower
				code = TigerDB::LM_WaterTower;
				break;
      case 74 : // Lighthouse beacon
				code = TigerDB::LM_LighthouseBeacon;
				break;
			case 75 : // Tank/tank farm with a number of liquid
				code = TigerDB::LM_Tank;
				break;
      case 76 : // Windmill farm
				code = TigerDB::LM_WindmillFarm;
				break;
      case 77 : // Solar farm
				code = TigerDB::LM_SolarFarm;
				break;
      case 78 : // Monument or memorial
				code = TigerDB::LM_MonumentMemorial;
				break;
      case 79 : // Survey or boundary memorial
				code = TigerDB::LM_SurveyBoundaryMemorial;
				break;

      // Open Space This category contains areas of open space with no inhabitants
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
				code = TigerDB::LM_NationalParkService;
				break;
			case 84 :
				code = TigerDB::LM_NationalForestOrOther;
				break;
			case 85 :
				code = TigerDB::LM_StateOrLocalPark_Forest;
				break;
      case 86 : // Zoo
				code = TigerDB::LM_Zoo;
				break;
      case 87 : // Vineyard, winery, orchard or other agricultural or horticultural establishment
				code = TigerDB::LM_VineyardWineryOrchard;
				break;
      case 88 : // Landfill, incinerator, dump, spoil
				code = TigerDB::LM_LandfillDump;
				break;

      // Special Purpose Landmark (unclassified)
			case 90 :
				code = TigerDB::LM_SpecialPurpose;
				break;
			case 91 :
				code = TigerDB::LM_InternalUSCensusBureau;
				break;
			case 92 :
				code = TigerDB::LM_Urbanizacion;
				break;
			case 93:
			case 94:
			case 95:
			case 96 : // Internal U.S. Census Bureau use
				code = TigerDB::LM_InternalUSCensusBureau;
				break;
		}
	  break;
				  
	case 'E' :	// Feature Class E, Physical Feature
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
      case 24 : // Levee, an embankment
				code = TigerDB::PF_Levee;
				break;
      case 25 : // Marsh/Swamp
				code = TigerDB::PF_MarshSwamp;
				break;
      case 26 : // Quarry, open pit mine or mine
				code = TigerDB::PF_QuarryMine;
				break;
      case 27 : // Dam
				code = TigerDB::PF_Dam;
				break;
		}
	  break;
				  
	case 'F' :	// Nonvisible Features
		switch( cfccNum )
		{
			default :
			  fprintf( stderr,  "*MapCFCC - Unknown CFCC F: %d\n", cfccNum );
				break;

			case 0 :
				code = TigerDB::NVF_BoundaryClassificationUnknown;
				break;

			case 10 : // Nonvisible jurisdictional boundary of a legal or administrative entity
			case 11 :	// Offset boundary of a legal entity
			case 12 :	// Corridor boundary of a legal entity
			case 13 :	// Nonvisible superseded 2000 legal boundary
			case 14 :
			case 15 :
			case 16 :
      case 17 : // Nonvisible State Legislative District boundary
      case 18 : // Nonvisible Congressional District boundary
      case 19 : // Nonvisible corrected 2000 legal boundary
				code = TigerDB::NVF_LegalOrAdministrativeBoundary;
				break;

			// Nonvisible Features for Database Topology
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

			//	Property Line
			case 40 :
				code = TigerDB::NVF_PropertyLine;
				break;
			case 41:
				code = TigerDB::NVF_PublicLandSurveySystem;
				break;

			case 50 :
				code = TigerDB::NVF_ZIPCodeBoundary;
				break;
			case 52:
				code = TigerDB::NVF_InternalUSCensusBureau;
				break;

			case 60 :		// Map edge (not in 2006?)
				break;

			//	Nonvisible Statistical Boundary
			case 70 :
			case 71 :
			case 72 :
			case 73 :
			case 74 :
				code = TigerDB::NVF_StatisticalBoundary;
				break;

			// Nonvisible Other Tabulation Boundary
			case 80 : // Nonvisible Other Tabulation Boundary
			case 81 :
			case 82 :
			case 83 :
			case 84 :
			case 85 :
				code = TigerDB::NVF_OtherTabulationBoundary;
				break;
      case 86 : // Internal U.S. Census Bureau use
				code = TigerDB::NVF_InternalUSCensusBureau;
				break;
      case 87 : // Oregon urban growth area boundary
				code = TigerDB::NVF_OregonUrbanAreaBoundary;
				break;
      case 88 : // Current statistical area boundary
				code = TigerDB::NVF_CurrentStatisticalAreaBoundary;
				break;
		}
	  break;
				  
	case 'H' :	// Feature Class H, Hydrography
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

			// Naturally Flowing Water Features
			case 10 : // Stream or river; major category used when the minor category could not be determined
			case 11 :
				code = TigerDB::HYDRO_PerennialStream;
				break;
			case 12 :
				code = TigerDB::HYDRO_IntermittentStream;
				break;
			case 13 :
				code = TigerDB::HYDRO_BraidedStream;
				break;

			// Man-Made Channel to Transport Water
			case 20 :  // Canal, ditch, or aqueduct
			case 21 : // Perennial canal, ditch, or aqueduct; major category used when the minor category could not be determined
				code = TigerDB::HYDRO_PerennialCanalDitchOrAqueduct;
				break;
			case 22 :
				code = TigerDB::HYDRO_IntermittentCanalDitchOrAqueduct;
				break;

			// Inland Body of Water
			case 30 :
			case 31 :
				code = TigerDB::HYDRO_PerennialLakeOrPond;
				break;
			case 32 :
				code = TigerDB::HYDRO_IntermittentLakeOrPond;
				break;

			// Man-Made Body of Water
			case 40 : // Reservoir; major category used when the minor category could not be determined
			case 41 : // Perennial reservoir
				code = TigerDB::HYDRO_PerennialReservoir;
				break;
			case 42 :
				code = TigerDB::HYDRO_IntermittentReservoir;
				break;
      case 43: // Treatment Pond
				code = TigerDB::HYDRO_TreatmentPond;
				break;

			// Seaward Body of Water
			case 50 :
			case 51 :
				code = TigerDB::HYDRO_BayEstuaryGulfOrSound;
				break;
			case 53 :
				code = TigerDB::HYDRO_SeaOrOcean;
				break;

			// Body of Water in a Man-Made Excavation
			case 60 :
				code = TigerDB::HYDRO_GravelPitOrQuarry;
				break;
        
      // Nonvisible Definition Between Water Bodies
			case 70 : // Nonvisible water area definition boundary; used to separate named water areas
				code = TigerDB::HYDRO_WaterAreaDefinitionBoundary;
				break;

			case 71 : // USGS closure line; used as a maritime shoreline
				code = TigerDB::HYDRO_USGSClosureLine;
				break;

			case 72 : // Census water center line; computed to use as a median positional boundary
				code = TigerDB::HYDRO_CensusWaterCenterLine;
				break;

			case 73 :
				code = TigerDB::HYDRO_CensusWaterBoundary12Mile;
				break;

			case 74:	// Census water boundary separating inland from coastal or Great Lakes
				code = TigerDB::HYDRO_WaterBoundaryInlandVsCoastal;
				break;

			case 75 : // Census water boundary separating coastal water from territorial sea at the 3-mile limit
				code = TigerDB::HYDRO_CensusWaterBoundary3Mile;
				break;

			case 76:  // Artificial path through double line hydrography
			case 77:
				code = TigerDB::HYDRO_ArtificialPath;
				break;

			// Special Water Feature
			case 80:  // Special water feature; major category used when the minor category could not be determined
				code = TigerDB::HYDRO_SpecialWaterFeature;
				break;
			case 81 :
				code = TigerDB::HYDRO_Glacier;
				break;
		}
	  break;

	case 'X' :	// Not Yet Classified
		break;
  }
  
  return( code );
}

static short GetStateFips( const TCHAR *string )
{
	wchar_t state[3];

  const char *pos;

  if( (pos = strstr(string, "TGR")) == 0)
    return( 0 );

	state[0] = pos[3];
	state[1] = pos[4];
	state[2] = '\0';

  return( (short)_wtoi( state ) );
}

static short GetCountyFips( const TCHAR *string )
{
  const char *pos;

  if( (pos = ::strstr( string, "TGR")) == 0 )
    return( 0 );
  
	pos += 5;

  return( (short)atoi(pos) );
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

static int FindIdInList(
	long edgeUseId,
	GeoDB::dir_t dir,
	std::vector<GeoDB::DirLineId> &lineIds
)
{
	for (int i = 0; i < lineIds.size(); i++)
	{
		const GeoDB::DirLineId& temp = lineIds[i];
		if (temp.id == edgeUseId && temp.dir == dir)
			return i;
	}
	return -1;
}

// Compares two coordinates to 6 decimal points of precision
/*
static bool Equal(XY_t& p1, XY_t& pt2)
{
	double diff = p1.x - pt2.x;
	if (diff < 0)
		diff = -diff;
	if (diff > tol)
		return false;

	diff = p1.y - pt2.y;
	if (diff < 0)
		diff = -diff;
	if (diff > tol)
		return false;

	return true;
}
*/
const double tol = 1.0e-6;

static bool chainEqual(ObjHandle& oh, unsigned nPts, XY_t pts[], TigerDB::Classification cCode)
{
	bool equal = true;
	TigerDB::Chain *chain = (TigerDB::Chain*)oh.Lock();
	XY_t sPt, ePt;
	chain->getNodes(&sPt, &ePt);
	if (chain->userCode == cCode && ((sPt.Equal(pts[0], tol) && ePt.Equal(pts[nPts - 1], tol)) ||
															     (sPt.Equal(pts[nPts - 1], tol) && ePt.Equal(pts[0], tol))))
	{
		if (nPts != chain->getNumPts())
			equal = false;
	}
	else
		equal = false;
	oh.Unlock();

	return equal;
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