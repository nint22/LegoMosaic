/***

 LegoBitmap - Converts BMP into a Lego Mosaic
 Copyright (c) 2014 Jeremy Bridon

 Class Description:
 
 Set of definitions of pieces, sets, helpful
 typedefs, etc.

***/

#ifndef __LEGOSET_H__
#define __LEGOSET_H__
#pragma once

#include <vector>
using namespace std;

#include <stdint.h>
#include <functional>

#include "Vec2.h"

#include "EasyBMP.h"

// Forward declarations
class LegoSet;

// A color is just a simple hex
typedef uint32_t BrickColor;

// Utility converter
inline BrickColor RGB8ToUint32( int r, int g, int b )
{
	return ( ( r & 0xFF ) << 16 ) | ( ( g & 0xFF ) << 8 ) | ( ( b & 0xFF ) << 0 );
}

// A lego peice; color, shape, and cost
struct BrickDefinition
{
	// ID must be unique!
	BrickDefinition( int id, const Vec2& shape, int cost )
		: m_id( id )
		, m_shape( shape )
		, m_cost( cost )
	{
	}

	// Copy constructor
	BrickDefinition( const BrickDefinition& src )
		: m_id( src.m_id )
		, m_shape( src.m_shape )
		, m_cost( src.m_cost )
	{
	}

	int m_id;
	Vec2 m_shape;
	int m_cost; // Pennies
};

// An instance of a brick that is in a position; points to brick definition
struct Brick
{
	Brick( int definitionId, const Vec2& position )
		: m_definitionId( definitionId )
		, m_position( position )
	{
	}

	Vec2 m_position;
	int m_definitionId;
};

// Helpful containers commonly used
typedef vector< BrickColor > BrickColorList;
typedef vector< BrickDefinition > BrickDefinitionList;
typedef vector< Brick > BrickList;
typedef std::vector< LegoSet* > LegoSetList;
typedef std::vector< int > BrickColorIndexList;

// A set of pieces that can be tested for solution, collision, etc.
class LegoSet
{
public:

	// Copy constructor is faster; will grab cached variables
	LegoSet( const Vec2& boardSize, const BrickDefinitionList& brickDefinitions, const BrickList& bricks, const BrickColorIndexList& boardColors );
	LegoSet( const LegoSet& legoSet );
	~LegoSet();
	
	// Add or remove a piece; returns true on success, false when out of bounds or collision
	bool AddBrick( const Brick& brick );

	// Compute valid positions that are on brick-empty positions
	std::vector< Vec2 > GetNextPositions();

	// Get copy of brick-list
	const BrickList& GetBrickList() const { return m_brickList; }

	// Returns true if board is filled and solved
	bool IsSolved();
	float GetFillPercentage();

	// Cost of the brick list
	int Cost() { return m_cost; }
	int Score() { return 100000 - (int)m_brickList.size() + m_pegCount * 10; } // Higher is better;

	// Return true if there is already a piece here
	// No bounds-checking!
	bool HasPeg( const Vec2& pos ) const;
	
	// Executes over the 2D given array size, or iterate over the brick's pegs
	void IterateBoard( std::function< void(Vec2) > func );
	void IterateBrick( const Brick& brick, std::function< void(Vec2) > func );

protected:
	
	Vec2 m_boardSize;
	BrickDefinitionList m_brickDefinitions;
	BrickList m_brickList;

	// 2D map of all bricks; int maps to m_brickList objects, indexed by 2D map value
	std::vector< bool > m_boardOccupancy;
	BrickColorIndexList m_boardColors;

	// Cached states
	int m_cost;
	int m_pegCount;
};

LegoSet::LegoSet( const Vec2& boardSize, const BrickDefinitionList& brickDefinitions, const BrickList& bricks, const BrickColorIndexList& boardColors )
	: m_boardSize( boardSize )
	, m_brickDefinitions( brickDefinitions )
	, m_boardColors( boardColors )
	, m_cost( 0 )
	, m_pegCount( 0 )
{
	// Allocate needed map
	m_boardOccupancy.resize( m_boardSize.x * m_boardSize.y, false );

	// Add given bricks, don't do a deep copy since we need to setup the board
	const int brickCount = (int)bricks.size();
	for( int i = 0; i < brickCount; i++ )
	{
		AddBrick( bricks.at( i ) );
	}
}

LegoSet::LegoSet( const LegoSet& legoSet )
{
	m_brickDefinitions = legoSet.m_brickDefinitions;
	m_brickList = legoSet.m_brickList;
	m_boardSize = legoSet.m_boardSize;
	m_boardOccupancy = legoSet.m_boardOccupancy;
	m_boardColors = legoSet.m_boardColors;
	m_cost = 0;
	m_pegCount = 0;
	
	const int brickCount = (int)m_brickList.size();
	for( int i = 0; i < brickCount; i++ )
	{
		BrickDefinition& brickDef = m_brickDefinitions.at( m_brickList.at( i ).m_definitionId );
		m_cost += brickDef.m_cost;
		m_pegCount += brickDef.m_shape.x * brickDef.m_shape.y;
	}
}

