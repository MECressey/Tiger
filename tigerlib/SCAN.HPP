//
//	SCAN.HPP - is the declaration of the Scan class which parses character data.
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
#if ! defined( __SCAN_HPP )
#define __SCAN_HPP

class Scan
{
  public :
	void Set( char * );

	Scan( char *string )			{ this->Set( string ); }
    int Get( char *, unsigned length = 1 );
    int Get( int *, unsigned length = 0 );
    int Get( long *, unsigned length = 0 );
    int Get( float *, unsigned length = 0 );
    int Get( double *, unsigned length = 0 );

  private :
    char *record;
};

#endif
