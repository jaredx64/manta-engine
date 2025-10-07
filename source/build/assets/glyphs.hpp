#pragma once

#include <core/types.hpp>
#include <core/list.hpp>
#include <core/buffer.hpp>
#include <core/string.hpp>

#include <build/cache.hpp>
#include <build/textureio.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Glyph
{
	CacheID cacheID;
	String texturePath;
	Texture2DBuffer textureBuffer;

	u16 atlasX1, atlasY1;
	u16 atlasX2, atlasY2;

	u16 imageX1, imageY1;
	u16 imageX2, imageY2;

	u16 u1, v1;
	u16 u2, v2;
};

using GlyphID = u32;
#define GLYPHID_MAX ( U32_MAX )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Glyphs
{
	GlyphID make_new( const Glyph &glyph );
	GlyphID make_new( Glyph &&glyph );

	void build();

	Glyph &operator[]( const GlyphID id ) { return glyphs[id]; }

	List<Glyph> glyphs;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////