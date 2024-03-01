//	tigerrtP.cpp - declarations for the TigerRecP class.
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
#include "scan.hpp"
#include "tigerrtP.h"

int TigerRecP::GetNextRec(FILE* file)
{
  char record[256];

  if (::fgets(record, sizeof(record) - 1, file) == NULL)
  {
    if (feof(file))
      return(0);

    return(-1);
  }

  Scan scan(record);
  char cc;

  scan.Get(&cc);		// Record type
  scan.Get(&this->version, 4);
  scan.Get(&this->file, 5);
  scan.Get(this->cenid, sizeof(this->cenid));
  this->cenid[5] = '\0';
  if (scan.Get(&this->polyid, 10) == 0)
    this->polyid = -1;

  scan.Get(&this->polyLong, 10);
  scan.Get(&this->polyLat, 10);
  scan.Get(&this->water, 1);

  return(1);
}
