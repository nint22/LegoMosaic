/***

 LegoBitmap - Converts BMP into a Lego Mosaic
 Copyright (c) 2014 Jeremy Bridon

 Main application entry point; uses the cheap
 1x1 to 2x6 bricks, ranging in price from 10-
 cents to 40-cents each.
 
 Bricks are defined through data in a given
 text-file via the command line argument. It'll
 start with a number for colors, three rows of
 these colors (space delimited 0-255 values per channel).
 This is followed by a number for bricks. Each brick
 has a width and height and cost (in pennies).

***/

#include "LegoBitmap.h"
#include "LegoSet.h"

int main( int argc, const char * argv[] )
{
    // Must have a brick def. file name
    if( argc != 3 )
    {
        printf( "Example usage: ./LegoBitmap BrickDefinitions.txt Input.bmp\n" );
        return 0;
    }
    
    FILE* file = fopen( argv[1], "r" );
    if( file == NULL )
    {
        printf( "Error: Unable to open the given file \"%s\"\n", argv[1] );
        return 0;
    }
    
    // Read in brick colors count
    int colorsCount = 0;
    if( fscanf( file, "%d", &colorsCount ) != 1 )
    {
        printf( "Error: No brick colors count found\n" );
        return 0;
    }

    // Read in all RGB colors for bricks
    BrickColorList brickColors;
    for( int i = 0; i < colorsCount; i++ )
    {
        int r = 0, g = 0, b = 0;
        fscanf( file, "%d %d %d", &r, &g, &b );
        brickColors.push_back( BrickColor() );
    }
    
    // Read number of bricks
    int brickCount = 0;
    if( fscanf( file, "%d", &brickCount ) != 1 )
    {
        printf( "Error: No brick structure count found\n" );
        return 0;
    }
    
    // Read all brick shapes and cost
	BrickDefinitionList brickDefinitions;
    for( int i = 0; i < colorsCount; i++ )
    {
        int w = 0, h = 0, c = 0;
        fscanf( file, "%d %d %d", &w, &h, &c );
        brickDefinitions.push_back( BrickDefinition( i, Vec2( w, h ), c ) );
    }
    
	// Load the given image
	LegoBitmap legoBitmap( brickDefinitions, brickColors );
	legoBitmap.Solve( argv[2] );

	return 0;
}
