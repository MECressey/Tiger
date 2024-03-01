//	tigerrt1.cpp - implementation for the TigerRec1 class and other associated structs: AddressRange and FeatureName.
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
#include "tigerrt1.h"
#include "tigerlib.h"

void AddressRange::Get( Scan &scan )
{
  int count = scan.Get( this->fraddl, 11 );
  this->fraddl[ count ] = '\0';

  count = scan.Get( this->toaddl, 11 );
  this->toaddl[ count ] = '\0';

  count = scan.Get( this->fraddr, 11 );
  this->fraddr[ count ] = '\0';

  count = scan.Get( this->toaddr, 11 );
  this->toaddr[ count ] = '\0';

  if( ( count = scan.Get( &this->friaddl ) ) == 0 )
		this->friaddl = ' ';

  if( ( count = scan.Get( &this->toiaddl ) ) == 0 )
		this->toiaddl = ' ';

  if( ( count = scan.Get( &this->friaddr ) ) == 0 )
		this->friaddr = ' ';
  
  if( ( count = scan.Get( &this->toiaddr ) ) == 0 )
		this->toiaddr = ' ';

  if( ! scan.Get( &this->zipl, 5 ) )
    this->zipl = -1;

  if( ! scan.Get( &this->zipr, 5 ) )
    this->zipr = -1;
}

void FeatureName::Get( Scan &scan )
{
  int count = scan.Get( this->fedirp, sizeof( this->fedirp ) - 1 );

  this->fedirp[ count ] = '\0';

	count = scan.Get( this->fename, sizeof( this->fename ) - 1 );
  this->fename[ count ] = '\0';

  count = scan.Get( this->fetype, sizeof( this->fetype ) - 1 );
  this->fetype[ count ] = '\0';

  count = scan.Get( this->fedirs, sizeof( this->fedirs ) - 1 );
  this->fedirs[ count ] = '\0';

	FixFeatureName(*this);
}

static int IsBlank( char *field, int nChars )
{
  for( int i = nChars; --i >= 0; )
  {
		if( *field != ' ' )
		  return( 0 );
		field++;
  }

  return( 1 );
}

int TigerRec1::GetNextRec( FILE *file )
{
  char record[ 256 ];
    
  if( ::fgets( record, sizeof( record ) - 1, file ) == NULL )
  {
    if( feof( file ) )
      return( 0 );

    return( -1 );
  }
    
//
//	Fix tract number so that handles the suffix correctly
//
  if( ! IsBlank( &record[ 170 ], 4 ) )
  {
		if( record[ 174 ] == ' ' )
		  record[ 174 ] = '0';
		if( record[ 175 ] == ' ' )
		  record[ 175 ] = '0';
  }

  if( ! IsBlank( &record[ 176 ], 4 ) )
  {
		if( record[ 180 ] == ' ' )
		  record[ 180 ] = '0';
		if( record[ 181 ] == ' ' )
		  record[ 181 ] = '0';
  }

  Scan scan( record );
  char cc;
  int count;
  
  scan.Get( &cc );
	int month;
  scan.Get( &month, 2 );
  scan.Get( &this->version, 2 );
  scan.Get( &this->tlid, 10 );
  if( scan.Get( &this->side ) == 0 )
    this->side = '\0';
  scan.Get( &this->source );

  this->fn.Get( scan );
  
  count = scan.Get( this->cfcc, 3 );
  this->cfcc[ count ] = '\0';

  this->ar.Get( scan );
  
  if( ! scan.Get( &this->fairl, 5 ) )
    this->fairl = -1;
  if( ! scan.Get( &this->fairr, 5 ) )
    this->fairr = -1;

  switch( this->version )
  {
	default :
	  printf( "* TigerRec1::GetNextRec - unknown version: %d\n", this->version );
	  break;

	case 5 :
	case 6:
#ifdef SAVE_FOR_NOW
	  if( ! scan.Get( &this->anrcl, 2 ) )
			this->anrcl = -1;
	  if( ! scan.Get( &this->anrcr, 2 ) )
			this->anrcr = -1;
	  break;
#endif

	case 21 :
	case 97 :
	case 98 :
	  scan.Get( &this->trustl );
	  scan.Get( &this->trustr );
	  scan.Get( &this->census1 );
	  scan.Get( &this->census2 );
#ifdef SAVE_FOR_NOW
	  scan.Get( &this->bbspcen );
	  scan.Get( &this->bbsppart );
#endif
	  break;
  }

  if( ! scan.Get( &this->statel, 2 ) )
    this->statel = -1;
  if( ! scan.Get( &this->stater, 2 ) )
    this->stater = -1;
  if( ! scan.Get( &this->countyl, 3 ) )
    this->countyl = -1;
  if( ! scan.Get( &this->countyr, 3 ) )
    this->countyr = -1;
  if( ! scan.Get( &this->fmcdl, 5 ) )
    this->fmcdl = -1;
  if( ! scan.Get( &this->fmcdr, 5 ) )
    this->fmcdr = -1;
  if( ! scan.Get( &this->fsmcdl, 5 ) )
    this->fsmcdl = -1;
  if( ! scan.Get( &this->fsmcdr, 5 ) )
    this->fsmcdr = -1;
  if( ! scan.Get( &this->fpll, 5 ) )
    this->fpll = -1;
  if( ! scan.Get( &this->fplr, 5 ) )
    this->fplr = -1;
  if( ! scan.Get( &this->ctbnal, 6 ) )
    this->ctbnal = -1;
  if( ! scan.Get( &this->ctbnar, 6 ) )
    this->ctbnar = -1;
  if( ! scan.Get( &this->blkl, 3 ) )
    this->blkl = -1;
  if( ! scan.Get( &this->blkls ) )
	this->blkls = ' ';
  if( ! scan.Get( &this->blkr, 3 ) )
    this->blkr = -1;
  if( ! scan.Get( &this->blkrs ) )
	this->blkrs = ' ';
  count = scan.Get( &this->frlong, 10 );
  count = scan.Get( &this->frlat, 9 );
  count = scan.Get( &this->tolong, 10 );
  count = scan.Get( &this->tolat, 9 );

  return( 1 );
}

void TigerRec1::GetFromGeoPoint( XY_t *pt )
{
  ::TigerToGeoPoint( this->frlong, this->frlat, &pt->x, &pt->y );
}

void TigerRec1::GetToGeoPoint( XY_t *pt )
{
  ::TigerToGeoPoint( this->tolong, this->tolat, &pt->x, &pt->y );
}

