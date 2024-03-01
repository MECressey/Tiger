//	tigerrt2.h - declarations for the TigerRec2 class.
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
#pragma once

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

