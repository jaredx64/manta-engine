#include <manta/gfx.hpp>
#include <config.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CoreGfx::api_init()
{
	return true;
}


bool CoreGfx::api_free()
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CoreGfx::api_frame_begin()
{
}


void CoreGfx::api_frame_end()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CoreGfx::api_swapchain_init( const u16 width, const u16 height, const bool fullscreen )
{
	return true;
}


bool CoreGfx::api_swapchain_free()
{
	return true;
}


bool CoreGfx::api_swapchain_resize( u16 width, u16 height, bool fullscreen )
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CoreGfx::api_viewport_init( const u16 width, const u16 height, const bool fullscreen )
{
	return true;
}


bool CoreGfx::api_viewport_free()
{
	return true;
}


bool CoreGfx::api_viewport_resize( const u16 width, const u16 height, const bool fullscreen )
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CoreGfx::api_set_raster_state( const GfxRasterState &state )
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CoreGfx::api_set_sampler_state( const GfxSamplerState &state )
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CoreGfx::api_set_blend_state( const GfxBlendState &state )
{
	return true;
}


void CoreGfx::api_clear_color( const Color color )
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CoreGfx::api_set_depth_state( const GfxDepthState &state )
{
	return true;
}


void CoreGfx::api_clear_depth( const float depth )
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CoreGfx::api_shader_init( GfxShaderResource *&resource,
	const u32 shaderID, const struct ShaderEntry &shaderEntry )
{
	return true;
}


bool CoreGfx::api_shader_free( GfxShaderResource *&resource )
{
	return true;
}


bool CoreGfx::api_shader_bind( GfxShaderResource *&resource )
{
	return true;
}

bool CoreGfx::api_shader_dispatch( GfxShaderResource *&resource,
	const u32 x, const u32 y, const u32 z )
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CoreGfx::api_vertex_buffer_init_dynamic( GfxVertexBufferResource *&resource, const u32 vertexFormatID,
	const GfxCPUAccessMode accessMode, const u32 size, const u32 stride )
{
	return true;
}


bool CoreGfx::api_vertex_buffer_init_static( GfxVertexBufferResource *&resource, const u32 vertexFormatID,
	const GfxCPUAccessMode accessMode, const void *const data,
	const u32 size, const u32 stride )
{
	return true;
}


bool CoreGfx::api_vertex_buffer_free( GfxVertexBufferResource *&resource )
{
	return true;
}


void CoreGfx::api_vertex_buffer_write_begin( GfxVertexBufferResource *const resource )
{
}


void CoreGfx::api_vertex_buffer_write_end( GfxVertexBufferResource *const resource )
{
}


bool CoreGfx::api_vertex_buffer_write( GfxVertexBufferResource *const resource, const void *const data, const u32 size )
{
	return true;
}


