#pragma once

#include <gfx.generated.hpp>

#include <core/list.hpp>
#include <core/debug.hpp>
#include <core/types.hpp>
#include <core/color.hpp>

#include <vendor/vendor.hpp>

#include <manta/assets.hpp>

#include <config.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if !defined( HEADLESS ) && ( GRAPHICS_D3D11 || GRAPHICS_D3D12 || GRAPHICS_METAL || GRAPHICS_OPENGL /*|| GRAPHICS_VULKAN*/ )
	#define GRAPHICS_ENABLED ( true )
#else
	#define GRAPHICS_ENABLED ( false )
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define GRAPHICS_API_IMPLEMENTATION_WARNING \
	Warning( "Using '%s' -- not implemented on " GRAPHICS_API_NAME, __FUNCTION__ )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// 'CoreGfx' = Engine/Backend Graphics (Internal)
// 'Gfx'  = Graphics API (Public)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define GFX_TEXTURE_SLOT_COUNT ( 8 )
#define GFX_TEXTURE_ARRAY_LENGTH_MAX ( 256 )
#define GFX_RENDER_TARGET_SLOT_COUNT ( 8 )
#define GFX_MIP_DEPTH_MAX ( 16 )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// GfxWork: lambda with no captures
// Used with: GfxRenderCommand::work(...)
#define GfxWork +[]()

// GfxWorkCapture: lambda with automatic scope captures
// Used with: Gfx::render_command_execute( cmd, work )
#define GfxWorkCapture [=]()

// GfxWorkVariable: lambda with single variable payload
// Used with: Gfx::render_command_execute( cmd, work )
#define GfxWorkVariable(variable) variable, +[]( decltype( variable ) &variable )

// GfxWorkStructure: lambda with structured payload and name
// Used with: Gfx::render_command_execute( cmd, work )
#define GfxWorkStructure(structure,name) structure, +[]( decltype( structure ) &name )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct GfxStatisticsFrame
{
	u32 drawCalls = 0;
	u32 vertexCount = 0;
	u32 bufferMaps = 0;
	u32 textureBinds = 0;
	u32 shaderBinds = 0;
};

struct GfxStatistics
{
	GfxStatisticsFrame frame = { };

	usize gpuMemorySwapchain = 0;
	usize gpuMemoryShaders = 0;
	usize gpuMemoryVertexBuffers = 0;
	usize gpuMemoryInstanceBuffers = 0;
	usize gpuMemoryIndexBuffers = 0;
	usize gpuMemoryUniformBuffers = 0;
	usize gpuMemoryConstantBuffers = 0;
	usize gpuMemoryMutableBuffers = 0;
	usize gpuMemoryTextures = 0;
	usize gpuMemoryRenderTargets = 0;

	usize total_memory() const
	{
		return
			gpuMemorySwapchain +
			gpuMemoryShaders +
			gpuMemoryVertexBuffers +
			gpuMemoryInstanceBuffers +
			gpuMemoryIndexBuffers +
			gpuMemoryUniformBuffers +
			gpuMemoryConstantBuffers +
			gpuMemoryMutableBuffers +
			gpuMemoryTextures +
			gpuMemoryRenderTargets;
	}
};

#define PROFILING_GFX ( true && COMPILE_DEBUG )

#if PROFILING_GFX
	#define PROFILE_GFX(expr) expr

	namespace Gfx
	{
		extern GfxStatistics stats;
		extern GfxStatistics statsPrevious;
	}
#else
	#define PROFILE_GFX(expr)
#endif

extern void debug_overlay_gfx( const float x, const float y );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define GFX_RESOURCE_COUNT_SHADER ( 256 * 256 )
#define GFX_RESOURCE_COUNT_BUFFER ( 256 * 256 )
#define GFX_RESOURCE_COUNT_MANAGED_BUFFER ( 256 * 256 )
#define GFX_RESOURCE_COUNT_VERTEX_BUFFER ( 256 * 256 )
#define GFX_RESOURCE_COUNT_INSTANCE_BUFFER ( 256 * 256 )
#define GFX_RESOURCE_COUNT_INDEX_BUFFER ( 256 * 256 )
#define GFX_RESOURCE_COUNT_UNIFORM_BUFFER ( 256 * 256 )
#define GFX_RESOURCE_COUNT_CONSTANT_BUFFER ( 256 * 256 )
#define GFX_RESOURCE_COUNT_MUTABLE_BUFFER ( 256 * 256 )
#define GFX_RESOURCE_COUNT_TEXTURE ( 1024 )
#define GFX_RESOURCE_COUNT_RENDER_TARGET ( 1024 )

#define GFX_MANAGED_BUFFER_POOL_MAX ( 8 )

using GfxResourceID = u32;
#define GFX_RESOURCE_ID_NULL ( U32_MAX )
struct GfxResource { GfxResourceID id = GFX_RESOURCE_ID_NULL; };

struct GfxShaderResource;
struct GfxBufferResource;
struct GfxVertexBufferResource;
struct GfxInstanceBufferResource;
struct GfxIndexBufferResource;
struct GfxUniformBufferResource;
struct GfxConstantBufferResource;
struct GfxMutableBufferResource;
struct GfxTextureResource;
struct GfxRenderTargetResource;

class GfxShader;
class GfxTexture;


namespace CoreGfx
{
	struct UniformBufferInitEntry { const char *name; const u32 size; };
	struct UniformBufferBindEntry { const u32 id; const u32 slot; };
	struct UniformBufferBindTable { const UniformBufferBindEntry *buffers; const u32 count; };

	// Implemented in gfx.cpp
	extern GfxTexture textures[CoreAssets::textureCount];

	// Implemented in gfx.generated.cpp
	extern GfxShader shaders[CoreGfx::shaderCount];
	extern GfxUniformBufferResource *uniformBuffers[CoreGfx::uniformBufferCount];
	extern const UniformBufferInitEntry uniformBufferInitEntries[CoreGfx::uniformBufferCount];
	extern const UniformBufferBindTable uniformBufferBindTableShaderVertex[CoreGfx::shaderCount];
	extern const UniformBufferBindTable uniformBufferBindTableShaderFragment[CoreGfx::shaderCount];
	extern const UniformBufferBindTable uniformBufferBindTableShaderCompute[CoreGfx::shaderCount];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum_type( GfxColorFormat, u8 )
{
	GfxColorFormat_NONE = 0,
	GfxColorFormat_R8G8B8A8_FLOAT,
	GfxColorFormat_R8G8B8A8_UINT,
	GfxColorFormat_R10G10B10A2_FLOAT,
	GfxColorFormat_R8_UINT,
	GfxColorFormat_R8G8,
	GfxColorFormat_R16_UINT,
	GfxColorFormat_R16_FLOAT,
	GfxColorFormat_R16G16,
	GfxColorFormat_R16G16F_FLOAT,
	GfxColorFormat_R16G16B16A16_FLOAT,
	GfxColorFormat_R16G16B16A16_UINT,
	GfxColorFormat_R32_FLOAT,
	GfxColorFormat_R32G32_FLOAT,
	GfxColorFormat_R32G32B32A32_FLOAT,
	GfxColorFormat_R32G32B32A32_UINT,
	GFXCOLORFORMAT_COUNT,
};


namespace CoreGfx
{
	constexpr u32 colorFormatPixelSizeBytes[GFXCOLORFORMAT_COUNT] =
	{
		0,  // GfxColorFormat_NONE
		4,  // GfxColorFormat_R8G8B8A8_FLOAT
		4,  // GfxColorFormat_R8G8B8A8_UINT
		4,  // GfxColorFormat_R10G10B10A2_FLOAT
		1,  // GfxColorFormat_R8_UINT
		2,  // GfxColorFormat_R8G8
		2,  // GfxColorFormat_R16_UINT
		2,  // GfxColorFormat_R16_FLOAT
		4,  // GfxColorFormat_R16G16
		4,  // GfxColorFormat_R16G16F_FLOAT
		8,  // GfxColorFormat_R16G16B16A16_FLOAT
		8,  // GfxColorFormat_R16G16B16A16_UINT
		4,  // GfxColorFormat_R32_FLOAT
		8,  // GfxColorFormat_R32G32_FLOAT
		16, // GfxColorFormat_R32G32B32A32_FLOAT
		16, // GfxColorFormat_R32G32B32A32_UINT
	};
	static_assert( ARRAY_LENGTH( colorFormatPixelSizeBytes ) == GFXCOLORFORMAT_COUNT, "Missing colorFormatPixelSizeBytes!" );

