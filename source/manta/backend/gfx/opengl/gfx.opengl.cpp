#include <manta/gfx.hpp>

#include <manta/backend/gfx/opengl/opengl.hpp>
#include <manta/backend/gfx/gfxfactory.hpp>
#include <gfx.api.generated.hpp>

#include <config.hpp>

#include <core/traits.hpp>
#include <core/memory.hpp>
#include <core/list.hpp>
#include <core/hashmap.hpp>
#include <core/checksum.hpp>

#include <manta/window.hpp>
#include <manta/math.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define GL_NULL ( 0 )

static bool OpenGLErrorDetected = false;
#define CHECK_ERROR(msg,...) \
	{ \
		int error = glGetError(); \
		if( error != GL_NO_ERROR && !OpenGLErrorDetected ) \
		{ \
			OpenGLErrorDetected = true; \
			Error( msg " (%d)\n", error, __VA_ARGS__ ); \
		} \
	}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static GfxShaderResource *boundShaderResource = nullptr;

static HashMap<u32, GLuint> constantBufferUniformBlockIndices;
static HashMap<u32, GLint> texture2DUniformLocations;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct OpenGLColorFormat
{
	OpenGLColorFormat( const GLenum format, const GLenum formatInternal, const GLenum formatType ) :
		format{ format }, formatInternal{ formatInternal }, formatType{ formatType } { }

	GLenum format, formatInternal, formatType;
};


static const OpenGLColorFormat OpenGLColorFormats[] =
{
	{ GL_RGBA, GL_RGBA8,    GL_UNSIGNED_BYTE },           // GfxColorFormat_NONE
	{ GL_RGBA, GL_RGBA8,    GL_UNSIGNED_BYTE },           // GfxColorFormat_R8G8B8A8
	{ GL_RGBA, GL_RGB10_A2, GL_UNSIGNED_INT_10_10_10_2 }, // GfxColorFormat_R10G10B10A2
	{ GL_RED,  GL_R8,       GL_UNSIGNED_BYTE },           // GfxColorFormat_R8
	{ GL_RG,   GL_RG8,      GL_UNSIGNED_BYTE },           // GfxColorFormat_R8G8
	{ GL_RED,  GL_R16F,     GL_UNSIGNED_SHORT },          // GfxColorFormat_R16
	{ GL_RG,   GL_RG16F,    GL_UNSIGNED_SHORT },          // GfxColorFormat_R16G16
	{ GL_RED,  GL_R32F,     GL_FLOAT },                   // GfxColorFormat_R32
};
static_assert( ARRAY_LENGTH( OpenGLColorFormats ) == GFXCOLORFORMAT_COUNT, "Missing GfxColorFormat!" );


struct OpenGLDepthStencilFormat
{
	OpenGLDepthStencilFormat( const GLenum format, const GLenum formatInternal, const GLenum formatType ) :
		format{ format }, formatInternal{ formatInternal }, formatType{ formatType } { }

	GLenum format, formatInternal, formatType;
};

static const OpenGLDepthStencilFormat OpenGLDepthStencilFormats[] =
{
    { GL_NONE,                 GL_NONE,               GL_NONE },               // GfxDepthFormat_NONE
    { GL_DEPTH_COMPONENT,      GL_DEPTH_COMPONENT16,  GL_UNSIGNED_SHORT },     // GfxDepthFormat_R16_FLOAT
    { GL_DEPTH_COMPONENT,      GL_DEPTH_COMPONENT16,  GL_UNSIGNED_SHORT },     // GfxDepthFormat_R16_UINT
    { GL_DEPTH_COMPONENT,      GL_DEPTH_COMPONENT32F, GL_FLOAT },              // GfxDepthFormat_R32_FLOAT
    { GL_DEPTH_COMPONENT,      GL_DEPTH_COMPONENT32F, GL_UNSIGNED_INT },       // GfxDepthFormat_R32_UINT
    { GL_DEPTH_STENCIL,        GL_DEPTH24_STENCIL8,   GL_UNSIGNED_INT_24_8 },  // GfxDepthFormat_R24_UINT_G8_UINT
};
static_assert( ARRAY_LENGTH( OpenGLDepthStencilFormats ) == GFXDEPTHFORMAT_COUNT, "Missing GfxDepthFormat!" );


static const GLenum OpenGLFillModes[] =
{
	GL_FILL, // GfxFillMode_SOLID
	GL_LINE, // GfxFillMode_WIREFRAME
};
static_assert( ARRAY_LENGTH( OpenGLFillModes ) == GFXFILLMODE_COUNT, "Missing GfxFillMode!" );


static const GLenum OpenGLCullModes[] =
{
	GL_FRONT_AND_BACK, // GfxCullMode_NONE
	GL_FRONT,          // GfxCullMode_FRONT
	GL_BACK,           // GfxCullMode_BACK
};
static_assert( ARRAY_LENGTH( OpenGLCullModes ) == GFXCULLMODE_COUNT, "Missing GfxCullMode!" );


static const GLint OpenGLFilteringModes[] =
{
	GL_NEAREST, // GfxFilteringMode_NEAREST
	GL_LINEAR,  // GfxFilteringMode_LINEAR
	GL_LINEAR,  // GfxFilteringMode_ANISOTROPIC TODO: Support anisotrophic
};
static_assert( ARRAY_LENGTH( OpenGLFilteringModes ) == GFXFILTERINGMODE_COUNT, "Missing GfxFilteringMode!" );


static const GLint OpenGLUVWrapModes[] =
{
	GL_REPEAT,          // GfxUVWrapMode_WRAP
	GL_MIRRORED_REPEAT, // GfxUVWrapMode_MIRROR
	GL_CLAMP_TO_EDGE,   // GfxUVWrapMode_CLAMP
};
static_assert( ARRAY_LENGTH( OpenGLUVWrapModes ) == GFXUVWRAPMODE_COUNT, "Missing GfxUVWrapMode!" );


