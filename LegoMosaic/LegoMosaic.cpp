/***
 
 LegoBitmap - Converts BMP into a Lego Mosaic
 Copyright (c) 2014 Jeremy Bridon
 
***/

#include "LegoMosaic.h"

#include <deque>

LegoMosaic::LegoMosaic( const BrickDefinitionList& brickDefinitions, const BrickColorList& brickColors )
    : m_brickDefinitions( brickDefinitions )
    , m_brickColors( brickColors )
    , m_solutionSet( NULL )
{
    
}

LegoMosaic::~LegoMosaic()
{
    delete m_solutionSet;
}

void LegoMosaic::Solve( const char* fileName, bool useBruteForce )
{
    // 1. Load the image
    LegoBitmap legoBitmap( fileName );
    if( legoBitmap.ConvertMosaic( m_brickColors ) == false )
    {
        printf( "Unable to convert the given file \"%s\" to the given Lego colors\n", fileName ? fileName : NULL );
    }
    legoBitmap.SavePng( "LegoMosaicProgress_Output.png", m_brickColors );
    
    m_boardSize = legoBitmap.GetBoardSize();
    
    BrickList brickList;
    m_solutionSet = new LegoSet( m_boardSize, brickList, m_brickDefinitions );
    
    // 2a. A* searching algorithm
    if( !useBruteForce )
    {
        // Empty starting state
        LegoSet legoSet( m_boardSize, brickList, m_brickDefinitions );
        
        // While not solved...
        while( !IsSolved( legoSet, legoBitmap ) )
        {
            Vec2List nextPositions = GetNextPositions( legoSet, legoBitmap );
            
            // Search this breadth; arbitrary (bad) initial rank
            int bestDefinitionIndex = -1;
            Vec2 bestPosition;
            float bestRank = 9999999.0f;
            
            // For each 1. Position, 2. Brick type
            // TODO: 3. brick orientation (not just horizontal vs. vertical, but rotate on corner)
            // Note that the color isn't searched; we just sample the position
            for( int posIndex = 0; posIndex < (int)nextPositions.size(); posIndex++ )
            {
                const Vec2& nextPosition = nextPositions[ posIndex ];
                
                for( int defIndex = 0; defIndex < (int)m_brickDefinitions.size(); defIndex++ )
                {
                    int colorIndex = legoBitmap.GetBrickColorIndex( nextPosition );
                    Brick testBrick( defIndex, colorIndex, nextPosition );
                    LegoSet testSet( legoSet );
                    
                    // If valid position *and* has a better rank...
                    if( testSet.AddBrick( testBrick, m_brickDefinitions, legoBitmap ) )
                    {
                        float newRank = testSet.GetRank();
                        if( newRank < bestRank )
                        {
                            bestRank = newRank;
                            bestPosition = nextPosition;
                            bestDefinitionIndex = defIndex;
                        }
                    }
                }
            }
            
            // Copy over the best, if any found, else it's a critical error (unsolvable)
            if( bestDefinitionIndex >= 0 )
            {
                // Add it to the solution set!
                int colorIndex = legoBitmap.GetBrickColorIndex( bestPosition );
                Brick brick( bestDefinitionIndex, colorIndex, bestPosition );
                
                legoSet.AddBrick( brick, m_brickDefinitions, legoBitmap );
                *m_solutionSet = legoSet;
                
                // Show progress: write it out to memory
                int searchDepth = (int)legoSet.GetBrickList().size();
                
                char fileName[ 512 ];
                sprintf( fileName, "LegoMosaicProgress_%05d.png", searchDepth );
                legoBitmap.SavePng( fileName, m_brickDefinitions, m_brickColors, legoSet );
                
                if( legoBitmap.GetMosaicPegCount() > 0 )
                {
                    printf( "Progress: %%%.2f, at search depth %d\n", ( float( legoSet.GetPlacedPegCount() ) / float( legoBitmap.GetMosaicPegCount() ) * 100.0f ), searchDepth );
                }
            }
            else
            {
                printf( "Critical error: unable to place a brick into an unsolved set\n" );
                exit( 0 );
            }
        }
        
    }
    
    // 2b. Breadth-first exhaustive search
    else
    {
        std::deque< LegoSet > workingQueue;
        std::vector< LegoSet > solutionList;
        
        // Start with empty base case
        LegoSet legoSet( m_boardSize, brickList, m_brickDefinitions );
        workingQueue.push_back( legoSet );
        
        uint64_t searchStepCount = 0;
        
        while( !workingQueue.empty() )
        {
            // Pop off this lego set and grow it
            LegoSet legoSet = workingQueue.front();
            workingQueue.pop_front();
            
            Vec2List nextPositions = GetNextPositions( legoSet, legoBitmap, searchStepCount > 0 );
            
            // For each 1. Position, 2. Brick type
            // Note that the color isn't searched; we just sample the position
            for( int posIndex = 0; posIndex < (int)nextPositions.size(); posIndex++ )
            {
                const Vec2& nextPosition = nextPositions[ posIndex ];
                
                for( int defIndex = 0; defIndex < (int)m_brickDefinitions.size(); defIndex++ )
                {
                    int colorIndex = legoBitmap.GetBrickColorIndex( nextPosition );
                    Brick testBrick( defIndex, colorIndex, nextPosition );
                    LegoSet testSet( legoSet );
                    
                    // If valid position, put into queue for further work
                    searchStepCount++;
                    if( testSet.AddBrick( testBrick, m_brickDefinitions, legoBitmap ) )
                    {
                        if( legoBitmap.GetMosaicPegCount() > 0 )
                        {
                            printf( "Progress: %%%.2f, at search depth %d, search count %lld\n", ( float( testSet.GetPlacedPegCount() ) / float( legoBitmap.GetMosaicPegCount() ) ) * 100.0f, (int)testSet.GetBrickList().size(), searchStepCount );
                        }
                        
                        // If valid solution, save it, else push back to queue
                        if( IsSolved( testSet, legoBitmap ) )
                        {
                            solutionList.push_back( testSet );
                            
                            printf( "Found a solution; brick-count: %d, cost: $%d.%d\n", (int)testSet.GetBrickList().size(), testSet.GetCost(), testSet.GetCost() % 100 );
                            
                            // Draw out this solution; so we can track which solution ID maps to output
                            char fileName[ 512 ];
                            sprintf( fileName, "LegoMosaicProgress_%05d.png", searchStepCount );
                            legoBitmap.SavePng( fileName, m_brickDefinitions, m_brickColors, testSet );
                        }
                        else
                        {
                            workingQueue.push_back( testSet );
                        }
                    }
                }
            }
        }
        
        // Find best solution based only on price
        int bestSolutionIndex = -1;
        int bestSolutionCost = 999999999;
        
        for( int i = 0; i < (int)solutionList.size(); i++ )
        {
            int cost = solutionList[ i ].GetCost();
            if( cost < bestSolutionCost )
            {
                bestSolutionCost = cost;
                bestSolutionIndex = i;
            }
        }
        
        if( bestSolutionIndex >= 0 )
        {
            *m_solutionSet = solutionList[ bestSolutionIndex ];
        }
        else
        {
            printf( "Critical error: unable to place a brick into an unsolved set\n" );
            exit( 0 );
        }
    }
    
    // 3. Print parts list, with price
    
}

