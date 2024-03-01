//	tigerrt5.cpp - implementation for the TigerRec5 class.
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
#include "scan.hpp"
#include "tigerrt5.h"

int TigerRec5::GetNextRec( FILE *file )
{
  char record[ 256 ];
    
  if( ::fgets( record, sizeof( record ) - 1, file ) == NULL )
  {
    if( feof( file ) )
      return( 0 );

    return( -1 );
  }
    
  Scan scan( record );
  char cc;

  scan.Get( &cc );		// Record type
  scan.Get(&this->version, 4);    // Version
  scan.Get( &this->state, 2 );
  scan.Get( &this->county, 3 );
  scan.Get( &this->feat, 8 );		// These appear to always be in ascending sorted order

  this->fn.Get( scan );

  return( 1 );
}
