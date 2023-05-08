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
