#include <stdio.h>

#include "tigerrec.hpp"
#include "doblock.h"

const char DELIMITER = '\t';

static int MakeBuffer(short countyFips, const TigerRec1 &rec1, char buffer[] )
{
  int i,
  	  count = 0;
	i = sprintf(&buffer[count], "%d", countyFips);
	count += i;
	buffer[count++] = DELIMITER;

	i = sprintf( &buffer[ count ], "%ld", rec1.tlid );
	count += i;
	buffer[count++] = DELIMITER;

	if( rec1.blkl != -1 )
	{
	  i = sprintf( &buffer[ count ], "%d", rec1.blkl );
	  count += i;
	}
	if( rec1.blkls != ' ' )
	  buffer[count++] = rec1.blkls;

	buffer[count++] = DELIMITER;
	
	if( rec1.blkr != -1 )
	{
	  i = sprintf( &buffer[ count ], "%d", rec1.blkr );
	  count += i;
	}
	if( rec1.blkrs != ' ' )
	  buffer[count++] = rec1.blkrs;
	buffer[count++] = DELIMITER;

	if( rec1.ctbnal != -1 )
	{
	  i = sprintf( &buffer[count], "%d", rec1.ctbnal );
	  count += i;
	}
	buffer[count++] = DELIMITER;
	
	if( rec1.ctbnar != -1 )
	{
	  i = sprintf( &buffer[count], "%d", rec1.ctbnar );
	  count += i;
	}
	buffer[count++] = DELIMITER;

	if( rec1.countyl != -1 )
	{
	  i = sprintf( &buffer[count], "%d", rec1.countyl );
	  count += i;
	}
	buffer[count++] = DELIMITER;
	
	if( rec1.countyr != -1 )
	{
	  i = sprintf( &buffer[count], "%d", rec1.countyr );
	  count += i;
	}
	buffer[count++] = '\n';
	buffer[count] = '\0';

	return( count );
}

void DoBlocks( FILE *file, FILE *file2, const TigerRec1 &rec1, short countyFips)
{
  char buffer[128];

  MakeBuffer(countyFips, rec1, buffer);

  if( rec1.blkl != rec1.blkr || rec1.blkls != rec1.blkrs ||
	  rec1.ctbnal != rec1.ctbnar || rec1.countyl != rec1.countyr ||
	  rec1.statel != rec1.stater )
  {
		if( file != 0 )
			fputs( buffer, file );
  }
  else if( file2 != 0 )
  {
		fputs( buffer, file2 );
  }
}
