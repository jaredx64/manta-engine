#pragma once

#include <gfx.generated.hpp>

#include <core/debug.hpp>
#include <core/types.hpp>
#include <core/color.hpp>

#include <vendor/vendor.hpp>

#include <manta/assets.hpp>

#include <config.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if !defined( HEADLESS ) && ( GRAPHICS_D3D11 || GRAPHICS_D3D12 || GRAPHICS_METAL || GRAPHICS_OPENGL || GRAPHICS_VULKAN )
	#define GRAPHICS_ENABLED ( true )
#else
	#define GRAPHICS_ENABLED ( false )
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define GRAPHICS_API_IMPLEMENTATION_WARNING \
	Warning( "Using '%s' -- not implemented on " GRAPHICS_API_NAME, __FUNCTION__ )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// 'Gfx'  = Graphics API (Public)
// 'CoreGfx' = Engine/Backend Graphics (Internal)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define GFX_TEXTURE_SLOT_COUNT       ( 8 )
#define GFX_RENDER_TARGET_SLOT_COUNT ( 8 )
#define GFX_MIP_DEPTH_MAX            ( 16 )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Shader;

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

	extern void debug_overlay_gfx( const float x, const float y );
#else
	#define PROFILE_GFX(expr)
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define GFX_RESOURCE_COUNT_SHADER             ( 256 * 256 )
#define GFX_RESOURCE_COUNT_VERTEX_BUFFER      ( 256 * 256 )
#define GFX_RESOURCE_COUNT_INSTANCE_BUFFER    ( 256 * 256 )
#define GFX_RESOURCE_COUNT_INDEX_BUFFER       ( 256 * 256 )
#define GFX_RESOURCE_COUNT_UNIFORM_BUFFER     ( 256 * 256 )
#define GFX_RESOURCE_COUNT_CONSTANT_BUFFER    ( 256 * 256 )
#define GFX_RESOURCE_COUNT_MUTABLE_BUFFER     ( 256 * 256 )
#define GFX_RESOURCE_COUNT_TEXTURE_1D         ( 1024 )
#define GFX_RESOURCE_COUNT_TEXTURE_2D         ( 1024 )
#define GFX_RESOURCE_COUNT_TEXTURE_2D_ARRAY   ( 1024 )
#define GFX_RESOURCE_COUNT_TEXTURE_3D         ( 1024 )
#define GFX_RESOURCE_COUNT_TEXTURE_3D_ARRAY   ( 1024 )
#define GFX_RESOURCE_COUNT_TEXTURE_CUBE       ( 1024 )
#define GFX_RESOURCE_COUNT_TEXTURE_CUBE_ARRAY ( 1024 )
#define GFX_RESOURCE_COUNT_RENDER_TARGET_2D   ( 1024 )

using GfxResourceID = u32;
#define GFX_RESOURCE_ID_NULL ( U32_MAX )

struct GfxResource
{
	GfxResourceID id = GFX_RESOURCE_ID_NULL;
};

