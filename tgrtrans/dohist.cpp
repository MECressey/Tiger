//
//	dohist.cpp - is the declaration of a method that writes History data to a text file.
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
