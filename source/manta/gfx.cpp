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

namespace GfxCore
{
	GfxState states[2];
	bool flip = false;
	bool rendering = false;

	GfxTexture2D textures[Assets::texturesCount];
	GfxShader shaders[Gfx::shadersCount];

	GfxSwapChain swapchain;
	GfxViewport viewport;

	Matrix matrixModel;
	Matrix matrixView;
	Matrix matrixViewInverse;
	Matrix matrixPerspective;
	Matrix matrixMVP;
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
		"  Index Buffers: %.4f mb", MB( stats.gpuMemoryIndexBuffers ) );
	drawY += 20.0f;

	draw_text_f( fnt_iosevka, 14, drawX, drawY, c_white,
		"  Constant Buffers: %.4f mb", MB( stats.gpuMemoryConstantBuffers ) );
	drawY += 20.0f;

	draw_text_f( fnt_iosevka, 14, drawX, drawY, c_white,
		"  Shader Programs: %.4f mb", MB( stats.gpuMemoryShaderPrograms ) );
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SysGfx::init()
{
#if GRAPHICS_ENABLED
	// Initialize Backend
	ErrorIf( !GfxCore::rb_init(), "%s: Failed to initialize %s backend!", __FUNCTION__, BUILD_GRAPHICS );

	// Initialize Textures
	ErrorIf( !SysGfx::init_textures(), "%s: Failed to initialize textures!", __FUNCTION__ );

	// Initialize Shaders
	ErrorIf( !SysGfx::init_shaders(), "%s: Failed to initialize shaders!", __FUNCTION__ );

	// Initialize Constant Buffers
 	ErrorIf( !GfxCore::rb_init_cbuffers(), "%s: Failed to initialize constant buffers!", __FUNCTION__ );

	// Initialize Quad Batch
	ErrorIf( !SysGfx::quadBatch.init( RENDER_QUAD_BATCH_SIZE ),
		"%s: Failed to initialize default quad batch!", __FUNCTION__ );
#endif

	// Success
	return true;
}


bool SysGfx::free()
{
#if GRAPHICS_ENABLED
	// Free Quad Batch
	ErrorIf( !SysGfx::quadBatch.free(), "%s: Failed to free default quad batch!", __FUNCTION__ );

	// Free Constant Buffers
	ErrorIf( !GfxCore::rb_free_cbuffers(), "%s: Failed to free shaders!", __FUNCTION__ );

	// Free Shaders
	ErrorIf( !SysGfx::free_shaders(), "%s: Failed to free shaders!", __FUNCTION__ );

	// Free Textures
	ErrorIf( !SysGfx::free_textures(), "%s: Failed to free textures!", __FUNCTION__ );

	// Free Backend
	ErrorIf( !GfxCore::rb_free(), "%s: Failed to free %s backend!", __FUNCTION__, BUILD_GRAPHICS );
#endif

	// Success
	return true;
}


bool SysGfx::init_textures()
{
#if GRAPHICS_ENABLED
	// Load Textures
	for( u32 i = 0; i < Assets::texturesCount; i++ )
	{
		const DiskTexture &diskTexture = Assets::textures[i];
		GfxCore::textures[i].init( Assets::binary.data + diskTexture.offset,
			diskTexture.width, diskTexture.height,
			GfxColorFormat_R8G8B8A8_FLOAT );
	}
#endif

	// Success
	return true;
}


bool SysGfx::free_textures()
{
#if GRAPHICS_ENABLED
	// Free Textures
	for( u32 i = 0; i < Assets::texturesCount; i++ )
	{
		GfxCore::textures[i].free();
	}
#endif

	// Success
	return true;
}


bool SysGfx::init_shaders()
{
#if GRAPHICS_ENABLED
	// Load Shaders
	for( u32 i = 0; i < Gfx::shadersCount; i++ )
	{
		const DiskShader &diskShader = Gfx::diskShaders[i];
		GfxCore::shaders[i].init( i, diskShader );
	}
#endif

	// Success
	return true;
}


bool SysGfx::free_shaders()
{
#if GRAPHICS_ENABLED
	// Free Shaders
	for( u32 i = 0; i < Gfx::shadersCount; i++ )
	{
		GfxCore::shaders[i].free();
	}
#endif

	// Success
	return true;
}


