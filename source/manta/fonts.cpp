#include <manta/fonts.hpp>

#include <vendor/vendor.hpp>
#include <vendor/stb/stb_truetype.hpp>

#include <core/memory.hpp>
#include <core/string.hpp>
#include <core/list.hpp>
#include <core/utf8.hpp>
#include <core/color.hpp>

#include <manta/draw.hpp>
#include <manta/filesystem.hpp>
#include <manta/textureio.hpp>
#include <manta/gfx.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace CoreFonts
{
	struct FontGlyphEntry
	{
		FontGlyphEntry( FontGlyphKey key, FontGlyphInfo value ) : key{ key }, value{ value } { }
		FontGlyphKey key;
		FontGlyphInfo value;
	};

	struct FontInfo
	{
		FontInfo() { }
		stbtt_fontinfo info;
	};

	// Global
	GfxTexture glyphAtlasTexture;

	// Static
	static u16 insertX = CoreFonts::FONTS_GLYPH_PADDING;
	static u16 insertY = CoreFonts::FONTS_GLYPH_PADDING;
	static u16 lineHeight = 0;

	static Texture2DBuffer glyphAtlasTextureBuffer;
	static byte *bitmap;

	static FontGlyphEntry *data = nullptr;
	static List<FontInfo> fontInfos;
	static List<FontGlyphEntry> dirtyGlyphs;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

u32 CoreFonts::FontGlyphKey::hash()
{
	// Calculate 'randomized' offset from font id and size using prime numbers
	const u32 p1 = 41;
	const u32 p2 = 37;
	const u32 p3 = 31;
	const u32 offset = ( ( ttf * p1 ) ^ ( size * p2 ) ) * p3 ^ ( ( ( ttf * p1 ) ^ ( size * p2 ) ) >> 16 );

	// Calculate hash index
	return static_cast<u32>( ( codepoint / CoreFonts::FONTS_GROUP_SIZE + offset ) % CoreFonts::FONTS_TABLE_SIZE );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CoreFonts::FontGlyphInfo::get_glyph_metrics( const u32 codepoint, const u16 ttf, const u16 size )
{
	CoreFonts::FontInfo &fontInfo = fontInfos[ttf];
	const float scale = stbtt_ScaleForPixelHeight( &fontInfo.info, size * ( 96.0f / 72.0f ) );

	// Max Ascent (TODO: considering caching this instead of getting it each time?)
	int mx0, my0, mx1, my1;
	stbtt_GetCodepointBitmapBox( &fontInfo.info, 'T', scale, scale, &mx0, &my0, &mx1, &my1 );

	// Get glyph metrics
	int x0, y0, x1, y1;
	stbtt_GetCodepointBitmapBox( &fontInfo.info, codepoint, scale, scale, &x0, &y0, &x1, &y1 );

	const int w = x1 - x0;
	const int h = y1 - y0;
	ErrorIf( w < 0 || h < 0,
		"Fonts: invalid glyph size (%dx%d) for codepoint %llu (font %d, size: %d)", w, h, codepoint, ttf, size );
	ErrorIf( static_cast<u32>( w ) >= CoreFonts::FONTS_GLYPH_SIZE_MAX ||
	         static_cast<u32>( h ) >= CoreFonts::FONTS_GLYPH_SIZE_MAX,
		"Fonts: exceeded glyph size (%dx%d) for codepoint %llu (font %d, size: %d)", w, h, codepoint, ttf, size );

	int advance, leftSideBearing;
	stbtt_GetCodepointHMetrics( &fontInfo.info, codepoint, &advance, &leftSideBearing );

	// Update FontGlyphInfo
	this->advance = static_cast<u8>( advance * scale );
	this->width = static_cast<u8>( w );
	this->height = static_cast<u8>( h );
	this->xshift = x0;
	this->yshift = y0 - my0;
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CoreFonts::init()
{
	// Init Fonts Table
	Assert( data == nullptr );
	constexpr usize size = CoreFonts::FONTS_GROUP_SIZE *
	                       CoreFonts::FONTS_TABLE_DEPTH *
	                       CoreFonts::FONTS_TABLE_SIZE *
	                       sizeof( CoreFonts::FontGlyphEntry );
	data = reinterpret_cast<CoreFonts::FontGlyphEntry *>( memory_alloc( size ) );
	memory_set( data, 0, size ); // Zero memory

	// Load Font Metrics
	fontInfos.init( CoreAssets::fontCount );
	for( u16 ttf = 0; ttf < CoreAssets::ttfCount; ttf++ )
	{
		// Get Font Info
		CoreFonts::FontInfo &fontInfo = fontInfos.add( { } );
		const byte *ttfData = &Assets::binary.data[Assets::ttf( ttf ).offset];
		ErrorReturnIf( stbtt_InitFont( &fontInfo.info, ttfData, stbtt_GetFontOffsetForIndex( ttfData, 0 ) ) != 1,
			false, "Fonts: failed to get metrics for ttf: %u", ttf );
	}

	// Init dirtyGlyphs list
	dirtyGlyphs.init();

	// Init Texture2DBuffer
	glyphAtlasTextureBuffer.init( CoreFonts::FONTS_TEXTURE_SIZE, CoreFonts::FONTS_TEXTURE_SIZE );
	glyphAtlasTextureBuffer.clear( rgba{ 0, 0, 0, 0 } );

	// Init Texture2D
	glyphAtlasTexture.init_2d( glyphAtlasTextureBuffer.data, glyphAtlasTextureBuffer.width,
		glyphAtlasTextureBuffer.height, GfxColorFormat_R8G8B8A8_FLOAT );

	// Init bitmap buffer
	bitmap = reinterpret_cast<byte *>(
		memory_alloc( CoreFonts::FONTS_GLYPH_SIZE_MAX * CoreFonts::FONTS_GLYPH_SIZE_MAX ) );

	return true;
}


bool CoreFonts::free()
{
	// Free RTFonts table
	if( data != nullptr )
	{
		memory_free( data );
		data = nullptr;
	}

	// Free font metrics
	fontInfos.free();

	// Free dirtyGlyphs list
	dirtyGlyphs.free();

	// Free Texture2D
	glyphAtlasTexture.free();

	// Free bitmap buffer
	if( bitmap != nullptr )
	{
		memory_free( bitmap );
		bitmap = nullptr;
	}

	return true;
}


CoreFonts::FontGlyphInfo &CoreFonts::get( FontGlyphKey key )
{
	// The goal here is to efficiently cache and retrieve font glyphs at runtime using an optimized "hashtable"
	//
	// FontGlyphEntry is 16 bytes (FontGlyphKey + FontGlyphInfo) meaning 4 glyphs fit in a 64 byte cache line.
	// The 'hash' index below ensures consecutive codepoints of a font and size (i.e. 'a', 'b', 'c', 'd') all share
	// a single L1 cache line. Since the hashing function can produce collisions, 'FONTS_TABLE_DEPTH' number of
	// collisions are allowed before the glyph retrieval is aborted

	const usize index = key.hash() * ( CoreFonts::FONTS_GROUP_SIZE * CoreFonts::FONTS_TABLE_DEPTH ) +
	                                 ( key.codepoint % CoreFonts::FONTS_GROUP_SIZE );

	for( u8 collision = 0; collision < FONTS_TABLE_DEPTH; collision++ )
	{
		// Retrieve FontGlyphEntry
		CoreFonts::FontGlyphEntry &entry = data[index + collision * CoreFonts::FONTS_GROUP_SIZE];

		// Found our key?
		if( LIKELY( entry.key == key ) )
		{
			return entry.value;
		}

		// Empty key?
		if( entry.key.codepoint == 0 )
		{
			// Retrieve metrics & cache the glyph
			entry.key = key;
			entry.value.get_glyph_metrics( key.codepoint, key.ttf, key.size );

			// Pack Glyph
			if( !pack( entry.value ) )
			{
				// Packing failed -- out of room? Return a "null key"
				AssertMsg( false, "Fonts: failed to pack glyph (codepoint: %llu)", index, key.codepoint );
				return data[0].value;
			}

			// Add glyph to rasterization list
			dirtyGlyphs.add( entry );
			return entry.value;
		}
	}

	// Return a "null key"
	AssertMsg( false, "Fonts: saturated glyph table at index %d (codepoint: %llu)", index, key.codepoint );
	return data[0].value;
}


bool CoreFonts::pack( CoreFonts::FontGlyphInfo &glyphInfo )
{
	// Check if the glyph pushes us onto a "new line"
	if( insertX + glyphInfo.width + CoreFonts::FONTS_GLYPH_PADDING >= glyphAtlasTextureBuffer.width )
	{
		insertX = CoreFonts::FONTS_GLYPH_PADDING;
		insertY += lineHeight + CoreFonts::FONTS_GLYPH_PADDING * 2;
		lineHeight = 0;
	}

	// No more room?
	if( insertY + glyphInfo.height + CoreFonts::FONTS_GLYPH_PADDING >= glyphAtlasTextureBuffer.height )
	{
		return false;
	}

	// Insert Glyph
	glyphInfo.u = insertX;
	glyphInfo.v = insertY;
	insertX += glyphInfo.width + CoreFonts::FONTS_GLYPH_PADDING * 2;
	lineHeight = glyphInfo.height > lineHeight ? glyphInfo.height : lineHeight;
	return true;
}


void CoreFonts::flush()
{
	// Clear Glyph table cache
	constexpr usize size = CoreFonts::FONTS_GROUP_SIZE *
	                       CoreFonts::FONTS_TABLE_DEPTH *
	                       CoreFonts::FONTS_TABLE_SIZE *
	                       sizeof( CoreFonts::FontGlyphEntry );
	memory_set( data, 0, size );

	// Clear Texture2DBuffer
	glyphAtlasTextureBuffer.clear( rgba { 0, 0, 0, 0 } );

	// Clear newGlyph list
	dirtyGlyphs.clear();

	// Update GPU texture
	glyphAtlasTexture.free();
	glyphAtlasTexture.init_2d( glyphAtlasTextureBuffer.data, glyphAtlasTextureBuffer.width,
		glyphAtlasTextureBuffer.height, GfxColorFormat_R8G8B8A8_FLOAT );

	// Reset packing state
	insertX = FONTS_GLYPH_PADDING;
	insertY = FONTS_GLYPH_PADDING;
	lineHeight = 0;
}


void CoreFonts::update()
{
	// Dirty?
	if( dirtyGlyphs.size() == 0 ) { return; }

	// Rasterize glyph bitmaps & copy to Texture2DBuffer
	for( FontGlyphEntry &entry : dirtyGlyphs )
	{
		// TODO: This can potentially cause frame spikes as rasterizing & copying glyphs is a CPU intensive task
		// Consider moving this to a background thread and double buffering?

		const CoreFonts::FontGlyphKey &key = entry.key;
		const CoreFonts::FontGlyphInfo &glyph = entry.value;
		const CoreFonts::FontInfo &fontInfo = fontInfos[entry.key.ttf];

		// Bitmap
		const float scale = stbtt_ScaleForPixelHeight( &fontInfo.info, key.size * ( 96.0f / 72.0f ) );
		stbtt_MakeCodepointBitmap( &fontInfo.info, bitmap, glyph.width, glyph.height,
		                           glyph.width, scale, scale, key.codepoint );

		// Transfer to Bitmap to RGBA Texture2DBuffer
		for( u16 x = 0; x < glyph.width; x++ )
		{
			for( u16 y = 0; y < glyph.height; y++ )
			{
				const u32 srcIndex = y * glyph.width + x;
				const u32 dstIndex = ( glyph.v + y ) * glyphAtlasTextureBuffer.width + ( glyph.u + x );
				glyphAtlasTextureBuffer.data[dstIndex] = rgba{ 255, 255, 255, static_cast<u8>( bitmap[srcIndex] ) };
			}
		}
	}

	// Clear dirtyGlyphs
	dirtyGlyphs.clear();

	// Update GPU texture
	glyphAtlasTexture.free();
	glyphAtlasTexture.init_2d( glyphAtlasTextureBuffer.data, glyphAtlasTextureBuffer.width,
		glyphAtlasTextureBuffer.height, GfxColorFormat_R8G8B8A8_FLOAT );
}


void CoreFonts::cache( const u16 ttf, const u16 size, const u32 start, const u32 end )
{
	for( u32 codepoint = start; codepoint <= end; codepoint++ )
	{
		get( CoreFonts::FontGlyphKey { codepoint, ttf, size } );
	}
}


void CoreFonts::cache( const u16 ttf, const u16 size, const char *buffer )
{
	u32 state = UTF8_ACCEPT;
	u32 codepoint;
	char c;
	while( ( c = *buffer++ ) != '\0' )
	{
		if( utf8_decode( &state, &codepoint, c ) != UTF8_ACCEPT ) { continue; }
		get( CoreFonts::FontGlyphKey { codepoint, ttf, size } );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////