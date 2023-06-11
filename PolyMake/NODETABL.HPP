#pragma once

#include "hashtabl.hpp"

class NodeTable : public HashTable
{
  public :
	NodeTable( unsigned size = 1 << 16 );
	~NodeTable( void );
	void Flush( void );
	
  private :
	int Equal( void *object, SLL *link );
};
