#pragma once

#include <gfx.generated.hpp>

#include <config.hpp>

#include <core/debug.hpp>
#include <core/types.hpp>

#include <manta/assets.hpp>
#include <manta/color.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//
// 'Gfx'  = Graphics API (Public)
// 'SysGfx' = Engine Graphics (Internal)
// 'GfxCore' = Core/Backend Graphics (Internal)
//

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
	usize gpuMemoryFramebuffer = 0;
	usize gpuMemoryTextures = 0;
	usize gpuMemoryRenderTargets = 0;
	usize gpuMemoryVertexBuffers = 0;
	usize gpuMemoryIndexBuffers = 0;
	usize gpuMemoryConstantBuffers = 0;
	usize gpuMemoryShaderPrograms = 0;

	usize total_memory() const
	{
		return gpuMemoryFramebuffer +
		       gpuMemoryTextures +
		       gpuMemoryRenderTargets +
		       gpuMemoryVertexBuffers +
		       gpuMemoryIndexBuffers +
		       gpuMemoryConstantBuffers +
			   gpuMemoryShaderPrograms;
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

using GfxResourceID = u32;
#define GFX_RESOURCE_ID_NULL ( U32_MAX )


struct GfxResource
{
	GfxResourceID id = GFX_RESOURCE_ID_NULL;
};


#define GFX_RESOURCE_COUNT_VERTEX_BUFFER    ( 256 * 256 )
#define GFX_RESOURCE_COUNT_INDEX_BUFFER     ( 256 * 256 )
#define GFX_RESOURCE_COUNT_CONSTANT_BUFFER  ( 256 * 256 )
#define GFX_RESOURCE_COUNT_SHADER           ( 256 * 256 )
#define GFX_RESOURCE_COUNT_TEXTURE_1D       ( 1024 )
#define GFX_RESOURCE_COUNT_TEXTURE_2D       ( 1024 )
#define GFX_RESOURCE_COUNT_TEXTURE_3D       ( 1024 )
#define GFX_RESOURCE_COUNT_RENDER_TARGET_1D ( 1024 )
#define GFX_RESOURCE_COUNT_RENDER_TARGET_2D ( 1024 )
#define GFX_RESOURCE_COUNT_RENDER_TARGET_3D ( 1024 )


struct GfxIndexBufferResource;
struct GfxVertexBufferResource;
struct GfxConstantBufferResource;
struct GfxShaderResource;
struct GfxTexture1DResource;
struct GfxTexture2DResource;
struct GfxTexture3DResource;
struct GfxRenderTarget1DResource;
struct GfxRenderTarget2DResource;
struct GfxRenderTarget3DResource;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct GfxBuiltInQuad
{
	GfxVertex::BuiltinVertex v1, v2, v3, v4;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum_type( GfxColorFormat, u8 )
{
	GfxColorFormat_NONE = 0,
	GfxColorFormat_R8G8B8A8,
	GfxColorFormat_R10G10B10A2,
	GfxColorFormat_R8,
	GfxColorFormat_R8G8,
	GfxColorFormat_R16,
	GfxColorFormat_R16G16,
	GfxColorFormat_R32,
	GFXCOLORFORMAT_COUNT,
};


namespace GfxCore
{
	constexpr u32 colorFormatPixelSizeBytes[GFXCOLORFORMAT_COUNT] =
	{
		0, // GfxColorFormat_NONE
		4, // GfxColorFormat_R8G8B8A8
		4, // GfxColorFormat_R10G10B10A2
		1, // GfxColorFormat_R8
		2, // GfxColorFormat_R8G8
		2, // GfxColorFormat_R16
		4, // GfxColorFormat_R16G16
		4, // GfxColorFormat_R32
	};
	static_assert( ARRAY_LENGTH( colorFormatPixelSizeBytes ) == GFXCOLORFORMAT_COUNT, "Missing colorFormatPixelSizeBytes!" );
}


#define GFX_SIZE_IMAGE_COLOR_BYTES( width, height, depth, format ) \
	( width * height * depth * GfxCore::colorFormatPixelSizeBytes[ format ] )


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


namespace GfxCore
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
	static_assert( ARRAY_LENGTH( colorFormatPixelSizeBytes ) == GFXCOLORFORMAT_COUNT,
		"Missing depthFormatPixelSizeBytes!" );
}


#define GFX_SIZE_IMAGE_DEPTH_BYTES( width, height, depth, format ) \
	( width * height * depth * GfxCore::depthFormatPixelSizeBytes[ format ] )


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

struct GfxSwapChain
{
	u16 width = WINDOW_WIDTH_DEFAULT;
	u16 height = WINDOW_HEIGHT_DEFAULT;
	bool fullscreen = false;
};


namespace GfxCore
{
	extern bool rb_swapchain_init( const u16 width, const u16 height, const bool fullscreen );
	extern bool rb_swapchain_free();
	extern bool rb_swapchain_resize( const u16 width, const u16 height, const bool fullscreen );
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct GfxViewport
{
	u16 width = WINDOW_WIDTH_DEFAULT;
	u16 height = WINDOW_HEIGHT_DEFAULT;
	bool fullscreen = false;
};


namespace GfxCore
{
	extern bool rb_viewport_init( const u16 width, const u16 height, const bool fullscreen );
	extern bool rb_viewport_free();
	extern bool rb_viewport_resize( const u16 width, const u16 height, const bool fullscreen );
};

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


struct GfxSamplerState // TODO: Per texture?
{
	GfxFilteringMode filterMode = GfxFilteringMode_NEAREST;
	GfxUVWrapMode wrapMode = GfxUVWrapMode_WRAP;

	bool operator==( const GfxSamplerState &other ) const
	{
		return filterMode == other.filterMode && wrapMode == other.wrapMode;
	}
};


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


struct GfxDepthState
{
	GfxDepthTestMode depthTestMode = GfxDepthTestMode_ALWAYS;
	GfxDepthWriteFlag depthWriteMask = GfxDepthWriteFlag_ALL;

	bool operator==( const GfxDepthState &other ) const
	{
		return depthTestMode == other.depthTestMode && depthWriteMask == other.depthWriteMask;
	}
};


struct GfxShaderState
{
	GfxShaderResource *resource = nullptr;
	GfxCoreCBuffer::ShaderGlobals_t globals;

	bool operator==( const GfxShaderState &other ) const
	{
		return resource == other.resource;
	}
};


namespace GfxCore
{
	bool rb_set_raster_state( const GfxRasterState &state );
	bool rb_set_sampler_state( const GfxSamplerState &state );
	bool rb_set_blend_state( const GfxBlendState &state );
	bool rb_set_depth_state( const GfxDepthState &state );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace GfxCore
{
	extern bool rb_index_buffer_init( GfxIndexBufferResource *&resource,
	                                  void *data, const u32 size, const double indToVertRatio,
	                                  const GfxIndexBufferFormat format, const GfxCPUAccessMode accessMode );
	extern bool rb_index_buffer_free( GfxIndexBufferResource *&resource );
}


struct GfxIndexBuffer
{
	GfxIndexBufferResource *resource = nullptr;

	void init( void *data, const u32 size, const double indToVertRatio,
	           const GfxIndexBufferFormat format, const GfxCPUAccessMode accessMode = GfxCPUAccessMode_WRITE )
	{
		ErrorIf( !GfxCore::rb_index_buffer_init( resource, data, size, indToVertRatio, format, accessMode ),
		         "Failed to init index buffer!" );
	}

	void free()
	{
		if( resource == nullptr ) { return; }
		ErrorIf( !GfxCore::rb_index_buffer_free( resource ), "Failed to free index buffer!" );
		resource = nullptr;
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace GfxCore
{
	extern bool rb_vertex_buffer_init_dynamic( GfxVertexBufferResource *&resource, const u32 vertexFormatID,
	                                           const GfxCPUAccessMode accessMode, const u32 size, const u32 stride );

	extern bool rb_vertex_buffer_init_static( GfxVertexBufferResource *&resource, const u32 vertexFormatID,
	                                          const GfxCPUAccessMode accessMode, const void *const data, const u32 size,
	                                          const u32 stride );

	extern bool rb_vertex_buffer_free( GfxVertexBufferResource *&resource );

	extern bool rb_vertex_buffer_draw( GfxVertexBufferResource *&resource );
	extern bool rb_vertex_buffer_draw_indexed( GfxVertexBufferResource *&resource,
	                                           GfxIndexBufferResource *&resourceIndexBuffer );

	extern void rb_vertex_buffer_write_begin( GfxVertexBufferResource *&resource );
	extern void rb_vertex_buffer_write_end( GfxVertexBufferResource *&resource );
	extern bool rb_vertex_buffer_write( GfxVertexBufferResource *&resource, const void *const data, const u32 size );

	extern u32 rb_vertex_buffer_current( GfxVertexBufferResource *&resource );
}


template <typename VertexType>
struct GfxVertexBuffer
{
	GfxVertexBufferResource *resource = nullptr;

	void init( const u32 count, const GfxCPUAccessMode accessMode = GfxCPUAccessMode_WRITE )
	{
		const u32 size = count * sizeof( VertexType );
		ErrorIf( !GfxCore::rb_vertex_buffer_init_dynamic( resource, GfxCoreVertex::vertex_format_id<VertexType>(),
			accessMode, size, sizeof( VertexType ) ), "Failed to init vertex buffer!" );
	}

	void free()
	{
		if( resource == nullptr ) { return; }
		ErrorIf( !GfxCore::rb_vertex_buffer_free( resource ), "Failed to free vertex buffer!" );
		resource = nullptr;
	}

	inline void draw()
	{
		GfxCore::rb_vertex_buffer_draw( resource );
	}

	inline void draw( GfxIndexBuffer &indexBuffer )
	{
		GfxCore::rb_vertex_buffer_draw_indexed( resource, indexBuffer.resource );
	}

	inline u32 current()
	{
		return GfxCore::rb_vertex_buffer_current( resource );
	}

	void write_begin()
	{
		GfxCore::rb_vertex_buffer_write_begin( resource );
	}

	void write_end()
	{
		GfxCore::rb_vertex_buffer_write_end( resource );
	}

	template <typename T> void write( const T &element )
	{
		GfxCore::rb_vertex_buffer_write( resource, &element, sizeof( element ) );
	}

	void write( void *data, const usize size )
	{
		GfxCore::rb_vertex_buffer_write( resource, data, size );
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace GfxCore
{
	extern bool rb_constant_buffer_init( GfxConstantBufferResource *&resource,
	                                     const char *name, const int index, const u32 size );
	extern bool rb_constant_buffer_free( GfxConstantBufferResource *&resource );

	extern void rb_constant_buffer_write_begin( GfxConstantBufferResource *&resource );
	extern void rb_constant_buffer_write_end( GfxConstantBufferResource *&resource );
	extern bool rb_constant_buffer_write( GfxConstantBufferResource *&resource, const void *data );

	extern bool rb_constant_buffer_bind_vertex( GfxConstantBufferResource *&resource, const int slot );
	extern bool rb_constant_buffer_bind_fragment( GfxConstantBufferResource *&resource, const int slot );
	extern bool rb_constant_buffer_bind_compute( GfxConstantBufferResource *&resource, const int slot );

	inline bool rb_constant_buffer_bind_all( GfxConstantBufferResource *&resource, const int slot )
	{
		if( !rb_constant_buffer_bind_vertex( resource, slot ) ) { return false; }
		if( !rb_constant_buffer_bind_fragment( resource, slot ) ) { return false; }
		if( !rb_constant_buffer_bind_compute( resource, slot ) ) { return false; }
		return true;
	}

	// gfx.generated.cpp
	extern GfxConstantBufferResource *gfxCBufferResources[GfxCore::constantBufferCount];
	extern bool rb_init_cbuffers();
	extern bool rb_free_cbuffers();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct GfxTexture2D
{
	GfxTexture2DResource *resource = nullptr;

	void init( void *data, const u16 width, const u16 height, const GfxColorFormat &format );
	void free();
	void bind( const int slot = 0 ) const;
	void release() const;
};

#if 0
struct GfxTexture1D
{
	GfxTexture1DResource *resource = nullptr;

	void init( void *data, const u16 width, const GfxColorFormat &format );
	void free();
	void bind( const int slot = 0 ) const;
	void release() const;
};

struct GfxTexture3D
{
	GfxTexture3DResource *resource = nullptr;

	void init( void *data, const u16 width, const u16 height, const u16 depth, const GfxColorFormat &format );
	void free();
	void bind( const int slot = 0 ) const;
	void release() const;
};
#endif

namespace GfxCore
{
	extern bool rb_texture_2d_init( GfxTexture2DResource *&resource, void *data,
	                                const u16 width, const u16 height,
	                                const GfxColorFormat &format );

	extern bool rb_texture_2d_free( GfxTexture2DResource *&resource );

	extern bool rb_texture_2d_bind( const GfxTexture2DResource *const &resource, const int slot );

	extern bool rb_texture_2d_release( const int slot );

	#if 0
	extern bool rb_texture_1d_init( GfxTexture1DResource *&texture1DResource,
	                                void *data, const u16 width, const GfxColorFormat &format );

	extern bool rb_texture_3d_init( GfxTexture3DResource *&texture3DResource,
	                                void *data, const u16 width, const u16 height, const u16 depth,
	                                const GfxColorFormat &format );

	extern bool rb_texture_1d_free( GfxTexture1DResource *&texture1DResource );

	extern bool rb_texture_3d_free( GfxTexture3DResource *&texture3DResource );

	extern bool rb_texture_1d_bind( const GfxTexture1DResource *const &texture1DResource, const int slot );

	extern bool rb_texture_3d_bind( const GfxTexture3DResource *const &texture3DResource, const int slot );

	extern bool rb_texture_1d_release( const int slot );

	extern bool rb_texture_3d_release( const int slot );
	#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct GfxRenderTargetDescription
{
	GfxColorFormat colorFormat = GfxColorFormat_R8G8B8A8;
	GfxDepthFormat depthFormat = GfxDepthFormat_NONE;
	bool cpuAccess = false;
};


struct GfxRenderTarget2D
{
	GfxRenderTarget2DResource *resource = nullptr;
	GfxTexture2D textureColor;
	GfxTexture2D textureDepth;
	u16 width, height;

	void init( const u16 width, const u16 height, const GfxRenderTargetDescription &desc );
	void free();
	void bind( const int slot = 0 ) const;
	void release() const;
};


#if 0
struct GfxRenderTarget1D
{
	GfxRenderTarget1DResource *resource = nullptr;
	GfxTexture1D textureColor;
	GfxTexture1D textureDepth;
	u16 width;

	void init( const u16 width, const GfxRenderTargetDescription &desc );
	void free();
	void bind( const int slot = 0 ) const;
	void release() const;
}


struct GfxRenderTarget3D
{
	GfxRenderTarget3DResource *resource = nullptr;
	GfxTexture3D textureColor;
	GfxTexture3D textureDepth;
	u16 width, height, depth;

	void init( const u16 width, const u16 height, const u16 depth, GfxRenderTargetDescription &desc );
	void free();
	void bind( const int slot = 0 ) const;
	void release() const;
};
#endif

namespace GfxCore
{
	extern bool rb_render_target_2d_init( GfxRenderTarget2DResource *&resource,
	                                      GfxTexture2DResource *&resourceColor,
	                                      GfxTexture2DResource *&resourceDepth,
	                                      const u16 width, const u16 height,
	                                      const GfxRenderTargetDescription &desc = { } );

	extern bool rb_render_target_2d_free( GfxRenderTarget2DResource *&resource,
	                                      GfxTexture2DResource *&resourceColor,
	                                      GfxTexture2DResource *&resourceDepth );

	extern bool rb_render_target_2d_bind( const GfxRenderTarget2DResource *const &resource, const int slot );

	extern bool rb_render_target_2d_release();

	extern bool rb_render_target_2d_buffer_read_color( GfxRenderTarget2DResource *&resource,
	                                                   GfxTexture2DResource *&resourceColor,
													   void *buffer, const u32 size );

	extern bool rb_render_target_2d_buffer_read_depth( GfxRenderTarget2DResource *&resource,
	                                                   GfxTexture2DResource *&resourceDepth,
	                                                   void *buffer, const u32 size );

	extern bool rb_render_target_2d_buffer_write_color( GfxRenderTarget2DResource *&resource,
	                                                    GfxTexture2DResource *&resourceColor,
	                                                    const void *const buffer, const u32 size );

	extern bool rb_render_target_2d_buffer_write_depth( GfxRenderTarget2DResource *&resource,
	                                                    GfxTexture2DResource *&resourceDepth,
	                                                    const void *const buffer, const u32 size );

	#if 0
	extern bool rb_render_target_1d_init( GfxRenderTarget1DResource *&renderTarget1DResource,
	                                      const u16 width, const GfxRenderTargetDescription &desc = { } );

	extern bool rb_render_target_3d_init( GfxRenderTarget3DResource *&renderTarget3DResource,
	                                      const u16 width, const u16 height, const u16 depth,
	                                      const GfxRenderTargetDescription &desc = { } );

	extern bool rb_render_target_1d_free( GfxRenderTarget1DResource *&renderTarget1DResource );

	extern bool rb_render_target_3d_free( GfxRenderTarget3DResource *&renderTarget3DResource );

	extern bool rb_render_target_1d_bind( const GfxRenderTarget1DResource *const &renderTarget1DResource,
	                                      const int slot );

	extern bool rb_render_target_3d_bind( const GfxRenderTarget3DResource *const &renderTarget3DResource,
	                                     const int slot );

	extern bool rb_renter_target_1d_release();
	extern bool rb_renter_target_3d_release();
	#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct GfxShader
{
	GfxShaderResource *resource = nullptr;
	u32 shaderID = 0;

	void init( const u32 shaderID, const struct DiskShader &diskShader );
	void free();
	void bind();
	void release();
};


namespace GfxCore
{
	extern bool rb_shader_init( GfxShaderResource *&resource, const u32 shaderID, const struct DiskShader &diskShader );
	extern bool rb_shader_free( GfxShaderResource *&resource );
	extern bool rb_shader_bind( GfxShaderResource *&resource );

	// gfx.generated.cpp
	extern FUNCTION_POINTER_ARRAY( bool, rb_shader_bind_constant_buffers_vertex );
	extern FUNCTION_POINTER_ARRAY( bool, rb_shader_bind_constant_buffers_fragment );
	extern FUNCTION_POINTER_ARRAY( bool, rb_shader_bind_constant_buffers_compute );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct GfxState
{
	GfxState()
	{
		for( u32 i = 0; i < 32; i++ ) { textureResource[i] = nullptr; }
	}

	GfxRasterState raster;
	GfxSamplerState sampler;
	GfxBlendState blend;
	GfxDepthState depth;
	GfxShaderState shader;

	GfxTexture2DResource *textureResource[32];
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace GfxCore
{
	extern GfxState states[2];
	extern bool flip;
	extern bool rendering;

	extern GfxTexture2D textures[Assets::texturesCount];
	extern GfxShader shaders[Gfx::shadersCount];

	extern GfxSwapChain swapchain;
	extern GfxViewport viewport;

	extern Matrix matrixModel;
	extern Matrix matrixView;
	extern Matrix matrixPerspective;
	extern Matrix matrixMVP;

	extern bool rb_init();
	extern bool rb_free();

	extern void rb_frame_begin();
	extern void rb_frame_end();

	extern void rb_clear_color( Color color );
	extern void rb_clear_depth( float depth = 1.0f );
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace SysGfx
{
	extern bool init();
	extern bool free();

	extern bool init_textures();
	extern bool free_textures();

	extern bool init_shaders();
	extern bool free_shaders();

	extern void state_reset();
	extern void state_apply( const bool dirty = false );

	// Builtin Quad Batch
	extern GfxIndexBuffer quadBatchIndexBuffer;
	extern GfxVertexBuffer<GfxVertex::BuiltinVertex> quadBatchVertexBuffer;
	extern bool quad_batch_init();
	extern bool quad_batch_free();
	extern void quad_batch_begin();
	extern void quad_batch_end();
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Gfx
{
	inline GfxState &state() { return GfxCore::states[GfxCore::flip]; }
	extern void set_state( const GfxState &state );

	extern void viewport_update();

	extern void frame_begin();
	extern void frame_end();

	extern void clear_color( const Color color );
	extern void clear_depth( float depth = 1.0f );

	extern void set_swapchain_size( const u16 width, const u16 height, const bool fullscreen );
	extern void set_viewport_size( const u16 width, const u16 height, const bool fullscreen );

	extern void set_shader_globals( const GfxCoreCBuffer::ShaderGlobals_t &globals );

	extern void set_matrix_model( const Matrix &matrix );
	extern void set_matrix_view( const Matrix &matrix );
	extern void set_matrix_perspective( const Matrix &matrix );
	extern void set_matrix_mvp( const Matrix &matModel, const Matrix &matView, const Matrix &matPerspective );

	extern void set_matrix_mvp_2d_orthographic( const float x, const float y,
	                                            const float zoom, const float angle,
                                                const float width, const float height,
	                                            const float znear = 0.0f, const float zfar = 1.0f );

	extern void set_matrix_mvp_3d_perspective( const float fov, const float aspect,
	                                           const float znear, const float zfar,
	                                           const float x, const float y, const float z,
	                                           const float xto, const float yto, const float zto,
	                                           const float xup, const float yup, const float zup );

	extern const Matrix &get_matrix_model();
	extern const Matrix &get_matrix_view();
	extern const Matrix &get_matrix_perspective();
	extern const Matrix &get_matrix_mvp();

	extern void set_raster_state( const GfxRasterState &state );
	extern void set_fill_mode( const GfxFillMode &mode );
	extern void set_cull_mode( const GfxCullMode &mode );
	extern void set_scissor( const int x1, const int y1, const int x2, const int y2 );
	extern void set_scissor_nested( const int x1, const int y1, const int x2, const int y2 );
	extern void reset_scissor();

	extern void set_sampler_state( const GfxSamplerState &state );
	extern void set_filtering_mode( const GfxFilteringMode &mode );
	extern void set_uv_wrap_mode( const GfxUVWrapMode &mode );

	extern void set_blend_state( const GfxBlendState &state );
	extern void set_blend_enabled( const bool enabled );

	extern void set_blend_mode_color( const GfxBlendFactor &srcFactor,
	                                  const GfxBlendFactor &dstFactor, const GfxBlendOperation &op );

	extern void set_blend_mode_alpha( const GfxBlendFactor &srcFactor,
	                                  const GfxBlendFactor &dstFactor, const GfxBlendOperation &op );

	extern void set_color_write_mask( const GfxColorWriteFlag &mask );

	extern void set_depth_state( const GfxDepthState &state );
	extern void set_depth_test_mode( const GfxDepthTestMode &mode );
	extern void set_depth_write_mask( const GfxDepthWriteFlag &mask );

	inline void shader_bind( const u32 shader ) { GfxCore::shaders[shader].bind(); }
	inline void shader_release() { GfxCore::shaders[SHADER_DEFAULT].bind(); }

	// Builtin Quad Batch
	extern bool quad_batch_break();
	extern bool quad_batch_can_break();
	extern void quad_batch_break_check();

	extern void quad_batch_write( const GfxBuiltInQuad &quad, const GfxTexture2D *const texture = nullptr );

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