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
