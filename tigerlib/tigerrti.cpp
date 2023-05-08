#include "scan.hpp"
#include "tigerrti.h"

int TigerRecI::GetNextRec( FILE *file )
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
  int state;

  scan.Get( &cc );		// Record type
  scan.Get( &this->version, 4 );
  scan.Get( &this->tlid, 10 );
  scan.Get( &this->state, 2 );
  scan.Get( &this->county, 3 );
  scan.Get( &this->rtlink );
  scan.Get( &state, 2 );
  if( scan.Get( &this->cenidl, 3 ) == 0 )
    this->cenidl = -1;
  if( scan.Get( &this->polyidl, 10 ) == 0 )
    this->polyidl = -1;

  scan.Get( &state, 2 );
  if( scan.Get( &this->cenidr, 3 ) == 0 )
    this->cenidr = -1;

  if( scan.Get( &this->polyidr, 10 ) == 0 )
    this->polyidr = -1;

  return( 1 );
}
