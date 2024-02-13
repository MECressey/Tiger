#include <afx.h>
#include <afxdb.h>

#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

//#include "bldpoly.h"
#include "bldpoly2.h"
#include "GEOTOOLS.HPP"
//#include "polyline.h"
#include "geopoint.hpp"
#include "RANGE.HPP"
//#include "csvpoly.h"

#include "TString.h"

using namespace NodeEdgePoly;

//#define NEW_STUFF
//#define TEST_HYDRO

static CArray<GeoDB::DirLineId, GeoDB::DirLineId &> polyIds;

inline void DoCalc(
  const XY_t &pt,
  const double &xcom, const double &ycom,
  double &xold, double &yold,
  double *xcg, double *ycg, double &area
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

static double CalcArea( GeoLine *line/*GEOPOINT pts[], int nPts*/,
	const double &xcom, const double &ycom, double *xcg, double *ycg, int dir )
{
  double area = 0.0;

  if( dir > 0 )
  {
		double xold = (double)line->sNode->pt/*pts[ 0 ]*/.x,
			   yold = (double)line->sNode->pt/*pts[ 0 ]*/.y;

		if( line->nPts > 2 )
		{
		  for( int i = 1; i < line->nPts; i++ )
		  {
				double x = (double)line->pts[ i ].x,
				       y = (double)line->pts[ i ].y,
				       aretri = (xcom - x) * (yold - ycom) + (xold - xcom) * (y - ycom);

		    *xcg += aretri * (x + xold);
		    *ycg += aretri * (y + yold);
		    area += aretri;
		    xold = x;
		    yold = y;
		  }
		}
		else
	      DoCalc( line->eNode->pt, xcom, ycom, xold, yold, xcg, ycg, area );
  }
  else
  {
//	--nPts;
		double xold = (double)line->eNode->pt/*pts[ nPts ]*/.x,
			   yold = (double)line->eNode->pt/*pts[ nPts ]*/.y;

		if( line->nPts > 2 )
		{
		  for( int i = line->nPts - 1; --i >= 0; )
		  {
				double x = (double)line->pts[ i ].x,
					   y = (double)line->pts[ i ].y,
				       aretri = (xcom - x) * (yold - ycom) + (xold - xcom) * (y - ycom);

				*xcg += aretri * (x + xold);
				*ycg += aretri * (y + yold);
				area += aretri;
				xold = x;
				yold = y;
		  }
		}
		else
	      DoCalc( line->sNode->pt, xcom, ycom, xold, yold, xcg, ycg, area );
  }

  return( area );
}

static int maxPts = 0;
static XY_t *points = 0;

static double CalcArea(GeoDB::Edge *line,
	const double& xcom, const double& ycom, double* xcg, double* ycg, int dir)
{
	double area = 0.0;
	XY_t sNode,
		eNode;
	int nPts = line->getNumPts();

	line->getNodes(&sNode, &eNode);

	if (nPts > 2)
	{
		if (nPts > maxPts)
		{
			if (points != 0)
				delete[] points;
			maxPts = nPts;
			points = new XY_t[maxPts];
		}
		line->Get(points);
	}

	if (dir > 0)
	{
		double xold = sNode.x,
					 yold = sNode.y;

		if (nPts > 2)
		{
			for (int i = 1; i < nPts; i++)
			{
				double x = points[i].x,
							 y = points[i].y,
							 aretri = (xcom - x) * (yold - ycom) + (xold - xcom) * (y - ycom);

				*xcg += aretri * (x + xold);
				*ycg += aretri * (y + yold);
				area += aretri;
				xold = x;
				yold = y;
			}
		}
		else
			DoCalc(eNode, xcom, ycom, xold, yold, xcg, ycg, area);
	}
	else
	{
		//	--nPts;
		double xold = eNode.x,
					 yold = eNode.y;

		if (nPts > 2)
		{
			for (int i = nPts - 1; --i >= 0; )
			{
				double x = points[i].x,
							 y = points[i].y,
							 aretri = (xcom - x) * (yold - ycom) + (xold - xcom) * (y - ycom);

				*xcg += aretri * (x + xold);
				*ycg += aretri * (y + yold);
				area += aretri;
				xold = x;
				yold = y;
			}
		}
		else
			DoCalc(sNode, xcom, ycom, xold, yold, xcg, ycg, area);
	}

	return(area);
}

void WritePolygon(
	FILE* file,
	const char* polyName,
	long polyId,
	const Range2D& mbr,
	double area,
	int state,
	unsigned polyType,
	const XY_t& centroid,
	int loi = 48
)
{
	char buffer[256];
	int count, ii;
	GeoPoint dPt;

	count = sprintf(buffer, "%s\t", polyName);
	/*  buffer[ count++ ] = '1';
		buffer[ count++ ] = ',';
	*/
	ii = sprintf(&buffer[count], "%ld\t", polyId);
	count += ii;

/*	if (GetCoordinateSystem() == 0)
	{
		ii = sprintf(&buffer[count], "%ld\t", (min.lat - 1) >> 1);
		count += ii;
		ii = sprintf(&buffer[count], "%ld\t", (min.lon - 1) >> 1);
		count += ii;
		ii = sprintf(&buffer[count], "%ld\t", (max.lat + 1) >> 1);
		count += ii;
		ii = sprintf(&buffer[count], "%ld\t", (max.lon + 1) >> 1);
		count += ii;
	}
	else*/
	{
		//dPt = min;

		ii = sprintf(&buffer[count], "%.4f\t", mbr.y.min);
		count += ii;

		ii = sprintf(&buffer[count], "%.4f\t", mbr.x.min);
		count += ii;

		//dPt = max;
		ii = sprintf(&buffer[count], "%.4f\t", mbr.y.max);
		count += ii;

		ii = sprintf(&buffer[count], "%.4f\t", mbr.x.max);
		count += ii;
	}

	ii = sprintf(&buffer[count], "%d\t", state);
	count += ii;
	//  buffer[ count++ ] = '0';

	ii = sprintf(&buffer[count], "%f\t", area);
	count += ii;

	ii = sprintf(&buffer[count], "%d\t", polyType);
	count += ii;

	//dPt = centroid;

	ii = sprintf(&buffer[count], "%.4f\t", centroid.y);
	count += ii;

	ii = sprintf(&buffer[count], "%.4f\t", centroid.x);
	count += ii;

	ii = sprintf(&buffer[count], "%d", loi);
	count += ii;

	buffer[count++] = '\n';
	buffer[count] = '\0';
	fputs(buffer, file);
}

const int POLY_INC = 3;

void WritePolyLine(FILE* file, long polyId, GeoDB::DirLineId polyIds[], int nIds, BOOL forward = TRUE)
{
	char buffer[256];
	int pos = sprintf(buffer, "%ld\t", polyId);
	int rtsq = -32768;

	if (forward)
	{
		for (int i = 0; i < nIds; i++)
		{
			const GeoDB::DirLineId& lineId = polyIds[i];
			int count = pos,
				ii;
			long tlid = lineId.id;
			int dir;

			if (lineId.dir > 0)
			{
				dir = 1;
			}
			else
			{
				dir = 0;
			}

			ii = sprintf(&buffer[count], "%ld\t", tlid);
			count += ii;

			ii = sprintf(&buffer[count], "%d\t", rtsq);
			count += ii;
			rtsq += POLY_INC;

			if (dir > 0)
				buffer[count++] = '1';
			else
				buffer[count++] = '0';

			buffer[count++] = '\n';
			buffer[count] = '\0';
			fputs(buffer, file);
		}
	}
	else
	{
		for (int i = nIds; --i >= 0; )
		{
			const GeoDB::DirLineId& lineId = polyIds[i];
			int count = pos,
				ii;
			long tlid = lineId.id;
			int dir;

			if (lineId.dir > 0)
			{
				dir = 0;
			}
			else
			{
				dir = 1;
			}

			ii = sprintf(&buffer[count], "%ld\t", tlid);
			count += ii;

			ii = sprintf(&buffer[count], "%d\t", rtsq);
			count += ii;
			rtsq += POLY_INC;

			if (dir > 0)
				buffer[count++] = '1';
			else
				buffer[count++] = '0';

			buffer[count++] = '\n';
			buffer[count] = '\0';
			fputs(buffer, file);
		}
	}
}

const int NA_SIZE = 20;

int BuildPoly2(
	TigerDB& tDB,
	std::map<int, int>& tlidMap,
  const char *polyName,
  unsigned polyDfcc,
  long polyId,
  CArray<GeoDB::DirLineId, GeoDB::DirLineId &> &lineIds,
  int nLines,
  LineTable &lTable,
  NodeTable &nTable,
  int stateCode,
	const char *name,
  long *newId,
  int extraIds,
  const char *islandName,
  int startDfcc,
  int isleDfcc,
  BOOL checkDir
)
{
  int nPolys = 0,
  	  nWater = 0,
	  nIslands = 0,
  	  i;
  int idCount = nLines;
  FILE *polyFile = 0,
  	   *pLineFile = 0;
  char state[ 20 ];
	GeoDB::DirLineId lid;

/*sprintf( state, "%d.csv", stateCode );*/

  try
  {
		CString fname = name/*"polys"*/;
	
		fname += "polys.tab";

		if( polyIds.GetSize() == 0 )
		  polyIds.SetSize( 1000, 500 );

/*		fname += (const char *)&state[ 0 ];*/
/*
		if( ( polyFile = fopen( TString(fname), "a+" ) ) == 0 )
		{
		  fprintf( stderr, "* BuildPoly2: cannot open file: %s\n", (const char *)TString(fname) );
		  nLines = -1;
		  goto ERR_RETURN;
		}
*/
		fname = name;
		fname += "pline.tab";
/*
		fname = "pline";
		fname += (const char *)&state[ 0 ];
*/
/*
		if( ( pLineFile = fopen( TString(fname), "a+")) == 0)
		{
		  fprintf( stderr, "* BuildPoly2: cannot open file: %s\n", (const char *)TString(fname) );
		  nLines = -1;
		  goto ERR_RETURN;
		}
*/
		while( nLines > 0 )
		{
	START_NEW_POLY :
		  BOOL allWater = TRUE;
		  int lineCount = 0;
		  long lineId = 0;
		  int side = 0;
		  XY_t sPt,
						   ePt;
		  long startId = 0;
		  int lastDFCC = 0;
		  GeoLine *currLine = 0;
		  PolyNode *node = 0;
		  BOOL backTracking = FALSE;
		  XY_t prevPt;

	//	find a line to start with
		  for( i = 0; i < idCount; i++ )
		  {
				const GeoDB::DirLineId &temp = lineIds.GetAt( i );

        if( ( lineId = temp.id/*lineIds.GetAt( i )*/ ) != 0 )
				{
				  startId = lineId;
//				  if( (startId = lineId ) < 0 )
//				    startId = -startId;

				  if( ( currLine = (GeoLine *)lTable.Find( &startId, (unsigned long)startId ) ) == 0 )
				  {
						fprintf( stderr, "* BuildPoly2: cannot find line: %ld\n", lineId );
						nPolys = -1;
						goto ERR_RETURN;
				  }

				  if( ( lastDFCC = currLine->dfcc ) == startDfcc )
				  {
						continue;			
				  }
#ifdef TEST_HYDRO
				  if( polyDfcc == OPEN_WATER )
				  {
						if( lastDFCC != 111 && lastDFCC != 112 )
						  continue;
				  }
#endif
				  lid = temp;
				  polyIds.SetAtGrow( lineCount++, lid/*lineId*/ );

          if( temp.dir/*lineId*/ > 0 )
				  {
						sPt = currLine->sNode->pt;
						ePt = currLine->eNode->pt;
						node = currLine->eNode;
						if( currLine->nPts > 2 )
						  prevPt = currLine->pts[ currLine->nPts - 2 ];
						else
						  prevPt = currLine->sNode->pt;
				  }
          else
				  {
						sPt = currLine->eNode->pt;
						ePt = currLine->sNode->pt;
						node = currLine->sNode;
						if( currLine->nPts > 2 )
						  prevPt = currLine->pts[ 1 ];
						else
						  prevPt = currLine->eNode->pt;
				  }

				  //if( currLine->cfcc[ 0 ] != 'H' )
				  //  allWater = FALSE;

					lid.id = 0;
				  lineIds.SetAt( i, lid/*0*/ );
				  break;
				}
		  }

		  if( i == idCount/*lineId == 0*/ )
		  {
				fprintf( stderr, "BuildPoly2: could not find a starting line\n" );
				if( startDfcc != 0 )
				{
				  break;
				}

//				nPolys = -1;
				goto ERR_RETURN;
		  }	  
		  --nLines;

	//
	//	Loop till it closes
	//
	 	  while( ! (ePt == sPt) )
		  {
#if defined( NEW_STUFF )
	START_OVER :
#endif
				int count = 0;
				double angle = ePt.Angle( prevPt );
				int j;
				int savePos;
				GeoLine *foundLine = 0;
	//
	//	Get all other lines sharing this end point
	//
#ifdef SAVE_FOR_NOW
		    unsigned long hashVal = ePt.Hash();

				if( ( node = (PolyNode *)nTable.Find( &ePt, hashVal ) ) == 0 )
				{
				  fprintf( stderr, "* BuildPoly2: cannot find node!\n" );
				}
#endif
				for( j = 0; j < node->count; j++ )
				{
				  long tlid;
				  int dfcc;
				  GeoLine *line = node->lines[ j ];

				  if( ( tlid = line->tlid ) == startId )
				    continue;

				  dfcc = line->dfcc;

				  for( i = 0; i < idCount; i++ )
				  {
						const GeoDB::DirLineId &temp = lineIds[ i ];

				    if( ( lineId = temp.id/*lineIds[ i ]*/ ) == 0 ||
				    			/*labs(*/ lineId /*)*/ != tlid )
				      continue;

	//	Matches the start point		  
						if( line->sNode == node /*line->sNode->pt == ePt*/ &&
							( ! checkDir || temp.dir /*lineId*/ > 0 ) )
						{
#ifdef TEST_HYDRO1
						  if( polyDfcc == OPEN_WATER && dfcc == 120 )
						  {
						    continue;
						  }
#endif
						  if( foundLine == 0 )			// save first line found
						  {
								foundLine = line;
						    savePos = i;
						  }

						  if( node->angles[ j ] > angle )
						  {
								foundLine = line;
							    savePos = i;
								goto FOUND_ONE;
						  }
			 			  count++;
						  break;
		        }

//	Matches the end point
						if( line->eNode == node/*line->eNode->pt == ePt*/ &&
								( ! checkDir || temp.dir/*lineId*/ < 0 /*|| i >= extraIds*/ ) )
				    {
#ifdef TEST_HYDRO1
						  if( polyDfcc == OPEN_WATER && dfcc == 120 )
						  {
						    continue;
						  }
#endif
						  if( foundLine == 0 )			// save first line found
						  {
								foundLine = line;
						    savePos = i;
						  }

							if( ! checkDir && temp.dir/*lineId*/ > 0 )
							{
								lid.id = lineId;
								lid.dir = -temp.dir;
								lineIds[ i ] = lid/*-lineId*/;
							}

						  if( node->angles[ j ] > angle )
						  {
								foundLine = line;
						    savePos = i;
								goto FOUND_ONE;
						  }
						  count++;
#ifdef TEST_HDYRO
						  if( lineId > 0 && i >= extraIds )
						  {
								lineIds.SetAt( i, -lineId );
						  }
#endif
						  break;
		        }
				  }
				}

FOUND_ONE :
				if( foundLine == 0 )
				{
//
//	Backtrack if we have come to dead-end
				  if( lineCount > 1 )
				  {
						const GeoDB::DirLineId &temp = polyIds[ --lineCount ];

						backTracking = TRUE;
						startId = temp.id/*polyIds[ --lineCount ]*/;
						fprintf( stderr, "* BT - Edge: %ld (%d)\n", startId, currLine->dfcc );
						lid = polyIds[ lineCount - 1 ];
						startId = lid.id/*polyIds[ lineCount - 1 ]*/;
/*					if( startId < 0 )
					  startId = -startId;*/

						if( currLine->sNode == node )
						{
						  ePt = currLine->eNode->pt;
						  node = currLine->eNode;
						  for( j = currLine->eNode->count; --j >= 0; )
						  {
								if( currLine->eNode->lines[ j ]->tlid == startId )
								{
								  currLine =  currLine->eNode->lines[ j ];
								  break;
								}
						  }
						  if( j < 0 )
						    fprintf( stderr, "* BuildPoly2 - could not find previous line\n" );
						}
						else if( currLine->eNode == node )
						{
						  ePt = currLine->sNode->pt;
						  node = currLine->sNode;
						  for( j = currLine->sNode->count; --j >= 0; )
						  {
								if( currLine->sNode->lines[ j ]->tlid == startId )
								{
								  currLine =  currLine->sNode->lines[ j ];
								  break;
								}
						  }
						  if( j < 0 )
						    fprintf( stderr, "* BuildPoly2 - could not find previous line\n" );
						}
						else
						{
						  fprintf( stderr, "* BuildPoly2 - currLine %ld does not match node\n", currLine->tlid );
						}

						if( currLine->sNode == node/*->pt == ePt*/ )
						{
						  if( currLine->nPts > 2 )
								prevPt = currLine->pts[ 1 ];
						  else
								prevPt = currLine->eNode->pt;
						}
						else if( currLine->eNode == node/*->pt == ePt*/ )
						{
						  if( currLine->nPts > 2 )
								prevPt = currLine->pts[ currLine->nPts - 2 ];
						  else
								prevPt = currLine->sNode->pt;
						}
						else
						{
						  fprintf( stderr, "* BuildPoly2 - prevline does not match endpoint\n" );
						}
						continue;
				  }

#ifdef SAVE_FOR_NOW
				  if( lastDFCC == startDfcc )
				  {
						fprintf( stderr, "BuildPoly2: Starting over: %ld\n", startId );
				    goto START_NEW_POLY;
				  }
#endif

#if defined( _DEBUG )
				  fprintf( stderr, "* BuildPoly2: Could not find a matching line\n" );
				  for( i = 0; i < lineCount; i++ )
				  {
						const GeoDB::DirLineId &temp = polyIds[ i ];

				    fprintf( stderr, "Id:%ld ", temp.id/*polyIds[ i ]*/ ); 
						if( ( i % 5 ) == 0 )
						  fprintf( stderr, "\n" );
				  }
				  fprintf( stderr, "\n" );
#endif
				  goto START_NEW_POLY;
				}

				if( backTracking )
				{
				  backTracking = FALSE;
				  fprintf( stderr, "*   End: %ld (%d)\n", foundLine->tlid, foundLine->dfcc );
				}

				//if( foundLine->cfcc[ 0 ] != 'H' )
				//  allWater = FALSE;

				if( foundLine->sNode == node )
				{
				  ePt = foundLine->eNode->pt;
				  node = foundLine->eNode;
				  if( foundLine->nPts > 2 )
						prevPt = foundLine->pts[ foundLine->nPts - 2 ];
				  else
						prevPt = foundLine->sNode->pt;
				}
				else if( foundLine->eNode == node )
				{
				  ePt = foundLine->sNode->pt;
				  node = foundLine->sNode;
				  if( foundLine->nPts > 2 )
						prevPt = foundLine->pts[ 1 ];
				  else
						prevPt = foundLine->eNode->pt;
				}
				else
				{
				  fprintf( stderr, "* BuildPoly2 - foundline %ld does not match node\n", foundLine->tlid );
				}

#ifdef NEW_STUFF
//
//	Look to see if line is already in list (in reverse direction). If so:
//	(1) put ids back into the master list (except the ones that matched)
//	(2) set currLine to the one before the line that was found (next one out of node angle)
/*				for (i = lineCount; --i >= 0; )
				{
				  long tlid;

				  if( ( tlid = polyIds[ i ].id ) < 0 )
						tlid = -tlid;

				  if( foundLine->tlid == tlid )
				  {
					fprintf( stderr, "* Edge: %ld (%d) already in list\n", tlid, foundLine->dfcc ); 
					if( i == 0 )
					  goto START_NEW_POLY;

					lid.id = 0;
					lineIds.SetAt( savePos, 0 );
					--nLines;

					j = 0;
					while( --lineCount > i )			
					{
					  while( lineIds[ j ] != 0 )
					  	j++;
					  lineIds[ j ] = polyIds[ lineCount ];
					  j++;
					  nLines++;
					}
	
					startId = polyIds[ lineCount - 1 ];
					if( startId < 0 )
					  startId = -startId;

					if( ( currLine = (GeoLine *)lTable.Find( &startId, (unsigned long)startId ) ) == 0 )
					{
					  fprintf( stderr, "* BuildPoly2: cannot find line: %ld\n", startId );
					}

					if( currLine->sNode == node )
					{
					  if( currLine->nPts > 2 )
						prevPt = currLine->pts[ 1 ];
					  else
						prevPt = currLine->eNode->pt;
					}
					else if( currLine->eNode == node )
					{
					  if( currLine->nPts > 2 )
						prevPt = currLine->pts[ foundLine->nPts - 2 ];
					  else
						prevPt = currLine->sNode->pt;
					}
					else
					{
					  fprintf( stderr, "* BuildPoly2 - currline %ld does not match node\n", startId );
					}
					lastDFCC = currLine->dfcc;
					goto START_OVER;
				  }
				}
*/
#endif
				lastDFCC = foundLine->dfcc;
				currLine = foundLine;
			
				lid = lineIds[ savePos ];
				startId = lid.id/*lineIds[ savePos ]*/;
				polyIds.SetAtGrow( lineCount++, lid/*startId*/ );
//			if( startId < 0 )
//			  startId = -startId;
				lid.id = 0;
				lineIds.SetAt( savePos, lid/*0*/ );
				--nLines;
		  }
 
		  if( allWater )
		  {
				nWater++;
		  }
	//
	//	Go through & calculate area, centroid & MBR
	//
		  double xcg = 0.0,
	   	  		 ycg = 0.0,
				 xcom = (double)sPt.x,
				 ycom = (double)sPt.y,
				 rval = 0.0;
			Range2D mbr;
		  /*XY_t min,
		  		   max;
		  min.lat = min.lon = ULONG_MAX;*/

		  for( i = 0; i < lineCount; i++ )
		  {
				long tlid;
				const GeoDB::DirLineId &temp = polyIds[ i ];

				tlid = temp.id/*polyIds[ i ]*/;

/*				if( ( tlid = lineId ) < 0 )
				  tlid = -tlid;*/

				if( ( currLine = (GeoLine *)lTable.Find( &tlid, (unsigned long)tlid ) ) == 0 )
				{
				  fprintf( stderr, "* BuildPoly2: cannot find line: %ld\n", tlid/*lineId*/ );
				  nPolys = -1;
				  goto ERR_RETURN;
				}

				mbr.Envelope(currLine->mbr);
				//GeoPoint::Envelope( &min, &max, currLine->min, currLine->max );

				if( temp.dir /*lineId*/ < 0 )
				{
				  rval += CalcArea( currLine, xcom, ycom, &xcg, &ycg, -1 );
				}
				else
				{
				  rval += CalcArea( currLine, xcom, ycom, &xcg, &ycg, 1 );
				}
		  }

		  {
				double areai = 1.0 / rval;
				if( ( xcg = ( xcg * areai + xcom ) * ( 1.0 / 3.0 ) ) <= 0.0 )
				  fprintf( stderr, "Centroid X: %lf\n", xcg );
				if( ( ycg = ( ycg * areai + ycom ) * ( 1.0 / 3.0 ) ) <= 0.0 )
				  fprintf( stderr, "Centroid Y: %lf\n", ycg );
		  }

		  XY_t centroid;

		  centroid.y = ( ycg + 0.5);
		  centroid.x = ( xcg + 0.5);
	//
	//	Positive area - right side is inside the polygon
	//	Negative area - left side is inside the polygon
	//
		  if( rval > 0.0 )
		  {
				rval *= 0.5;
				if( polyId > 0 && nPolys == 0 )
				{
#if defined( _DEBUG )
				  fprintf( stderr, "Poly: %ld, # lines: %d\n", polyId, lineCount );
#endif
/*
					WritePolyLine(pLineFile, polyId, &polyIds[0], lineCount);
				  WritePolygon( polyFile, polyName, polyId, mbr, rval, stateCode, polyDfcc, centroid );
*/
				}
				else
				{
				  if( polyId > 0 )
				  {
#if defined( _DEBUG )
				    fprintf( stderr, "Poly: %ld was created from %ld\n", *newId, polyId );
#endif
				  }
#if defined( _DEBUG )
				  fprintf( stderr, "Poly: %ld, # lines: %d\n", *newId, lineCount );
#endif
/*
					WritePolyLine( pLineFile, *newId, &polyIds[ 0 ], lineCount );
				  WritePolygon( polyFile, polyName, *newId, mbr, rval, stateCode, polyDfcc,
				  		centroid );
*/
				  --(*newId);
				}

		    nPolys++;
		  }
		  else
		  {
		    nIslands++;
				// Creating a polygon.  ISLANDS are coming out polygon - FIX LATER!!!
				ObjHandle po;
				int err;
				if ((err = tDB.NewObject(DB_POLY/*DB_TIGER_POLY*/, po/*, id*/)) != 0)
				{
					fprintf(stderr, "**BuildPoly2: dbOM.newObject failed\n");
				}

				GeoDB::Poly* poly = (GeoDB::Poly*)po.Lock();

				poly->userCode = TigerDB::HYDRO_PerennialLakeOrPond;
				poly->setArea(-rval);
				/*Range2D range;
				range.y.min = 44.4465;
				range.x.min = -69.2176;
				range.y.max = 44.4488;
				range.x.max = -69.2134;*/
				poly->setMBR(mbr);

				err = poly->Write();
				//po.Unlock();
				err = tDB.addToSpatialTree(po);

				for (int i = lineCount; --i >= 0;)
				//	for (int i = 0; i < lineCount; i++)
				{
					const GeoDB::DirLineId& lineId = polyIds[i];
					long tlid = lineId.id;
					int dir;

					ObjHandle eh;
					std::map<int, int>::iterator it = tlidMap.find(tlid);
					if (lineId.dir > 0)
					{
						dir = 1;
					}
					else
					{
						dir = 0;
					}
					if ((err = tDB.Read(it->second, eh)) != 0)
					{
						fprintf(stderr, "**BuildPoly2: failed to find Edge: %ld\n", it->second);
					}
					if ((err = poly->addEdge(eh, dir)) != 0)
					{
						fprintf(stderr, "**BuildPoly2: failed to add Edge: %ld to Poly: %ld\n", it->second, poly->dbAddress());
					}
				}
				po.Unlock();

				err = tDB.TrBegin();
				if ((err = tDB.TrEnd()) != 0)
				{
					fprintf(stderr, "**BuildPoly2: TrEnd() failed: %ld\n", err);
				}
				if( islandName )
				{
				  rval *= 0.5;
	#if defined( _DEBUG )
			 	  fprintf( stderr, "Isle: %ld, # lines: %d\n", *newId, lineCount );
	#endif
/*
				  WritePolyLine( pLineFile, *newId, &polyIds[ 0 ], lineCount, FALSE );
				  WritePolygon( polyFile, islandName, *newId, mbr, -rval, stateCode, isleDfcc, centroid );
*/
				  --(*newId);
				}
		  }
		}

		fprintf( stderr, "Islands: %d, Water: %d\n", nIslands, nWater );
  }
  catch( CDBException *e )
  {
//	THROW_LAST();
    fprintf( stderr, "BuildPoly2: DBerr: %s\n", e->m_strError );
		nPolys = -1;
  }
  catch( CMemoryException *e )
  {
		fprintf( stderr, "BuildPoly2: memory exception\n" );
		nPolys = -1;   
  }
  catch( CException *e )
  {
		fprintf( stderr, "BuildPoly2: C exception\n" );
		nPolys = -1;   
  }

ERR_RETURN :
  if( pLineFile )
    fclose( pLineFile );

  if( polyFile )
    fclose( polyFile );

  return( nPolys );
}

static int FindIdInList(
	long lineId,
	CArray<GeoDB::DirLineId, GeoDB::DirLineId&> &lineIds,
	int nLines
)
{
	for (int i = 0; i < nLines; i++)
	{
		const GeoDB::DirLineId& temp = lineIds[i];
		if (temp.id == lineId)
			return i;
	}
	return -1;
}

int BuildPoly3(
	TigerDB& tDB,
	//std::map<int, int>& tlidMap,
	const char* polyName,
	unsigned polyDfcc,
	long polyId,
	CArray<GeoDB::DirLineId, GeoDB::DirLineId&> &lineIds,
	int nLines,
	//LineTable& lTable,
	//NodeTable& nTable,
	int stateCode,
	const char* name,
	long* newId,
	int extraIds,
	const char* islandName,
	int startCode,
	int isleDfcc,
	BOOL checkDir
)
{
	int nPolys = 0,
		nWater = 0,
		nIslands = 0,
		i;
	int idCount = nLines;
	char state[20];
	GeoDB::DirLineId lid;

	try
	{
		if (polyIds.GetSize() == 0)
			polyIds.SetSize(1000, 500);

		while (nLines > 0)
		{
		START_NEW_POLY:
			DbHash dbHash;
			ObjHandle nh;
			BOOL allWater = TRUE;
			int lineCount = 0;
			long lineId = 0;
			int side = 0;
			XY_t sPt,
				ePt;
			long startId = 0;
			int lastCode = 0;
			//GeoLine* currLine = 0;
			//PolyNode* node = 0;
			BOOL backTracking = FALSE;
			XY_t prevPt;
			DbObject::Id lastId = 0;
			char lastDir = 0;
			int err;
			Range2D mbr;

			//	find a line to start with
			for (i = 0; i < idCount; i++)
			{
				const GeoDB::DirLineId& temp = lineIds.GetAt(i);

				if ((lineId = temp.id) != 0)
				{
					startId = lineId;

					ObjHandle oh;
					dbHash.tlid = startId;
					err = tDB.dacSearch(DB_EDGE, &dbHash, oh);
					if (err != 0)
					{
						fprintf(stderr, "* BuildPoly3: cannot find line: %ld\n", startId);
						nPolys = -1;
						goto ERR_RETURN;
					}
					TigerDB::Chain* currLine = (TigerDB::Chain * )oh.Lock();
					lastId = currLine->dbAddress();
					if ((lastCode = currLine->userCode) == startCode)  // ??? Doesn't make sense
					{
						oh.Unlock();
						continue;
					}
#ifdef TEST_HYDRO
					if (polyDfcc == OPEN_WATER)
					{
						if (lastDFCC != 111 && lastDFCC != 112)
							continue;
					}
#endif
					mbr.Envelope(currLine->getMBR());  // Edge could be a loop
					lid = temp;
					polyIds.SetAtGrow(lineCount++, lid);

					signed char zLevel;
					if (temp.dir > 0)
					{
						currLine->getNodes(&sPt, &ePt);
						err = currLine->getNode(nh, temp.dir, &zLevel);
					}
					else
					{
						currLine->getNodes(&ePt, &sPt);
						err = currLine->getNode(nh, 0, &zLevel);
					}
					assert(err == 0);

					//if( currLine->cfcc[ 0 ] != 'H' )
					//  allWater = FALSE;

					lid.id = 0;
					lineIds.SetAt(i, lid);
					oh.Unlock();
					break;
				}
			}

			if (i == idCount)
			{
				fprintf(stderr, "BuildPoly3: could not find a starting line\n");
				if (startCode != 0)
				{
					break;
				}

				//				nPolys = -1;
				goto ERR_RETURN;
			}
			--nLines;

			//
			//	Calculate area, centroid & MBR (assuming we will close)
			//
			double xcg = 0.0,
							ycg = 0.0,
							xcom = (double)sPt.x,
							ycom = (double)sPt.y,
							rval = 0.0;
			//Range2D mbr;
			//
			//	Loop till the area closes
			//
			while (ePt != sPt)
			{
#if defined( NEW_STUFF )
				START_OVER :
#endif
				int count = 0;
//				double angle = ePt.Angle(prevPt);
				int j;
				int saveListPos;
				//GeoLine* foundLine = 0;
				//
				//	Get all other lines sharing this end point
				//
				//GeoDB::Node * node = (GeoDB::Node*)nh.Lock();
				ObjHandle eh;
				GeoDB::dir_t outDir,
					           saveDir;
				const GeoDB::DirLineId& temp = polyIds[lineCount - 1];
				double angle;
				signed char zLevel;
				// Get the next directed edge out of the Node star
				bool found = false;
				saveListPos = -1;
				int pos = -1,
					  savePos = -1;
				ObjHandle nextEdge;
				ObjHandle nodeLink = nh;
				while ((err = GeoDB::Node::getNextDirectedEdge(nodeLink, eh, &outDir, &angle, &zLevel)) == 0)
				{ 
					pos += 1;
					GeoDB::Edge* line = (GeoDB::Edge*)eh.Lock();
					long id = line->userId;

					if (id == temp.id)
						found = true;
					else
					{
						int foundPos = FindIdInList(id, lineIds, idCount);

						if (foundPos >= 0)
						{
							char currDir = lineIds[foundPos].dir;

							if (saveListPos < 0 || (line->userCode == TigerDB::HYDRO_PerennialShoreline && currDir != saveDir))
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

				if (saveListPos < 0)
				{
					fprintf(stderr, "* BuildPoly3: cannot find next line: %ld in polygon in list\n", temp.id);
					assert(saveListPos >= 0);
				}
				assert(found && saveListPos != -1);

				//err = node->GetNextDirectedEdge(lastId, temp.dir > 0 ? temp.dir : 0, eh, &outDir);
				//nh.Unlock();
				TigerDB::Chain* edge = (TigerDB::Chain*)nextEdge.Lock();
				XY_t startPt,
						 endPt;
				edge->getNodes(&startPt, &endPt);
				long userId = edge->userId;

				if (saveDir > 0)
					assert(endPt == ePt);
				else
					assert(startPt == ePt);
/*
				saveListPos = FindIdInList(userId, lineIds, idCount);
				if (saveListPos < 0)
				{
					fprintf(stderr, "* BuildPoly3: cannot find line %d in list!\n", userId);
					assert(saveListPos >= 0);
				}
*/
				lid = lineIds[saveListPos];
				if (lid.dir < 0)
					lid.dir = 0;
				assert(saveDir != lid.dir);
				if (lid.dir > 0)
					ePt = endPt;
				else
					ePt = startPt;
				//signed char zLevel;
				err = edge->getNode(nh, lid.dir > 0 ? lid.dir : 0, &zLevel);
				assert(err == 0);
				lastId = edge->dbAddress();
				mbr.Envelope(edge->getMBR());
				//GeoPoint::Envelope( &min, &max, currLine->min, currLine->max );

				if (lid.dir <= 0)
				{
					rval += CalcArea(edge, xcom, ycom, & xcg, & ycg, -1);
				}
				else
				{
					rval += CalcArea(edge, xcom, ycom, &xcg, &ycg, 1);
				}
				nextEdge.Unlock();

				polyIds.SetAtGrow(lineCount++, lid);
				//			if( startId < 0 )
				//			  startId = -startId;
				lid.id = 0;
				lineIds.SetAt(saveListPos, lid);
				--nLines;
			}
#ifdef DO_LATER
			if (allWater)
			{
				nWater++;
			}
#endif
			//
			//	Calculate final area & centroid
			//
			double areai = 1.0 / rval;
			if ((xcg = (xcg * areai + xcom) * (1.0 / 3.0)) <= 0.0)
				fprintf(stderr, "Centroid X: %lf\n", xcg);
			if ((ycg = (ycg * areai + ycom) * (1.0 / 3.0)) <= 0.0)
				fprintf(stderr, "Centroid Y: %lf\n", ycg);

			XY_t centroid;

			centroid.y = (ycg + 0.5);
			centroid.x = (xcg + 0.5);
			//
			//	Positive area - right side is inside the polygon
			//	Negative area - left side is inside the polygon
			//
#ifdef SAVE_FOR_NOW
			if (rval > 0.0)
			{
				rval *= 0.5;
				if (polyId > 0 && nPolys == 0)
				{
#if defined( _DEBUG )
					fprintf(stderr, "Poly: %ld, # lines: %d\n", polyId, lineCount);
#endif
					/*
										WritePolyLine(pLineFile, polyId, &polyIds[0], lineCount);
										WritePolygon( polyFile, polyName, polyId, mbr, rval, stateCode, polyDfcc, centroid );
					*/
				}
				else
				{
					if (polyId > 0)
					{
#if defined( _DEBUG )
						fprintf(stderr, "Poly: %ld was created from %ld\n", *newId, polyId);
#endif
					}
#if defined( _DEBUG )
					fprintf(stderr, "Poly: %ld, # lines: %d\n", *newId, lineCount);
#endif
					/*
										WritePolyLine( pLineFile, *newId, &polyIds[ 0 ], lineCount );
										WritePolygon( polyFile, polyName, *newId, mbr, rval, stateCode, polyDfcc,
												centroid );
					*/
					--(*newId);
				}

				nPolys++;
			}
			else
#endif
			//if (rval > 0.0)
			{

				// Creating a polygon.  ISLANDS are coming out polygon - FIX LATER!!!
				ObjHandle po;
				int err;
				if ((err = tDB.NewDbObject(DB_POLY, po)) != 0)
				{
					fprintf(stderr, "**BuildPoly3: dbOM.NewDbObject failed\n");
				}

				TigerDB::Polygon* poly = (TigerDB::Polygon*)po.Lock();

				poly->userCode = TigerDB::HYDRO_PerennialLakeOrPond;
				poly->setArea(-rval);
				poly->setMBR(mbr);
				if (rval < 0.0)
				{
					nPolys++;
					poly->SetName("LAKE");
				}
				else
				{
					poly->SetName("ISLAND");
					nIslands++;
				}
				//err = poly->write();
				//po.Unlock();
				err = tDB.addToSpatialTree(po);

				//for (int i = lineCount; --i >= 0;)
				for (int i = 0; i < lineCount; i++)  // counter-clock wise
				{
					const GeoDB::DirLineId& lineId = polyIds[i];
					long tlid = lineId.id;
					int dir;

					ObjHandle eh;
					dbHash.tlid = tlid;
					err = tDB.dacSearch(DB_EDGE, &dbHash, eh);
					if (err != 0)
					{
						fprintf(stderr, "* BuildPoly3: cannot find line: %ld\n", startId);
						nPolys = -1;
						break;
					}
					//std::map<int, int>::iterator it = tlidMap.find(tlid);
					if (lineId.dir > 0)
						dir = 0/*1*/;
					else
						dir = 1/*0*/;
		
					/*if ((err = tDB.Read(it->second, eh)) != 0)
					{
						fprintf(stderr, "**BuildPoly2: failed to find Edge: %ld\n", it->second);
					}*/
					if ((err = poly->addEdge(eh, dir)) != 0)
					{
						fprintf(stderr, "**BuildPoly3: failed to add Edge: %ld to Poly: %ld\n", tlid, poly->dbAddress());
					}
				}
				po.Unlock();

				err = tDB.TrBegin();
				if ((err = tDB.TrEnd()) != 0)
				{
					fprintf(stderr, "**BuildPoly3: TrEnd() failed: %ld\n", err);
				}
				if (islandName)
				{
					rval *= 0.5;
#if defined( _DEBUG )
					fprintf(stderr, "Isle: %ld, # lines: %d\n", *newId, lineCount);
#endif
					/*
										WritePolyLine( pLineFile, *newId, &polyIds[ 0 ], lineCount, FALSE );
										WritePolygon( polyFile, islandName, *newId, mbr, -rval, stateCode, isleDfcc, centroid );
					*/
					--(*newId);
				}
			}
		}

		fprintf(stderr, "Islands: %d, Water: %d\n", nIslands, nWater);
	}
	catch (CDBException* e)
	{
		//	THROW_LAST();
		fprintf(stderr, "BuildPoly3: DBerr: %s\n", e->m_strError);
		nPolys = -1;
	}
	catch (CMemoryException* e)
	{
		fprintf(stderr, "BuildPoly3: memory exception\n");
		nPolys = -1;
	}
	catch (CException* e)
	{
		fprintf(stderr, "BuildPoly3: C exception\n");
		nPolys = -1;
	}

ERR_RETURN:

	return(nPolys);
}
