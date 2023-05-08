#if ! defined( DOLINE_H )
#define DOLINE_H

//#include <afx.h>
#include <stdio.h>
#include "tigerdb.hpp"

struct TigerParams
{
	int fips;
	int border;
	TigerDB::Classification code;
};

// File Format:
//	TLID
//	Class Code
//	From Lat
//	From Lon
//	To Lat
//	To Lon
//	MBR
//	Border bit
//	#pts
//	VarBinary
//	Blob

void WriteLine(
  FILE *,
	FILE *,
  long tlid,
  const XY_t &frNode,
  const XY_t &toNode,
  Range2D &range,
	const TigerParams &,
  unsigned nPts = 2,
  XY_t *sPts = 0,
	bool doAscii = true
);

#endif

