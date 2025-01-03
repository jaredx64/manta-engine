#include <build/assets/textures.hpp>

#include <vendor/math.hpp>
#include <core/list.hpp>

#include <build/build.hpp>
#include <build/assets.hpp>
#include <build/filesystem.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GlyphID Texture::add_glyph( Texture2DBuffer &&textureBuffer )
{
	// Error checking
	AssertMsg( textureBuffer, "Trying to pack texture '%s' with invalid textureBuffer", name.cstr() );

	// Make new Glyph and pass ownership of 'textureBuffer'
	GlyphID glyphID = Assets::glyphs.make_new( static_cast<Texture2DBuffer &&>( textureBuffer ) );
	Glyph &glyph = Assets::glyphs[glyphID];
	glyphs.add( glyphID );

	// Return GlyphID
	return glyphID;
}


struct Space
{
	Space( int x, int y, int w, int h ) : x(x), y(y), w(w), h(h) { }

	int x, y, w, h;

	inline int area() const { return w * h; }
	bool operator>(const Space& other) const { return this->area() > other.area(); }
};


static int compare_glyphs( const GlyphID *a, const GlyphID *b )
{
	const Glyph &glyphA = Assets::glyphs[*a];
	const Glyph &glyphB = Assets::glyphs[*b];
	const Texture2DBuffer &A = glyphA.textureBuffer;
	const Texture2DBuffer &B = glyphB.textureBuffer;
	const int valueA = A.width + A.height;
	const int valueB = B.width + B.height;
	return valueA < valueB ? -1 : valueA > valueB;
}


static GlyphID *partition_glyphs( GlyphID *low, GlyphID *high, const bool ascending )
{
	GlyphID *i = low;
	GlyphID *j = low;

	while( i <= high )
	{
		if( ( ascending && compare_glyphs( i, high ) > 0 ) ||
		    ( !ascending && compare_glyphs( i, high ) < 0 ) )
		{
			i++;
		}
		else
		{
			// Swap elements
			GlyphID temp = *i;
			*i = *j; *j = temp;
			i++; j++;
		}
	}

	return j - 1;
}


static void quicksort_glyphs( GlyphID *low, GlyphID *high, const bool ascending )
{
	GlyphID *part;
	if( low >= high ) { return; }
	part = partition_glyphs( low, high, ascending );
	quicksort_glyphs( low, part - 1, ascending );
	quicksort_glyphs( part + 1, high, ascending );
}


