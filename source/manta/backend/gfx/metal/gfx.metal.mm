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

#define GRAPHICS_API_NAME "Metal"

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

struct GfxShaderResource : public GfxResource
{
	static void release( GfxShaderResource *&resource );
	// ...
};


struct GfxVertexBufferResource : public GfxResource
{
	static void release( GfxVertexBufferResource *&resource );
	u32 vertexFormat = 0;
	bool mapped = false;
	// ...
};


struct GfxInstanceBufferResource : public GfxResource
{
	static void release( GfxInstanceBufferResource *&resource );
	u32 instanceFormat = 0;
	bool mapped = false;
	// ...
};


struct GfxIndexBufferResource : public GfxResource
{
	static void release( GfxIndexBufferResource *&resource );
	// ...
};


struct GfxUniformBufferResource : public GfxResource
{
	static void release( GfxUniformBufferResource *&resource );
	bool mapped = false;
	// ...
};


struct GfxTexture2DResource : public GfxResource
{
	static void release( GfxTexture2DResource *&resource );
	// ...
};


struct GfxRenderTarget2DResource : public GfxResource
{
	static void release( GfxRenderTarget2DResource *&resource );
	// ...
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static GfxResourceFactory<GfxShaderResource, GFX_RESOURCE_COUNT_SHADER> shaderResources;
static GfxResourceFactory<GfxVertexBufferResource, GFX_RESOURCE_COUNT_VERTEX_BUFFER> vertexBufferResources;
static GfxResourceFactory<GfxInstanceBufferResource, GFX_RESOURCE_COUNT_INSTANCE_BUFFER> instanceBufferResources;
static GfxResourceFactory<GfxIndexBufferResource, GFX_RESOURCE_COUNT_INDEX_BUFFER> indexBufferResources;
static GfxResourceFactory<GfxUniformBufferResource, GFX_RESOURCE_COUNT_UNIFORM_BUFFER> uniformBufferResources;
static GfxResourceFactory<GfxTexture2DResource, GFX_RESOURCE_COUNT_TEXTURE_2D> texture2DResources;
static GfxResourceFactory<GfxRenderTarget2DResource, GFX_RESOURCE_COUNT_RENDER_TARGET_2D> renderTarget2DResources;


static bool resources_init()
{
	shaderResources.init();
	vertexBufferResources.init();
	instanceBufferResources.init();
	indexBufferResources.init();
	uniformBufferResources.init();
	texture2DResources.init();
	renderTarget2DResources.init();


	return true;
}


static bool resources_free()
{
	renderTarget2DResources.free();
	texture2DResources.free();
	uniformBufferResources.free();
	indexBufferResources.free();
	instanceBufferResources.free();
	vertexBufferResources.free();
	shaderResources.free();

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CoreGfx::api_init()
{
	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING
	return true;
}


bool CoreGfx::api_free()
{
	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING
	return true;
}


void CoreGfx::api_frame_begin()
{
	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING
}


void CoreGfx::api_frame_end()
{
	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING
}


void CoreGfx::api_clear_color( const Color color )
{
	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING
}


void CoreGfx::api_clear_depth( const float depth )
{
	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CoreGfx::api_swapchain_init( const u16 width, const u16 height, const bool fullscreen )
{
	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING
	return true;
}


bool CoreGfx::api_swapchain_free()
{
	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING
	return true;
}


bool CoreGfx::api_swapchain_resize( u16 width, u16 height, bool fullscreen )
{
	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CoreGfx::api_viewport_init( const u16 width, const u16 height, const bool fullscreen )
{
	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING
	return true;
}


bool CoreGfx::api_viewport_free()
{
	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING
	return true;
}


bool CoreGfx::api_viewport_resize( const u16 width, const u16 height, const bool fullscreen )
{
	// Pass through to api_viewport_init (TODO: Do something else?)
	return api_viewport_init( width, height, fullscreen );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CoreGfx::api_set_raster_state( const GfxRasterState &state )
{
	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING
	return true;
}


bool CoreGfx::api_set_sampler_state( const GfxSamplerState &state )
{
	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING
	return true;
}


bool CoreGfx::api_set_blend_state( const GfxBlendState &state )
{
	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING
	return true;
}


bool CoreGfx::api_set_depth_state( const GfxDepthState &state )
{
	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GfxShaderResource::release( GfxShaderResource *&resource )
{
	if( resource == nullptr || resource->id == GFX_RESOURCE_ID_NULL ) { return; }

	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING

	shaderResources.remove( resource->id );
	resource = nullptr;
}

bool CoreGfx::api_shader_init( GfxShaderResource *&resource, const u32 shaderID, const struct ShaderEntry &shaderEntry )
{
	// Register Shader
	Assert( resource == nullptr );
	Assert( shaderID < CoreGfx::shaderCount );

	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING
	return true;
}


bool CoreGfx::api_shader_free( GfxShaderResource *&resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING
	return true;
}


bool CoreGfx::api_shader_bind( GfxShaderResource *&resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING
	return true;
}


bool CoreGfx::api_shader_dispatch( GfxShaderResource *&resource, const u32 x, const u32 y, const u32 z )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( resource->cs != nullptr );

	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GfxVertexBufferResource::release( GfxVertexBufferResource *&resource )
{
	if( resource == nullptr || resource->id == GFX_RESOURCE_ID_NULL ) { return; }

	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING

	vertexBufferResources.remove( resource->id );
	resource = nullptr;
}


bool CoreGfx::api_vertex_buffer_init_dynamic( GfxVertexBufferResource *&resource, const u32 vertexFormatID,
	const GfxCPUAccessMode accessMode, const u32 size, const u32 stride )
{
	Assert( resource == nullptr );
	Assert( accessMode == GfxCPUAccessMode_WRITE_DISCARD || accessMode == GfxCPUAccessMode_WRITE_NO_OVERWRITE );

	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING
	return true;
}


bool CoreGfx::api_vertex_buffer_init_static( GfxVertexBufferResource *&resource, const u32 vertexFormatID,
	const GfxCPUAccessMode accessMode, const void *const data,
	const u32 size, const u32 stride )
{
	Assert( resource == nullptr );

	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING
	return true;
}


bool CoreGfx::api_vertex_buffer_free( GfxVertexBufferResource *&resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING
	return true;
}


void CoreGfx::api_vertex_buffer_write_begin( GfxVertexBufferResource *const resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING
}


void CoreGfx::api_vertex_buffer_write_end( GfxVertexBufferResource *const resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING
}


bool CoreGfx::api_vertex_buffer_write( GfxVertexBufferResource *const resource,
	const void *const data, const u32 size )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING
	return true;
}


u32 CoreGfx::api_vertex_buffer_current( const GfxVertexBufferResource *const resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	return resource->current;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GfxInstanceBufferResource::release( GfxInstanceBufferResource *&resource )
{
	if( resource == nullptr || resource->id == GFX_RESOURCE_ID_NULL ) { return; }

	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING

	instanceBufferResources.remove( resource->id );
	resource = nullptr;
}


bool CoreGfx::api_instance_buffer_init_dynamic( GfxInstanceBufferResource *&resource, const u32 instanceFormatID,
	const GfxCPUAccessMode accessMode, const u32 size, const u32 stride )
{
	Assert( resource == nullptr );
	Assert( accessMode == GfxCPUAccessMode_WRITE_DISCARD || accessMode == GfxCPUAccessMode_WRITE_NO_OVERWRITE );

	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING
	return true;
}


bool CoreGfx::api_instance_buffer_init_static( GfxInstanceBufferResource *&resource, const u32 instanceFormatID,
	const GfxCPUAccessMode accessMode, const void *const data,
	const u32 size, const u32 stride )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING
	GRAPHICS_API_IMPLEMENTATION_WARNING
	return true;
}


bool CoreGfx::api_instance_buffer_free( GfxInstanceBufferResource *&resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING
	return true;
}


void CoreGfx::api_instance_buffer_write_begin( GfxInstanceBufferResource *const resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	if( resource->mapped == true ) { return; }

	const GfxCPUAccessMode accessMode = resource->accessMode;
	Assert( accessMode == GfxCPUAccessMode_WRITE ||
			accessMode == GfxCPUAccessMode_WRITE_DISCARD ||
			accessMode == GfxCPUAccessMode_WRITE_NO_OVERWRITE )

 	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING
}


void CoreGfx::api_instance_buffer_write_end( GfxInstanceBufferResource *const resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	if( resource->mapped == false ) { return; }

	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING
}


bool CoreGfx::api_instance_buffer_write( GfxInstanceBufferResource *const resource,
	const void *const data, const u32 size )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( resource->mapped );

	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING
	return true;
}


u32 CoreGfx::api_instance_buffer_current( const GfxInstanceBufferResource *const resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	return resource->current;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GfxIndexBufferResource::release( GfxIndexBufferResource *&resource )
{
	if( resource == nullptr || resource->id == GFX_RESOURCE_ID_NULL ) { return; }

	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING

	indexBufferResources.remove( resource->id );
	resource = nullptr;
}


bool CoreGfx::api_index_buffer_init( GfxIndexBufferResource *&resource,
	void *data, const u32 size, const double indicesToVerticesRatio,
	const GfxIndexBufferFormat format, const GfxCPUAccessMode accessMode )
{
	Assert( resource == nullptr );
	Assert( format != GfxIndexBufferFormat_NONE );
	Assert( format < GFXINDEXBUFFERFORMAT_COUNT );

	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING
	return true;
}


bool CoreGfx::api_index_buffer_free( GfxIndexBufferResource *&resource )
{
	Assert( resource != nullptr );

	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GfxUniformBufferResource::release( GfxUniformBufferResource *&resource )
{
	if( resource == nullptr || resource->id == GFX_RESOURCE_ID_NULL ) { return; }

	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING

	uniformBufferResources.remove( resource->id );
	resource = nullptr;
}


bool CoreGfx::api_uniform_buffer_init( GfxUniformBufferResource *&resource, const char *name,
	const int index, const u32 size )
{
	Assert( resource == nullptr );

	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING
	return true;
}


bool CoreGfx::api_uniform_buffer_free( GfxUniformBufferResource *&resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING
	return true;
}


void CoreGfx::api_uniform_buffer_write_begin( GfxUniformBufferResource *const resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	if( resource->mapped == true ) { return; }

 	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING
}


void CoreGfx::api_uniform_buffer_write_end( GfxUniformBufferResource *const resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	if( resource->mapped == false ) { return; }

	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING
}


bool CoreGfx::api_uniform_buffer_write( GfxUniformBufferResource *const resource, const void *data )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( resource->mapped );
	Assert( resource->data != nullptr );

	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING
	return true;
}


bool CoreGfx::api_uniform_buffer_bind_vertex( GfxUniformBufferResource *const resource, const int slot )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( slot >= 0 );

	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING
	return true;
}


bool CoreGfx::api_uniform_buffer_bind_fragment( GfxUniformBufferResource *const resource, const int slot )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( slot >= 0 );

	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING
	return true;
}


bool CoreGfx::api_uniform_buffer_bind_compute( GfxUniformBufferResource *const resource, const int slot )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( slot >= 0 );

	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// TODO: GfxConstantBuffer
// TODO: GfxMutableBuffer

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// TODO: GfxTexture1D

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GfxTexture2DResource::release( GfxTexture2DResource *&resource )
{
	if( resource == nullptr || resource->id == GFX_RESOURCE_ID_NULL ) { return; }

	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING

	texture2DResources.remove( resource->id );
	resource = nullptr;
}


bool CoreGfx::api_texture_2d_init( GfxTexture2DResource *&resource, void *pixels,
	const u16 width, const u16 height, const u16 levels, const GfxColorFormat &format )
{
	Assert( resource == nullptr );

	const usize pixelSizeBytes = CoreGfx::colorFormatPixelSizeBytes[format];
	const byte *source = reinterpret_cast<const byte *>( pixels );

	ErrorReturnIf( Gfx::mip_level_count_2d( width, height ) < levels, false,
		"%s: requested mip levels is more than texture size supports (attempted: %u)", levels )
	ErrorReturnIf( levels >= GFX_MIP_DEPTH_MAX, false,
		"%s: exceeded max mip level count (has: %u, max: %u)", levels, GFX_MIP_DEPTH_MAX );

	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING
	return true;
}


bool CoreGfx::api_texture_2d_free( GfxTexture2DResource *&resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING
	return true;
}


bool CoreGfx::api_texture_2d_bind( GfxTexture2DResource *const resource, const int slot )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( slot >= 0 && slot < GFX_TEXTURE_SLOT_COUNT );

	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING
	return true;
}


bool CoreGfx::api_texture_2d_release( GfxTexture2DResource *const resource, const int slot )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( slot >= 0 && slot < GFX_TEXTURE_SLOT_COUNT );
	Assert( textureSlots[slot] != nullptr );

	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// TODO: GfxTexture2DArray
// TODO: GfxTexture3D
// TODO: GfxTexture3DArray
// TODO: GfxTextureCube
// TODO: GfxTextureCubeArray

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GfxRenderTarget2DResource::release( GfxRenderTarget2DResource *&resource )
{
	if( resource == nullptr || resource->id == GFX_RESOURCE_ID_NULL ) { return; }

	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING

	renderTarget2DResources.remove( resource->id );
	resource = nullptr;
}


bool CoreGfx::api_render_target_2d_init( GfxRenderTarget2DResource *&resource,
	GfxTexture2DResource *&resourceColor, GfxTexture2DResource *&resourceDepth,
	const u16 width, const u16 height,
	const GfxRenderTargetDescription &desc )
{
	Assert( resource == nullptr );

	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING
	return true;
}


bool CoreGfx::api_render_target_2d_free( GfxRenderTarget2DResource *&resource,
	GfxTexture2DResource *&resourceColor,
	GfxTexture2DResource *&resourceDepth )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING
	return true;
}


bool CoreGfx::api_render_target_2d_copy(
	GfxRenderTarget2DResource *&srcResource,
	GfxTexture2DResource *&srcResourceColor, GfxTexture2DResource *&srcResourceDepth,
	GfxRenderTarget2DResource *&dstResource,
	GfxTexture2DResource *&dstResourceColor, GfxTexture2DResource *&dstResourceDepth )
{
	// Validate resources
	if( srcResource == nullptr || dstResource == nullptr ) { return false; }

	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING
	return true;
}


bool CoreGfx::api_render_target_2d_copy_part(
	GfxRenderTarget2DResource *&srcResource,
	GfxTexture2DResource *&srcResourceColor, GfxTexture2DResource *&srcResourceDepth,
	GfxRenderTarget2DResource *&dstResource,
	GfxTexture2DResource *&dstResourceColor, GfxTexture2DResource *&dstResourceDepth,
	u16 srcX, u16 srcY, u16 dstX, u16 dstY, u16 width, u16 height )
{
	// Validate resources
	if( srcResource == nullptr || dstResource == nullptr ) { return false; }

	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING
	return true;
}


bool CoreGfx::api_render_target_2d_buffer_read_color( GfxRenderTarget2DResource *&resource,
		GfxTexture2DResource *&resourceColor,
		void *buffer, const u32 size )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( resourceColor != nullptr && resourceColor->id != GFX_RESOURCE_ID_NULL );

	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING
	return true;
}


