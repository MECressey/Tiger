#include <afx.h>

#include "polynode.hpp"

PolyNode::PolyNode(  const XY_t &node )
{
  this->pt = node;
  this->count = 0;
  for( int i = 0; i < sizeof( this->lines ) / sizeof( lines[0] ); i++ )
  {
		this->lines[ i ] = 0;
		this->angles[ i ] = 0.0;
  }
}

PolyNode::~PolyNode( void )
{
}

void PolyNode::Insert( GeoLine *line/*long tlid*/, double angle )
{
  int i;

  for( i = 0; i < this->count; i++ )
  {
		if( this->angles[ i ] > angle )
		{
		  break;  
		}
  }

  for( int j = this->count; j > i; --j )
  {
		this->lines[ j ] = this->lines[ j - 1 ];
//	this->ids[ j ] = this->ids[ j - 1 ];
		this->angles[ j ] = this->angles[ j - 1 ];
  }

  ASSERT( i < sizeof( this->lines ) / sizeof( this->lines[0] ) );
  this->lines[ i ] = line;
//  this->ids[ i ] = tlid;
  this->angles[ i ] = angle;

//  ASSERT( this->count <= sizeof( this->lines ) / sizeof( this->lines[0] ) );
  this->count++;
}

BlockMem PolyNode::memBlock( sizeof( PolyNode ), 1000 );

void *PolyNode::operator new( size_t )
{
  return( memBlock.Alloc() );
}

void PolyNode::operator delete( void *obj )
{
  memBlock.Free( obj );
}
