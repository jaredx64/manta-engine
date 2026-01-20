#include <build/assets/glyphs.hpp>

#include <core/list.hpp>

#include <build/build.hpp>
#include <build/assets.hpp>
#include <build/filesystem.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GlyphID Glyphs::make_new( const Glyph &glyph )
{
	AssertMsg( glyphs.size() < GLYPHID_MAX, "Exceeded max number of Glyphs" );
	glyphs.add( glyph );
	return glyphs.size() - 1;
}


GlyphID Glyphs::make_new( Glyph &&glyph )
{
	AssertMsg( glyphs.size() < GLYPHID_MAX, "Exceeded max number of Glyphs" );
	glyphs.add( static_cast<Glyph &&>( glyph ) );
	return glyphs.size() - 1;
}


void Glyphs::build()
{
	Buffer &binary = Assets::binary;
	String &header = Assets::header;
	String &source = Assets::source;
	const u32 count = static_cast<u32>( glyphs.size() );

	Timer timer;

	// Load
	{
		// ...
	}

	// Binary
	{
		// ...
	}

	// Header
	{
		// Group
		assets_group( header );

		// Struct
		header.append( "namespace Assets { struct GlyphEntry; }\n\n" );

		// Table
		header.append( "namespace CoreAssets\n{\n" );
		header.append( "\tconstexpr u32 glyphCount = " ).append( count ).append( ";\n" );
		header.append( count > 0 ? "\textern const Assets::GlyphEntry glyphs[glyphCount];\n" :
			"\textern const Assets::GlyphEntry *glyphs;\n" );
		header.append( "}\n\n" );
	}

	// Source
	{
		// Group
		assets_group( source );
		source.append( "namespace CoreAssets\n{\n" );

		// Table
		if( count > 0 )
		{
			char buffer[PATH_SIZE];
			source.append( "\tconst Assets::GlyphEntry glyphs[glyphCount] =\n\t{\n" );
			for( Glyph &glyph : glyphs )
			{
				snprintf( buffer, PATH_SIZE,
					"\t\t{ %d, %d, %d, %d },\n",
					glyph.u1, glyph.v1,
					glyph.u2, glyph.v2 );

				source.append( buffer );
			}
			source.append( "\t};\n" );
		}
		else
		{
			source.append( "\tconst Assets::GlyphEntry *glyphs = nullptr;" );
		}
		source.append( "}\n\n" );
	}

	if( verbose_output() )
	{
		const usize count = glyphs.size();
		Print( PrintColor_White, TAB TAB "Wrote %d glyph%s", count, count == 1 ? "" : "s" );
		PrintLn( PrintColor_White, " (%.3f ms)", timer.elapsed_ms() );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////