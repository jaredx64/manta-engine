#pragma once

#include <core/types.hpp>
#include <core/list.hpp>
#include <core/buffer.hpp>
#include <core/string.hpp>

#include <build/assets/textures.hpp>
#include <build/assets/glyphs.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Sprite
{
public:
	TextureID textureID;
	GlyphID glyphID;
	u16 count;
	u16 width;
	u16 height;
	i16 xorigin;
	i16 yorigin;

	String name;
};

using SpriteID = u32;
#define SPRITEID_MAX ( U32_MAX )
#define SPRITEID_NULL ( SPRITEID_MAX )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Sprites
{
public:
	SpriteID register_new( const Sprite &sprite = Sprite { } );
	SpriteID register_new( Sprite &&sprite );
	SpriteID register_new_from_definition( String name, const char *path );

	usize gather( const char *path, bool recurse = true );
	void build();

public:
	List<Sprite> sprites;
	Sprite &operator[]( u32 spriteID ) { return sprites[spriteID]; }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////