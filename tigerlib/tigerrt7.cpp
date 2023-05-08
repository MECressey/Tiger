#include "scan.hpp"
#include "tigerrt7.h"

int TigerRec7::GetNextRec( FILE *file )
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

  scan.Get( &cc );		// Record type
  scan.Get( &this->version, 4 );
  scan.Get( &this->state, 2 );
  scan.Get( &this->county, 3 );
  scan.Get( &this->land, 10 );
  scan.Get( &this->source );
  count = scan.Get( this->cfcc, 3 );
  this->cfcc[ count ] = '\0';
  
  count = scan.Get( this->laname, 30 );
  this->laname[ count ] = '\0';

  if( scan.Get( &this->lalong, 10 ) == 0 )
    this->lalong = -1;

  if( scan.Get( &this->lalat, 9 ) == 0 )
    this->lalat = -1;

  return( 1 );
}
