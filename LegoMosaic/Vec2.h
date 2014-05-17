/***

 LegoBitmap - Converts BMP into a Lego Mosaic
 Copyright (c) 2014 Jeremy Bridon

 Description: Simple integer tuple; used for position
 and size.

***/

#ifndef __VEC2_H__
#define __VEC2_H__
#pragma once

class Vec2
{
public:
	
	Vec2()
		: x( 0 )
		, y( 0 )
	{
	}
	
	Vec2( int nx, int ny )
		: x( nx )
		, y( ny )
	{
	}

	Vec2( const Vec2& src )
		: x( src.x )
		, y( src.y )
	{
	}

	int& At( int i )
	{
		// Faster to add than to compare
		return *(&(x) + i);
	}

	int x, y;
};

typedef std::vector< Vec2 > Vec2List;

#endif // __VEC2_H__
