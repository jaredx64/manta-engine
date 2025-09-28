#include <manta/gfx.hpp>

#include <config.hpp>
#include <core/memory.hpp>
#include <pipeline.generated.hpp>

#include <manta/window.hpp>
#include <manta/draw.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static inline void draw_call()
{
#if GRAPHICS_ENABLED
	// TODO: Refactor this
	Gfx::quad_batch_break(); // Break the quad batch
	// TODO ... break any other batch?
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace CoreGfx
{
	GfxState states[2];
	bool flip = false;
	bool rendering = false;

	GfxShader shaders[CoreGfx::shaderCount];
	GfxTexture2D textures[CoreAssets::textureCount];

	GfxSwapChain swapchain;
	GfxViewport viewport;

	double_m44 matrixModel;
	double_m44 matrixView;
	double_m44 matrixViewInverse;
	double_m44 matrixPerspective;
	double_m44 matrixMVP;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if PROFILING_GFX
#include <manta/draw.hpp>

namespace Gfx
{
	GfxStatistics stats = { };
	GfxStatistics statsPrevious = { };
}


static void format_integer( char *buffer, usize size, int num )
{
	char tempBuffer[32];
	int length, i, j = 0;

	snprintf( tempBuffer, sizeof( tempBuffer ), "%d", num < 0 ? -num : num );
	length = static_cast<int>( strlen( tempBuffer ) );

	// Negative
	if( num < 0 )
	{
		buffer[0] = '-';
		j = 1;
	}

	// Commas
	for( i = 0; i < length; i++ )
	{
		buffer[j++] = tempBuffer[i];

		if( ( length - i - 1 ) % 3 == 0 && i != length - 1 && j < static_cast<int>( size ) - 1 )
		{
			buffer[j++] = ',';
		}

		if( j >= static_cast<int>( size - 1 ) )
		{
			buffer[j] = '\0';
			return;
		}
	}

	buffer[j] = '\0';
}

void debug_overlay_gfx( const float x, const float y )
{
	float drawX = x;
	float drawY = y;
	const GfxStatistics &stats = Gfx::stats;//Previous;

	char buffer[32];

	draw_text_f( fnt_iosevka, 14, drawX, drawY, c_white,
		"Draw Calls: %d", stats.frame.drawCalls );
	drawY += 20.0f;

	format_integer( buffer, sizeof( buffer ), stats.frame.vertexCount );
	draw_text_f( fnt_iosevka, 14, drawX, drawY, c_white,
		"Vertices: %s", buffer );
	drawY += 20.0f;

	format_integer( buffer, sizeof( buffer ), stats.frame.vertexCount / 3 );
	draw_text_f( fnt_iosevka, 14, drawX, drawY, c_white,
		"Triangles: %s", buffer );
	drawY += 20.0f;

	draw_text_f( fnt_iosevka, 14, drawX, drawY, c_white,
		"Texture Binds: %d", stats.frame.textureBinds );
	drawY += 20.0f;

	draw_text_f( fnt_iosevka, 14, drawX, drawY, c_white,
		"Shader Binds: %d", stats.frame.shaderBinds );
	drawY += 20.0f;

	drawY += 20.0f;
	draw_text_f( fnt_iosevka, 14, drawX, drawY, c_yellow,
		"GPU Memory Total: %.4f mb", MB( stats.total_memory() ) );
	drawY += 20.0f;

	draw_text_f( fnt_iosevka, 14, drawX, drawY, c_white,
		"  Swapchain: %.4f mb", MB( stats.gpuMemorySwapchain ) );
	drawY += 20.0f;

	draw_text_f( fnt_iosevka, 14, drawX, drawY, c_white,
		"  Textures: %.4f mb", MB( stats.gpuMemoryTextures ) );
	drawY += 20.0f;

	draw_text_f( fnt_iosevka, 14, drawX, drawY, c_white,
		"  Render Targets: %.4f mb", MB( stats.gpuMemoryRenderTargets ) );
	drawY += 20.0f;

	draw_text_f( fnt_iosevka, 14, drawX, drawY, c_white,
		"  Vertex Buffers: %.4f mb", MB( stats.gpuMemoryVertexBuffers ) );
	drawY += 20.0f;

	draw_text_f( fnt_iosevka, 14, drawX, drawY, c_white,
		"  Instance Buffers: %.4f mb", MB( stats.gpuMemoryInstanceBuffers ) );
	drawY += 20.0f;

	draw_text_f( fnt_iosevka, 14, drawX, drawY, c_white,
		"  Index Buffers: %.4f mb", MB( stats.gpuMemoryIndexBuffers ) );
	drawY += 20.0f;

	draw_text_f( fnt_iosevka, 14, drawX, drawY, c_white,
		"  Constant Buffers: %.4f mb", MB( stats.gpuMemoryUniformBuffers ) );
	drawY += 20.0f;

	draw_text_f( fnt_iosevka, 14, drawX, drawY, c_white,
		"  Shader Programs: %.4f mb", MB( stats.gpuMemoryShaderPrograms ) );
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CoreGfx::init()
{
#if GRAPHICS_ENABLED
	// Initialize Backend
	ErrorIf( !CoreGfx::rb_init(), "%s: Failed to initialize %s backend!", __FUNCTION__, BUILD_GRAPHICS );

	// Initialize Textures
	ErrorIf( !CoreGfx::init_textures(), "%s: Failed to initialize textures!", __FUNCTION__ );

	// Initialize Shaders
	ErrorIf( !CoreGfx::init_shaders(), "%s: Failed to initialize shaders!", __FUNCTION__ );

	// Initialize Constant Buffers
 	ErrorIf( !CoreGfx::rb_init_uniform_buffers(), "%s: Failed to initialize constant buffers!", __FUNCTION__ );

	// Initialize Quad Batch
	ErrorIf( !CoreGfx::quadBatch.init( RENDER_QUAD_BATCH_SIZE ),
		"%s: Failed to initialize default quad batch!", __FUNCTION__ );
#endif

	// Success
	return true;
}


bool CoreGfx::free()
{
#if GRAPHICS_ENABLED
	// Free Quad Batch
	ErrorIf( !CoreGfx::quadBatch.free(), "%s: Failed to free default quad batch!", __FUNCTION__ );

	// Free Constant Buffers
	ErrorIf( !CoreGfx::rb_free_uniform_buffers(), "%s: Failed to free shaders!", __FUNCTION__ );

	// Free Shaders
	ErrorIf( !CoreGfx::free_shaders(), "%s: Failed to free shaders!", __FUNCTION__ );

	// Free Textures
	ErrorIf( !CoreGfx::free_textures(), "%s: Failed to free textures!", __FUNCTION__ );

	// Free Backend
	ErrorIf( !CoreGfx::rb_free(), "%s: Failed to free %s backend!", __FUNCTION__, BUILD_GRAPHICS );
#endif

	// Success
	return true;
}


bool CoreGfx::init_textures()
{
#if GRAPHICS_ENABLED
	// Load Textures
	for( u32 i = 0; i < CoreAssets::textureCount; i++ )
	{
		const Assets::TextureEntry &binTexture = Assets::texture( i );

		CoreGfx::textures[i].init( Assets::binary.data + binTexture.offset,
			binTexture.width, binTexture.height, binTexture.levels,
			GfxColorFormat_R8G8B8A8_FLOAT );
	}
#endif

	// Success
	return true;
}


bool CoreGfx::free_textures()
{
#if GRAPHICS_ENABLED
	// Free Textures
	for( u32 i = 0; i < CoreAssets::textureCount; i++ )
	{
		CoreGfx::textures[i].free();
	}
#endif

	// Success
	return true;
}


bool CoreGfx::init_shaders()
{
#if GRAPHICS_ENABLED
	// Load Shaders
	for( u32 i = 0; i < CoreGfx::shaderCount; i++ )
	{
		const ShaderEntry &shaderEntry = CoreGfx::shaderEntries[i];
		CoreGfx::shaders[i].init( i, shaderEntry );
	}
#endif

	// Success
	return true;
}


bool CoreGfx::free_shaders()
{
#if GRAPHICS_ENABLED
	// Free Shaders
	for( u32 i = 0; i < CoreGfx::shaderCount; i++ )
	{
		CoreGfx::shaders[i].free();
	}
#endif

	// Success
	return true;
}


void CoreGfx::state_reset()
{
#if GRAPHICS_ENABLED
	CoreGfx::states[0] = { };
	CoreGfx::states[1] = { };
	CoreGfx::flip = false;

	// Default Shader (TODO: Refactor?)
	Gfx::shader_bind( Shader::SHADER_DEFAULT );

	// Update State
	CoreGfx::state_apply( true );
#endif
}


void CoreGfx::state_apply( const bool dirty )
{
#if GRAPHICS_ENABLED
	#define GFX_STATE_CHECK(condition) UNLIKELY( ( condition ) )

	// This function runs at every draw call
	// It is responsible for making sure that the GPU render state reflects the CPU render state
	GfxState &current = CoreGfx::states[CoreGfx::flip];
	GfxState &previous = CoreGfx::states[!CoreGfx::flip];

	// Raster State
	if( dirty || GFX_STATE_CHECK( current.raster != previous.raster ) )
		{ CoreGfx::rb_set_raster_state( current.raster ); }

	// Sampler State
	if( dirty || GFX_STATE_CHECK( current.sampler != previous.sampler ) )
		{ CoreGfx::rb_set_sampler_state( current.sampler ); }

	// Blend State
	if( dirty || GFX_STATE_CHECK( current.blend != previous.blend ) )
		{ CoreGfx::rb_set_blend_state( current.blend ); }

	// Depth State
	if( dirty || GFX_STATE_CHECK( current.depth != previous.depth ) )
		{ CoreGfx::rb_set_depth_state( current.depth ); }

#if 0
	// Texture 2D Binding
	if( GFX_STATE_CHECK( current.textureResource[0] != previous.textureResource[0] &&
	                     current.textureResource[0] != nullptr ) )
		{ CoreGfx::rb_texture_2d_bind( current.textureResource[0], 0 ); }
#endif

	// Cache State
	previous = current;
	CoreGfx::flip = !CoreGfx::flip;
#endif
}


byte *CoreGfx::scratch_buffer( const usize size )
{
#if GRAPHICS_ENABLED
	static byte *scratchData = nullptr;
	static usize scratchSize = 0;

	if( size == 0 )
	{
		scratchSize = 0;
		if( scratchData != nullptr ) { memory_free( scratchData ); }
		scratchData = nullptr;
		return scratchData;
	}
	else
	{
		if( scratchData == nullptr )
		{
			scratchSize = size;
			scratchData = reinterpret_cast<byte *>( memory_alloc( size ) );
		}
		else if( size > scratchSize )
		{
			scratchSize = size;
			scratchData = reinterpret_cast<byte *>( memory_realloc( scratchData, scratchSize ) );
		}

		memory_set( scratchData, 0, scratchSize );
		return scratchData;
	}
#endif
	return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

u16 Gfx::mip_level_count_2d( u16 width, u16 height )
{
	u16 l = 1;
	while( width > 1 && height > 1 ) { width >>= 1; height >>= 1; l++; }
	return l;
}


bool Gfx::mip_level_validate_2d( u16 width, u16 height, u16 levels )
{
	return levels <= mip_level_count_2d( width, height );
}


usize Gfx::mip_buffer_size_2d( u16 width, u16 height, u16 levels,
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

	return CoreGfx::colorFormatPixelSizeBytes[format] * count;
}


bool Gfx::mip_generate_next_2d( void *data, const u16 width, const u16 height,
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

	const u16 pixelSizeBytes = CoreGfx::colorFormatPixelSizeBytes[format];
	const usize mipSizeBytes = pixelSizeBytes * mipWidth * mipHeight;

	ErrorReturnIf( size != mipSizeBytes, false,
		"%s: dest size does not match request mip size (dst: %u bytes, mip: %u bytes)",
		__FUNCTION__, size, mipSizeBytes );

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


bool Gfx::mip_generate_next_2d_alloc( void *data, const u16 width, const u16 height,
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

	const u16 pixelSizeBytes = CoreGfx::colorFormatPixelSizeBytes[format];
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


bool Gfx::mip_generate_chain_2d( void *data, const u16 width, const u16 height,
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
	const u16 pixelSizeBytes = CoreGfx::colorFormatPixelSizeBytes[format];
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


bool Gfx::mip_generate_chain_2d_alloc( void *data, const u16 width, const u16 height,
	const GfxColorFormat format, void *&outData, usize &outSize )
{
	// Generates a mip level half the size of width, height
	Assert( format < GFXCOLORFORMAT_COUNT );
	const u16 pixelSizeBytes = CoreGfx::colorFormatPixelSizeBytes[format];

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

namespace CoreGfx
{
	GfxQuadBatch<GfxVertex::BuiltinVertex> quadBatch;
}


bool Gfx::quad_batch_break()
{
#if GRAPHICS_ENABLED
	return CoreGfx::quadBatch.batch_break();
#endif
	return false;
}


bool Gfx::quad_batch_can_break()
{
#if GRAPHICS_ENABLED
	return CoreGfx::quadBatch.can_break();
#else
	return true;
#endif
}


void Gfx::quad_batch_break_check()
{
#if GRAPHICS_ENABLED
	CoreGfx::quadBatch.break_check();
#endif
}


void Gfx::quad_batch_write( const GfxQuadBatch<GfxVertex::BuiltinVertex>::Quad &quad,
	const GfxTexture2D *const texture )
{
#if GRAPHICS_ENABLED
	if( LIKELY( texture != nullptr ) ) { texture->bind( 0 ); }
	CoreGfx::quadBatch.write( quad );
#endif
}


void Gfx::quad_batch_write( const float x1, const float y1, const float x2, const float y2,
	const u16 u1, const u16 v1, const u16 u2, const u16 v2,
	const Color c1, const Color c2, const Color c3, const Color c4,
	const GfxTexture2D *const texture, const float depth )
{
#if GRAPHICS_ENABLED
	if( LIKELY( texture != nullptr ) ) { texture->bind( 0 ); }

	const GfxQuadBatch<GfxVertex::BuiltinVertex>::Quad quad =
	{
		{ { x1, y1, depth }, { u1, v1 }, { c1.r, c1.g, c1.b, c1.a } },
		{ { x2, y1, depth }, { u2, v1 }, { c2.r, c2.g, c2.b, c2.a } },
		{ { x1, y2, depth }, { u1, v2 }, { c3.r, c3.g, c3.b, c3.a } },
		{ { x2, y2, depth }, { u2, v2 }, { c4.r, c4.g, c4.b, c4.a } },
	};

	CoreGfx::quadBatch.write( quad );
#endif
}


void Gfx::quad_batch_write( const float x1, const float y1, const float x2, const float y2,
	const float x3, const float y3, const float x4, const float y4,
	const u16 u1, const u16 v1, const u16 u2, const u16 v2,
	const Color c1, const Color c2, const Color c3, const Color c4,
	const GfxTexture2D *const texture, const float depth )
{
#if GRAPHICS_ENABLED
	if( LIKELY( texture != nullptr ) ) { texture->bind( 0 ); }

	const GfxQuadBatch<GfxVertex::BuiltinVertex>::Quad quad =
	{
		{ { x1, y1, depth }, { u1, v1 }, { c1.r, c1.g, c1.b, c1.a } },
		{ { x2, y2, depth }, { u2, v1 }, { c2.r, c2.g, c2.b, c2.a } },
		{ { x3, y3, depth }, { u1, v2 }, { c3.r, c3.g, c3.b, c3.a } },
		{ { x4, y4, depth }, { u2, v2 }, { c4.r, c4.g, c4.b, c4.a } },
	};

	CoreGfx::quadBatch.write( quad );
#endif
}


void Gfx::quad_batch_write( const float x1, const float y1, const float x2, const float y2,
                            const u16 u1, const u16 v1, const u16 u2, const u16 v2, const Color color,
                            const GfxTexture2D *const texture, const float depth )
{
#if GRAPHICS_ENABLED
	if( LIKELY( texture != nullptr ) ) { texture->bind( 0 ); }

	const GfxQuadBatch<GfxVertex::BuiltinVertex>::Quad quad =
	{
		{ { x1, y1, depth }, { u1, v1 }, { color.r, color.g, color.b, color.a } },
		{ { x2, y1, depth }, { u2, v1 }, { color.r, color.g, color.b, color.a } },
		{ { x1, y2, depth }, { u1, v2 }, { color.r, color.g, color.b, color.a } },
		{ { x2, y2, depth }, { u2, v2 }, { color.r, color.g, color.b, color.a } },
	};

	CoreGfx::quadBatch.write( quad );
#endif
}


void Gfx::quad_batch_write( const float x1, const float y1, const float x2, const float y2,
	const float x3, const float y3, const float x4, const float y4,
	const u16 u1, const u16 v1, const u16 u2, const u16 v2, const Color color,
	const GfxTexture2D *const texture, const float depth )
{
#if GRAPHICS_ENABLED
	if( LIKELY( texture != nullptr ) ) { texture->bind( 0 ); }

	const GfxQuadBatch<GfxVertex::BuiltinVertex>::Quad quad =
	{
		{ { x1, y1, depth }, { u1, v1 }, { color.r, color.g, color.b, color.a } },
		{ { x2, y2, depth }, { u2, v1 }, { color.r, color.g, color.b, color.a } },
		{ { x3, y3, depth }, { u1, v2 }, { color.r, color.g, color.b, color.a } },
		{ { x4, y4, depth }, { u2, v2 }, { color.r, color.g, color.b, color.a } },
	};

	CoreGfx::quadBatch.write( quad );
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GfxTexture2D::init( void *data, const u16 width, const u16 height, const GfxColorFormat &format )
{
#if GRAPHICS_ENABLED
	ErrorIf( !CoreGfx::rb_texture_2d_init( resource, data, width, height, 1, format ),
		"Failed to init GfxTexture2D!" );
#endif
}


void GfxTexture2D::init( void *data, const u16 width, const u16 height, const u16 levels, const GfxColorFormat &format )
{
#if GRAPHICS_ENABLED
	ErrorIf( levels == 0,
		"Must have at least one mip level (highest resolution)" );
	ErrorIf( !CoreGfx::rb_texture_2d_init( resource, data, width, height, levels, format ),
		"Failed to init GfxTexture2D!" );
#endif
}


void GfxTexture2D::free()
{
#if GRAPHICS_ENABLED
	if( resource == nullptr ) { return; }
	ErrorIf( !CoreGfx::rb_texture_2d_free( resource ),
		"Failed to free GfxTexture2D!" );
#endif
}


void GfxTexture2D::bind( const int slot ) const
{
#if GRAPHICS_ENABLED
	if( Gfx::state().textureResource[slot] == resource ) { return; }

	// Texture binding forces a batch break
	draw_call();

	Gfx::state().textureResource[slot] = resource;
	ErrorIf( !CoreGfx::rb_texture_2d_bind( resource, slot ),
		"Failed to bind GfxTexture2D to slot %d!", slot );
#endif
}


void GfxTexture2D::release() const
{
#if GRAPHICS_ENABLED
	for( int slot = 0; slot < GFX_TEXTURE_SLOT_COUNT; slot++ )
	{
		if( Gfx::state().textureResource[slot] == resource )
		{
			Gfx::state().textureResource[slot] = nullptr;
			ErrorIf( !CoreGfx::rb_texture_2d_release( resource, slot ),
				"Failed to release GfxTexture2D from slot %d!", slot );
		}
	}
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static double_m44 RT_CACHE_MATRIX_MODEL;
static double_m44 RT_CACHE_MATRIX_VIEW;
static double_m44 RT_CACHE_MATRIX_PERSPECTIVE;
static GfxViewport RT_CACHE_VIEWPORT;
static GfxBlendState RT_CACHE_BLEND_STATE;


void GfxRenderTarget2D::init( const u16 width, const u16 height, const GfxRenderTargetDescription &desc )
{
#if GRAPHICS_ENABLED
	ErrorIf( !CoreGfx::rb_render_target_2d_init( resource,
		textureColor.resource, textureDepth.resource, width, height, desc ),
		"Failed to init RenderTarget2D!" );

	this->width = width;
	this->height = height;
	this->desc = desc;
	this->slot = -1;
#endif
}


void GfxRenderTarget2D::free()
{
#if GRAPHICS_ENABLED
	if( resource == nullptr ) { return; }
	ErrorIf( !CoreGfx::rb_render_target_2d_free( resource, textureColor.resource, textureDepth.resource ),
		"Failed to free RenderTarget2D!" );
#endif
}


void GfxRenderTarget2D::resize( const u16 width, const u16 height )
{
#if GRAPHICS_ENABLED
	ErrorIf( resource == nullptr, "Attempting to resize RenderTarget2D that does not exist!" );
	free();
	init( width, height, desc );
#endif
}


void GfxRenderTarget2D::copy( GfxRenderTarget2D &source )
{
#if GRAPHICS_ENABLED
	ErrorIf( resource == nullptr, "Attempting to copy GfxRenderTarget2D that does not exist!" );
	ErrorIf( source.resource == nullptr, "Attempting to copy from GfxRenderTarget2D that does not exist!" );

	const bool success = CoreGfx::rb_render_target_2d_copy(
		source.resource, source.textureColor.resource, source.textureDepth.resource,
		resource, textureColor.resource, textureDepth.resource );

	ErrorIf( !success, "Copy GfxRenderTarget2D failed!" );
#endif
}


void GfxRenderTarget2D::copy_part( GfxRenderTarget2D &source, int srcX, int srcY, int dstX, int dstY, u16 w, u16 h )
{
#if GRAPHICS_ENABLED
	ErrorIf( resource == nullptr, "Attempting to copy GfxRenderTarget2D that does not exist!" );
	ErrorIf( source.resource == nullptr, "Attempting to copy from GfxRenderTarget2D that does not exist!" );

	// Clamp destination coordinates to the destination surface
	if( dstX < 0 ) { srcX -= dstX; w += dstX; w = max( 0, static_cast<int>( this->width ) ); }
	const u16 dstX1 = static_cast<u16>( clamp( dstX, 0, static_cast<int>( this->width ) ) );
	const u16 dstX2 = static_cast<u16>( clamp( dstX1 + w, 0, static_cast<int>( this->width ) ) );

	if( dstY < 0 ) { srcY -= dstY; h += dstY; h = max( 0, static_cast<int>( this->height ) ); }
	const u16 dstY1 = static_cast<u16>( clamp( dstY, 0, static_cast<int>( this->height ) ) );
	const u16 dstY2 = static_cast<u16>( clamp( dstY1 + h, 0, static_cast<int>( this->height ) ) );

	// Clamp source coorindates to the source surface
	const u16 srcX1 = static_cast<u16>( clamp( srcX, 0, static_cast<int>( source.width ) ) );
	const u16 srcX2 = static_cast<u16>( clamp( srcX1 + w, 0, static_cast<int>( source.width ) ) );
	const u16 srcY1 = static_cast<u16>( clamp( srcY, 0, static_cast<int>( source.height ) ) );
	const u16 srcY2 = static_cast<u16>( clamp( srcY1 + h, 0, static_cast<int>( source.height ) ) );

	// Determine the true, clamped copy dimensions
	const u16 copyWidth = static_cast<u16>( min( srcX2 - srcX1, dstX2 - dstX1 ) );
	const u16 copyHeight = static_cast<u16>( min( srcY2 - srcY1, dstY2 - dstY1 ) );
	if( copyWidth == 0 || copyHeight == 0 ) { return; } // Early exit if we're copying nothing

	// Manual Depth Buffer Copying
#if GRAPHICS_D3D11
	if( this->textureDepth.resource && source.textureDepth.resource )
	{
		Gfx::shader_bind( Shader::SHADER_DEFAULT_COPY_DEPTH );
		Gfx::set_depth_test_mode( GfxDepthTestMode_ALWAYS );
		this->bind();
		source.textureDepth.bind( 0 );
		{
			const u16 srcY1Inv = source.height - srcY1;

			const u16 u1 =
				static_cast<u16>( ( ( srcX1 ) / static_cast<float>( source.width ) ) * 0xFFFF );
			const u16 v1 =
				static_cast<u16>( ( ( srcY1 ) / static_cast<float>( source.height ) ) * 0xFFFF );
			const u16 u2 =
				static_cast<u16>( ( ( srcX1 + copyWidth ) / static_cast<float>( source.width ) ) * 0xFFFF );
			const u16 v2 =
				static_cast<u16>( ( ( srcY1 + copyHeight ) / static_cast<float>( source.height ) ) * 0xFFFF );

			draw_quad_uv( dstX1, dstY1, dstX1 + copyWidth, dstY1 + copyHeight, u1, v1, u2, v2 );
		}
		source.textureDepth.release();
		this->release();
		Gfx::set_depth_test_mode( GfxDepthTestMode_NONE );
		Gfx::shader_release();
	}
#endif

	// Copy Part
	const bool success = CoreGfx::rb_render_target_2d_copy_part(
		source.resource, source.textureColor.resource, source.textureDepth.resource,
		resource, textureColor.resource, textureDepth.resource,
		srcX1, srcY1, dstX1, dstY1, copyWidth, copyHeight );

	ErrorIf( !success, "Copy GfxRenderTarget2D failed!" );
#endif
}


void GfxRenderTarget2D::bind( const int slot )
{
#if GRAPHICS_ENABLED
	AssertMsg( this->slot < 0, "%d", this->slot );

	// Render target binding forces draw call
	draw_call();

	// Backend
	ErrorIf( !CoreGfx::rb_render_target_2d_bind( resource, slot ),
		"Failed to bind RenderTarget2D!" );

	if( slot == 0 )
	{
		// MVP Matrix
		RT_CACHE_MATRIX_MODEL = Gfx::get_matrix_model();
		Gfx::set_matrix_model( double_m44_build_identity() );
		RT_CACHE_MATRIX_VIEW = Gfx::get_matrix_view();
		Gfx::set_matrix_view( double_m44_build_identity() );
		RT_CACHE_MATRIX_PERSPECTIVE = Gfx::get_matrix_perspective();
		Gfx::set_matrix_perspective( double_m44_build_orthographic( 0.0, width, 0.0, height, 0.0, 1.0 ) );

		// Viewport
		RT_CACHE_VIEWPORT = CoreGfx::viewport;
		Gfx::set_viewport_size( width, height, false );

		// D3D11 needs to explicitly update depth test state here
		CoreGfx::rb_set_depth_state( Gfx::state().depth );

		// Blend State
		RT_CACHE_BLEND_STATE = Gfx::state().blend;
		Gfx::set_blend_enabled( true );
		Gfx::set_blend_mode_color( GfxBlendFactor_ONE, GfxBlendFactor_INV_SRC_ALPHA, GfxBlendOperation_ADD );

		// Apply State
		CoreGfx::state_apply();
	}

	// Set Slot
	this->slot = slot;
#endif
}


void GfxRenderTarget2D::release()
{
#if GRAPHICS_ENABLED
	Assert( this->slot >= 0 );

	// Render target releasing forces draw call
	draw_call();

	// Backend
	ErrorIf( !CoreGfx::rb_render_target_2d_release( this->resource, this->slot ),
		"Failed to release RenderTarget2D!" );

	if( this->slot == 0 )
	{
		// MVP Matrix
		Gfx::set_matrix_model( RT_CACHE_MATRIX_MODEL );
		Gfx::set_matrix_view( RT_CACHE_MATRIX_VIEW );
		Gfx::set_matrix_perspective( RT_CACHE_MATRIX_PERSPECTIVE );

		// Viewport
		Gfx::set_viewport_size( RT_CACHE_VIEWPORT.width, RT_CACHE_VIEWPORT.height, RT_CACHE_VIEWPORT.fullscreen );

		// Blend State
		Gfx::set_blend_state( RT_CACHE_BLEND_STATE );

		// Apply State
		CoreGfx::state_apply();
	}

	// Reset slot
	this->slot = -1;
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GfxShader::init( const u32 shaderID, const ShaderEntry &shaderEntry )
{
#if GRAPHICS_ENABLED
	this->shaderID = shaderID;
	ErrorIf( !CoreGfx::rb_shader_init( resource, shaderID, shaderEntry ),
		"Failed to init shader! (%u)", this->shaderID );
#endif
}


void GfxShader::free()
{
#if GRAPHICS_ENABLED
	ErrorIf( !CoreGfx::rb_shader_free( resource ),
		"Failed to free shader! (%u)", this->shaderID );
#endif
}


void GfxShader::bind()
{
#if GRAPHICS_ENABLED
	if( Gfx::state().shader.resource != resource )
	{
		draw_call(); // Shader changes force batch break
		Gfx::state().shader.resource = resource;
		Gfx::state().shader.shaderID = shaderID;
	}

	ErrorIf( !CoreGfx::rb_shader_bind( resource ),
		"Failed to bind shader!" );
	ErrorIf( !CoreGfx::rb_shader_bind_uniform_buffers_vertex[shaderID](),
		"Failed to bind vertex shader uniform buffers! (%u)", this->shaderID );
	ErrorIf( !CoreGfx::rb_shader_bind_uniform_buffers_fragment[shaderID](),
		"Failed to bind fragment shader uniform buffers! (%u)", this->shaderID );
	ErrorIf( !CoreGfx::rb_shader_bind_uniform_buffers_compute[shaderID](),
		"Failed to bind compute shader uniform buffers! (%u)", this->shaderID );
#endif
}


void GfxShader::release()
{
	CoreGfx::shaders[Shader::SHADER_DEFAULT].bind();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Gfx::viewport_update()
{
#if GRAPHICS_ENABLED
	// Update Viewport
	const int viewportWidth = static_cast<int>( Window::width * Window::scale );
	const int viewportHeight = static_cast<int>( Window::height * Window::scale );
	CoreGfx::rb_viewport_resize( viewportWidth, viewportHeight, Window::fullscreen );

	// Resize Swapchain
	CoreGfx::rb_swapchain_resize( Window::width, Window::height, Window::fullscreen );
#endif
}


void Gfx::frame_begin()
{
#if GRAPHICS_ENABLED
	// Reset State
	CoreGfx::state_reset();

	// Backend
	CoreGfx::rb_frame_begin();

	// Quad Batch
	CoreGfx::quadBatch.batch_begin();

	// Reset Matrices
	const double_m44 identity = double_m44_build_identity();
	Gfx::set_matrix_model( identity );
	Gfx::set_matrix_view( identity );
	Gfx::set_matrix_perspective( identity );

	// Start Rendering
	CoreGfx::rendering = true;
#endif
}


void Gfx::frame_end()
{
#if GRAPHICS_ENABLED
	// Stop Rendering
	CoreGfx::rendering = false;

	// Quad Batch
	CoreGfx::quadBatch.batch_end();

	// Backend
	CoreGfx::rb_frame_end();

	// Statistics
#if PROFILING_GFX
	memory_copy( &Gfx::statsPrevious, &Gfx::stats, sizeof( GfxStatistics ) );
	Gfx::stats.frame = { };
#endif
#endif
}


void Gfx::clear_color( const Color color )
{
#if GRAPHICS_ENABLED
	CoreGfx::rb_clear_color( color );
#endif
}


void Gfx::clear_depth( const float depth )
{
#if GRAPHICS_ENABLED
	CoreGfx::rb_clear_depth( depth );
#endif
}


void Gfx::set_swapchain_size( const u16 width, const u16 height, const bool fullscreen )
{
#if GRAPHICS_ENABLED
	CoreGfx::rb_swapchain_resize( width, height, fullscreen );
#endif
}


void Gfx::set_viewport_size( const u16 width, const u16 height, const bool fullscreen )
{
#if GRAPHICS_ENABLED
	CoreGfx::rb_viewport_resize( width, height, fullscreen );
#endif
}


void Gfx::set_shader_globals( const CoreGfxUniformBuffer::ShaderGlobals_t &globals )
{
#if GRAPHICS_ENABLED
	// Shader globals changes force a batch break
	if( Gfx::state().shader.globals == globals ) { return; }
	Gfx::state().shader.globals = globals;

	draw_call();
	globals.upload();
#endif
}


static void update_matrix_mvp()
{
#if GRAPHICS_ENABLED
	CoreGfx::matrixMVP = double_m44_multiply( CoreGfx::matrixPerspective,
		double_m44_multiply( CoreGfx::matrixView, CoreGfx::matrixModel ) );

	auto &globals = GfxUniformBuffer::ShaderGlobals;
	globals.matrixModel = float_m44_from_double_m44( CoreGfx::matrixModel );
	globals.matrixView = float_m44_from_double_m44( CoreGfx::matrixView );
	globals.matrixPerspective = float_m44_from_double_m44( CoreGfx::matrixPerspective );
	globals.matrixMVP = float_m44_from_double_m44( CoreGfx::matrixMVP );

	Gfx::set_shader_globals( globals );
#endif
};


void Gfx::set_matrix_model( const double_m44 &matrix )
{
#if GRAPHICS_ENABLED
	CoreGfx::matrixModel = matrix;
	update_matrix_mvp();
#endif
}


void Gfx::set_matrix_view( const double_m44 &matrix )
{
#if GRAPHICS_ENABLED
	CoreGfx::matrixView = matrix;
	update_matrix_mvp();
#endif
}


void Gfx::set_matrix_perspective( const double_m44 &matrix )
{
#if GRAPHICS_ENABLED
	CoreGfx::matrixPerspective = matrix;
	update_matrix_mvp();
#endif
}


void Gfx::set_matrix_mvp( const double_m44 &matModel, const double_m44 &matView, const double_m44 &matPerspective )
{
#if GRAPHICS_ENABLED
	CoreGfx::matrixModel = matModel;
	CoreGfx::matrixView = matView;
	CoreGfx::matrixPerspective = matPerspective;
	update_matrix_mvp();
#endif
}


void Gfx::set_matrix_mvp_2d_orthographic( const double x, const double y, const double zoom, const double angle,
	const double width, const double height, const double znear, const double zfar )
{
#if GRAPHICS_ENABLED
	CoreGfx::matrixModel = double_m44_build_identity();
	double_m44 matrixView = double_m44_build_rotation_z( angle * DEG2RAD );
	matrixView = double_m44_multiply( double_m44_build_scaling( zoom, zoom, 1.0 ), matrixView );
	matrixView = double_m44_multiply( double_m44_build_translation( -x, -y, 0.0 ), matrixView );
	CoreGfx::matrixView = matrixView;
	CoreGfx::matrixPerspective = double_m44_build_orthographic( 0.0, width, 0.0, height, znear, zfar );
	update_matrix_mvp();
#endif
}


void Gfx::set_matrix_mvp_3d_perspective( const double fov, const double aspect, const double znear, const double zfar,
	const double x, const double y, const double z, const double xto, const double yto,
	const double zto, const double xup, const double yup, const double zup )
{
#if GRAPHICS_ENABLED
	AssertMsg( znear > 0.0, "znear must be > 0.0f!" );
	CoreGfx::matrixModel = double_m44_build_identity();
	CoreGfx::matrixView = double_m44_build_lookat( x, y, z, xto, yto, zto, xup, yup, zup );
	CoreGfx::matrixPerspective = double_m44_build_perspective( fov, aspect, znear, zfar );
	update_matrix_mvp();
#endif
}


const double_m44 &Gfx::get_matrix_model()
{
	return CoreGfx::matrixModel;
}


const double_m44 &Gfx::get_matrix_view()
{
	return CoreGfx::matrixView;
}


const double_m44 &Gfx::get_matrix_perspective()
{
	return CoreGfx::matrixPerspective;
}


const double_m44 &Gfx::get_matrix_mvp()
{
	return CoreGfx::matrixMVP;
}


void Gfx::set_raster_state( const GfxRasterState &state )
{
#if GRAPHICS_ENABLED
	// Raster state changes force a batch break
	if( Gfx::state().raster == state ) { return; }
	draw_call();
	Gfx::state().raster = state;
#endif
}


void Gfx::set_fill_mode( const GfxFillMode &mode )
{
#if GRAPHICS_ENABLED
	GfxRasterState state = Gfx::state().raster;
	state.fillMode = mode;
	Gfx::set_raster_state( state );
#endif
}


void Gfx::set_cull_mode( const GfxCullMode &mode )
{
#if GRAPHICS_ENABLED
	GfxRasterState state = Gfx::state().raster;
	state.cullMode = mode;
	Gfx::set_raster_state( state );
#endif
}


void Gfx::set_scissor( const int x1, const int y1, const int x2, const int y2 )
{
#if GRAPHICS_ENABLED
	GfxRasterState state = Gfx::state().raster;
	state.scissor = true;
	state.scissorX1 = x1;
	state.scissorY1 = y1;
	state.scissorX2 = x2 > x1 ? x2 : x1;
	state.scissorY2 = y2 > y1 ? y2 : y1;
	Gfx::set_raster_state( state );
#endif
}


void Gfx::set_scissor_nested( const int x1, const int y1, const int x2, const int y2 )
{
#if GRAPHICS_ENABLED
	GfxRasterState state = Gfx::state().raster;
	if( !state.scissor ) { Gfx::set_scissor( x1, y1, x2, y2 ); return; }
	state.scissor = true;
	state.scissorX1 = max( state.scissorX1, x1 );
	state.scissorY1 = max( state.scissorY1, y1 );
	state.scissorX2 = min( state.scissorX2, x2 );
	state.scissorY2 = min( state.scissorY2, y2 );
	Gfx::set_raster_state( state );
#endif
}


void Gfx::reset_scissor()
{
#if GRAPHICS_ENABLED
	GfxRasterState state = Gfx::state().raster;
	state.scissor = false;
	state.scissorX1 = 0;
	state.scissorY1 = 0;
	state.scissorX2 = 0;
	state.scissorY2 = 0;
	Gfx::set_raster_state( state );
#endif
}


void Gfx::set_sampler_state( const GfxSamplerState &state )
{
#if GRAPHICS_ENABLED
	// Sampler state changes force a batch break
	if( Gfx::state().sampler == state ) { return; }
	draw_call();
	Gfx::state().sampler = state;
#endif
}


void Gfx::set_filtering_mode( const GfxFilteringMode &mode )
{
#if GRAPHICS_ENABLED
	GfxSamplerState state = Gfx::state().sampler;
	state.filterMode = mode;
	Gfx::set_sampler_state( state );
#endif
}


void Gfx::set_filtering_anisotropy( const int anisotropy )
{
#if GRAPHICS_ENABLED
	GfxSamplerState state = Gfx::state().sampler;
	state.anisotropy = clamp( anisotropy, 1, 16 );
	Gfx::set_sampler_state( state );
#endif
}


void Gfx::set_uv_wrap_mode( const GfxUVWrapMode &mode )
{
#if GRAPHICS_ENABLED
	GfxSamplerState state = Gfx::state().sampler;
	state.wrapMode = mode;
	Gfx::set_sampler_state( state );
#endif
}


void Gfx::set_blend_state( const GfxBlendState &state )
{
#if GRAPHICS_ENABLED
	// Blend state changes force a batch break
	if( Gfx::state().blend == state ) { return; }
	draw_call();
	Gfx::state().blend = state;
#endif
}


void Gfx::set_blend_enabled( const bool enabled )
{
#if GRAPHICS_ENABLED
	GfxBlendState state = Gfx::state().blend;
	state.blendEnable = enabled;
	Gfx::set_blend_state( state );
#endif
}


void Gfx::set_blend_mode_color( const GfxBlendFactor &srcFactor, const GfxBlendFactor &dstFactor,
	const GfxBlendOperation &op )
{
#if GRAPHICS_ENABLED
	GfxBlendState state = Gfx::state().blend;
	state.srcFactorColor = srcFactor;
	state.dstFactorColor = dstFactor;
	state.blendOperationColor = op;
	Gfx::set_blend_state( state );
#endif
}


void Gfx::set_blend_mode_alpha( const GfxBlendFactor &srcFactor, const GfxBlendFactor &dstFactor,
	const GfxBlendOperation &op )
{
#if GRAPHICS_ENABLED
	GfxBlendState state = Gfx::state().blend;
	state.srcFactorAlpha = srcFactor;
	state.dstFactorAlpha = dstFactor;
	state.blendOperationAlpha = op;
	Gfx::set_blend_state( state );
#endif
}


void Gfx::reset_blend_mode()
{
#if GRAPHICS_ENABLED
	Gfx::reset_blend_mode_color();
	Gfx::reset_blend_mode_alpha();
#endif
}


void Gfx::reset_blend_mode_color()
{
#if GRAPHICS_ENABLED
	Gfx::set_blend_mode_color( GfxBlendFactor_SRC_ALPHA, GfxBlendFactor_INV_SRC_ALPHA, GfxBlendOperation_ADD );
#endif
}


void Gfx::reset_blend_mode_alpha()
{
#if GRAPHICS_ENABLED
	Gfx::set_blend_mode_alpha( GfxBlendFactor_ONE, GfxBlendFactor_INV_SRC_ALPHA, GfxBlendOperation_ADD );
#endif
}


void Gfx::set_color_write_mask( const GfxColorWriteFlag &mask )
{
#if GRAPHICS_ENABLED
	GfxBlendState state = Gfx::state().blend;
	state.colorWriteMask = mask;
	Gfx::set_blend_state( state );
#endif
}


void Gfx::set_depth_state( const GfxDepthState &state )
{
#if GRAPHICS_ENABLED
	// Depth state changes force a batch break
	if( Gfx::state().depth == state ) { return; }
	draw_call();
	Gfx::state().depth = state;
#endif
}


void Gfx::set_depth_test_mode( const GfxDepthTestMode &mode )
{
#if GRAPHICS_ENABLED
	GfxDepthState state = Gfx::state().depth;
	state.depthTestMode = mode;
	Gfx::set_depth_state( state );
#endif
}


void Gfx::set_depth_write_mask( const GfxDepthWriteFlag &mask )
{
#if GRAPHICS_ENABLED
	GfxDepthState state = Gfx::state().depth;
	state.depthWriteMask = mask;
	Gfx::set_depth_state( state );
#endif
}


void Gfx::shader_bind( const Shader shader )
{
#if GRAPHICS_ENABLED
	CoreGfx::shaders[shader].bind();
#endif
}


void Gfx::shader_release()
{
#if GRAPHICS_ENABLED
	CoreGfx::shaders[Shader::SHADER_DEFAULT].bind();
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
