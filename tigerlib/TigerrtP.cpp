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
