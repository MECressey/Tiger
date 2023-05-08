#if ! defined( TIGERRTZ_H )
#define TIGERRTZ_H

#include <stdio.h>

struct TigerRecZ
{
  int version;
  long tlid;
  int rtsq;
  int zip4L,
  	  zip4R;

  int GetNextRec( FILE * );
};

#endif
