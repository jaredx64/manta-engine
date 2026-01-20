#pragma once

#include <core/types.hpp>
#include <core/list.hpp>
#include <core/buffer.hpp>
#include <core/string.hpp>

#include <build/assets/glyphs.hpp>
#include <build/textureio.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Texture
{
	Texture( String name ) : name( name ) { }

	List<u32> cacheIDs; // Glyph dependencies

	String name;
	usize offset;
	u16 width = 0;
	u16 height = 0;
	u16 levels = 0;

	bool atlasTexture = true;
	bool generateMips = false;

	List<GlyphID> glyphs;

	GlyphID add_glyph( const Glyph &glyph );
	GlyphID add_glyph( Glyph &&glyph );
	void pack();
};

using TextureID = u16;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Textures
{
	TextureID make_new( String &name );

	usize gather( const char *path, bool recurse = true );
	void process( const char *path );
	void build();

	Texture &operator[]( TextureID id ) { return textures[id]; }

	List<Texture> textures;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////