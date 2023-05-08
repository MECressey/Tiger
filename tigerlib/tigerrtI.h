#if ! defined( TIGERRTI_H )
#define TIGERRTI_H

#include <stdio.h>

struct TigerRecI
{
  int version;
  long tlid;
  int state;
  int county;
  char rtlink;
  int cenidl;		// just county code
  long polyidl;
  int cenidr;		// just county code
  long polyidr;

  int GetNextRec( FILE * );
};

#endif
