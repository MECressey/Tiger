#if ! defined( TIGERLIB_H )
#define TIGERLIB_H

#include "tigerrt1.h"

#include <afx.h>
int CfccToDfcc( const char *cfcc );

void TigerToGeoPoint( long tlon, long tlat, double *x, double *y );

bool FixFeatureName(FeatureName &);

#endif

