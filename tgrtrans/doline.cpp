//
//	doline.cpp - is the method that writes line data to a text file.
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
#include <string.h>

#include "doline.h"
#include "geopoint.hpp"
#include "geotools.hpp"

inline void SwapBytes( char array[] )
{
  char save = array[ 0 ];
  array[ 0 ] = array[ 6 ];
  array[ 6 ] = save;

  save = array[ 1 ];
  array[ 1 ] = array[ 7 ];
  array[ 7 ] = save;

  save = array[ 2 ];
  array[ 2 ] = array[ 4 ];
  array[ 4 ] = save;

  save = array[ 3 ];
  array[ 3 ] = array[ 5 ];
  array[ 5 ] = save;
}

void WriteLine(
  FILE *file,
	FILE *shapePtsFile,
  long tlid,
  const XY_t &frNode,
  const XY_t &toNode,
  Range2D &range,
	const TigerParams &params,
  unsigned nPts,
  XY_t *sPts,
	bool doAscii
)
{
  char buffer[256];
  unsigned i,
      count = 0;
	GeoPoint::Coord coord,
									coord2;
//	tlid
	if( doAscii )
	{
	  i = sprintf( &buffer[ count ], "%ld\t", tlid );
	}
	else
	{
		::memcpy( &buffer[ count ], &tlid, i = 4 );
	}
  count += i;

//	fips
	if( doAscii )
	{
		if( params.fips != -1 )
		{
			i = sprintf( &buffer[ count ], "%d", params.fips );
			count += i;
		}
		buffer[ count++ ] = '\t';
	}
	else
	{
		short SHORT_ZERO = 0;
		::memcpy( &buffer[ count ], &SHORT_ZERO, 2 );
		count += 2;
	}

//	Classification code
	if( doAscii )
	{
		i = sprintf( &buffer[ count ], "%d\t", params.code );
	}
	else
	{
		::memcpy( &buffer[count], &params.code, i = 1 );
	}
  count += i;

	coord = GeoPoint::DegToLat(frNode.y);
//	fr_lat
	if( doAscii )
	{
	  i = sprintf( &buffer[ count ], "%ld\t", coord );
	}
	else
	{
		::memcpy( &buffer[ count ], &coord, i = 4 );
	}
  count += i;

//	fr_lon
	coord = GeoPoint::DegToLon(frNode.x);
	if( doAscii )
	{
	  i = sprintf( &buffer[ count ], "%ld\t", coord );
	}
	else
	{
		::memcpy( &buffer[ count ], &coord, i = 4 );
	}
  count += i;

	coord = GeoPoint::DegToLat(toNode.y);
//	to_lat	
	if( doAscii )
	{
	  i = sprintf( &buffer[ count ], "%ld\t", coord );
	}
	else
	{
		::memcpy( &buffer[ count ], &coord, i = 4 );
	}
  count += i;

//	to_lon
	coord = GeoPoint::DegToLon(toNode.x);
	if( doAscii )
	{
	  i = sprintf( &buffer[ count ], "%ld\t", coord );
	}
	else
	{
		::memcpy( &buffer[ count ], &coord, i = 4 );
	}
  count += i;

	coord = GeoPoint::DegToLat(range.y.min);
	coord2 = GeoPoint::DegToLon(range.x.min);
	if( doAscii )
	{
		i = sprintf( &buffer[ count ], "%ld\t", coord );
		count += i;
		i = sprintf( &buffer[ count ], "%ld\t", coord2 );
		count += i;
	}
	else
	{
		::memcpy( &buffer[ count ], &coord, 4 );
		count += 4;
		::memcpy( &buffer[ count ], &coord2, 4 );
		count += 4;
	}

	coord = GeoPoint::DegToLat(range.y.max);
	coord2 = GeoPoint::DegToLon(range.x.max);
	if( doAscii )
	{
		i = sprintf( &buffer[ count ], "%ld\t", coord );
		count += i;
		i = sprintf( &buffer[ count ], "%ld\t", coord2 );
		count += i;
	}
	else
	{
		::memcpy( &buffer[ count ], &coord, 4 );
		count += 4;
		::memcpy( &buffer[ count ], &coord2, 4 );
		count += 4;
	}

//	border - byte
	if( doAscii )
	{
		if( params.border != -1 )
		{
			i = sprintf( &buffer[ count ], "%d", params.border );
			count += i;
		}
		buffer[ count++ ] = '\t';
	}
	else
	{
		if( params.border != -1 )
			buffer[ count++ ] = params.border;
		else
			buffer[ count++ ] = 0;
	}

#if defined( NEW_SCHEMA )
	char byteField;
//	Directional - byte
	if( ( byteField = params.directional ) == -1 )
		byteField = 0;

	if( byteField == 0 )
		byteField = 3;

	if( doAscii )
	{
		i = sprintf( &buffer[ count ], "%d", byteField );
		count += i;
		buffer[ count++ ] = '\t';
	}
	else
		buffer[ count++ ] = byteField;

//	Seasonal - bit
	if( ( byteField = params.isSeasonal ) == -1 )
		byteField = 0;
	else if( byteField > 1 )
		byteField = 1;

	if( doAscii )
	{
		i = sprintf( &buffer[ count ], "%d", byteField );
		count += i;
		buffer[ count++ ] = '\t';
	}
	else
		buffer[ count++ ] = byteField;

//	Scenic - bit
	if( ( byteField = params.isScenic ) == -1 )
		byteField = 0;
	else if( byteField > 1 )
		byteField = 1;

	if( doAscii )
	{
		i = sprintf( &buffer[ count ], "%d", byteField );
		count += i;
		buffer[ count++ ] = '\t';
	}
	else
		buffer[ count++ ] = byteField;

//	Overpass - bit
	if( ( byteField = params.isOverpass ) == -1 )
		byteField = 0;
	else if( byteField > 1 )
		byteField = 1;

	if( doAscii )
	{
		i = sprintf( &buffer[ count ], "%d", byteField );
		count += i;
		buffer[ count++ ] = '\t';
	}
	else
		buffer[ count++ ] = byteField;

//	Urban - bit
	if( ( byteField = params.isUrban ) == -1 )
		byteField = 0;
	else if( byteField > 1 )
		byteField = 1;

	if( doAscii )
	{
		i = sprintf( &buffer[ count ], "%d", byteField );
		count += i;
		buffer[ count++ ] = '\t';
	}
	else
		buffer[ count++ ] = byteField;

//	Toll - bit
	if( ( byteField = params.isToll ) == -1 )
		byteField = 0;
	else if( byteField > 1 )
		byteField = 1;

	if( doAscii )
	{
		i = sprintf( &buffer[ count ], "%d", byteField );
		count += i;
		buffer[ count++ ] = '\t';
	}
	else
		buffer[ count++ ] = byteField;

//	Tunnel - bit
	if( ( byteField = params.isTunnel ) == -1 )
		byteField = 0;
	else if( byteField > 1 )
		byteField = 1;

	if( doAscii )
	{
		i = sprintf( &buffer[ count ], "%d", byteField );
		count += i;
		buffer[ count++ ] = '\t';
	}
	else
		buffer[ count++ ] = byteField;

//	Routing - bit
	if( ( byteField = params.isRoutable ) == -1 )
		byteField = 0;
	else if( byteField > 1 )
		byteField = 1;

	if( doAscii )
	{
		i = sprintf( &buffer[ count ], "%d", byteField );
		count += i;
		buffer[ count++ ] = '\t';
	}
	else
		buffer[ count++ ] = byteField;

//	Bit8
	if( doAscii )
	{
		buffer[ count++ ] = '0';
		buffer[ count++ ] = '\t';
	}
	else
		buffer[ count++ ] = 0;
#endif

//	num_pts - small int
	if( doAscii )
	{
		i = sprintf( &buffer[ count ], "%d\t", nPts );
		count += i;
	}
	else
	{
		short s = (short)nPts;

		::memcpy( &buffer[ count ], &s, 2 );
		count += 2;
	}

//
// sPts - skip for now
//	buffer[ count++ ] = ',';
	if( doAscii )
	{
	  buffer[ count ] = '\0';
		::fputs( buffer, file );
	}
	else
	{
		::fwrite( buffer, count, 1, file );
	}

  if( nPts == 2 )
  {
		if (shapePtsFile == 0)
		{
			if( doAscii )
			{
				fputc( '\t', file );
			}
			else
			{
				buffer[ 0 ] = 0;		//	geo_pts
				buffer[ 1 ] = 0;
				buffer[ 2 ] = 0;		//	geo_blob
				buffer[ 3 ] = 0;
				buffer[ 4 ] = 0;
				buffer[ 5 ] = 0;

 				::fwrite( buffer, 6, 1, file );
			}
		}
  }
  else
  {
		nPts -= 2;
		if (shapePtsFile == 0)
		{
			short nVarBinBytes;
			long nBlobBytes;

			if( nPts > 31 )
			{
				nVarBinBytes = 0;
				nBlobBytes = nPts * 2 * sizeof( GeoPoint::Coord );
			}
			else
			{
				nVarBinBytes = nPts * 2 * sizeof( GeoPoint::Coord );
				nBlobBytes = 0;
			}

			if( nVarBinBytes == 0 )
			{
				if( doAscii )
					fputc( '\t', file );
				else
				{
					buffer[ 0 ] = 0;		//	geo_pts
					buffer[ 1 ] = 0;

					::fwrite( buffer, 2, 1, file );
				}
			}
			else
			{
				if( ! doAscii )
					::fwrite( &nVarBinBytes, 2, 1, file );
			}
			
			if( nBlobBytes != 0 )
			{
				if( ! doAscii )
					::fwrite( &nBlobBytes, 4, 1, file );
			}

			for( i = 0; i < nPts; i ++ )
			{
				coord = GeoPoint::DegToLat(sPts[i].x);
				coord2 = GeoPoint::DegToLon(sPts[i].y);

				if( doAscii )
				{
					sprintf( buffer, "%08lx", coord );
					SwapBytes( buffer );
					fputs( buffer, file );
					sprintf( buffer, "%08lx", coord2 );
					SwapBytes( buffer );
					fputs( buffer, file );
				}
				else
				{
					::memcpy( buffer, &coord, 4 );
					::memcpy( &buffer[ 4 ], &coord2, 4 );

					::fwrite( buffer, 8, 1, file );
				}
			}

			if( nBlobBytes == 0 )
			{
				if( doAscii )
					fputc( '\t', file );
				else
					::fwrite( &nBlobBytes, 4, 1, file );
			}
		}
		else
		{
			if( doAscii )
			{
				for( i = 0; i < nPts; i ++ )
				{
					coord = GeoPoint::DegToLat(sPts[i].x);
					coord2 = GeoPoint::DegToLon(sPts[i].y);

					sprintf( buffer, "%ld\t%d\t%ld\t%ld\n", tlid, nPts, coord, coord2 );
					::fputs( buffer, file );
				}
			}
		}
  }

	if( doAscii )
	{
	  fputc( '\n', file );
		if (shapePtsFile != 0)
		  fputc( '\n', shapePtsFile );
	}
}