static const GLenum OpenGLBlendFactors[] =
{
	GL_ZERO,                 // GfxBlendFactor_ZERO
	GL_ONE,                  // GfxBlendFactor_ONE
	GL_SRC_COLOR,            // GfxBlendFactor_SRC_COLOR
	GL_ONE_MINUS_SRC_COLOR,  // GfxBlendFactor_INV_SRC_COLOR
	GL_SRC_ALPHA,            // GfxBlendFactor_SRC_ALPHA
	GL_ONE_MINUS_SRC_ALPHA,  // GfxBlendFactor_INV_SRC_ALPHA
	GL_DST_ALPHA,            // GfxBlendFactor_DEST_ALPHA
	GL_ONE_MINUS_DST_ALPHA,  // GfxBlendFactor_INV_DEST_ALPHA
	GL_DST_COLOR,            // GfxBlendFactor_DEST_COLOR
	GL_ONE_MINUS_DST_COLOR,  // GfxBlendFactor_INV_DEST_COLOR
	GL_SRC_ALPHA_SATURATE,   // GfxBlendFactor_SRC_ALPHA_SAT
	GL_SRC_COLOR,            // GL_SRC1_COLOR,           // GfxBlendFactor_SRC1_COLOR      // TODO: Support these?
	GL_ONE_MINUS_SRC_COLOR,  // GL_ONE_MINUS_SRC1_COLOR, // GfxBlendFactor_INV_SRC1_COLOR
	GL_SRC_ALPHA,            // GL_SRC1_ALPHA,           // GfxBlendFactor_SRC1_ALPHA
	GL_ONE_MINUS_SRC_ALPHA,  // GL_ONE_MINUS_SRC1_ALPHA, // GfxBlendFactor_INV_SRC1_ALPHA
};
static_assert( ARRAY_LENGTH( OpenGLBlendFactors ) == GFXBLENDFACTOR_COUNT, "Missing GfxBlendFactor!" );


static const GLenum OpenGLBlendOperations[] =
{
	GL_FUNC_ADD,              // GfxBlendOperation_ADD
	GL_FUNC_SUBTRACT,         // GfxBlendOperation_SUBTRACT
	GL_FUNC_REVERSE_SUBTRACT, // GfxBlendOperation_REV_SUBTRACT
	GL_MIN,                   // GfxBlendOperation_MIN
	GL_MAX,                   // GfxBlendOperation_MAX
};
static_assert( ARRAY_LENGTH( OpenGLBlendOperations ) == GFXBLENDOPERATION_COUNT, "Missing GfxBlendOperation!" );


static const GLboolean OpenGLDepthWriteMasks[] =
{
	GL_FALSE, // GfxDepthWriteFlag_NONE
	GL_TRUE,  // GfxDepthWriteFlag_ALL
};
static_assert( ARRAY_LENGTH( OpenGLDepthWriteMasks ) == GFXDEPTHWRITEFLAG_COUNT, "Missing GfxDepthWriteFlag!" );


static const GLenum OpenGLDepthTestModes[] =
{
	GL_ALWAYS,   // GfxDepthTestMode_NONE
	GL_LESS,     // GfxDepthTestMode_LESS
	GL_LEQUAL,   // GfxDepthTestMode_LESS_EQUALS
	GL_GREATER,  // GfxDepthTestMode_GREATER
	GL_GEQUAL,   // GfxDepthTestMode_GREATER_EQUALS
	GL_EQUAL,    // GfxDepthTestMode_EQUAL
	GL_NOTEQUAL, // GfxDepthTestMode_NOT_EQUAL
	GL_ALWAYS,   // GfxDepthTestMode_ALWAYS
};
static_assert( ARRAY_LENGTH( OpenGLDepthTestModes ) == GFXDEPTHTESTMODE_COUNT, "Missing GfxDepthTestMode!" );


static const GLenum OpenGLIndexBufferFormats[] =
{
	GL_UNSIGNED_BYTE,   // GfxIndexBufferFormat_NONE
	GL_UNSIGNED_BYTE,   // GfxIndexBufferFormat_U8
	GL_UNSIGNED_SHORT,  // GfxIndexBufferFormat_U16
	GL_UNSIGNED_INT,    // GfxIndexBufferFormat_U32
};
static_assert( ARRAY_LENGTH( OpenGLIndexBufferFormats ) == GFXINDEXBUFFERFORMAT_COUNT, "Missing GfxIndexBufferFormat!" );

/*
struct D3D11MapAccessMode
{
	D3D11MapAccessMode( const u32 cpuAccessFlag, const D3D11_MAP d3d11Map, const D3D11_USAGE d3d11Usage ) :
		cpuAccessFlag{ cpuAccessFlag }, d3d11Map{ d3d11Map }, d3d11Usage{ d3d11Usage } { }

	u32 cpuAccessFlag;
	D3D11_MAP d3d11Map;
	D3D11_USAGE d3d11Usage;
};

enum
{
	D3D11_CPU_ACCESS_NONE = 0,
	D3D11_CPU_ACCESS_READ_WRITE = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE,
};

static const D3D11MapAccessMode D3D11CPUAccessModes[] =
{
	{ D3D11_CPU_ACCESS_NONE,       D3D11_MAP_WRITE_NO_OVERWRITE, D3D11_USAGE_IMMUTABLE }, // GfxCPUAccessMode_NONE
	{ D3D11_CPU_ACCESS_READ,       D3D11_MAP_READ,               D3D11_USAGE_IMMUTABLE }, // GfxCPUAccessMode_READ
	{ D3D11_CPU_ACCESS_READ_WRITE, D3D11_MAP_READ_WRITE,         D3D11_USAGE_DYNAMIC },   // GfxCPUAccessMode_READ_WRITE
	{ D3D11_CPU_ACCESS_WRITE,      D3D11_MAP_WRITE,              D3D11_USAGE_DYNAMIC },   // GfxCPUAccessMode_WRITE
	{ D3D11_CPU_ACCESS_WRITE,      D3D11_MAP_WRITE_DISCARD,      D3D11_USAGE_DYNAMIC },   // GfxCPUAccessMode_WRITE_DISCARD
	{ D3D11_CPU_ACCESS_WRITE,      D3D11_MAP_WRITE_NO_OVERWRITE, D3D11_USAGE_DYNAMIC },   // GfxCPUAccessMode_WRITE_NO_OVERWRITE
};
static_assert( ARRAY_LENGTH( D3D11CPUAccessModes ) == GFXCPUACCESSMODE_COUNT, "Missing GfxCPUAccessMode!" );
*/

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct GfxVertexBufferResource : public GfxResource
{
	GLuint vao;
	GLuint vbo;
	GfxCPUAccessMode accessMode;
	bool mapped = false;
	byte *data = nullptr;
	u32 size = 0;
	u32 stride = 0; // vertex size
	u32 offset = 0;
	u32 current = 0;
	u32 vertexFormat = 0;
};


