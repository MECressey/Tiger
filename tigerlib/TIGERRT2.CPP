//	tigerrt2.cpp - implementation for the TigerRec2 class.
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
#include <io.h>

#include "tigerrt2.h"
#include "scan.hpp"
#include "tigerlib.h"

TigerRec2::TigerRec2( void )
{
  this->version = 0;
  this->tlid = 0L;
  this->rtsq = 0;
  this->nPts = 0;
}

int TigerRec2::GetNextRec( FILE *file )
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
  int count;

  scan.Get( &cc );
  scan.Get( &this->version, 4 );
  scan.Get( &this->tlid, 10 );
  scan.Get( &this->rtsq, 3 );

  count = scan.Get( &this->longs[ 0 ], 10 );
  count = scan.Get( &this->lats[ 0 ], 9 );
  this->nPts = 1;

  for( int i = 1; i < 10; i++ )
  {
    count = scan.Get( &this->longs[ i ], 10 );
    if( this->longs[ i ] == 0L && count == 9 )
      break;
    count = scan.Get( &this->lats[ i ], 9 );
    ++this->nPts;
  }

  return( 1 );
}

unsigned TigerRec2::GetPoints( FILE *file, long *tLID, XY_t pts[], unsigned nPts )
{
  if( this->tlid == *tLID )
		if( this->GetNextRec( file ) != 1 )
		{
		  return( 0 );
		}  

  *tLID = this->tlid;
  
//	Get all the pts for a TLID
  unsigned count = 0;
  do
  {
    for( int i = 0; i < this->nPts; i++ )
    {
		  if( count < nPts )
		  {
		    ::TigerToGeoPoint( this->longs[ i ], this->lats[ i ],
		        &pts[ count ].x, &pts[ count ].y );
				count++;
		  }
    }
    
		if( this->nPts < 10 )
		  break;
	  
    if( this->GetNextRec( file ) <= 0 )
		  break;
  }
  while( this->tlid == *tLID );

  return( count );
}

#ifdef SAVE_FOR_NOW

unsigned TigerRec2::GetPoints( FILE *file, long tLID,
    GeoPoint pts[], unsigned nPts )
{
  long fpos = ::ftell( file );

//	Look for first record with the same TLID
  while( this->tlid < tLID )
  {
    if( this->GetNextRec( file ) <= 0 )
    {
      ::rewind( file );
      if( fpos == 0L )
        return( 0 );
	  break;
    }  
  }

  if( this->tlid != tLID )
    return( 0 );
  
//	Start at BOF & look for first record with the same TLID 
/*  while( this->tlid != tLID && ::ftell( file ) < fpos )
  {
    if( this->GetNextRec( file ) == 0 )
	  return( 0 );
  }
*/  
//	Get all the pts for a TLID
  unsigned count = 0;
  do
  {
    for( int i = 0; i < this->nPts; i++ )
    {
	  if( count < nPts )
	  {
	    ::TigerToGeoPoint( this->longs[ i ], this->lats[ i ],
	        &pts[ count ].lon, &pts[ count ].lat );
		count++;
	  }
    }
    
	if( this->nPts < 10 )
	  break;
	  
    this->GetNextRec( file );
  }
  while( this->tlid == tLID );

  return( count );
}

#endif
