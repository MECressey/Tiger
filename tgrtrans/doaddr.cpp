#include <stdio.h>
#include <string.h>

#include "tigerrec.hpp"
#include "doaddr.h"

void DoAddress( FILE *file, long tlid, int rtsq, const AddressRange &ar )
{
  char buffer[ 128 ];
  int i,
  	  count = 0;

  if( ar.fraddl[ 0 ] != '\0' || ar.toaddl[ 0 ] != '\0' ||
	  ar.fraddr[ 0 ] != '\0' || ar.toaddr[ 0 ] != '\0' )
  {
	i = sprintf( &buffer[ count ], "%ld,", tlid );
	count += i;

	i = sprintf( &buffer[ count ], "%d,", rtsq );
	count += i;

	if( ar.fraddl[ 0 ] != '\0' )
	{
	  i = ::strlen( ar.fraddl );
	  ::memcpy( &buffer[ count ], ar.fraddl, i );
	  count += i;
	}
	buffer[ count++ ] = ',';

	if( ar.toaddl[ 0 ] != '\0' )
	{
	  i = strlen( ar.toaddl );
	  memcpy( &buffer[ count ], ar.toaddl, i );
	  count += i;
	}
	buffer[ count++ ] = ',';

	if( ar.fraddr[ 0 ] != '\0' )
	{
	  i = strlen( ar.fraddr );
	  memcpy( &buffer[ count ], ar.fraddr, i );
	  count += i;
	}
	buffer[ count++ ] = ',';
	
	if( ar.toaddr[ 0 ] != '\0' )
	{
	  i = strlen( ar.toaddr );
	  memcpy( &buffer[ count ], ar.toaddr, i );
	  count += i;
	}
	buffer[ count++ ] = ',';

	if( ar.friaddl != ' ' )
	  buffer[ count++ ] = ar.friaddl;
	buffer[ count++ ] = ',';

	if( ar.toiaddl != ' ' )
	  buffer[ count++ ] = ar.toiaddl;
	buffer[ count++ ] = ',';

	if( ar.friaddr != ' ' )
	  buffer[ count++ ] = ar.friaddr;
	buffer[ count++ ] = ',';

	if( ar.toiaddr != ' ' )
	  buffer[ count++ ] = ar.toiaddr;
	buffer[ count++ ] = ',';

	if( ar.zipl != -1 )
	{
	  i = sprintf( &buffer[ count ], "%ld", ar.zipl );
	  count += i;
	}
	buffer[ count++ ] = ',';

	if( ar.zipr != -1 )
	{
	  i = sprintf( &buffer[ count ], "%ld", ar.zipr );
	  count += i;
	}

	buffer[ count++ ] = '\n';
	buffer[ count ] = '\0';
	fputs( buffer, file );
  }
}