struct GfxIndexBufferResource : public GfxResource
{
	GLuint ebo;
	GfxCPUAccessMode accessMode = GfxCPUAccessMode_NONE;
	GfxIndexBufferFormat format = GfxIndexBufferFormat_U32;
	double indToVertRatio = 1.0;
	u32 size = 0;
};


struct GfxConstantBufferResource : public GfxResource
{
	GLuint ubo;
	bool mapped = false;
	byte *data = nullptr;
	const char *name = "";
	int index = 0;
	u32 size = 0; // buffer size in bytes
};


struct GfxTexture2DResource : public GfxResource
{
	GLuint texture;
	GfxColorFormat colorFormat;
	u32 width = 0;
	u32 height = 0;
};


struct GfxRenderTarget2DResource : public GfxResource
{
	/*
	ID3D11RenderTargetView *targetColor = nullptr;
	ID3D11DepthStencilView *targetDepth = nullptr;
	ID3D11Texture2D *stagingColor = nullptr;
	ID3D11Texture2D *stagingDepth = nullptr;
	*/
	GLuint fbo = 0;
	GfxRenderTargetDescription desc = { };
	u16 width = 0;
	u16 height = 0;
};


struct GfxShaderResource : public GfxResource
{
	GLuint shaderVertex;
	GLuint shaderFragment;
	GLuint shaderCompute;
	GLuint program;
	u32 sizeVS, sizePS, sizeCS;
	u32 shaderID;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static GfxResourceFactory<GfxVertexBufferResource, GFX_RESOURCE_COUNT_VERTEX_BUFFER> vertexBufferResources;
static GfxResourceFactory<GfxIndexBufferResource, GFX_RESOURCE_COUNT_INDEX_BUFFER> indexBufferResources;
static GfxResourceFactory<GfxConstantBufferResource, GFX_RESOURCE_COUNT_CONSTANT_BUFFER> constantBufferResources;
static GfxResourceFactory<GfxShaderResource, GFX_RESOURCE_COUNT_SHADER> shaderResources;
static GfxResourceFactory<GfxTexture2DResource, GFX_RESOURCE_COUNT_TEXTURE_2D> texture2DResources;
static GfxResourceFactory<GfxRenderTarget2DResource, GFX_RESOURCE_COUNT_RENDER_TARGET_2D> renderTarget2DResources;


static bool resources_init()
{
	vertexBufferResources.init();
	indexBufferResources.init();
	constantBufferResources.init();
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
	constantBufferResources.free();
	texture2DResources.free();
	renderTarget2DResources.free();
	shaderResources.free();

	// Success
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void opengl_draw( const GLsizei vertexCount, const GLuint startVertexLocation )
{
	SysGfx::state_apply();
	glDrawArrays( GL_TRIANGLES, startVertexLocation, vertexCount );
	PROFILE_GFX( Gfx::stats.frame.drawCalls++ );
	PROFILE_GFX( Gfx::stats.frame.vertexCount += vertexCount );
}


static void opengl_draw_indexed( const GLsizei vertexCount, const GLuint startVertexLocation,
                                 const GLuint baseVertexLocation, GfxIndexBufferFormat format )
{
	SysGfx::state_apply();
	glDrawElements( GL_TRIANGLES, vertexCount, OpenGLIndexBufferFormats[format], 0 ); // Draw Call
	PROFILE_GFX( Gfx::stats.frame.drawCalls++ );
	PROFILE_GFX( Gfx::stats.frame.vertexCount += vertexCount );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static GLuint opengl_constant_buffer_uniform_block_index( GfxConstantBufferResource *&resource, const int slot )
{
	// Get hash "key" from cbuffer index, slot, and shader program
	char buffer[64];
	const u32 a = static_cast<u32>( resource->index );
	const u32 b = static_cast<u32>( boundShaderResource->program );
	const u32 c = static_cast<u32>( slot );
	snprintf( buffer, sizeof( buffer ), "%u_%u_%u", a, b, c ); // TODO: Refactor this?
	const u32 key = checksum_xcrc32( buffer, strlen( buffer ), 0 );

	// Block index already found?
	if( constantBufferUniformBlockIndices.contains( key ) )
	{
		return constantBufferUniformBlockIndices.get( key );
	}

	// Find & cache block index
	const GLuint index = nglGetUniformBlockIndex( boundShaderResource->program,
	                                              static_cast<const GLchar *>( resource->name ) );

	ErrorReturnIf( index == GL_INVALID_OPERATION, false,
		"OpenGL: Unable to get uniform block index (Invalid operation) (%s)",
		resource->name );

	ErrorReturnIf( index == GL_INVALID_INDEX, false,
		"OpenGL: Unable to get uniform block index (Invalid index) (%s) %d",
		resource->name, boundShaderResource->program );

	constantBufferUniformBlockIndices.add( key, index );
	return index;
}


static GLint opengl_texture_uniform_location( HashMap<u32, GLint> &cache, const int slot )
{
	// Get hash "key" from cbuffer index, slot, and shader program
	char buffer[64];
	const u32 a = static_cast<u32>( boundShaderResource->program );
	const u32 b = static_cast<u32>( slot );
	snprintf( buffer, sizeof( buffer ), "%u_%u", a, b ); // TODO: Refactor this?
	const u32 key = checksum_xcrc32( buffer, strlen( buffer ), 0 );

	// Uniform index already found?
	if( cache.contains( key ) )
	{
		return cache.get( key );
	}

	// Find & cache uniform index
	char name[64];
	snprintf( name, sizeof( name ), "u_texture%d", slot );
	const GLint location = nglGetUniformLocation( boundShaderResource->program,
	                                              static_cast<const GLchar *>( name ) );

	ErrorReturnIf( location == GL_INVALID_OPERATION, false,
		"OpenGL: Unable to get texture uniform index (Invalid operation)" );

	ErrorReturnIf( location == GL_INVALID_VALUE, false,
		"OpenGL: Unable to get texture uniform index (Invalid value)" );

	cache.add( key, location );
	return location;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GfxCore::rb_init()
{
	// GfxState
	GfxState &state = Gfx::state();
	const u16 w = Window::width;
	const u16 h = Window::height;
	const u16 f = Window::fullscreen;

	// Initialize OpenGL Context
	if( !opengl_init() )
	{
		ErrorReturnMsg( false, "%s: Failed to init OpenGL", __FUNCTION__ );
	}

#if DEPTH_BUFFER_ENABLED
	glEnable( GL_DEPTH_TEST );
	glDepthMask( true );
#endif

	// Resources
	resources_init();

	// Uniform Caches
	constantBufferUniformBlockIndices.init();
	texture2DUniformLocations.init();

	// Success
	return true;
}


bool GfxCore::rb_free()
{
	// Resources
	resources_free();

	// Uniform Caches
	constantBufferUniformBlockIndices.free();
	texture2DUniformLocations.free();

	// Success
	return true;
}


void GfxCore::rb_frame_begin()
{
	// OpenGL does nothing
}


void GfxCore::rb_frame_end()
{
#if COMPILE_DEBUG
	// Check OpenGL errors
	for( ;; )
	{
		int error = glGetError();
		if( error == GL_NO_ERROR ) { break; }
		Error( "OpenGL Error: %d\n", error );
	}
#endif

	// Swap Buffers
	const bool swap = opengl_swap();
	Assert( swap );
}


void GfxCore::rb_clear_color( const Color color )
{
	// Compiler likely does this regardless, but mul instruction is faster than div
	static constexpr float INV_255 = 1.0f / 255.0f;
	glClearColor( color.r * INV_255, color.g * INV_255, color.b * INV_255, color.a * INV_255 );
	glClear( GL_COLOR_BUFFER_BIT );
}


void GfxCore::rb_clear_depth( const float depth )
{
	glDepthMask( true );
	glClear( GL_DEPTH_BUFFER_BIT );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GfxCore::rb_swapchain_init( const u16 width, const u16 height, const bool fullscreen )
{
	PROFILE_GFX( Gfx::stats.gpuMemoryFramebuffer =
		GFX_SIZE_IMAGE_COLOR_BYTES( width, height, 1, GfxColorFormat_R8G8B8A8 ) );

#if DEPTH_BUFFER_ENABLED
	PROFILE_GFX( Gfx::stats.gpuMemoryFramebuffer +=
		GFX_SIZE_IMAGE_DEPTH_BYTES( width, height, 1, DEPTH_BUFFER_FORMAT ) );
#endif

	return true;
}


bool GfxCore::rb_swapchain_free()
{
	return true;
}


bool GfxCore::rb_swapchain_resize( u16 width, u16 height, bool fullscreen )
{
	PROFILE_GFX( Gfx::stats.gpuMemoryFramebuffer =
		GFX_SIZE_IMAGE_COLOR_BYTES( width, height, 1, GfxColorFormat_R8G8B8A8 ) );

#if DEPTH_BUFFER_ENABLED
	PROFILE_GFX( Gfx::stats.gpuMemoryFramebuffer +=
		GFX_SIZE_IMAGE_DEPTH_BYTES( width, height, 1, DEPTH_BUFFER_FORMAT ) );
#endif

	opengl_update();
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GfxCore::rb_viewport_init( const u16 width, const u16 height, const bool fullscreen )
{
	glViewport( 0, 0, width, height );
	return true;
}


bool GfxCore::rb_viewport_free()
{
	// OpenGL does nothing
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
	// Set Cull Mode
	glFrontFace( GL_CW );
	if( state.cullMode ) { glEnable( GL_CULL_FACE ); } else { glDisable( GL_CULL_FACE ); }
	glCullFace( OpenGLCullModes[state.cullMode] );

	// Set Fill Mode
	// TODO: Evaluate this, only GL_FRONT_AND_BACK appears to work
	// glPolygonMode( OpenGLCullModes[state.cullMode], OpenGLFillModes[state.fillMode] );
	glPolygonMode( GL_FRONT_AND_BACK, OpenGLFillModes[state.fillMode] );

	// Set Scissor
	if( state.scissor )
	{
		glEnable( GL_SCISSOR_TEST );
		GLint x = static_cast<GLint>( state.scissorX1 );
		GLint y = static_cast<GLint>( Window::height - state.scissorY2 ); // Flip y
		GLint width = static_cast<GLint>( state.scissorX2 - state.scissorX1 );
		GLint height = static_cast<GLint>( state.scissorY2 - state.scissorY1 );
		glScissor( x * Window::scale, y * Window::scale, width * Window::scale, height * Window::scale );
	}
	else
	{
		glDisable( GL_SCISSOR_TEST );
	}

	// Success
	return true;
}


bool GfxCore::rb_set_sampler_state( const GfxSamplerState &state )
{
	// Filter Mode
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, OpenGLFilteringModes[state.filterMode] );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, OpenGLFilteringModes[state.filterMode] );

	// UV Wrap Mode
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, OpenGLUVWrapModes[state.wrapMode] );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, OpenGLUVWrapModes[state.wrapMode] );

	// Success
	return true;
}


bool GfxCore::rb_set_blend_state( const GfxBlendState &state )
{
	// Set Blend Enable
	if( state.blendEnable )
	{
		glEnable( GL_BLEND );
	}
	else
	{
		glDisable( GL_BLEND );
	}

	// Set Color Write Mask
	GLboolean r = state.colorWriteMask & GfxColorWriteFlag_RED;
	GLboolean g = state.colorWriteMask & GfxColorWriteFlag_GREEN;
	GLboolean b = state.colorWriteMask & GfxColorWriteFlag_BLUE;
	GLboolean a = state.colorWriteMask & GfxColorWriteFlag_ALPHA;
	glColorMask( r, g, b, a );

	// Blend Functions
	nglBlendFuncSeparate( OpenGLBlendFactors[state.srcFactorColor], OpenGLBlendFactors[state.dstFactorColor],
	                      OpenGLBlendFactors[state.srcFactorAlpha], OpenGLBlendFactors[state.dstFactorAlpha] );

	// Blend Operations
	nglBlendEquationSeparate( OpenGLBlendOperations[state.blendOperationColor],
	                          OpenGLBlendOperations[state.blendOperationAlpha] );

	// Success
	return true;
}


bool GfxCore::rb_set_depth_state( const GfxDepthState &state )
{
	// Set Depth Test Enable
	if( state.depthTestMode != GfxDepthTestMode_NONE )
	{
		glEnable( GL_DEPTH_TEST );
	}
	else
	{
		glDisable( GL_DEPTH_TEST );
	}

	// Set Depth Test Mode
	glDepthFunc( OpenGLDepthTestModes[state.depthTestMode] );

	// Set Depth Write Mask
	glDepthMask( OpenGLDepthWriteMasks[state.depthWriteMask] );

	// Success
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// glMapNamedBufferRange( GLuint buffer, GLintptr offset, GLsizeiptr length, GLbitfield access );

bool GfxCore::rb_index_buffer_init( GfxIndexBufferResource *&resource, void *data, const u32 size,
                                    const double indToVertRatio,
                                    const GfxIndexBufferFormat format, const GfxCPUAccessMode accessMode )
{
	Assert( format != GfxIndexBufferFormat_NONE );
	Assert( format < GFXINDEXBUFFERFORMAT_COUNT );

	// Register IndexBuffer
	Assert( resource == nullptr );
	resource = indexBufferResources.make_new();
	resource->accessMode = accessMode;
	resource->format = format;
	resource->indToVertRatio = indToVertRatio;
	resource->size = size;

	// Generate Buffer
	nglGenBuffers( 1, &resource->ebo );

	// Bind Buffer
	nglBindBuffer( GL_ELEMENT_ARRAY_BUFFER, resource->ebo );

	// Write Index Data
	nglBufferData( GL_ELEMENT_ARRAY_BUFFER, size, data, GL_STATIC_DRAW );

	// Success
	return true;
}


bool GfxCore::rb_index_buffer_free( GfxIndexBufferResource *&resource )
{
	Assert( resource != nullptr );

	nglDeleteBuffers( 1, &resource->ebo );
	resource->ebo = GL_NULL;
	indexBufferResources.remove( resource->id );
	resource = nullptr;

	// Success
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GfxCore::rb_vertex_buffer_init_dynamic( GfxVertexBufferResource *&resource, const u32 vertexFormatID,
                                             const GfxCPUAccessMode accessMode, const u32 size, const u32 stride )
{
	// Register VertexBuffer
	Assert( accessMode == GfxCPUAccessMode_WRITE_DISCARD || accessMode == GfxCPUAccessMode_WRITE_NO_OVERWRITE );
	Assert( resource == nullptr );
	resource = vertexBufferResources.make_new();
	resource->size = size;
	resource->stride = stride;
	resource->accessMode = accessMode;
	resource->vertexFormat = vertexFormatID;

	// Generate Buffer
	nglGenVertexArrays( 1, &resource->vao );
	nglGenBuffers( 1, &resource->vbo );

	// Setup Vertex Buffer
	nglBindVertexArray( resource->vao );
	nglBindBuffer( GL_ARRAY_BUFFER, resource->vbo );
	nglBufferData( GL_ARRAY_BUFFER, size, nullptr, /*GL_DYNAMIC_DRAW*/GL_STATIC_DRAW ); // TODO: GL_DYNAMIC_DRAW

	// Success
	return true;
}


bool GfxCore::rb_vertex_buffer_init_static( GfxVertexBufferResource *&resource, const u32 vertexFormatID,
                                            const GfxCPUAccessMode accessMode, const void *const data,
                                            const u32 size, const u32 stride )
{
	// Success
	return true;
}


bool GfxCore::rb_vertex_buffer_free( GfxVertexBufferResource *&resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	PROFILE_GFX( Gfx::stats.gpuMemoryVertexBuffers -= resource->size );

	nglDeleteVertexArrays( 1, &resource->vao );
	resource->vao = GL_NULL;
	nglDeleteBuffers( 1, &resource->vbo );
	resource->vbo = GL_NULL;
	vertexBufferResources.remove( resource->id );
	resource = nullptr;

	// Success
	return true;
}


bool GfxCore::rb_vertex_buffer_draw( GfxVertexBufferResource *&resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	// Bind Buffer
	nglBindVertexArray( resource->vao );
	nglBindBuffer( GL_ARRAY_BUFFER, resource->vbo );
	CHECK_ERROR( "Failed to bind vertex buffer for draw (resource: %u)", resource->id )

	// Input Layout
	opengl_vertex_input_layout_bind[resource->vertexFormat]();

	// Submit Draw
	ErrorIf( resource->mapped, "Attempting to draw vertex buffer that is mapped! (resource: %u)", resource->id );
	const GLsizei count = static_cast<GLsizei>( resource->current / resource->stride );
	opengl_draw( count, 0 );

	// Success
	return true;
}


bool GfxCore::rb_vertex_buffer_draw_indexed( GfxVertexBufferResource *&resource,
                                             GfxIndexBufferResource *&resourceIndexBuffer )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( resourceIndexBuffer != nullptr && resourceIndexBuffer->id != GFX_RESOURCE_ID_NULL );

	// Bind Vertex Buffer
	nglBindVertexArray( resource->vao );
	nglBindBuffer( GL_ARRAY_BUFFER, resource->vbo );
	CHECK_ERROR( "Failed to bind vertex buffer for indexed draw (resource: %u)", resource->id )

	// Bind Index Buffer
	nglBindBuffer( GL_ELEMENT_ARRAY_BUFFER, resourceIndexBuffer->ebo );
	CHECK_ERROR( "Failed to bind index buffer for indexed draw (resource: %u)", resource->id )

	// Input Layout
	opengl_vertex_input_layout_bind[resource->vertexFormat]();

	// Submit Draw
	ErrorIf( resource->mapped, "Attempting to draw vertex buffer that is mapped! (resource: %u)", resource->id );
	const GLsizei count = static_cast<GLsizei>( resource->current / resource->stride *
	                                            resourceIndexBuffer->indToVertRatio );
	opengl_draw_indexed( count, 0, 0, resourceIndexBuffer->format );

	// Success
	return true;
}


void GfxCore::rb_vertex_buffer_write_begin( GfxVertexBufferResource *&resource )
{
	if( resource->mapped == true ) { return; }

	nglBindVertexArray( resource->vao );
	nglBindBuffer( GL_ARRAY_BUFFER, resource->vbo );
	CHECK_ERROR( "Failed to bind vertex buffer for write begin (resource: %u)", resource->id )

	GLbitfield access = 0;
	switch( resource->accessMode )
	{
		case GfxCPUAccessMode_WRITE:
			access = GL_MAP_WRITE_BIT;
		break;

		case GfxCPUAccessMode_WRITE_DISCARD:
			access = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT;
		break;

		default:
		case GfxCPUAccessMode_WRITE_NO_OVERWRITE:
			access = GL_MAP_WRITE_BIT;
			//Error( "Not supported yet" ); // TODO: Support this
		break;
	}

	resource->data = reinterpret_cast<byte *>( nglMapBufferRange( GL_ARRAY_BUFFER, 0, resource->size, access ) );
	CHECK_ERROR( "Failed to map vertex buffer for write begin (resource: %u)", resource->id )

	resource->mapped = true;
	resource->current = 0;
	resource->offset = 0;
}


void GfxCore::rb_vertex_buffer_write_end( GfxVertexBufferResource *&resource )
{
	if( resource->mapped == false ) { return; }

	nglBindVertexArray( resource->vao );
	nglBindBuffer( GL_ARRAY_BUFFER, resource->vbo );
	CHECK_ERROR( "Failed to bind vertex buffer for write end (resource: %u)", resource->id )

	nglUnmapBuffer( GL_ARRAY_BUFFER );
	CHECK_ERROR( "Failed to unmap vertex buffer for write end (resource: %u)", resource->id )

	resource->mapped = false;
}


bool GfxCore::rb_vertex_buffer_write( GfxVertexBufferResource *&resource, const void *const data, const u32 size )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( resource->mapped );

	memory_copy( resource->data + resource->current, data, size );
	resource->current += size;

	// Success
	return true;
}


u32 GfxCore::rb_vertex_buffer_current( GfxVertexBufferResource *&resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	return resource->current;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GfxCore::rb_constant_buffer_init( GfxConstantBufferResource *&resource, const char *name,
                                       const int index, const u32 size )
{
	// Register Constant Buffer
	Assert( resource == nullptr );
	resource = constantBufferResources.make_new();
	resource->name = name;
	resource->index = index;
	resource->size = size;

	nglGenBuffers( 1, &resource->ubo );
	CHECK_ERROR( "Failed to generate constant buffer (resource: %u)", resource->id );

	nglBindBuffer( GL_UNIFORM_BUFFER, resource->ubo );
	nglBufferData( GL_UNIFORM_BUFFER, size, nullptr, GL_DYNAMIC_DRAW );
	nglBindBuffer( GL_UNIFORM_BUFFER, 0 );
	CHECK_ERROR( "Failed to bind constant buffer for init (resource: %u)", resource->id );

	// Success
	return true;
}


bool GfxCore::rb_constant_buffer_free( GfxConstantBufferResource *&resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	PROFILE_GFX( Gfx::stats.gpuMemoryConstantBuffers -= resource->size );

	nglDeleteBuffers( 1, &resource->ubo );
	resource->ubo = GL_NULL;
	constantBufferResources.remove( resource->id );
	resource = nullptr;

	// Success
	return true;
}


void GfxCore::rb_constant_buffer_write_begin( GfxConstantBufferResource *&resource )
{
	if( resource->mapped == true ) { return; }
	resource->mapped = true;
}


void GfxCore::rb_constant_buffer_write_end( GfxConstantBufferResource *&resource )
{
	if( resource->mapped == false ) { return; }
	resource->mapped = false;
}


bool GfxCore::rb_constant_buffer_write( GfxConstantBufferResource *&resource, const void *data )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( resource->mapped );

	nglBindBuffer( GL_UNIFORM_BUFFER, resource->ubo );
	nglBufferSubData( GL_UNIFORM_BUFFER, 0, static_cast<GLsizeiptr>( resource->size ), data );
	nglBindBuffer( GL_UNIFORM_BUFFER, 0 );

	// Success
	return true;
}


bool GfxCore::rb_constant_buffer_bind_vertex( GfxConstantBufferResource *&resource, const int slot )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( slot >= 0 );

	nglBindBuffer( GL_UNIFORM_BUFFER, resource->ubo );
	nglBindBufferBase( GL_UNIFORM_BUFFER, static_cast<GLuint>( resource->index ), resource->ubo );

	const GLuint blockIndex = opengl_constant_buffer_uniform_block_index( resource, slot );
	nglUniformBlockBinding( boundShaderResource->program, blockIndex, static_cast<GLuint>( resource->index ) );

	// Success
	return true;
}


bool GfxCore::rb_constant_buffer_bind_fragment( GfxConstantBufferResource *&resource, const int slot )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( slot >= 0 );

