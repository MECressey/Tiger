#include "scan.hpp"
#include "tigerrt8.h"

int TigerRec8::GetNextRec( FILE *file )
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
  scan.Get( &this->state, 2 );
  scan.Get( &this->county, 3 );
  /*scan.Get(&state, 2);
  scan.Get( &this->cenid, 3 );*/
  scan.Get(this->cenid, sizeof(this->cenid));
  scan.Get( &this->polyid, 10 );
  scan.Get( &this->land, 10 );

  return( 1 );
}
