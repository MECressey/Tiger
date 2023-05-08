#if ! defined( TIGERRT6_H )
#define TIGERRT6_H

#include <stdio.h>
#include "tigerrt1.h"

struct TigerRec6
{
  int version;
  long tlid;
  int rtsq;
  AddressRange ar;

  int GetNextRec( FILE * );
};

#endif
