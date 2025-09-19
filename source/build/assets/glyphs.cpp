#include <build/assets/glyphs.hpp>

#include <core/list.hpp>

#include <build/build.hpp>
#include <build/assets.hpp>
#include <build/filesystem.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GlyphID Glyphs::make_new( Texture2DBuffer &&textureBuffer )
{
	// Register Glyph and move ownership of 'textureBuffer'
	AssertMsg( glyphs.size() < GLYPHID_MAX, "Exceeded max number of Glyphs" );
	Glyph &glyph = glyphs.add( { static_cast<Texture2DBuffer &&>( textureBuffer ) } );

	// Return GlyphID
	return glyphs.size() - 1;
}


GlyphID Glyphs::make_new( const u16 width, const u16 height )
{
	// Register Glyph and move ownership of new 'textureBuffer'
	Texture2DBuffer textureBuffer { width, height };
	Glyph &glyph = glyphs.add( { static_cast<Texture2DBuffer &&>( textureBuffer ) } );

	// Return GlyphID
	return glyphs.size() - 1;
}


void Glyphs::write()
{
	Buffer &binary = Assets::binary;
	String &header = Assets::header;
	String &source = Assets::source;

	Timer timer;

	// Binary - do nothing
	// n/a

	// Header
	{
		// Group
		assets_group( header );

		// Struct
		header.append( "struct BinGlyph;\n\n" );

		// Table
		header.append( "namespace Assets\n{\n" );
		header.append( "\tconstexpr u32 glyphsCount = " );
		header.append( static_cast<int>( glyphs.size() ) ).append( ";\n" );
		header.append( "\textern const BinGlyph glyphs[];\n" );
		header.append( "}\n\n" );
	}

	// Source
	{
		// Group
		assets_group( source );
		source.append( "namespace Assets\n{\n" );

		// Table
		char buffer[PATH_SIZE];
		source.append( "\tconst BinGlyph glyphs[glyphsCount] =\n\t{\n" );
		for( Glyph &glyph : glyphs )
		{
			snprintf( buffer, PATH_SIZE,
				"\t\t{ %d, %d, %d, %d },\n",
				glyph.u1, glyph.v1,
				glyph.u2, glyph.v2 );

			source.append( buffer );
		}
		source.append( "\t};\n" );
		source.append( "}\n\n" );
	}

	if( verbose_output() )
	{
		const usize count = glyphs.size();
		PrintColor( LOG_CYAN, "\t\tWrote %d glyph%s", count, count == 1 ? "" : "s" );
		PrintLnColor( LOG_WHITE, " (%.3f ms)", timer.elapsed_ms() );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////