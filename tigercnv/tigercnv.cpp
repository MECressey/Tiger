#include <afx.h>
#include <stdlib.h>
#include <string.h>

#include "tigerrec.hpp"
#include "tigerlib.h"

int main( int argc, char *argv[] )
{
  CString fName = argv[ 1 ],
	  		  baseName,
					name,
					work;
  TigerRec5 rec;
	FILE *input,
			 *output;
	long nProcessed = 0,
			 nChanged = 0;

  int dirIdx = fName.Find( "TGR" );
  int dotIdx = fName.ReverseFind( '.' );
//  baseName = ".\\";
  baseName = fName.Mid( dirIdx, dotIdx - dirIdx );

  if( ( input = fopen( fName, "r" ) ) == 0 )
	{
		printf( "Cannot open input %s\n", (const char *)fName );
		return( 0 );
	}
/*
  int beg = baseName.Find( "tgr" );
	baseName = baseName.Right(baseName.GetLength() - beg);
*/
	baseName += "t.tab";
  if( ( output = ::fopen( baseName, "w" ) ) == 0 )
	{
		printf( "Cannot open output %s\n", (const char *)baseName );
		return( 0 );
	}

  while( rec.GetNextRec( input ) > 0 )
	{
		nProcessed++;
		fprintf( output, "%d\t%d\t%ld\t%s\t%s\t%s\t%s\n", rec.state, rec.county,
//		fprintf( output, "%d\t%d\t%ld\t%s\t%s\t%s\t%s\t\n", rec.state, rec.county,  Needed extra tab for Access
			rec.feat, rec.fn.fedirp, rec.fn.fename, rec.fn.fetype, rec.fn.fedirs );
	}

	printf( "Processed %ld names, # changed: %ld\n", nProcessed, nChanged);

	::fclose( input );
	::fclose( output );

	return( 0 );
}
