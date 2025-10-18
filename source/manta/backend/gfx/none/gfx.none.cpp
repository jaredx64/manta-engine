#include <manta/gfx.hpp>
#include <config.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Resources

struct GfxShaderResource : public GfxResource
{
	static void release( GfxShaderResource *&resource );
};


struct GfxVertexBufferResource : public GfxResource
{
	static void release( GfxVertexBufferResource *&resource );
};


struct GfxInstanceBufferResource : public GfxResource
{
	static void release( GfxInstanceBufferResource *&resource );
};


struct GfxIndexBufferResource : public GfxResource
{
	static void release( GfxIndexBufferResource *&resource );
};


struct GfxUniformBufferResource : public GfxResource
{
	static void release( GfxUniformBufferResource *&resource );
};


struct GfxTextureResource : public GfxResource
{
	static void release( GfxTextureResource *&resource );
};


struct GfxRenderTargetResource : public GfxResource
{
	static void release( GfxRenderTargetResource *&resource );
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// GFX System

bool CoreGfx::api_init()
{
	return true;
}


bool CoreGfx::api_free()
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Frame

void CoreGfx::api_frame_begin()
{
}


void CoreGfx::api_frame_end()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Clear

void CoreGfx::api_clear_color( const Color color )
{
}


void CoreGfx::api_clear_depth( const float depth )
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Render State

bool CoreGfx::api_swapchain_init( const u16 width, const u16 height, const float dpi )
{
	return true;
}


bool CoreGfx::api_swapchain_free()
{
	return true;
}


bool CoreGfx::api_swapchain_set_size( const u16 width, const u16 height, const float dpi )
{
	return true;
}


bool CoreGfx::api_viewport_init( const u16 width, const u16 height, const float dpi )
{
	return true;
}


bool CoreGfx::api_viewport_free()
{
	return true;
}


bool CoreGfx::api_viewport_set_size( const u16 width, const u16 height, const float dpi )
{
	return true;
}


bool CoreGfx::api_scissor_set_state( const GfxStateScissor &state )
{
	return true;
}


bool CoreGfx::api_sampler_set_state( const GfxStateSampler &state )
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Pipeline State

bool CoreGfx::api_set_raster( const GfxPipelineDescription &description )
{
	return true;
}


bool CoreGfx::api_set_blend( const GfxPipelineDescription &description )
{
	return true;
}


bool CoreGfx::api_set_depth( const GfxPipelineDescription &description )
{
	return true;
}


bool CoreGfx::api_set_stencil( const GfxPipelineDescription &description )
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Shader

void GfxShaderResource::release( GfxShaderResource *&resource )
{
}


bool CoreGfx::api_shader_init( GfxShaderResource *&resource, const u32 shaderID, const struct ShaderEntry &shaderEntry )
{
	return true;
}


bool CoreGfx::api_shader_free( GfxShaderResource *&resource )
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Vertex Buffer

void GfxVertexBufferResource::release( GfxVertexBufferResource *&resource )
{
}


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


bool CoreGfx::api_vertex_buffer_write( GfxVertexBufferResource *const resource,
	const void *const data, const u32 size )
{
	return true;
}


u32 CoreGfx::api_vertex_buffer_current( const GfxVertexBufferResource *const resource )
{
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Instance Buffer

void GfxInstanceBufferResource::release( GfxInstanceBufferResource *&resource )
{
}


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
// Index Buffer

void GfxIndexBufferResource::release( GfxIndexBufferResource *&resource )
{
}


bool CoreGfx::api_index_buffer_init( GfxIndexBufferResource *&resource,
	void *data, const u32 size, const double indicesToVerticesRatio,
	const GfxIndexBufferFormat format, const GfxCPUAccessMode accessMode )
{
	return true;
}


bool CoreGfx::api_index_buffer_free( GfxIndexBufferResource *&resource )
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Uniform Buffer

void GfxUniformBufferResource::release( GfxUniformBufferResource *&resource )
{
}


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

// TODO: GfxConstantBuffer
// TODO: GfxMutableBuffer

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Texture

void GfxTextureResource::release( GfxTextureResource *&resource )
{
}


bool CoreGfx::api_texture_init( GfxTextureResource *&resource, void *pixels,
	const u16 width, const u16 height, const u16 levels, const GfxColorFormat &format )
{
	return true;
}


bool CoreGfx::api_texture_free( GfxTextureResource *&resource )
{
	return true;
}


bool CoreGfx::api_texture_bind( GfxTextureResource *const resource, const int slot )
{
	return true;
}


bool CoreGfx::api_texture_release( GfxTextureResource *const resource, const int slot )
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Render Target

void GfxRenderTargetResource::release( GfxRenderTargetResource *&resource )
{
}


bool CoreGfx::api_render_target_init( GfxRenderTargetResource *&resource,
	GfxTextureResource *&resourceColor, GfxTextureResource *&resourceDepth,
	const u16 width, const u16 height,
	const GfxRenderTargetDescription &desc )
{
	return true;
}


bool CoreGfx::api_render_target_free( GfxRenderTargetResource *&resource,
	GfxTextureResource *&resourceColor,
	GfxTextureResource *&resourceDepth )
{
	return true;
}


bool CoreGfx::api_render_target_copy(
	GfxRenderTargetResource *&srcResource,
	GfxTextureResource *&srcResourceColor, GfxTextureResource *&srcResourceDepth,
	GfxRenderTargetResource *&dstResource,
	GfxTextureResource *&dstResourceColor, GfxTextureResource *&dstResourceDepth )
{
	return true;
}


bool CoreGfx::api_render_target_copy_part(
	GfxRenderTargetResource *&srcResource,
	GfxTextureResource *&srcResourceColor, GfxTextureResource *&srcResourceDepth,
	GfxRenderTargetResource *&dstResource,
	GfxTextureResource *&dstResourceColor, GfxTextureResource *&dstResourceDepth,
	u16 srcX, u16 srcY, u16 dstX, u16 dstY, u16 width, u16 height )
{
	return true;
}


bool CoreGfx::api_render_target_buffer_read_color( GfxRenderTargetResource *&resource,
		GfxTextureResource *&resourceColor,
		void *buffer, const u32 size )
{
	return true;
}


bool api_render_target_buffer_read_depth( GfxRenderTargetResource *&resource,
	GfxTextureResource *&resourceDepth,
	void *buffer, const u32 size )
{
	return true;
}


bool CoreGfx::api_render_target_buffer_write_color( GfxRenderTargetResource *&resource,
	GfxTextureResource *&resourceColor,
	const void *const buffer, const u32 size )
{
	return true;
}


bool CoreGfx::api_render_target_buffer_write_depth( GfxRenderTargetResource *&resource,
	GfxTextureResource *&resourceDepth,
	const void *const buffer, const u32 size )
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Draw

bool CoreGfx::api_draw(
	const GfxVertexBufferResource *const resourceVertex, const u32 vertexCount,
	const GfxInstanceBufferResource *const resourceInstance, const u32 instanceCount,
	const GfxIndexBufferResource *const resourceIndex,
	const GfxPrimitiveType type )
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CoreGfx::api_dispatch( const u32 x, const u32 y, const u32 z )
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Render Command

void CoreGfx::api_render_command_execute( const GfxRenderCommand &command )
{
}


void CoreGfx::api_render_pass_begin( const GfxRenderPass &pass )
{
}


void CoreGfx::api_render_pass_end( const GfxRenderPass &pass )
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////