#include <manta/gfx.hpp>
#include <gfx.api.generated.hpp>

#include <config.hpp>

#include <core/buffer.hpp>
#include <core/memory.hpp>
#include <core/hashmap.hpp>
#include <core/math.hpp>

#include <manta/thread.hpp>
#include <manta/window.hpp>

#include <manta/backend/gfx/gfxfactory.hpp>
#include <manta/backend/gfx/metal/metal.hpp>
#include <manta/backend/window/cocoa/window.cocoa.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define GRAPHICS_API_NAME "Metal"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static CAMetalLayer *layer = nil;
static id<MTLDevice> device;

static id<CAMetalDrawable> drawable;
static id<MTLTexture> swapchainTextureDepth;
static bool isDefaultTargetActive = true;

static id<MTLCommandQueue> mtlCommandQueue;
static id<MTLCommandBuffer> mtlCommandBuffer;
static id<MTLRenderCommandEncoder> mtlEncoder;

static MTLViewport mtlViewport;
static MTLScissorRect mtlScissorRect;

static MTLRenderPassDescriptor *mtlRenderPassDescriptor = nil;

static MTLVertexDescriptor *mtlVertexDescriptor = nil;

static NSMutableDictionary<NSNumber *, id<MTLSamplerState>> *mtlSamplerCache = nil;
static MTLSamplerDescriptor *mtlSamplerDescriptor = nil;
static id<MTLSamplerState> mtlSamplerState;

static NSMutableDictionary<NSNumber *, id<MTLDepthStencilState>> *mtlDepthStencilCache = nil;
static MTLDepthStencilDescriptor *mtlDepthStencilDescriptor = nil;
static id<MTLDepthStencilState> mtlDepthStencilState;

static NSMutableDictionary<NSNumber *, id<MTLRenderPipelineState>> *mtlPipelineCache = nil;
static MTLRenderPipelineDescriptor *mtlPipelineDescriptor = nil;
static id<MTLRenderPipelineState> mtlPipelineState;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct UniformBufferBinding
{
	GfxResourceID resourceID = GFX_RESOURCE_ID_NULL;
	int slotVertex = -1;
	int slotFragment = -1;
	int slotCompute = -1;
};

static UniformBufferBinding stateBoundUniformBufferResources[GFX_RESOURCE_COUNT_UNIFORM_BUFFER];

static GfxRenderTargetResource *stateBoundTargetResources[GFX_RENDER_TARGET_SLOT_COUNT] = { nullptr };
static u32 stateBoundColorTextures[GFX_RENDER_TARGET_SLOT_COUNT] = { 0U };
static u32 stateBoundDepthTexture = 0U;

static GfxTextureResource *stateBoundTextureResources[GFX_TEXTURE_SLOT_COUNT] = { nullptr };

static u64 commitIDCurrent = 0LLU;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Metal Enums

static const MTLPixelFormat MetalColorFormats[] =
{
	MTLPixelFormatRGBA8Unorm,   // GfxColorFormat_NONE
	MTLPixelFormatRGBA8Unorm,   // GfxColorFormat_R8G8B8A8_FLOAT
	MTLPixelFormatRGBA8Uint,    // GfxColorFormat_R8G8B8A8_UINT
	MTLPixelFormatRGB10A2Unorm, // GfxColorFormat_R10G10B10A2_FLOAT
	MTLPixelFormatR8Uint,       // GfxColorFormat_R8_UINT
	MTLPixelFormatRG8Unorm,     // GfxColorFormat_R8G8
	MTLPixelFormatR16Uint,      // GfxColorFormat_R16_UINT
	MTLPixelFormatR16Float,     // GfxColorFormat_R16_FLOAT
	MTLPixelFormatRG16Uint,     // GfxColorFormat_R16G16
	MTLPixelFormatRG16Float,    // GfxColorFormat_R16G16F_FLOAT
	MTLPixelFormatRGBA16Float,  // GfxColorFormat_R16G16B16A16_FLOAT
	MTLPixelFormatRGBA16Uint,   // GfxColorFormat_R16G16B16A16_UINT
	MTLPixelFormatR32Float,     // GfxColorFormat_R32_FLOAT
	MTLPixelFormatRG32Float,    // GfxColorFormat_R32G32_FLOAT
	MTLPixelFormatRGBA32Float,  // GfxColorFormat_R32G32B32A32_FLOAT
	MTLPixelFormatRGBA32Uint,   // GfxColorFormat_R32G32B32A32_UINT
};
static_assert( ARRAY_LENGTH( MetalColorFormats ) == GFXCOLORFORMAT_COUNT,
	"Missing GfxColorFormat!" );


static const MTLPixelFormat MetalDepthStencilFormats[] =
{
	MTLPixelFormatInvalid,               // GfxDepthFormat_NONE
	MTLPixelFormatDepth16Unorm,          // GfxDepthFormat_R16_FLOAT
	MTLPixelFormatDepth16Unorm,          // GfxDepthFormat_R16_UINT
	MTLPixelFormatDepth32Float,          // GfxDepthFormat_R32_FLOAT
	MTLPixelFormatDepth32Float,          // GfxDepthFormat_R32_UINT
	MTLPixelFormatDepth24Unorm_Stencil8, // GfxDepthFormat_R24_UINT_G8_UINT
};
static_assert( ARRAY_LENGTH( MetalDepthStencilFormats ) == GFXDEPTHFORMAT_COUNT,
	"Missing GfxDepthFormat!" );


static const MTLPrimitiveType MetalPrimitiveTypes[] =
{
	MTLPrimitiveTypeTriangle,      // GfxPrimitiveType_TriangleList
	MTLPrimitiveTypeTriangleStrip, // GfxPrimitiveType_TriangleStrip
	MTLPrimitiveTypeLine,          // GfxPrimitiveType_LineList
	MTLPrimitiveTypeLineStrip,     // GfxPrimitiveType_LineStrip
	MTLPrimitiveTypePoint,         // GfxPrimitiveType_PointList
};
static_assert( ARRAY_LENGTH( MetalPrimitiveTypes ) == GFXPRIMITIVETYPE_COUNT,
	"Missing GfxPrimitiveType!" );


static const MTLIndexType MetalIndexBufferFormats[] =
{
	MTLIndexTypeUInt32, // GfxIndexBufferFormat_NONE
	MTLIndexTypeUInt16, // GfxIndexBufferFormat_U16
	MTLIndexTypeUInt32, // GfxIndexBufferFormat_U32
};
static_assert( ARRAY_LENGTH( MetalIndexBufferFormats ) == GFXINDEXBUFFERFORMAT_COUNT,
	"Missing GfxIndexBufferFormat!" );


static const MTLCullMode MetalCullModes[] =
{
	MTLCullModeNone,  // GfxCullMode_NONE
	MTLCullModeFront, // GfxCullMode_FRONT
	MTLCullModeBack,  // GfxCullMode_BACK
};
static_assert( ARRAY_LENGTH( MetalCullModes ) == GFXRASTERCULLMODE_COUNT,
	"Missing GfxCullMode!" );


static const MTLTriangleFillMode MetalFillModes[] =
{
	MTLTriangleFillModeFill,   // GfxFillMode_SOLID
	MTLTriangleFillModeLines,  // GfxFillMode_WIREFRAME
};
static_assert( ARRAY_LENGTH( MetalFillModes ) == GFXRASTERFILLMODE_COUNT,
	"Missing GfxFillMode!" );


static const MTLBlendFactor MetalBlendFactors[] =
{
	MTLBlendFactorZero,                     // GfxBlendFactor_ZERO
	MTLBlendFactorOne,                      // GfxBlendFactor_ONE
	MTLBlendFactorSourceColor,              // GfxBlendFactor_SRC_COLOR
	MTLBlendFactorOneMinusSourceColor,      // GfxBlendFactor_INV_SRC_COLOR
	MTLBlendFactorSourceAlpha,              // GfxBlendFactor_SRC_ALPHA
	MTLBlendFactorOneMinusSourceAlpha,      // GfxBlendFactor_INV_SRC_ALPHA
	MTLBlendFactorDestinationAlpha,         // GfxBlendFactor_DEST_ALPHA
	MTLBlendFactorOneMinusDestinationAlpha, // GfxBlendFactor_INV_DEST_ALPHA
	MTLBlendFactorDestinationColor,         // GfxBlendFactor_DEST_COLOR
	MTLBlendFactorOneMinusDestinationColor, // GfxBlendFactor_INV_DEST_COLOR
	MTLBlendFactorSourceAlphaSaturated,     // GfxBlendFactor_SRC_ALPHA_SAT
	// Approximations for dual-source
	MTLBlendFactorSourceColor,              // SRC1_COLOR
	MTLBlendFactorOneMinusSourceColor,      // INV_SRC1_COLOR
	MTLBlendFactorSourceAlpha,              // SRC1_ALPHA
	MTLBlendFactorOneMinusSourceAlpha,      // INV_SRC1_ALPHA
};
static_assert( ARRAY_LENGTH( MetalBlendFactors ) == GFXBLENDFACTOR_COUNT,
	"Missing GfxBlendFactor!" );


static const MTLBlendOperation MetalBlendOperations[] =
{
	MTLBlendOperationAdd,      // GfxBlendOperation_ADD
	MTLBlendOperationSubtract, // GfxBlendOperation_SUBTRACT
	MTLBlendOperationMin,      // GfxBlendOperation_MIN
	MTLBlendOperationMax,      // GfxBlendOperation_MAX
};
static_assert( ARRAY_LENGTH( MetalBlendOperations ) == GFXBLENDOPERATION_COUNT,
	"Missing GfxBlendOperation!" );


static const MTLCompareFunction MetalDepthFunctions[] =
{
	MTLCompareFunctionAlways,       // GfxDepthFunction_NONE
	MTLCompareFunctionLess,         // GfxDepthFunction_LESS
	MTLCompareFunctionLessEqual,    // GfxDepthFunction_LESS_EQUALS
	MTLCompareFunctionGreater,      // GfxDepthFunction_GREATER
	MTLCompareFunctionGreaterEqual, // GfxDepthFunction_GREATER_EQUALS
	MTLCompareFunctionEqual,        // GfxDepthFunction_EQUAL
	MTLCompareFunctionNotEqual,     // GfxDepthFunction_NOT_EQUAL
	MTLCompareFunctionAlways,       // GfxDepthFunction_ALWAYS
};
static_assert( ARRAY_LENGTH( MetalDepthFunctions ) == GFXDEPTHFUNCTION_COUNT,
	"Missing GfxDepthFunction!" );


static const MTLCompareFunction MetalStencilCompareFuncs[] =
{
	MTLCompareFunctionAlways,
	MTLCompareFunctionNever,
	MTLCompareFunctionLess,
	MTLCompareFunctionLessEqual,
	MTLCompareFunctionEqual,
	MTLCompareFunctionGreater,
	MTLCompareFunctionGreaterEqual,
	MTLCompareFunctionNotEqual,
	MTLCompareFunctionAlways,
};


