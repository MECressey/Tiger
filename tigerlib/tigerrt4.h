#if ! defined( TIGERRT4_H )
#define TIGERRT4_H

#include <stdio.h>

struct TigerRec4
{
  int version;
  long tlid;
  int rtsq;
  long feats[ 5 ];
  int nFeatures;

  int GetNextRec( FILE * );
};

#endif
