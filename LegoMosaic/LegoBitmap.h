/***

 LegoBitmap - Converts BMP into a Lego Mosaic
 Copyright (c) 2014 Jeremy Bridon

 Class Description:

 A lego-image solver, optimizing for least
 pieces, using an A* algorithm. The heuristic
 is the cost of sum lego pieces + .

***/

#ifndef __LEGOBITMAP_H__
#define __LEGOBITMAP_H__
#pragma once

#include <map>

#include <stdlib.h>
#include <math.h>
#include <climits>

#include <vector>
#include <algorithm>

#include "LegoSet.h"
#include "EasyBMP.h"

// Utilities
namespace
{
	inline RGBApixel BrickColorToRGBApixel( const BrickColor& brickColor )
	{
		RGBApixel pixel;
		
		pixel.Red = ( brickColor >> 16 ) & 0xFF;
		pixel.Green = ( brickColor >> 8 ) & 0xFF;
		pixel.Blue = ( brickColor >> 0 ) & 0xFF;
		pixel.Alpha = 255;

		return pixel;
	}

	bool solutionSortFunc( LegoSet* setA, LegoSet* setB )
	{
		return setA->Score() < setB->Score();
	}

	inline int Abs( int a )
	{
		return (a < 0) ? a : -a;
	}
}

class LegoBitmap
{
public:

	// Define a set of lego pieces and image file-name you're trying to mosaic-solve
	LegoBitmap( const BrickDefinitionList& brickDefinitions, const BrickColorList& brickColors );
	~LegoBitmap();

	// Attempts to solve, will do some helpful printf's
	void Solve( const char* sourceImageFileName );

protected:

	// Recursively searches with A*; returns true when a solution is found
	bool Solve();
	
	// Save current state image to file name
	void SaveBitmap( const char* fileName, const LegoSet& legoSet, int tileSize = 8 );

	// Executes over the 2D given array size
	void IterateBoard( std::function< void(Vec2) > func );

	// Itterate around the given brick instance, but only within bounds
	void IterateBrickOutline( const LegoSet& solutionSet, const Brick& brick, std::function< void(Vec2) > func );

	// Given a bitmap color, try to find the best color in the given list; returns -1 on failure
	int MatchBitmapColorToBrickColor( BrickColor givenColor );

	Vec2 m_boardSize;
	BrickColorIndexList m_boardColors;

	BrickColorList m_brickColors;
	BrickDefinitionList m_brickDefinitions;
};

LegoBitmap::LegoBitmap( const BrickDefinitionList& brickDefinitions, const BrickColorList& brickColors )
	: m_boardSize( 0, 0 )
	, m_brickColors( brickColors )
	, m_brickDefinitions( brickDefinitions )
{

}

LegoBitmap::~LegoBitmap()
{
	// ...
}