	nglBindBuffer( GL_UNIFORM_BUFFER, resource->ubo );
	nglBindBufferBase( GL_UNIFORM_BUFFER, static_cast<GLuint>( resource->index ), resource->ubo );

	const GLuint blockIndex = opengl_constant_buffer_uniform_block_index( resource, slot );
	nglUniformBlockBinding( boundShaderResource->program, blockIndex, static_cast<GLuint>( resource->index ) );

	// Success
	return true;
}


bool GfxCore::rb_constant_buffer_bind_compute( GfxConstantBufferResource *&resource, const int slot )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( slot >= 0 );

	nglBindBuffer( GL_UNIFORM_BUFFER, resource->ubo );
	nglBindBufferBase( GL_UNIFORM_BUFFER, static_cast<GLuint>( resource->index ), resource->ubo );

	const GLuint blockIndex = opengl_constant_buffer_uniform_block_index( resource, slot );
	nglUniformBlockBinding( boundShaderResource->program, blockIndex, static_cast<GLuint>( resource->index ) );

	// Success
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GfxCore::rb_texture_2d_init( GfxTexture2DResource *&resource, void *pixels,
                                  const u16 width, const u16 height, const GfxColorFormat &format )
{
	// Register Texture2D
	Assert( resource == nullptr );
	resource = texture2DResources.make_new();
	resource->colorFormat = format;
	resource->width = width;
	resource->height = height;

	// Create Texture
	glGenTextures( 1, &resource->texture );

	// Check Errors
	GLenum error = glGetError();
	if( error != GL_NO_ERROR )
	{
		ErrorReturnMsg( false, "%s: Failed to init texture (%d)", __FUNCTION__, error );
	}

	// Setup Texture2D Data (TODO: Switch to sampler objects?)
	glBindTexture( GL_TEXTURE_2D, resource->texture );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	const GLint glFormatInternal = OpenGLColorFormats[format].formatInternal;
	const GLenum glFormat = OpenGLColorFormats[format].format;
	const GLenum glFormatType = OpenGLColorFormats[format].formatType;
	glTexImage2D( GL_TEXTURE_2D, 0, glFormatInternal, width, height, 0, glFormat, glFormatType, pixels );
	glBindTexture( GL_TEXTURE_2D, 0 );

	PROFILE_GFX( Gfx::stats.gpuMemoryTextures += GFX_SIZE_IMAGE_COLOR_BYTES( width, height, 1, format ) );

	// Success
	return true;
}


