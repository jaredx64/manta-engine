#include <manta/gfx.hpp>
#include <gfx.api.generated.hpp>

#include <config.hpp>

#include <core/traits.hpp>
#include <core/memory.hpp>
#include <core/list.hpp>
#include <core/hashmap.hpp>
#include <core/checksum.hpp>
#include <core/math.hpp>

#include <manta/window.hpp>

#include <manta/backend/gfx/opengl/opengl.hpp>
#include <manta/backend/gfx/gfxfactory.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define GRAPHICS_API_NAME "OpenGL"

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

static bool GL_ERROR()
{
	return ( glGetError() != GL_NO_ERROR );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static GfxShaderResource *boundShaderResource = nullptr;

static HashMap<u32, GLuint> uniformBufferUniformBlockIndices;
static HashMap<u32, GLint> texture2DUniformLocations;

#define GFX_RENDER_TARGET_SLOT_COUNT ( 8 )
static GLuint RT_FBO = 0;
static GLuint RT_TEXTURE_COLOR[GFX_RENDER_TARGET_SLOT_COUNT];
static GLuint RT_TEXTURE_COLOR_TYPE[GFX_RENDER_TARGET_SLOT_COUNT];
static GLuint RT_TEXTURE_DEPTH = 0;
static GLuint RT_TEXTURE_DEPTH_TYPE = GL_TEXTURE_2D;

static GLuint RT_CACHE_TARGET_FBO; // HACK: change this?
static GLuint RT_CACHE_TARGET_TEXTURE_COLOR; // HACK: change this?
static GLuint RT_CACHE_TARGET_TEXTURE_DEPTH; // HACK: change this?

static bool FILTER_MODE_MIPMAPPING = false;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct OpenGLColorFormat
{
	OpenGLColorFormat( const GLenum format, const GLenum formatInternal, const GLenum formatType ) :
		format{ format }, formatInternal{ formatInternal }, formatType{ formatType } { }

	GLenum format, formatInternal, formatType;
};


static const OpenGLColorFormat OpenGLColorFormats[] =
{
	{ GL_RGBA,         GL_RGBA8,    GL_UNSIGNED_BYTE },           // GfxColorFormat_NONE
	{ GL_RGBA,         GL_RGBA8,    GL_UNSIGNED_BYTE },           // GfxColorFormat_R8G8B8A8_FLOAT
	{ GL_RGBA_INTEGER, GL_RGBA8UI,  GL_UNSIGNED_BYTE },           // GfxColorFormat_R8G8B8A8_UINT
	{ GL_RGBA,         GL_RGB10_A2, GL_UNSIGNED_INT_10_10_10_2 }, // GfxColorFormat_R10G10B10A2_FLOAT
	{ GL_RED,          GL_R8,       GL_UNSIGNED_BYTE },           // GfxColorFormat_R8
	{ GL_RG,           GL_RG8,      GL_UNSIGNED_BYTE },           // GfxColorFormat_R8G8
	{ GL_RED,          GL_R16UI,    GL_UNSIGNED_SHORT },          // GfxColorFormat_R16
	{ GL_RED,          GL_R16F,     GL_FLOAT },                   // GfxColorFormat_R16_FLOAT
	{ GL_RG,           GL_RG16UI,   GL_UNSIGNED_SHORT },          // GfxColorFormat_R16G16
	{ GL_RG,           GL_RG16F,    GL_FLOAT },                   // GfxColorFormat_R16G16F_FLOAT
	{ GL_RGBA,         GL_RGBA16F,  GL_FLOAT },                   // GfxColorFormat_R16G16B16A16_FLOAT
	{ GL_RGBA,         GL_RGBA16UI, GL_UNSIGNED_INT },            // GfxColorFormat_R16G16B16A16_UINT
	{ GL_RED,          GL_R32F,     GL_FLOAT },                   // GfxColorFormat_R32_FLOAT
	{ GL_RG,           GL_RG32F,    GL_FLOAT },                   // GfxColorFormat_R32G32_FLOAT
	{ GL_RGBA,         GL_RGBA32F,  GL_FLOAT },                   // GfxColorFormat_R32G32B32A32_FLOAT
	{ GL_RGBA_INTEGER, GL_RGBA32UI, GL_UNSIGNED_INT },            // GfxColorFormat_R32G32B32A32_UINT
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


static const GLenum OpenGLPrimitiveTypes[] =
{
	GL_TRIANGLES,      // GfxPrimitiveType_TriangleList
	GL_TRIANGLE_STRIP, // GfxPrimitiveType_TriangleStrip
	GL_LINES,          // GfxPrimitiveType_LineList
	GL_LINE_STRIP,     // GfxPrimitiveType_LineStrip
	GL_POINTS,         // GfxPrimitiveType_PointList
};
static_assert( ARRAY_LENGTH( OpenGLPrimitiveTypes ) == GFXPRIMITIVETYPE_COUNT, "Missing GfxPrimitiveType!" );


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


static const GLint OpenGLFilteringModes[][2] =
{
	{ GL_NEAREST, GL_NEAREST_MIPMAP_LINEAR }, // GfxFilteringMode_NEAREST
	{ GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR },   // GfxFilteringMode_LINEAR
	{ GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR },   // GfxFilteringMode_ANISOTROPIC TODO: Support anisotrophic
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
	// TODO: Support these?
	GL_SRC_COLOR,            // GL_SRC1_COLOR,                   // GfxBlendFactor_SRC1_COLOR?
	GL_ONE_MINUS_SRC_COLOR,  // GL_ONE_MINUS_SRC1_COLOR,         // GfxBlendFactor_INV_SRC1_COLOR
	GL_SRC_ALPHA,            // GL_SRC1_ALPHA,                   // GfxBlendFactor_SRC1_ALPHA
	GL_ONE_MINUS_SRC_ALPHA,  // GL_ONE_MINUS_SRC1_ALPHA,         // GfxBlendFactor_INV_SRC1_ALPHA
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct GfxShaderResource : public GfxResource
{
	static void release( GfxShaderResource *&resource );
	u32 shaderID;
	GLuint program = GL_NULL;
	usize sizeVS = 0;
	usize sizePS = 0;
	usize sizeCS = 0;
};


struct GfxVertexBufferResource : public GfxResource
{
	static void release( GfxVertexBufferResource *&resource );
	GLuint vao = GL_NULL;
	GLuint vbo = GL_NULL;
	GfxCPUAccessMode accessMode;
	bool mapped = false;
	byte *data = nullptr;
	u32 stride = 0; // vertex size
	u32 offset = 0;
	u32 current = 0;
	u32 vertexFormat = 0;
	usize size = 0;
};


struct GfxInstanceBufferResource : public GfxResource
{
	static void release( GfxInstanceBufferResource *&resource );
	GLuint vao = GL_NULL;
	GLuint vbo = GL_NULL;
	GfxCPUAccessMode accessMode;
	bool mapped = false;
	byte *data = nullptr;
	u32 stride = 0; // instance size
	u32 offset = 0;
	u32 current = 0;
	u32 instanceFormat = 0;
	usize size = 0;
};


struct GfxIndexBufferResource : public GfxResource
{
	static void release( GfxIndexBufferResource *&resource );
	GLuint ebo = GL_NULL;
	GfxCPUAccessMode accessMode = GfxCPUAccessMode_NONE;
	GfxIndexBufferFormat format = GfxIndexBufferFormat_U32;
	double indicesToVerticesRatio = 1.0;
	usize size = 0;
};


struct GfxUniformBufferResource : public GfxResource
{
	static void release( GfxUniformBufferResource *&resource );
	GLuint ubo = GL_NULL;
	bool mapped = false;
	byte *data = nullptr;
	const char *name = "";
	int index = 0;
	usize size = 0;
};


struct GfxTexture2DResource : public GfxResource
{
	static void release( GfxTexture2DResource *&resource );
	GLuint textureSS = GL_NULL;
	GLuint textureMS = GL_NULL;
	GfxColorFormat colorFormat;
	u32 width = 0;
	u32 height = 0;
	u16 levels = 0;
	usize size = 0;
};


struct GfxRenderTarget2DResource : public GfxResource
{
	static void release( GfxRenderTarget2DResource *&resource );
	GLuint fboSS = GL_NULL;
	GLuint fboMS = GL_NULL;
	GLuint pboColor = GL_NULL;
	GLuint pboDepth = GL_NULL;
	GfxTexture2DResource *resourceColor = nullptr;
	GfxTexture2DResource *resourceDepth = nullptr;
	GfxRenderTargetDescription desc = { };
	u16 width = 0;
	u16 height = 0;
	usize size = 0;
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

static void opengl_input_layout_init( const GLuint program, const u32 vertexFormat, const u32 instanceFormat )
{
	GLuint location = 0;

	if( vertexFormat != U32_MAX )
	{
		// opengl_input_layout_vertex_init[] - generated table in: gfx.api.generated.hpp
		Assert( CoreGfx::opengl_input_layout_vertex_init[vertexFormat] != nullptr );
		location += CoreGfx::opengl_input_layout_vertex_init[vertexFormat]( program, location );
	}

	if( instanceFormat != U32_MAX )
	{
		// opengl_input_layout_instance_init[] - generated table in: gfx.api.generated.hpp
		Assert( CoreGfx::opengl_input_layout_instance_init[instanceFormat] != nullptr );
		location += CoreGfx::opengl_input_layout_instance_init[instanceFormat]( program, location );
	}
}


static GLuint opengl_input_layout_bind_vertex( const u32 vertexFormat, const GLuint location )
{
	if( vertexFormat == U32_MAX ) { return 0; }
	// opengl_input_layout_vertex_bind[] - generated table in: gfx.api.generated.hpp
	Assert( CoreGfx::opengl_input_layout_vertex_bind[vertexFormat] != nullptr );
	return CoreGfx::opengl_input_layout_vertex_bind[vertexFormat]( location );
}


static GLuint opengl_input_layout_bind_instance( const u32 instanceFormat, const GLuint location )
{
	if( instanceFormat == U32_MAX ) { return 0; }
	// opengl_input_layout_instance_bind[] - generated table in: gfx.api.generated.hpp
	Assert( CoreGfx::opengl_input_layout_instance_bind[instanceFormat] != nullptr );
	return CoreGfx::opengl_input_layout_instance_bind[instanceFormat]( location );
}


static GLuint opengl_uniform_buffer_uniform_block_index( GfxUniformBufferResource *const resource, const int slot )
{
	// Get hash "key" from uniformBuffer index, slot, and shader program
	char buffer[64];
	const u32 a = static_cast<u32>( resource->index );
	const u32 b = static_cast<u32>( boundShaderResource->program );
	const u32 c = static_cast<u32>( slot );
	snprintf( buffer, sizeof( buffer ), "%u_%u_%u", a, b, c ); // TODO: Refactor this?
	const u32 key = checksum_xcrc32( buffer, strlen( buffer ), 0 );

	// Block index already found?
	if( uniformBufferUniformBlockIndices.contains( key ) )
	{
		return uniformBufferUniformBlockIndices.get( key );
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

	uniformBufferUniformBlockIndices.add( key, index );
	return index;
}


static GLint opengl_texture_uniform_location( HashMap<u32, GLint> &cache, const int slot )
{
	// Get hash "key" from uniformBuffer index, slot, and shader program
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


static bool opengl_update_fbo()
{
	// Bind FBO
	nglBindFramebuffer( GL_FRAMEBUFFER, RT_FBO );
	bool rtIsActive = false;

	// Bind Color Textures
	for( int i = 0; i < GFX_RENDER_TARGET_SLOT_COUNT; i++ )
	{
		const GLenum attachment = GL_COLOR_ATTACHMENT0 + i;
		nglFramebufferTexture2D( GL_FRAMEBUFFER, attachment,
			RT_TEXTURE_COLOR_TYPE[i], RT_TEXTURE_COLOR[i], 0 );
		rtIsActive |= ( RT_TEXTURE_COLOR[i] != 0 );
	}

	// Bind Depth Texture
	nglFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
		RT_TEXTURE_DEPTH_TYPE, RT_TEXTURE_DEPTH, 0 );
	rtIsActive |= ( RT_TEXTURE_DEPTH != 0 );

	// If Active, Update Draw Buffers
	if( rtIsActive )
	{
		GLenum buffers[GFX_RENDER_TARGET_SLOT_COUNT];
		for( int i = 0; i < GFX_RENDER_TARGET_SLOT_COUNT; ++i )
		{
			buffers[i] = ( RT_TEXTURE_COLOR[i] != 0 ) ? ( GL_COLOR_ATTACHMENT0 + i ) : GL_NONE;
		}
		nglDrawBuffers( GFX_RENDER_TARGET_SLOT_COUNT, buffers );

		// Check Status
		if( nglCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
		{
			ErrorReturnMsg( false, "%s: Failed to create framebuffer", __FUNCTION__ );
		}
	}
	// Else, Unbind FBO
	else
	{
		nglBindFramebuffer( GL_FRAMEBUFFER, 0 );
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CoreGfx::api_init()
{
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
	uniformBufferUniformBlockIndices.init();
	texture2DUniformLocations.init();

	// Render Targets
	nglGenFramebuffers( 1, &RT_FBO );
	for( int i = 0; i < GFX_RENDER_TARGET_SLOT_COUNT; i++ ) { RT_TEXTURE_COLOR[i] = 0; }
	RT_TEXTURE_DEPTH = 0;

	return true;
}


bool CoreGfx::api_free()
{
	// Resources
	resources_free();

	// Uniform Caches
	uniformBufferUniformBlockIndices.free();
	texture2DUniformLocations.free();

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CoreGfx::api_frame_begin()
{
	// OpenGL does nothing
}


void CoreGfx::api_frame_end()
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CoreGfx::api_swapchain_init( const u16 width, const u16 height, const bool fullscreen )
{
	PROFILE_GFX( Gfx::stats.gpuMemorySwapchain =
		GFX_SIZE_IMAGE_COLOR_BYTES( width, height, 1, GfxColorFormat_R8G8B8A8_FLOAT ) * 2 );

#if DEPTH_BUFFER_ENABLED
	PROFILE_GFX( Gfx::stats.gpuMemorySwapchain +=
		GFX_SIZE_IMAGE_DEPTH_BYTES( width, height, 1, DEPTH_BUFFER_FORMAT ) * 2 );
#endif

	return true;
}


bool CoreGfx::api_swapchain_free()
{
	return true;
}


bool CoreGfx::api_swapchain_resize( u16 width, u16 height, bool fullscreen )
{
	PROFILE_GFX( Gfx::stats.gpuMemorySwapchain =
		GFX_SIZE_IMAGE_COLOR_BYTES( width, height, 1, GfxColorFormat_R8G8B8A8_FLOAT ) * 2 );

#if DEPTH_BUFFER_ENABLED
	PROFILE_GFX( Gfx::stats.gpuMemorySwapchain +=
		GFX_SIZE_IMAGE_DEPTH_BYTES( width, height, 1, DEPTH_BUFFER_FORMAT ) * 2 );
#endif

	opengl_update();
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CoreGfx::api_viewport_init( const u16 width, const u16 height, const bool fullscreen )
{
	GfxViewport &viewport = CoreGfx::viewport;
	viewport.width = width;
	viewport.height = height;
	viewport.fullscreen = fullscreen;

	glViewport( 0, 0, width, height );
	return true;
}


bool CoreGfx::api_viewport_free()
{
	// OpenGL does nothing
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

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CoreGfx::api_set_sampler_state( const GfxSamplerState &state )
{
	// Filter Mode
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
		OpenGLFilteringModes[state.filterMode][0] );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		OpenGLFilteringModes[state.filterMode][FILTER_MODE_MIPMAPPING] );

	// UV Wrap Mode
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
		OpenGLUVWrapModes[state.wrapMode] );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
		OpenGLUVWrapModes[state.wrapMode] );

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CoreGfx::api_set_blend_state( const GfxBlendState &state )
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
	GLboolean r = static_cast<bool>( state.colorWriteMask & GfxColorWriteFlag_RED );
	GLboolean g = static_cast<bool>( state.colorWriteMask & GfxColorWriteFlag_GREEN );
	GLboolean b = static_cast<bool>( state.colorWriteMask & GfxColorWriteFlag_BLUE );
	GLboolean a = static_cast<bool>( state.colorWriteMask & GfxColorWriteFlag_ALPHA );
	glColorMask( r, g, b, a );

	// Blend Functions
	nglBlendFuncSeparate( OpenGLBlendFactors[state.srcFactorColor], OpenGLBlendFactors[state.dstFactorColor],
		OpenGLBlendFactors[state.srcFactorAlpha], OpenGLBlendFactors[state.dstFactorAlpha] );

	// Blend Operations
	nglBlendEquationSeparate( OpenGLBlendOperations[state.blendOperationColor],
		OpenGLBlendOperations[state.blendOperationAlpha] );

	return true;
}

void CoreGfx::api_clear_color( const Color color )
{
	// Compiler likely does this regardless, but mul instruction is faster than div
	static constexpr float INV_255 = 1.0f / 255.0f;
	glClearColor( color.r * INV_255, color.g * INV_255, color.b * INV_255, color.a * INV_255 );
	glClear( GL_COLOR_BUFFER_BIT );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CoreGfx::api_set_depth_state( const GfxDepthState &state )
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

	return true;
}


void CoreGfx::api_clear_depth( const float depth )
{
	glDepthMask( true );
	glClear( GL_DEPTH_BUFFER_BIT );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GfxShaderResource::release( GfxShaderResource *&resource )
{
	if( resource == nullptr || resource->id == GFX_RESOURCE_ID_NULL ) { return; }

	if( resource->program != GL_NULL ) { nglDeleteProgram( resource->program ); }

	shaderResources.remove( resource->id );
	resource = nullptr;
}


bool CoreGfx::api_shader_init( GfxShaderResource *&resource, const u32 shaderID, const struct ShaderEntry &shaderEntry )
{
	Assert( resource == nullptr );

	char info[1024];
	int status;

	// Register Shader
	resource = shaderResources.make_new();
	resource->shaderID = shaderID;

	// Create Shaders
	GLuint shaderVertex = nglCreateShader( GL_VERTEX_SHADER );
	GLuint shaderFragment = nglCreateShader( GL_FRAGMENT_SHADER );
	GLuint shaderCompute; // TODO

	// Source Shaders
	const byte *codeVertex = Assets::binary.data + shaderEntry.offsetVertex;
	nglShaderSource( shaderVertex, 1, reinterpret_cast<const GLchar **>( &codeVertex ),
		reinterpret_cast<const GLint *>( &shaderEntry.sizeVertex ) );

	const byte *codeFragment = Assets::binary.data + shaderEntry.offsetFragment;
	nglShaderSource( shaderFragment, 1, reinterpret_cast<const GLchar **>( &codeFragment ),
		reinterpret_cast<const GLint *>( &shaderEntry.sizeFragment ) );

	// Compile Vertex Shader
	nglCompileShader( shaderVertex );

	if( nglGetShaderiv( shaderVertex, GL_COMPILE_STATUS, &status ), !status )
	{
	#if COMPILE_DEBUG
		// TODO: Shader names in error messages
		nglGetShaderInfoLog( shaderVertex, sizeof( info ), nullptr, info );
		GfxShaderResource::release( resource );
		ErrorReturnMsg( false, "%s: failed to compile vertex shader! (%s)\n\n%s",
			__FUNCTION__, shaderEntry.name, info );
	#endif
	}

	// Compile Fragment Shader
	nglCompileShader( shaderFragment );

	if( nglGetShaderiv( shaderFragment, GL_COMPILE_STATUS, &status ), !status )
	{
	#if COMPILE_DEBUG
		// TODO: Shader names in error messages
		nglGetShaderInfoLog( shaderFragment, sizeof( info ), nullptr, info );
		GfxShaderResource::release( resource );
		ErrorReturnMsg( false, "%s: Failed to compile fragment shader! (%s)\n\n%s",
			__FUNCTION__, shaderEntry.name, info );
	#endif
	}

	// Create Program
	resource->program = nglCreateProgram();
	nglAttachShader( resource->program, shaderVertex );
	nglAttachShader( resource->program, shaderFragment );
	// TODO: compute shaders (must upgrade to Opengl 4.5 & deprecate it on macOS)

	// Input Layouts
	opengl_input_layout_init( resource->program,
		CoreGfx::shaderEntries[shaderID].vertexFormat, CoreGfx::shaderEntries[shaderID].instanceFormat );

	// Link Shader Program
	nglLinkProgram( resource->program );

	if( nglGetProgramiv( resource->program, GL_LINK_STATUS, &status ), !status )
	{
	#if COMPILE_DEBUG
		// TODO: Shader names in error messages
		nglGetProgramInfoLog( resource->program, sizeof( info ), nullptr, info );
		GfxShaderResource::release( resource );
		ErrorReturnMsg( false, "%s: Failed to link shader program! (%s)\n\n%s",
			__FUNCTION__, shaderEntry.name, info );
	#endif
	}

	// Delete Shaders
	nglDeleteShader( shaderVertex );
	nglDeleteShader( shaderFragment );

	resource->sizeVS = shaderEntry.sizeVertex;
	resource->sizeVS = shaderEntry.sizeFragment;
	resource->sizeCS = 0;
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


bool CoreGfx::api_shader_bind( GfxShaderResource *&resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	boundShaderResource = resource;
	nglUseProgram( resource->program );
	CHECK_ERROR( "Failed to bind shader program: %u", resource->shaderID );

	PROFILE_GFX( Gfx::stats.frame.shaderBinds++ );
	return true;
}


bool CoreGfx::api_shader_dispatch( GfxShaderResource *&resource, const u32 x, const u32 y, const u32 z )
{
	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GfxVertexBufferResource::release( GfxVertexBufferResource *&resource )
{
	if( resource == nullptr || resource->id == GFX_RESOURCE_ID_NULL ) { return; }

	if( resource->vao != GL_NULL ) { nglDeleteVertexArrays( 1, &resource->vao ); }
	if( resource->vbo != GL_NULL ) { nglDeleteBuffers( 1, &resource->vbo ); }

	vertexBufferResources.remove( resource->id );
	resource = nullptr;
}


bool CoreGfx::api_vertex_buffer_init_dynamic( GfxVertexBufferResource *&resource, const u32 vertexFormatID,
	const GfxCPUAccessMode accessMode, const u32 size, const u32 stride )
{
	Assert( resource == nullptr );
	Assert( accessMode == GfxCPUAccessMode_WRITE_DISCARD || accessMode == GfxCPUAccessMode_WRITE_NO_OVERWRITE );

	// Register VertexBuffer
	resource = vertexBufferResources.make_new();
	resource->stride = stride;
	resource->accessMode = accessMode;
	resource->vertexFormat = vertexFormatID;
	resource->size = size;

	// Generate Buffer VAO
	nglGenVertexArrays( 1, &resource->vao );

	// Generate Buffer VBO
	nglGenBuffers( 1, &resource->vbo );

	// Setup Vertex Buffer
	nglBindBuffer( GL_ARRAY_BUFFER, resource->vbo );
	nglBufferData( GL_ARRAY_BUFFER, size, nullptr, GL_STATIC_DRAW ); // TODO: GL_DYNAMIC_DRAW

	// Error Checking
	if( GL_ERROR() )
	{
		GfxVertexBufferResource::release( resource );
		ErrorReturnMsg( false, "%s: failed to init vertex buffer (%u)", __FUNCTION__, glGetError() );
	}

	PROFILE_GFX( Gfx::stats.gpuMemoryVertexBuffers += resource->size );
	return true;
}


bool CoreGfx::api_vertex_buffer_init_static( GfxVertexBufferResource *&resource, const u32 vertexFormatID,
	const GfxCPUAccessMode accessMode, const void *const data,
	const u32 size, const u32 stride )
{
	// TODO
	// ...

	return true;
}


bool CoreGfx::api_vertex_buffer_free( GfxVertexBufferResource *&resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	PROFILE_GFX( Gfx::stats.gpuMemoryVertexBuffers -= resource->size );

	GfxVertexBufferResource::release( resource );

	return true;
}


void CoreGfx::api_vertex_buffer_write_begin( GfxVertexBufferResource *const resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	if( resource->mapped == true ) { return; }

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


void CoreGfx::api_vertex_buffer_write_end( GfxVertexBufferResource *const resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	if( resource->mapped == false ) { return; }

	nglBindBuffer( GL_ARRAY_BUFFER, resource->vbo );
	CHECK_ERROR( "Failed to bind vertex buffer for write end (resource: %u)", resource->id )

	nglUnmapBuffer( GL_ARRAY_BUFFER );
	CHECK_ERROR( "Failed to unmap vertex buffer for write end (resource: %u)", resource->id )

	resource->mapped = false;
}


bool CoreGfx::api_vertex_buffer_write( GfxVertexBufferResource *const resource, const void *const data, const u32 size )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( data != nullptr );

	Assert( resource->mapped );
	memory_copy( resource->data + resource->current, data, size );
	resource->current += size;

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

	if( resource->vao != GL_NULL ) { nglDeleteVertexArrays( 1, &resource->vao ); }
	if( resource->vbo != GL_NULL ) { nglDeleteBuffers( 1, &resource->vbo ); }

	instanceBufferResources.remove( resource->id );
	resource = nullptr;
}


bool CoreGfx::api_instance_buffer_init_dynamic( GfxInstanceBufferResource *&resource, const u32 instanceFormatID,
	const GfxCPUAccessMode accessMode, const u32 size, const u32 stride )
{
	Assert( resource == nullptr );
	Assert( accessMode == GfxCPUAccessMode_WRITE_DISCARD || accessMode == GfxCPUAccessMode_WRITE_NO_OVERWRITE );

	// Register InstanceBuffer
	resource = instanceBufferResources.make_new();
	resource->stride = stride;
	resource->accessMode = accessMode;
	resource->instanceFormat = instanceFormatID;
	resource->size = size;

	// Generate Buffer VAO
	nglGenVertexArrays( 1, &resource->vao );

	// Generate Buffer VBO
	nglGenBuffers( 1, &resource->vbo );

	// Setup Instance Buffer
	nglBindBuffer( GL_ARRAY_BUFFER, resource->vbo );
	nglBufferData( GL_ARRAY_BUFFER, size, nullptr, GL_STATIC_DRAW ); // TODO: GL_DYNAMIC_DRAW

	// Error Checking
	if( GL_ERROR() )
	{
		GfxInstanceBufferResource::release( resource );
		ErrorReturnMsg( false, "%s: failed to init instance buffer (%u)", __FUNCTION__, glGetError() );
	}

	PROFILE_GFX( Gfx::stats.gpuMemoryInstanceBuffers += resource->size );
	return true;
}


bool CoreGfx::api_instance_buffer_init_static( GfxInstanceBufferResource *&resource, const u32 instanceFormatID,
	const GfxCPUAccessMode accessMode, const void *const data,
	const u32 size, const u32 stride )
{
	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING
	return true;
}


bool CoreGfx::api_instance_buffer_free( GfxInstanceBufferResource *&resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	PROFILE_GFX( Gfx::stats.gpuMemoryInstanceBuffers -= resource->size );

	GfxInstanceBufferResource::release( resource );

	return true;
}


void CoreGfx::api_instance_buffer_write_begin( GfxInstanceBufferResource *const resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	if( resource->mapped == true ) { return; }

	nglBindBuffer( GL_ARRAY_BUFFER, resource->vbo );
	CHECK_ERROR( "Failed to bind instance buffer for write begin (resource: %u)", resource->id )

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
	CHECK_ERROR( "Failed to map instance buffer for write begin (resource: %u)", resource->id )

	resource->mapped = true;
	resource->current = 0;
	resource->offset = 0;
}


void CoreGfx::api_instance_buffer_write_end( GfxInstanceBufferResource *const resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	if( resource->mapped == false ) { return; }

	nglBindBuffer( GL_ARRAY_BUFFER, resource->vbo );
	CHECK_ERROR( "Failed to bind instance buffer for write end (resource: %u)", resource->id )

	nglUnmapBuffer( GL_ARRAY_BUFFER );
	CHECK_ERROR( "Failed to unmap instance buffer for write end (resource: %u)", resource->id )

	resource->mapped = false;
}


bool CoreGfx::api_instance_buffer_write( GfxInstanceBufferResource *const resource, const void *const data, const u32 size )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( data != nullptr );

	Assert( resource->mapped );
	memory_copy( resource->data + resource->current, data, size );
	resource->current += size;

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

	if( resource->ebo != GL_NULL ) { nglDeleteBuffers( 1, &resource->ebo ); }

	indexBufferResources.remove( resource->id );
	resource = nullptr;
}


bool CoreGfx::api_index_buffer_init( GfxIndexBufferResource *&resource, void *data, const u32 size,
	const double indToVertRatio,
	const GfxIndexBufferFormat format, const GfxCPUAccessMode accessMode )
{
	Assert( resource == nullptr );
	Assert( format != GfxIndexBufferFormat_NONE );
	Assert( format < GFXINDEXBUFFERFORMAT_COUNT );

	// Register IndexBuffer
	resource = indexBufferResources.make_new();
	resource->accessMode = accessMode;
	resource->format = format;
	resource->indicesToVerticesRatio = indToVertRatio;
	resource->size = size;

	// Generate Buffer
	nglGenBuffers( 1, &resource->ebo );

	// Bind Buffer
	nglBindBuffer( GL_ELEMENT_ARRAY_BUFFER, resource->ebo );

	// Write Index Data
	nglBufferData( GL_ELEMENT_ARRAY_BUFFER, size, data, GL_STATIC_DRAW );

	// Error Checking
	if( GL_ERROR() )
	{
		GfxIndexBufferResource::release( resource );
		ErrorReturnMsg( false, "%s: failed to init index buffer (%u)", __FUNCTION__, glGetError() );
	}

	PROFILE_GFX( Gfx::stats.gpuMemoryIndexBuffers += resource->size );
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

void GfxUniformBufferResource::release( GfxUniformBufferResource *&resource )
{
	if( resource == nullptr || resource->id == GFX_RESOURCE_ID_NULL ) { return; }

	if( resource->ubo != GL_NULL ) { nglDeleteBuffers( 1, &resource->ubo ); }

	uniformBufferResources.remove( resource->id );
	resource = nullptr;
}



bool CoreGfx::api_uniform_buffer_init( GfxUniformBufferResource *&resource, const char *name,
	const int index, const u32 size )
{
	Assert( resource == nullptr );

	// Register UniformBuffer
	resource = uniformBufferResources.make_new();
	resource->name = name;
	resource->index = index;
	resource->size = size;

	nglGenBuffers( 1, &resource->ubo );
	CHECK_ERROR( "Failed to generate constant buffer (resource: %u)", resource->id );
	if( GL_ERROR() )
	{
		GfxUniformBufferResource::release( resource );
		ErrorReturnMsg( false, "%s: failed to init uniform buffer ubo (%u)", __FUNCTION__, glGetError() );
	}

	nglBindBuffer( GL_UNIFORM_BUFFER, resource->ubo );
	nglBufferData( GL_UNIFORM_BUFFER, size, nullptr, GL_DYNAMIC_DRAW );
	nglBindBuffer( GL_UNIFORM_BUFFER, 0 );
	CHECK_ERROR( "Failed to bind constant buffer for init (resource: %u)", resource->id );

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


void CoreGfx::api_uniform_buffer_write_begin( GfxUniformBufferResource *const resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	if( resource->mapped == true ) { return; }
	resource->mapped = true;
}


void CoreGfx::api_uniform_buffer_write_end( GfxUniformBufferResource *const resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	if( resource->mapped == false ) { return; }
	resource->mapped = false;
}


bool CoreGfx::api_uniform_buffer_write( GfxUniformBufferResource *const resource, const void *data )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( data != nullptr );
	Assert( resource->mapped );

	nglBindBuffer( GL_UNIFORM_BUFFER, resource->ubo );
	nglBufferSubData( GL_UNIFORM_BUFFER, 0, static_cast<GLsizeiptr>( resource->size ), data );
	nglBindBuffer( GL_UNIFORM_BUFFER, 0 );

	return true;
}


bool CoreGfx::api_uniform_buffer_bind_vertex( GfxUniformBufferResource *const resource, const int slot )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( slot >= 0 );

	nglBindBuffer( GL_UNIFORM_BUFFER, resource->ubo );
	nglBindBufferBase( GL_UNIFORM_BUFFER, static_cast<GLuint>( resource->index ), resource->ubo );

	const GLuint blockIndex = opengl_uniform_buffer_uniform_block_index( resource, slot );
	nglUniformBlockBinding( boundShaderResource->program, blockIndex, static_cast<GLuint>( resource->index ) );

	return true;
}


bool CoreGfx::api_uniform_buffer_bind_fragment( GfxUniformBufferResource *const resource, const int slot )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( slot >= 0 );

	nglBindBuffer( GL_UNIFORM_BUFFER, resource->ubo );
	nglBindBufferBase( GL_UNIFORM_BUFFER, static_cast<GLuint>( resource->index ), resource->ubo );

	const GLuint blockIndex = opengl_uniform_buffer_uniform_block_index( resource, slot );
	nglUniformBlockBinding( boundShaderResource->program, blockIndex, static_cast<GLuint>( resource->index ) );

	return true;
}


bool CoreGfx::api_uniform_buffer_bind_compute( GfxUniformBufferResource *const resource, const int slot )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( slot >= 0 );

	nglBindBuffer( GL_UNIFORM_BUFFER, resource->ubo );
	nglBindBufferBase( GL_UNIFORM_BUFFER, static_cast<GLuint>( resource->index ), resource->ubo );

	const GLuint blockIndex = opengl_uniform_buffer_uniform_block_index( resource, slot );
	nglUniformBlockBinding( boundShaderResource->program, blockIndex, static_cast<GLuint>( resource->index ) );

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

	if( resource->textureSS != GL_NULL ) { glDeleteTextures( 1, &resource->textureSS ); }

	texture2DResources.remove( resource->id );
	resource = nullptr;
}


bool CoreGfx::api_texture_2d_init( GfxTexture2DResource *&resource, void *pixels,
	const u16 width, const u16 height, const u16 levels, const GfxColorFormat &format )
{
	Assert( resource == nullptr );

	// Cache currently bound texture
	GLint CURRENT_TEXTURE = 0;
	glGetIntegerv( GL_TEXTURE_BINDING_2D, &CURRENT_TEXTURE );

	// Register Texture2D
	resource = texture2DResources.make_new();
	resource->colorFormat = format;
	resource->width = width;
	resource->height = height;
	resource->levels = levels;

	const usize pixelSizeBytes = CoreGfx::colorFormatPixelSizeBytes[format];
	const GLint glFormatInternal = OpenGLColorFormats[format].formatInternal;
	const GLenum glFormat = OpenGLColorFormats[format].format;
	const GLenum glFormatType = OpenGLColorFormats[format].formatType;

	// Error Checking
	if( Gfx::mip_level_count_2d( width, height ) < levels )
	{
		GfxTexture2DResource::release( resource );
		ErrorReturnMsg( false, "%s: requested mip levels is more than texture size supports (attempted: %u)", levels );
	}

	if( levels >= GFX_MIP_DEPTH_MAX )
	{
		GfxTexture2DResource::release( resource );
		ErrorReturnMsg( false, "%s: exceeded max mip level count (has: %u, max: %u)", levels, GFX_MIP_DEPTH_MAX );
	}

	// Create Texture
	glGenTextures( 1, &resource->textureSS );
	if( GL_ERROR() )
	{
		GfxTexture2DResource::release( resource );
		ErrorReturnMsg( false, "%s: failed to init texture (%d)", __FUNCTION__, glGetError() );
	}

	// Setup Texture2D Data (TODO: Switch to sampler objects?)
	glBindTexture( GL_TEXTURE_2D, resource->textureSS );
	{
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			( levels > 1 ) ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, levels - 1 );
		//glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		// Upload Mip Chain
		const byte *source = reinterpret_cast<const byte *>( pixels );
		for( u16 level = 0, mipWidth = width, mipHeight = height; level < levels; level++ )
		{
			const byte *sourceLevel = source;

#if true
			// Flip texture vertically
			const usize stride = mipWidth * pixelSizeBytes;
			u8* flipped = CoreGfx::scratch_buffer( stride * mipHeight );
			for (u16 y = 0; y < mipHeight; y++)
			{
				memory_copy( &flipped[y * stride], &source[ ( mipHeight - 1 - y ) * stride], stride );
			}
			sourceLevel = flipped;
#endif

			// Upload Level Data
			glTexImage2D( GL_TEXTURE_2D, level, glFormatInternal, mipWidth, mipHeight, 0,
				glFormat, glFormatType, sourceLevel );
			if( GL_ERROR() )
			{
				GfxTexture2DResource::release( resource );
				ErrorReturnMsg( false, "%s: failed to init texture mip level (tex: %d, mip: %u)",
					__FUNCTION__, glGetError(), level );
			}

			const usize mipLevelSize = pixelSizeBytes * mipWidth * mipHeight;
			source += mipLevelSize;

			resource->size += mipLevelSize;

			mipWidth = ( mipWidth > 1 ) ? ( mipWidth >> 1 ) : 1;
			mipHeight = ( mipHeight > 1 ) ? ( mipHeight >> 1 ) : 1;
		}
	}
	glBindTexture( GL_TEXTURE_2D, CURRENT_TEXTURE );

	PROFILE_GFX( Gfx::stats.gpuMemoryTextures += resource->size );
	return true;
}


bool CoreGfx::api_texture_2d_free( GfxTexture2DResource *&resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	PROFILE_GFX( Gfx::stats.gpuMemoryTextures -= resource->size );

	GfxTexture2DResource::release( resource );

	return true;
}


bool CoreGfx::api_texture_2d_bind( GfxTexture2DResource *const resource, const int slot )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( slot >= 0 && slot < GFX_TEXTURE_SLOT_COUNT );

	// Bind Texture2D
	const GLint location = opengl_texture_uniform_location( texture2DUniformLocations, slot );
	nglUniform1i( location, slot );
	nglActiveTexture( GL_TEXTURE0 + slot );
	glBindTexture( GL_TEXTURE_2D, resource->textureSS );

	// OpenGL requires us to explitily set the sampler state on textures binds (TODO: Refactor)
	FILTER_MODE_MIPMAPPING = resource->levels > 1;
	CoreGfx::api_set_sampler_state( Gfx::state().sampler );
	FILTER_MODE_MIPMAPPING = false;

	PROFILE_GFX( Gfx::stats.frame.textureBinds++ );
	return true;
}


bool CoreGfx::api_texture_2d_release( GfxTexture2DResource *const resource, const int slot )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( slot >= 0 && slot < GFX_TEXTURE_SLOT_COUNT );

	// Do nothing...

	PROFILE_GFX( Gfx::stats.frame.textureBinds-- );
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

	if( resource->fboSS != GL_NULL ) { nglDeleteFramebuffers( 1, &resource->fboSS ); }
	if( resource->fboMS != GL_NULL ) { nglDeleteFramebuffers( 1, &resource->fboMS ); }
	if( resource->pboColor != GL_NULL ) { nglDeleteBuffers( 1, &resource->pboColor ); }
	if( resource->pboDepth != GL_NULL ) { nglDeleteBuffers( 1, &resource->pboDepth ); }

	renderTarget2DResources.remove( resource->id );
	resource = nullptr;
}


bool CoreGfx::api_render_target_2d_init( GfxRenderTarget2DResource *&resource,
	GfxTexture2DResource *&resourceColor, GfxTexture2DResource *&resourceDepth,
	const u16 width, const u16 height,
	const GfxRenderTargetDescription &desc )
{
	Assert( resource == nullptr );

	// Cache currently bound texture
	GLint CURRENT_TEXTURE = 0;
	glGetIntegerv( GL_TEXTURE_BINDING_2D, &CURRENT_TEXTURE );

	// Register RenderTarget2D
	resource = renderTarget2DResources.make_new();
	resource->width = width;
	resource->height = height;
	resource->desc = desc;

	// Color Texture
	if( desc.colorFormat != GfxColorFormat_NONE )
	{
		// Register Resource
		Assert( resourceColor == nullptr );
		resourceColor = texture2DResources.make_new();
		resource->resourceColor = resourceColor;

		const GLint glFormatInternal = OpenGLColorFormats[desc.colorFormat].formatInternal;
		const GLenum glFormat = OpenGLColorFormats[desc.colorFormat].format;
		const GLenum glFormatType = OpenGLColorFormats[desc.colorFormat].formatType;

		// Texture (Single-Sample)
		glGenTextures( 1, &resourceColor->textureSS );
		if( GL_ERROR() )
		{
			GfxTexture2DResource::release( resourceColor );
			GfxRenderTarget2DResource::release( resource );
			ErrorReturnMsg( false, "%s: failed to init color single-sample texture (%d)",
				__FUNCTION__, glGetError() );
		}

		// Sampler (Single-Sample)
		glBindTexture( GL_TEXTURE_2D, resourceColor->textureSS );
		{
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

			resource->size += GFX_SIZE_IMAGE_COLOR_BYTES( width, height, 1, desc.colorFormat );

			glTexImage2D( GL_TEXTURE_2D, 0, glFormatInternal,
				width, height, 0, glFormat, glFormatType, nullptr );

			if( GL_ERROR() )
			{
				GfxTexture2DResource::release( resourceColor );
				GfxRenderTarget2DResource::release( resource );
				PrintLn( "%u, %u", desc.colorFormat, desc.depthFormat );
				ErrorReturnMsg( false, "%s: failed to upload color single-sample texture data (%d)",
					__FUNCTION__, glGetError() );
			}
		}
		glBindTexture( GL_TEXTURE_2D, CURRENT_TEXTURE );

		if( desc.sampleCount > 1 )
		{
			// Texture (Multi-Sample)
			glGenTextures( 1, &resourceColor->textureMS );
			if( GL_ERROR() )
			{
				GfxTexture2DResource::release( resourceColor );
				GfxRenderTarget2DResource::release( resource );
				ErrorReturnMsg( false, "%s: failed to init color multi-sample texture (%d)",
					__FUNCTION__, glGetError() );
			}

			// Sampler (Multi-Sample)
			glBindTexture( GL_TEXTURE_2D_MULTISAMPLE, resourceColor->textureMS );
			{
				int sampleCount = desc.sampleCount;

				resource->size += GFX_SIZE_IMAGE_COLOR_BYTES( width, height, 1, desc.colorFormat ) * sampleCount;

				nglTexImage2DMultisample( GL_TEXTURE_2D_MULTISAMPLE, sampleCount,
					glFormatInternal, width, height, GL_TRUE);

				if( GL_ERROR() )
				{
					GfxTexture2DResource::release( resourceColor );
					GfxRenderTarget2DResource::release( resource );
					ErrorReturnMsg( false, "%s: failed to upload color multi-sample texture data (%d)",
						__FUNCTION__, glGetError() );
				}
			}
			glBindTexture( GL_TEXTURE_2D, CURRENT_TEXTURE );
		}

		// CPU Readback
		if( desc.cpuAccess )
		{
			nglGenBuffers( 1, &resource->pboColor );
			nglBindBuffer( GL_PIXEL_PACK_BUFFER, resource->pboColor );
			{
				resource->size += GFX_SIZE_IMAGE_COLOR_BYTES( width, height, 1, desc.colorFormat );
				nglBufferData( GL_PIXEL_PACK_BUFFER, GFX_SIZE_IMAGE_COLOR_BYTES( width, height, 1, desc.colorFormat ),
					nullptr, GL_STREAM_READ );
			}
			nglBindBuffer( GL_PIXEL_PACK_BUFFER, 0 );

			if( GL_ERROR() )
			{
				GfxTexture2DResource::release( resourceColor );
				GfxRenderTarget2DResource::release( resource );
				ErrorReturnMsg( false, "%s: failed to init color readback pbo (%d)",
					__FUNCTION__, glGetError() );
			}
		}
	}

	// Depth Texture
	if( desc.depthFormat != GfxDepthFormat_NONE )
	{
		// Register Resource
		Assert( resourceDepth == nullptr );
		resourceDepth = texture2DResources.make_new();
		resource->resourceDepth = resourceDepth;

		const GLint glFormatInternal = OpenGLDepthStencilFormats[desc.depthFormat].formatInternal;
		const GLenum glFormat = OpenGLDepthStencilFormats[desc.depthFormat].format;
		const GLenum glFormatType = OpenGLDepthStencilFormats[desc.depthFormat].formatType;

		// Texture (Single-Sample)
		glGenTextures( 1, &resourceDepth->textureSS );
		if( GL_ERROR() )
		{
			GfxTexture2DResource::release( resourceColor );
			GfxTexture2DResource::release( resourceDepth );
			GfxRenderTarget2DResource::release( resource );
			ErrorReturnMsg( false, "%s: failed to init depth single-sample texture (%d)",
				__FUNCTION__, glGetError() );
		}

		// Sampler (Single-Sample)
		glBindTexture( GL_TEXTURE_2D, resourceDepth->textureSS );
		{
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

			resource->size += GFX_SIZE_IMAGE_DEPTH_BYTES( width, height, 1, desc.depthFormat );

			glTexImage2D( GL_TEXTURE_2D, 0, glFormatInternal, width, height,
				0, glFormat, glFormatType, nullptr );

			if( GL_ERROR() )
			{
				GfxTexture2DResource::release( resourceColor );
				GfxTexture2DResource::release( resourceDepth );
				GfxRenderTarget2DResource::release( resource );
				ErrorReturnMsg( false, "%s: failed to upload depth single-sample texture data (%d)",
					__FUNCTION__, glGetError() );
			}
		}
		glBindTexture( GL_TEXTURE_2D, CURRENT_TEXTURE );

		if( desc.sampleCount > 1 )
		{
			// Texture (Multi-Sample)
			glGenTextures( 1, &resourceDepth->textureMS );
			if( GL_ERROR() )
			{
				GfxTexture2DResource::release( resourceColor );
				GfxTexture2DResource::release( resourceDepth );
				GfxRenderTarget2DResource::release( resource );
				ErrorReturnMsg( false, "%s: failed to init color multi-sample texture (%d)",
					__FUNCTION__, glGetError() );
			}

			// Sampler (Multi-Sample)
			glBindTexture( GL_TEXTURE_2D_MULTISAMPLE, resourceDepth->textureMS );
			{
				int sampleCount = desc.sampleCount;

				resource->size += GFX_SIZE_IMAGE_COLOR_BYTES( width, height, 1, desc.colorFormat ) * sampleCount;

				nglTexImage2DMultisample( GL_TEXTURE_2D_MULTISAMPLE, sampleCount,
					glFormatInternal, width, height, GL_TRUE);

				if( GL_ERROR() )
				{
					GfxTexture2DResource::release( resourceColor );
					GfxTexture2DResource::release( resourceDepth );
					GfxRenderTarget2DResource::release( resource );
					ErrorReturnMsg( false, "%s: failed to upload color multi-sample texture data (%d)",
						__FUNCTION__, glGetError() );
				}
			}
			glBindTexture( GL_TEXTURE_2D, CURRENT_TEXTURE );
		}

		// CPU Readback
		if( desc.cpuAccess )
		{
			nglGenBuffers( 1, &resource->pboDepth );
			nglBindBuffer( GL_PIXEL_PACK_BUFFER, resource->pboDepth );
			{
				resource->size += GFX_SIZE_IMAGE_DEPTH_BYTES( width, height, 1, desc.depthFormat );
				nglBufferData( GL_PIXEL_PACK_BUFFER, GFX_SIZE_IMAGE_DEPTH_BYTES( width, height, 1, desc.depthFormat ),
					nullptr, GL_STREAM_READ );
			}
			nglBindBuffer( GL_PIXEL_PACK_BUFFER, 0 );

			if( GL_ERROR() )
			{
				GfxTexture2DResource::release( resourceColor );
				GfxRenderTarget2DResource::release( resource );
				ErrorReturnMsg( false, "%s: failed to init depth readback pbo (%d)", __FUNCTION__, glGetError() );
			}
		}
	}

	// FBO (Single-Sample)
	nglGenFramebuffers( 1, &resource->fboSS );
	nglBindFramebuffer( GL_FRAMEBUFFER, resource->fboSS );
	{
		// Attach Color
		if( desc.colorFormat != GfxColorFormat_NONE )
		{
			nglFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, resourceColor->textureSS, 0 );
		}

		// Attach Depth
		if( desc.depthFormat != GfxDepthFormat_NONE )
		{
			nglFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, resourceDepth->textureSS, 0 );
		}

		// Check Status
		if( nglCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
		{
			GfxTexture2DResource::release( resourceColor );
			GfxTexture2DResource::release( resourceDepth );
			GfxRenderTarget2DResource::release( resource );
			ErrorReturnMsg( false, "%s: failed to create framebuffer", __FUNCTION__ );
		}
	}
	nglBindFramebuffer( GL_FRAMEBUFFER, 0 );

	// FBO (Multi-Sample)
	if( desc.sampleCount > 1 )
	{
		nglGenFramebuffers( 1, &resource->fboMS );
		nglBindFramebuffer( GL_FRAMEBUFFER, resource->fboMS );
		{
			// Attach Color
			if( desc.colorFormat != GfxColorFormat_NONE )
			{
				nglFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE,
					resourceColor->textureMS, 0 );
			}

			// Attach Depth
			if( desc.depthFormat != GfxDepthFormat_NONE )
			{
				nglFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE,
					resourceDepth->textureMS, 0 );
			}

			// Check Status
			if( nglCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
			{
				GfxTexture2DResource::release( resourceColor );
				GfxTexture2DResource::release( resourceDepth );
				GfxRenderTarget2DResource::release( resource );
				ErrorReturnMsg( false, "%s: failed to create framebuffer", __FUNCTION__ );
			}
		}
		nglBindFramebuffer( GL_FRAMEBUFFER, 0 );
	}

	PROFILE_GFX( Gfx::stats.gpuMemoryRenderTargets += resource->size );
	return true;
}


bool CoreGfx::api_render_target_2d_free( GfxRenderTarget2DResource *&resource,
	GfxTexture2DResource *&resourceColor,GfxTexture2DResource *&resourceDepth )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	PROFILE_GFX( Gfx::stats.gpuMemoryRenderTargets -= resource->size );

	// Color
	if( resource->desc.colorFormat != GfxColorFormat_NONE )
	{
		Assert( resourceColor != nullptr && resourceColor->id != GFX_RESOURCE_ID_NULL );
		GfxTexture2DResource::release( resourceColor );
	}

	// Depth
	if( resource->desc.depthFormat != GfxDepthFormat_NONE )
	{
		Assert( resourceDepth != nullptr && resourceDepth->id != GFX_RESOURCE_ID_NULL );
		GfxTexture2DResource::release( resourceDepth );
	}

	// Target
	GfxRenderTarget2DResource::release( resource );

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

	// Ensure equal dimensions
	if( srcResource->width != dstResource->width ||
		srcResource->height != dstResource->height )
	{
		return false;
	}

	// Copy FBO
	GLbitfield flags = 0;

	if( srcResource->desc.colorFormat != GfxColorFormat_NONE &&
	    dstResource->desc.colorFormat != GfxColorFormat_NONE )
	{
		flags |= GL_COLOR_BUFFER_BIT;
	}

	if( srcResource->desc.depthFormat != GfxDepthFormat_NONE &&
	    dstResource->desc.depthFormat != GfxDepthFormat_NONE )
	{
		flags |= GL_DEPTH_BUFFER_BIT;
	}

	if( flags )
	{
		nglBindFramebuffer( GL_READ_FRAMEBUFFER, srcResource->fboSS );
		nglBindFramebuffer( GL_DRAW_FRAMEBUFFER, dstResource->fboSS );
		nglBlitFramebuffer(
			static_cast<GLint>( 0 ),
			static_cast<GLint>( 0 ),
			static_cast<GLint>( srcResource->width ),
			static_cast<GLint>( srcResource->height ),
			static_cast<GLint>( 0 ),
			static_cast<GLint>( 0 ),
			static_cast<GLint>( dstResource->width ),
			static_cast<GLint>( dstResource->height ),
			flags, GL_NEAREST );
		nglBindFramebuffer( GL_FRAMEBUFFER, 0 );
	}

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

	// Copy FBO
	GLbitfield flags = 0;

	if( srcResource->desc.colorFormat != GfxColorFormat_NONE &&
	    dstResource->desc.colorFormat != GfxColorFormat_NONE )
	{
		flags |= GL_COLOR_BUFFER_BIT;
	}

	if( srcResource->desc.depthFormat != GfxDepthFormat_NONE &&
	    dstResource->desc.depthFormat != GfxDepthFormat_NONE )
	{
		flags |= GL_DEPTH_BUFFER_BIT;
	}

	const u16 srcYInv = srcResource->height - srcY;
	const u16 dstYInv = dstResource->height - dstY;

	if( flags )
	{
		nglBindFramebuffer( GL_READ_FRAMEBUFFER, srcResource->fboSS );
		nglBindFramebuffer( GL_DRAW_FRAMEBUFFER, dstResource->fboSS );
		nglBlitFramebuffer(
			static_cast<GLint>( srcX ),
			static_cast<GLint>( srcYInv - height ),
			static_cast<GLint>( srcX + width ),
			static_cast<GLint>( srcYInv ),
			static_cast<GLint>( dstX ),
			static_cast<GLint>( dstYInv - height ),
			static_cast<GLint>( dstX + width ),
			static_cast<GLint>( dstYInv ),
			flags, GL_NEAREST );
		nglBindFramebuffer( GL_FRAMEBUFFER, 0 );
	}

	return true;
}


bool CoreGfx::api_render_target_2d_buffer_read_color( GfxRenderTarget2DResource *&resource,
	GfxTexture2DResource *&resourceColor,
	void *buffer, const u32 size )
{
	ErrorIf( !resource->desc.cpuAccess,
		"Trying to CPU access a render target that does not have CPU access flag!" );

	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( resourceColor != nullptr && resourceColor->id != GFX_RESOURCE_ID_NULL );
	Assert( resource->pboColor != 0 );

	const u32 sizeSource = GFX_SIZE_IMAGE_COLOR_BYTES(
		resource->width, resource->height, 1, resource->desc.colorFormat );

	Assert( sizeSource <= size );
	Assert( buffer != nullptr );

	// Cache FBO
	GLint fboPrevious, readBufferPrevious;
	glGetIntegerv( GL_READ_FRAMEBUFFER_BINDING, &fboPrevious );
	glGetIntegerv( GL_READ_BUFFER, &readBufferPrevious );

	// Bind FBO
	nglBindFramebuffer( GL_READ_FRAMEBUFFER, resource->fboSS );
	glReadBuffer( GL_COLOR_ATTACHMENT0 );

	// Bind PBO
	nglBindBuffer( GL_PIXEL_PACK_BUFFER, resource->pboColor );
	const GLenum glFormat = OpenGLColorFormats[resource->desc.colorFormat].format;
	const GLenum glFormatType = OpenGLColorFormats[resource->desc.colorFormat].formatType;

	// Read Pixels
	glReadPixels( 0, 0, resource->width, resource->height, glFormat, glFormatType, nullptr );

	// Map PBO & Copy Data
	byte *data = reinterpret_cast<byte *>( nglMapBuffer( GL_PIXEL_PACK_BUFFER, GL_READ_ONLY ) );
	if( data == nullptr ) { return false; }

#if true
	// Flip the texture vertically
	const usize stride = resource->width * colorFormatPixelSizeBytes[resource->desc.colorFormat];
	byte *flipped = reinterpret_cast<byte *>( buffer );
	byte *source = static_cast<byte *>( data );
	for( u16 y = 0; y < resource->height; y++ )
	{
		memory_copy( &flipped[y * stride], &source[( resource->height - 1 - y ) * stride], stride );
	}
#else
	memory_copy( buffer, data, size );
#endif

	// Unmap PBO
	nglUnmapBuffer( GL_PIXEL_PACK_BUFFER );
	nglBindBuffer( GL_PIXEL_PACK_BUFFER, 0 );

	// Unbind FBO
	nglBindFramebuffer( GL_READ_FRAMEBUFFER, fboPrevious );
	glReadBuffer( readBufferPrevious );

	return true;
}


bool api_render_target_2d_buffer_read_depth( GfxRenderTarget2DResource *&resource,
	GfxTexture2DResource *&resourceDepth,
	void *buffer, const u32 size )
{
	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING
	return true;
}


bool CoreGfx::api_render_target_2d_buffer_write_color( GfxRenderTarget2DResource *&resource,
	GfxTexture2DResource *&resourceColor,
	const void *const buffer, const u32 size )
{
	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING
	return true;
}


bool CoreGfx::api_render_target_2d_buffer_write_depth( GfxRenderTarget2DResource *&resource,
	GfxTexture2DResource *&resourceDepth,
	const void *const buffer, const u32 size )
{
	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING
	return true;
}


bool CoreGfx::api_render_target_2d_bind( GfxRenderTarget2DResource *const resource, int slot )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( slot >= 0 && slot < GFX_RENDER_TARGET_SLOT_COUNT );

	const bool hasMultiSample = resource->desc.sampleCount > 1;
	const bool hasColor = resource->resourceColor != nullptr;
	const bool hasDepth = resource->resourceDepth != nullptr;

	if( hasMultiSample )
	{
		RT_TEXTURE_COLOR[slot] = hasColor ? resource->resourceColor->textureMS : 0;
		RT_TEXTURE_COLOR_TYPE[slot] = GL_TEXTURE_2D_MULTISAMPLE;

		if( slot == 0 && hasDepth )
		{
			RT_TEXTURE_DEPTH = resource->resourceDepth->textureMS;
			RT_TEXTURE_DEPTH_TYPE = GL_TEXTURE_2D_MULTISAMPLE;
		}
	}
	else
	{
		RT_TEXTURE_COLOR[slot] = hasColor ? resource->resourceColor->textureSS : 0;
		RT_TEXTURE_COLOR_TYPE[slot] = GL_TEXTURE_2D;

		if( slot == 0 && hasDepth )
		{
			RT_TEXTURE_DEPTH = resource->resourceDepth->textureSS;
			RT_TEXTURE_DEPTH_TYPE = GL_TEXTURE_2D;
		}
	}

	return opengl_update_fbo();
}


