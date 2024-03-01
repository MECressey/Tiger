//
//	tigercnv.cpp - is a program that converts the .RF5 file to a *t.tab names file for loading into an RDBMS
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
			rec.feat, rec.fn.fedirp, rec.fn.fename, rec.fn.fetype, rec.fn.fedirs );
	}

	printf( "Processed %ld names, # changed: %ld\n", nProcessed, nChanged);

	::fclose( input );
	::fclose( output );

	return( 0 );
}