bool GfxCore::rb_texture_2d_free( GfxTexture2DResource *&resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	PROFILE_GFX( Gfx::stats.gpuMemoryTextures -=
		GFX_SIZE_IMAGE_COLOR_BYTES( resource->width, resource->height, 1, resource->colorFormat ) );

	glDeleteTextures( 1, &resource->texture );
	resource->texture = GL_NULL;
	texture2DResources.remove( resource->id );
	resource = nullptr;

	// Success
	return true;
}


bool GfxCore::rb_texture_2d_bind( const GfxTexture2DResource *const &resource, const int slot )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	// Bind Texture2D
	const GLint location = opengl_texture_uniform_location( texture2DUniformLocations, slot );
	nglUniform1i( location, slot );
	nglActiveTexture( GL_TEXTURE0 + slot );
	glBindTexture( GL_TEXTURE_2D, resource->texture );

	// OpenGL requires us to explitily set the sampler state on textures binds (TODO: Refactor)
	GfxCore::rb_set_sampler_state( Gfx::state().sampler );

	// Success
	return true;
}


bool GfxCore::rb_texture_2d_release( const int slot )
{
	// Do nothing...

	// Success
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//static ID3D11RenderTargetView *RT_CACHE_TARGET_COLOR; // HACK: change this?
//static ID3D11DepthStencilView *RT_CACHE_TARGET_DEPTH; // HACK: change this?


bool GfxCore::rb_render_target_2d_init( GfxRenderTarget2DResource *&resource,
                                        GfxTexture2DResource *&resourceColor, GfxTexture2DResource *&resourceDepth,
                                        const u16 width, const u16 height,
                                        const GfxRenderTargetDescription &desc )
{
	// Register RenderTarget2D
	Assert( resource == nullptr );
	resource = renderTarget2DResources.make_new();
	resource->width = width;
	resource->height = height;
	resource->desc = desc;

	// Color Texture
	{
		// Register Resource
		resourceColor = texture2DResources.make_new();

		// Create Texture
		glGenTextures( 1, &resourceColor->texture );

		// Check Errors
		GLenum error = glGetError();
		if( error != GL_NO_ERROR )
		{
			ErrorReturnMsg( false, "%s: RenderTarget2D: Failed to init texture (%d)", __FUNCTION__, error );
		}

		// Setup Texture2D Data (TODO: Switch to sampler objects?)
		glBindTexture( GL_TEXTURE_2D, resourceColor->texture );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		const GLint glFormatInternal = OpenGLColorFormats[desc.colorFormat].formatInternal;
		const GLenum glFormat = OpenGLColorFormats[desc.colorFormat].format;
		const GLenum glFormatType = OpenGLColorFormats[desc.colorFormat].formatType;
		glTexImage2D( GL_TEXTURE_2D, 0, glFormatInternal, width, height, 0, glFormat, glFormatType, nullptr );
		glBindTexture( GL_TEXTURE_2D, 0 );

		PROFILE_GFX( Gfx::stats.gpuMemoryTextures += GFX_SIZE_IMAGE_COLOR_BYTES( width, height, 1, desc.colorFormat ) );
	}

	// Depth Texture
	if( desc.depthFormat != GfxDepthFormat_NONE )
	{
		// Register Resource
		resourceDepth = texture2DResources.make_new();

		// Create Texture
		glGenTextures( 1, &resourceDepth->texture );

		// Check Errors
		GLenum error = glGetError();
		if( error != GL_NO_ERROR )
		{
			ErrorReturnMsg( false, "%s: RenderTarget2D: Failed to init depth texture (%d)", __FUNCTION__, error );
		}

		// Setup Texture2D Data (TODO: Switch to sampler objects?)
		glBindTexture( GL_TEXTURE_2D, resourceDepth->texture );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		const GLint glFormatInternal = OpenGLDepthStencilFormats[desc.depthFormat].formatInternal;
		const GLenum glFormat = OpenGLDepthStencilFormats[desc.depthFormat].format;
		const GLenum glFormatType = OpenGLDepthStencilFormats[desc.depthFormat].formatType;
		glTexImage2D( GL_TEXTURE_2D, 0, glFormatInternal, width, height, 0, glFormat, glFormatType, nullptr );
		glBindTexture( GL_TEXTURE_2D, 0 );

		PROFILE_GFX( Gfx::stats.gpuMemoryTextures += GFX_SIZE_IMAGE_DEPTH_BYTES( width, height, 1, desc.depthFormat ) );
	}

	// FBO
	nglGenFramebuffers( 1, &resource->fbo );
	nglBindFramebuffer( GL_FRAMEBUFFER, resource->fbo );
	{
		// Attach Color
		nglFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, resourceColor->texture, 0 );

		// Attach Depth
		if( desc.depthFormat != GfxDepthFormat_NONE )
		{
			nglFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, resourceDepth->texture, 0 );
		}

		// Check Status
		if( nglCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
		{
			ErrorReturnMsg( false, "%s: Failed to create framebuffer", __FUNCTION__ );
		}
	}
	nglBindFramebuffer( GL_FRAMEBUFFER, 0 );

	// Success
	return true;
}