	constexpr const char *colorFormatName[GFXCOLORFORMAT_COUNT] =
	{
		"NONE",               // GfxColorFormat_NONE
		"R8G8B8A8_FLOAT",     // GfxColorFormat_R8G8B8A8_FLOAT
		"R8G8B8A8_UINT",      // GfxColorFormat_R8G8B8A8_UINT
		"R10G10B10A2_FLOAT",  // GfxColorFormat_R10G10B10A2_FLOAT
		"R8_UINT",            // GfxColorFormat_R8_UINT
		"R8G8",               // GfxColorFormat_R8G8
		"R16_UINT",           // GfxColorFormat_R16_UINT
		"R16_FLOAT",          // GfxColorFormat_R16_FLOAT
		"R16G16",             // GfxColorFormat_R16G16
		"R16G16_FLOAT",       // GfxColorFormat_R16G16F_FLOAT
		"R16B16G16A16_FLOAT", // GfxColorFormat_R16G16B16A16_FLOAT
		"R16B16G16A16_UINT",  // GfxColorFormat_R16G16B16A16_UINT
		"R32_FLOAT",          // GfxColorFormat_R32_FLOAT
		"R32G32_FLOAT",       // GfxColorFormat_R32G32_FLOAT
		"R32G32B32A32_FLOAT", // GfxColorFormat_R32G32B32A32_FLOAT
		"R32G32B32A32_UINT",  // GfxColorFormat_R32G32B32A32_UINT
	};
	static_assert( ARRAY_LENGTH( colorFormatName ) == GFXCOLORFORMAT_COUNT, "Missing colorFormatName!" );
}


#define GFX_SIZE_IMAGE_COLOR_BYTES( width, height, depth, format ) \
	( width * height * depth * CoreGfx::colorFormatPixelSizeBytes[ format ] )


enum_type( GfxDepthFormat, u8 )
{
	GfxDepthFormat_NONE = 0,
	GfxDepthFormat_R16_FLOAT,
	GfxDepthFormat_R16_UINT,
	GfxDepthFormat_R32_FLOAT,
	GfxDepthFormat_R32_UINT,
	GfxDepthFormat_R24_UINT_G8_UINT,
	GFXDEPTHFORMAT_COUNT,
};


namespace CoreGfx
{
	constexpr u32 depthFormatPixelSizeBytes[GFXDEPTHFORMAT_COUNT] =
	{
		0, // GfxDepthFormat_NONE
		2, // GfxDepthFormat_R16_FLOAT
		2, // GfxDepthFormat_R16_UINT
		4, // GfxDepthFormat_R32_FLOAT
		4, // GfxDepthFormat_R32_UINT
		4, // GfxDepthFormat_R24_UINT_G8_UINT
	};
	static_assert( ARRAY_LENGTH( depthFormatPixelSizeBytes ) == GFXDEPTHFORMAT_COUNT,
		"Missing depthFormatPixelSizeBytes!" );

