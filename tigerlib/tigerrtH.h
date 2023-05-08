#if ! defined( TIGERRTH_H )
#define TIGERRTH_H

#include <stdio.h>

struct TigerRecH
{
  int version;
  long tlid;
  int state;
  int county;
  int hist;
  int source;
  long tlidFr1,
  	   tlidFr2,
	   tlidTo1,
	   tlidTo2;

  int GetNextRec( FILE * );
};

#endif