void LegoBitmap::Solve( const char* sourceImageFileName )
{
	// Load image if possible
	BMP srcImage;
	if( !srcImage.ReadFromFile( sourceImageFileName ) )
	{
		printf( "Unable to load given image!\n" );
		return;
	}
	m_boardSize = Vec2( srcImage.TellWidth(), srcImage.TellHeight() );

	// Save to color map, where it's a 2D array of color indices
	const int elementCount = m_boardSize.x * m_boardSize.y;
	m_boardColors.resize( elementCount );

	// Create an "out" image to show the color simplification
	BMP dstImage;
	dstImage.SetSize( m_boardSize.x, m_boardSize.y );
	dstImage.SetBitDepth( 3 * 8 ); // 3 channels, 8 bits each
	
	// Used to help start initial search
	bool firstPixelPosSet = false;
	Vec2 firstPixelPos( 0, 0 );

	// For each pixel, color-match
	IterateBoard( [&](Vec2 pos)
	{
		// Convert image to color index
		RGBApixel srcColor = srcImage.GetPixel( pos.x, pos.y );
		int bestColorIndex = MatchBitmapColorToBrickColor( RGB8ToUint32( srcColor.Red, srcColor.Green, srcColor.Blue ) );
		
		// Not yet saved, and not black color
		if( !firstPixelPosSet && bestColorIndex != 0 )
		{
			firstPixelPosSet = true;
			firstPixelPos = pos;
		}

		// Save to internal buffer
		m_boardColors[ pos.y * m_boardSize.x + pos.x ] = bestColorIndex;

		// Save to out image
		dstImage.SetPixel( pos.x, pos.y, BrickColorToRGBApixel( m_brickColors.at( bestColorIndex ) ) );

	} );

	if( !firstPixelPosSet )
	{
		printf( "Unable to find starting non-black pixel\n" );
		return;
	}

	// Save image
	dstImage.WriteToFile( "MosaicResult.bmp" );

	/*** Searching ***/

	// Initial set is empty, we then recursively search by
	// placing every brick combination for the next level (not empty spaces,
	// but always adjacent to existinb brick locations)

	LegoSet searchHead( m_boardSize, m_brickDefinitions, BrickList(), m_boardColors );
	
	for( int depth = 0; depth < INT_MAX; depth++ )
	{
		// Active working set
		printf( "Searching at depth #%d, %d bricks used\n", depth, searchHead.GetBrickList().size() );

		// Note that we start at the top-left most colored pixel if first iteration
		std::vector< Vec2 > nextPositions;
		if( depth == 0 )
		{
			nextPositions.push_back( firstPixelPos );
		}
		else
		{
			nextPositions = searchHead.GetNextPositions();
		}

		int nextPositionCount = nextPositions.size();
		nextPositionCount = nextPositionCount > 5 ? 5 : nextPositionCount;

		// Our best local set
		LegoSet legoSet( m_boardSize, m_brickDefinitions, searchHead.GetBrickList(), m_boardColors );

		// For each brick type
		bool isValid = false; // Has a valid local solution
		const int brickDefCount = m_brickDefinitions.size();
		for( int i = 0; i < brickDefCount; i++ )
		{
			// For each position
			for( int j = 0; j < nextPositionCount; j++ )
			{
				// Can we add this brick donfiguration and are we cheaper? If so, replace solution
				LegoSet testSet( searchHead );
				Brick nextBrick( i, nextPositions.at( j ) );

				if( testSet.AddBrick( nextBrick ) && ( isValid == false || testSet.Score() > legoSet.Score() ) )
				{
					isValid = true;
					legoSet = testSet;
				}
			}
		}

		// If we never got set, we cannot back-track (could implement dfs)
		if( isValid == false )
		{
			printf( "Unable to back-track!\n" );
			return;
		}

		// Look for next best set
		searchHead = legoSet;
		
		char fileName[ 2048 ];
		sprintf( fileName, "Depth%d.bmp", depth );
		SaveBitmap( fileName, searchHead );

		printf( "Best fill rate: %.1f%%\n", searchHead.GetFillPercentage() * 100.0f );
		
		// Unless.. we've found a solution!
		if( searchHead.IsSolved() )
		{
			int cost = searchHead.Cost();
			printf( "Found solution! Piece count %d, cost $%d.%02d\n", searchHead.GetBrickList().size(), cost / 100, cost % 100 );
			return;
		}
	}

	// Broke out of loop
	return;
}