static const MTLStencilOperation MetalStencilOps[] =
{
	MTLStencilOperationKeep,
	MTLStencilOperationZero,
	MTLStencilOperationReplace,
	MTLStencilOperationIncrementClamp,
	MTLStencilOperationDecrementClamp,
	MTLStencilOperationInvert,
	MTLStencilOperationIncrementWrap,
	MTLStencilOperationDecrementWrap,
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Resources

static id<MTLFunction> mtlVertexProgram[GFX_RESOURCE_COUNT_SHADER] = { nil };
static id<MTLFunction> mtlFragmentProgram[GFX_RESOURCE_COUNT_SHADER] = { nil };
static id<MTLFunction> mtlComputeProgram[GFX_RESOURCE_COUNT_SHADER] = { nil };

struct GfxShaderResource : public GfxResource
{
	static void release( GfxShaderResource *&resource );
	usize sizeVS = 0LLU;
	usize sizePS = 0LLU;
	usize sizeCS = 0LLU;
	u32 shaderID = 0U;
	u32 vertexFormat = U32_MAX;
	u32 instanceFormat = U32_MAX;
};


static id<MTLBuffer> metalBuffers[GFX_RESOURCE_COUNT_BUFFER] = { nil };

struct GfxBufferResource : public GfxResource
{
	static void release( GfxBufferResource *&resource );
	GfxWriteMode writeMode;
	usize capacity = 0LLU;
	usize writeRangeStart = USIZE_MAX;
	usize writeRangeEnd = 0LLU;
	bool mapped = false;
};


struct GfxBuffer
{
	bool init( usize capacity ) { return CoreGfx::api_buffer_init( resource, capacity ); }
	bool free() { GfxBufferResource::release( resource ); return true; }
	void write_begin() { CoreGfx::api_buffer_write_begin( resource ); }
	void write_end() { CoreGfx::api_buffer_write_end( resource ); }
	void write( const void *data, usize size, usize offset )
	{
		CoreGfx::api_buffer_write( resource, data, size, offset );
	}
	GfxBufferResource *resource = nullptr;
};


struct GfxVertexBufferResource : public GfxResource
{
	static void release( GfxVertexBufferResource *&resource );

	ThreadedWriterSPSC<GfxBuffer> writer;
	GfxBuffer buffers[3] = { };
	GfxWriteMode writeMode;
	bool mapped = false;

	u32 vertexFormat = 0;
	u32 stride = 0; // per-vertex stride in bytes
	u32 size = 0;
	u32 current = 0;
};


struct GfxInstanceBufferResource : public GfxResource
{
	static void release( GfxInstanceBufferResource *&resource );

	ThreadedWriterSPSC<GfxBuffer> writer;
	GfxBuffer buffers[3] = { };
	GfxWriteMode writeMode;
	bool mapped = false;

	u32 instanceFormat = 0;
	u32 stride = 0; // per-instance size in bytes
	u32 size = 0;
	u32 current = 0;
};


static id<MTLBuffer> mtlIndexBuffers[GFX_RESOURCE_COUNT_INDEX_BUFFER] = { nil };

struct GfxIndexBufferResource : public GfxResource
{
	static void release( GfxIndexBufferResource *&resource );

	GfxWriteMode writeMode = GfxWriteMode_NONE;
	GfxIndexBufferFormat format = GfxIndexBufferFormat_U32;
	double indicesToVerticesRatio = 1.0;
	u32 size = 0; // buffer size in bytes
};


struct GfxUniformBufferResource : public GfxResource
{
	static void release( GfxUniformBufferResource *&resource );

	ThreadedWriterSPSC<GfxBuffer> writer;
	GfxBuffer buffers[3] = { };
	bool mapped = false;

	int index = 0;
	NSUInteger size = 0; // buffer size in bytes
	const char *name = "";
};


static id<MTLTexture> mtlTextures[GFX_RESOURCE_COUNT_TEXTURE] = { nil };

struct GfxTextureResource : public GfxResource
{
	static void release( GfxTextureResource *&resource );

	GfxTextureType type;
	GfxColorFormat colorFormat;
	u16 width = 0;
	u16 height = 0;
	u16 depth = 0;
	u16 levels = 0;
	usize size = 0;
};


struct GfxRenderTargetResource : public GfxResource
{
	static void release( GfxRenderTargetResource *&resource );

	GfxRenderTargetDescription desc = { };
	GfxTextureResource *textureColorSS = nullptr;
	GfxTextureResource *textureColorMS = nullptr;
	GfxTextureResource *textureDepthSS = nullptr;
	GfxTextureResource *textureDepthMS = nullptr;

	u16 width = 0;
	u16 height = 0;
	usize size = 0;
};


static GfxResourceFactory<GfxShaderResource, GFX_RESOURCE_COUNT_SHADER> shaderResources;
static GfxResourceFactory<GfxBufferResource, GFX_RESOURCE_COUNT_BUFFER> bufferResources;
static GfxResourceFactory<GfxVertexBufferResource, GFX_RESOURCE_COUNT_VERTEX_BUFFER> vertexBufferResources;
static GfxResourceFactory<GfxInstanceBufferResource, GFX_RESOURCE_COUNT_INSTANCE_BUFFER> instanceBufferResources;
static GfxResourceFactory<GfxIndexBufferResource, GFX_RESOURCE_COUNT_INDEX_BUFFER> indexBufferResources;
static GfxResourceFactory<GfxUniformBufferResource, GFX_RESOURCE_COUNT_UNIFORM_BUFFER> uniformBufferResources;
static GfxResourceFactory<GfxTextureResource, GFX_RESOURCE_COUNT_TEXTURE> textureResources;
static GfxResourceFactory<GfxRenderTargetResource, GFX_RESOURCE_COUNT_RENDER_TARGET> renderTargetResources;


static bool resources_init()
{
	shaderResources.init();
	bufferResources.init();
	vertexBufferResources.init();
	instanceBufferResources.init();
	indexBufferResources.init();
	uniformBufferResources.init();
	textureResources.init();
	renderTargetResources.init();

	return true;
}


static bool resources_free()
{
	renderTargetResources.free();
	textureResources.free();
	uniformBufferResources.free();
	indexBufferResources.free();
	instanceBufferResources.free();
	vertexBufferResources.free();
	bufferResources.free();
	shaderResources.free();

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Metal System

static const char *metal_get_error_message( NSError *error )
{
	if( error == nil ) { return ""; }
	NSString *errorMessage = error.localizedDescription;
	const char *message = [errorMessage UTF8String];
	return message;
};


static bool metal_device_init()
{
	device = MTLCreateSystemDefaultDevice();
	return device != nil;
}


static bool metal_device_free()
{
	[device release];
	device = nil;
	return true;
}


static bool metal_command_queue_init()
{
	mtlCommandQueue = [device newCommandQueue];
	return mtlCommandQueue != nil;
}


static bool metal_command_queue_free()
{
	[mtlCommandQueue release];
	mtlCommandQueue = nil;
	return true;
}


static bool metal_resource_cache_init()
{
	Assert( mtlVertexDescriptor == nil );
	mtlVertexDescriptor = [[MTLVertexDescriptor alloc] init];

	Assert( mtlSamplerCache == nil );
	mtlSamplerCache = [[NSMutableDictionary alloc] init];
	Assert( mtlSamplerDescriptor == nil );
	mtlSamplerDescriptor = [[MTLSamplerDescriptor alloc] init];

	Assert( mtlDepthStencilCache == nil );
	mtlDepthStencilCache = [[NSMutableDictionary alloc] init];
	Assert( mtlDepthStencilDescriptor == nil );
	mtlDepthStencilDescriptor = [[MTLDepthStencilDescriptor alloc] init];

	Assert( mtlPipelineCache == nil );
	mtlPipelineCache = [[NSMutableDictionary alloc] init];
	Assert( mtlPipelineDescriptor == nil );
	mtlPipelineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];

	for( int i = 0; i < GFX_RESOURCE_COUNT_UNIFORM_BUFFER; i++ )
	{
		stateBoundUniformBufferResources[i] = UniformBufferBinding { };
	}

	return true;
}


static bool metal_resource_cache_free()
{
	if( mtlVertexDescriptor ) { [mtlVertexDescriptor release]; mtlVertexDescriptor = nil; }

	if( mtlSamplerCache ) { [mtlSamplerCache release]; mtlSamplerCache = nil; }
	if( mtlSamplerDescriptor ) { [mtlSamplerDescriptor release]; mtlSamplerDescriptor = nil; }

	if( mtlDepthStencilCache ) { [mtlDepthStencilCache release]; mtlDepthStencilCache = nil; }
	if( mtlDepthStencilDescriptor ) { [mtlDepthStencilDescriptor release]; mtlDepthStencilDescriptor = nil; }

	if( mtlPipelineCache ) { [mtlPipelineCache release]; mtlPipelineCache = nil; }
	if( mtlPipelineDescriptor ) { [mtlPipelineDescriptor release]; mtlPipelineDescriptor = nil; }

	return true;
}


static void metal_command_buffer_begin()
{
	Assert( mtlCommandBuffer == nil );
	mtlCommandBuffer = [mtlCommandQueue commandBuffer];
}


static void metal_command_buffer_commit( bool present = false )
{
	Assert( mtlCommandBuffer != nil );
	Assert( mtlEncoder == nil );

	const u64 commitIDCallback = commitIDCurrent;
	[mtlCommandBuffer addCompletedHandler:^(id<MTLCommandBuffer> buffer) {
		dispatch_async( dispatch_get_global_queue( DISPATCH_QUEUE_PRIORITY_DEFAULT, 0 ), ^{
			// NOTE: The assumption here is that Metal executes this callback on
			// a background thread. This is vital because in situations of resource contention, the CPU
			// might be spin-locked while waiting for a resource to free up. If the callback executes on
			// the main thread, the spin-lock will never release resulting in a deadlock.

			for( GfxVertexBufferResource &resource : vertexBufferResources )
			{
				resource.writer.release( commitIDCallback );
			}

			for( GfxInstanceBufferResource &resource : instanceBufferResources )
			{
				resource.writer.release( commitIDCallback );
			}

			for( GfxUniformBufferResource &resource : uniformBufferResources )
			{
				resource.writer.release( commitIDCallback );
			}
		});
	}];

	if( present ) { [mtlCommandBuffer presentDrawable:drawable]; }
	[mtlCommandBuffer commit];
	mtlCommandBuffer = nil;

	commitIDCurrent++;
}


static void metal_encoder_begin()
{
	Assert( mtlCommandBuffer != nil );
	Assert( mtlEncoder == nil );
	mtlEncoder = [mtlCommandBuffer renderCommandEncoderWithDescriptor: mtlRenderPassDescriptor];
	Assert( mtlEncoder != nil );
}


static void metal_encoder_end()
{
	Assert( mtlEncoder != nil );
	[mtlEncoder endEncoding];
	mtlEncoder = nil;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Clear

static bool hasClearColor = false;
static Color clearColor = c_white;

static bool hasClearDepth = false;
static float clearDepth = 1.0f;


static void apply_clear_color()
{
	Assert( mtlEncoder == nil );

	if( hasClearColor )
	{
		static const float INV_255_F = 1.0f / 255.0f;
		const float r = clearColor.r * INV_255_F;
		const float g = clearColor.g * INV_255_F;
		const float b = clearColor.b * INV_255_F;
		const float a = clearColor.a * INV_255_F;

		for( int slot = 0; slot < GFX_RENDER_TARGET_SLOT_COUNT; slot++ )
		{
			if( mtlRenderPassDescriptor.colorAttachments[slot] == nil ) { continue; }
			mtlRenderPassDescriptor.colorAttachments[slot].loadAction = MTLLoadActionClear;
			mtlRenderPassDescriptor.colorAttachments[slot].clearColor = MTLClearColorMake( r, g, b, a );
			mtlRenderPassDescriptor.colorAttachments[slot].storeAction = MTLStoreActionStore;
		}
	}
	else
	{
		for( int slot = 0; slot < GFX_RENDER_TARGET_SLOT_COUNT; slot++ )
		{
			if( mtlRenderPassDescriptor.colorAttachments[slot] == nil ) { continue; }
			mtlRenderPassDescriptor.colorAttachments[slot].loadAction = MTLLoadActionLoad;
			mtlRenderPassDescriptor.colorAttachments[slot].storeAction = MTLStoreActionStore;
		}
	}

	hasClearColor = false;
}


static void apply_clear_depth()
{
	Assert( mtlEncoder == nil );

	if( mtlRenderPassDescriptor.depthAttachment != nil )
	{
		if( hasClearDepth )
		{
			mtlRenderPassDescriptor.depthAttachment.loadAction = MTLLoadActionClear;
			mtlRenderPassDescriptor.depthAttachment.clearDepth = clearDepth;
			mtlRenderPassDescriptor.depthAttachment.storeAction = MTLStoreActionStore;
		}
		else
		{
			mtlRenderPassDescriptor.depthAttachment.loadAction = MTLLoadActionLoad;
			mtlRenderPassDescriptor.depthAttachment.storeAction = MTLStoreActionStore;
		}
	}

	hasClearDepth = false;
	clearDepth = 1.0;
}


void CoreGfx::api_clear_color( Color color )
{
	hasClearColor = true;
	clearColor = color;
}


void CoreGfx::api_clear_depth( float depth )
{
	hasClearDepth = true;
	clearDepth = depth;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static u64 metal_pipeline_descriptor_hash()
{
	auto hash_combine = []( u64 a, u64 b ) -> u64
	{
		a ^= b + 0x9e3779b97f4a7c15ULL + ( a << 6 ) + ( a >> 2 );
		return a;
	};

	u64 h = reinterpret_cast<uintptr_t>( mtlPipelineDescriptor.vertexFunction );
	h = hash_combine( h, reinterpret_cast<uintptr_t>( mtlPipelineDescriptor.fragmentFunction ) );
	h = hash_combine( h, mtlPipelineDescriptor.rasterSampleCount );
	h = hash_combine( h, mtlPipelineDescriptor.depthAttachmentPixelFormat );

	for( int i = 0; i < 8; ++i )
	{
		auto *a = mtlPipelineDescriptor.colorAttachments[i];
		if( a == nil ) { continue; }
		h = hash_combine( h, a.pixelFormat );

		const u64 bh = ( static_cast<u64>( a.sourceRGBBlendFactor) |
			( static_cast<u64>( a.destinationRGBBlendFactor ) << 8 ) |
			( static_cast<u64>( a.rgbBlendOperation ) << 16 ) |
			( static_cast<u64>( a.sourceAlphaBlendFactor ) << 24) |
			( static_cast<u64>( a.destinationAlphaBlendFactor ) << 32 ) |
			( static_cast<u64>( a.alphaBlendOperation ) << 40 ) |
			( static_cast<u64>( a.writeMask ) << 48 ) |
			( static_cast<u64>( a.blendingEnabled ) << 56 ) );
		h = hash_combine( h, bh );
	}

	if( mtlPipelineDescriptor.vertexDescriptor )
	{
		for( int i = 0; i < 16; ++i )
		{
			auto *attr = mtlPipelineDescriptor.vertexDescriptor.attributes[i];
			if( attr == nil ) { continue; }
			const u64 ah = ( static_cast<u64>( attr.format ) ) |
				( static_cast<u64>( attr.offset ) << 8 ) |
				( static_cast<u64>( attr.bufferIndex ) << 32 );
			h = hash_combine( h, ah );
		}
		for( int i = 0; i < 4; ++i )
		{
			auto *layout = mtlPipelineDescriptor.vertexDescriptor.layouts[i];
			if( layout == nil ) { continue; }
			const u64 lh = ( static_cast<u64>( layout.stride ) ) |
				( static_cast<u64>( layout.stepRate ) << 16 ) |
				( static_cast<u64>( layout.stepFunction ) << 32 );
			h = hash_combine( h, lh );
		}
	}

	return h;
}


void metal_pipeline_descriptor_prepare_shaders( const GfxStatePipeline &pipelineState )
{
	Assert( mtlPipelineDescriptor != nil );
	Assert( mtlVertexDescriptor != nil );

	GfxShaderResource *resource = pipelineState.shader;
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	mtlPipelineDescriptor.vertexFunction = mtlVertexProgram[resource->id];
	mtlPipelineDescriptor.fragmentFunction = mtlFragmentProgram[resource->id];

	bool hasVertexFormat = ( resource->vertexFormat != U32_MAX );
	bool hasInstanceFormat = ( resource->instanceFormat != U32_MAX );
	if( hasVertexFormat || hasInstanceFormat )
	{
		[mtlVertexDescriptor reset];
		NSUInteger attributeOffset = 0;

		if( resource->vertexFormat != U32_MAX )
		{
			Assert( resource->vertexFormat < CoreGfx::inputLayoutFormatsVertexCount );
			const MetalInputLayoutFormats &layout = CoreGfx::inputLayoutFormatsVertex[resource->vertexFormat];
			for( NSUInteger i = 0; i < layout.attributesCount; i++ )
			{
				mtlVertexDescriptor.attributes[attributeOffset + i].format = layout.attributes[i].format;
				mtlVertexDescriptor.attributes[attributeOffset + i].offset = layout.attributes[i].offset;
				mtlVertexDescriptor.attributes[attributeOffset + i].bufferIndex = 0;
			}
			if( layout.attributesCount > 0 )
			{
				mtlVertexDescriptor.layouts[0].stride = layout.stepStride;
				mtlVertexDescriptor.layouts[0].stepRate = 1;
				mtlVertexDescriptor.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;
			}
			attributeOffset = layout.attributesCount;
		}

		if( resource->instanceFormat != U32_MAX )
		{
			Assert( resource->instanceFormat < CoreGfx::inputLayoutFormatsInstanceCount );
			const MetalInputLayoutFormats &layout = CoreGfx::inputLayoutFormatsInstance[resource->instanceFormat];
			for( NSUInteger i = 0; i < layout.attributesCount; i++ )
			{
				mtlVertexDescriptor.attributes[attributeOffset + i].format = layout.attributes[i].format;
				mtlVertexDescriptor.attributes[attributeOffset + i].offset = layout.attributes[i].offset;
				mtlVertexDescriptor.attributes[attributeOffset + i].bufferIndex = 1;
			}
			if( layout.attributesCount > 0 )
			{
				mtlVertexDescriptor.layouts[1].stride = layout.stepStride;
				mtlVertexDescriptor.layouts[1].stepRate = 1;
				mtlVertexDescriptor.layouts[1].stepFunction = MTLVertexStepFunctionPerInstance;
			}
		}

		mtlPipelineDescriptor.vertexDescriptor = mtlVertexDescriptor;
	}

	CoreGfx::state.pipeline.shader = pipelineState.shader;
}


void metal_pipeline_descriptor_prepare_targets( const GfxStatePipeline &pipelineState )
{
	Assert( mtlPipelineDescriptor != nil );

	if( isDefaultTargetActive )
	{
		mtlPipelineDescriptor.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
		mtlPipelineDescriptor.depthAttachmentPixelFormat = swapchainTextureDepth == nil ?
			MTLPixelFormatInvalid : MTLPixelFormatDepth32Float;
		mtlPipelineDescriptor.rasterSampleCount = 1;
	}
	else
	{
		int sampleCount = 0;

		for( int i = 0; i < GFX_RENDER_TARGET_SLOT_COUNT; i++ )
		{
			if( stateBoundTargetResources[i] == nullptr ) { continue; }

			mtlPipelineDescriptor.colorAttachments[i].pixelFormat =
				MetalColorFormats[stateBoundTargetResources[i]->desc.colorFormat];

			if( sampleCount == 0 ) { sampleCount = stateBoundTargetResources[i]->desc.sampleCount; continue; }
			AssertMsg( sampleCount == stateBoundTargetResources[i]->desc.sampleCount,
				"All render tagets must have the sample sample count!" );
		}

		if( stateBoundDepthTexture != 0U && stateBoundTargetResources[0] != nullptr )
		{
			mtlPipelineDescriptor.depthAttachmentPixelFormat =
				MetalDepthStencilFormats[stateBoundTargetResources[0]->desc.depthFormat];
		}

		mtlPipelineDescriptor.rasterSampleCount = max( sampleCount, 1 );
	}
}


void metal_pipeline_descriptor_prepare_blend_modes( const GfxStatePipeline &pipelineState )
{
	Assert( mtlPipelineDescriptor != nil );
	const GfxPipelineDescription &state = pipelineState.description;

	for( NSUInteger i = 0; i < GFX_RENDER_TARGET_SLOT_COUNT; ++i )
	{
		MTLRenderPipelineColorAttachmentDescriptor *attachment = mtlPipelineDescriptor.colorAttachments[i];
		if( attachment == nil ) { continue; }

		attachment.blendingEnabled = state.blendEnabled;

		attachment.rgbBlendOperation = MetalBlendOperations[state.blendOperationColor];
		attachment.alphaBlendOperation = MetalBlendOperations[state.blendOperationAlpha];

		attachment.sourceRGBBlendFactor = MetalBlendFactors[state.blendSrcFactorColor];
		attachment.destinationRGBBlendFactor = MetalBlendFactors[state.blendDstFactorColor];
		attachment.sourceAlphaBlendFactor = MetalBlendFactors[state.blendSrcFactorAlpha];
		attachment.destinationAlphaBlendFactor = MetalBlendFactors[state.blendDstFactorAlpha];

		MTLColorWriteMask writeMask = 0;
		if( state.blendColorWriteMask & GfxBlendWrite_RED ) { writeMask |= MTLColorWriteMaskRed; }
		if( state.blendColorWriteMask & GfxBlendWrite_GREEN ) { writeMask |= MTLColorWriteMaskGreen; }
		if( state.blendColorWriteMask & GfxBlendWrite_BLUE ) { writeMask |= MTLColorWriteMaskBlue; }
		if( state.blendColorWriteMask & GfxBlendWrite_ALPHA ) { writeMask |= MTLColorWriteMaskAlpha; }
		attachment.writeMask = writeMask;
	}

	CoreGfx::state.pipeline.description.blend_set( pipelineState.description );
	BITFLAG_UNSET( CoreGfx::state.dirtyFlags, GfxStateDirtyFlag_BLEND );
}


static void metal_prepare_pipeline_state( const GfxStatePipeline &pipelineState )
{
	Assert( mtlPipelineDescriptor != nil );
	[mtlPipelineDescriptor reset];

	metal_pipeline_descriptor_prepare_shaders( pipelineState );
	metal_pipeline_descriptor_prepare_targets( pipelineState );
	metal_pipeline_descriptor_prepare_blend_modes( pipelineState );

	NSNumber *key = [NSNumber numberWithUnsignedLongLong: metal_pipeline_descriptor_hash()];
	id<MTLRenderPipelineState> mtlPipelineStateCached = [mtlPipelineCache objectForKey: key];

	if( mtlPipelineStateCached )
	{
		mtlPipelineState = mtlPipelineStateCached;
		Assert( mtlPipelineState != nil );
	}
	else
	{
		NSError *error = nil;
		mtlPipelineState = [device newRenderPipelineStateWithDescriptor: mtlPipelineDescriptor error: &error];
		ErrorIf( !mtlPipelineState, "%s: Failed to init pipeline state!\n\n%s",
			__FUNCTION__, metal_get_error_message( error ) );
		Assert( mtlPipelineState != nil );
		[mtlPipelineCache setObject: mtlPipelineState forKey: key];
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static u64 metal_depth_stencil_descriptor_hash()
{
	auto hash_combine = []( u64 a, u64 b ) -> u64
	{
		a ^= b + 0x9e3779b97f4a7c15ULL + ( a << 6 ) + ( a >> 2 );
		return a;
	};

	u64 h = 0;
	h = hash_combine( h, static_cast<u64>( mtlDepthStencilDescriptor.depthCompareFunction ) );
	h = hash_combine( h, static_cast<u64>( mtlDepthStencilDescriptor.depthWriteEnabled ) );

	if( mtlDepthStencilDescriptor.frontFaceStencil != nil )
	{
		h = hash_combine( h, static_cast<u64>( mtlDepthStencilDescriptor.frontFaceStencil.stencilCompareFunction ) );
		h = hash_combine( h, static_cast<u64>( mtlDepthStencilDescriptor.frontFaceStencil.stencilFailureOperation ) );
		h = hash_combine( h, static_cast<u64>( mtlDepthStencilDescriptor.frontFaceStencil.depthFailureOperation ) );
		h = hash_combine( h, static_cast<u64>( mtlDepthStencilDescriptor.frontFaceStencil.depthStencilPassOperation ) );
		h = hash_combine( h, static_cast<u64>( mtlDepthStencilDescriptor.frontFaceStencil.readMask ) );
		h = hash_combine( h, static_cast<u64>( mtlDepthStencilDescriptor.frontFaceStencil.writeMask ) );
	}

	if( mtlDepthStencilDescriptor.backFaceStencil != nil )
	{
		h = hash_combine( h, static_cast<u64>( mtlDepthStencilDescriptor.backFaceStencil.stencilCompareFunction ) );
		h = hash_combine( h, static_cast<u64>( mtlDepthStencilDescriptor.backFaceStencil.stencilFailureOperation ) );
		h = hash_combine( h, static_cast<u64>( mtlDepthStencilDescriptor.backFaceStencil.depthFailureOperation ) );
		h = hash_combine( h, static_cast<u64>( mtlDepthStencilDescriptor.backFaceStencil.depthStencilPassOperation ) );
		h = hash_combine( h, static_cast<u64>( mtlDepthStencilDescriptor.backFaceStencil.readMask ) );
		h = hash_combine( h, static_cast<u64>( mtlDepthStencilDescriptor.backFaceStencil.writeMask ) );
	}

	return h;
}


static void metal_prepare_depth_stencil_state( const GfxPipelineDescription &state )
{
	Assert( mtlDepthStencilDescriptor != nil );

	const bool isDepthEnabled = ( state.depthFunction != GfxDepthFunction_NONE );
	if( isDepthEnabled )
	{
		mtlDepthStencilDescriptor.depthCompareFunction = MetalDepthFunctions[state.depthFunction];
		mtlDepthStencilDescriptor.depthWriteEnabled = ( state.depthWriteMask == GfxDepthWrite_ALL );
	}
	else
	{
		mtlDepthStencilDescriptor.depthCompareFunction = MTLCompareFunctionAlways;
		mtlDepthStencilDescriptor.depthWriteEnabled = NO;
	}

	NSNumber *key = [NSNumber numberWithUnsignedLongLong: metal_depth_stencil_descriptor_hash()];
	id<MTLDepthStencilState> mtlDepthStencilStateCached = [mtlDepthStencilCache objectForKey: key];

	if( mtlDepthStencilStateCached )
	{
		mtlDepthStencilState = mtlDepthStencilStateCached;
		Assert( mtlDepthStencilState != nil );
	}
	else
	{
		mtlDepthStencilState = [device newDepthStencilStateWithDescriptor: mtlDepthStencilDescriptor];
		Assert( mtlDepthStencilState != nil );
		[mtlDepthStencilCache setObject: mtlDepthStencilState forKey: key];
	}

	CoreGfx::state.pipeline.description.depth_set_state( state );
	BITFLAG_UNSET( CoreGfx::state.dirtyFlags, GfxStateDirtyFlag_DEPTH );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static u64 metal_sampler_descriptor_hash()
{
	auto hash_combine = []( u64 a, u64 b ) -> u64
	{
		a ^= b + 0x9e3779b97f4a7c15ULL + ( a << 6 ) + ( a >> 2 );
		return a;
	};

	u64 h = 0;

	h = hash_combine( h, static_cast<u64>( mtlSamplerDescriptor.minFilter ) );
	h = hash_combine( h, static_cast<u64>( mtlSamplerDescriptor.magFilter ) );
	h = hash_combine( h, static_cast<u64>( mtlSamplerDescriptor.mipFilter ) );

	h = hash_combine( h, static_cast<u64>( mtlSamplerDescriptor.sAddressMode));
	h = hash_combine( h, static_cast<u64>( mtlSamplerDescriptor.tAddressMode));
	h = hash_combine( h, static_cast<u64>( mtlSamplerDescriptor.rAddressMode));

	h = hash_combine( h, static_cast<u64>( mtlSamplerDescriptor.lodMinClamp * 1000.0 ) );
	h = hash_combine( h, static_cast<u64>( mtlSamplerDescriptor.lodMaxClamp * 1000.0 ) );
	h = hash_combine( h, static_cast<u64>( mtlSamplerDescriptor.lodAverage * 1000.0 ) );

	h = hash_combine( h, static_cast<u64>( mtlSamplerDescriptor.maxAnisotropy ) );
	h = hash_combine( h, static_cast<u64>( mtlSamplerDescriptor.normalizedCoordinates ) );
	h = hash_combine( h, static_cast<u64>( mtlSamplerDescriptor.compareFunction ) );

	h = hash_combine( h, static_cast<u64>( mtlSamplerDescriptor.supportArgumentBuffers ) );

	return h;
}


static void metal_prepare_sampler_state( const GfxStateSampler &state )
{
	Assert( mtlSamplerDescriptor != nil );

	switch( state.filterMode )
	{
		case GfxSamplerFilteringMode_NEAREST:
			mtlSamplerDescriptor.minFilter = MTLSamplerMinMagFilterNearest;
			mtlSamplerDescriptor.magFilter = MTLSamplerMinMagFilterNearest;
			mtlSamplerDescriptor.mipFilter = MTLSamplerMipFilterNearest;
		break;

		case GfxSamplerFilteringMode_LINEAR:
			mtlSamplerDescriptor.minFilter = MTLSamplerMinMagFilterLinear;
			mtlSamplerDescriptor.magFilter = MTLSamplerMinMagFilterLinear;
			mtlSamplerDescriptor.mipFilter = MTLSamplerMipFilterLinear;
		break;

		case GfxSamplerFilteringMode_MAG_NEAREST_MIN_LINEAR:
			mtlSamplerDescriptor.minFilter = MTLSamplerMinMagFilterLinear;
			mtlSamplerDescriptor.magFilter = MTLSamplerMinMagFilterNearest;
			mtlSamplerDescriptor.mipFilter = MTLSamplerMipFilterLinear;
		break;

		case GfxSamplerFilteringMode_MAG_LINEAR_MIN_NEAREST:
			mtlSamplerDescriptor.minFilter = MTLSamplerMinMagFilterNearest;
			mtlSamplerDescriptor.magFilter = MTLSamplerMinMagFilterLinear;
			mtlSamplerDescriptor.mipFilter = MTLSamplerMipFilterLinear;
		break;

		case GfxSamplerFilteringMode_ANISOTROPIC:
			mtlSamplerDescriptor.minFilter = MTLSamplerMinMagFilterLinear;
			mtlSamplerDescriptor.magFilter = MTLSamplerMinMagFilterLinear;
			mtlSamplerDescriptor.mipFilter = MTLSamplerMipFilterLinear;
			mtlSamplerDescriptor.maxAnisotropy = state.anisotropy;
		break;

		default:
			mtlSamplerDescriptor.minFilter = MTLSamplerMinMagFilterNearest;
			mtlSamplerDescriptor.magFilter = MTLSamplerMinMagFilterNearest;
			mtlSamplerDescriptor.mipFilter = MTLSamplerMipFilterNearest;
		break;
	}

	MTLSamplerAddressMode addressMode;
	switch( state.wrapMode )
	{
		case GfxSamplerWrapMode_WRAP: addressMode = MTLSamplerAddressModeRepeat; break;
		case GfxSamplerWrapMode_MIRROR: addressMode = MTLSamplerAddressModeMirrorRepeat; break;
		case GfxSamplerWrapMode_CLAMP: addressMode = MTLSamplerAddressModeClampToEdge; break;
		default: addressMode = MTLSamplerAddressModeRepeat; break;
	}

	mtlSamplerDescriptor.sAddressMode = addressMode;
	mtlSamplerDescriptor.tAddressMode = addressMode;
	mtlSamplerDescriptor.rAddressMode = addressMode;

	mtlSamplerDescriptor.lodMinClamp = 0.0;
	mtlSamplerDescriptor.lodMaxClamp = FLT_MAX;
	mtlSamplerDescriptor.normalizedCoordinates = YES;

	NSNumber *key = [NSNumber numberWithUnsignedLongLong: metal_sampler_descriptor_hash()];
	id<MTLSamplerState> mtlSamplerStateCached = [mtlSamplerCache objectForKey: key];

	if( mtlSamplerStateCached )
	{
		mtlSamplerState = mtlSamplerStateCached;
		Assert( mtlSamplerState != nil );
	}
	else
	{
		mtlSamplerState = [device newSamplerStateWithDescriptor: mtlSamplerDescriptor];
		Assert( mtlSamplerState != nil );
		[mtlSamplerCache setObject: mtlSamplerState forKey: key];
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void metal_prepare_viewport( const GfxStateViewport &state )
{
	mtlViewport.originX = 0.0;
	mtlViewport.originY = 0.0;
	mtlViewport.width = static_cast<double>( state.width );
	mtlViewport.height = static_cast<double>( state.height );
	mtlViewport.znear = 0.0;
	mtlViewport.zfar = 1.0;
}


static void metal_prepare_scissor( const GfxStateScissor &state )
{
	if( CoreGfx::state.scissor.is_enabled() )
	{
		mtlScissorRect.x = state.x1;
		mtlScissorRect.y = state.y1;
		mtlScissorRect.width = ( state.x2 - state.x1 );
		mtlScissorRect.height = ( state.y2 - state.y1 );
	}
	else
	{
		// NOTE: Disabling scissor in Metal is the same as matching it to viewport size
		mtlScissorRect.x = 0;
		mtlScissorRect.y = 0;
		mtlScissorRect.width = static_cast<u32>( CoreGfx::state.viewport.width );
		mtlScissorRect.height = static_cast<u32>( CoreGfx::state.viewport.height );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void metal_prepare_encoder_raster_state( const GfxPipelineDescription &state )
{
	if( mtlEncoder != nil )
	{
		[mtlEncoder setCullMode: MetalCullModes[state.rasterCullMode]];
		[mtlEncoder setFrontFacingWinding: MTLWindingClockwise];
		[mtlEncoder setTriangleFillMode: MetalFillModes[state.rasterFillMode]];
	}

	CoreGfx::state.pipeline.description.raster_set( state );
	BITFLAG_UNSET( CoreGfx::state.dirtyFlags, GfxStateDirtyFlag_RASTER );
}


static void metal_prepare_encoder_texture_bindings()
{
	Assert( mtlEncoder != nil );

	for( int slot = 0; slot < GFX_TEXTURE_SLOT_COUNT; slot++ )
	{
		if( stateBoundTextureResources[slot] == nullptr ) { continue; }
		const u32 resourceID = stateBoundTextureResources[slot]->id;
		if( resourceID == U32_MAX ) { stateBoundTextureResources[slot] = nullptr; continue; }

		[mtlEncoder setVertexTexture: mtlTextures[resourceID] atIndex: slot];
		[mtlEncoder setFragmentTexture: mtlTextures[resourceID] atIndex: slot];
	}
}


static void metal_prepare_encoder( const GfxStatePipeline &pipelineState )
{
	apply_clear_color();
	apply_clear_depth();
	metal_encoder_begin();

	metal_prepare_pipeline_state( pipelineState );
	[mtlEncoder setRenderPipelineState: mtlPipelineState];

	metal_prepare_depth_stencil_state( pipelineState.description );
	[mtlEncoder setDepthStencilState: mtlDepthStencilState];

	metal_prepare_sampler_state( CoreGfx::state.sampler );
	[mtlEncoder setFragmentSamplerState: mtlSamplerState atIndex: 0];

	metal_prepare_viewport( CoreGfx::state.viewport );
	[mtlEncoder setViewport: mtlViewport];

	metal_prepare_scissor( CoreGfx::state.scissor );
	[mtlEncoder setScissorRect: mtlScissorRect];

	metal_prepare_encoder_raster_state( pipelineState.description );
	metal_prepare_encoder_texture_bindings();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Metal Render State

static void render_target_2d_resolve_msaa( GfxRenderTargetResource *resource )
{
	if( resource == nullptr ) { return; }
	if( resource->desc.sampleCount <= 1 ) { return; }
	if( resource->desc.colorFormat == GfxColorFormat_NONE ) { return; }

	Assert( mtlEncoder == nil );
	Assert( resource->textureColorMS != nullptr && resource->textureColorMS->id != GFX_RESOURCE_ID_NULL );
	Assert( resource->textureColorSS != nullptr && resource->textureColorSS->id != GFX_RESOURCE_ID_NULL );

	id<MTLCommandBuffer> resolveCommandBuffer = [mtlCommandQueue commandBuffer];

	MTLRenderPassDescriptor *resolvePass = [[MTLRenderPassDescriptor renderPassDescriptor] retain];

	resolvePass.colorAttachments[0].texture = mtlTextures[resource->textureColorMS->id];
	resolvePass.colorAttachments[0].resolveTexture = mtlTextures[resource->textureColorSS->id];
	resolvePass.colorAttachments[0].loadAction = MTLLoadActionLoad;
	resolvePass.colorAttachments[0].storeAction = MTLStoreActionMultisampleResolve;

	id<MTLRenderCommandEncoder> resolveEncoder = [resolveCommandBuffer renderCommandEncoderWithDescriptor: resolvePass];
	[resolveEncoder endEncoding];
	[resolvePass release];

	[resolveCommandBuffer commit];
}


static bool bind_targets( GfxRenderTargetResource *const *resources )
{
	static u16 cacheViewportWidth;
	static u16 cacheViewportHeight;
	static double_m44 cacheMatrixModel;
	static double_m44 cacheMatrixView;
	static double_m44 cacheMatrixPerspective;
	static bool hasCachedSwapchainState = false;

	bool dirtySlot[GFX_RENDER_TARGET_SLOT_COUNT] = { false };
	bool dirtyState = BITFLAG_IS_SET( CoreGfx::state.dirtyFlags, GfxStateDirtyFlag_TARGETS );
	bool hasCustomRenderTargets = false;

	// FBO State Cache
	for( int slot = 0; slot < GFX_RENDER_TARGET_SLOT_COUNT; slot++ )
	{
		GfxRenderTargetResource *const resource = resources[slot];

		if( stateBoundTargetResources[slot] != nullptr && stateBoundTargetResources[slot] != resource )
		{
			render_target_2d_resolve_msaa( stateBoundTargetResources[slot] );
		}

		hasCustomRenderTargets |= ( resource != nullptr );
		dirtySlot[slot] |= ( stateBoundTargetResources[slot] != resource );
		stateBoundTargetResources[slot] = resource;

		if( resource == nullptr )
		{
			dirtySlot[slot] |= ( stateBoundColorTextures[slot] != 0U );
			stateBoundColorTextures[slot] = 0U;

			if( slot == 0 )
			{
				dirtySlot[slot] |= ( stateBoundDepthTexture != 0U );
				stateBoundDepthTexture = 0U;
			}
		}
		else
		{
			const bool hasMultiSample = resource->desc.sampleCount > 1;
			const bool hasColor = resource->desc.colorFormat != GfxColorFormat_NONE;
			hasCustomRenderTargets |= hasColor;
			const bool hasDepth = resource->desc.depthFormat != GfxDepthFormat_NONE;
			hasCustomRenderTargets |= hasDepth;

			// Color
			if( hasColor )
			{
				const u32 colorTexture = hasMultiSample ?
					resource->textureColorMS->id : resource->textureColorSS->id;
				dirtySlot[slot] |= ( stateBoundColorTextures[slot] != ( colorTexture + 1 ) );
				stateBoundColorTextures[slot] = ( colorTexture + 1 );
			}
			else
			{
				dirtySlot[slot] |= ( stateBoundColorTextures[slot] != 0 );
				stateBoundColorTextures[slot] = 0;
			}

			// Depth
			if( slot == 0 )
			{
				if( hasDepth )
				{
					const u32 depthTexture = hasMultiSample ?
						resource->textureDepthMS->id : resource->textureDepthSS->id;
					dirtySlot[slot] |= ( stateBoundDepthTexture != ( depthTexture + 1 ) );
					stateBoundDepthTexture = ( depthTexture + 1 );
				}
				else
				{
					dirtySlot[slot] |= ( stateBoundDepthTexture != 0U );
					stateBoundDepthTexture = 0U;
				}
			}
		}

		dirtyState |= dirtySlot[slot];
	}

	const bool cacheSwapchainState = isDefaultTargetActive && !hasCachedSwapchainState;
	isDefaultTargetActive = !hasCustomRenderTargets;
	if( !dirtyState ) { return true; }

#if !SWAPCHAIN_DEPTH_ENABLED
	if( isDefaultTargetActive )
	{
		// NOTE: When returning to the default swapchain and not SWAPCHAIN_DEPTH_ENABLED, we explicitly disable depth
		GfxPipelineDescription desc = CoreGfx::state.pipeline.description;
		desc.depthFunction = GfxDepthFunction_NONE;
		desc.depthWriteMask = GfxDepthWrite_NONE;
		CoreGfx::state.pipeline.description.depth_set_state( desc );
	}
#endif

	if( mtlRenderPassDescriptor ) { [mtlRenderPassDescriptor release]; mtlRenderPassDescriptor = nil; }
	mtlRenderPassDescriptor = [[MTLRenderPassDescriptor renderPassDescriptor] retain];

	if( hasCustomRenderTargets )
	{
		for( int slot = 0; slot < GFX_RENDER_TARGET_SLOT_COUNT; slot++ )
		{
			if( !dirtySlot[slot] ) { continue; }
			mtlRenderPassDescriptor.colorAttachments[slot].texture = stateBoundColorTextures[slot] == 0U ?
				nil : mtlTextures[stateBoundColorTextures[slot] - 1];
			mtlRenderPassDescriptor.colorAttachments[slot].loadAction = MTLLoadActionLoad;
			mtlRenderPassDescriptor.colorAttachments[slot].storeAction = MTLStoreActionStore;
		}

		if( dirtySlot[0] )
		{
			mtlRenderPassDescriptor.depthAttachment.texture = stateBoundDepthTexture == 0U ?
				nil : mtlTextures[stateBoundDepthTexture - 1];
			mtlRenderPassDescriptor.depthAttachment.loadAction = MTLLoadActionLoad;
			mtlRenderPassDescriptor.depthAttachment.storeAction = MTLStoreActionStore;
		}
	}
	else
	{
		mtlRenderPassDescriptor.colorAttachments[0].texture = drawable.texture;
		mtlRenderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionLoad;
		mtlRenderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;

		if( swapchainTextureDepth != nil )
		{
			mtlRenderPassDescriptor.depthAttachment.texture = swapchainTextureDepth;
			mtlRenderPassDescriptor.depthAttachment.loadAction = MTLLoadActionLoad;
			mtlRenderPassDescriptor.depthAttachment.storeAction = MTLStoreActionStore;
		}
	}

	// Metal: When binding to a new render pass, we need to update the viewport and MVP matrices
	if( hasCustomRenderTargets )
	{
		if( cacheSwapchainState )
		{
			cacheViewportWidth = CoreGfx::state.viewport.width;
			cacheViewportHeight = CoreGfx::state.viewport.height;
			cacheMatrixModel = Gfx::get_matrix_model();
			cacheMatrixView = Gfx::get_matrix_view();
			cacheMatrixPerspective = Gfx::get_matrix_perspective();
			hasCachedSwapchainState = true;
		}

		if( resources[0] != nullptr )
		{
			const u16 width = resources[0]->width;
			const u16 height = resources[0]->height;
			Gfx::viewport_set_size( width, height );
			Gfx::set_matrix_model( double_m44_build_identity() );
			Gfx::set_matrix_view( double_m44_build_identity() );
			Gfx::set_matrix_perspective( double_m44_build_orthographic( 0.0, width, 0.0, height, 0.0, 1.0 ) );
		}
	}
	else if( hasCachedSwapchainState )
	{
		Gfx::viewport_set_size( cacheViewportWidth, cacheViewportHeight );
		Gfx::set_matrix_mvp( cacheMatrixModel, cacheMatrixView, cacheMatrixPerspective );
		hasCachedSwapchainState = false;
	}

	BITFLAG_UNSET( CoreGfx::state.dirtyFlags, GfxStateDirtyFlag_TARGETS );
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
// GFX System

bool CoreGfx::api_init()
{
#if SWAPCHAIN_DPI_SCALED
	const u16 w = Window::width;
	const u16 h = Window::height;
#else
	const u16 w = Window::width * Window::scale;
	const u16 h = Window::height * Window::scale;
#endif

	ErrorReturnIf( !metal_device_init(), false, "%s: Failed to initialize Metal device", __FUNCTION__ );
	ErrorReturnIf( !metal_command_queue_init(), false, "%s: Failed to initialize Metal command queue", __FUNCTION__ );
	ErrorReturnIf( !metal_resource_cache_init(), false, "%s: Failed to initialize Metal resource cache", __FUNCTION__ );
	ErrorReturnIf( !api_swapchain_init( w, h ), false, "%s: Failed to initialize swap chain", __FUNCTION__ );
	ErrorReturnIf( !resources_init(), false, "%s: Failed to initialize gfx resources", __FUNCTION__ );

	return true;
}


bool CoreGfx::api_free()
{
	resources_free();
	api_swapchain_free();
	metal_resource_cache_free();
	metal_command_queue_free();
	metal_device_free();

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Frame

void CoreGfx::api_frame_begin()
{
	metal_command_buffer_begin();

	if( drawable ) { [drawable release]; drawable = nil; }
	drawable = [[layer nextDrawable] retain];
	if( !drawable ) { Error( "Invalid drawable!" ); return; } // TODO: Skip frame

	BITFLAG_SET( CoreGfx::state.dirtyFlags, GfxStateDirtyFlag_TARGETS );
	render_targets_reset();
	CoreGfx::state.scissor = GfxStateScissor { };
	api_sampler_set_state( GfxStateSampler { } );
}


void CoreGfx::api_frame_end()
{
	if( mtlEncoder != nil ) { metal_encoder_end(); }
	metal_command_buffer_commit( true );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Render State

bool CoreGfx::api_swapchain_init( u16 width, u16 height )
{
	CoreGfx::state.swapchain.width = width;
	CoreGfx::state.swapchain.height = height;

	Assert( layer == nil );
	layer = [[CAMetalLayer layer] retain];
	layer.device = device;
	layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
	layer.framebufferOnly = YES;
	layer.drawableSize = CGSizeMake( width, height );
	layer.contentsScale = Window::scale;
	layer.opaque = YES;
	layer.displaySyncEnabled = YES;
	CoreWindow::view.layer = layer;

	PROFILE_GFX( Gfx::stats.gpuMemorySwapchain +=
		GFX_SIZE_IMAGE_COLOR_BYTES( width, height, 1, GfxColorFormat_R8G8B8A8_FLOAT ) * 2 );

#if SWAPCHAIN_DEPTH_ENABLED
	MTLTextureDescriptor *depthDescriptor = [MTLTextureDescriptor
		texture2DDescriptorWithPixelFormat: MTLPixelFormatDepth32Float
		width: width
		height: height
		mipmapped: NO];
	depthDescriptor.usage = MTLTextureUsageRenderTarget;

	Assert( swapchainTextureDepth == nil );
	swapchainTextureDepth = [device newTextureWithDescriptor: depthDescriptor];
	ErrorReturnIf( swapchainTextureDepth == nil, false, "Failed to initialize swapchain depth buffer!" );

	PROFILE_GFX( Gfx::stats.gpuMemorySwapchain +=
		GFX_SIZE_IMAGE_DEPTH_BYTES( width, height, 1, SWAPCHAIN_DEPTH_FORMAT ) * 2 );
#endif

	CoreGfx::api_viewport_set_size( width, height );
	return true;
}


bool CoreGfx::api_swapchain_free()
{
	if( layer ) { [layer release]; layer = nil; }
	CoreWindow::view.layer = nil;

	if( drawable ) { [drawable release]; drawable = nil; }
	if( swapchainTextureDepth ) { [swapchainTextureDepth release]; swapchainTextureDepth = nil; }
	return true;
}


bool CoreGfx::api_swapchain_set_size( u16 width, u16 height )
{
	const u16 widthPrevious = CoreGfx::state.swapchain.width;
	const u16 heightPrevious = CoreGfx::state.swapchain.height;

	CoreGfx::state.swapchain.width = width;
	CoreGfx::state.swapchain.height = height;

	layer.drawableSize = CGSizeMake( width, height );
	layer.contentsScale = Window::scale;

	PROFILE_GFX( Gfx::stats.gpuMemorySwapchain -=
		GFX_SIZE_IMAGE_COLOR_BYTES( widthPrevious, heightPrevious, 1, GfxColorFormat_R8G8B8A8_FLOAT ) * 2 );
	PROFILE_GFX( Gfx::stats.gpuMemorySwapchain +=
		GFX_SIZE_IMAGE_COLOR_BYTES( width, height, 1, GfxColorFormat_R8G8B8A8_FLOAT ) * 2 );

#if SWAPCHAIN_DEPTH_ENABLED
	MTLTextureDescriptor *depthDescriptor = [MTLTextureDescriptor
		texture2DDescriptorWithPixelFormat: MTLPixelFormatDepth32Float
		width: width
		height: height
		mipmapped: NO];
	depthDescriptor.usage = MTLTextureUsageRenderTarget;

	if( swapchainTextureDepth ) { [swapchainTextureDepth release]; swapchainTextureDepth = nil; }
	swapchainTextureDepth = [device newTextureWithDescriptor: depthDescriptor];
	Assert( swapchainTextureDepth != nil );

	PROFILE_GFX( Gfx::stats.gpuMemorySwapchain -=
		GFX_SIZE_IMAGE_DEPTH_BYTES( widthPrevious, heightPrevious, 1, SWAPCHAIN_DEPTH_FORMAT ) * 2 );
	PROFILE_GFX( Gfx::stats.gpuMemorySwapchain +=
		GFX_SIZE_IMAGE_DEPTH_BYTES( width, height, 1, SWAPCHAIN_DEPTH_FORMAT ) * 2 );
#endif

	BITFLAG_SET( CoreGfx::state.dirtyFlags, GfxStateDirtyFlag_TARGETS );
	render_targets_reset();
	return true;
}


bool CoreGfx::api_viewport_init( u16 width, u16 height )
{
	return api_viewport_set_size( width, height );
}


bool CoreGfx::api_viewport_free()
{
	return true;
}


bool CoreGfx::api_viewport_set_size( u16 width, u16 height )
{
	CoreGfx::state.viewport.width = width;
	CoreGfx::state.viewport.height = height;
	return true;
}


bool CoreGfx::api_scissor_set_state( const GfxStateScissor &state )
{
	CoreGfx::state.scissor = state;

	if( mtlEncoder != nil )
	{
		metal_prepare_scissor( state );
		[mtlEncoder setScissorRect: mtlScissorRect];
	}

	return true;
}


bool CoreGfx::api_sampler_set_state( const GfxStateSampler &state )
{
	CoreGfx::state.sampler = state;
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Pipeline State

bool CoreGfx::api_set_raster( const GfxPipelineDescription &description )
{
	metal_prepare_encoder_raster_state( description );
	return true;
}


bool CoreGfx::api_set_blend( const GfxPipelineDescription &description )
{
	// TODO
	//return apply_pipeline_state_blend( description );
	return true;
}


bool CoreGfx::api_set_depth( const GfxPipelineDescription &description )
{
	// TODO
	//return apply_pipeline_state_depth( description );
	return true;
}


bool CoreGfx::api_set_stencil( const GfxPipelineDescription &description )
{
	// TODO
	//return apply_pipeline_state_stencil( description );
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Shader

void GfxShaderResource::release( GfxShaderResource *&resource )
{
	if( resource == nullptr || resource->id == GFX_RESOURCE_ID_NULL ) { return; }

	if( mtlVertexProgram[resource->id] != nil )
	{
		[mtlVertexProgram[resource->id] release];
		mtlVertexProgram[resource->id] = nil;
	}

	if( mtlFragmentProgram[resource->id] != nil )
	{
		[mtlFragmentProgram[resource->id] release];
		mtlFragmentProgram[resource->id] = nil;
	}

	if( mtlComputeProgram[resource->id] != nil )
	{
		[mtlComputeProgram[resource->id] release];
		mtlComputeProgram[resource->id] = nil;
	}

	shaderResources.remove( resource->id );
	resource = nullptr;
}


bool CoreGfx::api_shader_init( GfxShaderResource *&resource, u32 shaderID,
	const struct ShaderEntry &shaderEntry )
{
	Assert( resource == nullptr );
	Assert( shaderID < CoreGfx::shaderCount );

	resource = shaderResources.make_new();
	resource->shaderID = shaderID;
	resource->vertexFormat = shaderEntry.vertexFormat;
	resource->instanceFormat = shaderEntry.instanceFormat;

	// Vertex
	{
		const char *bytecodeData = reinterpret_cast<const char *>(
			Assets::binary.data + shaderEntry.offsetVertex + BINARY_OFFSET_GFX );
		const usize bytecodeSize = shaderEntry.sizeVertex;
		NSError *error = nil;
		dispatch_data_t data = dispatch_data_create( bytecodeData, bytecodeSize, NULL, DISPATCH_DATA_DESTRUCTOR_DEFAULT );
		id<MTLLibrary> library = [device newLibraryWithData: data error: &error];
		mtlVertexProgram[resource->id] = [library newFunctionWithName: @"vs_main"];
		dispatch_release( data );
		[library release];
	}

	// Fragment
	{
		const char *bytecodeData = reinterpret_cast<const char *>(
			Assets::binary.data + shaderEntry.offsetFragment + BINARY_OFFSET_GFX );
		const usize bytecodeSize = shaderEntry.sizeFragment;
		NSData *libraryData = [NSData dataWithBytes: bytecodeData length: bytecodeSize];
		NSError *error = nil;
		dispatch_data_t data = dispatch_data_create( bytecodeData, bytecodeSize, NULL, DISPATCH_DATA_DESTRUCTOR_DEFAULT );
		id<MTLLibrary> library = [device newLibraryWithData: data error: &error];
		mtlFragmentProgram[resource->id] = [library newFunctionWithName: @"fs_main"];
		dispatch_release( data );
		[library release];
	}

	PROFILE_GFX( Gfx::stats.gpuMemoryShaders += resource->sizeVS );
	PROFILE_GFX( Gfx::stats.gpuMemoryShaders += resource->sizePS );
	PROFILE_GFX( Gfx::stats.gpuMemoryShaders += resource->sizeCS );
	return true;
}


bool CoreGfx::api_shader_free( GfxShaderResource *&resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	PROFILE_GFX( Gfx::stats.gpuMemoryShaders -= resource->sizeVS );
	PROFILE_GFX( Gfx::stats.gpuMemoryShaders -= resource->sizePS );
	PROFILE_GFX( Gfx::stats.gpuMemoryShaders -= resource->sizeCS );

	GfxShaderResource::release( resource );
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Buffer

void GfxBufferResource::release( GfxBufferResource *&resource )
{
	if( resource == nullptr || resource->id == GFX_RESOURCE_ID_NULL ) { return; }

	if( metalBuffers[resource->id] != nil )
	{
		[metalBuffers[resource->id] release];
		metalBuffers[resource->id] = nil;
	}

	bufferResources.remove( resource->id );

	resource = nullptr;
}


bool CoreGfx::api_buffer_init( GfxBufferResource *&resource, usize capacity )
{
	Assert( resource == nullptr );

	resource = bufferResources.make_new();
	resource->capacity = capacity;
	resource->mapped = false;

	metalBuffers[resource->id] = [device
		newBufferWithLength: capacity
		options: MTLResourceStorageModeManaged];

	if( metalBuffers[resource->id] == nil )
	{
		GfxBufferResource::release( resource );
		ErrorReturnMsg( false, "%s: Failed to create MTLBuffer for GfxBufferResource!", __FUNCTION__ );
	}

	return true;
}


bool CoreGfx::api_buffer_free( GfxBufferResource *&resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	GfxBufferResource::release( resource );

	return true;
}


void CoreGfx::api_buffer_write_begin( GfxBufferResource *resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	Assert( !resource->mapped );
	resource->mapped = true;

	resource->writeRangeStart = USIZE_MAX;
	resource->writeRangeEnd = 0LLU;
}


void CoreGfx::api_buffer_write_end( GfxBufferResource *resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	Assert( resource->mapped );
	resource->mapped = false;

	if( resource->writeRangeEnd <= resource->writeRangeStart ) { return; }

	const usize offset = resource->writeRangeStart;
	const usize length = resource->writeRangeEnd - resource->writeRangeStart;
	[metalBuffers[resource->id] didModifyRange: NSMakeRange( offset, length )];
}


void CoreGfx::api_buffer_write( GfxBufferResource *resource, const void *data, usize size, usize offset )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	Assert( resource->mapped );
	ErrorIf( offset + size > resource->capacity, "GfxBufferResource: buffer overflow in buffer_write()!" );
	memory_copy( reinterpret_cast<byte *>( [metalBuffers[resource->id] contents] ) + offset, data, size );

	resource->writeRangeStart = min( resource->writeRangeStart, offset );
	resource->writeRangeEnd = max( resource->writeRangeEnd, offset + size );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Vertex Buffer

void GfxVertexBufferResource::release( GfxVertexBufferResource *&resource )
{
	if( resource == nullptr || resource->id == GFX_RESOURCE_ID_NULL ) { return; }

	resource->writer.free( true );

	vertexBufferResources.remove( resource->id );
	resource = nullptr;
}


bool CoreGfx::api_vertex_buffer_init( GfxVertexBufferResource *&resource, u32 vertexFormatID,
	GfxWriteMode writeMode, u32 capacity, u32 stride )
{
	Assert( resource == nullptr );
	resource = vertexBufferResources.make_new();
	resource->writeMode = writeMode;
	resource->mapped = false;
	resource->vertexFormat = vertexFormatID;
	resource->stride = stride;
	resource->current = 0U;

	switch( writeMode )
	{
		case GfxWriteMode_ONCE:
			resource->writer.init( resource->buffers, 1, capacity, WriteMode_PERSISTENT, false );
			resource->size = capacity * 1;
		break;

		case GfxWriteMode_OVERWRITE:
			resource->writer.init( resource->buffers, 3, capacity, WriteMode_OVERWRITE, true );
			resource->size = capacity * 3;
		break;

		case GfxWriteMode_RING:
			resource->writer.init( resource->buffers, 3, capacity, WriteMode_RING, true );
			resource->size = capacity * 3;
		break;
	}

	PROFILE_GFX( Gfx::stats.gpuMemoryVertexBuffers += resource->size );
	return true;
}


bool CoreGfx::api_vertex_buffer_free( GfxVertexBufferResource *&resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	PROFILE_GFX( Gfx::stats.gpuMemoryVertexBuffers -= resource->size );

	GfxVertexBufferResource::release( resource );

	return true;
}


void CoreGfx::api_vertex_buffer_write_begin( GfxVertexBufferResource *resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	Assert( !resource->mapped );
	resource->mapped = true;
	resource->writer.write_begin( true );
	resource->current = 0U;
}


void CoreGfx::api_vertex_buffer_write_end( GfxVertexBufferResource *resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	Assert( resource->mapped );
	resource->writer.write_end();
	resource->mapped = false;
}


void CoreGfx::api_vertex_buffer_write( GfxVertexBufferResource *resource, const void *data, u32 size )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	Assert( resource->mapped );
	resource->current += size;
	resource->writer.write( data, size );
}


u32 CoreGfx::api_vertex_buffer_current( const GfxVertexBufferResource *resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	return resource->current;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Instance Buffer

void GfxInstanceBufferResource::release( GfxInstanceBufferResource *&resource )
{
	if( resource == nullptr || resource->id == GFX_RESOURCE_ID_NULL ) { return; }

	resource->writer.free( true );

	instanceBufferResources.remove( resource->id );
	resource = nullptr;
}


bool CoreGfx::api_instance_buffer_init( GfxInstanceBufferResource *&resource, u32 instanceFormatID,
	GfxWriteMode writeMode, u32 capacity, u32 stride )
{
	Assert( resource == nullptr );
	resource = instanceBufferResources.make_new();
	resource->writeMode = writeMode;
	resource->mapped = false;
	resource->instanceFormat = instanceFormatID;
	resource->stride = stride;
	resource->current = 0U;

	switch( writeMode )
	{
		case GfxWriteMode_ONCE:
			resource->writer.init( resource->buffers, 1, capacity, WriteMode_PERSISTENT, false );
			resource->size = capacity * 1;
		break;

		case GfxWriteMode_OVERWRITE:
			resource->writer.init( resource->buffers, 3, capacity, WriteMode_OVERWRITE, true );
			resource->size = capacity * 3;
		break;

		case GfxWriteMode_RING:
			resource->writer.init( resource->buffers, 3, capacity, WriteMode_RING, true );
			resource->size = capacity * 3;
		break;
	}

	PROFILE_GFX( Gfx::stats.gpuMemoryInstanceBuffers += resource->size );
	return true;
}


bool CoreGfx::api_instance_buffer_free( GfxInstanceBufferResource *&resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	PROFILE_GFX( Gfx::stats.gpuMemoryInstanceBuffers -= resource->size );

	GfxInstanceBufferResource::release( resource );

	return true;
}


void CoreGfx::api_instance_buffer_write_begin( GfxInstanceBufferResource *resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	Assert( !resource->mapped );
	resource->mapped = true;
	resource->writer.write_begin( true );
	resource->current = 0U;
}


void CoreGfx::api_instance_buffer_write_end( GfxInstanceBufferResource *resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	Assert( resource->mapped );
	resource->writer.write_end();
	resource->mapped = false;
}


void CoreGfx::api_instance_buffer_write( GfxInstanceBufferResource *resource,
	const void *data, u32 size )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	Assert( resource->mapped );
	resource->writer.write( data, size );
	resource->current += size;
}


u32 CoreGfx::api_instance_buffer_current( const GfxInstanceBufferResource *resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	return resource->current;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Index Buffer

void GfxIndexBufferResource::release( GfxIndexBufferResource *&resource )
{
	if( resource == nullptr || resource->id == GFX_RESOURCE_ID_NULL ) { return; }

	if( mtlIndexBuffers[resource->id] != nil )
	{
		[mtlIndexBuffers[resource->id] release];
		mtlIndexBuffers[ resource->id ] = nil;
	}

	indexBufferResources.remove( resource->id );
	resource = nullptr;
}


bool CoreGfx::api_index_buffer_init( GfxIndexBufferResource *&resource,
	void *data, u32 capacity, double indicesToVerticesRatio, GfxIndexBufferFormat format, GfxWriteMode writeMode )
{
	Assert( resource == nullptr );
	Assert( format != GfxIndexBufferFormat_NONE );
	Assert( format < GFXINDEXBUFFERFORMAT_COUNT );

	resource = indexBufferResources.make_new();
	resource->writeMode = writeMode;
	resource->format = format;
	resource->indicesToVerticesRatio = indicesToVerticesRatio;
	resource->size = capacity;

	if( data != nullptr )
	{
		mtlIndexBuffers[resource->id] = [device
			newBufferWithBytes: data
			length: capacity
			options: MTLResourceStorageModeShared];
	}
	else
	{
		mtlIndexBuffers[resource->id] = [device
			newBufferWithLength: capacity
			options: MTLResourceStorageModeShared];
	}

	if( mtlIndexBuffers[resource->id] == nil )
	{
		GfxIndexBufferResource::release( resource );
		ErrorReturnMsg( false, "%s: Failed to create MTLBuffer for index buffer!", __FUNCTION__ );
	}

	PROFILE_GFX( Gfx::stats.gpuMemoryIndexBuffers += capacity );
	return true;
}


bool CoreGfx::api_index_buffer_free( GfxIndexBufferResource *&resource )
{
	Assert( resource != nullptr );

	PROFILE_GFX( Gfx::stats.gpuMemoryIndexBuffers -= resource->size );

	GfxIndexBufferResource::release( resource );
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Uniform Buffer

void GfxUniformBufferResource::release( GfxUniformBufferResource *&resource )
{
	if( resource == nullptr || resource->id == GFX_RESOURCE_ID_NULL ) { return; }

	resource->writer.free( true );

	uniformBufferResources.remove( resource->id );
	resource = nullptr;
}


bool CoreGfx::api_uniform_buffer_init( GfxUniformBufferResource *&resource, const char *name,
	int index, u32 capacity )
{
	Assert( resource == nullptr );
	resource = uniformBufferResources.make_new();
	resource->name = name;
	resource->index = index;
	resource->size = capacity;

	resource->writer.init( resource->buffers, 3, capacity, WriteMode_OVERWRITE, false );
	resource->mapped = false;

	PROFILE_GFX( Gfx::stats.gpuMemoryUniformBuffers += resource->size );
	return true;
}


bool CoreGfx::api_uniform_buffer_free( GfxUniformBufferResource *&resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	PROFILE_GFX( Gfx::stats.gpuMemoryUniformBuffers -= resource->size );

	GfxUniformBufferResource::release( resource );

	return true;
}


void CoreGfx::api_uniform_buffer_write_begin( GfxUniformBufferResource *resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	Assert( !resource->mapped );
	resource->mapped = true;
	resource->writer.write_begin( true );
}


void CoreGfx::api_uniform_buffer_write_end( GfxUniformBufferResource *resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	Assert( resource->mapped );
	resource->writer.write_end();
	resource->mapped = false;
}


void CoreGfx::api_uniform_buffer_write( GfxUniformBufferResource *resource, const void *data )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	Assert( resource->mapped );
	resource->writer.write( data, resource->size );
}


bool CoreGfx::api_uniform_buffer_bind_vertex( GfxUniformBufferResource *const resource, const int slot )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( slot >= 0 );
	Assert( mtlEncoder != nil );

	stateBoundUniformBufferResources[resource->id].resourceID = resource->id;
	stateBoundUniformBufferResources[resource->id].slotVertex = slot;

	return true;
}


bool CoreGfx::api_uniform_buffer_bind_fragment( GfxUniformBufferResource *resource, int slot )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( slot >= 0 );
	Assert( mtlEncoder != nil );

	stateBoundUniformBufferResources[resource->id].resourceID = resource->id;
	stateBoundUniformBufferResources[resource->id].slotFragment = slot;

	return true;
}


bool CoreGfx::api_uniform_buffer_bind_compute( GfxUniformBufferResource *resource, int slot )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( slot >= 0 );
	Assert( mtlEncoder != nil );

	stateBoundUniformBufferResources[resource->id].resourceID = resource->id;
	stateBoundUniformBufferResources[resource->id].slotCompute = slot;

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

	if( mtlTextures[resource->id] != nil )
	{
		[mtlTextures[resource->id] release];
		mtlTextures[resource->id] = nil;
	}

	textureResources.remove(resource->id);
	resource = nullptr;
}


bool CoreGfx::api_texture_init( GfxTextureResource *&resource, void *pixels,
	u16 width, u16 height, u16 levels, const GfxColorFormat &format )
{
	Assert( resource == nullptr );

	const usize pixelSizeBytes = CoreGfx::colorFormatPixelSizeBytes[format];
	const byte *source = reinterpret_cast<const byte *>( pixels );

	ErrorReturnIf( Gfx::mip_level_count_2d( width, height ) < levels, false,
		"%s: requested mip levels exceed max", levels );

	resource = textureResources.make_new();
	resource->type = GfxTextureType_2D;
	resource->colorFormat = format;
	resource->width = width;
	resource->height = height;
	resource->depth = 1;
	resource->levels = levels;

	MTLTextureDescriptor *desc = [MTLTextureDescriptor
		texture2DDescriptorWithPixelFormat: MetalColorFormats[format]
		width: width
		height: height
		mipmapped: ( levels > 1 )];

	desc.usage = MTLTextureUsageShaderRead;
	desc.storageMode = MTLStorageModeShared;
	desc.cpuCacheMode = MTLCPUCacheModeDefaultCache;

	mtlTextures[resource->id] = [device newTextureWithDescriptor: desc];
	ErrorReturnIf( mtlTextures[resource->id] == nil, false,
		"%s: Failed to create MTLTexture", __FUNCTION__ );

	// Upload data
	id<MTLCommandBuffer> cmd = [mtlCommandQueue commandBuffer];
	id<MTLBlitCommandEncoder> blit = [cmd blitCommandEncoder];
	u16 mipWidth = width;
	u16 mipHeight = height;
	const byte *src = source;

	for( u16 level = 0; level < levels; level++ )
	{
		MTLRegion region = MTLRegionMake2D( 0, 0, mipWidth, mipHeight );

		[mtlTextures[resource->id]
			replaceRegion: region
			mipmapLevel: level
			withBytes: src
			bytesPerRow: mipWidth * pixelSizeBytes];

		const usize mipLevelSizeBytes = mipWidth * mipHeight * pixelSizeBytes;
		src += mipLevelSizeBytes;
		resource->size += mipLevelSizeBytes;
		mipWidth = ( mipWidth > 1 ) ? ( mipWidth >> 1 ) : 1;
		mipHeight = ( mipHeight > 1 ) ? ( mipHeight >> 1 ) : 1;
	}

	[blit endEncoding];
	[cmd commit];
	[cmd waitUntilCompleted];

	PROFILE_GFX( Gfx::stats.gpuMemoryTextures += resource->size );
	return true;
}


bool CoreGfx::api_texture_free( GfxTextureResource *&resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	PROFILE_GFX( Gfx::stats.gpuMemoryTextures -= resource->size );

	GfxTextureResource::release( resource );

	return true;
}


bool CoreGfx::api_texture_bind( GfxTextureResource *resource, int slot )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( slot >= 0 && slot < GFX_TEXTURE_SLOT_COUNT );

	if( stateBoundTextureResources[slot] == resource ) { return true; }

	stateBoundTextureResources[slot] = resource;
	PROFILE_GFX( Gfx::stats.frame.textureBinds++ );

	if( mtlEncoder != nil )
	{
		[mtlEncoder setVertexTexture: mtlTextures[resource->id] atIndex: slot];
		[mtlEncoder setFragmentTexture: mtlTextures[resource->id] atIndex: slot];
	}

	return true;
}


bool CoreGfx::api_texture_release( GfxTextureResource *resource, int slot )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( slot >= 0 && slot < GFX_TEXTURE_SLOT_COUNT );

	Assert( stateBoundTextureResources[slot] == resource );
	stateBoundTextureResources[slot] = nullptr;

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Render Target

void GfxRenderTargetResource::release( GfxRenderTargetResource *&resource )
{
	if( resource == nullptr || resource->id == GFX_RESOURCE_ID_NULL ) { return; }

	if( resource->textureColorSS != nullptr ) { GfxTextureResource::release( resource->textureColorSS ); }
	if( resource->textureColorMS != nullptr ) { GfxTextureResource::release( resource->textureColorMS ); }
	if( resource->textureDepthSS != nullptr ) { GfxTextureResource::release( resource->textureDepthSS ); }
	if( resource->textureDepthMS != nullptr ) { GfxTextureResource::release( resource->textureDepthMS ); }

	renderTargetResources.remove( resource->id );
	resource = nullptr;
}


bool CoreGfx::api_render_target_init( GfxRenderTargetResource *&resource,
	GfxTextureResource *&resourceColor, GfxTextureResource *&resourceDepth,
	u16 width, u16 height, const GfxRenderTargetDescription &desc )
{
	Assert( resource == nullptr );
	Assert( resourceColor == nullptr );
	Assert( resourceDepth == nullptr );

	resource = renderTargetResources.make_new();
	resource->width = width;
	resource->height = height;
	resource->desc = desc;

	if( desc.colorFormat != GfxColorFormat_NONE )
	{
		Assert( resource->textureColorSS == nullptr );
		resource->textureColorSS = textureResources.make_new();
		resource->textureColorSS->type = GfxTextureType_2D;
		resourceColor = resource->textureColorSS;

		MTLTextureDescriptor *colorDescriptorSS = [[MTLTextureDescriptor alloc] init];
		colorDescriptorSS.textureType = MTLTextureType2D;
		colorDescriptorSS.pixelFormat = MetalColorFormats[desc.colorFormat];
		colorDescriptorSS.width = width;
		colorDescriptorSS.height = height;
		colorDescriptorSS.sampleCount = 1;
		colorDescriptorSS.storageMode = desc.cpuAccess ? MTLStorageModeShared : MTLStorageModePrivate;
		colorDescriptorSS.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;

		mtlTextures[resource->textureColorSS->id] = [device newTextureWithDescriptor: colorDescriptorSS];

		ErrorReturnIf( mtlTextures[resource->textureColorSS->id] == nil, false,
			"%s: Failed to create color MTLTexture for render target!", __FUNCTION__);

		resource->size += GFX_SIZE_IMAGE_COLOR_BYTES( width, height, 1, desc.colorFormat );

		if( desc.sampleCount > 1 )
		{
			Assert( resource->textureColorMS == nullptr );
			resource->textureColorMS = textureResources.make_new();
			resource->textureColorMS->type = GfxTextureType_2D;

			MTLTextureDescriptor *colorDescriptorMS = [[MTLTextureDescriptor alloc] init];
			colorDescriptorMS.textureType = MTLTextureType2DMultisample;
			colorDescriptorMS.pixelFormat = MetalColorFormats[desc.colorFormat];
			colorDescriptorMS.width = width;
			colorDescriptorMS.height = height;
			colorDescriptorMS.sampleCount = desc.sampleCount;
			colorDescriptorMS.storageMode = desc.cpuAccess ? MTLStorageModeShared : MTLStorageModePrivate;
			colorDescriptorMS.usage = MTLTextureUsageRenderTarget;

			mtlTextures[resource->textureColorMS->id] = [device newTextureWithDescriptor: colorDescriptorMS];

			ErrorReturnIf( mtlTextures[resource->textureColorMS->id] == nil, false,
				"%s: Failed to create multisample color MTLTexture for render target!", __FUNCTION__);

			resource->size += GFX_SIZE_IMAGE_COLOR_BYTES( width, height, 1, desc.colorFormat ) * desc.sampleCount;
		}
	}

	if( desc.depthFormat != GfxDepthFormat_NONE )
	{
		Assert( resource->textureDepthSS == nullptr );
		resource->textureDepthSS = textureResources.make_new();
		resource->textureDepthSS->type = GfxTextureType_2D;
		resourceDepth = resource->textureDepthSS;

		MTLTextureDescriptor *depthDescriptorSS = [[MTLTextureDescriptor alloc] init];
		depthDescriptorSS.textureType = MTLTextureType2D;
		depthDescriptorSS.pixelFormat = MetalDepthStencilFormats[desc.depthFormat];
		depthDescriptorSS.width = width;
		depthDescriptorSS.height = height;
		depthDescriptorSS.sampleCount = 1;
		depthDescriptorSS.storageMode = desc.cpuAccess ? MTLStorageModeShared : MTLStorageModePrivate;
		depthDescriptorSS.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;

		mtlTextures[resource->textureDepthSS->id] = [device newTextureWithDescriptor: depthDescriptorSS];

		ErrorReturnIf( mtlTextures[resource->textureDepthSS->id] == nil, false,
			"%s: Failed to create depth MTLTexture for render target!", __FUNCTION__ );

		resource->size += GFX_SIZE_IMAGE_DEPTH_BYTES( width, height, 1, desc.depthFormat );

		if( desc.sampleCount > 1 )
		{
			Assert( resource->textureDepthMS == nullptr );
			resource->textureDepthMS = textureResources.make_new();
			resource->textureDepthMS->type = GfxTextureType_2D;

			MTLTextureDescriptor *depthDescriptorMS = [[MTLTextureDescriptor alloc] init];
			depthDescriptorMS.textureType = MTLTextureType2DMultisample;
			depthDescriptorMS.pixelFormat = MetalDepthStencilFormats[desc.depthFormat];
			depthDescriptorMS.width = width;
			depthDescriptorMS.height = height;
			depthDescriptorMS.sampleCount = desc.sampleCount;
			depthDescriptorMS.storageMode = desc.cpuAccess ? MTLStorageModeShared : MTLStorageModePrivate;
			depthDescriptorMS.usage = MTLTextureUsageRenderTarget;

			mtlTextures[resource->textureDepthMS->id] = [device newTextureWithDescriptor: depthDescriptorMS];

			ErrorReturnIf( mtlTextures[resource->textureDepthMS->id] == nil, false,
				"%s: Failed to create multisample depth MTLTexture for render target!", __FUNCTION__ );

			resource->size += GFX_SIZE_IMAGE_DEPTH_BYTES( width, height, 1, desc.depthFormat ) * desc.sampleCount;
		}
	}

	PROFILE_GFX( Gfx::stats.gpuMemoryRenderTargets += resource->size );
	return true;
}


bool CoreGfx::api_render_target_free( GfxRenderTargetResource *&resource,
	GfxTextureResource *&resourceColor,
	GfxTextureResource *&resourceDepth )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	if( resource->desc.colorFormat != GfxColorFormat_NONE )
	{
		Assert( resourceColor != nullptr && resourceColor->id != GFX_RESOURCE_ID_NULL );
		GfxTextureResource::release( resourceColor );
	}

	if( resource->desc.depthFormat != GfxDepthFormat_NONE )
	{
		Assert( resourceDepth != nullptr && resourceDepth->id != GFX_RESOURCE_ID_NULL );
		GfxTextureResource::release( resourceDepth );
	}

	PROFILE_GFX( Gfx::stats.gpuMemoryRenderTargets -= resource->size );
	GfxRenderTargetResource::release( resource );

	return true;
}


bool CoreGfx::api_render_target_copy_color(
	GfxRenderTargetResource *&srcResource, GfxTextureResource *&srcResourceColor,
	GfxRenderTargetResource *&dstResource, GfxTextureResource *&dstResourceColor )
{
	if( !srcResource || !dstResource ) { return false; }

	id<MTLCommandBuffer> cmd = [mtlCommandQueue commandBuffer];
	id<MTLBlitCommandEncoder> blit = [cmd blitCommandEncoder];

	if( srcResource->textureColorSS && dstResource->textureColorSS )
	{
		[blit
			copyFromTexture: mtlTextures[srcResource->textureColorSS->id]
			sourceSlice: 0
			sourceLevel: 0
			sourceOrigin: MTLOriginMake( 0, 0, 0 )
			sourceSize: MTLSizeMake( srcResource->width, srcResource->height, 1 )
			toTexture: mtlTextures[dstResource->textureColorSS->id]
			destinationSlice: 0
			destinationLevel: 0
			destinationOrigin: MTLOriginMake( 0, 0, 0 )];
	}

	[blit endEncoding];
	[cmd commit];
	[cmd waitUntilCompleted];

	return true;
}


bool CoreGfx::api_render_target_copy_depth(
	GfxRenderTargetResource *&srcResource, GfxTextureResource *&srcResourceDepth,
	GfxRenderTargetResource *&dstResource, GfxTextureResource *&dstResourceDepth )
{
	if( !srcResource || !dstResource ) { return false; }

	id<MTLCommandBuffer> cmd = [mtlCommandQueue commandBuffer];
	id<MTLBlitCommandEncoder> blit = [cmd blitCommandEncoder];

#if 0
	if( srcResource->textureDepthSS && dstResource->textureDepthSS )
	{
		// TODO: Evaluate this -- Metal doesn't like copying depth
		[blit
			copyFromTexture: mtlTextures[srcResource->textureDepthSS->id]
			sourceSlice: 0
			sourceLevel: 0
			sourceOrigin: MTLOriginMake( 0, 0, 0 )
			sourceSize: MTLSizeMake( srcResource->width, srcResource->height, 1 )
			toTexture: mtlTextures[dstResource->textureDepthSS->id]
			destinationSlice: 0
			destinationLevel: 0
			destinationOrigin: MTLOriginMake( 0, 0, 0 )];
	}
#endif

	[blit endEncoding];
	[cmd commit];
	[cmd waitUntilCompleted];

	return true;
}


bool CoreGfx::api_render_target_copy(
	GfxRenderTargetResource *&srcResource,
	GfxTextureResource *&srcResourceColor, GfxTextureResource *&srcResourceDepth,
	GfxRenderTargetResource *&dstResource,
	GfxTextureResource *&dstResourceColor, GfxTextureResource *&dstResourceDepth )
{
	if( !srcResource || !dstResource ) { return false; }

	id<MTLCommandBuffer> cmd = [mtlCommandQueue commandBuffer];
	id<MTLBlitCommandEncoder> blit = [cmd blitCommandEncoder];

	if( srcResource->textureColorSS && dstResource->textureColorSS )
	{
		[blit
			copyFromTexture: mtlTextures[srcResource->textureColorSS->id]
			sourceSlice: 0
			sourceLevel: 0
			sourceOrigin: MTLOriginMake( 0, 0, 0 )
			sourceSize: MTLSizeMake( srcResource->width, srcResource->height, 1 )
			toTexture: mtlTextures[dstResource->textureColorSS->id]
			destinationSlice: 0
			destinationLevel: 0
			destinationOrigin: MTLOriginMake( 0, 0, 0 )];
	}

#if 0
	if( srcResource->textureDepthSS && dstResource->textureDepthSS )
	{
		// TODO: Evaluate this -- Metal doesn't like copying depth
		[blit
			copyFromTexture: mtlTextures[srcResource->textureDepthSS->id]
			sourceSlice: 0
			sourceLevel: 0
			sourceOrigin: MTLOriginMake( 0, 0, 0 )
			sourceSize: MTLSizeMake( srcResource->width, srcResource->height, 1 )
			toTexture: mtlTextures[dstResource->textureDepthSS->id]
			destinationSlice: 0
			destinationLevel: 0
			destinationOrigin: MTLOriginMake( 0, 0, 0 )];
	}
#endif

	[blit endEncoding];
	[cmd commit];
	[cmd waitUntilCompleted];

	return true;
}


bool CoreGfx::api_render_target_copy_part(
	GfxRenderTargetResource *&srcResource,
	GfxTextureResource *&srcResourceColor, GfxTextureResource *&srcResourceDepth,
	GfxRenderTargetResource *&dstResource,
	GfxTextureResource *&dstResourceColor, GfxTextureResource *&dstResourceDepth,
	u16 srcX, u16 srcY, u16 dstX, u16 dstY, u16 width, u16 height )
{
	if( srcResource == nullptr || dstResource == nullptr ) { return false; }

	MTLOrigin srcOrigin = { srcX, srcY, 0 };
	MTLOrigin dstOrigin = { dstX, dstY, 0 };
	MTLSize copySize = { width, height, 1 };

	id<MTLBlitCommandEncoder> blitEncoder = [mtlCommandBuffer blitCommandEncoder];

	[blitEncoder copyFromTexture: mtlTextures[srcResourceColor->id]
		sourceSlice: 0
		sourceLevel: 0
		sourceOrigin: srcOrigin
		sourceSize: copySize
		toTexture: mtlTextures[dstResourceColor->id]
		destinationSlice: 0
		destinationLevel: 0
		destinationOrigin: dstOrigin];

	[blitEncoder endEncoding];
	return true;
}


bool CoreGfx::api_render_target_buffer_read_color( GfxRenderTargetResource *&resource,
		GfxTextureResource *&resourceColor,
		void *buffer, const u32 size )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( resourceColor != nullptr && resourceColor->id != GFX_RESOURCE_ID_NULL );

	ErrorIf( !resource->desc.cpuAccess,
		"Trying to CPU access a render target that does not have CPU access flag!");

	const NSUInteger bytesPerRow = resource->width *
		CoreGfx::colorFormatPixelSizeBytes[resource->desc.colorFormat];
	Assert( size >= bytesPerRow * resource->height );

	MTLRegion region =
	{
		{ 0, 0, 0 },
		{ resource->width, resource->height, 1 }
	};

	[mtlTextures[resourceColor->id]
		getBytes: buffer
		bytesPerRow: bytesPerRow
		fromRegion: region
		mipmapLevel: 0];

	return true;
}


bool CoreGfx::api_render_target_buffer_read_color_async_request( GfxRenderTargetResource *&resource,
	GfxTextureResource *&resourceColor,
	void *buffer, u32 size )
{
	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this
	return true;
}


bool CoreGfx::api_render_target_buffer_read_color_async_poll( GfxRenderTargetResource *&resource,
	GfxTextureResource *&resourceColor,
	void *buffer, u32 size )
{
	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this
	return true;
}


bool api_render_target_buffer_read_depth( GfxRenderTargetResource *&resource,
	GfxTextureResource *&resourceDepth,
	void *buffer, u32 size )
{
	GRAPHICS_API_IMPLEMENTATION_WARNING;
	return true;
}


bool CoreGfx::api_render_target_buffer_read_depth_async_request( GfxRenderTargetResource *&resource,
	GfxTextureResource *&resourceDepth,
	void *buffer, u32 size )
{
	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this
	return true;
}


bool CoreGfx::api_render_target_buffer_read_depth_async_poll( GfxRenderTargetResource *&resource,
	GfxTextureResource *&resourceDepth,
	void *buffer, u32 size )
{
	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this
	return true;
}


bool CoreGfx::api_render_target_buffer_write_color( GfxRenderTargetResource *&resource,
	GfxTextureResource *&resourceColor,
	const void *buffer, u32 size )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( resourceColor != nullptr && resourceColor->id != GFX_RESOURCE_ID_NULL );

	ErrorIf( !resource->desc.cpuAccess,
		"Trying to CPU access a render target that does not have CPU access flag!" );

	const NSUInteger bytesPerRow = resource->width *
		CoreGfx::colorFormatPixelSizeBytes[resource->desc.colorFormat];
	Assert( size >= bytesPerRow * resource->height );

	MTLRegion region =
	{
		{ 0, 0, 0 },
		{ resource->width, resource->height, 1 }
	};

	[mtlTextures[resourceColor->id]
		replaceRegion: region
		mipmapLevel: 0
		withBytes: buffer
		bytesPerRow: bytesPerRow];

	return true;
}


bool CoreGfx::api_render_target_buffer_write_depth( GfxRenderTargetResource *&resource,
	GfxTextureResource *&resourceDepth,
	const void *buffer, u32 size )
{
	GRAPHICS_API_IMPLEMENTATION_WARNING;
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Draw

static void metal_draw( NSUInteger vertexCount, NSUInteger vertexStartLocation,
	GfxPrimitiveType primitiveType )
{
	[mtlEncoder drawPrimitives: MetalPrimitiveTypes[primitiveType]
		vertexStart: vertexStartLocation
		vertexCount: vertexCount];

	PROFILE_GFX( Gfx::stats.frame.drawCalls++ );
	PROFILE_GFX( Gfx::stats.frame.vertexCount += vertexCount );
}


static void metal_draw_instanced( NSUInteger vertexCount, NSUInteger instanceCount,
	NSUInteger vertexStartLocation, NSUInteger instanceStartLocation,
	GfxPrimitiveType primitiveType)
{
	[mtlEncoder drawPrimitives: MetalPrimitiveTypes[primitiveType]
		vertexStart: vertexStartLocation
		vertexCount: vertexCount
		instanceCount: instanceCount
		baseInstance: instanceStartLocation];

	PROFILE_GFX( Gfx::stats.frame.drawCalls++ );
	PROFILE_GFX( Gfx::stats.frame.vertexCount += vertexCount * instanceCount );
}


static void metal_draw_indexed( NSUInteger indexCount, GfxIndexBufferFormat indexFormat,
	id<MTLBuffer> indexBuffer, NSUInteger indexBufferOffset, NSUInteger indexStartLocation,
	GfxPrimitiveType primitiveType )
{
	NSUInteger indexStrides[] =
	{
		4, // GfxIndexBufferFormat_NONE
		2, // GfxIndexBufferFormat_U16
		4, // GfxIndexBufferFormat_U32
	};

	const NSUInteger offsetBytes = indexBufferOffset + indexStartLocation * indexStrides[indexFormat];

	[mtlEncoder drawIndexedPrimitives: MetalPrimitiveTypes[primitiveType]
		indexCount: indexCount
		indexType: MetalIndexBufferFormats[indexFormat]
		indexBuffer: indexBuffer
		indexBufferOffset: offsetBytes];

	PROFILE_GFX( Gfx::stats.frame.drawCalls++ );
	PROFILE_GFX( Gfx::stats.frame.vertexCount += indexCount );
}


static void metal_draw_instanced_indexed( NSUInteger indexCount, GfxIndexBufferFormat indexFormat,
	id<MTLBuffer> indexBuffer, NSUInteger indexBufferOffset, NSUInteger indexStartLocation,
	NSUInteger instanceCount, NSUInteger instanceStartLocation,
	GfxPrimitiveType primitiveType )
{
	NSUInteger indexStrides[] =
	{
		4, // GfxIndexBufferFormat_NONE
		2, // GfxIndexBufferFormat_U16
		4, // GfxIndexBufferFormat_U32
	};

	const NSUInteger offsetBytes = indexBufferOffset + indexStartLocation * indexStrides[indexFormat];

	[mtlEncoder drawIndexedPrimitives: MetalPrimitiveTypes[primitiveType]
		indexCount: indexCount
		indexType: MetalIndexBufferFormats[indexFormat]
		indexBuffer: indexBuffer
		indexBufferOffset: offsetBytes
		instanceCount: instanceCount
		baseVertex: 0
		baseInstance: instanceStartLocation];

	PROFILE_GFX( Gfx::stats.frame.drawCalls++ );
	PROFILE_GFX( Gfx::stats.frame.vertexCount += indexCount * instanceCount );
}


bool CoreGfx::api_draw(
	const GfxVertexBufferResource *resourceVertex, u32 vertexCount,
	const GfxInstanceBufferResource *resourceInstance, u32 instanceCount,
	const GfxIndexBufferResource *resourceIndex,
	GfxPrimitiveType type )
{
	Assert( resourceVertex == nullptr || resourceVertex->id != GFX_RESOURCE_ID_NULL );
	Assert( resourceInstance == nullptr || resourceInstance->id != GFX_RESOURCE_ID_NULL );
	Assert( resourceIndex == nullptr || resourceIndex->id != GFX_RESOURCE_ID_NULL );

	const GfxShaderResource *const shader = CoreGfx::state.pipeline.shader;

	AssertMsg( resourceVertex == nullptr ||
		CoreGfx::shaderEntries[shader->shaderID].vertexFormat == resourceVertex->vertexFormat,
		"Vertex format mismatch with shader!" );
	AssertMsg( resourceInstance == nullptr ||
		CoreGfx::shaderEntries[shader->shaderID].instanceFormat == resourceInstance->instanceFormat,
		"Instance format mismatch with shader!" );

	ErrorIf( resourceVertex != nullptr && resourceVertex->mapped, "Attempting to draw mapped vertex buffer!" );
	ErrorIf( resourceInstance != nullptr && resourceInstance->mapped, "Attempting to draw mapped instance buffer!" );

	if( resourceVertex != nullptr )
	{
		auto vbCommit =
			const_cast<GfxVertexBufferResource *>( resourceVertex )->writer.commit( commitIDCurrent, true );

		Assert( metalBuffers[vbCommit.resource->resource->id] != nil );
		[mtlEncoder setVertexBuffer: metalBuffers[vbCommit.resource->resource->id]
			offset: vbCommit.offset
			atIndex: 0];
	}

	if( resourceInstance != nullptr )
	{
		auto ibCommit =
			const_cast<GfxInstanceBufferResource *>( resourceInstance )->writer.commit( commitIDCurrent, true );

		Assert( metalBuffers[ibCommit.resource->resource->id] != nil );
		[mtlEncoder setVertexBuffer: metalBuffers[ibCommit.resource->resource->id]
			offset: ibCommit.offset
			atIndex: 1];
	}

	for( int i = 0; i < GFX_RESOURCE_COUNT_UNIFORM_BUFFER; i++ )
	{
		if( stateBoundUniformBufferResources[i].resourceID == GFX_RESOURCE_ID_NULL ) { continue; }

		GfxUniformBufferResource &resource = uniformBufferResources.get( stateBoundUniformBufferResources[i].resourceID );
		Assert( resource.id == stateBoundUniformBufferResources[i].resourceID );

		auto ubCommit = resource.writer.commit( commitIDCurrent, true );
		Assert( metalBuffers[ubCommit.resource->resource->id] != nil );

		const int slotVertex = stateBoundUniformBufferResources[i].slotVertex;
		if( slotVertex >= 0 )
		{
			[mtlEncoder setVertexBuffer: metalBuffers[ubCommit.resource->resource->id]
				offset: ubCommit.offset
				atIndex: static_cast<NSUInteger>( 2 + slotVertex )];
		}

		const int slotFragment = stateBoundUniformBufferResources[i].slotFragment;
		if( slotFragment >= 0 )
		{
			[mtlEncoder setFragmentBuffer: metalBuffers[ubCommit.resource->resource->id]
				offset: ubCommit.offset
				atIndex: static_cast<NSUInteger>( 2 + slotFragment )];
		}

		stateBoundUniformBufferResources[i] = UniformBufferBinding { };
	}

	if( resourceIndex == nullptr )
	{
		if( instanceCount > 0 )
		{
			metal_draw_instanced( static_cast<NSUInteger>( vertexCount ),
				static_cast<NSUInteger>( instanceCount ), 0, 0, type );
		}
		else
		{
			metal_draw( static_cast<NSUInteger>( vertexCount ), 0, type );
		}
	}
	else
	{
		Assert( resourceIndex->format < GFXINDEXBUFFERFORMAT_COUNT );
		Assert( mtlIndexBuffers[resourceIndex->id] != nil );

		const NSUInteger count = static_cast<NSUInteger>( vertexCount * resourceIndex->indicesToVerticesRatio );

		if( instanceCount > 0 )
		{
			metal_draw_instanced_indexed( count, resourceIndex->format, mtlIndexBuffers[resourceIndex->id],
				0, 0, static_cast<NSUInteger>( instanceCount ), 0, type );
		}
		else
		{
			metal_draw_indexed( count, resourceIndex->format, mtlIndexBuffers[resourceIndex->id],
				0, 0, type );
		}
	}

	PROFILE_GFX( Gfx::stats.frame.drawCalls++ );
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CoreGfx::api_dispatch( u32 x, u32 y, u32 z )
{
	// TODO
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Render Command

void CoreGfx::api_render_command_execute( const GfxRenderCommand &command )
{
	render_pass_validate();

	metal_prepare_encoder( command.pipeline );

	ErrorIf( !CoreGfx::shader_bind_uniform_buffers_vertex( command.pipeline.shader->shaderID ),
		"Failed to bind vertex shader uniform buffers! (%u)", command.pipeline.shader->shaderID );
	ErrorIf( !CoreGfx::shader_bind_uniform_buffers_fragment( command.pipeline.shader->shaderID ),
		"Failed to bind fragment shader uniform buffers! (%u)", command.pipeline.shader->shaderID );
	ErrorIf( !CoreGfx::shader_bind_uniform_buffers_compute( command.pipeline.shader->shaderID ),
		"Failed to bind compute shader uniform buffers! (%u)", command.pipeline.shader->shaderID );

	if( command.workFunctionInvoker )
	{
		command.workFunctionInvoker( command.workFunction,
			GfxRenderCommand::renderCommandArgsStack.get( command.workPayloadOffset, command.workPayloadSize ) );
	}
}


void CoreGfx::api_render_command_execute_post( const GfxRenderCommand &command )
{
	if( mtlEncoder != nil )
	{
		metal_encoder_end();
		metal_command_buffer_commit();
		metal_command_buffer_begin();
	}
}


void CoreGfx::api_render_pass_begin( const GfxRenderPass &pass )
{
	bind_targets( pass.targets );
}


void CoreGfx::api_render_pass_end( const GfxRenderPass &pass )
{
	// NOTE: Do nothing here, since subsequent api_render_pass_begin() will rebind targets
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////