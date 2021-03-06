/***
 
 LegoBitmap - Converts BMP into a Lego Mosaic
 Copyright (c) 2014 Jeremy Bridon

***/

#include "LegoBitmap.h"

#include "lodepng.h"

// Dither Bayer matrix from wikipedia:
// en.wikipedia.org/wiki/Ordered_dithering
namespace
{
    const float cDitherDivFactor = 128.0f;
    const float cDitherMatrix[ 8 ][ 8 ] = {
        { 1.0f / cDitherDivFactor, 49.0f / cDitherDivFactor, 13.0f / cDitherDivFactor, 61.0f / cDitherDivFactor,  4.0f / cDitherDivFactor, 52.0f / cDitherDivFactor, 16.0f / cDitherDivFactor, 64.0f / cDitherDivFactor, },
        { 33.0f / cDitherDivFactor, 17.0f / cDitherDivFactor, 45.0f / cDitherDivFactor, 29.0f / cDitherDivFactor, 36.0f / cDitherDivFactor, 20.0f / cDitherDivFactor, 48.0f / cDitherDivFactor, 32.0f / cDitherDivFactor, },
        { 9.0f / cDitherDivFactor, 57.0f / cDitherDivFactor,  5.0f / cDitherDivFactor, 53.0f / cDitherDivFactor, 12.0f / cDitherDivFactor, 60.0f / cDitherDivFactor,  8.0f / cDitherDivFactor, 56.0f / cDitherDivFactor, },
        { 41.0f / cDitherDivFactor, 25.0f / cDitherDivFactor, 37.0f / cDitherDivFactor, 21.0f / cDitherDivFactor, 44.0f / cDitherDivFactor, 28.0f / cDitherDivFactor, 40.0f / cDitherDivFactor, 24.0f / cDitherDivFactor, },
        { 3.0f / cDitherDivFactor, 51.0f / cDitherDivFactor, 15.0f / cDitherDivFactor, 63.0f / cDitherDivFactor,  2.0f / cDitherDivFactor, 50.0f / cDitherDivFactor, 14.0f / cDitherDivFactor, 62.0f / cDitherDivFactor, },
        { 35.0f / cDitherDivFactor, 19.0f / cDitherDivFactor, 47.0f / cDitherDivFactor, 31.0f / cDitherDivFactor, 34.0f / cDitherDivFactor, 18.0f / cDitherDivFactor, 46.0f / cDitherDivFactor, 30.0f / cDitherDivFactor, },
        { 11.0f / cDitherDivFactor, 59.0f / cDitherDivFactor,  7.0f / cDitherDivFactor, 55.0f / cDitherDivFactor, 10.0f / cDitherDivFactor, 58.0f / cDitherDivFactor,  6.0f / cDitherDivFactor, 54.0f / cDitherDivFactor, },
        { 43.0f / cDitherDivFactor, 27.0f / cDitherDivFactor, 39.0f / cDitherDivFactor, 23.0f / cDitherDivFactor, 42.0f / cDitherDivFactor, 26.0f / cDitherDivFactor, 38.0f / cDitherDivFactor, 22.0f / cDitherDivFactor, },
    };
}

LegoBitmap::LegoBitmap( const char* fileName )
    : m_boardSize( 0, 0 )
    , m_validPegs( 0 )
{
    unsigned int width;
    unsigned int height;
    std::vector< unsigned char > pngBuffer;
    
    if( lodepng::decode( pngBuffer, width, height, fileName ) != 0 )
    {
        printf( "Unable to load the given file \"%s\"; lodepng::decode(...) failed\n", fileName ? fileName : NULL );
        return;
    }
    
    m_boardSize = Vec2( width, height );
    
    // Convert to packed-buffer array
    for( int i = 0; i < pngBuffer.size(); i += 4 )
    {
        int r = (int)pngBuffer.at( i + 0 );
        int g = (int)pngBuffer.at( i + 1 );
        int b = (int)pngBuffer.at( i + 2 );
        int a = (int)pngBuffer.at( i + 3 );
        
        BrickColor brickColor;
        ConvertColor( r, g, b, a, brickColor );
        m_pngBuffer.push_back( brickColor );
    }
}

LegoBitmap::LegoBitmap( const LegoBitmap& legoBitmap )
{
    m_boardSize = legoBitmap.m_boardSize;
    m_pngBuffer = legoBitmap.m_pngBuffer;
    m_colorIndices = legoBitmap.m_colorIndices;
    m_validPegs = legoBitmap.m_validPegs;
}

LegoBitmap::~LegoBitmap()
{
	// ...
}

