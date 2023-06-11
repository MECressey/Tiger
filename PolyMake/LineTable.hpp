#pragma once

//#include "delorme.h"
//#include "dms.h"
#include "sll.hpp"

//#include "GEOPOINT.HPP"
#include "GEOTOOLS.HPP"
#include "RANGE.HPP"

#include "blockmem.hpp"
#include "hashtabl.hpp"
#include "polynode.HPP"
//#include "snode.hpp"

class GeoLine : public SLL
{
public:
	GeoLine(void);
	~GeoLine(void);
	long tlid;
	short dfcc;
	//char cfcc[4];
	XY_t* pts;
	int nPts;
	Range2D mbr;
	XY_t sPt,
			 ePt;
	/*GeoPoint min;
	GeoPoint max;
	GeoPoint sPt,
					 ePt;*/
	PolyNode* sNode,
					* eNode;
	void* info;

	int GetPts(XY_t pts[]);

	void* operator new(size_t);
	void operator delete(void*);

private:
	static BlockMem memBlock;
};

class LineTable : public HashTable
{
  public :
		LineTable( unsigned size = 1 << 12 );
		~LineTable( void );
		void Flush( void );
	
  private :
		int Equal( void *object, SLL *link );
};
