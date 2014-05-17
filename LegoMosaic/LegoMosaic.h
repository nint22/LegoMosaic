/***
 
 LegoBitmap - Converts BMP into a Lego Mosaic
 Copyright (c) 2014 Jeremy Bridon
 
 Description: Solves for a given image, converting it first into
 a mosaic image that matches (based on euclidian distance of each
 pixel's RGB value compared to the given list of RGB brick-color
 valies), then fills the set using a non-recursive A* search algorithm.
 Though the solving function is not recursive, the internal data
 structures allow stepping back to solve for global correctness.
 You can explicitly turn this on (it's essentially a global brute-
 force strategy) by passing true as a second argument to the
 Solve function.
 
 ***/

#ifndef __LEGOMOSAIC_H__
#define __LEGOMOSAIC_H__

#include "LegoBitmap.h"
#include "LegoSet.h"

class LegoMosaic
{
    
public:
    
    // Constructor takes list of brick definitions (brick sizes and cost) and brick colors (just hex values)
    LegoMosaic( const BrickDefinitionList& brickDefinitions, const BrickColorList& brickColors );
    ~LegoMosaic();
    
    // Solve, doing an A* search algorithm
    void Solve( const char* fileName, bool useBruteForce = false );
    
    // Print the purchase order / parts list
    void PrintSolution( const std::vector< char* > brickColorNames );
    
protected:
    
    // Return a list of positions that are on the edge of placed lego pieces or image edge
    Vec2List GetNextPositions( const LegoSet& legoSet, const LegoBitmap& legoBitmap );
    
    // Returns true if all colors are covered by bricks
    bool IsSolved( const LegoSet& legoSet, const LegoBitmap& legoBitmap );
    
    // Helpful for drawing / pixel parsing
    void IterateBoard( std::function< void(Vec2) > func );
    
private:
    
    BrickDefinitionList m_brickDefinitions;
    BrickColorList m_brickColors;
    
    Vec2 m_boardSize;
    Vec2List m_legalPositions;
    
    LegoSet* m_solutionSet;
    
};

#endif // __LEGOMOSAIC_H__
