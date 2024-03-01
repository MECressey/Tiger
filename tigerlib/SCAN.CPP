//
//	SCAN.CPP - is the implementation of the Scan class which parses character data.
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
#include <stdlib.h>
#include <string.h>
//#include "stdafx.h"
#include "scan.hpp"

void Scan::Set( char *string )
{
  this->record = string;
}


int Scan::Get( char *string, unsigned length )
{
  int count = 0;

//  Scan off leading blanks
  while( length > 0 )
  {
		if( *this->record != ' ' )
		  break;
	  
		this->record++;
		--length;
  }

  while( length > 0 )
  {
		*string++ = *this->record++;
		count++;
		--length;
  }

//	Strip off trailing blanks
  while( count > 0 && *(--string) == ' ' )
  {
		--count;
  }
  
  return( count );
}


int Scan::Get( long *integer, unsigned length )
{
  char buf[ 60 ];

  while( *this->record == ' ' )
  {
		this->record++;
    if( --length == 0 )
      return( 0 );
  }

  if( *this->record == '+' )
  {
		this->record++;
    if( --length == 0 )
      return( 0 );
  }
  
  memcpy( buf, this->record, length );
  buf[ length ] = '\0';

  *integer = atol( buf );
  this->record += length;
  
  return( length );
}

int Scan::Get( double *decimal, unsigned length )
{
  char buf[ 60 ];

  while( *this->record == ' ' )
  {
		this->record++;
    if( --length == 0 )
      return( 0 );
  }

  if( *this->record == '+' )
  {
		this->record++;
    if( --length == 0 )
      return( 0 );
  }
  
  memcpy( buf, this->record, length );
  buf[ length ] = '\0';

  *decimal = atof( buf );
  this->record += length;
  
  return( length );
}

int Scan::Get( float *decimal, unsigned length )
{
  double f;
  
  length = this->Get( &f, length );
 
  *decimal = (float)f;
     
  return( length );
}

int Scan::Get( int *integer, unsigned length )
{
  long ii;
  int ret_val;
  
  if( ret_val = this->Get( &ii, length ) )
    *integer = (int)ii; 

  return( ret_val );
}
