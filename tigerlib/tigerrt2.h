#if ! defined( TIGERRT2_H )
#define TIGERRT2_H

#include <stdio.h>
#include "geotools.hpp"

struct TigerRec2
{
  int version;
  long tlid;
  int rtsq;
  long longs[ 10 ];
  long lats[ 10 ];
  int nPts;

  int GetNextRec( FILE * );
  TigerRec2( void );
  unsigned GetPoints( FILE *, long *tLID, XY_t pts[], unsigned nPts );
};

#endif
