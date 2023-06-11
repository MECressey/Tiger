#pragma once

#include "sll.hpp"
//#include "geopoint.hpp"
#include "GEOTOOLS.HPP"

#include "blockmem.hpp"

class GeoLine;

class PolyNode : public SLL
{
  public :
	PolyNode( const XY_t & );
	~PolyNode( void );

	GeoLine *lines[ 10 ];
//	long ids[ 10 ];
	double angles[ 10 ];
	unsigned char count;
	void Insert( GeoLine *, double angle );
	//GeoPoint pt;
	XY_t pt;
	void *operator new( size_t );
	void operator delete( void * );

  private :
	static BlockMem memBlock;
};
