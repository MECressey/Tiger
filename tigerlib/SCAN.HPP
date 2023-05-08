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
