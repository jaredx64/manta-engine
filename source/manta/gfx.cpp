#include <manta/gfx.hpp>

#include <pipeline.generated.hpp>

#include <core/memory.hpp>

#include <manta/window.hpp>
#include <manta/draw.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace CoreGfx
{
	GfxState state = { };
	GfxQuadBatch<GfxVertex::BuiltinVertex> batch;

	GfxShader shaders[CoreGfx::shaderCount] = { };
	GfxTexture textures[CoreAssets::textureCount] = { };
	GfxUniformBufferResource *uniformBuffers[CoreGfx::uniformBufferCount] = { nullptr }; // Filled: gfx.generated.cpp
}


GfxStack GfxRenderCommand::renderCommandArgsStack;
GfxStack GfxRenderCommand::renderCommandTagsStack;

List<List<GfxRenderCommand>> GfxRenderGraph::renderGraphCommandLists;
usize GfxRenderGraph::renderGraphCommandListCurrent = 0LLU;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void state_change_break_batches()
{
#if GRAPHICS_ENABLED
	Gfx::quad_batch_break();
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Profiling

#if PROFILING_GFX
namespace Gfx
{
	GfxStatistics stats = { };
	GfxStatistics statsPrevious = { };
}


static void format_integer( char *buffer, usize size, int num )
{
	char temp[32];
	int length, i, j = 0;

	snprintf( temp, sizeof( temp ), "%d", num < 0 ? -num : num );
	length = static_cast<int>( strlen( temp ) );

	// Negative
	if( num < 0 ) { buffer[0] = '-'; j = 1; }

	// Commas
	for( i = 0; i < length; i++ )
	{
		buffer[j++] = temp[i];
		if( ( length - i - 1 ) % 3 == 0 && i != length - 1 && j < static_cast<int>( size ) - 1 ) { buffer[j++] = ','; }
		if( j >= static_cast<int>( size - 1 ) ) { buffer[j] = '\0'; return; }
	}

	buffer[j] = '\0';
}


#include <manta/draw.hpp>
void debug_overlay_gfx( const float x, const float y )
{
	float drawX = x;
	float drawY = y;
	const GfxStatistics &stats = Gfx::stats;//Previous;

	char buffer[32];

	// State

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

	// Memory

	drawY += 20.0f;
	draw_text_f( fnt_iosevka, 14, drawX, drawY, c_yellow,
		"GPU Memory Total: %.4f mb", MB( stats.total_memory() ) );
	drawY += 20.0f;

	draw_text_f( fnt_iosevka, 14, drawX, drawY, c_white,
		"  Swapchain: %.4f mb", MB( stats.gpuMemorySwapchain ) );
	drawY += 20.0f;

	draw_text_f( fnt_iosevka, 14, drawX, drawY, c_white,
		"  Shaders: %.4f mb", MB( stats.gpuMemoryShaders ) );
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
		"  Uniform Buffers: %.4f mb", MB( stats.gpuMemoryUniformBuffers ) );
	drawY += 20.0f;

	draw_text_f( fnt_iosevka, 14, drawX, drawY, c_white,
		"  Constant Buffers: %.4f mb", MB( stats.gpuMemoryConstantBuffers ) );
	drawY += 20.0f;

	draw_text_f( fnt_iosevka, 14, drawX, drawY, c_white,
		"  Mutable Buffers: %.4f mb", MB( stats.gpuMemoryMutableBuffers ) );
	drawY += 20.0f;

	draw_text_f( fnt_iosevka, 14, drawX, drawY, c_white,
		"  Textures: %.4f mb", MB( stats.gpuMemoryTextures ) );
	drawY += 20.0f;

	draw_text_f( fnt_iosevka, 14, drawX, drawY, c_white,
		"  Render Targets: %.4f mb", MB( stats.gpuMemoryRenderTargets ) );
	drawY += 20.0f;
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// System

bool CoreGfx::init()
{
#if GRAPHICS_ENABLED
	ErrorIf( !CoreGfx::api_init(), "%s: Failed to initialize %s backend!", __FUNCTION__, BUILD_GRAPHICS );
	ErrorIf( !CoreGfx::init_textures(), "%s: Failed to initialize textures!", __FUNCTION__ );
	ErrorIf( !CoreGfx::init_uniform_buffers(), "%s: Failed to initialize uniform buffers!", __FUNCTION__ );
	ErrorIf( !CoreGfx::init_shaders(), "%s: Failed to initialize shaders!", __FUNCTION__ );
	ErrorIf( !CoreGfx::init_commands(), "%s: Failed to initialize command system!", __FUNCTION__ );
	ErrorIf( !CoreGfx::batch.init( GFX_QUAD_BATCH_SIZE ), "%s: Failed to initialize quad batch!", __FUNCTION__ );
#endif
	return true;
}


bool CoreGfx::free()
{
#if GRAPHICS_ENABLED
	ErrorIf( !CoreGfx::batch.free(), "%s: Failed to free default quad batch!", __FUNCTION__ );
	ErrorIf( !CoreGfx::free_commands(), "%s: Failed to free command system!", __FUNCTION__ );
	ErrorIf( !CoreGfx::free_shaders(), "%s: Failed to free shaders!", __FUNCTION__ );
	ErrorIf( !CoreGfx::free_uniform_buffers(), "%s: Failed to free uniform buffers!", __FUNCTION__ );
	ErrorIf( !CoreGfx::free_textures(), "%s: Failed to free textures!", __FUNCTION__ );
	ErrorIf( !CoreGfx::api_free(), "%s: Failed to free %s backend!", __FUNCTION__, BUILD_GRAPHICS );
#endif
	return true;
}


bool CoreGfx::init_textures()
{
#if GRAPHICS_ENABLED
	for( u32 i = 0; i < CoreAssets::textureCount; i++ )
	{
		const Assets::TextureEntry &textureEntry = Assets::texture( i );
		CoreGfx::textures[i].init_2d( Assets::binary.data + textureEntry.offset,
			textureEntry.width, textureEntry.height, textureEntry.levels,
			GfxColorFormat_R8G8B8A8_FLOAT );
	}
#endif

	return true;
}


bool CoreGfx::free_textures()
{
#if GRAPHICS_ENABLED
	for( u32 i = 0; i < CoreAssets::textureCount; i++ )
	{
		CoreGfx::textures[i].free();
	}
#endif

	return true;
}


bool CoreGfx::init_uniform_buffers()
{
#if GRAPHICS_ENABLED
	static_assert( ARRAY_LENGTH( uniformBufferInitEntries ) == uniformBufferCount, "Missing GfxUniformBuffer!" );
	for( u32 i = 0; i < uniformBufferCount; i++ )
	{
		const UniformBufferInitEntry &entry = uniformBufferInitEntries[i];
		if( !api_uniform_buffer_init( uniformBuffers[i], entry.name, i, entry.size ) ) { return false; }
	}
#endif

	return true;
}


bool CoreGfx::free_uniform_buffers()
{
#if GRAPHICS_ENABLED
	for( u32 i = 0; i < uniformBufferCount; i++ )
	{
		if( !api_uniform_buffer_free( uniformBuffers[i] ) ) { return false; }
	}
#endif

	return true;
}


bool CoreGfx::init_shaders()
{
#if GRAPHICS_ENABLED
	for( u32 i = 0; i < CoreGfx::shaderCount; i++ )
	{
		const ShaderEntry &shaderEntry = CoreGfx::shaderEntries[i];
		CoreGfx::shaders[i].init( i, shaderEntry );
	}
#endif

	return true;
}


bool CoreGfx::free_shaders()
{
#if GRAPHICS_ENABLED
	for( u32 i = 0; i < CoreGfx::shaderCount; i++ )
	{
		CoreGfx::shaders[i].free();
	}
#endif

	return true;
}


bool CoreGfx::init_commands()
{
	GfxRenderCommand::renderCommandArgsStack.init( 4096 );
	GfxRenderCommand::renderCommandTagsStack.init( 4096 );

	GfxRenderGraph::renderGraphCommandLists.init();
	GfxRenderGraph::renderGraphCommandListCurrent = 0LLU;

	return true;
}


bool CoreGfx::free_commands()
{
	GfxRenderCommand::renderCommandArgsStack.free();
	GfxRenderCommand::renderCommandTagsStack.free();

	GfxRenderGraph::renderGraphCommandLists.free();

	return true;
}


void CoreGfx::clear_commands()
{
	GfxRenderCommand::renderCommandArgsStack.clear();
	GfxRenderCommand::renderCommandTagsStack.clear();

	for( usize i = 0; i < GfxRenderGraph::renderGraphCommandListCurrent; i++ )
	{
		GfxRenderGraph::renderGraphCommandLists[i].clear();
	}
	GfxRenderGraph::renderGraphCommandListCurrent = 0LLU;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Frame

void Gfx::frame_begin()
{
#if GRAPHICS_ENABLED
	AssertMsg( !CoreGfx::state.rendering, "Gfx::frame_begin() called without Gfx::frame_end()!" );
	CoreGfx::api_frame_begin();
	CoreGfx::state.rendering = true;

	CoreGfx::state_reset();

	CoreGfx::clear_commands();

	CoreGfx::quad_batch_frame_begin();
#endif
}


void Gfx::frame_end()
{
#if GRAPHICS_ENABLED
	AssertMsg( CoreGfx::state.rendering, "Gfx::frame_end() called without Gfx::frame_begin()!" );

	AssertMsg( CoreGfx::state.renderPassActive == nullptr,
		"Gfx::frame_end() while GfxRenderPass is still active! (%p)", CoreGfx::state.renderPassActive );
	AssertMsg( CoreGfx::state.renderCommandActive == nullptr,
		"Gfx::frame_end() while GfxRenderCommand is still active! (%p)", CoreGfx::state.renderCommandActive );

	CoreGfx::quad_batch_frame_end();

#if PROFILING_GFX
	memory_copy( &Gfx::statsPrevious, &Gfx::stats, sizeof( GfxStatistics ) );
	Gfx::stats.frame = { };
#endif

	CoreGfx::state.rendering = false;
	CoreGfx::api_frame_end();
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Stack

void GfxStack::init( const usize reserve )
{
	MemoryAssert( data == nullptr );
	Assert( reserve > 0 );
	capacity = reserve;
	current = 0LLU;
	data = reinterpret_cast<byte *>( memory_alloc( capacity ) );
}


void GfxStack::free()
{
	if( data == nullptr ) { return; }
	memory_free( data );
	data = nullptr;
	capacity = 0LLU;
	current = 0LLU;
}


void GfxStack::grow()
{
	MemoryAssert( data != nullptr );
	Assert( capacity >= 1 && capacity < USIZE_MAX );

	capacity = capacity > USIZE_MAX / 2 ? USIZE_MAX : capacity * 2;
	data = reinterpret_cast<byte *>( memory_realloc( data, capacity ) );
	ErrorIf( data == nullptr, "Failed to reallocate memory for GfxStack (%p: realloc %d bytes)",
		data, capacity );
}


void GfxStack::clear()
{
	current = 0LLU;
}


usize GfxStack::add( const void *buffer, const u16 size, const u16 alignment )
{
	MemoryAssert( data != nullptr );

	// Align
	Assert( alignment >= 1 );
	current = ( current + alignment - 1 ) & ~( alignment - 1 );

	// Grow
	for( ; current + size > capacity; grow() ) { }

	// Write
	const usize offset = current;
	ErrorIf( offset + size > capacity, "GfxStack: add exceeded capacity" );
	memory_copy( &data[offset], buffer, size );
	current += size;
	return offset;
}


void *GfxStack::get( const usize offset, const u16 size )
{
	Assert( offset + size <= current );
	return size > 0 ? reinterpret_cast<void *>( &data[offset] ) : nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Clear

void Gfx::clear_color( const Color color )
{
#if GRAPHICS_ENABLED
	AssertMsg( CoreGfx::state.renderCommandActive == nullptr, "Cannot call clear_color() within a render command!" );
	CoreGfx::api_clear_color( color );
#endif
}


void Gfx::clear_depth( const float depth )
{
#if GRAPHICS_ENABLED
	AssertMsg( CoreGfx::state.renderCommandActive == nullptr, "Cannot call clear_depth() within a render command!" );
	CoreGfx::api_clear_depth( depth );
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// State (System)

void GfxPipelineDescription::raster_set( const GfxPipelineDescription &state )
{
	rasterCullMode = state.rasterCullMode;
	rasterFillMode = state.rasterFillMode;
}


void GfxPipelineDescription::blend_set( const GfxPipelineDescription &state )
{
	blendEnabled = state.blendEnabled;
	blendColorWriteMask = state.blendColorWriteMask;
	blendSrcFactorColor = state.blendSrcFactorColor;
	blendSrcFactorAlpha = state.blendSrcFactorAlpha;
	blendDstFactorColor = state.blendDstFactorColor;
	blendDstFactorAlpha = state.blendDstFactorAlpha;
	blendOperationColor = state.blendOperationColor;
	blendOperationAlpha = state.blendOperationAlpha;
}


void GfxPipelineDescription::depth_set_state( const GfxPipelineDescription &state )
{
	depthWriteMask = state.depthWriteMask;
	depthFunction = state.depthFunction;
}


void GfxPipelineDescription::stencil_set_state( const GfxPipelineDescription &state )
{
	stencil = state.stencil;
}


bool GfxPipelineDescription::equal_raster( const GfxPipelineDescription &other ) const
{
	return
		rasterCullMode == other.rasterCullMode &&
		rasterFillMode == other.rasterFillMode;
}


bool GfxPipelineDescription::equal_blend( const GfxPipelineDescription &other ) const
{
	return
		blendEnabled == other.blendEnabled &&
		blendColorWriteMask == other.blendColorWriteMask &&
		blendSrcFactorColor == other.blendSrcFactorColor &&
		blendSrcFactorAlpha == other.blendSrcFactorAlpha &&
		blendDstFactorColor == other.blendDstFactorColor &&
		blendDstFactorAlpha == other.blendDstFactorAlpha &&
		blendOperationColor == other.blendOperationColor &&
		blendOperationAlpha == other.blendOperationAlpha;
}


bool GfxPipelineDescription::equal_depth( const GfxPipelineDescription &other ) const
{
	return
		depthWriteMask == other.depthWriteMask &&
		depthFunction == other.depthFunction;
}


bool GfxPipelineDescription::equal_stencil( const GfxPipelineDescription &other ) const
{
	return
		stencil == other.stencil;
}


void CoreGfx::state_reset()
{
#if GRAPHICS_ENABLED
	const double_m44 identity = double_m44_build_identity();
	Gfx::set_matrix_model( identity );
	Gfx::set_matrix_view( identity );
	Gfx::set_matrix_perspective( identity );
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Render State

void Gfx::scissor_set_state( const GfxStateScissor &state )
{
#if GRAPHICS_ENABLED
	state_change_break_batches();
	CoreGfx::api_scissor_set_state( state );
#endif
}


void Gfx::scissor_set( const u16 x1, const u16 y1, const u16 x2, const u16 y2 )
{
#if GRAPHICS_ENABLED
	GfxStateScissor scissor = CoreGfx::state.scissor;
	scissor.x1 = x1;
	scissor.y1 = y1;
	scissor.x2 = x2 > x1 ? x2 : x1;
	scissor.y2 = y2 > y1 ? y2 : y1;
	Gfx::scissor_set_state( scissor );
#endif
}


void Gfx::scissor_set_nested( const u16 x1, const u16 y1, const u16 x2, const u16 y2 )
{
#if GRAPHICS_ENABLED
	GfxStateScissor scissor = CoreGfx::state.scissor;
	if( !scissor.is_enabled() ) { Gfx::scissor_set( x1, y1, x2, y2 ); return; }
	scissor.x1 = max( scissor.x1, x1 );
	scissor.y1 = max( scissor.y1, y1 );
	scissor.x2 = min( scissor.x2, x2 );
	scissor.y2 = min( scissor.y2, y2 );
	Gfx::scissor_set_state( scissor );
#endif
}


void Gfx::scissor_reset()
{
#if GRAPHICS_ENABLED
	GfxStateScissor scissor = CoreGfx::state.scissor;
	scissor.x1 = 0;
	scissor.y1 = 0;
	scissor.x2 = 0;
	scissor.y2 = 0;
	Gfx::scissor_set_state( scissor );
#endif
}


void Gfx::sampler_set_state( const GfxStateSampler &state )
{
#if GRAPHICS_ENABLED
	state_change_break_batches();
	CoreGfx::api_sampler_set_state( state );
#endif
}


void Gfx::sampler_filtering_mode( const GfxSamplerFilteringMode &mode )
{
#if GRAPHICS_ENABLED
	GfxStateSampler state = CoreGfx::state.sampler;
	state.filterMode = mode;
	Gfx::sampler_set_state( state );
#endif
}


void Gfx::sampler_filtering_anisotropy( const int anisotropy )
{
#if GRAPHICS_ENABLED
	GfxStateSampler state = CoreGfx::state.sampler;
	state.anisotropy = clamp( anisotropy, 1, 16 );
	Gfx::sampler_set_state( state );
#endif
}


void Gfx::sampler_wrap_mode( const GfxSamplerWrapMode &mode )
{
#if GRAPHICS_ENABLED
	GfxStateSampler state = CoreGfx::state.sampler;
	state.wrapMode = mode;
	Gfx::sampler_set_state( state );
#endif
}


void CoreGfx::swapchain_viewport_update()
{
#if GRAPHICS_ENABLED
#if SWAPCHAIN_DPI_SCALED
	const u16 widthPixels = static_cast<u16>( Window::width );
	const u16 heightPixels = static_cast<u16>( Window::height );
#else
	const u16 widthPixels = static_cast<u16>( Window::width * Window::scale );
	const u16 heightPixels = static_cast<u16>( Window::height * Window::scale );
#endif
	CoreGfx::api_swapchain_set_size( widthPixels, heightPixels );
	CoreGfx::api_viewport_set_size( widthPixels, heightPixels );
#endif
}


int Gfx::swapchain_width()
{
#if GRAPHICS_ENABLED
	return CoreGfx::state.swapchain.width;
#endif
}


int Gfx::swapchain_height()
{
#if GRAPHICS_ENABLED
	return CoreGfx::state.swapchain.height;
#endif
}


int Gfx::viewport_width()
{
#if GRAPHICS_ENABLED
	return CoreGfx::state.viewport.width;
#endif
}


int Gfx::viewport_height()
{
#if GRAPHICS_ENABLED
	return CoreGfx::state.viewport.height;
#endif
}


void Gfx::viewport_set_size( const u16 width, const u16 height )
{
#if GRAPHICS_ENABLED
	state_change_break_batches();
	CoreGfx::api_viewport_set_size( width, height );
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Pipeline State

void Gfx::raster_set_state( const GfxPipelineDescription &state )
{
#if GRAPHICS_ENABLED
	AssertMsg( CoreGfx::state.renderCommandActive, "Raster state changes must be part of a GfxRenderCommand!" );
	state_change_break_batches();
	CoreGfx::api_set_raster( state );
#endif
}


void Gfx::raster_fill_mode( const GfxFillMode &mode )
{
#if GRAPHICS_ENABLED
	GfxPipelineDescription state = CoreGfx::state.pipeline.description;
	state.rasterFillMode = mode;
	Gfx::raster_set_state( state );
#endif
}


void Gfx::raster_cull_mode( const GfxCullMode &mode )
{
#if GRAPHICS_ENABLED
	GfxPipelineDescription state = CoreGfx::state.pipeline.description;
	state.rasterCullMode = mode;
	Gfx::raster_set_state( state );
#endif
}


void Gfx::blend_set_state( const GfxPipelineDescription &state )
{
#if GRAPHICS_ENABLED
	AssertMsg( CoreGfx::state.renderCommandActive, "Blend state changes must be part of a GfxRenderCommand!" );
	state_change_break_batches();
	CoreGfx::api_set_blend( state );
#endif
}


void Gfx::blend_enabled( const bool enabled )
{
#if GRAPHICS_ENABLED
	GfxPipelineDescription state = CoreGfx::state.pipeline.description;
	state.blendEnabled = enabled;
	Gfx::blend_set_state( state );
#endif
}


void Gfx::blend_mode_color( const GfxBlendFactor &srcFactor, const GfxBlendFactor &dstFactor,
	const GfxBlendOperation &op )
{
#if GRAPHICS_ENABLED
	GfxPipelineDescription state = CoreGfx::state.pipeline.description;
	state.blendSrcFactorColor = srcFactor;
	state.blendDstFactorColor = dstFactor;
	state.blendOperationColor = op;
	Gfx::blend_set_state( state );
#endif
}


void Gfx::blend_mode_alpha( const GfxBlendFactor &srcFactor, const GfxBlendFactor &dstFactor,
	const GfxBlendOperation &op )
{
#if GRAPHICS_ENABLED
	GfxPipelineDescription state = CoreGfx::state.pipeline.description;
	state.blendSrcFactorAlpha = srcFactor;
	state.blendDstFactorAlpha = dstFactor;
	state.blendOperationAlpha = op;
	Gfx::blend_set_state( state );
#endif
}


void Gfx::blend_mode_reset()
{
#if GRAPHICS_ENABLED
	Gfx::blend_mode_reset_color();
	Gfx::blend_mode_reset_alpha();
#endif
}


void Gfx::blend_mode_reset_color()
{
#if GRAPHICS_ENABLED
	Gfx::blend_mode_color( GfxBlendFactor_SRC_ALPHA, GfxBlendFactor_INV_SRC_ALPHA, GfxBlendOperation_ADD );
#endif
}


void Gfx::blend_mode_reset_alpha()
{
#if GRAPHICS_ENABLED
	Gfx::blend_mode_alpha( GfxBlendFactor_ONE, GfxBlendFactor_INV_SRC_ALPHA, GfxBlendOperation_ADD );
#endif
}


void Gfx::blend_write_mask( const GfxBlendWrite &mask )
{
#if GRAPHICS_ENABLED
	GfxPipelineDescription state = CoreGfx::state.pipeline.description;
	state.blendColorWriteMask = mask;
	Gfx::blend_set_state( state );
#endif
}


void Gfx::depth_set_state( const GfxPipelineDescription &state )
{
#if GRAPHICS_ENABLED
	AssertMsg( CoreGfx::state.renderCommandActive, "Depth state changes must be part of a GfxRenderCommand!" );
	state_change_break_batches();
	CoreGfx::api_set_depth( state );
#endif
}


void Gfx::depth_function( const GfxDepthFunction &function )
{
#if GRAPHICS_ENABLED
	GfxPipelineDescription state = CoreGfx::state.pipeline.description;
	state.depthFunction = function;
	Gfx::depth_set_state( state );
#endif
}


void Gfx::depth_write_mask( const GfxDepthWrite &mask )
{
#if GRAPHICS_ENABLED
	GfxPipelineDescription state = CoreGfx::state.pipeline.description;
	state.depthWriteMask = mask;
	Gfx::depth_set_state( state );
#endif
}


void Gfx::stencil_set_state( const GfxPipelineDescription &state )
{
#if GRAPHICS_ENABLED
	AssertMsg( CoreGfx::state.renderCommandActive, "Stencil state changes must be part of a GfxRenderCommand!" );
	state_change_break_batches();
	CoreGfx::api_set_stencil( state );
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Uniforms (Pipeline)

void CoreGfx::update_matrix_mvp()
{
#if GRAPHICS_ENABLED
	CoreGfx::state.matrixMVP = double_m44_multiply( CoreGfx::state.matrixPerspective,
		double_m44_multiply( CoreGfx::state.matrixView, CoreGfx::state.matrixModel ) );
	CoreGfx::state.matrixMVPInverse = double_m44_multiply( CoreGfx::state.matrixModelInverse,
		double_m44_multiply( CoreGfx::state.matrixViewInverse, CoreGfx::state.matrixPerspectiveInverse ) );

	auto &uniforms = GfxUniformBuffer::UniformsPipeline;
	uniforms.matrixModel = float_m44_from_double_m44( CoreGfx::state.matrixModel );
	uniforms.matrixView = float_m44_from_double_m44( CoreGfx::state.matrixView );
	uniforms.matrixPerspective = float_m44_from_double_m44( CoreGfx::state.matrixPerspective );
	uniforms.matrixMVP = float_m44_from_double_m44( CoreGfx::state.matrixMVP );
	uniforms.matrixModelInverse = float_m44_from_double_m44( CoreGfx::state.matrixModelInverse );
	uniforms.matrixViewInverse = float_m44_from_double_m44( CoreGfx::state.matrixViewInverse );
	uniforms.matrixPerspectiveInverse = float_m44_from_double_m44( CoreGfx::state.matrixPerspectiveInverse );
	uniforms.matrixMVPInverse = float_m44_from_double_m44( CoreGfx::state.matrixMVPInverse );
	Gfx::set_shader_globals( uniforms );
#endif
};


void Gfx::set_matrix_mvp_2d_orthographic( const double x, const double y, const double zoom, const double angle,
	const double width, const double height, const double znear, const double zfar )
{
#if GRAPHICS_ENABLED
	CoreGfx::state.matrixModel = double_m44_build_identity();
	CoreGfx::state.matrixModelInverse = double_m44_inverse( CoreGfx::state.matrixModel );

	double_m44 matrixView = double_m44_build_rotation_z( angle * DEG2RAD );
	matrixView = double_m44_multiply( double_m44_build_scaling( zoom, zoom, 1.0 ), matrixView );
	matrixView = double_m44_multiply( double_m44_build_translation( -x, -y, 0.0 ), matrixView );
	CoreGfx::state.matrixView = matrixView;
	CoreGfx::state.matrixViewInverse = double_m44_inverse( CoreGfx::state.matrixView );

	CoreGfx::state.matrixPerspective = double_m44_build_orthographic( 0.0, width, 0.0, height, znear, zfar );
	CoreGfx::state.matrixPerspectiveInverse = double_m44_inverse( CoreGfx::state.matrixPerspective );

	CoreGfx::update_matrix_mvp();
#endif
}


void Gfx::set_matrix_mvp_3d_perspective( const double fov, const double aspect, const double znear, const double zfar,
	const double x, const double y, const double z, const double xto, const double yto,
	const double zto, const double xup, const double yup, const double zup )
{
#if GRAPHICS_ENABLED
	AssertMsg( znear > 0.0, "znear must be > 0.0!" );

	CoreGfx::state.matrixModel = double_m44_build_identity();
	CoreGfx::state.matrixModelInverse = double_m44_inverse( CoreGfx::state.matrixModel );

	CoreGfx::state.matrixView = double_m44_build_lookat( x, y, z, xto, yto, zto, xup, yup, zup );
	CoreGfx::state.matrixViewInverse = double_m44_inverse( CoreGfx::state.matrixView );

	CoreGfx::state.matrixPerspective = double_m44_build_perspective( fov, aspect, znear, zfar );
	CoreGfx::state.matrixPerspectiveInverse = double_m44_inverse( CoreGfx::state.matrixPerspective );

	CoreGfx::update_matrix_mvp();
#endif
}


void Gfx::set_matrix_model( const double_m44 &matrix )
{
#if GRAPHICS_ENABLED
	CoreGfx::state.matrixModel = matrix;
	CoreGfx::state.matrixModelInverse = double_m44_inverse( matrix );
	CoreGfx::update_matrix_mvp();
#endif
}


void Gfx::set_matrix_view( const double_m44 &matrix )
{
#if GRAPHICS_ENABLED
	CoreGfx::state.matrixView = matrix;
	CoreGfx::state.matrixViewInverse = double_m44_inverse( matrix );
	CoreGfx::update_matrix_mvp();
#endif
}


void Gfx::set_matrix_perspective( const double_m44 &matrix )
{
#if GRAPHICS_ENABLED
	CoreGfx::state.matrixPerspective = matrix;
	CoreGfx::state.matrixPerspectiveInverse = double_m44_inverse( matrix );
	CoreGfx::update_matrix_mvp();
#endif
}


void Gfx::set_matrix_mvp( const double_m44 &matModel, const double_m44 &matView, const double_m44 &matPerspective )
{
#if GRAPHICS_ENABLED
	CoreGfx::state.matrixModel = matModel;
	CoreGfx::state.matrixModelInverse = double_m44_inverse( matModel );
	CoreGfx::state.matrixView = matView;
	CoreGfx::state.matrixViewInverse = double_m44_inverse( matView );
	CoreGfx::state.matrixPerspective = matPerspective;
	CoreGfx::state.matrixPerspectiveInverse = double_m44_inverse( matPerspective );
	CoreGfx::update_matrix_mvp();
#endif
}


const double_m44 &Gfx::get_matrix_model()
{
	return CoreGfx::state.matrixModel;
}


const double_m44 &Gfx::get_matrix_view()
{
	return CoreGfx::state.matrixView;
}


const double_m44 &Gfx::get_matrix_perspective()
{
	return CoreGfx::state.matrixPerspective;
}


const double_m44 &Gfx::get_matrix_mvp()
{
	return CoreGfx::state.matrixMVP;
}


const double_m44 &Gfx::get_matrix_view_inverse()
{
	return CoreGfx::state.matrixView;
}


const double_m44 &Gfx::get_matrix_model_inverse()
{
	return CoreGfx::state.matrixModel;
}


const double_m44 &Gfx::get_matrix_perspective_inverse()
{
	return CoreGfx::state.matrixPerspective;
}


const double_m44 &Gfx::get_matrix_mvp_inverse()
{
	return CoreGfx::state.matrixMVP;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Shader

bool CoreGfx::shader_bind_uniform_buffers_vertex( const u32 shaderID )
{
	Assert( shaderID < CoreGfx::shaderCount );
	const UniformBufferBindTable &table = uniformBufferBindTableShaderVertex[shaderID];
	if( table.buffers == nullptr ) { return true; }

	for( u32 i = 0; i < table.count; i++ )
	{
		const UniformBufferBindEntry &entry = table.buffers[i];
		if( !api_uniform_buffer_bind_vertex( CoreGfx::uniformBuffers[entry.id], entry.slot ) ) { return false; }
	}

	return true;
}


bool CoreGfx::shader_bind_uniform_buffers_fragment( const u32 shaderID )
{
	Assert( shaderID < CoreGfx::shaderCount );
	const UniformBufferBindTable &table = uniformBufferBindTableShaderFragment[shaderID];
	if( table.buffers == nullptr ) { return true; }

	for( u32 i = 0; i < table.count; i++ )
	{
		const UniformBufferBindEntry &entry = table.buffers[i];
		if( !api_uniform_buffer_bind_fragment( CoreGfx::uniformBuffers[entry.id], entry.slot ) ) { return false; }
	}

	return true;
}


bool CoreGfx::shader_bind_uniform_buffers_compute( const u32 shaderID )
{
	Assert( shaderID < CoreGfx::shaderCount );
	const UniformBufferBindTable &table = uniformBufferBindTableShaderCompute[shaderID];
	if( table.buffers == nullptr ) { return true; }

	for( u32 i = 0; i < table.count; i++ )
	{
		const UniformBufferBindEntry &entry = table.buffers[i];
		if( !api_uniform_buffer_bind_compute( CoreGfx::uniformBuffers[entry.id], entry.slot ) ) { return false; }
	}

	return true;
}


void GfxShader::init( const u32 shaderID, const ShaderEntry &shaderEntry )
{
#if GRAPHICS_ENABLED
	this->id = shaderID;
	ErrorIf( !CoreGfx::api_shader_init( resource, shaderID, shaderEntry ), "Failed to init shader! (%u)", this->id );
#endif
}


void GfxShader::free()
{
#if GRAPHICS_ENABLED
	ErrorIf( !CoreGfx::api_shader_free( resource ), "Failed to free shader! (%u)", this->id );
#endif
}


void Gfx::set_shader_globals( const CoreGfxUniformBuffer::UniformsPipeline_t &globals )
{
#if GRAPHICS_ENABLED
	if( CoreGfx::state.pipelineUniforms == globals ) { return; }
	CoreGfx::state.pipelineUniforms = globals;
	//state_change_break_batches();
	globals.upload();
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// GfxVertexBuffer: gfx.hpp
// GfxInstanceBuffer: gfx.hpp
// GfxIndexBuffer: gfx.hpp
// GfxUniformBuffer: gfx.hpp
// GfxConstantBuffer: gfx.hpp
// GfxMutableBuffer: gfx.hpp

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Texture

void GfxTexture::init_2d( void *data, const u16 width, const u16 height, const GfxColorFormat &format )
{
#if GRAPHICS_ENABLED
	ErrorIf( !CoreGfx::api_texture_init( resource, data, width, height, 1, format ),
		"Failed to init GfxTexture!" );
#endif
}


void GfxTexture::init_2d( void *data, const u16 width, const u16 height, const u16 levels,
	const GfxColorFormat &format )
{
#if GRAPHICS_ENABLED
	ErrorIf( levels == 0,
		"Must have at least one mip level (highest resolution)" );
	ErrorIf( !CoreGfx::api_texture_init( resource, data, width, height, levels, format ),
		"Failed to init GfxTexture!" );
#endif
}


void GfxTexture::free()
{
#if GRAPHICS_ENABLED
	if( resource == nullptr ) { return; }
	ErrorIf( !CoreGfx::api_texture_free( resource ),
		"Failed to free GfxTexture!" );
#endif
}


void Gfx::bind_texture( const int slot, const GfxTexture &texture )
{
#if GRAPHICS_ENABLED
	Assert( slot >= 0 && slot < GFX_RENDER_TARGET_SLOT_COUNT );

	state_change_break_batches(); // Texture binding forces a batch break

	CoreGfx::state.boundTexture[slot] = texture.resource;
	ErrorIf( !CoreGfx::api_texture_bind( texture.resource, slot ),
		"Failed to bind GfxTexture to slot %d!", slot );
#endif
}


void Gfx::bind_texture( const int slot, const Texture texture )
{
#if GRAPHICS_ENABLED
	Assert( texture < CoreAssets::textureCount );
	bind_texture( slot, CoreGfx::textures[texture] );
#endif
}


void Gfx::release_texture( const int slot, const GfxTexture &texture )
{
#if GRAPHICS_ENABLED
	Assert( slot >= 0 && slot < GFX_RENDER_TARGET_SLOT_COUNT );

	if( CoreGfx::state.boundTexture[slot] != texture.resource ) { return; }

	CoreGfx::state.boundTexture[slot] = nullptr;
	ErrorIf( !CoreGfx::api_texture_release( texture.resource, slot ),
		"Failed to release GfxTexture from slot %d!", slot );
#endif
}


void Gfx::release_texture( const int slot, const Texture texture )
{
#if GRAPHICS_ENABLED
	Assert( texture < CoreAssets::textureCount );
	release_texture( slot, CoreGfx::textures[texture] );
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Render Target

void GfxRenderTarget::init( const u16 width, const u16 height, const GfxRenderTargetDescription &desc )
{
#if GRAPHICS_ENABLED
	ErrorIf( !CoreGfx::api_render_target_init( resource,
		textureColor.resource, textureDepth.resource, width, height, desc ),
		"Failed to init RenderTarget2D!" );

	this->width = width;
	this->height = height;
	this->desc = desc;
	this->slot = -1;
#endif
}


void GfxRenderTarget::free()
{
#if GRAPHICS_ENABLED
	if( resource == nullptr ) { return; }
	ErrorIf( !CoreGfx::api_render_target_free( resource, textureColor.resource, textureDepth.resource ),
		"Failed to free RenderTarget2D!" );
#endif
}


void GfxRenderTarget::resize( const u16 width, const u16 height )
{
#if GRAPHICS_ENABLED
	ErrorIf( resource == nullptr, "Attempting to resize RenderTarget2D that does not exist!" );
	free();
	init( width, height, desc );
#endif
}


void GfxRenderTarget::copy( GfxRenderTarget &source )
{
#if GRAPHICS_ENABLED
	ErrorIf( resource == nullptr, "Attempting to copy GfxRenderTarget that does not exist!" );
	ErrorIf( source.resource == nullptr, "Attempting to copy from GfxRenderTarget that does not exist!" );

	const bool success = CoreGfx::api_render_target_copy(
		source.resource, source.textureColor.resource, source.textureDepth.resource,
		resource, textureColor.resource, textureDepth.resource );

	ErrorIf( !success, "Copy GfxRenderTarget failed!" );
#endif
}


void GfxRenderTarget::copy_part( GfxRenderTarget &source,
	int srcX, int srcY, int dstX, int dstY, u16 w, u16 h )
{
#if GRAPHICS_ENABLED
	ErrorIf( resource == nullptr, "Attempting to copy GfxRenderTarget that does not exist!" );
	ErrorIf( source.resource == nullptr, "Attempting to copy from GfxRenderTarget that does not exist!" );

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
		const u16 dstWidth = this->width;
		const u16 dstHeight = this->height;

		GfxRenderPass pass;
		pass.set_name_f( "%s%s", __FUNCTION__, " - Depth" );
		pass.target( 0, *this );
		Gfx::render_pass_begin( pass );
		{
			GfxRenderCommand cmd;
			cmd.shader( Shader::SHADER_DEFAULT_COPY_DEPTH );
			cmd.depth_function( GfxDepthFunction_ALWAYS );
			cmd.depth_write_mask( GfxDepthWrite_ALL );
			Gfx::render_command_execute( cmd, [=]()
				{
					Gfx::set_matrix_mvp_2d_orthographic( 0.0, 0.0, 1.0, 0.0, dstWidth, dstHeight );

					Gfx::bind_texture( 0, source.textureDepth );

					const u16 srcY1Inv = source.height - srcY1;
					const u16 u1 = static_cast<u16>( ( ( srcX1 ) /
						static_cast<float>( source.width ) ) * 0xFFFF );
					const u16 v1 = static_cast<u16>( ( ( srcY1 ) /
						static_cast<float>( source.height ) ) * 0xFFFF );
					const u16 u2 = static_cast<u16>( ( ( srcX1 + copyWidth ) /
						static_cast<float>( source.width ) ) * 0xFFFF );
					const u16 v2 = static_cast<u16>( ( ( srcY1 + copyHeight ) /
						static_cast<float>( source.height ) ) * 0xFFFF );

					draw_quad_uv( dstX1, dstY1, dstX1 + copyWidth, dstY1 + copyHeight, u1, v1, u2, v2 );
				} );
		}
		Gfx::render_pass_end( pass );
	}
#endif

	// Copy Part
	const bool success = CoreGfx::api_render_target_copy_part(
		source.resource, source.textureColor.resource, source.textureDepth.resource,
		resource, textureColor.resource, textureDepth.resource,
		srcX1, srcY1, dstX1, dstY1, copyWidth, copyHeight );

	ErrorIf( !success, "Copy GfxRenderTarget failed!" );
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Render Command

void GfxRenderCommand::shader( const GfxShader &shader )
{
#if GRAPHICS_ENABLED
	Assert( shader.resource != nullptr );
	this->pipeline.shader = shader.resource;
#endif
}


void GfxRenderCommand::shader( const Shader shader )
{
#if GRAPHICS_ENABLED
	Assert( shader < CoreGfx::shaderCount );
	this->shader( CoreGfx::shaders[shader] );
#endif
}


void GfxRenderCommand::set_pipeline_description( const GfxPipelineDescription &description )
{
#if GRAPHICS_ENABLED
	this->pipeline.description = description;
#endif
}


void GfxRenderCommand::raster_fill_mode( const GfxFillMode &mode )
{
#if GRAPHICS_ENABLED
	Assert( mode < GFXRASTERFILLMODE_COUNT );
	this->pipeline.description.rasterFillMode = mode;
#endif
}


void GfxRenderCommand::raster_cull_mode( const GfxCullMode &mode )
{
#if GRAPHICS_ENABLED
	Assert( mode < GFXRASTERCULLMODE_COUNT );
	this->pipeline.description.rasterCullMode = mode;
#endif
}


void GfxRenderCommand::blend_enabled( const bool enabled )
{
#if GRAPHICS_ENABLED
	this->pipeline.description.blendEnabled = enabled;
#endif
}


void GfxRenderCommand::blend_mode_color( const GfxBlendFactor &srcFactor,
	const GfxBlendFactor &dstFactor, const GfxBlendOperation &operation )
{
#if GRAPHICS_ENABLED
	Assert( srcFactor < GFXBLENDFACTOR_COUNT );
	Assert( dstFactor < GFXBLENDFACTOR_COUNT );
	Assert( operation < GFXBLENDOPERATION_COUNT );
	this->pipeline.description.blendSrcFactorColor = srcFactor;
	this->pipeline.description.blendDstFactorColor = dstFactor;
	this->pipeline.description.blendOperationColor = operation;
#endif
}


void GfxRenderCommand::blend_mode_alpha( const GfxBlendFactor &srcFactor,
	const GfxBlendFactor &dstFactor, const GfxBlendOperation &operation )
{
#if GRAPHICS_ENABLED
	Assert( srcFactor < GFXBLENDFACTOR_COUNT );
	Assert( dstFactor < GFXBLENDFACTOR_COUNT );
	Assert( operation < GFXBLENDOPERATION_COUNT );
	this->pipeline.description.blendSrcFactorAlpha = srcFactor;
	this->pipeline.description.blendDstFactorAlpha = dstFactor;
	this->pipeline.description.blendOperationAlpha = operation;
#endif
}


void GfxRenderCommand::blend_write_mask( const GfxBlendWrite &mask )
{
#if GRAPHICS_ENABLED
	this->pipeline.description.blendColorWriteMask = mask;
#endif
}


void GfxRenderCommand::depth_function( const GfxDepthFunction &mode )
{
#if GRAPHICS_ENABLED
	Assert( mode < GFXDEPTHFUNCTION_COUNT );
	this->pipeline.description.depthFunction = mode;
#endif
}


void GfxRenderCommand::depth_write_mask( const GfxDepthWrite &mask )
{
#if GRAPHICS_ENABLED
	this->pipeline.description.depthWriteMask = mask;
#endif
}


void CoreGfx::render_command_execute_begin( const GfxRenderCommand &command )
{
#if GRAPHICS_ENABLED
	if( CoreGfx::state.renderCommandAllowBatchBreak ) { Gfx::quad_batch_break(); }
	CoreGfx::state.renderCommandActive = &command;
#endif
}


void CoreGfx::render_command_execute_end( const GfxRenderCommand &command )
{
#if GRAPHICS_ENABLED
	if( CoreGfx::state.renderCommandAllowBatchBreak ) { Gfx::quad_batch_break(); }
	CoreGfx::state.renderCommandActive = nullptr;
#endif
}


void Gfx::render_command_execute( const GfxRenderCommand &command )
{
#if GRAPHICS_ENABLED
	AssertMsg( CoreGfx::state.renderCommandActive == nullptr, "GfxCommands cannot be nested!" );

	CoreGfx::render_command_execute_begin( command );
	CoreGfx::api_render_command_execute( command );
	CoreGfx::render_command_execute_end( command );
	CoreGfx::api_render_command_execute_post( command );
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Render Graph

List<GfxRenderCommand> &GfxRenderGraph::command_list()
{
	if( commandList == USIZE_MAX )
	{
		commandList = GfxRenderGraph::renderGraphCommandListCurrent++;
		if( commandList == GfxRenderGraph::renderGraphCommandLists.current )
		{
			List<GfxRenderCommand> list;
			list.init();
			return GfxRenderGraph::renderGraphCommandLists.add( static_cast<List<GfxRenderCommand> &&>( list ) );
		}
	}

	return GfxRenderGraph::renderGraphCommandLists[commandList];
}


const List<GfxRenderCommand> &GfxRenderGraph::command_list() const
{
	Assert( commandList != USIZE_MAX );
	return GfxRenderGraph::renderGraphCommandLists[commandList];
}


void GfxRenderGraph::add_command( const GfxRenderCommand &command )
{
#if GRAPHICS_ENABLED
	command_list().add( command );
#endif
}


void GfxRenderGraph::add_command( GfxRenderCommand &&command )
{
#if GRAPHICS_ENABLED
	command_list().add( static_cast<GfxRenderCommand &&>( command ) );
#endif
}


static usize render_graph_partition( GfxRenderGraph &graph, usize left, usize right,
	bool ( *compare )( GfxRenderCommand &, GfxRenderCommand & ) )
{
	List<GfxRenderCommand> &commandList = graph.command_list();
	GfxRenderCommand &pivot = commandList[right];

	usize i = left;
	for( usize j = left; j < right; j++ )
	{
		GfxRenderCommand &cmdA  = commandList[j];
		if( compare( cmdA, pivot ) ) { commandList.swap( i, j ); i++; }
	}
	commandList.swap( i, right );

	return i;
}


static void render_graph_quicksort( GfxRenderGraph &graph, usize left, usize right,
	bool ( *compare )( GfxRenderCommand &, GfxRenderCommand & ) )
{
	if( left >= right ) { return; }
	usize pivotIndex = render_graph_partition( graph, left, right, compare );
	if( pivotIndex > 0 ) { render_graph_quicksort( graph, left, pivotIndex - 1LLU, compare ); }
	render_graph_quicksort( graph, pivotIndex + 1LLU, right, compare );
}


void Gfx::render_graph_sort( GfxRenderGraph &graph,
	bool ( *compare )( GfxRenderCommand &, GfxRenderCommand & ) )
{
#if GRAPHICS_ENABLED
	if( graph.commandList == USIZE_MAX ) { return; }
	List<GfxRenderCommand> &commandList = graph.command_list();
	if( commandList.current <= 1 ) { return; }
	render_graph_quicksort( graph, 0LLU, commandList.current - 1LLU, compare );
#endif
}


void Gfx::render_graph_execute( const GfxRenderGraph &graph )
{
#if GRAPHICS_ENABLED
	if( graph.commandList == USIZE_MAX ) { return; }
	const List<GfxRenderCommand> &commandList = graph.command_list();
	for( const GfxRenderCommand &command : commandList ) { Gfx::render_command_execute( command ); }
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Render Pass

void GfxRenderPass::target( const int slot, const GfxRenderTarget &target )
{
#if GRAPHICS_ENABLED
	Assert( slot >= 0 && slot < GFX_RENDER_TARGET_SLOT_COUNT );
	Assert( target.resource != nullptr );
	this->targets[slot] = target.resource;
#endif
}


void GfxRenderPass::set_name( const char *name )
{
#if GRAPHICS_ENABLED
#if COMPILE_DEBUG
	snprintf( this->name, sizeof( this->name ), "%s", name );
#endif
#endif
}


void GfxRenderPass::set_name_f( const char *name, ... )
{
#if GRAPHICS_ENABLED
#if COMPILE_DEBUG
	va_list args;
	va_start( args, name );
	vsnprintf( this->name, sizeof( this->name ), name, args );
	va_end( args );
#endif
#endif
}


void Gfx::render_pass_begin( const GfxRenderPass &pass )
{
#if GRAPHICS_ENABLED
	Assert( CoreGfx::state.renderPassActive == nullptr )

	if( CoreGfx::state.renderPassAllowBatchBreak ) { Gfx::quad_batch_break(); }

	CoreGfx::state.renderPassActive = &pass;
	CoreGfx::api_render_pass_begin( pass );
#endif
}


void Gfx::render_pass_end( const GfxRenderPass &pass )
{
#if GRAPHICS_ENABLED
	Assert( CoreGfx::state.renderPassActive == &pass );

	if( CoreGfx::state.renderPassAllowBatchBreak ) { Gfx::quad_batch_break(); }

	CoreGfx::api_render_pass_end( pass );
	CoreGfx::state.renderPassActive = nullptr;
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Utility

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
// Quad Batch (Built-in)

void CoreGfx::quad_batch_frame_begin()
{
#if GRAPHICS_ENABLED
	CoreGfx::batch.active = true;
	CoreGfx::batch.batch_begin();
#endif
}


void CoreGfx::quad_batch_frame_end()
{
#if GRAPHICS_ENABLED
	CoreGfx::batch.batch_end();
	CoreGfx::batch.active = false;
#endif
}


void Gfx::quad_batch_break()
{
#if GRAPHICS_ENABLED
	if( !CoreGfx::batch.active ) { return; }
	CoreGfx::batch.batch_break();
#endif
}


bool Gfx::quad_batch_can_break()
{
#if GRAPHICS_ENABLED
	if( !CoreGfx::batch.active ) { return false; }
	return CoreGfx::batch.can_break();
#else
	return true;
#endif
}


void Gfx::quad_batch_break_check()
{
#if GRAPHICS_ENABLED
	if( !CoreGfx::batch.active ) { return; }
	CoreGfx::batch.break_check();
#endif
}


void Gfx::quad_batch_write( const GfxQuadBatch<GfxVertex::BuiltinVertex>::Quad &quad,
	const GfxTexture *const texture )
{
#if GRAPHICS_ENABLED
	Assert( CoreGfx::batch.active );

	if( LIKELY( texture != nullptr ) && CoreGfx::state.boundTexture[0] != texture->resource )
	{
		Gfx::bind_texture( 0, *texture );
	}

	CoreGfx::batch.write( quad );
#endif
}


void Gfx::quad_batch_write( const float x1, const float y1, const float x2, const float y2,
	const u16 u1, const u16 v1, const u16 u2, const u16 v2,
	const Color c1, const Color c2, const Color c3, const Color c4,
	const GfxTexture *const texture, const float depth )
{
#if GRAPHICS_ENABLED
	Assert( CoreGfx::batch.active );

	if( LIKELY( texture != nullptr ) && CoreGfx::state.boundTexture[0] != texture->resource )
	{
		Gfx::bind_texture( 0, *texture );
	}

	const GfxQuadBatch<GfxVertex::BuiltinVertex>::Quad quad =
	{
		{ { x1, y1, depth }, { u1, v1 }, { c1.r, c1.g, c1.b, c1.a } },
		{ { x2, y1, depth }, { u2, v1 }, { c2.r, c2.g, c2.b, c2.a } },
		{ { x1, y2, depth }, { u1, v2 }, { c3.r, c3.g, c3.b, c3.a } },
		{ { x2, y2, depth }, { u2, v2 }, { c4.r, c4.g, c4.b, c4.a } },
	};

	CoreGfx::batch.write( quad );
#endif
}


void Gfx::quad_batch_write( const float x1, const float y1, const float x2, const float y2,
	const float x3, const float y3, const float x4, const float y4,
	const u16 u1, const u16 v1, const u16 u2, const u16 v2,
	const Color c1, const Color c2, const Color c3, const Color c4,
	const GfxTexture *const texture, const float depth )
{
#if GRAPHICS_ENABLED
	Assert( CoreGfx::batch.active );

	if( LIKELY( texture != nullptr ) && CoreGfx::state.boundTexture[0] != texture->resource )
	{
		Gfx::bind_texture( 0, *texture );
	}

	const GfxQuadBatch<GfxVertex::BuiltinVertex>::Quad quad =
	{
		{ { x1, y1, depth }, { u1, v1 }, { c1.r, c1.g, c1.b, c1.a } },
		{ { x2, y2, depth }, { u2, v1 }, { c2.r, c2.g, c2.b, c2.a } },
		{ { x3, y3, depth }, { u1, v2 }, { c3.r, c3.g, c3.b, c3.a } },
		{ { x4, y4, depth }, { u2, v2 }, { c4.r, c4.g, c4.b, c4.a } },
	};

	CoreGfx::batch.write( quad );
#endif
}


void Gfx::quad_batch_write( const float x1, const float y1, const float x2, const float y2,
	const u16 u1, const u16 v1, const u16 u2, const u16 v2, const Color color,
	const GfxTexture *const texture, const float depth )
{
#if GRAPHICS_ENABLED
	Assert( CoreGfx::batch.active );

	if( LIKELY( texture != nullptr ) && CoreGfx::state.boundTexture[0] != texture->resource )
	{
		Gfx::bind_texture( 0, *texture );
	}

	const GfxQuadBatch<GfxVertex::BuiltinVertex>::Quad quad =
	{
		{ { x1, y1, depth }, { u1, v1 }, { color.r, color.g, color.b, color.a } },
		{ { x2, y1, depth }, { u2, v1 }, { color.r, color.g, color.b, color.a } },
		{ { x1, y2, depth }, { u1, v2 }, { color.r, color.g, color.b, color.a } },
		{ { x2, y2, depth }, { u2, v2 }, { color.r, color.g, color.b, color.a } },
	};

	CoreGfx::batch.write( quad );
#endif
}


void Gfx::quad_batch_write( const float x1, const float y1, const float x2, const float y2,
	const float x3, const float y3, const float x4, const float y4,
	const u16 u1, const u16 v1, const u16 u2, const u16 v2, const Color color,
	const GfxTexture *const texture, const float depth )
{
#if GRAPHICS_ENABLED
	Assert( CoreGfx::batch.active );

	if( LIKELY( texture != nullptr ) && CoreGfx::state.boundTexture[0] != texture->resource )
	{
		Gfx::bind_texture( 0, *texture );
	}

	const GfxQuadBatch<GfxVertex::BuiltinVertex>::Quad quad =
	{
		{ { x1, y1, depth }, { u1, v1 }, { color.r, color.g, color.b, color.a } },
		{ { x2, y2, depth }, { u2, v1 }, { color.r, color.g, color.b, color.a } },
		{ { x3, y3, depth }, { u1, v2 }, { color.r, color.g, color.b, color.a } },
		{ { x4, y4, depth }, { u2, v2 }, { color.r, color.g, color.b, color.a } },
	};

	CoreGfx::batch.write( quad );
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////