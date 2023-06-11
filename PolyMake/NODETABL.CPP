#include <afx.h>

#include "nodetabl.hpp"
#include "polynode.hpp"

NodeTable::NodeTable( unsigned size ) : HashTable( size )
{
}

NodeTable::~NodeTable( void )
{
  this->Flush();
}

void NodeTable::Flush( void )
{
  for( unsigned i = 0; i < this->MAX_HASH; i++ )
  {
    if( this->table[ i ].empty() )
      continue;

    SLL *prev = this->table[ i ].initGetNext();
	SLL *next = prev->getNext();
	do
	{
	  PolyNode *curr = (PolyNode *)next;
	  next = next->getNext();
	  curr->delink( prev );
	  delete curr;
	}
    while( ! next->end() );
  }
  this->nItems = 0;
}

int NodeTable::Equal( void *object, SLL *link )
{
	XY_t *gPt = (XY_t *)object;
	PolyNode *gNode = (PolyNode *)link;
	
	if( gPt->y == gNode->pt.y && gPt->x == gNode->pt.x )
	  return( 1 );
	
	return( 0 );
}

