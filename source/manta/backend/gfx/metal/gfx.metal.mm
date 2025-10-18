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

#define GRAPHICS_API_NAME "Metal"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if 1
#define DECL_ZERO(type, name) type name; memory_set( &name, 0, sizeof( type ) )
#else
#define DECL_ZERO(type, name) type name
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Metal Backend State

static id<MTLDevice> device;

static id<MTLCommandQueue> commandQueue;
static id<MTLRenderPipelineState> piplineState;
static id<MTLBuffer> vertices;
static id<MTLTexture> depthTarget;

static MTLRenderPassDescriptor *drawableRendererDescriptor;
static NSUInteger frameNum;

static bool isDefaultTargetActive = true;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Metal Enums


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Resources

struct GfxShaderResource : public GfxResource
{
	static void release( GfxShaderResource *&resource );
	// ...
};


struct GfxVertexBufferResource : public GfxResource
{
	static void release( GfxVertexBufferResource *&resource );
	GfxCPUAccessMode accessMode;
	u32 vertexFormat = 0;
	bool mapped = false;
	u32 current = 0;
	// ...
};


struct GfxInstanceBufferResource : public GfxResource
{
	static void release( GfxInstanceBufferResource *&resource );
	GfxCPUAccessMode accessMode;
	u32 instanceFormat = 0;
	bool mapped = false;
	u32 current = 0;
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


struct GfxTextureResource : public GfxResource
{
	static void release( GfxTextureResource *&resource );
	// ...
};


struct GfxRenderTargetResource : public GfxResource
{
	static void release( GfxRenderTargetResource *&resource );
	// ...
};


static GfxResourceFactory<GfxShaderResource, GFX_RESOURCE_COUNT_SHADER> shaderResources;
static GfxResourceFactory<GfxVertexBufferResource, GFX_RESOURCE_COUNT_VERTEX_BUFFER> vertexBufferResources;
static GfxResourceFactory<GfxInstanceBufferResource, GFX_RESOURCE_COUNT_INSTANCE_BUFFER> instanceBufferResources;
static GfxResourceFactory<GfxIndexBufferResource, GFX_RESOURCE_COUNT_INDEX_BUFFER> indexBufferResources;
static GfxResourceFactory<GfxUniformBufferResource, GFX_RESOURCE_COUNT_UNIFORM_BUFFER> uniformBuffers;
static GfxResourceFactory<GfxTextureResource, GFX_RESOURCE_COUNT_TEXTURE> textureResources;
static GfxResourceFactory<GfxRenderTargetResource, GFX_RESOURCE_COUNT_RENDER_TARGET> renderTargetResources;


static bool resources_init()
{
	shaderResources.init();
	vertexBufferResources.init();
	instanceBufferResources.init();
	indexBufferResources.init();
	uniformBuffers.init();
	textureResources.init();
	renderTargetResources.init();


	return true;
}


static bool resources_free()
{
	renderTargetResources.free();
	textureResources.free();
	uniformBuffers.free();
	indexBufferResources.free();
	instanceBufferResources.free();
	vertexBufferResources.free();
	shaderResources.free();

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Metal System


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Metal Render State

static bool bind_shader( GfxShaderResource *const resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	if( resource == CoreGfx::state.pipeline.shader ) { return true; }

	CoreGfx::state.pipeline.shader = resource;
	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this

	return true;
}


static bool apply_pipeline_state_raster( const GfxPipelineDescription &state )
{
	GfxPipelineDescription &stateCurrent = CoreGfx::state.pipeline.description;
	const bool dirty = BITFLAG_IS_SET( CoreGfx::state.dirtyFlags, GfxStateDirtyFlag_RASTER );
	if( !dirty && state.equal_raster( stateCurrent ) ) { return true; }

	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this

	stateCurrent.raster_set( state );
	BITFLAG_UNSET( CoreGfx::state.dirtyFlags, GfxStateDirtyFlag_RASTER );
	return true;
}


static bool apply_pipeline_state_blend( const GfxPipelineDescription &state )
{
	GfxPipelineDescription &stateCurrent = CoreGfx::state.pipeline.description;
	const bool dirty = BITFLAG_IS_SET( CoreGfx::state.dirtyFlags, GfxStateDirtyFlag_BLEND );
	if( !dirty && state.equal_blend( stateCurrent ) ) { return true; }

	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this

	stateCurrent.blend_set( state );
	BITFLAG_UNSET( CoreGfx::state.dirtyFlags, GfxStateDirtyFlag_BLEND );
	return true;
}


static bool apply_pipeline_state_depth( const GfxPipelineDescription &state )
{
	GfxPipelineDescription &stateCurrent = CoreGfx::state.pipeline.description;
	const bool dirty = BITFLAG_IS_SET( CoreGfx::state.dirtyFlags, GfxStateDirtyFlag_DEPTH );
	if( !dirty && state.equal_depth( stateCurrent ) ) { return true; }

	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this

	stateCurrent.depth_set_state( state );
	BITFLAG_UNSET( CoreGfx::state.dirtyFlags, GfxStateDirtyFlag_DEPTH );
	return true;
}


static bool apply_pipeline_state_stencil( const GfxPipelineDescription &state )
{
	GfxPipelineDescription &stateCurrent = CoreGfx::state.pipeline.description;
	const bool dirty = BITFLAG_IS_SET( CoreGfx::state.dirtyFlags, GfxStateDirtyFlag_STENCIL );
	if( !dirty && state.equal_stencil( stateCurrent ) ) { return true; }

	// TODO: Implement this

	stateCurrent.stencil_set_state( state );
	BITFLAG_UNSET( CoreGfx::state.dirtyFlags, GfxStateDirtyFlag_STENCIL );
	return true;
}


static bool apply_state_scissor( const GfxStateScissor &state )
{
	const bool dirty = BITFLAG_IS_SET( CoreGfx::state.dirtyFlags, GfxStateDirtyFlag_SCISSOR );
	if( !dirty && state == CoreGfx::state.scissor ) { return true; }

	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this

	CoreGfx::state.scissor = state;
	BITFLAG_UNSET( CoreGfx::state.dirtyFlags, GfxStateDirtyFlag_SCISSOR );
	return true;
}


static bool bind_targets( GfxRenderTargetResource *const resources[] )
{
	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this
	return true;
}


static void render_targets_reset()
{
	GfxRenderTargetResource *targets[GFX_RENDER_TARGET_SLOT_COUNT] = { nullptr };
	bind_targets( targets );
}


static void render_pass_validate()
{
	if( CoreGfx::state.renderPassActive ) { return; }
	if( isDefaultTargetActive ) { return; }
	render_targets_reset();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CoreGfx::api_init()
{
	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this
	return true;
}


bool CoreGfx::api_free()
{
	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Frame

void CoreGfx::api_frame_begin()
{
	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this
}


void CoreGfx::api_frame_end()
{
	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Clear

void CoreGfx::api_clear_color( const Color color )
{
	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this
}


void CoreGfx::api_clear_depth( const float depth )
{
	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Render State

bool CoreGfx::api_swapchain_init( const u16 width, const u16 height, const float dpi )
{
	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this
	return true;
}


bool CoreGfx::api_swapchain_free()
{
	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this
	return true;
}


bool CoreGfx::api_swapchain_set_size( const u16 width, const u16 height, const float dpi )
{
	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this
	return true;
}


bool CoreGfx::api_viewport_init( const u16 width, const u16 height, const float dpi )
{
	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this
	return true;
}


bool CoreGfx::api_viewport_free()
{
	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this
	return true;
}


bool CoreGfx::api_viewport_set_size( const u16 width, const u16 height, const float dpi )
{
	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this
	return true;
}


bool CoreGfx::api_scissor_set_state( const GfxStateScissor &state )
{
	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this
	return true;
}


bool CoreGfx::api_sampler_set_state( const GfxStateSampler &state )
{
	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CoreGfx::api_set_raster( const GfxPipelineDescription &description )
{
	return apply_pipeline_state_raster( description );
}


bool CoreGfx::api_set_blend( const GfxPipelineDescription &description )
{
	return apply_pipeline_state_blend( description );
}


bool CoreGfx::api_set_depth( const GfxPipelineDescription &description )
{
	return apply_pipeline_state_depth( description );
}


bool CoreGfx::api_set_stencil( const GfxPipelineDescription &description )
{
	return apply_pipeline_state_stencil( description );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Shader

void GfxShaderResource::release( GfxShaderResource *&resource )
{
	if( resource == nullptr || resource->id == GFX_RESOURCE_ID_NULL ) { return; }

	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this

	shaderResources.remove( resource->id );
	resource = nullptr;
}


bool CoreGfx::api_shader_init( GfxShaderResource *&resource, const u32 shaderID,
	const struct ShaderEntry &shaderEntry )
{
	Assert( resource == nullptr );
	Assert( shaderID < CoreGfx::shaderCount );

	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this
	return true;
}


bool CoreGfx::api_shader_free( GfxShaderResource *&resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Vertex Buffer

void GfxVertexBufferResource::release( GfxVertexBufferResource *&resource )
{
	if( resource == nullptr || resource->id == GFX_RESOURCE_ID_NULL ) { return; }

	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this

	vertexBufferResources.remove( resource->id );
	resource = nullptr;
}


bool CoreGfx::api_vertex_buffer_init_dynamic( GfxVertexBufferResource *&resource, const u32 vertexFormatID,
	const GfxCPUAccessMode accessMode, const u32 size, const u32 stride )
{
	Assert( resource == nullptr );
	Assert( accessMode == GfxCPUAccessMode_WRITE_DISCARD || accessMode == GfxCPUAccessMode_WRITE_NO_OVERWRITE );

	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this
	return true;
}


bool CoreGfx::api_vertex_buffer_init_static( GfxVertexBufferResource *&resource, const u32 vertexFormatID,
	const GfxCPUAccessMode accessMode, const void *const data,
	const u32 size, const u32 stride )
{
	Assert( resource == nullptr );

	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this
	return true;
}


bool CoreGfx::api_vertex_buffer_free( GfxVertexBufferResource *&resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this
	return true;
}


void CoreGfx::api_vertex_buffer_write_begin( GfxVertexBufferResource *const resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this
}


void CoreGfx::api_vertex_buffer_write_end( GfxVertexBufferResource *const resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this
}


bool CoreGfx::api_vertex_buffer_write( GfxVertexBufferResource *const resource,
	const void *const data, const u32 size )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this
	return true;
}


u32 CoreGfx::api_vertex_buffer_current( const GfxVertexBufferResource *const resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	return resource->current;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Instance Buffer

void GfxInstanceBufferResource::release( GfxInstanceBufferResource *&resource )
{
	if( resource == nullptr || resource->id == GFX_RESOURCE_ID_NULL ) { return; }

	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this

	instanceBufferResources.remove( resource->id );
	resource = nullptr;
}


bool CoreGfx::api_instance_buffer_init_dynamic( GfxInstanceBufferResource *&resource,
	const u32 instanceFormatID, const GfxCPUAccessMode accessMode, const u32 size, const u32 stride )
{
	Assert( resource == nullptr );
	Assert( accessMode == GfxCPUAccessMode_WRITE_DISCARD || accessMode == GfxCPUAccessMode_WRITE_NO_OVERWRITE );

	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this
	return true;
}


bool CoreGfx::api_instance_buffer_init_static( GfxInstanceBufferResource *&resource,
	const u32 instanceFormatID, const GfxCPUAccessMode accessMode, const void *const data,
	const u32 size, const u32 stride )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING
	return true;
}


bool CoreGfx::api_instance_buffer_free( GfxInstanceBufferResource *&resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this
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

 	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this
}


void CoreGfx::api_instance_buffer_write_end( GfxInstanceBufferResource *const resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	if( resource->mapped == false ) { return; }

	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this
}


bool CoreGfx::api_instance_buffer_write( GfxInstanceBufferResource *const resource,
	const void *const data, const u32 size )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( resource->mapped );

	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this
	return true;
}


u32 CoreGfx::api_instance_buffer_current( const GfxInstanceBufferResource *const resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	return resource->current;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Index Buffer

void GfxIndexBufferResource::release( GfxIndexBufferResource *&resource )
{
	if( resource == nullptr || resource->id == GFX_RESOURCE_ID_NULL ) { return; }

	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this

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

	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this
	return true;
}


bool CoreGfx::api_index_buffer_free( GfxIndexBufferResource *&resource )
{
	Assert( resource != nullptr );

	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Uniform Buffer

void GfxUniformBufferResource::release( GfxUniformBufferResource *&resource )
{
	if( resource == nullptr || resource->id == GFX_RESOURCE_ID_NULL ) { return; }

	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this

	uniformBuffers.remove( resource->id );
	resource = nullptr;
}


bool CoreGfx::api_uniform_buffer_init( GfxUniformBufferResource *&resource, const char *name,
	const int index, const u32 size )
{
	Assert( resource == nullptr );

	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this
	return true;
}


bool CoreGfx::api_uniform_buffer_free( GfxUniformBufferResource *&resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this
	return true;
}


void CoreGfx::api_uniform_buffer_write_begin( GfxUniformBufferResource *const resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	if( resource->mapped == true ) { return; }

 	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this
}


void CoreGfx::api_uniform_buffer_write_end( GfxUniformBufferResource *const resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	if( resource->mapped == false ) { return; }

	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this
}


bool CoreGfx::api_uniform_buffer_write( GfxUniformBufferResource *const resource, const void *data )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( resource->mapped );
	Assert( resource->data != nullptr );

	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this
	return true;
}


bool CoreGfx::api_uniform_buffer_bind_vertex( GfxUniformBufferResource *const resource, const int slot )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( slot >= 0 );

	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this
	return true;
}


bool CoreGfx::api_uniform_buffer_bind_fragment( GfxUniformBufferResource *const resource, const int slot )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( slot >= 0 );

	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this
	return true;
}


bool CoreGfx::api_uniform_buffer_bind_compute( GfxUniformBufferResource *const resource, const int slot )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( slot >= 0 );

	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// TODO: GfxConstantBuffer
// TODO: GfxMutableBuffer

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Texture

void GfxTextureResource::release( GfxTextureResource *&resource )
{
	if( resource == nullptr || resource->id == GFX_RESOURCE_ID_NULL ) { return; }

	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this

	textureResources.remove( resource->id );
	resource = nullptr;
}


bool CoreGfx::api_texture_init( GfxTextureResource *&resource, void *pixels,
	const u16 width, const u16 height, const u16 levels, const GfxColorFormat &format )
{
	Assert( resource == nullptr );

	const usize pixelSizeBytes = CoreGfx::colorFormatPixelSizeBytes[format];
	const byte *source = reinterpret_cast<const byte *>( pixels );

	ErrorReturnIf( Gfx::mip_level_count_2d( width, height ) < levels, false,
		"%s: requested mip levels is more than texture size supports (attempted: %u)", levels )
	ErrorReturnIf( levels >= GFX_MIP_DEPTH_MAX, false,
		"%s: exceeded max mip level count (has: %u, max: %u)", levels, GFX_MIP_DEPTH_MAX );

	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this
	return true;
}


bool CoreGfx::api_texture_free( GfxTextureResource *&resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this
	return true;
}


bool CoreGfx::api_texture_bind( GfxTextureResource *const resource, const int slot )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( slot >= 0 && slot < GFX_TEXTURE_SLOT_COUNT );

	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this
	return true;
}


bool CoreGfx::api_texture_release( GfxTextureResource *const resource, const int slot )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( slot >= 0 && slot < GFX_TEXTURE_SLOT_COUNT );
	Assert( textureSlots[slot] != nullptr );

	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Render Target

void GfxRenderTargetResource::release( GfxRenderTargetResource *&resource )
{
	if( resource == nullptr || resource->id == GFX_RESOURCE_ID_NULL ) { return; }

	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this

	renderTargetResources.remove( resource->id );
	resource = nullptr;
}


bool CoreGfx::api_render_target_init( GfxRenderTargetResource *&resource,
	GfxTextureResource *&resourceColor, GfxTextureResource *&resourceDepth,
	const u16 width, const u16 height,
	const GfxRenderTargetDescription &desc )
{
	Assert( resource == nullptr );

	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this
	return true;
}


bool CoreGfx::api_render_target_free( GfxRenderTargetResource *&resource,
	GfxTextureResource *&resourceColor,
	GfxTextureResource *&resourceDepth )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this
	return true;
}


bool CoreGfx::api_render_target_copy(
	GfxRenderTargetResource *&srcResource,
	GfxTextureResource *&srcResourceColor, GfxTextureResource *&srcResourceDepth,
	GfxRenderTargetResource *&dstResource,
	GfxTextureResource *&dstResourceColor, GfxTextureResource *&dstResourceDepth )
{
	// Validate resources
	if( srcResource == nullptr || dstResource == nullptr ) { return false; }

	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this
	return true;
}


bool CoreGfx::api_render_target_copy_part(
	GfxRenderTargetResource *&srcResource,
	GfxTextureResource *&srcResourceColor, GfxTextureResource *&srcResourceDepth,
	GfxRenderTargetResource *&dstResource,
	GfxTextureResource *&dstResourceColor, GfxTextureResource *&dstResourceDepth,
	u16 srcX, u16 srcY, u16 dstX, u16 dstY, u16 width, u16 height )
{
	// Validate resources
	if( srcResource == nullptr || dstResource == nullptr ) { return false; }

	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this
	return true;
}


bool CoreGfx::api_render_target_buffer_read_color( GfxRenderTargetResource *&resource,
		GfxTextureResource *&resourceColor,
		void *buffer, const u32 size )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( resourceColor != nullptr && resourceColor->id != GFX_RESOURCE_ID_NULL );

	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this
	return true;
}


bool api_render_target_buffer_read_depth( GfxRenderTargetResource *&resource,
	GfxTextureResource *&resourceDepth,
	void *buffer, const u32 size )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this
	return true;
}


bool CoreGfx::api_render_target_buffer_write_color( GfxRenderTargetResource *&resource,
	GfxTextureResource *&resourceColor,
	const void *const buffer, const u32 size )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( resourceColor != nullptr && resourceColor->id != GFX_RESOURCE_ID_NULL );

	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this
	return true;
}


bool CoreGfx::api_render_target_buffer_write_depth( GfxRenderTargetResource *&resource,
	GfxTextureResource *&resourceDepth,
	const void *const buffer, const u32 size )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this
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
	Assert( resourceVertex == nullptr || resourceVertex->id != GFX_RESOURCE_ID_NULL );
	Assert( resourceInstance == nullptr || resourceInstance->id != GFX_RESOURCE_ID_NULL );
	Assert( resourceIndex == nullptr || resourceIndex->id != GFX_RESOURCE_ID_NULL );

