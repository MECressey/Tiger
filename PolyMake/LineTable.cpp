//#include "stdafx.h"

#include <string.h>
#include "linetable.hpp"

BlockMem GeoLine::memBlock(sizeof(GeoLine), 1000);

GeoLine::GeoLine(void)
{
  this->tlid = 0;
  this->dfcc = 0;
  this->nPts = 0;
  this->pts = 0;
  //  this->area = 0.0;
  this->sNode = this->eNode = 0;
  this->info = 0;
}

GeoLine::~GeoLine(void)
{
  if (this->pts != 0)
    delete[] this->pts;

  /*if( this->info != 0 )
      delete [] this->info;*/
}

int GeoLine::GetPts(XY_t pts[])
{
  pts[0] = this->sNode->pt;
  pts[this->nPts - 1] = this->eNode->pt;

  if (this->nPts > 2)
  {
    memcpy(&pts[1], this->pts, (this->nPts - 2) * sizeof(XY_t));
  }

  return(this->nPts);
}

void* GeoLine::operator new(size_t)
{
  return(memBlock.Alloc());
}

void GeoLine::operator delete(void* obj)
{
  memBlock.Free(obj);
}
LineTable::LineTable( unsigned size ) : HashTable( size )
{
}

LineTable::~LineTable( void )
{
  this->Flush();
}

void LineTable::Flush( void )
{
  for( unsigned i = 0; i < this->MAX_HASH; i++ )
  {
    if( this->table[ i ].empty() )
      continue;

    SLL *prev = this->table[ i ].initGetNext();
		SLL *next = prev->getNext();
		do
		{
      GeoLine* curr = (GeoLine*)next;;
			next = next->getNext();
			curr->delink( prev );
			delete curr;
		}
    while( ! next->end() );
  }
  this->nItems = 0;
}

int LineTable::Equal( void *object, SLL *link )
{
//	long id = (long)object;
  GeoLine* line = (GeoLine *)link;

  if (line->tlid == *(long*)object)
    return(1);

	return( 0 );
}
