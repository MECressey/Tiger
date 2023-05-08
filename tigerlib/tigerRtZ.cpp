#include "scan.hpp"
#include "tigerrtZ.h"

int TigerRecZ::GetNextRec( FILE *file )
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
  if( scan.Get( &this->zip4L, 4 ) == 0 )
		this->zip4L = -1;

  if( scan.Get( &this->zip4R, 4 ) == 0 )
		this->zip4R = -1;

  return( 1 );
}
