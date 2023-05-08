#if ! defined( TIGERRT8_H )
#define TIGERRT8_H

#include <stdio.h>

struct TigerRec8
{
  int version;
  int state;
  int county;
  int cenid;		// just county code
  long polyid;
  long land;

  int GetNextRec( FILE * );
};

#endif