bool LegoBitmap::ConvertMosaic( const BrickColorList& brickColorList, bool dither )
{
    if( m_pngBuffer.size() <= 0 )
    {
        return false;
    }
    
    // Allocate the board-colors map; defaults buffer values to -1 (no color)
    m_colorIndices.resize( m_boardSize.x * m_boardSize.y );
    
	// For each pixel, color-match
	IterateBoard( [&](Vec2 pos)
        {
            // Convert image to color index
            BrickColor color = m_pngBuffer[ pos.y * m_boardSize.x + pos.x ];
            int bestColorIndex = MatchColorToColorIndex( brickColorList, color );
            
            if( bestColorIndex >= 0 )
            {
                m_validPegs++;
                
                // Dither if needed
                if( dither )
                {
                    BrickColor brickColor = brickColorList.at( bestColorIndex );
                    DitherColor( pos, brickColor );
                    
                    // Re-map to the best color
                    bestColorIndex = MatchColorToColorIndex( brickColorList, brickColor );
                }
            }
            
            // Save to internal buffer if non-zero
            int pegIndex = pos.y * m_boardSize.x + pos.x;
            m_colorIndices[ pegIndex ] = bestColorIndex;
        }
    );
    
    return true;
}

const BrickColor& LegoBitmap::GetBrickColor( const Vec2& pegPos ) const
{
    static const BrickColor cNoColor = 0x00000000;
    
    int pegIndex = pegPos.y * m_boardSize.x + pegPos.x;
    if( pegIndex >= 0 && pegIndex < (int)m_pngBuffer.size() )
    {
        return m_pngBuffer[ pegIndex ];
    }
    else
    {
        return cNoColor;
    };
}

int LegoBitmap::GetBrickColorIndex( const Vec2& pegPos ) const
{
    int pegIndex = pegPos.y * m_boardSize.x + pegPos.x;
    if( pegIndex >= 0 && pegIndex < (int)m_colorIndices.size() )
    {
        return m_colorIndices[ pegIndex ];
    }
    else
    {
        return -1;
    };
}

void LegoBitmap::SavePng( const char* fileName, const BrickColorList& brickColorList ) const
{
    // Pack as RGBA buffer
    std::vector< unsigned char > pngBuffer;
	IterateBoard( [&](Vec2 pos)
        {
            int pegIndex = pos.y * m_boardSize.x + pos.x;
            int colorIndex = m_colorIndices[ pegIndex ];
            int r, g, b, a;
            
            if( colorIndex >= 0 )
            {
                const BrickColor& brickColor = brickColorList[ colorIndex ];
                ConvertColor( brickColor, &r, &g, &b, &a );
            }
            else
            {
                r = g = b = a = 0;
            }
            
            pngBuffer.push_back( r );
            pngBuffer.push_back( g );
            pngBuffer.push_back( b );
            pngBuffer.push_back( a );
        }
    );
    
    if( lodepng::encode( fileName, pngBuffer, m_boardSize.x, m_boardSize.y ) != 0 )
    {
        printf( "Saving to \"%s\" failed!\n", fileName );
    }
}

