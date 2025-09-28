#include <build/assets/textures.hpp>

#include <vendor/math.hpp>
#include <core/list.hpp>

#include <build/build.hpp>
#include <build/assets.hpp>
#include <build/filesystem.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum_type( GfxColorFormat, u8 )
{
	GfxColorFormat_NONE = 0,
	GfxColorFormat_R8G8B8A8_FLOAT,
	GfxColorFormat_R8G8B8A8_UINT,
	GfxColorFormat_R10G10B10A2_FLOAT,
	GfxColorFormat_R8,
	GfxColorFormat_R8G8,
	GfxColorFormat_R16,
	GfxColorFormat_R16_FLOAT,
	GfxColorFormat_R16G16,
	GfxColorFormat_R16G16F_FLOAT,
	GfxColorFormat_R16G16B16A16_FLOAT,
	GfxColorFormat_R16G16B16A16_UINT,
	GfxColorFormat_R32_FLOAT,
	GfxColorFormat_R32G32_FLOAT,
	GfxColorFormat_R32G32B32A32_FLOAT,
	GfxColorFormat_R32G32B32A32_UINT,
	GFXCOLORFORMAT_COUNT,
};


constexpr u32 colorFormatPixelSizeBytes[GFXCOLORFORMAT_COUNT] =
{
	0,  // GfxColorFormat_NONE
	4,  // GfxColorFormat_R8G8B8A8_FLOAT
	4,  // GfxColorFormat_R8G8B8A8_UINT
	4,  // GfxColorFormat_R10G10B10A2_FLOAT
	1,  // GfxColorFormat_R8
	2,  // GfxColorFormat_R8G8
	2,  // GfxColorFormat_R16
	2,  // GfxColorFormat_R16_FLOAT
	4,  // GfxColorFormat_R16G16
	4,  // GfxColorFormat_R16G16F_FLOAT
	8,  // GfxColorFormat_R16G16B16A16_FLOAT
	8,  // GfxColorFormat_R16G16B16A16_UINT
	4,  // GfxColorFormat_R32_FLOAT
	8,  // GfxColorFormat_R32G32_FLOAT
	16, // GfxColorFormat_R32G32B32A32_FLOAT
	16, // GfxColorFormat_R32G32B32A32_UINT
};


static u16 mip_level_count_2d( u16 width, u16 height )
{
	u16 l = 1;
	while( width > 1 && height > 1 ) { width >>= 1; height >>= 1; l++; }
	return l;
}


static bool mip_level_validate_2d( u16 width, u16 height, u16 levels )
{
	return levels <= mip_level_count_2d( width, height );
}


static usize mip_buffer_size_2d( u16 width, u16 height, u16 levels,
	const GfxColorFormat format )
{
	ErrorReturnIf( format >= GFXCOLORFORMAT_COUNT, 0,
		"%s: invalid GfxColorFormat: %u", __FUNCTION__, format );
	ErrorReturnIf( !mip_level_validate_2d( width, height, levels ), 0,
		"%s: invalid levels (%u) for dimensions: %u x %u", __FUNCTION__,
		levels, width, height );

	usize count = 0;

	while( levels > 0 )
	{
		count += width * height;
		width >>= 1;
		height >>= 1;
		levels--;
	}

	return colorFormatPixelSizeBytes[format] * count;
}


