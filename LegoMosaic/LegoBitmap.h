/***

 LegoBitmap - Converts BMP into a Lego Mosaic
 Copyright (c) 2014 Jeremy Bridon

 Description: A lego-image solver, optimizing for least
 pieces, using an A* algorithm. The heuristic is the total-
 cost of lego pieces.
 
 Though this class is called "LegoBitmap", it is written
 for PNG file format suppoert; BMP references are legacy
 comments.

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

// A color is just a simple hex
typedef uint32_t BrickColor;
typedef std::vector< BrickColor > BrickColorList;

// Easy to query mosaic-converted bitmap image
class LegoBitmap
{
public:

	// Define a set of lego pieces and image file-name you're trying to mosaic-solve
	LegoBitmap( const char* fileName );
    LegoBitmap( const LegoBitmap& legoBitmap );
	~LegoBitmap();
    
    const Vec2& GetBoardSize() const { return m_boardSize; }
    
    // Converts pixel buffer to best-matched mosaic colors; return false on failure (no image loaded, no colors, etc.)
    bool ConvertMosaic( const BrickColorList& brickColorList );
    
    // Get the brick color index at the given; returns -1 on alpha or when not yet converted to mosaic
    const BrickColor& GetBrickColor( const Vec2& pegPos ) const;
    int GetBrickColorIndex( const Vec2& pegPos ) const;
    
    // Save current image *.png to file; can draw in special format for debugging
    void SavePng( const char* fileName, const BrickColorList& brickColorList ) const;
	void SavePng( const char* fileName, const BrickDefinitionList& brickDefinitions, const BrickColorList& brickColors, const LegoSet& legoSet, int tileSize = 5 ) const;
    
    // Set of helpful color conversion functions
    static void ConvertColor( int r, int g, int b, int a, BrickColor& dst );
    static void ConvertColor( const BrickColor& src, int* rOut, int* gOut, int* bOut, int* aOut );
    
protected:
    
	// Given a bitmap color, try to find the best color in the given list; returns -1 on failure
	int MatchColorToColorIndex( const BrickColorList& brickColors, const BrickColor& givenColor );
    
    // Helpful for drawing / pixel parsing
    void IterateBoard( std::function< void(Vec2) > func ) const;
    
private:
    
    // Width x Height (in pixels)
    Vec2 m_boardSize;
    
    // The PNG image, saved in a temporary color buffer, byte-order ARGB
    // Indexing is linear: m_pngBuffer[ y * width + x ]
    // Note that both arrays are parallel, though the m_colorIndices maps to the given brickColorList
    std::vector< BrickColor > m_pngBuffer;
    std::vector< int > m_colorIndices;
    
};

#endif //__LEGOBITMAP_H__