void LegoBitmap::SavePng( const char* fileName, const BrickDefinitionList& brickDefinitions, const BrickColorList& brickColors, const LegoSet& legoSet, int tileSize ) const
{
    // Prepare RGBA buffer for direct writing
    std::vector< unsigned char > pngBuffer;
    pngBuffer.resize( m_boardSize.x * tileSize * m_boardSize.y * tileSize * 4 );
    
	// Fill all alpha
	IterateBoard( [&](Vec2 pos)
        {
            for( int dx = 0; dx < tileSize; dx++ )
            {
                for( int dy = 0; dy < tileSize; dy++ )
                {
                    for( int channel = 0; channel < 4; channel++ )
                    {
                        int x = pos.x * tileSize + dx;
                        int y = pos.y * tileSize + dy;
                        int pegIndex = y * m_boardSize.x * tileSize * 4 + x * 4 + channel;
                        pngBuffer[ pegIndex ] = 0;
                    }
                }
            }
        }
    );
    
	// For each brick
	const int brickCount = (int)legoSet.GetBrickList().size();
	for( int i = 0; i < brickCount; i++ )
	{
		Brick brick = legoSet.GetBrickList().at( i );
		const BrickDefinition& brickDef = brickDefinitions.at( brick.m_definitionId );
        const BrickColor& brickColor = brickColors.at( brick.m_colorId );
        
        int r, g, b, a;
        ConvertColor( brickColor, &r, &g, &b, &a );
        
        int edgeR = std::min( r + 25, 255 );
        int edgeG = std::min( g + 25, 255 );
        int edgeB = std::min( b + 25, 255 );
        
		int startX = brick.m_position.x;
		int endX = brick.m_position.x + brickDef.m_shape.x;
		
		int startY = brick.m_position.y;
		int endY = brick.m_position.y + brickDef.m_shape.y;
        
		// For each peg on the brick
		for( int y = startY; y < endY; y++ )
		{
			for( int x = startX; x < endX; x++ )
			{
				// Fill brick
				for( int dx = 0; dx < tileSize; dx++ )
				{
					for( int dy = 0; dy < tileSize; dy++ )
					{
                        // If on edge, draw more white (round up channel)
						Vec2 pos( x * tileSize + dx, y * tileSize + dy );
                        bool isEdge = ( ( pos.x == startX * tileSize ) || ( pos.y == startY * tileSize ) || ( pos.x == endX * tileSize - 1 ) || ( pos.y == endY * tileSize - 1 ) );
                        
                        int pegIndex = pos.y * m_boardSize.x * tileSize * 4 + pos.x * 4;
                        
                        pngBuffer[ pegIndex + 0 ] = isEdge ? edgeR : r;
                        pngBuffer[ pegIndex + 1 ] = isEdge ? edgeG : g;
                        pngBuffer[ pegIndex + 2 ] = isEdge ? edgeB : b;
                        pngBuffer[ pegIndex + 3 ] = 0xFF; // Full alpha, so it's visible
					}
				}
			}
		}
	}
    
    // Done drawing, write out
    if( lodepng::encode( fileName, pngBuffer, m_boardSize.x * tileSize, m_boardSize.y * tileSize ) != 0 )
    {
        printf( "Saving to \"%s\" failed!\n", fileName );
    }
}

void LegoBitmap::ConvertColor( int r, int g, int b, int a, BrickColor& dst )
{
    dst = ( a << 24 ) | ( r << 16 ) | ( g << 8 ) | ( b << 0 );
}

void LegoBitmap::ConvertColor( const BrickColor& src, int* rOut, int* gOut, int* bOut, int* aOut )
{
    if( aOut != NULL ) *aOut = ( src >> 24 ) & 0xFF;
    if( rOut != NULL ) *rOut = ( src >> 16 ) & 0xFF;
    if( gOut != NULL ) *gOut = ( src >>  8 ) & 0xFF;
    if( bOut != NULL ) *bOut = ( src >>  0 ) & 0xFF;
}

int LegoBitmap::MatchColorToColorIndex( const BrickColorList& brickColors, const BrickColor& givenColor )
{
    // Note: Even though there are more correct ways (e.g. functions based on human-eye
    // perceptions), we're keeping it to euclidian dist for simplicity's sake
    // http://en.wikipedia.org/wiki/Color_difference#CIE94
    
    int r, g, b, a;
    ConvertColor( givenColor, &r, &g, &b, &a );
    
    // Ignore if color is not full-alpha
    if( a != 255 )
    {
        return -1;
    }
    
    // Best match while searching
    int bestRank = 9999999;
    int bestMatchIndex = -1;
    
	int count = (int)brickColors.size();
    for( int i = 0; i < count; i++ )
    {
        const BrickColor& color = brickColors.at( i );
        
        int tr, tg, tb;
        ConvertColor( color, &tr, &tg, &tb, NULL );
        
        // Euclid dist
        int colorRank = abs( r - tr )
                      + abs( g - tg )
                      + abs( b - tb );
        
        if( colorRank < bestRank )
        {
            bestRank = colorRank;
            bestMatchIndex = i;
        }
    }
    
    return bestMatchIndex;
}

void LegoBitmap::IterateBoard( std::function< void(Vec2) > func ) const
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

void LegoBitmap::DitherColor( const Vec2& pos, BrickColor& colorInOut )
{
    // Convert to float
    int r, g, b, a;
    ConvertColor( colorInOut, &r, &g, &b, &a );
    
    float fr = float( r ) / 255.0f;
    float fg = float( g ) / 255.0f;
    float fb = float( b ) / 255.0f;
    
    // Add threshold
    fr = fr - fr * cDitherMatrix[ pos.x % 8 ][ pos.y % 8 ];
    fg = fg - fg * cDitherMatrix[ pos.x % 8 ][ pos.y % 8 ];
    fb = fb - fb * cDitherMatrix[ pos.x % 8 ][ pos.y % 8 ];
    
    // Convert back to bytes
    r = int( fr * 255.0f );
    g = int( fg * 255.0f );
    b = int( fb * 255.0f );
    
    ConvertColor( r, g, b, a, colorInOut );
}