static bool mip_generate_next_2d( void *data, const u16 width, const u16 height,
	const GfxColorFormat format, void *dest, const usize size )
{
	ErrorReturnIf( format >= GFXCOLORFORMAT_COUNT, false,
		"%s: invalid GfxColorFormat: %u", __FUNCTION__, format );
	ErrorReturnIf( data == nullptr, false,
		"%s: data is nullptr", __FUNCTION__ );
	ErrorReturnIf( dest == nullptr, false,
		"%s: dest is nullptr", __FUNCTION__ );
	ErrorReturnIf( width <= 1 || height <= 1, false,
		"%s: cannot generate mip for input dimensions: %u x %u", __FUNCTION__, width, height );

	const u16 mipWidth = width / 2;
	const u16 mipHeight = height / 2;

	const u16 pixelSizeBytes = colorFormatPixelSizeBytes[format];
	const usize mipSizeBytes = pixelSizeBytes * mipWidth * mipHeight;

	ErrorReturnIf( size != mipSizeBytes, false,
		"%s: dest size does not match request mip size (dst: %u bytes, mip: %u bytes %u,%u)",
		__FUNCTION__, size, mipSizeBytes, width, height );

	for( u16 y = 0; y < mipHeight; y++ )
	for( u16 x = 0; x < mipWidth; x++ )
	{
		u32 srcX = x * 2;
		u32 srcY = y * 2;

		const u8 *p00 = static_cast<u8 *>( data ) + ( ( srcY + 0 ) * width + ( srcX + 0 ) ) * pixelSizeBytes;
		const u8 *p10 = static_cast<u8 *>( data ) + ( ( srcY + 0 ) * width + ( srcX + 1 ) ) * pixelSizeBytes;
		const u8 *p01 = static_cast<u8 *>( data ) + ( ( srcY + 1 ) * width + ( srcX + 0 ) ) * pixelSizeBytes;
		const u8 *p11 = static_cast<u8 *>( data ) + ( ( srcY + 1 ) * width + ( srcX + 1 ) ) * pixelSizeBytes;

		u8 *out = static_cast<u8 *>( dest ) + ( y * mipWidth + x ) * pixelSizeBytes;

		switch( format )
		{
			case GfxColorFormat_R8:
			{
				u32 v = p00[0] + p10[0] + p01[0] + p11[0];
				out[0] = static_cast<u8>( v / 4 );
			}
			break;

			case GfxColorFormat_R8G8:
			{
				for( int c = 0; c < 2; c++ )
				{
					out[c] = static_cast<u8>( ( p00[c] + p10[c] + p01[c] + p11[c] ) / 4 );
				}
			}
			break;

			case GfxColorFormat_R8G8B8A8_UINT:
			case GfxColorFormat_R8G8B8A8_FLOAT:
			{
				for( int c = 0; c < 4; c++ )
				{
					out[c] = static_cast<u8>( ( p00[c] + p10[c] + p01[c] + p11[c] ) / 4 );
				}
			}
			break;

			case GfxColorFormat_R16:
			case GfxColorFormat_R16_FLOAT:
			{
				u16 *o = reinterpret_cast<u16 *>( out );
				const u16 *s0 = reinterpret_cast<const u16 *>( p00 );
				const u16 *s1 = reinterpret_cast<const u16 *>( p10 );
				const u16 *s2 = reinterpret_cast<const u16 *>( p01 );
				const u16 *s3 = reinterpret_cast<const u16 *>( p11 );
				o[0] = static_cast<u16>( static_cast<u32>( s0[0] ) + s1[0] + s2[0] + s3[0] ) / 4;
			}
			break;

			case GfxColorFormat_R16G16:
			case GfxColorFormat_R16G16F_FLOAT:
			{
				u16 *o = reinterpret_cast<u16 *>( out );
				const u16* s0 = reinterpret_cast<const u16 *>( p00 );
				const u16* s1 = reinterpret_cast<const u16 *>( p10 );
				const u16* s2 = reinterpret_cast<const u16 *>( p01 );
				const u16* s3 = reinterpret_cast<const u16 *>( p11 );
				for( int c = 0; c < 2; c++ )
				{
					o[c] = static_cast<u16>( static_cast<u32>( s0[c] ) + s1[c] + s2[c] + s3[c] ) / 4;
				}
			}
			break;

			case GfxColorFormat_R16G16B16A16_UINT:
			case GfxColorFormat_R16G16B16A16_FLOAT:
			{
				u16 *o = reinterpret_cast<u16 *>( out );
				const u16 *s0 = reinterpret_cast<const u16 *>( p00 );
				const u16 *s1 = reinterpret_cast<const u16 *>( p10 );
				const u16 *s2 = reinterpret_cast<const u16 *>( p01 );
				const u16 *s3 = reinterpret_cast<const u16 *>( p11 );
				for( int c = 0; c < 4; c++ )
				{
					o[c] = static_cast<u16>( static_cast<u32>( s0[c] ) + s1[c] + s2[c] + s3[c] ) / 4;
				}
			}
			break;

			case GfxColorFormat_R32_FLOAT:
			{
				float *o = reinterpret_cast<float *>( out );
				const float *s0 = reinterpret_cast<const float *>( p00 );
				const float *s1 = reinterpret_cast<const float *>( p10 );
				const float *s2 = reinterpret_cast<const float *>( p01 );
				const float *s3 = reinterpret_cast<const float *>( p11 );
				o[0] = ( s0[0] + s1[0] + s2[0] + s3[0] ) * 0.25f;
			}
			break;

			case GfxColorFormat_R32G32_FLOAT:
			{
				float *o = reinterpret_cast<float *>( out );
				const float *s0 = reinterpret_cast<const float *>( p00 );
				const float *s1 = reinterpret_cast<const float *>( p10 );
				const float *s2 = reinterpret_cast<const float *>( p01 );
				const float *s3 = reinterpret_cast<const float *>( p11 );
				for( int c = 0; c < 2; c++ )
				{
					o[c] = ( s0[c] + s1[c] + s2[c] + s3[c] ) * 0.25f;
				}
			}
			break;

			case GfxColorFormat_R32G32B32A32_FLOAT:
			{
				float *o = reinterpret_cast<float *>( out );
				const float *s0 = reinterpret_cast<const float *>( p00 );
				const float *s1 = reinterpret_cast<const float *>( p10 );
				const float *s2 = reinterpret_cast<const float *>( p01 );
				const float *s3 = reinterpret_cast<const float *>( p11 );
				for( int c = 0; c < 4; c++ )
				{
					o[c] = ( s0[c] + s1[c] + s2[c] + s3[c] ) * 0.25f;
				}
			}
			break;

			case GfxColorFormat_R32G32B32A32_UINT:
			{
				u32 *o = reinterpret_cast<u32 *>( out );
				const u32 *s0 = reinterpret_cast<const u32 *>( p00 );
				const u32 *s1 = reinterpret_cast<const u32 *>( p10 );
				const u32 *s2 = reinterpret_cast<const u32 *>( p01 );
				const u32 *s3 = reinterpret_cast<const u32 *>( p11 );
				for( int c = 0; c < 4; c++ )
				{
					o[c] = ( s0[c] + s1[c] + s2[c] + s3[c] ) / 4;
				}
			}
			break;

			case GfxColorFormat_R10G10B10A2_FLOAT:
			{
				// Packed as 32-bit
				auto unpack = []( u32 v, int shift, int bits ) -> u32
				{
					return ( v >> shift ) & ( ( 1u << bits ) - 1u );
				};

				auto pack = []( u32 r, u32 g, u32 b, u32 a ) -> u32
				{
					return ( r & 0x3FF ) | ( ( g & 0x3FF) << 10 ) |
						( ( b & 0x3FF ) << 20 ) | ( ( a & 0x3 ) << 30 );
				};

				const u32 *s0 = reinterpret_cast<const u32 *>( p00 );
				const u32 *s1 = reinterpret_cast<const u32 *>( p10 );
				const u32 *s2 = reinterpret_cast<const u32 *>( p01 );
				const u32 *s3 = reinterpret_cast<const u32 *>( p11 );

				u32 r = ( unpack( *s0, 0, 10 ) + unpack( *s1, 0, 10 ) +
					unpack( *s2, 0, 10 ) + unpack( *s3, 0, 10 ) ) / 4;
				u32 g = ( unpack( *s0, 10, 10 ) + unpack( *s1, 10, 10 ) +
					unpack( *s2, 10, 10 ) + unpack( *s3, 10, 10 ) ) / 4;
				u32 b = ( unpack( *s0, 20, 10 ) + unpack( *s1, 20, 10 ) +
					unpack( *s2, 20, 10 ) + unpack( *s3, 20, 10 ) ) / 4;
				u32 a = ( unpack( *s0, 30, 2 ) + unpack( *s1, 30, 2 ) +
					unpack( *s2, 30, 2 ) + unpack( *s3, 30, 2 ) ) / 4;

				*reinterpret_cast<u32 *>( out ) = pack( r, g, b, a );
			}
			break;

			default:
			{
				ErrorReturnMsg( false, "%s: unsupported format %u", __FUNCTION__, format );
			}
			break;
		}
	}

	return true;
}


