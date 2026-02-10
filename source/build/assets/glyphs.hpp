#pragma once

#include <core/types.hpp>
#include <core/list.hpp>
#include <core/buffer.hpp>
#include <core/string.hpp>

#include <build/cache.hpp>
#include <build/textures.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Glyph
{
public:
	String texturePath;
	Texture2DBuffer textureBuffer;

	u16 atlasX1, atlasY1;
	u16 atlasX2, atlasY2;

	u16 imageX1, imageY1;
	u16 imageX2, imageY2;

	u16 u1, v1;
	u16 u2, v2;

	CacheKey cacheKey;
};

using GlyphID = u32;
#define GLYPHID_MAX ( U32_MAX )
#define GLYPHID_NULL ( GLYPHID_MAX )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Glyphs
{
public:
	GlyphID register_new( const Glyph &glyph = Glyph { } );
	GlyphID register_new( Glyph &&glyph );

	void build();

public:
	List<Glyph> glyphs;
	Glyph &operator[]( GlyphID id ) { return glyphs[id]; }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////