bool CoreGfx::api_render_target_2d_release( GfxRenderTarget2DResource *const resource, const int slot )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( slot >= 0 && slot < GFX_RENDER_TARGET_SLOT_COUNT );

	// MSAA Resolve
	if( resource->desc.sampleCount > 1 )
	{
		GLbitfield flags = 0;
		if( resource->desc.colorFormat != GfxColorFormat_NONE ) { flags |= GL_COLOR_BUFFER_BIT; }
		if( resource->desc.depthFormat != GfxDepthFormat_NONE ) { flags |= GL_DEPTH_BUFFER_BIT; }
		if( resource->desc.depthFormat == GfxDepthFormat_R24_UINT_G8_UINT ) { flags |= GL_STENCIL_BUFFER_BIT; }

		nglBindFramebuffer( GL_READ_FRAMEBUFFER, resource->fboMS );
		nglBindFramebuffer( GL_DRAW_FRAMEBUFFER, resource->fboSS );
		nglBlitFramebuffer( 0, 0, resource->width, resource->height,
			0, 0, resource->width, resource->height, flags, GL_NEAREST );
	}

	// Reset Slot
	RT_TEXTURE_COLOR[slot] = 0;
	RT_TEXTURE_COLOR_TYPE[slot] = GL_TEXTURE_2D;
	RT_TEXTURE_DEPTH = slot == 0 ? 0 : RT_TEXTURE_DEPTH;
	RT_TEXTURE_DEPTH_TYPE = slot == 0 ? GL_TEXTURE_2D : RT_TEXTURE_DEPTH_TYPE;

	return opengl_update_fbo();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void opengl_draw( const GLsizei vertexCount, const GLuint startVertexLocation,
	const GfxPrimitiveType type )
{
	CoreGfx::state_apply();
	Assert( type < GFXPRIMITIVETYPE_COUNT );
	glDrawArrays( OpenGLPrimitiveTypes[type], startVertexLocation, vertexCount );
	PROFILE_GFX( Gfx::stats.frame.drawCalls++ );
	PROFILE_GFX( Gfx::stats.frame.vertexCount += vertexCount );
}