void LegoMosaic::PrintSolution( const std::vector< char* > brickColorNames )
{
    const int colorCount = (int)m_brickColors.size();
    const int brickDefCount = (int)m_brickDefinitions.size();
    
    // Parts count: partsList[ colorIndex ][ brick defintion index ] = count
    std::vector< std::vector< int > > partsList;
    partsList.resize( colorCount );
    for( int i = 0; i < colorCount; i++ )
    {
        partsList[ i ].resize( brickDefCount, 0 );
    }
    
    // Grab parts list
    const BrickList& brickList = m_solutionSet->GetBrickList();
    for( int i = 0; i < (int)brickList.size(); i++ )
    {
        int colorId = brickList[ i ].m_colorId;
        int brickId = brickList[ i ].m_definitionId;
        
        partsList[ colorId ][ brickId ]++;
    }
    
    // Print parts in color order, then in parts order
    for( int i = 0; i < colorCount; i++ )
    {
        // Check if it has data first
        int colorPartCount = 0;
        for( int j = 0; j < brickDefCount; j++ )
        {
            colorPartCount += partsList[ i ][ j ];
        }
        
        // Skip on no data
        if( colorPartCount <= 0 )
        {
            printf( "Color \"%s\" is unused\n", brickColorNames[ i ] );
            continue;
        }
        
        printf( "Color \"%s\" has %d parts:\n", brickColorNames[ i ], colorPartCount );
        
        // Print the parts
        for( int j = 0; j < brickDefCount; j++ )
        {
            if( partsList[ i ][ j ] > 0 )
            {
                Vec2 partSize = m_brickDefinitions[ j ].m_shape;
                int cost = m_brickDefinitions[ j ].m_cost;
                printf( "\t%d needed for part #%d ( %d x %d, %d cents per unit )\n", partsList[ i ][ j ], j, partSize.x, partSize.y, cost );
            }
        }
    }
    
    // Print total cost
    printf( "> Total bricks: %d\n", (int)brickList.size() );
    printf( "> Total cost: $%d.%d\n", m_solutionSet->GetCost() / 100, m_solutionSet->GetCost() % 100 );
}

