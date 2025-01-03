#pragma once

#include <core/types.hpp>
#include <core/list.hpp>
#include <core/buffer.hpp>
#include <core/string.hpp>

#include <build/textureio.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Glyph
{
	Glyph( Texture2DBuffer &&textureBuffer ) :
		textureBuffer( static_cast<Texture2DBuffer &&>( textureBuffer ) ),
		x1( 0 ), y1( 0 ), x2( 0 ), y2( 0 ) { };

	Texture2DBuffer textureBuffer;
	u16 x1, y1;
	u16 x2, y2;

	u16 u1, v1;
	u16 u2, v2;
};

using GlyphID = u32;
#define GLYPHID_MAX ( U32_MAX )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Glyphs
{
	GlyphID make_new( Texture2DBuffer &&textureBuffer );
	GlyphID make_new( const u16 width, const u16 height );

	void write();

	Glyph &operator[]( const GlyphID id ) { return glyphs[id]; }

	List<Glyph> glyphs;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////