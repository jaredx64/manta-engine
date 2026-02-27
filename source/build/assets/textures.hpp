#pragma once

#include <core/types.hpp>
#include <core/assets.hpp>
#include <core/list.hpp>
#include <core/buffer.hpp>
#include <core/string.hpp>

#include <build/assets/glyphs.hpp>
#include <build/textures.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Texture
{
public:
	GlyphID add_glyph( const Glyph &glyph );
	GlyphID add_glyph( Glyph &&glyph );
	void pack();

public:
	bool atlasTexture = true;
	bool generateMips = false;
	usize offset;
	u16 width = 0;
	u16 height = 0;
	u16 depth = 0;
	u16 layers = 0;
	u16 levels = 0;
	Assets::TextureColorFormat format = Assets::TextureColorFormat_R8G8B8A8;
	List<GlyphID> glyphs;

	List<CacheKey> glyphCacheKey; // Glyph dependencies
	String name;
};

using TextureID = u16;
#define TEXTUREID_MAX ( U16_MAX )
#define TEXTUREID_NULL ( TEXTUREID_MAX )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Textures
{
public:
	TextureID register_new( String &name );
	TextureID register_new_from_definition( String name, const char *path );
	TextureID register_new_from_file( String &name, const char *path, bool generateMips );

	usize gather( const char *path, bool recurse = true );
	void build();

public:
	List<Texture> textures;
	Texture &operator[]( TextureID id ) { return textures[id]; }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////