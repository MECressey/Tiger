#include "scan.hpp"
#include "tigerrt6.h"

int TigerRec6::GetNextRec( FILE *file )
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
  scan.Get( &this->version, 4 );
  scan.Get( &this->tlid, 10 );
  scan.Get( &this->rtsq, 3 );

  this->ar.Get( scan );

  return( 1 );
}
