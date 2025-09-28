#include <manta/gfx.hpp>
#include <config.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CoreGfx::rb_init()
{
	return true;
}


bool CoreGfx::rb_free()
{
	return true;
}


void CoreGfx::rb_frame_begin()
{
}


void CoreGfx::rb_frame_end()
{
}


void CoreGfx::rb_clear_color( const Color color )
{
}


void CoreGfx::rb_clear_depth( const float depth )
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CoreGfx::rb_swapchain_init( const u16 width, const u16 height, const bool fullscreen )
{
	return true;
}


bool CoreGfx::rb_swapchain_free()
{
	return true;
}


bool CoreGfx::rb_swapchain_resize( u16 width, u16 height, bool fullscreen )
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CoreGfx::rb_viewport_init( const u16 width, const u16 height, const bool fullscreen )
{
	return true;
}


bool CoreGfx::rb_viewport_free()
{
	return true;
}


bool CoreGfx::rb_viewport_resize( const u16 width, const u16 height, const bool fullscreen )
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CoreGfx::rb_set_raster_state( const GfxRasterState &state )
{
	return true;
}


bool CoreGfx::rb_set_sampler_state( const GfxSamplerState &state )
{
	return true;
}


bool CoreGfx::rb_set_blend_state( const GfxBlendState &state )
{
	return true;
}


bool CoreGfx::rb_set_depth_state( const GfxDepthState &state )
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CoreGfx::rb_index_buffer_init( GfxIndexBufferResource *&resource,
	void *data, const u32 size, const double indToVertRatio,
	const GfxIndexBufferFormat format, const GfxCPUAccessMode accessMode )
{
	return true;
}


bool CoreGfx::rb_index_buffer_free( GfxIndexBufferResource *&resource )
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CoreGfx::rb_vertex_buffer_init_dynamic( GfxVertexBufferResource *&resource, const u32 vertexFormatID,
	const GfxCPUAccessMode accessMode, const u32 size, const u32 stride )
{
	return true;
}


bool CoreGfx::rb_vertex_buffer_init_static( GfxVertexBufferResource *&resource, const u32 vertexFormatID,
	const GfxCPUAccessMode accessMode, const void *const data,
	const u32 size, const u32 stride )
{
	return true;
}


bool CoreGfx::rb_vertex_buffer_free( GfxVertexBufferResource *&resource )
{
	return true;
}


bool CoreGfx::rb_vertex_buffer_draw( GfxVertexBufferResource *&resource,
	const GfxPrimitiveType type )
{
	return true;
}


bool CoreGfx::rb_vertex_buffer_draw_indexed( GfxVertexBufferResource *&resource,
	GfxIndexBufferResource *&resourceIndexBuffer, const GfxPrimitiveType type )
{
	return true;
}


void CoreGfx::rb_vertex_buffer_write_begin( GfxVertexBufferResource *&resource )
{
}


void CoreGfx::rb_vertex_buffer_write_end( GfxVertexBufferResource *&resource )
{
}


bool CoreGfx::rb_vertex_buffer_write( GfxVertexBufferResource *&resource, const void *const data, const u32 size )
{
	return true;
}


u32 CoreGfx::rb_vertex_buffer_current( GfxVertexBufferResource *&resource )
{
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CoreGfx::rb_uniform_buffer_init( GfxUniformBufferResource *&resource, const char *name,
	const int index, const u32 size )
{
	return true;
}


bool CoreGfx::rb_uniform_buffer_free( GfxUniformBufferResource *&resource )
{
	return true;
}


void CoreGfx::rb_constant_buffered_write_begin( GfxUniformBufferResource *&resource )
{
}


void CoreGfx::rb_constant_buffered_write_end( GfxUniformBufferResource *&resource )
{
}


bool CoreGfx::rb_constant_buffered_write( GfxUniformBufferResource *&resource, const void *data )
{
	return true;
}


bool CoreGfx::rb_uniform_buffer_bind_vertex( GfxUniformBufferResource *&resource, const int slot )
{
	return true;
}


bool CoreGfx::rb_uniform_buffer_bind_fragment( GfxUniformBufferResource *&resource, const int slot )
{
	return true;
}


bool CoreGfx::rb_uniform_buffer_bind_compute( GfxUniformBufferResource *&resource, const int slot )
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CoreGfx::rb_texture_2d_init( GfxTexture2DResource *&resource, void *pixels,
                                  const u16 width, const u16 height, const u16 levels, const GfxColorFormat &format )
{
	return true;
}


bool CoreGfx::rb_texture_2d_free( GfxTexture2DResource *&resource )
{
	return true;
}


bool CoreGfx::rb_texture_2d_bind( const GfxTexture2DResource *const &resource, const int slot )
{
	return true;
}


bool CoreGfx::rb_texture_2d_release( const GfxTexture2DResource *const &resource, const int slot )
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CoreGfx::rb_render_target_2d_init( GfxRenderTarget2DResource *&resource,
	GfxTexture2DResource *&resourceColor, GfxTexture2DResource *&resourceDepth,
	const u16 width, const u16 height,
	const GfxRenderTargetDescription &desc )
{
	return true;
}


bool CoreGfx::rb_render_target_2d_free( GfxRenderTarget2DResource *&resource,
	GfxTexture2DResource *&resourceColor,
	GfxTexture2DResource *&resourceDepth )
{
	return true;
}


bool CoreGfx::rb_render_target_2d_copy(
	GfxRenderTarget2DResource *&srcResource,
	GfxTexture2DResource *&srcResourceColor, GfxTexture2DResource *&srcResourceDepth,
	GfxRenderTarget2DResource *&dstResource,
	GfxTexture2DResource *&dstResourceColor, GfxTexture2DResource *&dstResourceDepth )
{
	return true;
}


bool CoreGfx::rb_render_target_2d_copy_part(
	GfxRenderTarget2DResource *&srcResource,
	GfxTexture2DResource *&srcResourceColor, GfxTexture2DResource *&srcResourceDepth,
	GfxRenderTarget2DResource *&dstResource,
	GfxTexture2DResource *&dstResourceColor, GfxTexture2DResource *&dstResourceDepth,
	u16 srcX, u16 srcY, u16 dstX, u16 dstY, u16 width, u16 height )
{
	return true;
}


bool CoreGfx::rb_render_target_2d_buffer_read_color( GfxRenderTarget2DResource *&resource,
                                                     GfxTexture2DResource *&resourceColor,
                                                     void *buffer, const u32 size )
{
	return true;
}


bool rb_render_target_2d_buffer_read_depth( GfxRenderTarget2DResource *&resource,
	GfxTexture2DResource *&resourceDepth,
	void *buffer, const u32 size )
{
	return true;
}


bool CoreGfx::rb_render_target_2d_buffered_write_color( GfxRenderTarget2DResource *&resource,
	GfxTexture2DResource *&resourceColor,
	const void *const buffer, const u32 size )
{
	return true;
}


bool CoreGfx::rb_render_target_2d_bind( const GfxRenderTarget2DResource *const &resource, int slot )
{
	return true;
}


bool CoreGfx::rb_render_target_2d_release( const GfxRenderTarget2DResource *const &resource, const int slot )
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CoreGfx::rb_shader_init( GfxShaderResource *&resource, const u32 shaderID, const struct ShaderEntry &shaderEntry )
{
	return true;
}


bool CoreGfx::rb_shader_free( GfxShaderResource *&resource )
{
	return true;
}


bool CoreGfx::rb_shader_bind( GfxShaderResource *&resource )
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////