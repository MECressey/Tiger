//
//	dogtpoly.cpp - are the methods that writes Tiger GT Polygon/Landmark data to several text files.
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
#include <stdio.h>
#include <string.h>

#include "tigerrec.hpp"
#include "tigerlib.h"
#include "dogtpoly.h"

void DoGTpoly( FILE *file, const TigerRecI &rec )
{
  char buffer[ 80 ];
  int i,
  	  count = 0;

  i = sprintf( buffer, "%ld,", rec.tlid );
  count += i;

  /*if (rec.cenidl != -1)
  {
    i = sprintf( &buffer[ count ], "%d", rec.cenidl );
    count += i;
  }*/
  memcpy(&buffer[count], rec.cenidl, sizeof(rec.cenidl));
  count += sizeof(rec.cenidl);
  buffer[ count++ ] = ',';
	
  if( rec.polyidl != -1 )
  {
    i = sprintf( &buffer[ count ], "%ld", rec.polyidl );
    count += i;
  }
  buffer[ count++ ] = ',';

  memcpy(&buffer[count], rec.cenidr, sizeof(rec.cenidr));
  count += sizeof(rec.cenidr);
  buffer[ count++ ] = ',';
	
  if( rec.polyidr != -1 )
  {
    i = sprintf( &buffer[ count ], "%ld", rec.polyidr );
    count += i;
  }

  buffer[ count++ ] = '\n';
  buffer[ count ] = '\0';
  fputs( buffer, file );
}


void DoLMlink( FILE *file, const TigerRec8 &rec )
{
  char buffer[ 80 ];
  int i,
  	  count = 0;

  i = sprintf(&buffer[count], "%d,", rec.state);
  count += i;

  i = sprintf( &buffer[ count ], "%d,", rec.county );
  count += i;

  i = sprintf( &buffer[ count ], "%ld,", rec.land );
  count += i;
  
  memcpy(&buffer[count], rec.cenid, sizeof(rec.cenid));
  count += sizeof(rec.cenid);
  buffer[count++] = ',';

  i = sprintf( &buffer[ count ], "%ld", rec.polyid );
  count += i;

  buffer[ count++ ] = '\n';
  buffer[ count ] = '\0';
  fputs( buffer, file );
}


void DoLMarea( FILE *file, const TigerRec7 &rec )
{
  char buffer[ 80 ];
  int i,
  	  count = 0;

  if( ( i = strlen( (const char *)rec.laname ) ) == 0 )
  {
		memcpy( buffer, "UNKNOWN", 7 );
		count = 7;
  }
  else
  {
    memcpy( buffer, rec.laname, i );
    count += i;
  }
  buffer[ count++ ] = ',';

  i = sprintf( &buffer[ count ], "%d,", rec.county );
  count += i;

  i = sprintf( &buffer[ count ], "%ld,", rec.land );
  count += i;

  for( i = 0; i < 3; i++ )
    buffer[ count++ ] = rec.cfcc[ i ];

  buffer[ count++ ] = ',';
  buffer[ count++ ] = rec.source;

  buffer[ count++ ] = '\n';
  buffer[ count ] = '\0';
  fputs( buffer, file );
}


void DoLMpoint( FILE *file, const TigerRec7 &rec )
{
  char buffer[ 80 ];
  int i,
  	  count = 0;
  XY_t pt;
  //GEOPOINT pt;
  
  if( ( i = strlen( (const char *)rec.laname ) ) == 0 )
  {
		memcpy( buffer, "UNKNOWN", 7 );
		count = 7;
  }
  else
  {
    memcpy( buffer, rec.laname, i );
    count += i;
  }
  buffer[ count++ ] = ',';

  TigerToGeoPoint( rec.lalong, rec.lalat, &pt.x, &pt.y );

  i = sprintf( &buffer[ count ], "%d,", rec.county );
  count += i;

  i = sprintf( &buffer[ count ], "%ld,", rec.land );
  count += i;

  for( i = 0; i < 3; i++ )
    buffer[ count++ ] = rec.cfcc[ i ];

  buffer[ count++ ] = ',';
  buffer[ count++ ] = rec.source;
  buffer[ count++ ] = ',';

  i = sprintf( &buffer[ count ], "%ld,", pt.y );    // This method won't work for XY_t
  count += i;

  i = sprintf( &buffer[ count ], "%ld", pt.x );
  count += i;

  buffer[ count++ ] = '\n';
  buffer[ count ] = '\0';
  fputs( buffer, file );
}
