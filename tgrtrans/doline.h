//
//	doline.h - is the declaration of a method that writes line data to a text file.
//  Copyright(C) 2024 Michael E. Cressey
//
//	This program is free software : you can redistribute it and /or modify it under the terms of the
//	GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or
//	any later version.
//
//	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
//	implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License along with this program.
//  If not, see https://www.gnu.org/licenses/
//
#if ! defined( DOLINE_H )
#define DOLINE_H

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

