//	tigerrt1.h - declarations for the TigerRec1 class and other associated structs: AddressRange and FeatureName.
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
#include "scan.hpp"

struct AddressRange
{
  char fraddl[ 12 ];
  char toaddl[ 12 ];
  char fraddr[ 12 ];
  char toaddr[ 12 ];
  char friaddl;
  char toiaddl;
  char friaddr;
  char toiaddr;
  long zipl;
  long zipr;

  void Get( Scan & );
};

struct FeatureName
{
  char fedirp[ 3 ];
  char fename[ 31 ];
  char fetype[ 5 ];
  char fedirs[ 3 ];

  void Get( Scan & );
};

struct TigerRec1
{
  int version;
  long tlid;
  char side;
  char source;
  FeatureName fn;
  char cfcc[ 4 ];
  AddressRange ar;
  int fairl;
  int fairr;
  char trustl,
  	   trustr;
  char census1;
  char census2;
  int statel;
  int stater;
  int countyl;
  int countyr;
  long fmcdl;
  long fmcdr;
  long fsmcdl;
  long fsmcdr;
  long fpll;
  long fplr;
  long ctbnal;
  long ctbnar;
  int blkl;
  char blkls;
  int blkr;
  char blkrs;
  long frlong;
  long frlat;
  long tolong;
  long tolat;

  int GetNextRec( FILE * );
  void GetFromGeoPoint( XY_t * );
  void GetToGeoPoint( XY_t * );
};

