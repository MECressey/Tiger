//	tigerrtI.h - declarations for the TigerRecI class.
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

