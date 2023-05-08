#include <stdio.h>

#include "tigerrec.hpp"
#include "dopoly.h"

void DoPolygons( FILE *file, const TigerRec1 &rec1 )
{
  char buffer[ 256 ];
  int i,
      pos = sprintf( buffer, "%ld,", rec1.tlid ),
  	  count = pos;

  if( rec1.fairl != rec1.fairr )
  {
	buffer[ count++ ] = '1';		// FAIR
	buffer[ count++ ] = ',';

	if( rec1.fairl != -1 )
	{
	  i = sprintf( &buffer[ count ], "%d", rec1.fairl );
	  count += i;
	}
	buffer[ count++ ] = ',';

	if( rec1.fairr != -1 )
	{
	  i = sprintf( &buffer[ count ], "%d", rec1.fairr );
	  count += i;
	}

	buffer[ count++ ] = '\n';
	buffer[ count ] = '\0';
	fputs( buffer, file );
	count = pos;
  }

  if( rec1.version <= 5 )
  {
	if( rec1.anrcl != rec1.anrcr )
	{
	  buffer[ count++ ] = '2';		// ANRC
	  buffer[ count++ ] = ',';

	  if( rec1.anrcl != -1 )
	  {
	    i = sprintf( &buffer[ count ], "%d", rec1.anrcl );
	    count += i;
	  }
	  buffer[ count++ ] = ',';

	  if( rec1.anrcr != -1 )
	  {
	    i = sprintf( &buffer[ count ], "%d", rec1.anrcr );
	    count += i;
	  }

	  buffer[ count++ ] = '\n';
	  buffer[ count ] = '\0';
	  fputs( buffer, file );
	  count = pos;
    }
  }

  if( rec1.fmcdl != rec1.fmcdr )
  {
	buffer[ count++ ] = '3';		// MCD
	buffer[ count++ ] = ',';

  	if( rec1.fmcdl != -1 )
	{
	  i = sprintf( &buffer[ count ], "%ld", rec1.fmcdl );
	  count += i;
	}
	buffer[ count++ ] = ',';

	if( rec1.fmcdr != -1 )
	{
	  i = sprintf( &buffer[ count ], "%ld", rec1.fmcdr );
	  count += i;
	}

	buffer[ count++ ] = '\n';
	buffer[ count ] = '\0';
	fputs( buffer, file );
	count = pos;
  }

  if( rec1.fsmcdl != rec1.fsmcdr )
  {
	buffer[ count++ ] = '4';		// SMCD
	buffer[ count++ ] = ',';

	if( rec1.fsmcdl != -1 )
	{
	  i = sprintf( &buffer[ count ], "%ld", rec1.fsmcdl );
	  count += i;
	}
	buffer[ count++ ] = ',';

	if( rec1.fsmcdr != -1 )
	{
	  i = sprintf( &buffer[ count ], "%ld", rec1.fsmcdr );
	  count += i;
	}

	buffer[ count++ ] = '\n';
	buffer[ count ] = '\0';
	fputs( buffer, file );
	count = pos;
  }

  if( rec1.fpll != rec1.fplr )
  {
	buffer[ count++ ] = '5';		// FPL
	buffer[ count++ ] = ',';

	if( rec1.fpll != -1 )
	{
	  i = sprintf( &buffer[ count ], "%ld", rec1.fpll );
	  count += i;
	}
	buffer[ count++ ] = ',';

	if( rec1.fplr != -1 )
	{
	  i = sprintf( &buffer[ count ], "%ld", rec1.fplr );
	  count += i;
	}

	buffer[ count++ ] = '\n';
	buffer[ count ] = '\0';
	fputs( buffer, file );
	count = pos;
  }

  if( rec1.countyl != rec1.countyr )
  {
	buffer[ count++ ] = '6';		// COUNTY
	buffer[ count++ ] = ',';

	if( rec1.countyl != -1 )
	{
	  i = sprintf( &buffer[ count ], "%ld", rec1.countyl );
	  count += i;
	}
	buffer[ count++ ] = ',';

	if( rec1.countyr != -1 )
	{
	  i = sprintf( &buffer[ count ], "%ld", rec1.countyr );
	  count += i;
	}

	buffer[ count++ ] = '\n';
	buffer[ count ] = '\0';
	fputs( buffer, file );
	count = pos;
  }	  

#ifdef SAVE_FOR_NOW
  if( rec1.statel != rec1.stater )
  {
	buffer[ count++ ] = '7';		// STATE
	buffer[ count++ ] = ',';

	if( rec1.statel != -1 )
	{
	  i = sprintf( &buffer[ count ], "%ld", rec1.statel );
	  count += i;
	}
	buffer[ count++ ] = ',';

	if( rec1.stater != -1 )
	{
	  i = sprintf( &buffer[ count ], "%ld", rec1.stater );
	  count += i;
	}

	buffer[ count++ ] = '\n';
	buffer[ count ] = '\0';
	fputs( buffer, file );
	count = pos;
  }	  
#endif
}
