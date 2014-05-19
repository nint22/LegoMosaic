/***

 LegoBitmap - Converts BMP into a Lego Mosaic
 Copyright (c) 2014 Jeremy Bridon

 Description: Set of definitions of pieces, sets,
 helpful typedefs, etc.

***/

#ifndef __LEGOSET_H__
#define __LEGOSET_H__
#pragma once

#include <vector>

#include <stdint.h>
#include <functional>

#include "Vec2.h"

class LegoBitmap;

// Brick definition is just a definition ID (indexes into given list), shape, and cost
struct BrickDefinition
{
	// ID must be unique!
	BrickDefinition( int definitionId, const Vec2& shape, int cost )
		: m_definitionId( definitionId )
		, m_shape( shape )
		, m_cost( cost )
	{
	}

	// Copy constructor
	BrickDefinition( const BrickDefinition& src )
		: m_definitionId( src.m_definitionId )
		, m_shape( src.m_shape )
		, m_cost( src.m_cost )
	{
	}

	int m_definitionId;
	Vec2 m_shape;
	int m_cost;         // Always in pennies!
};
typedef std::vector< BrickDefinition > BrickDefinitionList;

// An instance of a brick: it has a shape (definitionID), a color (colorID), and placement position (position)
struct Brick
{
	Brick( int definitionId, int colorId, const Vec2& position )
		: m_definitionId( definitionId )
        , m_colorId( colorId )
        , m_position( position )
	{
	}

	Vec2 m_position;
	int m_definitionId;
    int m_colorId;
};
typedef std::vector< Brick > BrickList;

// A set of pieces that can be tested for solution, collision, etc.
// Note that to save memory usage, we only keep indexes into the brick definition and color list
class LegoSet
{
public:

    // Note that the brick definitions is not copied; just used for cost setup
	LegoSet( const Vec2& boardSize, const BrickList& bricks, const BrickDefinitionList& brickDefinitions );
	LegoSet( const LegoSet& legoSet );
	~LegoSet();
	
    // Attempt adding a brick; will return false if unable to add brick (out of bounds, bad color, etc.)
	bool AddBrick( const Brick& brick, const BrickDefinitionList& brickDefinitions, const LegoBitmap& legoBitmap );
    
	// Get copy of brick-list
	const BrickList& GetBrickList() const { return m_brickList; }

    // Return true / false on the occupancy state
    bool IsPegOccupied( const Vec2& pos ) const;
    
	// Cost of the brick list in pennies
	int GetCost() const { return m_cost; }
    float GetRank() const { return float( m_cost ) / float( m_pegCount ); }
    float GetPlacedPegCount() const { return m_pegCount; }
    
protected:
    
	// Executes over the 2D given array size, or iterate over the brick's pegs
	void IterateBrick( const Vec2& pos, const Vec2& size, std::function< void(Vec2) > func );
    
private:
	
	Vec2 m_boardSize;
	BrickList m_brickList;
    
	// 2D map of all bricks; int maps to m_brickList objects, indexed by 2D map value
	std::vector< bool > m_boardOccupancy;
    
	// Cached states
	int m_cost;
    int m_pegCount;
};

#endif // __LEGOSET_H__