void SysGfx::state_reset()
{
#if GRAPHICS_ENABLED
	GfxCore::states[0] = { };
	GfxCore::states[1] = { };
	GfxCore::flip = false;

	// Default Shader (TODO: Refactor?)
	Gfx::shader_bind( SHADER_DEFAULT );

	// Update State
	SysGfx::state_apply( true );
#endif
}


void SysGfx::state_apply( const bool dirty )
{
#if GRAPHICS_ENABLED
	#define GFX_STATE_CHECK(condition) UNLIKELY( ( condition ) )

	// This function runs at every draw call
	// It is responsible for making sure that the GPU render state reflects the CPU render state
	GfxState &current = GfxCore::states[GfxCore::flip];
	GfxState &previous = GfxCore::states[!GfxCore::flip];

	// Raster State
	if( dirty || GFX_STATE_CHECK( current.raster != previous.raster ) )
		{ GfxCore::rb_set_raster_state( current.raster ); }

	// Sampler State
	if( dirty || GFX_STATE_CHECK( current.sampler != previous.sampler ) )
		{ GfxCore::rb_set_sampler_state( current.sampler ); }

	// Blend State
	if( dirty || GFX_STATE_CHECK( current.blend != previous.blend ) )
		{ GfxCore::rb_set_blend_state( current.blend ); }

	// Depth State
	if( dirty || GFX_STATE_CHECK( current.depth != previous.depth ) )
		{ GfxCore::rb_set_depth_state( current.depth ); }

	// Texture 2D Binding
	if( GFX_STATE_CHECK( current.textureResource[0] != previous.textureResource[0] &&
	                     current.textureResource[0] != nullptr ) )
		{ GfxCore::rb_texture_2d_bind( current.textureResource[0], 0 ); }

	// Cache State
	previous = current;
	GfxCore::flip = !GfxCore::flip;
#endif
}


byte *SysGfx::scratch_buffer( const usize size )
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

namespace SysGfx
{
	GfxQuadBatch<GfxVertex::BuiltinVertex> quadBatch;
}


bool Gfx::quad_batch_break()
{
#if GRAPHICS_ENABLED
	return SysGfx::quadBatch.batch_break();
#endif
	return false;
}


bool Gfx::quad_batch_can_break()
{
#if GRAPHICS_ENABLED
	return SysGfx::quadBatch.can_break();
#else
	return true;
#endif
}


void Gfx::quad_batch_break_check()
{
#if GRAPHICS_ENABLED
	SysGfx::quadBatch.break_check();
#endif
}


