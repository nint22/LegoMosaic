/***
 
 LegoBitmap - Converts BMP into a Lego Mosaic
 Copyright (c) 2014 Jeremy Bridon
 
***/

#include "LegoSet.h"

#include "LegoBitmap.h"

LegoSet::LegoSet( const Vec2& boardSize, const BrickList& bricks, const BrickDefinitionList& brickDefinitions )
    : m_boardSize( boardSize )
    , m_brickList( bricks )
    , m_cost( 0 )
    , m_pegCount( 0 )
{
	// Allocate needed map, default to un-filled
	m_boardOccupancy.resize( m_boardSize.x * m_boardSize.y, false );
    
	// Add given bricks, don't do a deep copy since we need to setup the board
	const int brickCount = (int)bricks.size();
	for( int i = 0; i < brickCount; i++ )
	{
        const Brick& brick = m_brickList[ i ];
        const BrickDefinition& brickDefinition = brickDefinitions[ brick.m_definitionId ];
        IterateBrick( m_brickList[ i ].m_position, brickDefinition.m_shape, [&](Vec2 pos)
            {
                int pegIndex = pos.y * m_boardSize.x + pos.x;
                m_boardOccupancy[ pegIndex ] = true;
            }
        );
	}
}

LegoSet::LegoSet( const LegoSet& legoSet )
{
	m_boardSize = legoSet.m_boardSize;
	m_brickList = legoSet.m_brickList;
	m_boardOccupancy = legoSet.m_boardOccupancy;
	m_cost = legoSet.m_cost;
    m_pegCount = legoSet.m_pegCount;
}

LegoSet::~LegoSet()
{
	// ...
}

bool LegoSet::AddBrick( const Brick& brick, const BrickDefinitionList& brickDefinitions, const LegoBitmap& legoBitmap )
{
    // Get brick size
    const BrickDefinition& brickDefinition = brickDefinitions[ brick.m_definitionId ];
    Vec2 brickSize = brickDefinition.m_shape;
    
    // Simple bounds check
    if( brick.m_position.x < 0 || brick.m_position.y < 0 ||
        ( ( brick.m_position.x + brickSize.x - 1 ) >= m_boardSize.x ) ||
        ( ( brick.m_position.y + brickSize.y - 1 ) >= m_boardSize.y ) )
    {
        return false;
    }
    
	// Initial color index
	int brickColorIndex = brick.m_colorId;
	if( brickColorIndex < 0 )
    {
        return false;
    }
    
	// Color and bounds matching
	bool fail = false;
	IterateBrick( brick.m_position, brickSize, [&](Vec2 pos)
        {
            // Color must match; if we go out of bounds, the color is -1 (so we can fail out early)
            // Must also not intersect existing bricks
            bool alreadyOccupied = IsPegOccupied( pos );
            bool differentColor = legoBitmap.GetBrickColorIndex( pos ) != brickColorIndex;
            fail |= alreadyOccupied || differentColor; // Note the "|=", very important!
        }
    );
	
	if( fail )
	{
		return false;
	}
    
	// Good to place, just append to list, and write to buffer
	m_brickList.push_back( brick );
    
	// Optimization (cache it now to quickly test later)
	m_cost += brickDefinition.m_cost;
    m_pegCount += brickDefinition.m_shape.x * brickDefinition.m_shape.y;
	
	IterateBrick( brick.m_position, brickSize, [&](Vec2 pos)
        {
            if( m_boardOccupancy[ pos.y * m_boardSize.x + pos.x ] == true )
                printf( "Inconsistency problem!!" );
            m_boardOccupancy[ pos.y * m_boardSize.x + pos.x ] = true;
        }
    );
    
	// All done!
	return true;
}

bool LegoSet::IsPegOccupied( const Vec2& pos ) const
{
    int pegIndex = pos.y * m_boardSize.x + pos.x;
    return m_boardOccupancy[ pegIndex ];
}

void LegoSet::IterateBrick( const Vec2& pos, const Vec2& size, std::function< void(Vec2) > func )
{
	// Go through the brick size
	for( int y = 0; y < size.y; y++ )
	{
		for( int x = 0; x < size.x; x++ )
		{
			Vec2 funcPos( pos.x + x, pos.y + y );
			if( funcPos.x >= 0 && funcPos.y >= 0 && funcPos.x < m_boardSize.x && funcPos.y < m_boardSize.y )
			{
				func( funcPos );
			}
		}
	}
}