void LegoBitmap::SaveBitmap( const char* fileName, const LegoSet& legoSet, int tileSize )
{
	// Open bmp to write out to, clearing all to white
	// For each brick we have, write black outline and fill with color
	BMP dstImage;
	dstImage.SetSize( m_boardSize.x * tileSize, m_boardSize.y * tileSize );
	dstImage.SetBitDepth( 3 * 8 );

	// Define black and white for drawing
	RGBApixel blackPixel;
	blackPixel.Red = blackPixel.Green = blackPixel.Blue = 0;
	blackPixel.Alpha = 255;

	RGBApixel whitePixel;
	whitePixel.Red = whitePixel.Green = whitePixel.Blue = 255;
	whitePixel.Alpha = 255;

	// Fill black
	IterateBoard( [&](Vec2 pos)
	{
		for( int dx = 0; dx < tileSize; dx++ )
		{
			for( int dy = 0; dy < tileSize; dy++ )
			{
				dstImage.SetPixel( pos.x * tileSize + dx, pos.y * tileSize + dy, blackPixel );
			}
		}

	} );

	// For each brick
	const int brickCount = legoSet.GetBrickList().size();
	for( int i = 0; i < brickCount; i++ )
	{
		Brick brick = legoSet.GetBrickList().at( i );
		BrickDefinition& brickDef = m_brickDefinitions.at( brick.m_definitionId );

		int startX = brick.m_position.x;
		int endX = brick.m_position.x + brickDef.m_shape.x;
		
		int startY = brick.m_position.y;
		int endY = brick.m_position.y + brickDef.m_shape.y;

		// For each peg on the brick
		for( int y = startY; y < endY; y++ )
		{
			for( int x = startX; x < endX; x++ )
			{
				// Unique color per brick
				int colorIndex = m_boardColors[ y * m_boardSize.x + x ];

				// Ignore if black
				if( colorIndex == 0 ) continue;

				// Fill with best color
				RGBApixel pixelColor = BrickColorToRGBApixel( m_brickColors.at( colorIndex ) );

				// Fill brick
				for( int dx = 0; dx < tileSize; dx++ )
				{
					for( int dy = 0; dy < tileSize; dy++ )
					{
						Vec2 pos( x * tileSize + dx, y * tileSize + dy );
						bool isEdge = ( ( pos.x == startX * tileSize ) || ( pos.y == startY * tileSize ) || ( pos.x == endX * tileSize - 1 ) || ( pos.y == endY * tileSize - 1 ) );

						dstImage.SetPixel( pos.x, pos.y, isEdge ? whitePixel : pixelColor );
					}
				}

			}
		}
		// End of peg-iteration
	}

	// Save out
	dstImage.WriteToFile( fileName );
}

void LegoBitmap::IterateBoard( std::function< void(Vec2) > func )
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

void LegoBitmap::IterateBrickOutline( const LegoSet& solutionSet, const Brick& brick, std::function< void(Vec2) > func )
{
	// Start and end positions
	Vec2 pos = brick.m_position;
	pos.x -= 1;
	pos.y -= 1;

	Vec2 size = m_brickDefinitions.at( brick.m_definitionId ).m_shape;
	size.x += 2;
	size.y += 2;

	Vec2 end = Vec2( pos.x + size.x, pos.y + size.y );

	// Go through the positions
	for( int y = pos.y; y < end.y; y++ )
	{
		for( int x = pos.x; x < end.x; x++ )
		{
			// Must be in-bounds
			if( x < 0 || x >= m_boardSize.x || y < 0 || y >= m_boardSize.y )
				continue;

			// Must not intersect existing piece
			if( solutionSet.HasPeg( Vec2( x, y ) ) )
				continue;

			// Only call on brick outline
			if( x == 0 || x == (end.x - 1) || y == 0 || y == (end.y - 1) )
				func( Vec2( x, y ) );
		}
	}
}

int LegoBitmap::MatchBitmapColorToBrickColor( BrickColor givenColor )
{
    // "Best" color is lowest "Distance-sum" value
    int bestMatch = 255 * 3; // Start worst-case
    int bestMatchIndex = -1;

	const int cBrickColorCount = m_brickColors.size();
    for( int i = 0; i < cBrickColorCount; i++ )
    {
        BrickColor testColor = m_brickColors.at( i );
        int rank = 0;
        for( int j = 0; j < 3; j++ )
        {
			// Rank is difference *per channel*
            rank += Abs( ( ( testColor >> 16 ) & 0xFF ) - ( ( givenColor >> 16 ) & 0xFF ) );
            rank += Abs( ( ( testColor >> 8  ) & 0xFF ) - ( ( givenColor >> 8  ) & 0xFF ) );
            rank += Abs( ( ( testColor >> 0  ) & 0xFF ) - ( ( givenColor >> 0  ) & 0xFF ) );
        }

        if( rank < bestMatch )
        {
            bestMatch = rank;
            bestMatchIndex = i;
        }
    }

    return bestMatchIndex;
}

#endif //__LEGOBITMAP_H__
