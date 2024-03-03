//
//	bldpoly2.h - declaration for building hydro polygons using Tiger/Line blocks.  The Tiger/Line files ended in 2006
//	and the Census Bureau started using ESRI shapefiles as the data exchange format.
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

#include <map>
#include "linetable.hpp"
#include "nodetabl.hpp"
#include "tigerdb.hpp"

using namespace NodeEdgePoly;

class DbHash : public DbHashAccess {
public:
  long tlid;
  int is_equal(DbObject* dbo) { return this->tlid == ((TigerDB::Chain*)dbo)->userId; }
  long int hashKey(int nBits) { return HashTable::HashDK(nBits, tlid); }
};

const int OPEN_WATER = 1251;
const int LAND_ISLAND = 1252;

// New method for building Hydro polygons using GeoDB Topology
int BuildPoly(
  TigerDB& tDB,
  const char* polyName,
  unsigned polyDfcc,
  long polyId,
  CArray<GeoDB::DirLineId, GeoDB::DirLineId&>&,
  int nLines,
  int stateCode,
  const char* name,
  long* newId,
  int extraIds,
  const char* islandName = 0,
  int startDfcc = 0,				// Do not start with this dfcc if it is set
  int isleDfcc = LAND_ISLAND,
  BOOL checkDir = TRUE
);

// This is the old method for building Hydro polygons using Tiger/Line blocks
int BuildPoly2(
  TigerDB &tDB,
  std::map<int, int>& tlidMap,
  const char *polyName,
  unsigned polyDfcc,
  long polyId,
  CArray<GeoDB::DirLineId, GeoDB::DirLineId &> &,
  int nLines,
  LineTable &,
  NodeTable &,
  int stateCode,
	const char *name,
  long *newId,
  int extraIds,
  const char *islandName = 0,
  int startDfcc = 0,				// Do not start with this dfcc if it is set
  int isleDfcc = LAND_ISLAND,
  BOOL checkDir = TRUE
);



