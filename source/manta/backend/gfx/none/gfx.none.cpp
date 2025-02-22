#include <manta/gfx.hpp>
#include <config.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GfxCore::rb_init()
{
	return true;
}


bool GfxCore::rb_free()
{
	return true;
}


void GfxCore::rb_frame_begin()
{
}


void GfxCore::rb_frame_end()
{
}


void GfxCore::rb_clear_color( const Color color )
{
}


void GfxCore::rb_clear_depth( const float depth )
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GfxCore::rb_swapchain_init( const u16 width, const u16 height, const bool fullscreen )
{
	return true;
}


bool GfxCore::rb_swapchain_free()
{
	return true;
}


bool GfxCore::rb_swapchain_resize( u16 width, u16 height, bool fullscreen )
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GfxCore::rb_viewport_init( const u16 width, const u16 height, const bool fullscreen )
{
	return true;
}


bool GfxCore::rb_viewport_free()
{
	return true;
}


bool GfxCore::rb_viewport_resize( const u16 width, const u16 height, const bool fullscreen )
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GfxCore::rb_set_raster_state( const GfxRasterState &state )
{
	return true;
}


bool GfxCore::rb_set_sampler_state( const GfxSamplerState &state )
{
	return true;
}


bool GfxCore::rb_set_blend_state( const GfxBlendState &state )
{
	return true;
}


bool GfxCore::rb_set_depth_state( const GfxDepthState &state )
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GfxCore::rb_index_buffer_init( GfxIndexBufferResource *&resource,
	void *data, const u32 size, const double indToVertRatio,
	const GfxIndexBufferFormat format, const GfxCPUAccessMode accessMode )
{
	return true;
}


bool GfxCore::rb_index_buffer_free( GfxIndexBufferResource *&resource )
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GfxCore::rb_vertex_buffer_init_dynamic( GfxVertexBufferResource *&resource, const u32 vertexFormatID,
	const GfxCPUAccessMode accessMode, const u32 size, const u32 stride )
{
	return true;
}


bool GfxCore::rb_vertex_buffer_init_static( GfxVertexBufferResource *&resource, const u32 vertexFormatID,
	const GfxCPUAccessMode accessMode, const void *const data,
	const u32 size, const u32 stride )
{
	return true;
}


bool GfxCore::rb_vertex_buffer_free( GfxVertexBufferResource *&resource )
{
	return true;
}


bool GfxCore::rb_vertex_buffer_draw( GfxVertexBufferResource *&resource )
{
	return true;
}


bool GfxCore::rb_vertex_buffer_draw_indexed( GfxVertexBufferResource *&resource, GfxIndexBufferResource *&resourceIndexBuffer )
{
	return true;
}


void GfxCore::rb_vertex_buffered_write_begin( GfxVertexBufferResource *&resource )
{
}


void GfxCore::rb_vertex_buffered_write_end( GfxVertexBufferResource *&resource )
{
}


bool GfxCore::rb_vertex_buffered_write( GfxVertexBufferResource *&resource, const void *const data, const u32 size )
{
	return true;
}


u32 GfxCore::rb_vertex_buffer_current( GfxVertexBufferResource *&resource )
{
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GfxCore::rb_constant_buffer_init( GfxConstantBufferResource *&resource, const char *name,
	const int index, const u32 size )
{
	return true;
}


bool GfxCore::rb_constant_buffer_free( GfxConstantBufferResource *&resource )
{
	return true;
}


void GfxCore::rb_constant_buffered_write_begin( GfxConstantBufferResource *&resource )
{
}


void GfxCore::rb_constant_buffered_write_end( GfxConstantBufferResource *&resource )
{
}


bool GfxCore::rb_constant_buffered_write( GfxConstantBufferResource *&resource, const void *data )
{
	return true;
}


bool GfxCore::rb_constant_buffer_bind_vertex( GfxConstantBufferResource *&resource, const int slot )
{
	return true;
}


bool GfxCore::rb_constant_buffer_bind_fragment( GfxConstantBufferResource *&resource, const int slot )
{
	return true;
}


bool GfxCore::rb_constant_buffer_bind_compute( GfxConstantBufferResource *&resource, const int slot )
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GfxCore::rb_texture_2d_init( GfxTexture2DResource *&resource, void *pixels,
                                  const u16 width, const u16 height, const GfxColorFormat &format )
{
	return true;
}


bool GfxCore::rb_texture_2d_free( GfxTexture2DResource *&resource )
{
	return true;
}


bool GfxCore::rb_texture_2d_bind( const GfxTexture2DResource *const &resource, const int slot )
{
	return true;
}


bool GfxCore::rb_texture_2d_release( const int slot )
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GfxCore::rb_render_target_2d_init( GfxRenderTarget2DResource *&resource,
	GfxTexture2DResource *&resourceColor, GfxTexture2DResource *&resourceDepth,
	const u16 width, const u16 height,
	const GfxRenderTargetDescription &desc )
{
	return true;
}


bool GfxCore::rb_render_target_2d_free( GfxRenderTarget2DResource *&resource,
	GfxTexture2DResource *&resourceColor,
	GfxTexture2DResource *&resourceDepth )
{
	return true;
}


bool GfxCore::rb_render_target_2d_copy(
	GfxRenderTarget2DResource *&srcResource,
	GfxTexture2DResource *&srcResourceColor, GfxTexture2DResource *&srcResourceDepth,
	GfxRenderTarget2DResource *&dstResource,
	GfxTexture2DResource *&dstResourceColor, GfxTexture2DResource *&dstResourceDepth )
{
	return true;
}


bool GfxCore::rb_render_target_2d_copy_part(
	GfxRenderTarget2DResource *&srcResource,
	GfxTexture2DResource *&srcResourceColor, GfxTexture2DResource *&srcResourceDepth,
	GfxRenderTarget2DResource *&dstResource,
	GfxTexture2DResource *&dstResourceColor, GfxTexture2DResource *&dstResourceDepth,
	u16 srcX, u16 srcY, u16 dstX, u16 dstY, u16 width, u16 height )
{
	return true;
}


bool GfxCore::rb_render_target_2d_buffer_read_color( GfxRenderTarget2DResource *&resource,
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


bool GfxCore::rb_render_target_2d_buffered_write_color( GfxRenderTarget2DResource *&resource,
	GfxTexture2DResource *&resourceColor,
	const void *const buffer, const u32 size )
{
	return true;
}


bool GfxCore::rb_render_target_2d_bind( const GfxRenderTarget2DResource *const &resource, int slot )
{
	return true;
}


bool GfxCore::rb_render_target_2d_release( const int slot )
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GfxCore::rb_shader_init( GfxShaderResource *&resource, const u32 shaderID, const struct DiskShader &diskShader )
{
	return true;
}


bool GfxCore::rb_shader_free( GfxShaderResource *&resource )
{
	return true;
}


bool GfxCore::rb_shader_bind( GfxShaderResource *&resource )
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////