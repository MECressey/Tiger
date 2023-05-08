#include "scan.hpp"
#include "tigerrt4.h"

int TigerRec4::GetNextRec( FILE *file )
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
  scan.Get( &this->tlid, 10 );
  scan.Get( &this->rtsq, 3 );

  this->nFeatures = 0;
  for( int i = 0; i < 5; i++ )
  {
		if( ( count = scan.Get( &this->feats[ i ], 8 ) ) == 0 )
		  break;
		this->nFeatures++;
  }

  return( 1 );
}
