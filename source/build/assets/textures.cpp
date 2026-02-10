#include <build/assets/textures.hpp>

#include <vendor/math.hpp>
#include <vendor/stb/stb_image.hpp>

#include <core/list.hpp>
#include <core/json.hpp>
#include <core/checksum.hpp>

#include <build/build.hpp>
#include <build/assets.hpp>
#include <build/filesystem.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct CacheTextureEntry
{
	int width;
	int height;
	int channels;
};


struct CacheTextureBinary
{
	int width;
	int height;
	int channels;
	int levels;
	usize offset;
	usize size;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static Texture2DBuffer resourceNull;
static List<Texture2DBuffer> resourceList;
static HashMap<u32, usize> resourceMap;

Texture2DBuffer &texture_load_file( const char *path )
{
	const u32 key = Hash::hash( path );
	if( !resourceMap.contains( key ) )
	{
		Texture2DBuffer texture;
		if( texture.load( path ) )
		{
			resourceMap.set( key, resourceList.count() );
			return resourceList.add( static_cast<Texture2DBuffer &&>( texture ) );
		}
		else
		{
			return resourceNull;
		}
	}
	else
	{
		return resourceList.at( resourceMap.get( key ) );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum_type( GfxColorFormat, u8 )
{
	GfxColorFormat_NONE = 0,
	GfxColorFormat_R8G8B8A8_FLOAT,
	GfxColorFormat_R8G8B8A8_UINT,
	GfxColorFormat_R10G10B10A2_FLOAT,
	GfxColorFormat_R8_UINT,
	GfxColorFormat_R8G8,
	GfxColorFormat_R16_UINT,
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
	1,  // GfxColorFormat_R8_UINT
	2,  // GfxColorFormat_R8G8
	2,  // GfxColorFormat_R16_UINT
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
			case GfxColorFormat_R8_UINT:
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

			case GfxColorFormat_R16_UINT:
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

GlyphID Texture::add_glyph( const Glyph &glyph )
{
	glyphCacheKey.add( glyph.cacheKey );
	const GlyphID glyphID = Assets::glyphs.register_new( glyph );
	glyphs.add( glyphID );
	return glyphID;
}


GlyphID Texture::add_glyph( Glyph &&glyph )
{
	glyphCacheKey.add( glyph.cacheKey );
	const GlyphID glyphID = Assets::glyphs.register_new( static_cast<Glyph &&>( glyph ) );
	glyphs.add( glyphID );
	return glyphID;
}


struct Space
{
	Space( int x, int y, int w, int h ) : x(x), y(y), w(w), h(h) { }

	int x, y, w, h;

	inline int area() const { return w * h; }
	bool operator>( const Space& other ) const { return this->area() > other.area(); }
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
	quicksort_glyphs( &glyphs[0], &glyphs[glyphs.count() - 1], false );

	// Pack
	for( ;; )
	{
		// Initialize list of "empty spaces"
		List<Space> spaces;
		spaces.add( Space { 0, 0, size, size } );

		// Loop over Glyphs6
		for( GlyphID glyphID : glyphs )
		{
			Glyph &glyph = Assets::glyphs[glyphID];
			const u16 glyphWidth = glyph.imageX2 - glyph.imageX1;
			const u16 glyphHeight = glyph.imageY2 - glyph.imageY1;

			// Loop over spaces back to front (smallest spaces are at the back of the list)
			usize index = USIZE_MAX;
			for( usize i = spaces.count(); i > 0; i-- )
			{
				Space &space = spaces[i - 1];
				if( space.w >= glyphWidth + padding * 2 &&
					space.h >= glyphHeight + padding * 2 )
				{
					// Found a suitable space!
					index = i - 1;
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
			glyph.atlasX1 = space.x + padding;
			glyph.atlasY1 = space.y + padding;
			glyph.atlasX2 = glyph.atlasX1 + glyphWidth;
			glyph.atlasY2 = glyph.atlasY1 + glyphHeight;

			glyph.u1 = static_cast<u16>( glyph.atlasX1 / static_cast<float>( size ) * 65536.0f );
			glyph.v1 = static_cast<u16>( glyph.atlasY1 / static_cast<float>( size ) * 65536.0f );
			glyph.u2 = static_cast<u16>( glyph.atlasX2 / static_cast<float>( size ) * 65536.0f );
			glyph.v2 = static_cast<u16>( glyph.atlasY2 / static_cast<float>( size ) * 65536.0f );

			// Split space
			Space hSplit
			{
				space.x,
				space.y + glyphHeight + padding * 2,
				space.w,
				space.h - glyphHeight - padding * 2
			};

			Space vSplit
			{
				space.x + glyphWidth + padding * 2,
				space.y,
				space.w - glyphWidth - padding * 2,
				glyphHeight + padding * 2
			};

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

TextureID Textures::register_new( String &name )
{
	// NOTE: Registers a new, blank Texture resource
	for( usize i = 0; i < textures.count(); i++ )
	{
		Texture &texture = textures[i];
		if( texture.name.equals( name ) ) { return i; }
	}

	AssertMsg( textures.count() < TEXTUREID_MAX, "Exceeded max number of Textures" );
	Texture &texture = textures.add( Texture { } );
	texture.name = name;
	return static_cast<TextureID>( textures.count() - 1 );
}


TextureID Textures::register_new_from_definition( String name, const char *path )
{
	static char pathDirectory[PATH_SIZE];
	path_get_directory( pathDirectory, sizeof( pathDirectory ), path );

	// Register Definition File
	AssetFile fileDefinition;
	if( !asset_file_register( fileDefinition, path ) )
	{
		Error( "Unable to locate texture file: %s", path );
		return TEXTUREID_NULL;
	}

	// Open Definition JSON
	String fileDefinitionContents;
	if( !fileDefinitionContents.load( path ) )
	{
		Error( "Unable to open texture file: %s", path );
		return TEXTUREID_NULL;
	}
	JSON fileDefinitionJSON { fileDefinitionContents };

	// Parse Definition JSON
	static char pathImage[PATH_SIZE];
	String pathImageRelative = fileDefinitionJSON.get_string( "path" );
	ErrorIf( pathImageRelative.length_bytes() == 0,
		"Texture '%s' has an invalid image path (required)", fileDefinition.name );
	snprintf( pathImage, sizeof( pathImage ),
		"%s" SLASH "%s", pathDirectory, pathImageRelative.cstr() );
	bool generateMips = fileDefinitionJSON.get_bool( "mips" );

	// Register Color Image File
	AssetFile fileImage;
	if( !asset_file_register( fileImage, pathImage ) )
	{
		Error( "Texture '%s' - Unable to locate image file: '%s'", fileDefinition.name, pathImage );
		return TEXTUREID_NULL;
	}

	// Check Cache
	int width = 0;
	int height = 0;
	int channels = 0;
	CacheTextureEntry cacheTexture;
	const CacheKey cacheKey = Hash::hash64_from( fileDefinition.cache_id(), fileImage.cache_id() );
	if( Assets::cache.fetch( cacheKey, cacheTexture ) )
	{
		width = cacheTexture.width;
		height = cacheTexture.height;
		channels = cacheTexture.channels;
	}
	else
	{
		Assets::cache.dirty |= true; // Dirty Cache
		stbi_info( fileImage.path, &width, &height, &channels );
	}

	// Validate Image Dimensions
	ErrorIf( width <= 0 || height <= 0 || channels <= 0,
		"Texture '%s' has invalid dimensions: (w: %d, h: %d, c: %d)",
		width, height, channels );
	ErrorIf( width > U16_MAX || height > U16_MAX || channels > 4,
		"Texture '%s' has invalid dimensions: (w: %d, h: %d, c: %d)",
		width, height, channels );

	const TextureID textureID = register_new( name );
	Texture &texture = textures[textureID];
	texture.name = name;
	texture.atlasTexture = false;
	texture.generateMips = generateMips;

	Glyph glyph;
	glyph.cacheKey = cacheKey;
	glyph.texturePath = pathImage;
	glyph.imageX1 = static_cast<u16>( 0 );
	glyph.imageY1 = static_cast<u16>( 0 );
	glyph.imageX2 = static_cast<u16>( width );
	glyph.imageY2 = static_cast<u16>( height );
	texture.add_glyph( static_cast<Glyph &&>( glyph ) );

	// Cache
	cacheTexture.width = width;
	cacheTexture.height = height;
	cacheTexture.channels = channels;
	Assets::cache.store( cacheKey, cacheTexture );

	return textureID;
}


TextureID Textures::register_new_from_file( String &name, const char *path, bool generateMips )
{
	// NOTE: Registers a new texture resource loaded from a specified image file
	for( usize i = 0; i < textures.count(); i++ )
	{
		Texture &texture = textures[i];
		if( texture.name.equals( name ) ) { return i; }
	}

	AssetFile file;
	if( !asset_file_register( file, path ) ) { return TEXTUREID_NULL; }
	const CacheKey cacheKey = Hash::hash32_from( 0x82B0FF2E, file.cache_id() );

	int width = 0;
	int height = 0;
	int channels = 0;
	if( !stbi_info( path, &width, &height, &channels ) ) { return TEXTUREID_NULL; }

	AssertMsg( textures.count() < TEXTUREID_MAX, "Exceeded max number of Textures" );
	Texture &texture = textures.add( Texture { } );
	texture.name = name;
	texture.atlasTexture = false;
	texture.generateMips = generateMips;

	Glyph glyph;
	glyph.cacheKey = cacheKey;
	glyph.texturePath = path;
	glyph.imageX1 = static_cast<u16>( 0 );
	glyph.imageY1 = static_cast<u16>( 0 );
	glyph.imageX2 = static_cast<u16>( width );
	glyph.imageY2 = static_cast<u16>( height );
	texture.add_glyph( static_cast<Glyph &&>( glyph ) );

	return static_cast<TextureID>( textures.count() - 1 );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

usize Textures::gather( const char *path, bool recurse )
{
	List<FileInfo> files;
	directory_iterate( files, path, ".texture", recurse );

	static char name[PATH_SIZE];
	for( FileInfo &fileInfo : files )
	{
		Assets::cacheFileCount++;
		path_remove_extension( name, sizeof( name ), fileInfo.name );
		register_new_from_definition( name, fileInfo.path );
	}

	return files.count();
}


void Textures::build()
{
	Buffer &binary = Assets::binary;
	String &header = Assets::header;
	String &source = Assets::source;
	const u32 count = static_cast<u32>( textures.count() );

	Timer timer;
	usize sizeBytes = 0;

	// Load & Binary
	{
		for( Texture &texture : textures )
		{
			const u32 numGlyphs = texture.glyphs.count();
			Assert( numGlyphs > 0 );

			// Texture CacheKey
			const CacheKey cacheKey = static_cast<CacheKey>( checksum_xcrc32(
				reinterpret_cast<char *>( texture.glyphCacheKey.data ),
				texture.glyphCacheKey.count() * sizeof( CacheKey ), 0 ) );

			// Atlas Texture
			if( numGlyphs >= 1 && texture.atlasTexture )
			{
				// Pack Atlas
				texture.pack();

				// If the atlas is unchanged, read from the previous binary
				CacheTextureBinary cacheTextureBinary;
				if( Assets::cache.fetch( cacheKey, cacheTextureBinary ) )
				{
					// Read & Write Binary (Cached)
					texture.levels = 1;
					texture.offset = binary.write_from_file( Build::pathOutputRuntimeBinary,
						Assets::cacheReadOffset + cacheTextureBinary.offset, cacheTextureBinary.size );

					Assets::log_asset_cache( "Texture", texture.name.cstr() );
					ErrorIf( texture.offset == USIZE_MAX, "Failed to read cached texture from binary!" );

					// Cache
					cacheTextureBinary.offset = texture.offset;
					Assets::cache.store( cacheKey, cacheTextureBinary );
				}
				else
				{
					// Generate Texture Binary
					Texture2DBuffer textureBinary = Texture2DBuffer { texture.width, texture.height };

					for( GlyphID glyphID : texture.glyphs )
					{
						Glyph &glyph = Assets::glyphs[glyphID];
						Texture2DBuffer &textureGlyph = glyph.textureBuffer.data != nullptr ?
							glyph.textureBuffer : texture_load_file( glyph.texturePath );

						textureBinary.splice( textureGlyph,
							glyph.imageX1, glyph.imageY1, glyph.imageX2, glyph.imageY2,
							glyph.atlasX1, glyph.atlasY1 );
					}

				#if 0
					char path[PATH_SIZE];
					strjoin( path, Build::pathOutput, SLASH "generated" SLASH,
						( texture.name + "_atlas.png" ).cstr() );
					textureBuffer.save( path );
				#endif

					// Write Binary
					texture.levels = 1;
					const usize size = texture.width * texture.height * sizeof( rgba );
					texture.offset = binary.write( textureBinary.data, size );
					Assets::log_asset_build( "Texture", texture.name.cstr() );
					sizeBytes += size;

					// Cache
					cacheTextureBinary.width = texture.width;
					cacheTextureBinary.height = texture.height;
					cacheTextureBinary.levels = texture.levels;
					cacheTextureBinary.offset = texture.offset;
					cacheTextureBinary.size = size;
					Assets::cache.store( cacheKey, cacheTextureBinary );
				}
			}
			// Independent Texture
			else if( numGlyphs == 1 )
			{
				// If the atlas is unchanged, read from the previous binary
				CacheTextureBinary cacheTextureBinary;
				if( Assets::cache.fetch( cacheKey, cacheTextureBinary ) )
				{
					// Read & Write Binary (Cached)
					texture.width = cacheTextureBinary.width;
					texture.height = cacheTextureBinary.height;
					texture.levels = cacheTextureBinary.levels;
					texture.offset = binary.write_from_file( Build::pathOutputRuntimeBinary,
						Assets::cacheReadOffset + cacheTextureBinary.offset, cacheTextureBinary.size );

					Assets::log_asset_cache( "Texture", texture.name.cstr() );
					ErrorIf( texture.offset == USIZE_MAX, "Failed to read cached texture from binary!" );

					// Cache
					cacheTextureBinary.offset = texture.offset;
					Assets::cache.store( cacheKey, cacheTextureBinary );
				}
				else
				{
					// Generate Texture Binary
					Glyph &glyph = Assets::glyphs[texture.glyphs[0]];
					Texture2DBuffer &textureBinary = glyph.textureBuffer.data != nullptr ?
						glyph.textureBuffer : texture_load_file( glyph.texturePath );

					texture.width = textureBinary.width;
					texture.height = textureBinary.height;
					usize size = 0;

					// Mipmapping
					if( texture.generateMips )
					{
						texture.levels = mip_level_count_2d( texture.width, texture.height );
						Assert( texture.levels > 0 );
						void *mip = nullptr;

						if( mip_generate_chain_2d_alloc( textureBinary.data,
								texture.width, texture.height, GfxColorFormat_R8G8B8A8_FLOAT, mip, size ) )
						{
							texture.offset = binary.write( mip, size );
							Assets::log_asset_build( "Texture", texture.name.cstr() );
							sizeBytes += size;
							memory_free( mip );
						}
						else
						{
							Error( "Failed to generate mips for texture: %s (%u x %u)",
								texture.name.cstr(), texture.width, texture.height );
						}
					}
					else
					// No mipmaps -- write file directly as is
					{
						texture.levels = 1;
						size = texture.width * texture.height * sizeof( rgba );
						texture.offset = binary.write( textureBinary.data, size );
						sizeBytes += size;
					}

				#if 0
					char path[PATH_SIZE];
					strjoin( path, Build::pathOutput, SLASH "generated" SLASH,
						( texture.name + ".png" ).cstr() );
					textureGlyph.save( path );
				#endif

					// Cache
					cacheTextureBinary.width = texture.width;
					cacheTextureBinary.height = texture.height;
					cacheTextureBinary.levels = texture.levels;
					cacheTextureBinary.offset = texture.offset;
					cacheTextureBinary.size = size;
					Assets::cache.store( cacheKey, cacheTextureBinary );
				}
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
		header.append( "enum_class\n(\n\tTexture, u32,\n\n" );
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
				snprintf( buffer, PATH_SIZE,
					"\t\t{ BINARY_OFFSET_ASSETS + %lluLLU, %u, %u, %u }, // %s\n",
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
		const usize count = textures.count();
		Print( PrintColor_White, TAB TAB "Wrote %d texture%s", count, count == 1 ? "" : "s", MB( sizeBytes ) );
		PrintLn( PrintColor_White, " (%.3f ms)", timer.elapsed_ms() );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////