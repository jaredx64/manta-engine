#include <manta/gfx.hpp>
#include <gfx.api.generated.hpp>

#include <config.hpp>

#include <core/buffer.hpp>
#include <core/memory.hpp>
#include <core/hashmap.hpp>
#include <core/math.hpp>

#include <manta/window.hpp>
#include <manta/backend/gfx/gfxfactory.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#import <QuartzCore/QuartzCore.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static id<MTLDevice> device;

static id<MTLCommandQueue> commandQueue;
static id<MTLRenderPipelineState> piplineState;
static id<MTLBuffer> vertices;
static id<MTLTexture> depthTarget;

static MTLRenderPassDescriptor *drawableRendererDescriptor;
static NSUInteger frameNum;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if 1
#define DECL_ZERO(type, name) type name; memory_set( &name, 0, sizeof( type ) )
#else
#define DECL_ZERO(type, name) type name
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct GfxVertexBufferResource : public GfxResource
{
	// ...
};


struct GfxIndexBufferResource : public GfxResource
{
	// ...
};


struct GfxUniformBufferResource : public GfxResource
{
	// ...
};


struct GfxTexture2DResource : public GfxResource
{
	// ...
};


struct GfxRenderTarget2DResource : public GfxResource
{
	// ...
};


struct GfxShaderResource : public GfxResource
{
	// ...
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static GfxResourceFactory<GfxVertexBufferResource, GFX_RESOURCE_COUNT_VERTEX_BUFFER> vertexBufferResources;
static GfxResourceFactory<GfxIndexBufferResource, GFX_RESOURCE_COUNT_INDEX_BUFFER> indexBufferResources;
static GfxResourceFactory<GfxUniformBufferResource, GFX_RESOURCE_COUNT_CONSTANT_BUFFER> uniformBufferResources;
static GfxResourceFactory<GfxShaderResource, GFX_RESOURCE_COUNT_SHADER> shaderResources;
static GfxResourceFactory<GfxTexture2DResource, GFX_RESOURCE_COUNT_TEXTURE_2D> texture2DResources;
static GfxResourceFactory<GfxRenderTarget2DResource, GFX_RESOURCE_COUNT_RENDER_TARGET_2D> renderTarget2DResources;


static bool resources_init()
{
	vertexBufferResources.init();
	indexBufferResources.init();
	uniformBufferResources.init();
	texture2DResources.init();
	renderTarget2DResources.init();
	shaderResources.init();

	// Success
	return true;
}


static bool resources_free()
{
	vertexBufferResources.free();
	indexBufferResources.free();
	uniformBufferResources.free();
	texture2DResources.free();
	renderTarget2DResources.free();
	shaderResources.free();

	// Success
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GfxCore::rb_init()
{
	// TODO: ...

	// Success
	return true;
}


bool GfxCore::rb_free()
{
	// TODO: ...

	// Success
	return true;
}


void GfxCore::rb_frame_begin()
{
	// TODO: ...
}


void GfxCore::rb_frame_end()
{
	// TODO: ...
}


void GfxCore::rb_clear_color( const Color color )
{
	// TODO: ...
}


void GfxCore::rb_clear_depth( const float depth )
{
	// TODO: ...
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GfxCore::rb_swapchain_init( const u16 width, const u16 height, const bool fullscreen )
{
	// TODO: ...

	// Success
	return true;
}


bool GfxCore::rb_swapchain_free()
{
	// TODO: ...

	return true;
}


bool GfxCore::rb_swapchain_resize( u16 width, u16 height, bool fullscreen )
{
	// TODO: ...

	// Success
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GfxCore::rb_viewport_init( const u16 width, const u16 height, const bool fullscreen )
{
	// TODO: ...

	return true;
}


bool GfxCore::rb_viewport_free()
{
	// TODO
	return true;
}


bool GfxCore::rb_viewport_resize( const u16 width, const u16 height, const bool fullscreen )
{
	// Pass through to rb_viewport_init (TODO: Do something else?)
	return rb_viewport_init( width, height, fullscreen );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GfxCore::rb_set_raster_state( const GfxRasterState &state )
{
	// TODO: ...

	// Success
	return true;
}


bool GfxCore::rb_set_sampler_state( const GfxSamplerState &state )
{
	// TODO: ...

	return true;
}


bool GfxCore::rb_set_blend_state( const GfxBlendState &state )
{
	// TODO: ...

	return true;
}


bool GfxCore::rb_set_depth_state( const GfxDepthState &state )
{
	// TODO: ...

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GfxCore::rb_index_buffer_init( GfxIndexBufferResource *&resource,
	void *data, const u32 size, const double indToVertRatio,
	const GfxIndexBufferFormat format, const GfxCPUAccessMode accessMode )
{
	// TODO: ...

	// Success
	return true;
}


bool GfxCore::rb_index_buffer_free( GfxIndexBufferResource *&resource )
{
	// TODO: ...

	// Success
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GfxCore::rb_vertex_buffer_init_dynamic( GfxVertexBufferResource *&resource, const u32 vertexFormatID,
                                             const GfxCPUAccessMode accessMode, const u32 size, const u32 stride )
{
		// TODO: ...

	// Success
	return true;
}


bool GfxCore::rb_vertex_buffer_init_static( GfxVertexBufferResource *&resource, const u32 vertexFormatID,
	const GfxCPUAccessMode accessMode, const void *const data,
	const u32 size, const u32 stride )
{
	// TODO: ...

	// Success
	return true;
}


bool GfxCore::rb_vertex_buffer_free( GfxVertexBufferResource *&resource )
{
	// TODO: ...

	// Success
	return true;
}


bool GfxCore::rb_vertex_buffer_draw( GfxVertexBufferResource *&resource )
{
	// TODO: ...

	// Success
	return true;
}


bool GfxCore::rb_vertex_buffer_draw_indexed( GfxVertexBufferResource *&resource, GfxIndexBufferResource *&resourceIndexBuffer )
{
	// TODO: ...

	// Success
	return true;
}


void GfxCore::rb_vertex_buffered_write_begin( GfxVertexBufferResource *&resource )
{
	// TODO: ...
}


void GfxCore::rb_vertex_buffered_write_end( GfxVertexBufferResource *&resource )
{
	// TODO: ...
}


bool GfxCore::rb_vertex_buffered_write( GfxVertexBufferResource *&resource, const void *const data, const u32 size )
{
	// TODO: ...

	// Success
	return true;
}


u32 GfxCore::rb_vertex_buffer_current( GfxVertexBufferResource *&resource )
{
	// TODO: ...

	return 0;//resource->current;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GfxCore::rb_uniform_buffer_init( GfxUniformBufferResource *&resource, const char *name,
	const int index, const u32 size )
{
	// TODO: ...

	// Success
	return true;
}


bool GfxCore::rb_uniform_buffer_free( GfxUniformBufferResource *&resource )
{
	// TODO: ...

	// Success
	return true;
}


void GfxCore::rb_constant_buffered_write_begin( GfxUniformBufferResource *&resource )
{
	// TODO: ...
}


void GfxCore::rb_constant_buffered_write_end( GfxUniformBufferResource *&resource )
{
	// TODO: ...
}


bool GfxCore::rb_constant_buffered_write( GfxUniformBufferResource *&resource, const void *data )
{
	// TODO: ...

	// Success
	return true;
}


bool GfxCore::rb_uniform_buffer_bind_vertex( GfxUniformBufferResource *&resource, const int slot )
{
	// TODO: ...

	// Success
	return true;
}


bool GfxCore::rb_uniform_buffer_bind_fragment( GfxUniformBufferResource *&resource, const int slot )
{
	// TODO: ...

	// Success
	return true;
}


bool GfxCore::rb_uniform_buffer_bind_compute( GfxUniformBufferResource *&resource, const int slot )
{
	// TODO: ...

	// Success
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GfxCore::rb_texture_2d_init( GfxTexture2DResource *&resource, void *pixels,
                                  const u16 width, const u16 height, const u16 levels, const GfxColorFormat &format )
{
	// TODO: ...

	// Success
	return true;
}


bool GfxCore::rb_texture_2d_free( GfxTexture2DResource *&resource )
{
	// TODO: ...

	// Success
	return true;
}


bool GfxCore::rb_texture_2d_bind( const GfxTexture2DResource *const &resource, const int slot )
{
	// TODO: ...

	// Success
	return true;
}


bool GfxCore::rb_texture_2d_release( const GfxTexture2DResource *const &resource, const int slot )
{
	// TODO: ...

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GfxCore::rb_render_target_2d_init( GfxRenderTarget2DResource *&resource,
	GfxTexture2DResource *&resourceColor, GfxTexture2DResource *&resourceDepth,
	const u16 width, const u16 height,
	const GfxRenderTargetDescription &desc )
{
	// TODO: ...

	// Success
	return true;
}


bool GfxCore::rb_render_target_2d_free( GfxRenderTarget2DResource *&resource,
	GfxTexture2DResource *&resourceColor,
	GfxTexture2DResource *&resourceDepth )
{
	// TODO: ...

	// Success
	return true;
}


bool GfxCore::rb_render_target_2d_copy(
	GfxRenderTarget2DResource *&srcResource,
	GfxTexture2DResource *&srcResourceColor, GfxTexture2DResource *&srcResourceDepth,
	GfxRenderTarget2DResource *&dstResource,
	GfxTexture2DResource *&dstResourceColor, GfxTexture2DResource *&dstResourceDepth )
{
	// TODO: ...

	// Success
	return true;
}


bool GfxCore::rb_render_target_2d_copy_part(
	GfxRenderTarget2DResource *&srcResource,
	GfxTexture2DResource *&srcResourceColor, GfxTexture2DResource *&srcResourceDepth,
	GfxRenderTarget2DResource *&dstResource,
	GfxTexture2DResource *&dstResourceColor, GfxTexture2DResource *&dstResourceDepth,
	u16 srcX, u16 srcY, u16 dstX, u16 dstY, u16 width, u16 height )
{
	// TODO: ...

	// Success
	return true;
}


bool GfxCore::rb_render_target_2d_buffer_read_color( GfxRenderTarget2DResource *&resource,
		GfxTexture2DResource *&resourceColor,
		void *buffer, const u32 size )
{
	// TODO: ...

	// Success
	return true;
}


bool rb_render_target_2d_buffer_read_depth( GfxRenderTarget2DResource *&resource,
	GfxTexture2DResource *&resourceDepth,
	void *buffer, const u32 size )
{
	// TODO: ...

	// Success
	return true;
}


bool GfxCore::rb_render_target_2d_buffered_write_color( GfxRenderTarget2DResource *&resource,
	GfxTexture2DResource *&resourceColor,
	const void *const buffer, const u32 size )
{
	// TODO: ...

	// Success
	return true;
}


bool GfxCore::rb_render_target_2d_bind( const GfxRenderTarget2DResource *const &resource, int slot )
{
	// TODO: ...

	// Success
	return true;
}


bool GfxCore::rb_render_target_2d_release( const GfxRenderTarget2DResource *const &resource, const int slot )
{
	// TODO: ...

	// Success
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GfxCore::rb_shader_init( GfxShaderResource *&resource, const u32 shaderID, const struct BinShader &binShader )
{
	// TODO: ...

	// Success
	return true;
}


bool GfxCore::rb_shader_free( GfxShaderResource *&resource )
{
	// TODO: ...

	// Success
	return true;
}


bool GfxCore::rb_shader_bind( GfxShaderResource *&resource )
{
	// TODO: ...

	// Success
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////