bool api_render_target_2d_buffer_read_depth( GfxRenderTarget2DResource *&resource,
	GfxTexture2DResource *&resourceDepth,
	void *buffer, const u32 size )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING
	return true;
}


bool CoreGfx::api_render_target_2d_buffer_write_color( GfxRenderTarget2DResource *&resource,
	GfxTexture2DResource *&resourceColor,
	const void *const buffer, const u32 size )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( resourceColor != nullptr && resourceColor->id != GFX_RESOURCE_ID_NULL );

	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING
	return true;
}


bool CoreGfx::api_render_target_2d_buffer_write_depth( GfxRenderTarget2DResource *&resource,
	GfxTexture2DResource *&resourceDepth,
	const void *const buffer, const u32 size )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING
	return true;
}


bool CoreGfx::api_render_target_2d_bind( GfxRenderTarget2DResource *const resource, int slot )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( slot >= 0 && slot < GFX_RENDER_TARGET_SLOT_COUNT );

	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING
	return true;
}


bool CoreGfx::api_render_target_2d_release( GfxRenderTarget2DResource *const resource, const int slot )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( slot >= 0 && slot < GFX_RENDER_TARGET_SLOT_COUNT );

	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CoreGfx::api_draw(
		const GfxVertexBufferResource *const resourceVertex, const u32 vertexCount,
		const GfxInstanceBufferResource *const resourceInstance, const u32 instanceCount,
		const GfxIndexBufferResource *const resourceIndex,
		const GfxPrimitiveType type )
{
	Assert( resourceVertex == nullptr || resourceVertex->id != GFX_RESOURCE_ID_NULL );
	Assert( resourceInstance == nullptr || resourceInstance->id != GFX_RESOURCE_ID_NULL );
	Assert( resourceIndex == nullptr || resourceIndex->id != GFX_RESOURCE_ID_NULL );

	AssertMsg( resourceVertex == nullptr ||
		CoreGfx::shaderEntries[Gfx::state().shader.shaderID].vertexFormat == resourceVertex->vertexFormat,
		"Attempting to draw a vertex buffer with a shader of a different vertex format!" );
	AssertMsg( resourceInstance == nullptr ||
		CoreGfx::shaderEntries[Gfx::state().shader.shaderID].instanceFormat == resourceInstance->instanceFormat,
		"Attempting to draw a vertex buffer with a shader of a different instance format!" );

	ErrorIf( resourceVertex != nullptr && resourceVertex->mapped,
		"Attempting to draw vertex buffer that is mapped! (resource: %u)", resourceVertex->id );
	ErrorIf( resourceInstance != nullptr && resourceInstance->mapped,
		"Attempting to draw instance buffer that is mapped! (resource: %u)", resourceInstance->id );

	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////