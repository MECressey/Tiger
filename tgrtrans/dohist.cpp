#include <stdio.h>
#include <string.h>

#include "tigerrec.hpp"
#include "tigerlib.h"
#include "dogtpoly.h"

void DoHistory( FILE *file, const TigerRecH &rec )
{
  char buffer[ 80 ];
  int i,
  	  count = 0;

  i = sprintf( buffer, "%ld,", rec.tlid );
  count += i;

  i = sprintf( &buffer[ count ], "%ld,", rec.tlidFr1 );
  count += i;

  i = sprintf( &buffer[ count ], "%ld,", rec.tlidFr2 );
  count += i;

  i = sprintf( &buffer[ count ], "%ld,", rec.tlidTo1 );
  count += i;

  i = sprintf( &buffer[ count ], "%ld", rec.tlidTo2 );
  count += i;

  buffer[ count++ ] = '\n';
  buffer[ count ] = '\0';
  fputs( buffer, file );
}
