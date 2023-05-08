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

  if( rec.cenidl != -1 )
  {
    i = sprintf( &buffer[ count ], "%d", rec.cenidl );
    count += i;
  }
  buffer[ count++ ] = ',';
	
  if( rec.polyidl != -1 )
  {
    i = sprintf( &buffer[ count ], "%ld", rec.polyidl );
    count += i;
  }
  buffer[ count++ ] = ',';

  if( rec.cenidr != -1 )
  {
    i = sprintf( &buffer[ count ], "%d", rec.cenidr );
    count += i;
  }
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

  i = sprintf( &buffer[ count ], "%d,", rec.county );
  count += i;

  i = sprintf( &buffer[ count ], "%ld,", rec.land );
  count += i;
  
  i = sprintf( &buffer[ count ], "%d,", rec.cenid );
  count += i;

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
  GEOPOINT pt;
  
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

  TigerToGeoPoint( rec.lalong, rec.lalat, &pt.lon, &pt.lat );

  i = sprintf( &buffer[ count ], "%d,", rec.county );
  count += i;

  i = sprintf( &buffer[ count ], "%ld,", rec.land );
  count += i;

  for( i = 0; i < 3; i++ )
    buffer[ count++ ] = rec.cfcc[ i ];

  buffer[ count++ ] = ',';
  buffer[ count++ ] = rec.source;
  buffer[ count++ ] = ',';

  i = sprintf( &buffer[ count ], "%ld,", pt.lat );
  count += i;

  i = sprintf( &buffer[ count ], "%ld", pt.lon );
  count += i;

  buffer[ count++ ] = '\n';
  buffer[ count ] = '\0';
  fputs( buffer, file );
}