u32 CoreGfx::api_vertex_buffer_current( const GfxVertexBufferResource *const resource )
{
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


bool CoreGfx::api_instance_buffer_init_dynamic( GfxInstanceBufferResource *&resource, const u32 instanceFormatID,
	const GfxCPUAccessMode accessMode, const u32 size, const u32 stride )
{
	return true;
}


bool CoreGfx::api_instance_buffer_init_static( GfxInstanceBufferResource *&resource, const u32 instanceFormatID,
	const GfxCPUAccessMode accessMode, const void *const data,
	const u32 size, const u32 stride )
{
	return true;
}


bool CoreGfx::api_instance_buffer_free( GfxInstanceBufferResource *&resource )
{
	return true;
}


void CoreGfx::api_instance_buffer_write_begin( GfxInstanceBufferResource *const resource )
{
}


void CoreGfx::api_instance_buffer_write_end( GfxInstanceBufferResource *const resource )
{
}


bool CoreGfx::api_instance_buffer_write( GfxInstanceBufferResource *const resource,
	const void *const data, const u32 size )
{
	return true;
}


u32 CoreGfx::api_instance_buffer_current( const GfxInstanceBufferResource *const resource )
{
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CoreGfx::api_index_buffer_init( GfxIndexBufferResource *&resource,
	void *data, const u32 size, const double indToVertRatio,
	const GfxIndexBufferFormat format, const GfxCPUAccessMode accessMode )
{
	return true;
}


bool CoreGfx::api_index_buffer_free( GfxIndexBufferResource *&resource )
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CoreGfx::api_uniform_buffer_init( GfxUniformBufferResource *&resource, const char *name,
	const int index, const u32 size )
{
	return true;
}


bool CoreGfx::api_uniform_buffer_free( GfxUniformBufferResource *&resource )
{
	return true;
}


void CoreGfx::api_uniform_buffer_write_begin( GfxUniformBufferResource *const resource )
{
}


void CoreGfx::api_uniform_buffer_write_end( GfxUniformBufferResource *const resource )
{
}


bool CoreGfx::api_uniform_buffer_write( GfxUniformBufferResource *const resource, const void *data )
{
	return true;
}


bool CoreGfx::api_uniform_buffer_bind_vertex( GfxUniformBufferResource *const resource, const int slot )
{
	return true;
}


bool CoreGfx::api_uniform_buffer_bind_fragment( GfxUniformBufferResource *const resource, const int slot )
{
	return true;
}


bool CoreGfx::api_uniform_buffer_bind_compute( GfxUniformBufferResource *const resource, const int slot )
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// TODO: GfxConstantBuffer (StructuredBuffer)
// TODO: GfxMutableBuffer (RWStructuredBuffer)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// TODO: GfxTexture1D

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CoreGfx::api_texture_2d_init( GfxTexture2DResource *&resource, void *pixels,
	const u16 width, const u16 height, const u16 levels, const GfxColorFormat &format )
{
	return true;
}


bool CoreGfx::api_texture_2d_free( GfxTexture2DResource *&resource )
{
	return true;
}


bool CoreGfx::api_texture_2d_bind( GfxTexture2DResource *const resource, const int slot )
{
	return true;
}


bool CoreGfx::api_texture_2d_release( GfxTexture2DResource *const resource, const int slot )
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// TODO: GfxTexture2DArray
// TODO: GfxTexture3D
// TODO: GfxTexture3DArray
// TODO: GfxTextureCube
// TODO: GfxTextureCubeArray

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CoreGfx::api_render_target_2d_init( GfxRenderTarget2DResource *&resource,
	GfxTexture2DResource *&resourceColor, GfxTexture2DResource *&resourceDepth,
	const u16 width, const u16 height,
	const GfxRenderTargetDescription &desc )
{
	return true;
}


bool CoreGfx::api_render_target_2d_free( GfxRenderTarget2DResource *&resource,
	GfxTexture2DResource *&resourceColor,
	GfxTexture2DResource *&resourceDepth )
{
	return true;
}


bool CoreGfx::api_render_target_2d_copy(
	GfxRenderTarget2DResource *&srcResource,
	GfxTexture2DResource *&srcResourceColor, GfxTexture2DResource *&srcResourceDepth,
	GfxRenderTarget2DResource *&dstResource,
	GfxTexture2DResource *&dstResourceColor, GfxTexture2DResource *&dstResourceDepth )
{
	return true;
}


bool CoreGfx::api_render_target_2d_copy_part(
	GfxRenderTarget2DResource *&srcResource,
	GfxTexture2DResource *&srcResourceColor, GfxTexture2DResource *&srcResourceDepth,
	GfxRenderTarget2DResource *&dstResource,
	GfxTexture2DResource *&dstResourceColor, GfxTexture2DResource *&dstResourceDepth,
	u16 srcX, u16 srcY, u16 dstX, u16 dstY, u16 width, u16 height )
{
	return true;
}


bool CoreGfx::api_render_target_2d_buffer_read_color( GfxRenderTarget2DResource *&resource,
	GfxTexture2DResource *&resourceColor,
	void *buffer, const u32 size )
{
	return true;
}


bool api_render_target_2d_buffer_read_depth( GfxRenderTarget2DResource *&resource,
	GfxTexture2DResource *&resourceDepth,
	void *buffer, const u32 size )
{
	return true;
}


bool CoreGfx::api_render_target_2d_buffer_write_color( GfxRenderTarget2DResource *&resource,
	GfxTexture2DResource *&resourceColor,
	const void *const buffer, const u32 size )
{
	return true;
}


bool CoreGfx::api_render_target_2d_buffer_write_depth( GfxRenderTarget2DResource *&resource,
	GfxTexture2DResource *&resourceDepth,
	const void *const buffer, const u32 size )
{
	return true;
}


bool CoreGfx::api_render_target_2d_bind( GfxRenderTarget2DResource *const resource, int slot )
{
	return true;
}


bool CoreGfx::api_render_target_2d_release( GfxRenderTarget2DResource *const resource, const int slot )
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CoreGfx::api_draw(
		const GfxVertexBufferResource *const resourceVertex, const u32 vertexCount,
		const GfxInstanceBufferResource *const resourceInstance, const u32 instanceCount,
		const GfxIndexBufferResource *const resourceIndex,
		const GfxPrimitiveType type )
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////