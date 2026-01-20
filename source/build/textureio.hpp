#pragma once

#include <core/types.hpp>
#include <core/debug.hpp>
#include <core/memory.hpp>

#include <build/color.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Texture2DBuffer
{
public:
	Texture2DBuffer() : data( nullptr ), width( 0 ), height( 0 ) { }
	Texture2DBuffer( u16 width, u16 height ) { init( width, height ); }
	Texture2DBuffer( const char *path ) { load( path ); }
	Texture2DBuffer( const Texture2DBuffer &other ) { copy( other ); }
	Texture2DBuffer( Texture2DBuffer &&other ) { move( static_cast<Texture2DBuffer &&>( other ) ); }
	~Texture2DBuffer() { free(); }

	Texture2DBuffer &operator=( const Texture2DBuffer &other )
		{ copy( other ); return *this; }
	Texture2DBuffer &operator=( Texture2DBuffer &&other )
		{ move( static_cast<Texture2DBuffer &&>( other ) ); return *this; }

public:
	void init( u16 width, u16 height );
	void free();
	void copy( const Texture2DBuffer &other );
	void move( Texture2DBuffer &&other );

	bool save( const char *path );
	bool load( const char *path );

	void clear( rgba color );

	void splice( Texture2DBuffer &source, u16 srcX1, u16 srcY1, u16 srcX2, u16 srcY2, u16 dstX, u16 dstY );

	void splice( Texture2DBuffer &source, u16 dstX, u16 dstY )
	{
		splice( source, 0, 0, source.width, source.height, dstX, dstY );
	}

	rgba &at( u32 index )
	{
		Assert( index < static_cast<u32>( width ) * static_cast<u32>( height ) );
		return data[index];
	}

	rgba &at( u16 x, u16 y )
	{
		Assert( x < width && y < height );
		return data[ y * width + x ];
	}

	void set( u32 index, rgba value )
	{
		Assert( index < static_cast<u32>( width ) * static_cast<u32>( height ) );
		data[index] = value;
	}

	void set( u16 x, u16 y, rgba value )
	{
		Assert( x < width && y < height );
		data[ y * width + x ] = value;
	}

	rgba &operator[]( u32 index ) { return at( index ); }
	explicit operator bool() const { return data != nullptr; }

public:
	rgba *data = nullptr;
	u16 width;
	u16 height;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern bool png_to_ico( const char *pathPNG, const char *pathICO );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////