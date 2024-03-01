//
//	tigerlib.h - has some general purposed methods for processing Tiger/Line files (2006).
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
#if ! defined( TIGERLIB_H )
#define TIGERLIB_H

#include "tigerrt1.h"
#include <afx.h>

int CfccToDfcc( const char *cfcc );

void TigerToGeoPoint( long tlon, long tlat, double *x, double *y );

bool FixFeatureName(FeatureName &);

#endif

