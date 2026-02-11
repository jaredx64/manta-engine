#pragma once

#include <core/types.hpp>
#include <core/debug.hpp>
#include <core/memory.hpp>
#include <core/color.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace CoreTextureIO
{
	struct TextureBuffer
	{
		byte *data = nullptr;
		u8 pixelFormat = ColorFormat_RGBA8;
		u8 pixelSize = 4;
		u16 width = 0;
		u16 height = 0;
		u16 depth = 0;
	};
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Texture2DBuffer
{
public:
	Texture2DBuffer() : data { nullptr }, width { 0 }, height { 0 } { }

public:
	void init( u16 width, u16 height );
	void free();
	void copy( const Texture2DBuffer &other );
	void move( Texture2DBuffer &&other );

	bool is_initialized() const { return data != nullptr; }

	bool save( const char *path );
	bool load( const char *path );

	void clear( Color color );

	void splice( Texture2DBuffer &source, u16 srcX1, u16 srcY1,
	    u16 srcX2, u16 srcY2, u16 dstX, u16 dstY );

	inline void splice( Texture2DBuffer &source, u16 dstX, u16 dstY )
	{
		splice( source, 0, 0, source.width, source.height, dstX, dstY );
	}

	inline Color &at( u32 index )
	{
		Assert( index < static_cast<u32>( width ) * static_cast<u32>( height ) );
		return data[index];
	}

	inline Color &at( u16 x, u16 y )
	{
		Assert( x < width && y < height );
		return data[ y * width + x ];
	}

	inline void set( u32 index, Color value )
	{
		Assert( index < static_cast<u32>( width ) * static_cast<u32>( height ) );
		data[index] = value;
	}

	inline void set( u16 x, u16 y, Color value )
	{
		Assert( x < width && y < height );
		data[ y * width + x ] = value;
	}

	inline Color &operator[]( u32 index ) { return at( index ); }
	explicit operator bool() const { return data != nullptr; }

public:
	Color *data = nullptr;
	u16 width;
	u16 height;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////