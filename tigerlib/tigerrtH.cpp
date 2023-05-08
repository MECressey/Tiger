#include "scan.hpp"
#include "tigerrtH.h"

int TigerRecH::GetNextRec( FILE *file )
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
  scan.Get( &this->state, 2 );
  scan.Get( &this->county, 3 );
  scan.Get( &this->tlid, 10 );
  if( scan.Get( &this->hist, 1 ) == 0 )
    this->hist = -1;
  if( scan.Get( &this->source, 1 ) == 0 )
    this->source = -1;
  scan.Get( &this->tlidFr1, 10 );
  scan.Get( &this->tlidFr2, 10 );
  scan.Get( &this->tlidTo1, 10 );
  scan.Get( &this->tlidTo2, 10 );

  return( 1 );
}