	AssertMsg( resourceVertex == nullptr ||
		CoreGfx::shaderEntries[CoreGfx::state..shader.id].vertexFormat == resourceVertex->vertexFormat,
		"Attempting to draw a vertex buffer with a shader of a different vertex format!" );
	AssertMsg( resourceInstance == nullptr ||
		CoreGfx::shaderEntries[CoreGfx::state.shader.id].instanceFormat == resourceInstance->instanceFormat,
		"Attempting to draw a vertex buffer with a shader of a different instance format!" );

	ErrorIf( resourceVertex != nullptr && resourceVertex->mapped,
		"Attempting to draw vertex buffer that is mapped! (resource: %u)", resourceVertex->id );
	ErrorIf( resourceInstance != nullptr && resourceInstance->mapped,
		"Attempting to draw instance buffer that is mapped! (resource: %u)", resourceInstance->id );

	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Dispatch

bool CoreGfx::api_dispatch( const u32 x, const u32 y, const u32 z )
{
	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Render Command

void CoreGfx::api_render_command_execute( const GfxRenderCommand &command )
{
	render_pass_validate();

	bind_shader( command.pipeline.shader );
	apply_pipeline_state_raster( command.pipeline.description );
	apply_pipeline_state_blend( command.pipeline.description );
	apply_pipeline_state_depth( command.pipeline.description );
	apply_pipeline_state_stencil( command.pipeline.description );

	apply_state_scissor( CoreGfx::state.scissor );

	if( !command.workFunctionInvoker ) { return; }
	command.workFunctionInvoker( command.workFunction,
		GfxRenderCommand::renderCommandArgsStack.get( command.workPayloadOffset, command.workPayloadSize ) );
}


void CoreGfx::api_render_pass_begin( const GfxRenderPass &pass )
{
	bind_targets( pass.targets );
}


void CoreGfx::api_render_pass_end( const GfxRenderPass &pass )
{
#if COMPILE_DEBUG
	if( pass.name[0] != '\0' ) { annotation->EndEvent(); }
#endif

	// NOTE: Do nothing here, since subsequent api_render_pass_begin() will rebind targets
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////