struct GfxShaderResource;
struct GfxVertexBufferResource;
struct GfxInstanceBufferResource;
struct GfxIndexBufferResource;
struct GfxUniformBufferResource;
struct GfxConstantBufferResource;
struct GfxMutableBufferResource;
struct GfxTexture1DResource;
struct GfxTexture2DResource;
struct GfxTexture2DArrayResource;
struct GfxTexture3DResource;
struct GfxTexture3DArrayResource;
struct GfxTextureCubeResource;
struct GfxTextureCubeArrayResource;
struct GfxRenderTarget2DResource;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum_type( GfxColorFormat, u8 )
{
	GfxColorFormat_NONE = 0,
	GfxColorFormat_R8G8B8A8_FLOAT,
	GfxColorFormat_R8G8B8A8_UINT,
	GfxColorFormat_R10G10B10A2_FLOAT,
	GfxColorFormat_R8,
	GfxColorFormat_R8G8,
	GfxColorFormat_R16,
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
		1,  // GfxColorFormat_R8
		2,  // GfxColorFormat_R8G8
		2,  // GfxColorFormat_R16
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
		"R8",                 // GfxColorFormat_R8
		"R8G8",               // GfxColorFormat_R8G8
		"R16",                // GfxColorFormat_R16
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


enum_type( GfxDepthTestMode, u8 )
{
	GfxDepthTestMode_NONE = 0,
	GfxDepthTestMode_LESS,
	GfxDepthTestMode_LESS_EQUALS,
	GfxDepthTestMode_GREATER,
	GfxDepthTestMode_GREATER_EQUALS,
	GfxDepthTestMode_EQUAL,
	GfxDepthTestMode_NOT_EQUAL,
	GfxDepthTestMode_ALWAYS,
	GFXDEPTHTESTMODE_COUNT,
};


enum_type( GfxDepthWriteFlag, u8 )
{
	GfxDepthWriteFlag_NONE = 0,
	GfxDepthWriteFlag_ALL,
	GFXDEPTHWRITEFLAG_COUNT,
};


enum_type( GfxPrimitiveType, u8 )
{
	GfxPrimitiveType_TriangleList = 0,
	GfxPrimitiveType_TriangleStrip,
	GfxPrimitiveType_LineList,
	GfxPrimitiveType_LineStrip,
	GfxPrimitiveType_PointList,
	GFXPRIMITIVETYPE_COUNT,
};


enum_type( GfxFillMode, u8 )
{
	GfxFillMode_SOLID = 0,
	GfxFillMode_WIREFRAME,
	GFXFILLMODE_COUNT,
};


enum_type( GfxCullMode, u8 )
{
	GfxCullMode_NONE = 0,
	GfxCullMode_FRONT,
	GfxCullMode_BACK,
	GFXCULLMODE_COUNT,
};


enum_type( GfxFilteringMode, u8 )
{
	GfxFilteringMode_NEAREST = 0,
	GfxFilteringMode_LINEAR,
	GfxFilteringMode_ANISOTROPIC,
	GFXFILTERINGMODE_COUNT,
};


enum_type( GfxUVWrapMode, u8 )
{
	GfxUVWrapMode_WRAP = 0,
	GfxUVWrapMode_MIRROR,
	GfxUVWrapMode_CLAMP,
	GFXUVWRAPMODE_COUNT,
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
	GfxBlendOperation_REV_SUBTRACT,
	GfxBlendOperation_MIN,
	GfxBlendOperation_MAX,
	GFXBLENDOPERATION_COUNT,
};


enum_type( GfxColorWriteFlag, u8 )
{
	GfxColorWriteFlag_NONE = 0,
	GfxColorWriteFlag_RED = ( 1 << 0 ),
	GfxColorWriteFlag_GREEN = ( 1 << 1 ),
	GfxColorWriteFlag_BLUE = ( 1 << 2 ),
	GfxColorWriteFlag_ALPHA = ( 1 << 3 ),
	GfxColorWriteFlag_ALL = 0xF,
	GFXCOLORWRITEFLAG_COUNT,
};


enum_type( GfxIndexBufferFormat, u8 )
{
	GfxIndexBufferFormat_NONE = 0,
	GfxIndexBufferFormat_U8,
	GfxIndexBufferFormat_U16,
	GfxIndexBufferFormat_U32,
	GFXINDEXBUFFERFORMAT_COUNT,
};


enum_type( GfxCPUAccessMode, u8 )
{
	GfxCPUAccessMode_NONE = 0,
	GfxCPUAccessMode_READ,
	GfxCPUAccessMode_READ_WRITE,
	GfxCPUAccessMode_WRITE,
	GfxCPUAccessMode_WRITE_DISCARD,
	GfxCPUAccessMode_WRITE_NO_OVERWRITE,
	GFXCPUACCESSMODE_COUNT,
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace CoreGfx
{
	extern bool init();
	extern bool free();

	extern bool init_textures();
	extern bool free_textures();

	extern bool init_shaders();
	extern bool free_shaders();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace CoreGfx
{
	extern bool api_init();
	extern bool api_free();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

struct GfxSwapChain
{
	u16 width = WINDOW_WIDTH_DEFAULT;
	u16 height = WINDOW_HEIGHT_DEFAULT;
	bool fullscreen = false;
};


namespace CoreGfx
{
	extern GfxSwapChain swapchain;

	extern bool api_swapchain_init( const u16 width, const u16 height, const bool fullscreen );
	extern bool api_swapchain_free();
	extern bool api_swapchain_resize( const u16 width, const u16 height, const bool fullscreen );
};


namespace Gfx
{
	extern void set_swapchain_size( const u16 width, const u16 height, const bool fullscreen );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct GfxViewport
{
	u16 width = WINDOW_WIDTH_DEFAULT;
	u16 height = WINDOW_HEIGHT_DEFAULT;
	bool fullscreen = false;
};


namespace CoreGfx
{
	extern GfxViewport viewport;

	extern bool api_viewport_init( const u16 width, const u16 height, const bool fullscreen );
	extern bool api_viewport_free();
	extern bool api_viewport_resize( const u16 width, const u16 height, const bool fullscreen );
};


namespace Gfx
{
	extern void viewport_update();
	extern void set_viewport_size( const u16 width, const u16 height, const bool fullscreen );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct GfxRasterState
{
	GfxFillMode fillMode = GfxFillMode_SOLID;
	GfxCullMode cullMode = GfxCullMode_NONE;
	bool scissor = false;
	int scissorX1 = 0;
	int scissorY1 = 0;
	int scissorX2 = 0;
	int scissorY2 = 0;

	bool operator==( const GfxRasterState &other ) const
	{
		return fillMode == other.fillMode &&
			cullMode == other.cullMode &&
			scissor == other.scissor &&
			scissorX1 == other.scissorX1 &&
			scissorY1 == other.scissorY1 &&
			scissorX2 == other.scissorX2 &&
			scissorY2 == other.scissorY2;
	}
};


namespace CoreGfx
{
	bool api_set_raster_state( const GfxRasterState &state );
}


namespace Gfx
{
	extern void set_raster_state( const GfxRasterState &state );
	extern void set_fill_mode( const GfxFillMode &mode );
	extern void set_cull_mode( const GfxCullMode &mode );
	extern void set_scissor( const int x1, const int y1, const int x2, const int y2 );
	extern void set_scissor_nested( const int x1, const int y1, const int x2, const int y2 );
	extern void reset_scissor();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct GfxSamplerState // TODO: Per texture?
{
	GfxFilteringMode filterMode = GfxFilteringMode_NEAREST;
	GfxUVWrapMode wrapMode = GfxUVWrapMode_WRAP;
	int anisotropy = 1;

	bool operator==( const GfxSamplerState &other ) const
	{
		return filterMode == other.filterMode &&
			wrapMode == other.wrapMode  &&
			anisotropy == other.anisotropy;
	}
};


namespace CoreGfx
{
	bool api_set_sampler_state( const GfxSamplerState &state );
}


namespace Gfx
{
	extern void set_sampler_state( const GfxSamplerState &state );
	extern void set_filtering_mode( const GfxFilteringMode &mode );
	extern void set_filtering_anisotropy( const int anisotropy );
	extern void set_uv_wrap_mode( const GfxUVWrapMode &mode );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct GfxBlendState
{
	bool blendEnable = true;
	GfxBlendFactor srcFactorColor = GfxBlendFactor_SRC_ALPHA;
	GfxBlendFactor dstFactorColor = GfxBlendFactor_INV_SRC_ALPHA;
	GfxBlendOperation blendOperationColor = GfxBlendOperation_ADD;
	GfxBlendFactor srcFactorAlpha = GfxBlendFactor_ONE;
	GfxBlendFactor dstFactorAlpha = GfxBlendFactor_INV_SRC_ALPHA;
	GfxBlendOperation blendOperationAlpha = GfxBlendOperation_ADD;
	GfxColorWriteFlag colorWriteMask = GfxColorWriteFlag_ALL;

	bool operator==( const GfxBlendState &other ) const
	{
		return blendEnable == other.blendEnable &&
			srcFactorColor == other.srcFactorColor &&
			dstFactorColor == other.dstFactorColor &&
			blendOperationColor == other.blendOperationColor &&
			srcFactorAlpha == other.srcFactorAlpha &&
			dstFactorAlpha == other.dstFactorAlpha &&
			blendOperationAlpha == other.blendOperationAlpha &&
			colorWriteMask == other.colorWriteMask;
	}
};


namespace CoreGfx
{
	extern bool api_set_blend_state( const GfxBlendState &state );
	extern void api_clear_color( Color color );
}


namespace Gfx
{
	extern void set_blend_state( const GfxBlendState &state );
	extern void set_blend_enabled( const bool enabled );

	extern void set_blend_mode_color( const GfxBlendFactor &srcFactor,
		const GfxBlendFactor &dstFactor, const GfxBlendOperation &op );

	extern void set_blend_mode_alpha( const GfxBlendFactor &srcFactor,
		const GfxBlendFactor &dstFactor, const GfxBlendOperation &op );

	extern void reset_blend_mode();
	extern void reset_blend_mode_color();
	extern void reset_blend_mode_alpha();
	extern void set_color_write_mask( const GfxColorWriteFlag &mask );

	extern void clear_color( const Color color );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct GfxDepthState
{
	GfxDepthTestMode depthTestMode = GfxDepthTestMode_ALWAYS;
	GfxDepthWriteFlag depthWriteMask = GfxDepthWriteFlag_ALL;

	bool operator==( const GfxDepthState &other ) const
	{
		return depthTestMode == other.depthTestMode && depthWriteMask == other.depthWriteMask;
	}
};


namespace CoreGfx
{
	extern bool api_set_depth_state( const GfxDepthState &state );
	extern void api_clear_depth( float depth = 1.0f );
}


namespace Gfx
{
	extern void set_depth_state( const GfxDepthState &state );
	extern void set_depth_test_mode( const GfxDepthTestMode &mode );
	extern void set_depth_write_mask( const GfxDepthWriteFlag &mask );
	extern void clear_depth( float depth = 1.0f );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct GfxShaderState
{
	u32 shaderID = 0;
	GfxShaderResource *resource = nullptr;
	CoreGfxUniformBuffer::PipelineUniforms_t globals;

	bool operator==( const GfxShaderState &other ) const
	{
		return resource == other.resource;
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct GfxState
{
	GfxState() { for( u32 i = 0; i < 32; i++ ) { textureResource[i] = nullptr; } }

	GfxRasterState raster;
	GfxSamplerState sampler;
	GfxBlendState blend;
	GfxDepthState depth;
	GfxShaderState shader;

	GfxTexture2DResource *textureResource[32];
};


namespace CoreGfx
{
	extern GfxState states[2];
	extern bool flip;
	extern bool rendering;

	extern void state_reset();
	extern void state_apply( const bool dirty = false );
};


namespace Gfx
{
	inline GfxState &state() { return CoreGfx::states[CoreGfx::flip]; }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace CoreGfx
{
	extern bool api_shader_init( GfxShaderResource *&resource,
		const u32 shaderID, const struct ShaderEntry &shaderEntry );

	extern bool api_shader_free( GfxShaderResource *&resource );

	extern bool api_shader_bind( GfxShaderResource *&resource );

	extern bool api_shader_dispatch( GfxShaderResource *&resource, const u32 x, const u32 y, const u32 z );


	extern FUNCTION_POINTER_ARRAY( bool, api_shader_bind_uniform_buffers_vertex ); // imp: gfx.generated.cpp
	extern FUNCTION_POINTER_ARRAY( bool, api_shader_bind_uniform_buffers_fragment ); // imp: gfx.generated.cpp
	extern FUNCTION_POINTER_ARRAY( bool, api_shader_bind_uniform_buffers_compute ); // imp: gfx.generated.cpp
}


class GfxShader
{
public:
	void init( const u32 shaderID, const struct ShaderEntry &shaderEntry );
	void free();
	void bind();
	void release();
	void dispatch( const u32 x, const u32 y, const u32 z );

public:
	GfxShaderResource *resource = nullptr;
	u32 shaderID = 0;
};


namespace CoreGfx
{
	extern GfxShader shaders[CoreGfx::shaderCount];
}


namespace Gfx
{
	extern u32 shader_current();

	extern void shader_bind( const Shader shader );

	extern void shader_release();

	extern void shader_dispatch( const Shader shader, const u32 x, const u32 y, const u32 z );

	extern void set_shader_globals( const CoreGfxUniformBuffer::PipelineUniforms_t &globals );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace CoreGfx
{
	extern bool api_vertex_buffer_init_dynamic( GfxVertexBufferResource *&resource, const u32 vertexFormatID,
		const GfxCPUAccessMode accessMode, const u32 size, const u32 stride );

	extern bool api_vertex_buffer_init_static( GfxVertexBufferResource *&resource, const u32 vertexFormatID,
		const GfxCPUAccessMode accessMode, const void *const data, const u32 size, const u32 stride );

	extern bool api_vertex_buffer_free( GfxVertexBufferResource *&resource );

	extern void api_vertex_buffer_write_begin( GfxVertexBufferResource *const resource );

	extern void api_vertex_buffer_write_end( GfxVertexBufferResource *const resource );

	extern bool api_vertex_buffer_write( GfxVertexBufferResource *const resource,
		const void *const data, const u32 size );

	extern u32 api_vertex_buffer_current( const GfxVertexBufferResource *const resource );
}


template <typename VertexFormat> class GfxVertexBuffer
{
public:
	void init( const u32 count, const GfxCPUAccessMode accessMode = GfxCPUAccessMode_WRITE )
	{
		const u32 size = count * sizeof( VertexFormat );
		ErrorIf( !CoreGfx::api_vertex_buffer_init_dynamic( resource,
			CoreGfxVertex::vertex_format_id<VertexFormat>(),
			accessMode, size, sizeof( VertexFormat ) ), "Failed to init vertex buffer!" );
	}

	void free()
	{
		if( resource == nullptr ) { return; }
		ErrorIf( !CoreGfx::api_vertex_buffer_free( resource ), "Failed to free vertex buffer!" );
		resource = nullptr;
	}

	inline u32 current() const
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

	void write( void *data, const usize size )
	{
		Assert( size % sizeof( VertexFormat ) == 0 );
		CoreGfx::api_vertex_buffer_write( resource, data, size );
	}

public:
	GfxVertexBufferResource *resource = nullptr;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace CoreGfx
{
	extern bool api_instance_buffer_init_dynamic( GfxInstanceBufferResource *&resource, const u32 instanceFormatID,
		const GfxCPUAccessMode accessMode, const u32 size, const u32 stride );

	extern bool api_instance_buffer_init_static( GfxInstanceBufferResource *&resource, const u32 instanceFormatID,
		const GfxCPUAccessMode accessMode, const void *const data, const u32 size, const u32 stride );

	extern bool api_instance_buffer_free( GfxInstanceBufferResource *&resource );

	extern void api_instance_buffer_write_begin( GfxInstanceBufferResource *const resource );

	extern void api_instance_buffer_write_end( GfxInstanceBufferResource *const resource );

	extern bool api_instance_buffer_write( GfxInstanceBufferResource *const resource,
		const void *const data, const u32 size );

	extern u32 api_instance_buffer_current( const GfxInstanceBufferResource *const resource );
}


template <typename InstanceType> class GfxInstanceBuffer
{
public:
	void init( const u32 count, const GfxCPUAccessMode accessMode = GfxCPUAccessMode_WRITE )
	{
		const u32 size = count * sizeof( InstanceType );
		ErrorIf( !CoreGfx::api_instance_buffer_init_dynamic( resource,
			CoreGfxInstance::instance_format_id<InstanceType>(),
			accessMode, size, sizeof( InstanceType ) ), "Failed to init instance buffer!" );
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
		static_assert( sizeof( T ) % sizeof( InstanceType ) == 0, "Element size must match InstanceBuffer type" );
		CoreGfx::api_instance_buffer_write( resource, &element, sizeof( element ) );
	}

	void write( void *data, const usize size )
	{
		Assert( size % sizeof( InstanceType ) == 0 );
		CoreGfx::api_instance_buffer_write( resource, data, size );
	}

public:
	GfxInstanceBufferResource *resource = nullptr;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace CoreGfx
{
	extern bool api_index_buffer_init( GfxIndexBufferResource *&resource,
		void *data, const u32 size, const double indicesToVerticiesRatio,
		const GfxIndexBufferFormat format, const GfxCPUAccessMode accessMode );

	extern bool api_index_buffer_free( GfxIndexBufferResource *&resource );
}


class GfxIndexBuffer
{
public:
	void init( void *data, const u32 size, const double indicesToVerticiesRatio,
	           const GfxIndexBufferFormat format, const GfxCPUAccessMode accessMode = GfxCPUAccessMode_WRITE )
	{
		this->indicesToVerticiesRatio = indicesToVerticiesRatio;
		ErrorIf( !CoreGfx::api_index_buffer_init( resource, data, size, indicesToVerticiesRatio, format, accessMode ),
		         "Failed to init index buffer!" );
	}

	void free()
	{
		if( resource == nullptr ) { return; }
		ErrorIf( !CoreGfx::api_index_buffer_free( resource ), "Failed to free index buffer!" );
		resource = nullptr;
	}

public:
	GfxIndexBufferResource *resource = nullptr;

private:
	double indicesToVerticiesRatio = 1.0;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace CoreGfx
{
	extern GfxUniformBufferResource *gfxUniformBufferResources[CoreGfx::uniformBufferCount]; // imp: gfx.generated.cpp

	extern bool api_uniform_buffer_init( GfxUniformBufferResource *&resource,
	                                     const char *name, const int index, const u32 size );
	extern bool api_uniform_buffer_free( GfxUniformBufferResource *&resource );

	extern void api_uniform_buffer_write_begin( GfxUniformBufferResource *const resource );

	extern void api_uniform_buffer_write_end( GfxUniformBufferResource *const resource );

	extern bool api_uniform_buffer_write( GfxUniformBufferResource *const resource, const void *data );

	extern bool api_uniform_buffer_bind_vertex( GfxUniformBufferResource *const resource, const int slot );

	extern bool api_uniform_buffer_bind_fragment( GfxUniformBufferResource *const resource, const int slot );

	extern bool api_uniform_buffer_bind_compute( GfxUniformBufferResource *const resource, const int slot );

	inline bool api_uniform_buffer_bind_all( GfxUniformBufferResource *const resource, const int slot )
	{
		if( !api_uniform_buffer_bind_vertex( resource, slot ) ) { return false; }
		if( !api_uniform_buffer_bind_fragment( resource, slot ) ) { return false; }
		if( !api_uniform_buffer_bind_compute( resource, slot ) ) { return false; }
		return true;
	}

	extern bool api_init_uniform_buffers();

	extern bool api_free_uniform_buffers();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// TODO: GfxConstantBuffer (StructuredBuffer)
// TODO: GfxMutableBuffer (RWStructuredBuffer)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// TODO: GfxTexture1D

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace CoreGfx
{
	extern bool api_texture_2d_init( GfxTexture2DResource *&resource, void *data,
		const u16 width, const u16 height, const u16 levels,
		const GfxColorFormat &format );

	extern bool api_texture_2d_free( GfxTexture2DResource *&resource );

	extern bool api_texture_2d_bind( GfxTexture2DResource *const resource, const int slot );

	extern bool api_texture_2d_release( GfxTexture2DResource *const resource, const int slot );
}


class GfxTexture2D
{
public:
	void init( void *data, const u16 width, const u16 height, const GfxColorFormat &format );
	void init( void *data, const u16 width, const u16 height, const u16 levels, const GfxColorFormat &format );
	void free();
	void bind( const int slot = 0 ) const;
	void release() const;

public:
	GfxTexture2DResource *resource = nullptr;
};


namespace CoreGfx
{
	extern GfxTexture2D textures[CoreAssets::textureCount];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// TODO: GfxTexture2DArray
// TODO: GfxTexture3D
// TODO: GfxTexture3DArray
// TODO: GfxTextureCube
// TODO: GfxTextureCubeArray

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct GfxRenderTargetDescription
{
	GfxColorFormat colorFormat = GfxColorFormat_R8G8B8A8_FLOAT;
	GfxDepthFormat depthFormat = GfxDepthFormat_NONE;
	bool cpuAccess = false;
	int sampleCount = 0;
};


namespace CoreGfx
{
	extern bool api_render_target_2d_init( GfxRenderTarget2DResource *&resource,
		GfxTexture2DResource *&resourceColor,
		GfxTexture2DResource *&resourceDepth,
		const u16 width, const u16 height,
		const GfxRenderTargetDescription &desc = { } );

	extern bool api_render_target_2d_free( GfxRenderTarget2DResource *&resource,
		GfxTexture2DResource *&resourceColor,
		GfxTexture2DResource *&resourceDepth );

	extern bool api_render_target_2d_copy(
		GfxRenderTarget2DResource *&srcResource,
		GfxTexture2DResource *&srcResourceColor, GfxTexture2DResource *&srcResourceDepth,
		GfxRenderTarget2DResource *&dstResource,
		GfxTexture2DResource *&dstResourceColor, GfxTexture2DResource *&dstResourceDepth );

	extern bool api_render_target_2d_copy_part(
		GfxRenderTarget2DResource *&srcResource,
		GfxTexture2DResource *&srcResourceColor, GfxTexture2DResource *&srcResourceDepth,
		GfxRenderTarget2DResource *&dstResource,
		GfxTexture2DResource *&dstResourceColor, GfxTexture2DResource *&dstResourceDepth,
		const u16 srcX, const u16 srcY, const u16 dstX, const u16 dstY, const u16 width, const u16 height );

	extern bool api_render_target_2d_buffer_read_color( GfxRenderTarget2DResource *&resource,
		GfxTexture2DResource *&resourceColor,
		void *buffer, const u32 size );

	extern bool api_render_target_2d_buffer_read_depth( GfxRenderTarget2DResource *&resource,
		GfxTexture2DResource *&resourceDepth,
		void *buffer, const u32 size );

	extern bool api_render_target_2d_buffer_write_color( GfxRenderTarget2DResource *&resource,
		GfxTexture2DResource *&resourceColor,
		const void *const buffer, const u32 size );

	extern bool api_render_target_2d_buffer_write_depth( GfxRenderTarget2DResource *&resource,
		GfxTexture2DResource *&resourceDepth,
		const void *const buffer, const u32 size );

	extern bool api_render_target_2d_bind( GfxRenderTarget2DResource *const resource, const int slot );

	extern bool api_render_target_2d_release( GfxRenderTarget2DResource *const resource, const int slot );
}


class GfxRenderTarget2D
{
public:
	void init( const u16 width, const u16 height, const GfxRenderTargetDescription &desc );
	void free();
	void copy( GfxRenderTarget2D &source );
	void copy_part( GfxRenderTarget2D &source, int srcX, int srcY, int dstX, int dstY, u16 width, u16 height );
	void resize( const u16 width, const u16 height );
	void bind( const int slot = 0 );
	void release();

public:
	GfxRenderTarget2DResource *resource = nullptr;
	GfxRenderTargetDescription desc;
	GfxTexture2D textureColor;
	GfxTexture2D textureDepth;
	u16 width, height;
	int slot = -1;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace CoreGfx
{
	extern bool api_draw(
		const GfxVertexBufferResource *const resourceVertex, const u32 vertexCount,
		const GfxInstanceBufferResource *const resourceInstance, const u32 instanceCount,
		const GfxIndexBufferResource *const resourceIndex,
		const GfxPrimitiveType type );
}


namespace Gfx
{
	inline void draw_vertices(
		const u32 vertexCount,
		const GfxPrimitiveType type = GfxPrimitiveType_TriangleList )
	{
		CoreGfx::api_draw(
			nullptr, vertexCount,
			nullptr, 0,
			nullptr, type );
	}


	inline void draw_vertices_instanced(
		const u32 vertexCount,
		const u32 instanceCount,
		const GfxPrimitiveType type = GfxPrimitiveType_TriangleList )
	{
		CoreGfx::api_draw(
			nullptr, vertexCount,
			nullptr, instanceCount,
			nullptr, type );
	}


	template <typename InstanceFormat>
	inline void draw_vertices_instanced(
		const u32 vertexCount,
		const GfxInstanceBuffer<InstanceFormat> &instanceBuffer,
		const GfxPrimitiveType type = GfxPrimitiveType_TriangleList )
	{
		const u32 instanceCount = instanceBuffer.current() / sizeof( InstanceFormat );
		CoreGfx::api_draw(
			nullptr, vertexCount,
			instanceBuffer.resource, instanceCount,
			nullptr, type );
	}


	inline void draw_vertices_indexed(
		const u32 vertexCount,
		const GfxIndexBuffer &indexBuffer,
		const GfxPrimitiveType type = GfxPrimitiveType_TriangleList )
	{
		CoreGfx::api_draw(
			nullptr, vertexCount,
			nullptr, 0,
			indexBuffer.resource, type );
	}


	inline void draw_vertices_instanced_indexed(
		const u32 vertexCount,
		const u32 instanceCount,
		const GfxIndexBuffer &indexBuffer,
		const GfxPrimitiveType type = GfxPrimitiveType_TriangleList )
	{
		CoreGfx::api_draw(
			nullptr, vertexCount,
			nullptr, instanceCount,
			indexBuffer.resource, type );
	}


	template <typename InstanceFormat>
	inline void draw_vertices_instanced_indexed(
		const u32 vertexCount,
		const GfxInstanceBuffer<InstanceFormat> &instanceBuffer,
		const GfxIndexBuffer &indexBuffer,
		const GfxPrimitiveType type = GfxPrimitiveType_TriangleList )
	{
		const u32 instanceCount = instanceBuffer.current() / sizeof( InstanceFormat );
		CoreGfx::api_draw(
			nullptr, vertexCount,
			instanceBuffer.resource, instanceCount,
			indexBuffer.resource, type );
	}


	template <typename VertexFormat>
	inline void draw_vertex_buffer(
		const GfxVertexBuffer<VertexFormat> &vertexBuffer,
		const GfxPrimitiveType type = GfxPrimitiveType_TriangleList )
	{
		const u32 vertexCount = vertexBuffer.current() / sizeof( VertexFormat );
		CoreGfx::api_draw( vertexBuffer.resource, vertexCount,
			nullptr, 0,
			nullptr, type );
	}


	template <typename VertexFormat, typename InstanceFormat>
	inline void draw_vertex_buffer_instanced(
		const GfxVertexBuffer<VertexFormat> &vertexBuffer,
		const GfxInstanceBuffer<InstanceFormat> &instanceBuffer,
		const GfxPrimitiveType type = GfxPrimitiveType_TriangleList )
	{
		const u32 vertexCount = vertexBuffer.current() / sizeof( VertexFormat );
		const u32 instanceCount = instanceBuffer.current() / sizeof( InstanceFormat );
		CoreGfx::api_draw( vertexBuffer.resource, vertexCount,
			instanceBuffer.resource, instanceCount,
			nullptr, type );
	}


	template <typename VertexFormat>
	inline void draw_vertex_buffer_instanced(
		const GfxVertexBuffer<VertexFormat> &vertexBuffer,
		const u32 instanceCount,
		const GfxPrimitiveType type = GfxPrimitiveType_TriangleList )
	{
		const u32 vertexCount = vertexBuffer.current() / sizeof( VertexFormat );
		CoreGfx::api_draw( vertexBuffer.resource, vertexCount,
			nullptr, instanceCount,
			nullptr, type );
	}


	template <typename VertexFormat>
	inline void draw_vertex_buffer_indexed(
		const GfxVertexBuffer<VertexFormat> &vertexBuffer,
		const GfxIndexBuffer &indexBuffer,
		const GfxPrimitiveType type = GfxPrimitiveType_TriangleList )
	{
		const u32 vertexCount = vertexBuffer.current() / sizeof( VertexFormat );
		CoreGfx::api_draw( vertexBuffer.resource, vertexCount,
			nullptr, 0,
			indexBuffer.resource, type );
	}


	template <typename VertexFormat, typename InstanceFormat>
	inline void draw_vertex_buffer_instanced_indexed(
		const GfxVertexBuffer<VertexFormat> &vertexBuffer,
		const GfxInstanceBuffer<InstanceFormat> &instanceBuffer,
		const GfxIndexBuffer &indexBuffer,
		const GfxPrimitiveType type = GfxPrimitiveType_TriangleList )
	{
		const u32 vertexCount = vertexBuffer.current() / sizeof( VertexFormat );
		const u32 instanceCount = instanceBuffer.current() / sizeof( InstanceFormat );
		CoreGfx::api_draw( vertexBuffer.resource, vertexCount,
			instanceBuffer.resource, instanceCount,
			indexBuffer.resource, type );
	}


	template <typename VertexFormat>
	inline void draw_vertex_buffer_instanced_indexed(
		const GfxVertexBuffer<VertexFormat> &vertexBuffer,
		const u32 instanceCount,
		const GfxIndexBuffer &indexBuffer,
		const GfxPrimitiveType type = GfxPrimitiveType_TriangleList )
	{
		const u32 vertexCount = vertexBuffer.current() / sizeof( VertexFormat );
		CoreGfx::api_draw( vertexBuffer.resource, vertexCount,
			nullptr, instanceCount,
			indexBuffer.resource, type );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace CoreGfx
{
	extern double_m44 matrixModel;
	extern double_m44 matrixView;
	extern double_m44 matrixPerspective;
	extern double_m44 matrixMVP;
}


namespace CoreGfx
{
	extern void update_matrix_mvp();
}


namespace Gfx
{
	extern void set_matrix_mvp_2d_orthographic( const double x, const double y,
		const double zoom, const double angle,
		const double width, const double height,
		const double znear = 0.0f, const double zfar = 1.0f );

	extern void set_matrix_mvp_3d_perspective( const double fov, const double aspect,
		const double znear, const double zfar,
		const double x, const double y, const double z,
		const double xto, const double yto, const double zto,
		const double xup, const double yup, const double zup );

	extern void set_matrix_model( const double_m44 &matrix );

	extern void set_matrix_view( const double_m44 &matrix );

	extern void set_matrix_perspective( const double_m44 &matrix );

	extern void set_matrix_mvp( const double_m44 &matModel, const double_m44 &matView,
		const double_m44 &matPerspective );

	extern const double_m44 &get_matrix_model();

	extern const double_m44 &get_matrix_view();

	extern const double_m44 &get_matrix_perspective();

	extern const double_m44 &get_matrix_mvp();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Gfx
{
	extern u16 mip_level_count_2d( u16 width, u16 height );

	extern bool mip_level_validate_2d( u16 width, u16 height, u16 levels );

	extern usize mip_buffer_size_2d( u16 width, u16 height, u16 levels,
		const GfxColorFormat format );

	extern bool mip_generate_next_2d( void *data, const u16 width, const u16 height,
		const GfxColorFormat format, void *dest, const usize size );

	extern bool mip_generate_next_2d_alloc( void *data, const u16 width, const u16 height,
		const GfxColorFormat format, void *&outData, usize &outSize );

	extern bool mip_generate_chain_2d( void *data, const u16 width, const u16 height,
		const GfxColorFormat format, void *dest, const usize size );

	extern bool mip_generate_chain_2d_alloc( void *data, const u16 width, const u16 height,
		const GfxColorFormat format, void *&outData, usize &outSize );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename VertexFormat> class GfxQuadBatch
{
public:
	struct Quad { VertexFormat v0, v1, v2, v3; };

public:
	bool init( const u32 capacity )
	{
		Assert( capacity * 6 < U32_MAX );
		this->capacity = capacity;

#if GRAPHICS_ENABLED
		vertexBuffer.init( this->capacity * 6, GfxCPUAccessMode_WRITE_DISCARD );

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

		indexBuffer.init( indices, size, 6.0f / 4.0f, GfxIndexBufferFormat_U32 );
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
		Gfx::draw_vertex_buffer_indexed( vertexBuffer, indexBuffer );
#endif
	}

	bool batch_break()
	{
#if GRAPHICS_ENABLED
		if( vertexBuffer.current() > 0 )
		{
			batch_end();
			batch_begin();
			return true;
		}
#endif
		return false;
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

	void write(
		const VertexFormat &v0,
		const VertexFormat &v1,
		const VertexFormat &v2,
		const VertexFormat &v3 )
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
	u32 capacity = 0;
	GfxVertexBuffer<VertexFormat> vertexBuffer;
	GfxIndexBuffer indexBuffer;
};


namespace CoreGfx
{
	extern GfxQuadBatch<GfxVertex::BuiltinVertex> quadBatch;
}


namespace Gfx
{
	extern bool quad_batch_break();
	extern bool quad_batch_can_break();
	extern void quad_batch_break_check();

	extern void quad_batch_write( const GfxQuadBatch<GfxVertex::BuiltinVertex>::Quad &quad,
		const GfxTexture2D *const texture = nullptr );

	extern void quad_batch_write( const float x1, const float y1, const float x2, const float y2,
		const u16 u1, const u16 v1, const u16 u2, const u16 v2,
		const Color c1, const Color c2, const Color c3, const Color c4,
		const GfxTexture2D *const texture = nullptr, const float depth = 0.0f );

	extern void quad_batch_write( const float x1, const float y1, const float x2, const float y2,
		const float x3, const float y3, const float x4, const float y4,
		const u16 u1, const u16 v1, const u16 u2, const u16 v2,
		const Color c1, const Color c2, const Color c3, const Color c4,
		const GfxTexture2D *const texture = nullptr, const float depth = 0.0f );

	extern void quad_batch_write( const float x1, const float y1, const float x2, const float y2,
		const u16 u1, const u16 v1, const u16 u2, const u16 v2, const Color color,
		const GfxTexture2D *const texture = nullptr, const float depth = 0.0f );

	extern void quad_batch_write( const float x1, const float y1, const float x2, const float y2,
		const float x3, const float y3, const float x4, const float y4,
		const u16 u1, const u16 v1, const u16 u2, const u16 v2, const Color color,
		const GfxTexture2D *const texture = nullptr, const float depth = 0.0f );
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace CoreGfx
{
	extern byte *scratch_buffer( const usize size );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////