void Texture::pack()
{
	// Starting size
	u16 size = 32;
	u8 padding = 1;

	// Sort Glyphs
	quicksort_glyphs( &glyphs[0], &glyphs[glyphs.size() - 1], false );

	// Pack
	for( ;; )
	{
		// Initialize list of "empty spaces"
		List<Space> spaces;
		spaces.add( { 0, 0, size, size } );

		// Loop over Glyphs6
		for( GlyphID glyphID : glyphs )
		{
			Glyph &glyph = Assets::glyphs[glyphID];

			// Loop over spaces back to front (smallest spaces are at the back of the list)
			usize index = USIZE_MAX;
			for( usize i = spaces.size(); i > 0; i-- )
			{
				Space &space = spaces[i-1];
				if( space.w >= glyph.textureBuffer.width + padding * 2 &&
					space.h >= glyph.textureBuffer.height + padding * 2 )
				{
					// Found a suitable space!
					index = i-1;
					break;
				}
			}

			// Glyph couldn't fit in any space?
			if( index == USIZE_MAX )
			{
				// Retry with doubled size
				size *= 2;
				ErrorIf( size >= 4096,
					"Failed to pack texture '%s' -- glyphs exceeded max texture resolution %dx%d",
					name.cstr(), 4096, 4096 );
				goto retry;
			}

			// Found a space
			Space &space = spaces[index];
			glyph.x1 = space.x + padding;
			glyph.y1 = space.y + padding;
			glyph.x2 = glyph.x1 + glyph.textureBuffer.width;
			glyph.y2 = glyph.y1 + glyph.textureBuffer.height;

			glyph.u1 = static_cast<u16>( glyph.x1 / static_cast<float>( size ) * 65536.0f );
			glyph.v1 = static_cast<u16>( glyph.y1 / static_cast<float>( size ) * 65536.0f );
			glyph.u2 = static_cast<u16>( glyph.x2 / static_cast<float>( size ) * 65536.0f );
			glyph.v2 = static_cast<u16>( glyph.y2 / static_cast<float>( size ) * 65536.0f );

			// Split space
			Space hSplit { space.x,
			               space.y + glyph.textureBuffer.height + padding * 2,
			               space.w,
			               space.h - glyph.textureBuffer.height - padding * 2 };

			Space vSplit { space.x + glyph.textureBuffer.width + padding * 2,
			               space.y,
			               space.w - glyph.textureBuffer.width - padding * 2,
			               glyph.textureBuffer.height + padding * 2 };

			// Remove space (swap back entry with index)
			spaces.remove_swap( index );

			// No-split case
			if( hSplit.area() == 0 && vSplit.area() == 0 ) { continue; } else
			// Single-split only cases
			if( hSplit.area() == 0 ) { spaces.add( vSplit ); } else
			if( vSplit.area() == 0 ) { spaces.add( hSplit ); } else
			// Double-split
			{
				spaces.add( hSplit > vSplit ? hSplit : vSplit ); // Bigger split
				spaces.add( vSplit > hSplit ? hSplit : vSplit ); // Smaller split
			}
		}

		goto success;
		retry: continue;
	}

success:
	width = size;
	height = size;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TextureID Textures::make_new( String &name )
{
	// Check if an Texture with this name already exists
	for( usize i = 0; i < textures.size(); i++ )
	{
		Texture &texture = textures[i];
		if( texture.name.equals( name ) ) { return i; }
	}

	// Make new Texture
	Texture &texture = textures.add( { name } );
	return static_cast<TextureID>( textures.size() - 1 );
}


TextureID Textures::make_new( String &name, Texture2DBuffer &&textureBuffer )
{
	// Make new Texture
	Texture &texture = textures.add( { name } );
	texture.width = textureBuffer.width;
	texture.height = textureBuffer.height;
	texture.add_glyph( static_cast<Texture2DBuffer &&>( textureBuffer ) );
	return static_cast<TextureID>( textures.size() - 1 );
}


void Textures::write()
{
	Buffer &binary = Assets::binary;
	String &header = Assets::header;
	String &source = Assets::source;

	Timer timer;
	usize sizeBytes = 0;

	// Binary
	{
		for( Texture &texture : textures )
		{
			const u32 numGlyphs = texture.glyphs.size();
			Assert( numGlyphs > 0 );

			// Atlas Texture
			if( numGlyphs >= 1 && texture.atlasTexture )
			{
				// Pack Glyphs
				texture.pack();

				// Initialize Texture2DBuffer
				Texture2DBuffer textureBuffer { texture.width, texture.height };

				for( GlyphID glyphID : texture.glyphs )
				{
					Glyph &glyph = Assets::glyphs[glyphID];
					textureBuffer.splice( glyph.textureBuffer,
					                      0, 0, glyph.textureBuffer.width, glyph.textureBuffer.height,
					                      glyph.x1, glyph.y1 );
				}

				// Write Binary
				texture.offset = binary.tell;
				binary.write( textureBuffer.data, texture.width * texture.height * sizeof( rgba ) );
				sizeBytes += texture.width * texture.height * sizeof( rgba );

				char path[PATH_SIZE];
				strjoin( path, Build::pathOutput, SLASH "generated" SLASH, ( texture.name + "_atlas.png" ).cstr() );
				textureBuffer.save( path );
			}
			// Independent Texture
			else if( numGlyphs == 1 )
			{
				Glyph &glyph = Assets::glyphs[texture.glyphs[0]];
				texture.width = glyph.textureBuffer.width;
				texture.height = glyph.textureBuffer.height;

				// Write Binary
				texture.offset = binary.tell;
				binary.write( glyph.textureBuffer.data, texture.width * texture.height * sizeof( rgba ) );
				sizeBytes += texture.width * texture.height * sizeof( rgba );

				#if 0
					char path[PATH_SIZE];
					strjoin( path, Build::pathOutput, SLASH "generated" SLASH, ( texture.name + ".png" ).cstr() );
					glyph.textureBuffer.save( path );
				#endif
			}
			else
			{
				Error( "Attempting to write null texture to binary file! (texture: %s)", texture.name.cstr() );
			}
		}
	}

	// Header
	{
		// Group
		assets_group( header );

		// Struct
		assets_struct( header,
			"DiskTexture",
			"u32 offset;",
			"u16 width;",
			"u16 height;" );

		// Table
		header.append( "namespace Assets\n{\n" );
		header.append( "\tconstexpr u32 texturesCount = " );
		header.append( static_cast<int>( textures.size() ) ).append( ";\n" );
		header.append( "\textern const DiskTexture textures[];\n" );
		header.append( "}\n\n" );
	}

	// Source
	{
		// Group
		assets_group( source );
		source.append( "namespace Assets\n{\n" );

		// Table
		source.append( "\tconst DiskTexture textures[texturesCount] =\n\t{\n" );
		char buffer[PATH_SIZE];
		for( Texture &texture : textures )
		{
			snprintf( buffer, PATH_SIZE, "\t\t{ %lluULL, %u, %u },\n",
				texture.offset,
				texture.width,
				texture.height );

			source.append( buffer );
		}
		source.append( "\t};\n" );
		source.append( "}\n\n" );
	}

	if( verbose_output() )
	{
		const usize count = textures.size();
		PrintColor( LOG_CYAN, "\t\tWrote %d texture%s - %.2f mb", count, count == 1 ? "" : "s", MB( sizeBytes ) );
		PrintLnColor( LOG_WHITE, " (%.3f ms)", timer.elapsed_ms() );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////