LegoSet::~LegoSet()
{
	// ...
}

bool LegoSet::AddBrick( const Brick& brick )
{
	// Initial color index
	int brickColor = -1;
	if( brick.m_position.x >= 0 && brick.m_position.y >= 0 && brick.m_position.x < m_boardSize.x && brick.m_position.y < m_boardSize.y )
	{
		brickColor = m_boardColors[ brick.m_position.y * m_boardSize.x + brick.m_position.x ];
	}
	else
	{
		return false;
	}

	// Collision and bounds checking
	bool fail = false;
	IterateBrick( brick, [&](Vec2 pos)
	{
		int pegColor = m_boardColors[ pos.y * m_boardSize.x + pos.x ];

		// If out of bounds, fail
		if( pos.x < 0 || pos.y < 0 || pos.x >= m_boardSize.x || pos.y >= m_boardSize.y )
		{
			fail = true;
		}
		// Peg must be zero
		else if( m_boardOccupancy[ pos.y * m_boardSize.x + pos.x ] )
		{
			fail = true;
		}
		// Peg must be same color
		else if( brickColor != pegColor )
		{
			fail = true;
		}

	} );
	
	if( fail )
	{
		return false;
	}

	// Good to place, just append to list, and write to buffer
	BrickDefinition brickDefinition = m_brickDefinitions.at( brick.m_definitionId );
	m_brickList.push_back( brick );

	// Optimization (cache it now to quickly test later)
	m_cost += brickDefinition.m_cost;
	m_pegCount += brickDefinition.m_shape.x * brickDefinition.m_shape.y;

	IterateBrick( brick, [&](Vec2 pos)
	{
		m_boardOccupancy[ pos.y * m_boardSize.x + pos.x ] = true;

	} );

	// All done!
	return true;
}

std::vector< Vec2 > LegoSet::GetNextPositions()
{
	// Basically do edge-detection
	std::vector< Vec2 > edges;
	
	// Special case: if board is empty, start at top-left
	if( m_brickList.size() <= 0 )
	{
		edges.push_back( Vec2( 0, 0 ) );
		return edges;
	}

	// Up,down,left,right
	static const Vec2 cOffsets[ 4 ] = {
			Vec2( 0, -1 ),
			Vec2( 0, 1 ),
			Vec2(-1, 0 ),
			Vec2( 1, 0 ),
	};

	IterateBoard( [&](Vec2 pos)
	{
		bool isOccupied = m_boardOccupancy[ pos.y * m_boardSize.x + pos.x ];

		// If the directly adjacent positions are different
		// than my position, queue for next position
		for( int i = 0; i < 4; i++ )
		{
			Vec2 testPos( pos.x + cOffsets[ i ].x, pos.y + cOffsets[ i ].y );

			// Bounds check
			if( testPos.x >= 0 && testPos.y >= 0 && testPos.x < m_boardSize.x && testPos.y < m_boardSize.y )
			{
				// Queue to list if different than center
				bool isTestOccupied = m_boardOccupancy[ testPos.y * m_boardSize.x + testPos.x ];
				if( ( isOccupied == false ) && ( isOccupied != isTestOccupied ) )
				{
					edges.push_back( pos );
					return; // Stop searching
				}
			}
		}

	} );

	return edges;
}

bool LegoSet::IsSolved()
{
	// Optimization: just have to make sure the peg-count is equal to fill-count
	return bool( m_pegCount >= ( m_boardSize.x * m_boardSize.y ) );
}

float LegoSet::GetFillPercentage()
{
	return float( m_pegCount ) / float( m_boardSize.x * m_boardSize.y );
}

bool LegoSet::HasPeg( const Vec2& pos ) const
{
	return m_boardOccupancy[ pos.y * m_boardSize.x + pos.x ];
}

void LegoSet::IterateBoard( std::function< void(Vec2) > func )
{
	// Go through the array
	for( int y = 0; y < m_boardSize.y; y++ )
	{
		for( int x = 0; x < m_boardSize.x; x++ )
		{
			func( Vec2( x, y ) );
		}
	}
}

void LegoSet::IterateBrick( const Brick& brick, std::function< void(Vec2) > func )
{
	// Go through the brick size
	BrickDefinition brickDefinition = m_brickDefinitions.at( brick.m_definitionId );
	for( int y = 0; y < brickDefinition.m_shape.y; y++ )
	{
		for( int x = 0; x < brickDefinition.m_shape.x; x++ )
		{
			Vec2 pos( brick.m_position.x + x, brick.m_position.y + y );
			if( pos.x >= 0 && pos.y >= 0 && pos.x < m_boardSize.x && pos.y < m_boardSize.y )
			{
				func( pos );
			}
		}
	}
}

#endif // __LEGOSET_H__
