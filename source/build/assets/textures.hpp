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

	String name;
	usize offset;
	u16 width = 0;
	u16 height = 0;
	u16 levels = 0;

	bool atlasTexture = true;
	bool generateMips = false;
	List<GlyphID> glyphs;
	GlyphID add_glyph( Texture2DBuffer &&textureBuffer );
	void pack();
};

using TextureID = u16;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Textures
{
	TextureID make_new( String &name );
	TextureID make_new( String &name, Texture2DBuffer &&textureBuffer );

	void gather( const char *path, const bool recurse = true );
	void load( const char *path );
	void write();

	Texture &operator[]( const TextureID id ) { return textures[id]; }

	List<Texture> textures;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////