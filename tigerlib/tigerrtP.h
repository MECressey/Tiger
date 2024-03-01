//	tigerrtP.h - declarations for the TigerRecP class.
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

struct TigerRecP
{
  int version;
  int file;
  char cenid[6];
  long polyid;
  long polyLong;
  long polyLat;
  char water;

  int GetNextRec(FILE*);
};
