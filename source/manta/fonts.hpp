#pragma once

#include <config.hpp>

#include <core/types.hpp>
#include <core/debug.hpp>

#include <manta/gfx.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Font
{
public:
	constexpr Font( const u16 id, const u16 ttf ) : id{ id }, ttf{ ttf } { };
	constexpr operator u8() const { return static_cast<u8>( id ); }

	/*constexpr */Font regular() const { return Font { id, CoreAssets::fonts[id].ttfs[0] }; }
	/*constexpr */Font italics() const { return Font { id, CoreAssets::fonts[id].ttfs[1] }; }
	/*constexpr */Font bold() const { return Font { id, CoreAssets::fonts[id].ttfs[2] }; }
	/*constexpr */Font bold_italics() const { return Font { id, CoreAssets::fonts[id].ttfs[3] }; }

	const u16 id, ttf;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace CoreFonts
{
	constexpr u32 FONTS_GROUP_SIZE = 4;
	constexpr u32 FONTS_TABLE_DEPTH = 8;
	constexpr u32 FONTS_TABLE_SIZE = 4096;
	constexpr u32 FONTS_TEXTURE_SIZE = 1024;
	constexpr u32 FONTS_GLYPH_PADDING = 1;
	constexpr u32 FONTS_GLYPH_SIZE_MAX = 256; // FontGlyphInfo width/height is u8

	struct FontGlyphKey
	{
		FontGlyphKey( u32 codepoint, u16 ttf, u16 size ) : codepoint{ codepoint }, ttf{ ttf }, size{ size } { }

		u32 hash();

		bool operator==( const FontGlyphKey &other ) const
		{
			return codepoint == other.codepoint &&
			       ttf == other.ttf &&
			       size == other.size;
		}

		u32 codepoint;
		u16 ttf, size;
	};
	static_assert( sizeof( FontGlyphKey ) == 8, "FontGlyphKey not 8 bytes!" );

	struct FontGlyphInfo
	{
		bool get_glyph_metrics( const u32 codepoint, const u16 ttf, const u16 size );

		u32 u       : 12; // 4096 max
		u32 v       : 12; // 4096 max
		u32 advance :  8; //  256 max
		u8 width, height;
		i8 xshift, yshift;
	};
	static_assert( sizeof( FontGlyphInfo ) == 8, "FontGlyphInfo not 8 bytes!" );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace CoreFonts
{
	extern bool init();
	extern bool free();
	extern void flush();
	extern void update();

	extern CoreFonts::FontGlyphInfo &get( CoreFonts::FontGlyphKey key );
	extern bool pack( CoreFonts::FontGlyphInfo &glyphInfo );

	extern void cache( const u16 ttf, const u16 size, const u32 start, const u32 end );
	extern void cache( const u16 ttf, const u16 size, const char *buffer );

	extern GfxTexture2D texture2D;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////