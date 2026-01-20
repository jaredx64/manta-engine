#pragma once

#include <core/types.hpp>
#include <core/list.hpp>
#include <core/buffer.hpp>
#include <core/string.hpp>

#include <build/assets/textures.hpp>
#include <build/assets/glyphs.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Sprite
{
	TextureID textureID;
	GlyphID glyphID;
	u16 count;
	u16 width;
	u16 height;
	i16 xorigin;
	i16 yorigin;
	String name;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Sprites
{
	void make_new( const Sprite &sprite );

	usize gather( const char *path, bool recurse = true );
	void process( const char *path );
	void build();

	Sprite &operator[]( u32 spriteID ) { return sprites[spriteID]; }

	List<Sprite> sprites;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////