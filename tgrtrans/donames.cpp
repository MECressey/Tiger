#include <stdio.h>

#include "tigerrec.hpp"
#include "donames.h"

const char DELIMITER = '\t';

void DoNames( FILE *file, int stateFips, int countyFips, long featNum,
		long tlid, int rtsq, const FeatureName &fn )
{
  char buffer[ 128 ];
  int i,
  	  count = 0;

/*  if( fn.fedirp[ 0 ] != ' ' || fn.fename[ 0 ] != ' ' ||
	  fn.fetype[ 0 ] != ' ' || fn.fedirs[ 0 ] != ' ' )
*/
  {
		i = sprintf( &buffer[ count ], "%d", stateFips );
		count += i;
		buffer[ count++ ] = DELIMITER;

		i = sprintf( &buffer[ count ], "%d", countyFips );
		count += i;
		buffer[ count++ ] = DELIMITER;

		i = sprintf( &buffer[ count ], "%ld", tlid );
		count += i;
		buffer[ count++ ] = DELIMITER;

		i = sprintf( &buffer[ count ], "%ld", featNum );
		count += i;
		buffer[ count++ ] = DELIMITER;

		i = sprintf( &buffer[ count ], "%d", rtsq );
		count += i;

#ifdef SAVE_FOR_NOW
		buffer[ count++ ] = DELIMITER;

		if( fn.fename[ 0 ] != ' ' )
		{
		  i = strlen( fn.fename );
		  memcpy( &buffer[ count ], fn.fename, i );
		  count += i;
		}
		buffer[ count++ ] = DELIMITER;
	
		if( fn.fedirp[ 0 ] != ' ' )
		{
		  i = strlen( fn.fedirp );
		  memcpy( &buffer[ count ], fn.fedirp, i );
		  count += i;
		}
		buffer[ count++ ] = DELIMITER;
		  
		if( fn.fetype[ 0 ] != ' ' )
		{
		  i = strlen( fn.fetype );
		  memcpy( &buffer[ count ], fn.fetype, i );
		  count += i;
		}
		buffer[ count++ ] = DELIMITER;

		if( fn.fedirs[ 0 ] != ' ' )
		{
		  i = strlen( fn.fedirs );
		  memcpy( &buffer[ count ], fn.fedirs, i );
		  count += i;
		}
#endif

		buffer[ count++ ] = '\n';
		buffer[ count ] = '\0';
		fputs( buffer, file );
  }
}