bool GfxCore::rb_render_target_2d_free( GfxRenderTarget2DResource *&resource,
                                        GfxTexture2DResource *&resourceColor,GfxTexture2DResource *&resourceDepth )
{
	// Success
	return true;
}


bool GfxCore::rb_render_target_2d_buffer_read_color( GfxRenderTarget2DResource *&resource,
                                                     GfxTexture2DResource *&resourceColor,
                                                     void *buffer, const u32 size )
{
	// Success
	return true;
}


bool rb_render_target_2d_buffer_read_depth( GfxRenderTarget2DResource *&resource,
                                            GfxTexture2DResource *&resourceDepth,
                                            void *buffer, const u32 size )
{
	// Success
	return true;
}


bool GfxCore::rb_render_target_2d_buffer_write_color( GfxRenderTarget2DResource *&resource,
                                                      GfxTexture2DResource *&resourceColor,
                                                      const void *const buffer, const u32 size )
{
	// Success
	return true;
}


bool GfxCore::rb_render_target_2d_bind( const GfxRenderTarget2DResource *const &resource, int slot )
{
	// Bind Framebuffer to surface0's FBO
	/*
	ASSERT( surface0->exists );
	nglBindFramebuffer( GL_FRAMEBUFFER, surface0->fbo );

	// Attach MRTs
	uint8 renderTargetCount = 0;
	if( surface0 != nullptr ) {                             nglFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, surface0->color_texture.texture, 0 ); renderTargetCount++; }
	if( surface1 != nullptr ) { ASSERT( surface1->exists ); nglFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, surface1->color_texture.texture, 0 ); renderTargetCount++; }
	if( surface2 != nullptr ) { ASSERT( surface2->exists ); nglFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, surface2->color_texture.texture, 0 ); renderTargetCount++; }
	if( surface3 != nullptr ) { ASSERT( surface3->exists ); nglFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, surface3->color_texture.texture, 0 ); renderTargetCount++; }

	// Set MRT glDrawBuffers (so they can be written to)
	GLenum buffers[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
	nglDrawBuffers( renderTargetCount, buffers );
	g_RENDER_STATE.renderTargetCount = renderTargetCount;
	*/

	// TODO: Multiple bound render textures

	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	nglBindFramebuffer( GL_FRAMEBUFFER, resource->fbo );
	//nglFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, resource->textureColor, 0 );
	//nglFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, resource->textureDepth, 0 );
	GLenum buffers[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
	nglDrawBuffers( 1, buffers );

	// Success
	return true;
}


bool GfxCore::rb_render_target_2d_release()
{
	nglBindFramebuffer( GL_FRAMEBUFFER, 0 );

	// Success
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GfxCore::rb_shader_init( GfxShaderResource *&resource, const u32 shaderID, const struct DiskShader &diskShader )
{
	char info[1024];
	int status;

	// Register Shader
	Assert( resource == nullptr );
	resource = shaderResources.make_new();
	resource->shaderID = shaderID;

	// Create Shaders
	resource->shaderVertex = nglCreateShader( GL_VERTEX_SHADER );
	resource->shaderFragment = nglCreateShader( GL_FRAGMENT_SHADER );

	// Source Shaders
	const byte *codeVertex = Assets::binary.data + diskShader.offsetVertex;
	nglShaderSource( resource->shaderVertex, 1, reinterpret_cast<const GLchar **>( &codeVertex ),
	                                            reinterpret_cast<const GLint *>( &diskShader.sizeVertex ) );

	const byte *codeFragment = Assets::binary.data + diskShader.offsetFragment;
	nglShaderSource( resource->shaderFragment, 1, reinterpret_cast<const GLchar **>( &codeFragment ),
	                                              reinterpret_cast<const GLint *>( &diskShader.sizeFragment ) );

	// Compile Vertex Shader
	nglCompileShader( resource->shaderVertex );

	if( nglGetShaderiv( resource->shaderVertex, GL_COMPILE_STATUS, &status ), !status )
	{
	#if COMPILE_DEBUG
		// TODO: Shader names in error messages
		nglGetShaderInfoLog( resource->shaderVertex, sizeof( info ), nullptr, info );
		ErrorReturnMsg( false,
			"%s: Failed to compile vertex shader! (shader: %u)\n\n%s", __FUNCTION__, shaderID, info );
	#endif
	}

	// Compile Fragment Shader
	nglCompileShader( resource->shaderFragment );

	if( nglGetShaderiv( resource->shaderFragment, GL_COMPILE_STATUS, &status ), !status )
	{
	#if COMPILE_DEBUG
		// TODO: Shader names in error messages
		nglGetShaderInfoLog( resource->shaderFragment, sizeof( info ), nullptr, info );
		ErrorReturnMsg( false,
			"%s: Failed to compile fragment shader! (shader: %u)\n\n%s", __FUNCTION__, shaderID, info );
	#endif
	}

	// Create Program
	resource->program = nglCreateProgram();
	nglAttachShader( resource->program, resource->shaderVertex );
	nglAttachShader( resource->program, resource->shaderFragment );
	// TODO... compute shaders

	// Input Layout
	opengl_vertex_input_layout_init[Gfx::diskShaders[shaderID].vertexFormat]( resource->program );

	// Link Shader Program
	nglLinkProgram( resource->program );

	if( nglGetProgramiv( resource->program, GL_LINK_STATUS, &status ), !status )
	{
	#if COMPILE_DEBUG
		// TODO: Shader names in error messages
		nglGetProgramInfoLog( resource->program, sizeof( info ), nullptr, info );
		ErrorReturnMsg( false,
			"%s: Failed to link shader program! (shader: %u)\n\n%s", __FUNCTION__, shaderID, info );
	#endif
	}

	// Delete Shaders
	nglDeleteShader( resource->shaderVertex );
	nglDeleteShader( resource->shaderFragment );

	resource->sizeVS = diskShader.sizeVertex;
	PROFILE_GFX( Gfx::stats.gpuMemoryShaderPrograms += resource->sizeVS );

	resource->sizeVS = diskShader.sizeFragment;
	PROFILE_GFX( Gfx::stats.gpuMemoryShaderPrograms += resource->sizePS );

	// Success
	return true;
}


bool GfxCore::rb_shader_free( GfxShaderResource *&resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	PROFILE_GFX( Gfx::stats.gpuMemoryShaderPrograms -= resource->sizeVS );
	PROFILE_GFX( Gfx::stats.gpuMemoryShaderPrograms -= resource->sizePS );

	nglDeleteProgram( resource->program );
	resource->program = GL_NULL;
	shaderResources.remove( resource->id );
	resource = nullptr;

	// Success
	return true;
}


bool GfxCore::rb_shader_bind( GfxShaderResource *&resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	boundShaderResource = resource;
	nglUseProgram( resource->program );

	CHECK_ERROR( "opengl error post? %d", resource->program  );

	// Success
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////