/***

 LegoBitmap - Converts BMP into a Lego Mosaic
 Copyright (c) 2014 Jeremy Bridon

 Description: Reads from the command-line parameters two
 file paths. The first is a file containing a list of brick
 shapes, costs, and colors. The second is the image that
 you want to convert. The image must be a BMP, with full
 alpha on pixels that you do not want to mosaic.
 
 The brick definitions file is a plain-text file that starts
 with a number for colors, as an integer, where each rows has
 three RGB values (space-delimited, values of 0 to 255 inclusive).
 This is followed by a number for bricks. Each brick has a width
 and height and cost (in pennies); these values are space-delimited.
 Bricks that cost more than a dollar should still be written in
 pennies: e.g. a $1.25 brick is just 125 (pennies).

***/

#include <vector>
#include "LegoMosaic.h"

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
    std::vector< char* > brickColorNames;
    BrickColorList brickColors;
    for( int i = 0; i < colorsCount; i++ )
    {
        char nameBuffer[ 512 ] = "";
        int r = 0, g = 0, b = 0;
        fscanf( file, "%s %d %d %d", nameBuffer, &r, &g, &b );
        
        BrickColor color;
        LegoBitmap::ConvertColor( r, g, b, 255, color);
        
        brickColorNames.push_back( strdup( nameBuffer ) );
        brickColors.push_back( color );
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
    for( int i = 0; i < brickCount; i++ )
    {
        int w = 0, h = 0, c = 0;
        fscanf( file, "%d %d %d", &w, &h, &c );
        brickDefinitions.push_back( BrickDefinition( i, Vec2( w, h ), c ) );
    }
    
	// Load the given image
	LegoMosaic legoMosaic( brickDefinitions, brickColors );
	legoMosaic.Solve( argv[2] );

    // Print the solution set's data
    legoMosaic.PrintSolution( brickColorNames );
    
    // Release the strdup'ed strings
    for( int i = 0; i < (int)brickColorNames.size(); i++ )
    {
        delete brickColorNames[ i ];
    }
    
	return 0;
}