static void opengl_draw_instanced( const GLsizei vertexCount, const GLuint startVertexLocation,
	const GLsizei instanceCount, const GfxPrimitiveType type )
{
	CoreGfx::state_apply();
	Assert( type < GFXPRIMITIVETYPE_COUNT );
	nglDrawArraysInstanced( OpenGLPrimitiveTypes[type], startVertexLocation, vertexCount, instanceCount );
	PROFILE_GFX( Gfx::stats.frame.drawCalls++ );
	PROFILE_GFX( Gfx::stats.frame.vertexCount += vertexCount * instanceCount );
}


static void opengl_draw_indexed( const GLsizei vertexCount, const GLuint startVertexLocation,
	const GLuint baseVertexLocation, const GfxIndexBufferFormat format, const GfxPrimitiveType type )
{
	CoreGfx::state_apply();
	Assert( type < GFXPRIMITIVETYPE_COUNT );
	glDrawElements( OpenGLPrimitiveTypes[type], vertexCount,
		OpenGLIndexBufferFormats[format], 0 );
	PROFILE_GFX( Gfx::stats.frame.drawCalls++ );
	PROFILE_GFX( Gfx::stats.frame.vertexCount += vertexCount );
}


static void opengl_draw_instanced_indexed( const GLsizei vertexCount, const GLuint startVertexLocation,
	const GLsizei instanceCount, const GLuint baseVertexLocation,
	const GfxIndexBufferFormat format, const GfxPrimitiveType type )
{
	CoreGfx::state_apply();
	Assert( type < GFXPRIMITIVETYPE_COUNT );
	nglDrawElementsInstanced( OpenGLPrimitiveTypes[type], vertexCount,
		OpenGLIndexBufferFormats[format], 0, instanceCount );
	PROFILE_GFX( Gfx::stats.frame.drawCalls++ );
	PROFILE_GFX( Gfx::stats.frame.vertexCount += vertexCount * instanceCount );
}


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

	// Bind VAO
	if( resourceVertex != nullptr )
	{
		nglBindVertexArray( resourceVertex->vao );
	}
	else if( resourceInstance != nullptr )
	{
		nglBindVertexArray( resourceInstance->vao );
	}

	// Bind Vertex Buffer
	GLuint location = 0;
	if( resourceVertex != nullptr )
	{
		nglBindBuffer( GL_ARRAY_BUFFER, resourceVertex->vbo );
		CHECK_ERROR( "Failed to bind vertex buffer for draw (resource: %u)", resourceVertex->id )
		location = opengl_input_layout_bind_vertex( resourceVertex->vertexFormat, 0 );
	}

	// Bind Instance Buffer
	if( resourceInstance != nullptr )
	{
		nglBindBuffer( GL_ARRAY_BUFFER, resourceInstance->vbo );
		CHECK_ERROR( "Failed to bind instance buffer for draw (resource: %u)", resourceInstance->id )
		opengl_input_layout_bind_instance( resourceInstance->instanceFormat, location );
	}

	// Bind Index Buffer
	if( resourceIndex != nullptr )
	{
		nglBindBuffer( GL_ELEMENT_ARRAY_BUFFER, resourceIndex->ebo );
		CHECK_ERROR( "Failed to bind index buffer for indexed draw (resource: %u)", resourceVertex->id )
	}

	// Submit Draw
	if( resourceIndex == nullptr )
	{
		if( instanceCount > 0 )
		{
			opengl_draw_instanced( vertexCount, 0, instanceCount, type );
		}
		else
		{
			opengl_draw( vertexCount, 0, type );
		}
	}
	else
	{
		const GLsizei count = static_cast<GLsizei>( vertexCount * resourceIndex->indicesToVerticesRatio );

		if( instanceCount > 0 )
		{
			opengl_draw_instanced_indexed( count, 0, instanceCount, 0, resourceIndex->format, type );
		}
		else
		{
			opengl_draw_indexed( count, 0, 0, resourceIndex->format, type );
		}
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////