	constexpr const char *depthFormatName[GFXDEPTHFORMAT_COUNT] =
	{
		"NONE",             // GfxDepthFormat_NONE
		"R16_FLOAT",        // GfxDepthFormat_R16_FLOAT
		"R16_UINT",         // GfxDepthFormat_R16_UINT
		"R32_FLOAT",        // GfxDepthFormat_R32_FLOAT
		"R32_UINT",         // GfxDepthFormat_R32_UINT
		"R24_UINT_G8_UINT", // GfxDepthFormat_R24_UINT_G8_UINT
	};
	static_assert( ARRAY_LENGTH( depthFormatName ) == GFXDEPTHFORMAT_COUNT, "Missing depthFormatName!" );
}


#define GFX_SIZE_IMAGE_DEPTH_BYTES( width, height, depth, format ) \
	( width * height * depth * CoreGfx::depthFormatPixelSizeBytes[ format ] )


enum_type( GfxPrimitiveType, u8 )
{
	GfxPrimitiveType_TriangleList = 0,
	GfxPrimitiveType_TriangleStrip,
	GfxPrimitiveType_LineList,
	GfxPrimitiveType_LineStrip,
	GfxPrimitiveType_PointList,
	GFXPRIMITIVETYPE_COUNT,
};


enum_type( GfxIndexBufferFormat, u8 )
{
	GfxIndexBufferFormat_NONE = 0,
	GfxIndexBufferFormat_U16,
	GfxIndexBufferFormat_U32,
	GFXINDEXBUFFERFORMAT_COUNT,
};


enum_type( GfxWriteMode, u8 )
{
	// TODO:
	GfxWriteMode_NONE,
	GfxWriteMode_ONCE,
	GfxWriteMode_OVERWRITE,
	GfxWriteMode_RING,
	GFXWRITEMODE_COUNT,
};


enum_type( GfxFillMode, u8 )
{
	GfxFillMode_SOLID = 0,
	GfxFillMode_WIREFRAME,
	GFXRASTERFILLMODE_COUNT,
};


enum_type( GfxCullMode, u8 )
{
	GfxCullMode_NONE = 0,
	GfxCullMode_FRONT,
	GfxCullMode_BACK,
	GFXRASTERCULLMODE_COUNT,
};


enum_type( GfxSamplerFilteringMode, u8 )
{
	GfxSamplerFilteringMode_NEAREST = 0,
	GfxSamplerFilteringMode_LINEAR,
	GfxSamplerFilteringMode_MAG_NEAREST_MIN_LINEAR,
	GfxSamplerFilteringMode_MAG_LINEAR_MIN_NEAREST,
	GfxSamplerFilteringMode_ANISOTROPIC,
	GFXSAMPLERFILTERINGMODE_COUNT,
};


enum_type( GfxSamplerWrapMode, u8 )
{
	GfxSamplerWrapMode_WRAP = 0,
	GfxSamplerWrapMode_MIRROR,
	GfxSamplerWrapMode_CLAMP,
	GFXSAMPLERWRAPMODE_COUNT,
};


enum_type( GfxBlendFactor, u8 )
{
	GfxBlendFactor_ZERO = 0,
	GfxBlendFactor_ONE,
	GfxBlendFactor_SRC_COLOR,
	GfxBlendFactor_INV_SRC_COLOR,
	GfxBlendFactor_SRC_ALPHA,
	GfxBlendFactor_INV_SRC_ALPHA,
	GfxBlendFactor_DEST_ALPHA,
	GfxBlendFactor_INV_DEST_ALPHA,
	GfxBlendFactor_DEST_COLOR,
	GfxBlendFactor_INV_DEST_COLOR,
	GfxBlendFactor_SRC_ALPHA_SAT,
	GfxBlendFactor_SRC1_COLOR,
	GfxBlendFactor_INV_SRC1_COLOR,
	GfxBlendFactor_SRC1_ALPHA,
	GfxBlendFactor_INV_SRC1_ALPHA,
	GFXBLENDFACTOR_COUNT,
};


enum_type( GfxBlendOperation, u8 )
{
	GfxBlendOperation_ADD = 0,
	GfxBlendOperation_SUBTRACT,
	GfxBlendOperation_MIN,
	GfxBlendOperation_MAX,
	GFXBLENDOPERATION_COUNT,
};


enum_type( GfxBlendWrite, u8 )
{
	GfxBlendWrite_NONE = 0,
	GfxBlendWrite_RED = ( 1 << 0 ),
	GfxBlendWrite_GREEN = ( 1 << 1 ),
	GfxBlendWrite_BLUE = ( 1 << 2 ),
	GfxBlendWrite_ALPHA = ( 1 << 3 ),
	GfxBlendWrite_ALL = 0xF,
	GFXBLENDWRITE_COUNT,
};


enum_type( GfxDepthFunction, u8 )
{
	GfxDepthFunction_NONE = 0,
	GfxDepthFunction_LESS,
	GfxDepthFunction_LESS_EQUALS,
	GfxDepthFunction_GREATER,
	GfxDepthFunction_GREATER_EQUALS,
	GfxDepthFunction_EQUAL,
	GfxDepthFunction_NOT_EQUAL,
	GfxDepthFunction_ALWAYS,
	GFXDEPTHFUNCTION_COUNT,
};


enum_type( GfxDepthWrite, u8 )
{
	GfxDepthWrite_NONE = 0,
	GfxDepthWrite_ALL,
	GFXDEPTHWRITE_COUNT,
};


enum_type( GfxTextureType, u8 )
{
	GfxTextureType_2D,
	GfxTexturetype_2D_ARRAY,
	GfxTextureType_3D,
	GfxTextureType_CUBE,
	GfxTextureType_CUBE_ARRAY,
	GFXTEXTURETYPE_COUNT,
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// System

namespace CoreGfx
{
	extern bool init_api();
	extern bool free_api();

	extern bool init();
	extern bool free();

	extern bool init_textures();
	extern bool free_textures();

	extern bool init_shaders();
	extern bool free_shaders();

	extern bool init_buffers();
	extern bool free_buffers();

	extern bool init_commands();
	extern bool free_commands();
	extern void clear_commands();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Frame

namespace CoreGfx
{
	extern void api_frame_begin();
	extern void api_frame_end();
}

namespace Gfx
{
	extern void frame_begin();
	extern void frame_end();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Stack

class GfxStack
{
public:
	void init( usize reserve = 1 );
	void free();
	void grow();
	void clear();

public:
	usize add( const void *buffer, u16 size, u16 alignment );
	void *get( usize offset, u16 size );

public:
	usize capacity = 0LLU;
	usize current = 0LLU;
	byte *data = nullptr;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Clear

namespace CoreGfx
{
	extern void api_clear_color( Color color );
	extern void api_clear_depth( float depth );
}

namespace Gfx
{
	extern void clear_color( Color color );
	extern void clear_depth( float depth = 1.0f );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// State (System)

struct GfxStateSwapchain
{
	u16 width = WINDOW_WIDTH_DEFAULT;
	u16 height = WINDOW_HEIGHT_DEFAULT;
};


struct GfxStateViewport
{
	u16 width = WINDOW_WIDTH_DEFAULT;
	u16 height = WINDOW_HEIGHT_DEFAULT;
};


struct GfxStateScissor
{
	u16 x1 = 0;
	u16 y1 = 0;
	u16 x2 = 0;
	u16 y2 = 0;

	bool is_enabled() const { return x1 != x2 && y1 != y2; }
	bool operator==( const GfxStateScissor &other ) const = default;
};


struct GfxStateSampler
{
	GfxSamplerFilteringMode filterMode = GfxSamplerFilteringMode_NEAREST;
	GfxSamplerWrapMode wrapMode = GfxSamplerWrapMode_WRAP;
	int anisotropy = 1;

	bool operator==( const GfxStateSampler &other ) const = default;
};


enum_type( GfxStateDirtyFlag, u32 )
{
	GfxStateDirtyFlag_NONE = 0U,
	GfxStateDirtyFlag_SWAPCHAIN = ( 1U << 1U ),
	GfxStateDirtyFlag_VIEWPORT = ( 1U << 2U ),
	GfxStateDirtyFlag_SCISSOR = ( 1U << 3U ),
	GfxStateDirtyFlag_TARGETS = ( 1U << 4U ),
	GfxStateDirtyFlag_SHADER = ( 1U << 5U ),
	GfxStateDirtyFlag_RASTER = ( 1U << 6U ),
	GfxStateDirtyFlag_BLEND = ( 1U << 7U ),
	GfxStateDirtyFlag_DEPTH = ( 1U << 8U ),
	GfxStateDirtyFlag_STENCIL = ( 1U << 9U ),
	GfxStateDirtyFlag_ALL = 0xFFFFFFFF,
};


struct GfxPipelineDescription
{
	u32 rasterCullMode : 2 = GfxCullMode_NONE; // GfxCullMode
	u32 rasterFillMode : 1 = GfxFillMode_SOLID; // GfxFillMode

	u32 blendEnabled : 1 = true;
	u32 blendColorWriteMask : 4 = GfxBlendWrite_ALL; // GfxBlendWrite
	u32 blendSrcFactorColor : 4 = GfxBlendFactor_SRC_ALPHA; // GfxBlendFactor
	u32 blendSrcFactorAlpha : 4 = GfxBlendFactor_ONE; // GfxBlendFactor
	u32 blendDstFactorColor : 4 = GfxBlendFactor_INV_SRC_ALPHA; // GfxBlendFactor
	u32 blendDstFactorAlpha : 4 = GfxBlendFactor_INV_SRC_ALPHA; // GfxBlendFactor
	u32 blendOperationColor : 2 = GfxBlendOperation_ADD; // GfxBlendOperation
	u32 blendOperationAlpha : 2 = GfxBlendOperation_ADD; // GfxBlendOperation

	u32 depthFunction : 3 = GfxDepthFunction_NONE; // GfxDepthFunction
	u32 depthWriteMask : 1 = GfxDepthWrite_ALL; // GfxDepthWrite

	u32 stencil = 0; // TODO: Implement stencil

	void raster_set( const GfxPipelineDescription &desc );
	void blend_set( const GfxPipelineDescription &desc );
	void depth_set_state( const GfxPipelineDescription &desc );
	void stencil_set_state( const GfxPipelineDescription &desc );

	bool equal_raster( const GfxPipelineDescription &other ) const;
	bool equal_blend( const GfxPipelineDescription &other ) const;
	bool equal_depth( const GfxPipelineDescription &other ) const;
	bool equal_stencil( const GfxPipelineDescription &other ) const;

	bool operator==( const GfxPipelineDescription &other ) const = default;

	// NOTE: We want GfxPipelineDescription to be as small as possible (i.e. 64 bits here)
	// so that we can use the struct itself as a 64-bit HashMap key for construction/retrieving
	// pipeline state objects (Vulkan/Metal/D3D12). Creating keys this way guarantees that
	// no permutation of pipeline state will ever produce overlapping keys
	u64 hash() const
	{
		static_assert( sizeof( GfxPipelineDescription ) == sizeof( u64 ), "must be 8 bytes!" );
		return *reinterpret_cast<const u64 *>( this );
	}
};


struct GfxStatePipeline
{
	GfxPipelineDescription description = GfxPipelineDescription { };
	GfxShaderResource *shader = nullptr;

	bool operator==( const GfxStatePipeline &other ) const = default;
};
static_assert( sizeof( GfxStatePipeline ) == 16, "GfxStatePipeline size changed!" );


struct GfxStateCompute
{
	GfxShaderResource *shader = nullptr;

	bool operator==( const GfxStateCompute &other ) const = default;
};
static_assert( sizeof( GfxStateCompute ) == 8, "GfxStatePipeline size changed!" );


struct GfxState
{
	bool rendering = false;

	GfxStateDirtyFlag dirtyFlags = GfxStateDirtyFlag_ALL;

	GfxStateSwapchain swapchain = GfxStateSwapchain { };
	GfxStateViewport viewport = GfxStateViewport { };
	GfxStateScissor scissor = GfxStateScissor { };
	GfxStateSampler sampler = GfxStateSampler { };
	GfxStatePipeline pipeline = GfxStatePipeline { };
	GfxStateCompute compute = GfxStateCompute { };

	GfxTextureResource *boundTexture[GFX_TEXTURE_SLOT_COUNT] = { nullptr };
	CoreGfxUniformBuffer::UniformsPipeline_t pipelineUniforms = { };
	double_m44 matrixModel = { };
	double_m44 matrixView = { };
	double_m44 matrixPerspective = { };
	double_m44 matrixMVP = { };
	double_m44 matrixModelInverse = { };
	double_m44 matrixViewInverse = { };
	double_m44 matrixPerspectiveInverse = { };
	double_m44 matrixMVPInverse = { };

	const class GfxRenderPass *renderPassActive = nullptr;
	const class GfxRenderCommand *renderCommandActive = nullptr;
	bool renderPassAllowBatchBreak = true;
	bool renderCommandAllowBatchBreak = true;
};


namespace CoreGfx
{
	extern GfxState state;

	extern void state_reset();
};


namespace Gfx
{
	inline const GfxState &state() { return CoreGfx::state; }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Render State

namespace CoreGfx
{
	extern bool api_swapchain_init( u16 width, u16 height );
	extern bool api_swapchain_free();
	extern bool api_swapchain_set_size( u16 width, u16 height );

	extern bool api_viewport_init( u16 width, u16 height );
	extern bool api_viewport_free();
	extern bool api_viewport_set_size( u16 width, u16 height );

	extern bool api_scissor_set_state( const GfxStateScissor &state );

	extern bool api_sampler_set_state( const GfxStateSampler &state );
};


namespace CoreGfx
{
	extern void swapchain_viewport_update();
}


namespace Gfx
{
	extern void scissor_set_state( const GfxStateScissor &state );
	extern void scissor_set( u16 x1, u16 y1, u16 x2, u16 y2 );
	extern void scissor_set_nested( u16 x1, u16 y1, u16 x2, u16 y2 );
	extern void scissor_reset();

	extern void sampler_set_state( const GfxStateSampler &state );
	extern void sampler_filtering_mode( const GfxSamplerFilteringMode &mode );
	extern void sampler_filtering_anisotropy( int anisotropy );
	extern void sampler_wrap_mode( const GfxSamplerWrapMode &mode );

	extern int swapchain_width();
	extern int swapchain_height();

	extern int viewport_width();
	extern int viewport_height();
	extern void viewport_set_size( u16 width, u16 height );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Pipeline State

namespace CoreGfx
{
	extern bool api_set_raster( const GfxPipelineDescription &description );
	extern bool api_set_blend( const GfxPipelineDescription &description );
	extern bool api_set_depth( const GfxPipelineDescription &description );
	extern bool api_set_stencil( const GfxPipelineDescription &description );
}


namespace Gfx
{
	extern void raster_set_state( const GfxPipelineDescription &description );
	extern void raster_fill_mode( const GfxFillMode &mode );
	extern void raster_cull_mode( const GfxCullMode &mode );

	extern void blend_set_state( const GfxPipelineDescription &description );
	extern void blend_enabled( bool enabled );
	extern void blend_mode_color( const GfxBlendFactor &srcFactor,
		const GfxBlendFactor &dstFactor, const GfxBlendOperation &operation );
	extern void blend_mode_alpha( const GfxBlendFactor &srcFactor,
		const GfxBlendFactor &dstFactor, const GfxBlendOperation &operation );
	extern void blend_mode_reset();
	extern void blend_mode_reset_color();
	extern void blend_mode_reset_alpha();
	extern void blend_write_mask( const GfxBlendWrite &mask );

	extern void depth_set_state( const GfxPipelineDescription &description );
	extern void depth_function( const GfxDepthFunction &function );
	extern void depth_write_mask( const GfxDepthWrite &mask );

	extern void stencil_set_state( const GfxPipelineDescription &description );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Uniforms (Pipeline)

namespace CoreGfx
{
	extern void update_matrix_mvp();
}


namespace Gfx
{
	extern void set_matrix_mvp_2d_orthographic( double x, double y, double zoom, double angle,
		double width, double height, double znear = 0.0f, double zfar = 1.0f );

	extern void set_matrix_mvp_3d_perspective( double fov, double aspect, double znear, double zfar,
		double x, double y, double z, double xto, double yto, double zto, double xup, double yup, double zup );

	extern void set_matrix_model( const double_m44 &matrix );

	extern void set_matrix_view( const double_m44 &matrix );

	extern void set_matrix_perspective( const double_m44 &matrix );

	extern void set_matrix_mvp( const double_m44 &matModel, const double_m44 &matView,
		const double_m44 &matPerspective );

	extern const double_m44 &get_matrix_model();
	extern const double_m44 &get_matrix_view();
	extern const double_m44 &get_matrix_perspective();
	extern const double_m44 &get_matrix_mvp();

	extern const double_m44 &get_matrix_model_inverse();
	extern const double_m44 &get_matrix_view_inverse();
	extern const double_m44 &get_matrix_perspective_inverse();
	extern const double_m44 &get_matrix_mvp_inverse();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Shader

enum_type( GfxShaderStage, u8 )
{
	GfxShaderStage_Vertex = ( 1 << 0 ),
	GfxShaderStage_Fragment = ( 1 << 1 ),
	GfxShaderStage_Compute = ( 1 << 2 ),
};


namespace CoreGfx
{
	extern bool api_shader_init( GfxShaderResource *&resource, u32 shaderID, const struct ShaderEntry &shaderEntry );
	extern bool api_shader_free( GfxShaderResource *&resource );

	extern bool shader_has_stage( u32 shaderID, GfxShaderStage stage );

	extern bool shader_bind_uniform_buffers_vertex( u32 shaderID );
	extern bool shader_bind_uniform_buffers_fragment( u32 shaderID );
	extern bool shader_bind_uniform_buffers_compute( u32 shaderID );
}


class GfxShader
{
public:
	void init( u32 shaderID, const struct ShaderEntry &shaderEntry );
	void free();

	bool is_initialized() const
	{
		return resource != nullptr;
	}

public:
	GfxShaderResource *resource = nullptr;
	u32 id = 0;
};


namespace Gfx
{
	extern void set_shader_globals( const CoreGfxUniformBuffer::UniformsPipeline_t &globals );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Buffer

namespace CoreGfx
{
	extern bool api_buffer_init( GfxBufferResource *&resource, usize capacity );
	extern bool api_buffer_free( GfxBufferResource *&resource );

	extern void api_buffer_write_begin( GfxBufferResource *resource );
	extern void api_buffer_write_end( GfxBufferResource *resource );
	extern void api_buffer_write( GfxBufferResource *resource, const void *data, usize size, usize offset );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Vertex Buffer

namespace CoreGfx
{
	extern bool api_vertex_buffer_init( GfxVertexBufferResource *&resource, u32 vertexFormatID,
		GfxWriteMode writeMode, u32 capacity, u32 stride );

	extern bool api_vertex_buffer_free( GfxVertexBufferResource *&resource );

	extern void api_vertex_buffer_write_begin( GfxVertexBufferResource *resource );

	extern void api_vertex_buffer_write_end( GfxVertexBufferResource *resource );

	extern void api_vertex_buffer_write( GfxVertexBufferResource *resource, const void *data, u32 size );

	extern u32 api_vertex_buffer_current( const GfxVertexBufferResource *resource );
}


template <typename VertexFormat> class GfxVertexBuffer
{
public:
	void init( u32 count, GfxWriteMode writeMode )
	{
		const u32 capacity = count * sizeof( VertexFormat );
		ErrorIf( !CoreGfx::api_vertex_buffer_init( resource, CoreGfxVertex::vertex_format_id<VertexFormat>(),
			writeMode, capacity, sizeof( VertexFormat ) ), "Failed to init vertex buffer!" );
	}

	void free()
	{
		if( resource == nullptr ) { return; }
		ErrorIf( !CoreGfx::api_vertex_buffer_free( resource ), "Failed to free vertex buffer!" );
		resource = nullptr;
	}

	u32 current() const
	{
		return CoreGfx::api_vertex_buffer_current( resource );
	}

	void write_begin()
	{
		CoreGfx::api_vertex_buffer_write_begin( resource );
	}

	void write_end()
	{
		CoreGfx::api_vertex_buffer_write_end( resource );
	}

	template <typename T> void write( const T &element )
	{
		static_assert( sizeof( T ) % sizeof( VertexFormat ) == 0,
			"Element size must match VertexBuffer type" );
		CoreGfx::api_vertex_buffer_write( resource, &element, sizeof( element ) );
	}

	void write( void *data, usize size )
	{
		Assert( size % sizeof( VertexFormat ) == 0 );
		CoreGfx::api_vertex_buffer_write( resource, data, size );
	}

	bool is_initialized() const
	{
		return resource != nullptr;
	}

public:
	GfxVertexBufferResource *resource = nullptr;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Instance Buffer

namespace CoreGfx
{
	extern bool api_instance_buffer_init( GfxInstanceBufferResource *&resource, u32 instanceFormatID,
		GfxWriteMode writeMode, u32 capacity, u32 stride );

	extern bool api_instance_buffer_free( GfxInstanceBufferResource *&resource );

	extern void api_instance_buffer_write_begin( GfxInstanceBufferResource *resource );

	extern void api_instance_buffer_write_end( GfxInstanceBufferResource *resource );

	extern void api_instance_buffer_write( GfxInstanceBufferResource *resource, const void *data, u32 size );

	extern u32 api_instance_buffer_current( const GfxInstanceBufferResource *resource );
}


template <typename InstanceType> class GfxInstanceBuffer
{
public:
	void init( u32 count, GfxWriteMode writeMode )
	{
		const u32 capacity = count * sizeof( InstanceType );
		ErrorIf( !CoreGfx::api_instance_buffer_init( resource, CoreGfxInstance::instance_format_id<InstanceType>(),
			writeMode, capacity, sizeof( InstanceType ) ), "Failed to init instance buffer!" );
	}

	void free()
	{
		if( resource == nullptr ) { return; }
		ErrorIf( !CoreGfx::api_instance_buffer_free( resource ), "Failed to free instance buffer!" );
		resource = nullptr;
	}

	inline u32 current() const
	{
		return CoreGfx::api_instance_buffer_current( resource );
	}

	void write_begin()
	{
		CoreGfx::api_instance_buffer_write_begin( resource );
	}

	void write_end()
	{
		CoreGfx::api_instance_buffer_write_end( resource );
	}

	template <typename T> void write( const T &element )
	{
	#if GRAPHICS_ENABLED
		static_assert( sizeof( T ) % sizeof( InstanceType ) == 0, "Element size must match InstanceBuffer type" );
		CoreGfx::api_instance_buffer_write( resource, &element, sizeof( element ) );
	#endif
	}

	void write( void *data, usize size )
	{
	#if GRAPHICS_ENABLED
		Assert( size % sizeof( InstanceType ) == 0 );
		CoreGfx::api_instance_buffer_write( resource, data, size );
	#endif
	}

	bool is_initialized() const
	{
		return resource != nullptr;
	}

public:
	GfxInstanceBufferResource *resource = nullptr;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Index Buffer

namespace CoreGfx
{
	extern bool api_index_buffer_init( GfxIndexBufferResource *&resource, void *data, u32 size,
		double indicesToVerticiesRatio, GfxIndexBufferFormat format, GfxWriteMode writeMode );

	extern bool api_index_buffer_free( GfxIndexBufferResource *&resource );
}


class GfxIndexBuffer
{
public:
	void init( void *data, u32 size, double indicesToVerticiesRatio, GfxIndexBufferFormat format )
	{
		this->indicesToVerticiesRatio = indicesToVerticiesRatio;
		ErrorIf( !CoreGfx::api_index_buffer_init( resource, data, size,
			indicesToVerticiesRatio, format, GfxWriteMode_ONCE ), "Failed to init index buffer!" );
	}

	void free()
	{
		if( resource == nullptr ) { return; }
		ErrorIf( !CoreGfx::api_index_buffer_free( resource ), "Failed to free index buffer!" );
		resource = nullptr;
	}

	bool is_initialized() const
	{
		return resource != nullptr;
	}

public:
	GfxIndexBufferResource *resource = nullptr;

private:
	double indicesToVerticiesRatio = 1.0;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Uniform Buffer

namespace CoreGfx
{
	extern bool api_uniform_buffer_init( GfxUniformBufferResource *&resource,
		const char *name, int index, u32 size );

	extern bool api_uniform_buffer_free( GfxUniformBufferResource *&resource );

	extern void api_uniform_buffer_write_begin( GfxUniformBufferResource *resource );

	extern void api_uniform_buffer_write_end( GfxUniformBufferResource *resource );

	extern void api_uniform_buffer_write( GfxUniformBufferResource *resource, const void *data );

	extern bool api_uniform_buffer_bind_vertex( GfxUniformBufferResource *resource, int slot );

	extern bool api_uniform_buffer_bind_fragment( GfxUniformBufferResource *resource, int slot );

	extern bool api_uniform_buffer_bind_compute( GfxUniformBufferResource *resource, int slot );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// TODO: GfxConstantBuffer (StructuredBuffer)
// TODO: GfxMutableBuffer (RWStructuredBuffer)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Texture

namespace CoreGfx
{
	extern bool api_texture_init( GfxTextureResource *&resource, void *data,
		u16 width, u16 height, u16 levels, const GfxColorFormat &format );

	extern bool api_texture_free( GfxTextureResource *&resource );

	extern bool api_texture_bind( GfxTextureResource *resource, int slot );

	extern bool api_texture_release( GfxTextureResource *resource, int slot );
}


class GfxTexture
{
public:
	void init_2d( void *data, u16 width, u16 height, const GfxColorFormat &format );
	void init_2d( void *data, u16 width, u16 height, u16 levels, const GfxColorFormat &format );
	void free();

	bool is_initialized() const
	{
		return resource != nullptr;
	}

public:
	GfxTextureResource *resource = nullptr;
	GfxTextureType type = GfxTextureType_2D;
};


namespace Gfx
{
	extern void bind_texture( int slot, Texture texture );
	extern void bind_texture( int slot, const GfxTexture &texture );

	extern void release_texture( int slot, Texture texture );
	extern void release_texture( int slot, const GfxTexture &texture );
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Render Target

struct GfxRenderTargetDescription
{
	GfxColorFormat colorFormat = GfxColorFormat_R8G8B8A8_FLOAT;
	GfxDepthFormat depthFormat = GfxDepthFormat_NONE;
	bool cpuAccess = false;
	int sampleCount = 0;
};


namespace CoreGfx
{
	extern bool api_render_target_init( GfxRenderTargetResource *&resource,
		GfxTextureResource *&resourceColor,
		GfxTextureResource *&resourceDepth,
		u16 width, u16 height, u16 layers,
		const GfxRenderTargetDescription &desc = { } );

	extern bool api_render_target_free( GfxRenderTargetResource *&resource,
		GfxTextureResource *&resourceColor,
		GfxTextureResource *&resourceDepth );

	extern bool api_render_target_copy_color(
		GfxRenderTargetResource *&srcResource, GfxTextureResource *&srcResourceColor,
		GfxRenderTargetResource *&dstResource, GfxTextureResource *&dstResourceColor );

	extern bool api_render_target_copy_depth(
		GfxRenderTargetResource *&srcResource, GfxTextureResource *&srcResourceDepth,
		GfxRenderTargetResource *&dstResource, GfxTextureResource *&dstResourceDepth );

	extern bool api_render_target_copy(
		GfxRenderTargetResource *&srcResource,
		GfxTextureResource *&srcResourceColor, GfxTextureResource *&srcResourceDepth,
		GfxRenderTargetResource *&dstResource,
		GfxTextureResource *&dstResourceColor, GfxTextureResource *&dstResourceDepth );

	extern bool api_render_target_copy_part(
		GfxRenderTargetResource *&srcResource,
		GfxTextureResource *&srcResourceColor, GfxTextureResource *&srcResourceDepth,
		GfxRenderTargetResource *&dstResource,
		GfxTextureResource *&dstResourceColor, GfxTextureResource *&dstResourceDepth,
		u16 srcX, u16 srcY, u16 dstX, u16 dstY, u16 width, u16 height );

	extern bool api_render_target_buffer_read_color( GfxRenderTargetResource *&resource,
		GfxTextureResource *&resourceColor,
		void *buffer, u32 size );

	extern bool api_render_target_buffer_read_color_async_request( GfxRenderTargetResource *&resource,
		GfxTextureResource *&resourceColor,
		void *buffer, u32 size );

	extern bool api_render_target_buffer_read_color_async_poll( GfxRenderTargetResource *&resource,
		GfxTextureResource *&resourceColor,
		void *buffer, u32 size );

	extern bool api_render_target_buffer_read_depth( GfxRenderTargetResource *&resource,
		GfxTextureResource *&resourceDepth,
		void *buffer, u32 size );

	extern bool api_render_target_buffer_read_depth_async_request( GfxRenderTargetResource *&resource,
		GfxTextureResource *&resourceColor,
		void *buffer, u32 size );

	extern bool api_render_target_buffer_read_depth_async_poll( GfxRenderTargetResource *&resource,
		GfxTextureResource *&resourceColor,
		void *buffer, u32 size );

	extern bool api_render_target_buffer_write_color( GfxRenderTargetResource *&resource,
		GfxTextureResource *&resourceColor,
		const void *buffer, u32 size );

	extern bool api_render_target_buffer_write_depth( GfxRenderTargetResource *&resource,
		GfxTextureResource *&resourceDepth,
		const void *buffer, u32 size );
}


class GfxRenderTarget
{
public:
	void init( u16 width, u16 height, const GfxRenderTargetDescription &desc );
	void init( u16 width, u16 height, u16 layers, const GfxRenderTargetDescription &desc );
	void free();
	void copy( GfxRenderTarget &source );
	void copy_color( GfxRenderTarget &source );
	void copy_depth( GfxRenderTarget &source );
	void copy_part( GfxRenderTarget &source, int srcX, int srcY, int dstX, int dstY, u16 width, u16 height );
	void resize( u16 width, u16 height );
	void bind( int slot = 0 );
	void release();

	bool is_initialized() const
	{
		return resource != nullptr;
	}

public:
	GfxRenderTargetResource *resource = nullptr;
	GfxRenderTargetDescription desc;
	GfxTexture textureColor;
	GfxTexture textureDepth;
	u16 width, height, layers;
	int slot = -1;

private:
	static GfxRenderTargetResource *bound;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Draw

namespace CoreGfx
{
	extern bool api_draw(
		const GfxVertexBufferResource *resourceVertex, u32 vertexCount,
		const GfxInstanceBufferResource *resourceInstance, u32 instanceCount,
		const GfxIndexBufferResource *resourceIndex,
		GfxPrimitiveType type );
}


namespace Gfx
{
	inline void draw_vertices( u32 vertexCount,
		GfxPrimitiveType type = GfxPrimitiveType_TriangleList )
	{
	#if GRAPHICS_ENABLED
		AssertMsg( CoreGfx::state.renderCommandActive, "Draw calls must be part of a GfxRenderCommand!" );
		CoreGfx::api_draw(
			nullptr, vertexCount,
			nullptr, 0,
			nullptr, type );
	#endif
	}


	inline void draw_vertices_instanced( u32 vertexCount, u32 instanceCount,
		GfxPrimitiveType type = GfxPrimitiveType_TriangleList )
	{
	#if GRAPHICS_ENABLED
		AssertMsg( CoreGfx::state.renderCommandActive, "Draw calls must be part of a GfxRenderCommand!" );
		CoreGfx::api_draw(
			nullptr, vertexCount,
			nullptr, instanceCount,
			nullptr, type );
	#endif
	}


	template <typename InstanceFormat>
	inline void draw_vertices_instanced( u32 vertexCount,
		const GfxInstanceBuffer<InstanceFormat> &instanceBuffer,
		GfxPrimitiveType type = GfxPrimitiveType_TriangleList )
	{
	#if GRAPHICS_ENABLED
		AssertMsg( CoreGfx::state.renderCommandActive, "Draw calls must be part of a GfxRenderCommand!" );
		const u32 instanceCount = instanceBuffer.current() / sizeof( InstanceFormat );
		if( instanceCount == 0 ) { return; }
		CoreGfx::api_draw(
			nullptr, vertexCount,
			instanceBuffer.resource, instanceCount,
			nullptr, type );
	#endif
	}


	inline void draw_vertices_indexed( u32 vertexCount,
		const GfxIndexBuffer &indexBuffer,
		GfxPrimitiveType type = GfxPrimitiveType_TriangleList )
	{
	#if GRAPHICS_ENABLED
		AssertMsg( CoreGfx::state.renderCommandActive, "Draw calls must be part of a GfxRenderCommand!" );
		CoreGfx::api_draw(
			nullptr, vertexCount,
			nullptr, 0,
			indexBuffer.resource, type );
	#endif
	}


	inline void draw_vertices_instanced_indexed( u32 vertexCount, u32 instanceCount,
		const GfxIndexBuffer &indexBuffer,
		GfxPrimitiveType type = GfxPrimitiveType_TriangleList )
	{
	#if GRAPHICS_ENABLED
		AssertMsg( CoreGfx::state.renderCommandActive, "Draw calls must be part of a GfxRenderCommand!" );
		CoreGfx::api_draw(
			nullptr, vertexCount,
			nullptr, instanceCount,
			indexBuffer.resource, type );
	#endif
	}


	template <typename InstanceFormat>
	inline void draw_vertices_instanced_indexed( u32 vertexCount,
		const GfxInstanceBuffer<InstanceFormat> &instanceBuffer,
		const GfxIndexBuffer &indexBuffer,
		GfxPrimitiveType type = GfxPrimitiveType_TriangleList )
	{
	#if GRAPHICS_ENABLED
		AssertMsg( CoreGfx::state.renderCommandActive, "Draw calls must be part of a GfxRenderCommand!" );
		const u32 instanceCount = instanceBuffer.current() / sizeof( InstanceFormat );
		if( instanceCount == 0 ) { return; }
		CoreGfx::api_draw(
			nullptr, vertexCount,
			instanceBuffer.resource, instanceCount,
			indexBuffer.resource, type );
	#endif
	}


	template <typename VertexFormat>
	inline void draw_vertex_buffer(
		const GfxVertexBuffer<VertexFormat> &vertexBuffer,
		GfxPrimitiveType type = GfxPrimitiveType_TriangleList )
	{
	#if GRAPHICS_ENABLED
		AssertMsg( CoreGfx::state.renderCommandActive, "Draw calls must be part of a GfxRenderCommand!" );
		const u32 vertexCount = vertexBuffer.current() / sizeof( VertexFormat );
		if( vertexCount == 0 ) { return; }
		CoreGfx::api_draw( vertexBuffer.resource, vertexCount,
			nullptr, 0,
			nullptr, type );
	#endif
	}


	template <typename VertexFormat, typename InstanceFormat>
	inline void draw_vertex_buffer_instanced(
		const GfxVertexBuffer<VertexFormat> &vertexBuffer,
		const GfxInstanceBuffer<InstanceFormat> &instanceBuffer,
		GfxPrimitiveType type = GfxPrimitiveType_TriangleList )
	{
	#if GRAPHICS_ENABLED
		AssertMsg( CoreGfx::state.renderCommandActive, "Draw calls must be part of a GfxRenderCommand!" );
		const u32 vertexCount = vertexBuffer.current() / sizeof( VertexFormat );
		const u32 instanceCount = instanceBuffer.current() / sizeof( InstanceFormat );
		if( vertexCount == 0 || instanceCount == 0 ) { return; }
		CoreGfx::api_draw( vertexBuffer.resource, vertexCount,
			instanceBuffer.resource, instanceCount,
			nullptr, type );
	#endif
	}


	template <typename VertexFormat>
	inline void draw_vertex_buffer_instanced(
		const GfxVertexBuffer<VertexFormat> &vertexBuffer,
		const u32 instanceCount,
		GfxPrimitiveType type = GfxPrimitiveType_TriangleList )
	{
	#if GRAPHICS_ENABLED
		AssertMsg( CoreGfx::state.renderCommandActive, "Draw calls must be part of a GfxRenderCommand!" );
		const u32 vertexCount = vertexBuffer.current() / sizeof( VertexFormat );
		if( vertexCount == 0 ) { return; }
		CoreGfx::api_draw( vertexBuffer.resource, vertexCount,
			nullptr, instanceCount,
			nullptr, type );
	#endif
	}


	template <typename VertexFormat>
	inline void draw_vertex_buffer_indexed(
		const GfxVertexBuffer<VertexFormat> &vertexBuffer,
		const GfxIndexBuffer &indexBuffer,
		GfxPrimitiveType type = GfxPrimitiveType_TriangleList )
	{
	#if GRAPHICS_ENABLED
		AssertMsg( CoreGfx::state.renderCommandActive, "Draw calls must be part of a GfxRenderCommand!" );
		const u32 vertexCount = vertexBuffer.current() / sizeof( VertexFormat );
		if( vertexCount == 0 ) { return; }
		CoreGfx::api_draw( vertexBuffer.resource, vertexCount,
			nullptr, 0,
			indexBuffer.resource, type );
	#endif
	}


	template <typename VertexFormat, typename InstanceFormat>
	inline void draw_vertex_buffer_instanced_indexed(
		const GfxVertexBuffer<VertexFormat> &vertexBuffer,
		const GfxInstanceBuffer<InstanceFormat> &instanceBuffer,
		const GfxIndexBuffer &indexBuffer,
		GfxPrimitiveType type = GfxPrimitiveType_TriangleList )
	{
	#if GRAPHICS_ENABLED
		AssertMsg( CoreGfx::state.renderCommandActive, "Draw calls must be part of a GfxRenderCommand!" );
		const u32 vertexCount = vertexBuffer.current() / sizeof( VertexFormat );
		const u32 instanceCount = instanceBuffer.current() / sizeof( InstanceFormat );
		if( vertexCount == 0 || instanceCount == 0 ) { return; }
		CoreGfx::api_draw( vertexBuffer.resource, vertexCount,
			instanceBuffer.resource, instanceCount,
			indexBuffer.resource, type );
	#endif
	}


	template <typename VertexFormat>
	inline void draw_vertex_buffer_instanced_indexed(
		const GfxVertexBuffer<VertexFormat> &vertexBuffer,
		const u32 instanceCount,
		const GfxIndexBuffer &indexBuffer,
		GfxPrimitiveType type = GfxPrimitiveType_TriangleList )
	{
	#if GRAPHICS_ENABLED
		AssertMsg( CoreGfx::state.renderCommandActive, "Draw calls must be part of a GfxRenderCommand!" );
		const u32 vertexCount = vertexBuffer.current() / sizeof( VertexFormat );
		if( vertexCount == 0 ) { return; }
		CoreGfx::api_draw( vertexBuffer.resource, vertexCount,
			nullptr, instanceCount,
			indexBuffer.resource, type );
	#endif
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Dispatch

namespace CoreGfx
{
	extern bool api_dispatch( u32 x, u32 y, u32 z );
};


namespace Gfx
{
	inline void dispatch( u32 x, u32 y, u32 z )
	{
	#if GRAPHICS_ENABLED
		AssertMsg( CoreGfx::state.renderCommandActive, "Dispatch calls must be part of a GfxRenderCommand!" );
		CoreGfx::api_dispatch( x, y, z );
	#endif
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Render Command

class GfxRenderCommand
{
public:
	void shader( const GfxShader &shader );
	void shader( Shader shader );

	void set_pipeline_description( const GfxPipelineDescription &description );

	void raster_fill_mode( const GfxFillMode &mode );
	void raster_cull_mode( const GfxCullMode &mode );

	void blend_enabled( bool enabled );
	void blend_mode_color( const GfxBlendFactor &srcFactor,
		const GfxBlendFactor &dstFactor, const GfxBlendOperation &operation );
	void blend_mode_alpha( const GfxBlendFactor &srcFactor,
		const GfxBlendFactor &dstFactor, const GfxBlendOperation &operation );
	void blend_write_mask( const GfxBlendWrite &mask );
	void depth_function( const GfxDepthFunction &mode );
	void depth_write_mask( const GfxDepthWrite &mask );

	void clear_color( Color color );
	void clear_depth( float depth );

public:
	template <typename T> void set_tags( const T &tags )
	{
	#if GRAPHICS_ENABLED
		static_assert( sizeof( T ) <= U16_MAX, "Tag payload size must not exceed 65535 bytes!" );
		tagsPayloadOffset = GfxRenderCommand::renderCommandTagsStack.add( &tags, sizeof( T ), alignof( T ) );
		tagsPayloadSize = static_cast<u16>( sizeof( T ) );
	#endif
	}

	template <typename T> T *get_tags()
	{
	#if GRAPHICS_ENABLED
		void *tags = GfxRenderCommand::renderCommandTagsStack.get( tagsPayloadOffset, tagsPayloadSize );
		return reinterpret_cast<T *>( tags );
	#endif
	}

	// Usage:
	// pass.work( +[]() { ... } );
	// pass.work( GfxWork { ... } );
	void work( void ( *function )() )
	{
	#if GRAPHICS_ENABLED
		workPayloadOffset = 0LLU;
		workPayloadSize = 0U;

		workFunction = reinterpret_cast<void *>( function );
		workFunctionInvoker = +[]( void *func, void *args ) { reinterpret_cast<void (*)()>( func )(); };
	#endif
	}

	// Usage:
	// pass.work( args, +[]( Type &args ) { ... } );
	// pass.work( GfxWorkVariable( args ) { ... } );
	// pass.work( GfxWorkStructure( Type( ... ), args ) { ... } );
	template <typename T> void work( T &&payload, void ( *function )( T &args ) )
	{
	#if GRAPHICS_ENABLED
		static_assert( sizeof( T ) <= U16_MAX, "Payload size must not exceed 65535 bytes!" );
		workPayloadOffset = GfxRenderCommand::renderCommandArgsStack.add( &payload, sizeof( T ), alignof( T ) );
		workPayloadSize = static_cast<u16>( sizeof( T ) );

		workFunction = reinterpret_cast<void *>( function );
		workFunctionInvoker = +[]( void *func, void *args )
		{
			reinterpret_cast<void (*)(T &)>( func )( *reinterpret_cast<T *>( args ) );
		};
	#endif
	}

	// Usage:
	// pass.work( args, +[]( Type &args ) { ... } );
	// pass.work( GfxWorkVariable( variable ) { ... } );
	// pass.work( GfxWorkStructure( Type( ... ), name ) { ... } );
	template <typename T> void work( const T &payload, void ( *function )( T &args ) )
	{
	#if GRAPHICS_ENABLED
		static_assert( sizeof( T ) <= U16_MAX, "Payload size must not exceed 65535 bytes!" );
		workPayloadOffset = GfxRenderCommand::renderCommandArgsStack.add( &payload, sizeof( T ), alignof( T ) );
		workPayloadSize = static_cast<u16>( sizeof( T ) );

		workFunction = reinterpret_cast<void *>( function );
		workFunctionInvoker = +[]( void *func, void *args )
		{
			reinterpret_cast<void (*)(T &)>( func )( *reinterpret_cast<T *>( args ) );
		};
	#endif
	}

public:
	static GfxStack renderCommandArgsStack;
	static GfxStack renderCommandTagsStack;

public:
	GfxStatePipeline pipeline = GfxStatePipeline { };
	bool clearColor = false;
	bool clearDepth = false;
	Color clearColorValue = c_white;
	float clearDepthValue = 1.0f;

	void ( *workFunctionInvoker )( void *, void * ) = nullptr;
	void *workFunction = nullptr;
	usize workPayloadOffset = 0LLU;
	usize tagsPayloadOffset = 0LLU;
	u16 workPayloadSize = 0U;
	u16 tagsPayloadSize = 0U;
};
constexpr usize s = sizeof( GfxRenderCommand );


namespace CoreGfx
{
	extern void api_render_command_execute( const GfxRenderCommand &command );
	extern void api_render_command_execute_post( const GfxRenderCommand &command );
}


namespace CoreGfx
{
	extern void render_command_execute_begin( const GfxRenderCommand &command );
	extern void render_command_execute_end( const GfxRenderCommand &command );
}


namespace Gfx
{
	extern void render_command_execute( const GfxRenderCommand &command );

	// Usage:
	// Gfx::render_command_execute( cmd, <lambda> { ... } );
	// Gfx::render_command_execute( cmd, <function pointer> );
	// Gfx::render_command_execute( cmd, GfxWorkCapture { ... } );
	template <typename FunctionOrLambda>
	void render_command_execute( const GfxRenderCommand &command, FunctionOrLambda work )
	{
	#if GRAPHICS_ENABLED
		AssertMsg( CoreGfx::state.renderCommandActive == nullptr, "GfxCommands cannot be nested!" );

		CoreGfx::render_command_execute_begin( command );
		CoreGfx::api_render_command_execute( command ); work();
		CoreGfx::render_command_execute_end( command );
		CoreGfx::api_render_command_execute_post( command );
	#endif
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Render Graph

class GfxRenderGraph
{
public:
	List<GfxRenderCommand> &command_list();
	const List<GfxRenderCommand> &command_list() const;

	void add_command( const GfxRenderCommand &command );
	void add_command( GfxRenderCommand &&command );

public:
	static List<List<GfxRenderCommand>>renderGraphCommandLists;
	static usize renderGraphCommandListCurrent;

public:
	usize commandList = USIZE_MAX;
};


namespace Gfx
{
	extern void render_graph_sort( GfxRenderGraph &graph,
		bool ( *sort )( GfxRenderCommand &cmdA, GfxRenderCommand &cmdB ) );
	extern void render_graph_execute( const GfxRenderGraph &pass );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Render Pass

class GfxRenderPass
{
public:
	void target( int slot, const GfxRenderTarget &target, int layer = 0 );
	void set_name( const char *name );
	void set_name_f( const char *name, ... );

public:
	GfxRenderTargetResource *targets[GFX_RENDER_TARGET_SLOT_COUNT] = { nullptr };
	int layers[GFX_RENDER_TARGET_SLOT_COUNT] = { 0 };
#if COMPILE_DEBUG
	char name[128] = { '\0' };
#endif
};


namespace CoreGfx
{
	extern void api_render_pass_begin( const GfxRenderPass &pass );
	extern void api_render_pass_end( const GfxRenderPass &pass );
}


namespace Gfx
{
	extern void render_pass_begin( const GfxRenderPass &pass );
	extern void render_pass_end( const GfxRenderPass &pass );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Utility

namespace CoreGfx
{
	extern byte *scratch_buffer( usize size );
}


namespace Gfx
{
	extern u16 mip_level_count_2d( u16 width, u16 height );

	extern bool mip_level_validate_2d( u16 width, u16 height, u16 levels );

	extern usize mip_buffer_size_2d( u16 width, u16 height, u16 levels,
		GfxColorFormat format );

	extern bool mip_generate_next_2d( const void *data, u16 width, u16 height,
		GfxColorFormat format, void *dest, usize size );

	extern bool mip_generate_next_2d_alloc( const void *data, u16 width, u16 height,
		GfxColorFormat format, void *&outData, usize &outSize );

	extern bool mip_generate_chain_2d( const void *data, u16 width, u16 height,
		GfxColorFormat format, void *dest, usize size );

	extern bool mip_generate_chain_2d_alloc( const void *data, u16 width, u16 height,
		GfxColorFormat format, void *&outData, usize &outSize );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Quad Batch (Built-in)

template <typename VertexFormat> class GfxQuadBatch
{
public:
	struct Quad { VertexFormat v0, v1, v2, v3; };

public:
	bool init( u32 capacity = GFX_QUAD_BATCH_SIZE )
	{
		Assert( capacity * 6 < U32_MAX );
		this->capacity = capacity;

#if GRAPHICS_ENABLED
		vertexBuffer.init( this->capacity * 6, GfxWriteMode_RING );

		const usize size = this->capacity * 6 * sizeof( u32 );
		u32 *indices = reinterpret_cast<u32 *>( memory_alloc( size ) );

		u32 i = 0; u32 j = 0;
		do
		{
			indices[j + 0] = i;
			indices[j + 1] = i + 1;
			indices[j + 2] = i + 2;
			indices[j + 3] = i + 3;
			indices[j + 4] = i + 2;
			indices[j + 5] = i + 1;
			i += 4; j += 6;
		}
		while ( j < this->capacity * 6 );

		constexpr double ratio = 6.0 / 4.0;
		indexBuffer.init( indices, size, ratio, GfxIndexBufferFormat_U32 );
		memory_free( indices );
#endif

		return true;
	}

	bool free()
	{
#if GRAPHICS_ENABLED
		vertexBuffer.free();
		indexBuffer.free();
#endif
		return true;
	}

	void batch_begin()
	{
#if GRAPHICS_ENABLED
		vertexBuffer.write_begin();
#endif
	}

	void batch_end()
	{
#if GRAPHICS_ENABLED
		vertexBuffer.write_end();

		if( vertexBuffer.current() == 0 ) { return; }

		if( CoreGfx::state.renderCommandActive == nullptr )
		{
			GfxRenderCommand cmd;
			cmd.shader( Shader::SHADER_DEFAULT );

			CoreGfx::state.renderCommandAllowBatchBreak = false;
			Gfx::render_command_execute( cmd, [this]()
				{
					Gfx::draw_vertex_buffer_indexed( vertexBuffer, indexBuffer );
				} );
			CoreGfx::state.renderCommandAllowBatchBreak = true;
		}
		else
		{
			Gfx::draw_vertex_buffer_indexed( vertexBuffer, indexBuffer );
		}
#endif
	}

	void batch_break()
	{
#if GRAPHICS_ENABLED
		if( vertexBuffer.current() == 0 ) { return; }
		if( !CoreGfx::state.renderCommandAllowBatchBreak ) { return; }

		batch_end();
		batch_begin();
#endif
	}

	bool can_break()
	{
#if GRAPHICS_ENABLED
		return vertexBuffer.current() >= capacity * sizeof( Quad );
#else
		return true;
#endif
	}

	void break_check()
	{
#if GRAPHICS_ENABLED
		if( UNLIKELY( can_break() ) ) { batch_break(); }
#endif
	}

	void write( const Quad &quad )
	{
#if GRAPHICS_ENABLED
		break_check();
		vertexBuffer.write( quad );
#endif
	}

	void write( const VertexFormat &v0, const VertexFormat &v1, const VertexFormat &v2, const VertexFormat &v3 )
	{
#if GRAPHICS_ENABLED
		break_check();
		vertexBuffer.write( Quad { v0, v1, v2, v3 } );
#endif
	}

	void draw()
	{
#if GRAPHICS_ENABLED
		batch_break();
#endif
	}

public:
	bool active = false;
	u32 capacity = 0;
	GfxVertexBuffer<VertexFormat> vertexBuffer;
	GfxIndexBuffer indexBuffer;
};


namespace CoreGfx
{
	extern GfxQuadBatch<GfxVertex::BuiltinVertex> batch;

	extern void quad_batch_frame_begin();
	extern void quad_batch_frame_end();
}


namespace Gfx
{
	extern void quad_batch_break();
	extern bool quad_batch_can_break();
	extern void quad_batch_break_check();

	extern void quad_batch_write( const GfxQuadBatch<GfxVertex::BuiltinVertex>::Quad &quad,
		const GfxTexture *texture = nullptr );

	extern void quad_batch_write( float x1, float y1, float x2, float y2,
		u16 u1, u16 v1, u16 u2, u16 v2, Color c1, Color c2, Color c3, Color c4,
		const GfxTexture *texture = nullptr, float depth = 0.0f );

	extern void quad_batch_write( float x1, float y1, float x2, float y2,
		float x3, float y3, float x4, float y4,
		u16 u1, u16 v1, u16 u2, u16 v2, Color c1, Color c2, Color c3, Color c4,
		const GfxTexture *texture = nullptr, float depth = 0.0f );

	extern void quad_batch_write( float x1, float y1, float x2, float y2,
		u16 u1, u16 v1, u16 u2, u16 v2, Color color,
		const GfxTexture *texture = nullptr, float depth = 0.0f );

	extern void quad_batch_write( float x1, float y1, float x2, float y2,
		float x3, float y3, float x4, float y4,
		u16 u1, u16 v1, u16 u2, u16 v2, Color color,
		const GfxTexture *texture = nullptr, float depth = 0.0f );
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////