Vec2List LegoMosaic::GetNextPositions( const LegoSet& legoSet, const LegoBitmap& legoBitmap, bool onlyAppend  )
{
    // This is a bit expensive: basically we're doing edge-detection, where a pixel on an unplaced peg,
    // if it is directly adjacent to a placed lego piece *or* image edge, is pushed to this list and returned
    
	// Basically do edge-detection
	Vec2List edgePositions;
	
	// Up, down, left, right offsets
	static const Vec2 cOffsets[ 4 ] =
    {
        Vec2( 0, -1 ),
        Vec2( 0, 1 ),
        Vec2(-1, 0 ),
        Vec2( 1, 0 ),
	};
    
    // For each peg on the board
	IterateBoard( [&](Vec2 pos)
        {
            // Ignore if the current spot is already occupied in the set *or* is an invalid color
            if( legoSet.IsPegOccupied( pos ) || legoBitmap.GetBrickColorIndex( pos ) < 0 )
            {
                return;
            }
            
            // Now check the neighboring positions for these states...
            for( int i = 0; i < 4; i++ )
            {
                Vec2 adjPos( pos.x + cOffsets[ i ].x, pos.y + cOffsets[ i ].y );
                
                bool inBoard = adjPos.x >= 0 && adjPos.y >= 0 && adjPos.x < m_boardSize.x && adjPos.y < m_boardSize.y;
                bool pegOccupied = inBoard && legoSet.IsPegOccupied( adjPos );
                bool pegHasColor = inBoard && legoBitmap.GetBrickColorIndex( adjPos ) < 0;
                
                // Only test if adjacent to other Lego bricks
                if( onlyAppend && pegOccupied )
                {
                    // Save and stop searching
                    edgePositions.push_back( pos );
                    return;
                }
                
                // Only test if adjacent to other Lego bricks or next to an empty pixel
                else if( !onlyAppend && ( !inBoard || pegOccupied || pegHasColor ) )
                {
                    // Save and stop searching
                    edgePositions.push_back( pos );
                    return;
                }
            }
        }
    );
    
	return edgePositions;
}

bool LegoMosaic::IsSolved( const LegoSet& legoSet, const LegoBitmap& legoBitmap )
{
    bool isFilled = (legoSet.GetBrickList().size() > 0);
    
    IterateBoard( [&](Vec2 pos)
        {
            // If there is a color and it isn't occupied by a lego piece, flag as bad
            if( legoBitmap.GetBrickColorIndex( pos ) > 0 && legoSet.IsPegOccupied( pos ) == false )
            {
                isFilled = false;
            }
        }
    );
    
    return isFilled;
}

void LegoMosaic::IterateBoard( std::function< void(Vec2) > func )
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