void Gfx::quad_batch_write( const GfxQuadBatch<GfxVertex::BuiltinVertex>::Quad &quad,
	const GfxTexture2D *const texture )
{
#if GRAPHICS_ENABLED
	if( LIKELY( texture != nullptr ) ) { texture->bind( 0 ); }
	SysGfx::quadBatch.write( quad );
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

	SysGfx::quadBatch.write( quad );
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

	SysGfx::quadBatch.write( quad );
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

	SysGfx::quadBatch.write( quad );
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

	SysGfx::quadBatch.write( quad );
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GfxTexture2D::init( void *data, const u16 width, const u16 height, const GfxColorFormat &format )
{
#if GRAPHICS_ENABLED
	ErrorIf( !GfxCore::rb_texture_2d_init( resource, data, width, height, format ),
		"Failed to init Texture2D!" );
#endif
}


void GfxTexture2D::free()
{
#if GRAPHICS_ENABLED
	if( resource == nullptr ) { return; }
	ErrorIf( !GfxCore::rb_texture_2d_free( resource ),
		"Failed to free Texture2D!" );
#endif
}


void GfxTexture2D::bind( const int slot ) const
{
#if GRAPHICS_ENABLED
	// Texture binding forces a batch break
	if( Gfx::state().textureResource[slot] != resource ) { draw_call(); }

	Gfx::state().textureResource[slot] = resource;
	ErrorIf( !GfxCore::rb_texture_2d_bind( resource, slot ),
		"Failed to bind Texture2D to slot %d!", slot );
#endif
}


void GfxTexture2D::release() const
{
#if GRAPHICS_ENABLED
	// ...
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static Matrix RT_CACHE_MATRIX_MODEL;
static Matrix RT_CACHE_MATRIX_VIEW;
static Matrix RT_CACHE_MATRIX_PERSPECTIVE;
static GfxViewport RT_CACHE_VIEWPORT;
static GfxBlendState RT_CACHE_BLEND_STATE;


void GfxRenderTarget2D::init( const u16 width, const u16 height, const GfxRenderTargetDescription &desc )
{
#if GRAPHICS_ENABLED
	ErrorIf( !GfxCore::rb_render_target_2d_init( resource, textureColor.resource, textureDepth.resource,
	                                             width, height, desc ),
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
	ErrorIf( !GfxCore::rb_render_target_2d_free( resource, textureColor.resource, textureDepth.resource ),
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

	const bool success = GfxCore::rb_render_target_2d_copy(
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
		Gfx::shader_bind( SHADER_DEFAULT_COPY_DEPTH );
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
	const bool success = GfxCore::rb_render_target_2d_copy_part(
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
	ErrorIf( !GfxCore::rb_render_target_2d_bind( resource, slot ),
		"Failed to bind RenderTarget2D!" );

	if( slot == 0 )
	{
		// MVP Matrix
		RT_CACHE_MATRIX_MODEL = Gfx::get_matrix_model();
		Gfx::set_matrix_model( matrix_build_identity() );
		RT_CACHE_MATRIX_VIEW = Gfx::get_matrix_view();
		Gfx::set_matrix_view( matrix_build_identity() );
		RT_CACHE_MATRIX_PERSPECTIVE = Gfx::get_matrix_perspective();
		Gfx::set_matrix_perspective( matrix_build_orthographic( 0.0f, width, 0.0f, height, 0.0f, 1.0f ) );

		// Viewport
		RT_CACHE_VIEWPORT = GfxCore::viewport;
		Gfx::set_viewport_size( width, height, false );

		// D3D11 needs to explicitly update depth test state here
		GfxCore::rb_set_depth_state( Gfx::state().depth );

		// Blend State
		RT_CACHE_BLEND_STATE = Gfx::state().blend;
		Gfx::set_blend_enabled( true );
		Gfx::set_blend_mode_color( GfxBlendFactor_ONE, GfxBlendFactor_INV_SRC_ALPHA, GfxBlendOperation_ADD );

		// Apply State
		SysGfx::state_apply();
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
	ErrorIf( !GfxCore::rb_render_target_2d_release( this->slot ),
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
		SysGfx::state_apply();
	}

	// Reset slot
	this->slot = -1;
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GfxShader::init( const u32 shaderID, const DiskShader &diskShader )
{
#if GRAPHICS_ENABLED
	this->shaderID = shaderID;
	ErrorIf( !GfxCore::rb_shader_init( resource, shaderID, diskShader ),
		"Failed to init shader! (%u)", this->shaderID );
#endif
}


void GfxShader::free()
{
#if GRAPHICS_ENABLED
	ErrorIf( !GfxCore::rb_shader_free( resource ),
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
	}

	ErrorIf( !GfxCore::rb_shader_bind( resource ),
		"Failed to bind shader!" );
	ErrorIf( !GfxCore::rb_shader_bind_constant_buffers_vertex[shaderID](),
		"Failed to bind vertex shader cbuffers! (%u)", this->shaderID );
	ErrorIf( !GfxCore::rb_shader_bind_constant_buffers_fragment[shaderID](),
		"Failed to bind fragment shader cbuffers! (%u)", this->shaderID );
	ErrorIf( !GfxCore::rb_shader_bind_constant_buffers_compute[shaderID](),
		"Failed to bind compute shader cbuffers! (%u)", this->shaderID );
#endif
}


void GfxShader::release()
{
	GfxCore::shaders[SHADER_DEFAULT].bind();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Gfx::viewport_update()
{
#if GRAPHICS_ENABLED
	// Update Viewport
	const int viewportWidth = static_cast<int>( Window::width * Window::scale );
	const int viewportHeight = static_cast<int>( Window::height * Window::scale );
	GfxCore::rb_viewport_resize( viewportWidth, viewportHeight, Window::fullscreen );

	// Resize Swapchain
	GfxCore::rb_swapchain_resize( Window::width, Window::height, Window::fullscreen );
#endif
}


void Gfx::frame_begin()
{
#if GRAPHICS_ENABLED
	// Reset State
	SysGfx::state_reset();

	// Backend
	GfxCore::rb_frame_begin();

	// Quad Batch
	SysGfx::quadBatch.batch_begin();

	// Reset Matrices
	const Matrix identity = matrix_build_identity();
	Gfx::set_matrix_model( identity );
	Gfx::set_matrix_view( identity );
	Gfx::set_matrix_perspective( identity );

	// Start Rendering
	GfxCore::rendering = true;
#endif
}


void Gfx::frame_end()
{
#if GRAPHICS_ENABLED
	// Stop Rendering
	GfxCore::rendering = false;

	// Quad Batch
	SysGfx::quadBatch.batch_end();

	// Backend
	GfxCore::rb_frame_end();

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
	GfxCore::rb_clear_color( color );
#endif
}


void Gfx::clear_depth( const float depth )
{
#if GRAPHICS_ENABLED
	GfxCore::rb_clear_depth( depth );
#endif
}


void Gfx::set_swapchain_size( const u16 width, const u16 height, const bool fullscreen )
{
#if GRAPHICS_ENABLED
	GfxCore::rb_swapchain_resize( width, height, fullscreen );
#endif
}


void Gfx::set_viewport_size( const u16 width, const u16 height, const bool fullscreen )
{
#if GRAPHICS_ENABLED
	GfxCore::rb_viewport_resize( width, height, fullscreen );
#endif
}


void Gfx::set_shader_globals( const GfxCoreCBuffer::ShaderGlobals_t &globals )
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
	GfxCore::matrixMVP = matrix_multiply( GfxCore::matrixPerspective,
		matrix_multiply( GfxCore::matrixView, GfxCore::matrixModel ) );

	auto &globals = GfxCBuffer::ShaderGlobals;
	globals.matrixModel = GfxCore::matrixModel;
	globals.matrixView = GfxCore::matrixView;
	globals.matrixPerspective = GfxCore::matrixPerspective;
	globals.matrixMVP = GfxCore::matrixMVP;

	Gfx::set_shader_globals( globals );
#endif
};


void Gfx::set_matrix_model( const Matrix &matrix )
{
#if GRAPHICS_ENABLED
	GfxCore::matrixModel = matrix;
	update_matrix_mvp();
#endif
}


void Gfx::set_matrix_view( const Matrix &matrix )
{
#if GRAPHICS_ENABLED
	GfxCore::matrixView = matrix;
	update_matrix_mvp();
#endif
}


void Gfx::set_matrix_perspective( const Matrix &matrix )
{
#if GRAPHICS_ENABLED
	GfxCore::matrixPerspective = matrix;
	update_matrix_mvp();
#endif
}


void Gfx::set_matrix_mvp( const Matrix &matModel, const Matrix &matView, const Matrix &matPerspective )
{
#if GRAPHICS_ENABLED
	GfxCore::matrixModel = matModel;
	GfxCore::matrixView = matView;
	GfxCore::matrixPerspective = matPerspective;
	update_matrix_mvp();
#endif
}


void Gfx::set_matrix_mvp_2d_orthographic( const float x, const float y, const float zoom, const float angle,
	const float width, const float height, const float znear, const float zfar )
{
#if GRAPHICS_ENABLED
	GfxCore::matrixModel = matrix_build_identity();
	Matrix matrixView = matrix_build_rotation_z( angle * DEG2RAD );
	matrixView = matrix_multiply( matrix_build_scaling( zoom, zoom, 1.0f ), matrixView );
	matrixView = matrix_multiply( matrix_build_translation( -x, -y, 0.0f ), matrixView );
	GfxCore::matrixView = matrixView;
	GfxCore::matrixPerspective = matrix_build_orthographic( 0.0f, width, 0.0f, height, znear, zfar );
	update_matrix_mvp();
#endif
}


void Gfx::set_matrix_mvp_3d_perspective( const float fov, const float aspect, const float znear, const float zfar,
	const float x, const float y, const float z, const float xto, const float yto,
	const float zto, const float xup, const float yup, const float zup )
{
#if GRAPHICS_ENABLED
	AssertMsg( znear > 0.0f, "znear must be > 0.0f!" );
	GfxCore::matrixModel = matrix_build_identity();
	GfxCore::matrixView = matrix_build_lookat( x, y, z, xto, yto, zto, xup, yup, zup );
	GfxCore::matrixPerspective = matrix_build_perspective( fov, aspect, znear, zfar );
	update_matrix_mvp();
#endif
}


const Matrix &Gfx::get_matrix_model()
{
	return GfxCore::matrixModel;
}


const Matrix &Gfx::get_matrix_view()
{
	return GfxCore::matrixView;
}


const Matrix &Gfx::get_matrix_perspective()
{
	return GfxCore::matrixPerspective;
}


const Matrix &Gfx::get_matrix_mvp()
{
	return GfxCore::matrixMVP;
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
