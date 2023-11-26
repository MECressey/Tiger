#pragma once

#include <map>
//#include "standard.h"

//#include "cdblines.h"
//#include "cdbpolys.h"
//#include "cdbpline.h"
#include "linetable.hpp"
#include "nodetabl.hpp"
//#include "polytabl.hpp"
#include "tigerdb.hpp"

class DbHash : public DbHashAccess {
public:
  long tlid;
  int is_equal(DbObject* dbo) { return this->tlid == ((TigerDB::Chain*)dbo)->userId/*GetTLID()*/; }
  long int hashKey(int nBits) { return HashTable::HashDK(nBits, tlid); }
};

const int OPEN_WATER = 1251;
const int LAND_ISLAND = 1252;
/*
struct DirLineId
{
  long id;
  signed char dir;
};
*/
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

int BuildPoly3(
  TigerDB& tDB,
  //std::map<int, int>& tlidMap,
  const char* polyName,
  unsigned polyDfcc,
  long polyId,
  CArray<GeoDB::DirLineId, GeoDB::DirLineId&>&,
  int nLines,
  //LineTable&,
  //NodeTable&,
  int stateCode,
  const char* name,
  long* newId,
  int extraIds,
  const char* islandName = 0,
  int startDfcc = 0,				// Do not start with this dfcc if it is set
  int isleDfcc = LAND_ISLAND,
  BOOL checkDir = TRUE
);



