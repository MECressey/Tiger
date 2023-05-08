#if ! defined( TIGERRT5_H )
#define TIGERRT5_H

#include <stdio.h>
#include "tigerrt1.h"

struct TigerRec5
{
  int version;
  int state;
  int county;
  long feat;
  FeatureName fn;

  int GetNextRec( FILE * );
};

#endif
