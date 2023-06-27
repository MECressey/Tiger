#if ! defined( TIGERRTI_H )
#define TIGERRTI_H

#include <stdio.h>

struct TigerRecI
{
  int version;
  int state;
  int county;
  long tlid;
  int tzids;
  int tzide;
  //char rtlink;
  //int cenidl;		// just county code
  char cenidl[5];
  long polyidl;
  char cenidr[5];
  //int cenidr;		// just county code
  long polyidr;

  int GetNextRec( FILE * );
};

#endif
