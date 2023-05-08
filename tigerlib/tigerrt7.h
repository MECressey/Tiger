#if ! defined( TIGERRT7_H )
#define TIGERRT7_H

#include <stdio.h>

struct TigerRec7
{
  int version;
  int state;
  int county;
  long land;
  char source;
  char cfcc[ 4 ];
  char laname[ 31 ];
  long lalong;
  long lalat;

  int GetNextRec( FILE * );
};

#endif