bool mip_generate_next_2d_alloc( void *data, const u16 width, const u16 height,
	const GfxColorFormat format, void *&outData, usize &outSize )
{
	ErrorReturnIf( format >= GFXCOLORFORMAT_COUNT, false,
		"%s: invalid GfxColorFormat: %u", __FUNCTION__, format );
	ErrorReturnIf( data == nullptr, false,
		"%s: data is nullptr", __FUNCTION__ );
	ErrorReturnIf( outData != nullptr, false,
		"%s: outData is not nullptr", __FUNCTION__ );
	ErrorReturnIf( width <= 1 || height <= 1, false,
		"%s: cannot generate mip for input dimensions: %u x %u", __FUNCTION__, width, height );

	const u16 mipWidth = width / 2;
	const u16 mipHeight = height / 2;

	const u16 pixelSizeBytes = colorFormatPixelSizeBytes[format];
	outSize = pixelSizeBytes * mipWidth * mipHeight;
	outData = memory_alloc( outSize );

	// Generate mip
	if( !mip_generate_next_2d( data, width, height, format, outData, outSize ) )
	{
		memory_free( outData );
		outData = nullptr;
		outSize = 0;
		return false;
	}

	return true;
}


static bool mip_generate_chain_2d( void *data, const u16 width, const u16 height,
	const GfxColorFormat format, void *dest, const usize size )
{
	ErrorReturnIf( format >= GFXCOLORFORMAT_COUNT, false,
		"%s: invalid GfxColorFormat: %u", __FUNCTION__, format );
	ErrorReturnIf( data == nullptr, false,
		"%s: data is nullptr", __FUNCTION__ );
	ErrorReturnIf( dest == nullptr, false,
		"%s: dest is nullptr", __FUNCTION__ );
	ErrorReturnIf( width <= 1 || height <= 1, false,
		"%s: cannot generate mip for input dimensions: %u x %u", __FUNCTION__, width, height );

	const u16 levels = mip_level_count_2d( width, height );
	const u16 pixelSizeBytes = colorFormatPixelSizeBytes[format];
	const usize mipSizeBytes = mip_buffer_size_2d( width, height, levels, format );

	ErrorReturnIf( size != mipSizeBytes, false,
		"%s: dest size does not match request mip size (dest: %u bytes, mip: %u bytes)",
		__FUNCTION__, size, mipSizeBytes );

	// Full Resolution
	byte *mipSrc = reinterpret_cast<byte *>( data );
	byte *mipDst = reinterpret_cast<byte *>( dest );
	memory_copy( mipDst, mipSrc, pixelSizeBytes * width * height );
	mipDst += pixelSizeBytes * width * height;

	// Generated Mips
	for( u16 level = 1, w = width, h = height; level < levels; level++ )
	{
		if( !mip_generate_next_2d( mipSrc, w, h, format, mipDst, ( pixelSizeBytes * w * h ) / 4 ) ) { return false; }
		w /= 2;
		h /= 2;
		mipSrc = mipDst;
		mipDst += pixelSizeBytes * w * h;
	}

	return true;
}


static bool mip_generate_chain_2d_alloc( void *data, const u16 width, const u16 height,
	const GfxColorFormat format, void *&outData, usize &outSize )
{
	// Generates a mip level half the size of width, height
	Assert( format < GFXCOLORFORMAT_COUNT );
	const u16 pixelSizeBytes = colorFormatPixelSizeBytes[format];

	ErrorReturnIf( data == nullptr, false,
		"%s: data is nullptr", __FUNCTION__ );
	ErrorReturnIf( outData != nullptr, false,
		"%s: outData is not nullptr", __FUNCTION__ );
	ErrorReturnIf( width <= 1 || height <= 1, false,
		"%s: cannot generate mip for input dimensions: %u x %u", __FUNCTION__, width, height );

	const u16 levels = mip_level_count_2d( width, height );

	outSize = mip_buffer_size_2d( width, height, levels, format );
	outData = memory_alloc( outSize );

	// Generate mips
	if( !mip_generate_chain_2d( data, width, height, format, outData, outSize ) )
	{
		memory_free( outData );
		outData = nullptr;
		outSize = 0;
		return false;
	}

	return true;
}

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
				ErrorIf( size > 4096,
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
	const u32 count = static_cast<u32>( textures.size() );

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
				texture.levels = 1;
				binary.write( textureBuffer.data, texture.width * texture.height * sizeof( rgba ) );
				sizeBytes += texture.width * texture.height * sizeof( rgba );

				//char path[PATH_SIZE];
				//strjoin( path, Build::pathOutput, SLASH "generated" SLASH, ( texture.name + "_atlas.png" ).cstr() );
				//textureBuffer.save( path );
			}
			// Independent Texture
			else if( numGlyphs == 1 )
			{
				Glyph &glyph = Assets::glyphs[texture.glyphs[0]];
				texture.width = glyph.textureBuffer.width;
				texture.height = glyph.textureBuffer.height;
				texture.offset = binary.tell;

				// Mipmapping
				texture.levels = mip_level_count_2d( texture.width, texture.height );
				Assert( texture.levels > 0 );
				void *mip = nullptr;
				usize size = 0;

				if( mip_generate_chain_2d_alloc( glyph.textureBuffer.data,
						texture.width, texture.height, GfxColorFormat_R8G8B8A8_FLOAT, mip, size ) )
				{
					// Write Binary
					binary.write( mip, size );
					sizeBytes += size;
					memory_free( mip );
				}
				else
				{
					binary.write( glyph.textureBuffer.data, texture.width * texture.height * sizeof( rgba ) );
					sizeBytes += texture.width * texture.height * sizeof( rgba );
				}

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

#if 1
		// Enums
		header.append( "enum_class_type\n(\n\tTexture, u32,\n\n" );
		for( Texture &texture : textures ) { header.append( "\t" ).append( texture.name ).append( ",\n" ); }
		header.append( "\n\tNull = 0,\n" );
		header.append( ");\n\n" );
#endif

		// Struct
		header.append( "namespace Assets { struct TextureEntry; }\n\n" );

		// Table
		header.append( "namespace CoreAssets\n{\n" );
		header.append( "\tconstexpr u32 textureCount = " ).append( count ).append( ";\n" );
		header.append( count > 0 ? "\textern const Assets::TextureEntry textures[];\n" :
			"\textern const Assets::TextureEntry *textures;\n" );
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
			source.append( "\tconst Assets::TextureEntry textures[textureCount] =\n\t{\n" );
			char buffer[PATH_SIZE];
			for( Texture &texture : textures )
			{
				snprintf( buffer, PATH_SIZE, "\t\t{ %lluULL, %u, %u, %u }, // %s\n",
					texture.offset,
					texture.width,
					texture.height,
					texture.levels,
					texture.name.cstr() );

				source.append( buffer );
			}
			source.append( "\t};\n" );
		}
		else
		{
			source.append( "\tconst Assets::TextureEntry *textures = nullptr;\n" );
		}

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