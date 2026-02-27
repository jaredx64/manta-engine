#include <manta/gfx.hpp>
#include <gfx.api.generated.hpp>

#include <config.hpp>

#include <core/traits.hpp>
#include <core/memory.hpp>
#include <core/hashmap.hpp>
#include <core/checksum.hpp>
#include <core/math.hpp>

#include <manta/window.hpp>

#include <manta/backend/gfx/opengl/opengl.hpp>
#include <manta/backend/gfx/gfxfactory.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define GRAPHICS_API_NAME "OpenGL"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void glSetEnabled( GLenum cap, bool enable )
{
	if( enable ) { glEnable( cap ); return; }
	glDisable( cap );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool OPENGL_ERROR()
{
	return ( glGetError() != GL_NO_ERROR );
}

static bool OPENGL_ERROR_DETECTED = false;
#define OPENGL_CHECK_ERROR(msg,...) \
	{ \
		GLenum error = glGetError(); \
		if( error != GL_NO_ERROR && !OPENGL_ERROR_DETECTED ) \
		{ \
			OPENGL_ERROR_DETECTED = true; \
			Error( msg " (%u)\n", error, __VA_ARGS__ ); \
		} \
	}

#if COMPILE_DEBUG
struct ErrorChecker
{
	const char *function = "";

	ErrorChecker( const char *func )
	{
		function = func;
		GLenum error = glGetError();
		if( error != GL_NO_ERROR )
		{
			char message[1024];
			snprintf( message, sizeof( message ), "OpenGL State Error: %u\n    > begin scope: %s",
				error, function );
			Error( "%s", message );
		}
	}

	~ErrorChecker()
	{
		GLenum error = glGetError();
		if( error != GL_NO_ERROR )
		{
			char message[1024];
			snprintf( message, sizeof( message ), "OpenGL State Error: %u\n    > end scope: %s",
				error, function );
			Error( "%s", message );
		}
	}
};
#define OPENGL_CHECK_ERRORS_SCOPE ErrorChecker __errorChecker { __FUNCTION__ };
#else
	#define OPENGL_CHECK_ERRORS_SCOPE
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OpenGL Backend State

static GLuint stateBoundFBO = 0U;
static GLuint stateBoundColorTextures[GFX_RENDER_TARGET_SLOT_COUNT] = { 0U };
static GLuint stateBoundColorTypes[GFX_RENDER_TARGET_SLOT_COUNT] = { 0U };
static GLint stateBoundColorLayer[GFX_RENDER_TARGET_SLOT_COUNT] = { 0 };
static GLuint stateBoundDepthTexture = 0U;
static GLuint stateBoundDepthType = GL_TEXTURE_2D;
static GLint stateBoundDepthLayer = 0U;
static GfxRenderTargetResource *stateBoundTargetResources[GFX_RENDER_TARGET_SLOT_COUNT] = { nullptr };

static GLuint drawCallVAOEmpty = 0U;

static bool isDefaultTargetActive = true;
static bool isFilterModeMipmapping = false;

static HashMap<u32, GLuint> uniformBufferUniformBlockIndices;
static HashMap<u32, GLint> uniformLocationsTexture;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OpenGL Enums

struct OpenGLColorFormat { GLenum format, formatInternal, formatType; };
static const OpenGLColorFormat OpenGLColorFormats[] =
{
	{ GL_RGBA, GL_RGBA8, GL_UNSIGNED_BYTE },              // GfxColorFormat_NONE
	{ GL_RGBA, GL_RGBA8, GL_UNSIGNED_BYTE },              // GfxColorFormat_R8G8B8A8_FLOAT
	{ GL_RGBA_INTEGER, GL_RGBA8UI,    },                  // GfxColorFormat_R8G8B8A8_UINT
	{ GL_RGBA, GL_RGB10_A2, GL_UNSIGNED_INT_10_10_10_2 }, // GfxColorFormat_R10G10B10A2_FLOAT
	{ GL_RED_INTEGER, GL_R8UI, GL_UNSIGNED_BYTE },        // GfxColorFormat_R8_UINT
	{ GL_RG, GL_RG8, GL_UNSIGNED_BYTE },                  // GfxColorFormat_R8G8
	{ GL_RED_INTEGER, GL_R16UI, GL_UNSIGNED_SHORT },      // GfxColorFormat_R16_UINT
	{ GL_RED, GL_R16F, GL_FLOAT },                        // GfxColorFormat_R16_FLOAT
	{ GL_RG, GL_RG16UI, GL_UNSIGNED_SHORT },              // GfxColorFormat_R16G16
	{ GL_RG, GL_RG16F, GL_FLOAT },                        // GfxColorFormat_R16G16F_FLOAT
	{ GL_RGBA, GL_RGBA16F, GL_FLOAT },                    // GfxColorFormat_R16G16B16A16_FLOAT
	{ GL_RGBA, GL_RGBA16UI, GL_UNSIGNED_INT },            // GfxColorFormat_R16G16B16A16_UINT
	{ GL_RED, GL_R32F, GL_FLOAT },                        // GfxColorFormat_R32_FLOAT
	{ GL_RG, GL_RG32F, GL_FLOAT },                        // GfxColorFormat_R32G32_FLOAT
	{ GL_RGBA, GL_RGBA32F, GL_FLOAT },                    // GfxColorFormat_R32G32B32A32_FLOAT
	{ GL_RGBA_INTEGER, GL_RGBA32UI, GL_UNSIGNED_INT },    // GfxColorFormat_R32G32B32A32_UINT
};
static_assert( ARRAY_LENGTH( OpenGLColorFormats ) == GFXCOLORFORMAT_COUNT,
	"Missing GfxColorFormat!" );


struct OpenGLDepthStencilFormat { GLenum format, formatInternal, formatType; };
static const OpenGLDepthStencilFormat OpenGLDepthStencilFormats[] =
{
	{ GL_NONE, GL_NONE, GL_NONE },                                    // GfxDepthFormat_NONE
	{ GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT16, GL_UNSIGNED_SHORT },  // GfxDepthFormat_R16_FLOAT
	{ GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT16, GL_UNSIGNED_SHORT },  // GfxDepthFormat_R16_UINT
	{ GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT32F, GL_FLOAT },          // GfxDepthFormat_R32_FLOAT
	{ GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT32F, GL_UNSIGNED_INT },   // GfxDepthFormat_R32_UINT
	{ GL_DEPTH_STENCIL, GL_DEPTH24_STENCIL8, GL_UNSIGNED_INT_24_8 },  // GfxDepthFormat_R24_UINT_G8_UINT
};
static_assert( ARRAY_LENGTH( OpenGLDepthStencilFormats ) == GFXDEPTHFORMAT_COUNT,
	"Missing GfxDepthFormat!" );


static const GLenum OpenGLPrimitiveTypes[] =
{
	GL_TRIANGLES,      // GfxPrimitiveType_TriangleList
	GL_TRIANGLE_STRIP, // GfxPrimitiveType_TriangleStrip
	GL_LINES,          // GfxPrimitiveType_LineList
	GL_LINE_STRIP,     // GfxPrimitiveType_LineStrip
	GL_POINTS,         // GfxPrimitiveType_PointList
};
static_assert( ARRAY_LENGTH( OpenGLPrimitiveTypes ) == GFXPRIMITIVETYPE_COUNT,
	"Missing GfxPrimitiveType!" );


static const GLenum OpenGLFillModes[] =
{
	GL_FILL, // GfxFillMode_SOLID
	GL_LINE, // GfxFillMode_WIREFRAME
};
static_assert( ARRAY_LENGTH( OpenGLFillModes ) == GFXRASTERFILLMODE_COUNT,
	"Missing GfxFillMode!" );


static const GLenum OpenGLCullModes[] =
{
	GL_FRONT_AND_BACK, // GfxCullMode_NONE
	GL_FRONT,          // GfxCullMode_FRONT
	GL_BACK,           // GfxCullMode_BACK
};
static_assert( ARRAY_LENGTH( OpenGLCullModes ) == GFXRASTERCULLMODE_COUNT,
	"Missing GfxCullMode!" );


static const GLint OpenGLFilteringModes[][3] =
{
	// Mag, Min, Mip
	{ GL_NEAREST, GL_NEAREST, GL_NEAREST_MIPMAP_LINEAR }, // GfxSamplerFilteringMode_NEAREST
	{ GL_LINEAR, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR },    // GfxSamplerFilteringMode_LINEAR
	{ GL_NEAREST, GL_LINEAR, GL_NEAREST_MIPMAP_LINEAR },  // GfxSamplerFilteringMode_MAG_NEAREST_MIN_LINEAR
	{ GL_LINEAR, GL_NEAREST, GL_NEAREST_MIPMAP_LINEAR },  // GfxSamplerFilteringMode_MAG_LINEAR_MIN_NEAREST
	{ GL_LINEAR, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR },    // GfxSamplerFilteringMode_ANISOTROPIC (TODO: Support anisotrophic)
};
static_assert( ARRAY_LENGTH( OpenGLFilteringModes ) == GFXSAMPLERFILTERINGMODE_COUNT,
	"Missing GfxSamplerFilteringMode!" );


static const GLint OpenGLUVWrapModes[] =
{
	GL_REPEAT,          // GfxSamplerWrapMode_WRAP
	GL_MIRRORED_REPEAT, // GfxSamplerWrapMode_MIRROR
	GL_CLAMP_TO_EDGE,   // GfxSamplerWrapMode_CLAMP
};
static_assert( ARRAY_LENGTH( OpenGLUVWrapModes ) == GFXSAMPLERWRAPMODE_COUNT,
	"Missing GfxSamplerWrapMode!" );


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
static_assert( ARRAY_LENGTH( OpenGLBlendFactors ) == GFXBLENDFACTOR_COUNT,
	"Missing GfxBlendFactor!" );


static const GLenum OpenGLBlendOperations[] =
{
	GL_FUNC_ADD,              // GfxBlendOperation_ADD
	GL_FUNC_SUBTRACT,         // GfxBlendOperation_SUBTRACT
	GL_MIN,                   // GfxBlendOperation_MIN
	GL_MAX,                   // GfxBlendOperation_MAX
};
static_assert( ARRAY_LENGTH( OpenGLBlendOperations ) == GFXBLENDOPERATION_COUNT,
	"Missing GfxBlendOperation!" );


static const GLboolean OpenGLDepthWriteMasks[] =
{
	GL_FALSE, // GfxDepthWrite_NONE
	GL_TRUE,  // GfxDepthWrite_ALL
};
static_assert( ARRAY_LENGTH( OpenGLDepthWriteMasks ) == GFXDEPTHWRITE_COUNT,
	"Missing GfxDepthWrite!" );


static const GLenum OpenGLDepthFunctions[] =
{
	GL_ALWAYS,   // GfxDepthFunction_NONE
	GL_LESS,     // GfxDepthFunction_LESS
	GL_LEQUAL,   // GfxDepthFunction_LESS_EQUALS
	GL_GREATER,  // GfxDepthFunction_GREATER
	GL_GEQUAL,   // GfxDepthFunction_GREATER_EQUALS
	GL_EQUAL,    // GfxDepthFunction_EQUAL
	GL_NOTEQUAL, // GfxDepthFunction_NOT_EQUAL
	GL_ALWAYS,   // GfxDepthFunction_ALWAYS
};
static_assert( ARRAY_LENGTH( OpenGLDepthFunctions ) == GFXDEPTHFUNCTION_COUNT,
	"Missing GfxDepthFunction!" );


static const GLenum OpenGLIndexBufferFormats[] =
{
	GL_UNSIGNED_BYTE,  // GfxIndexBufferFormat_NONE
	GL_UNSIGNED_SHORT, // GfxIndexBufferFormat_U16
	GL_UNSIGNED_INT,   // GfxIndexBufferFormat_U32
};
static_assert( ARRAY_LENGTH( OpenGLIndexBufferFormats ) == GFXINDEXBUFFERFORMAT_COUNT,
	"Missing GfxIndexBufferFormat!" );


static const GLenum OpenGLTextureTypes[] =
{
	GL_TEXTURE_2D, // GfxTextureType_2D
	GL_TEXTURE_2D_ARRAY, // GfxTexturetype_2D_ARRAY
	GL_TEXTURE_3D, // GfxTextureType_3D
	GL_TEXTURE_CUBE_MAP, // GfxTextureType_CUBE
	GL_TEXTURE_CUBE_MAP_ARRAY, // GfxTextureType_CUBE_ARRAY
};
static_assert( ARRAY_LENGTH( OpenGLTextureTypes ) == GFXTEXTURETYPE_COUNT,
	"Missing GfxTextureType!" );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Resources

struct GfxShaderResource : public GfxResource
{
	static void release( GfxShaderResource *&resource );

	u32 shaderID = 0;
	GLuint program = 0;

	usize sizeVS = 0;
	usize sizePS = 0;
	usize sizeCS = 0;
};


struct GfxVertexBufferResource : public GfxResource
{
	static void release( GfxVertexBufferResource *&resource );

	GfxWriteMode writeMode;
	bool mapped = false;

	u32 vertexFormat = 0;
	byte *data = nullptr;
	usize size = 0;
	u32 stride = 0; // vertex size
	u32 offset = 0;
	u32 current = 0;

	usize writeRangeStart = USIZE_MAX;
	usize writeRangeEnd = 0;

	GLuint vao = 0;
	GLuint vbo = 0;
};


struct GfxInstanceBufferResource : public GfxResource
{
	static void release( GfxInstanceBufferResource *&resource );

	GfxWriteMode writeMode;
	bool mapped = false;

	u32 instanceFormat = 0;
	byte *data = nullptr;
	usize size = 0;
	u32 stride = 0; // instance size
	u32 offset = 0;
	u32 current = 0;

	usize writeRangeStart = USIZE_MAX;
	usize writeRangeEnd = 0;

	GLuint vao = 0;
	GLuint vbo = 0;
};


struct GfxIndexBufferResource : public GfxResource
{
	static void release( GfxIndexBufferResource *&resource );

	GfxWriteMode writeMode = GfxWriteMode_NONE;

	GfxIndexBufferFormat format = GfxIndexBufferFormat_U32;
	double indicesToVerticesRatio = 1.0;
	usize size = 0;

	GLuint ebo = 0;
};


struct GfxUniformBufferResource : public GfxResource
{
	static void release( GfxUniformBufferResource *&resource );

	bool mapped = false;

	byte *data = nullptr;
	const char *name = "";
	int index = 0;
	usize size = 0;

	GLuint ubo = 0;
};


struct GfxTextureResource : public GfxResource
{
	static void release( GfxTextureResource *&resource );
	GLuint textureSS = 0;
	GLuint textureMS = 0;
	GfxTextureType type;
	GfxColorFormat colorFormat;
	u16 width = 0;
	u16 height = 0;
	u16 depth = 0;
	u16 levels = 0;
	u16 layers = 0;
	usize size = 0;
};


struct GfxRenderTargetResource : public GfxResource
{
	static void release( GfxRenderTargetResource *&resource );
	GLuint fboSS = 0;
	GLuint fboMS = 0;
	GLuint pboColor = 0;
	GLuint pboDepth = 0;
	GfxTextureResource *resourceColor = nullptr;
	GfxTextureResource *resourceDepth = nullptr;
	GLsync syncObj = nullptr;
	bool asyncReadback = false;
	GfxRenderTargetDescription desc = { };
	u16 width = 0;
	u16 height = 0;
	u16 layers = 0;
	usize size = 0;
};


static GfxResourceFactory<GfxShaderResource, GFX_RESOURCE_COUNT_SHADER> shaderResources;
static GfxResourceFactory<GfxVertexBufferResource, GFX_RESOURCE_COUNT_VERTEX_BUFFER> vertexBufferResources;
static GfxResourceFactory<GfxInstanceBufferResource, GFX_RESOURCE_COUNT_INSTANCE_BUFFER> instanceBufferResources;
static GfxResourceFactory<GfxIndexBufferResource, GFX_RESOURCE_COUNT_INDEX_BUFFER> indexBufferResources;
static GfxResourceFactory<GfxUniformBufferResource, GFX_RESOURCE_COUNT_UNIFORM_BUFFER> uniformBufferResources;
static GfxResourceFactory<GfxTextureResource, GFX_RESOURCE_COUNT_TEXTURE> textureResources;
static GfxResourceFactory<GfxRenderTargetResource, GFX_RESOURCE_COUNT_RENDER_TARGET> renderTargetResources;


static bool resources_init()
{
	shaderResources.init();
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
	shaderResources.free();

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OpenGL System

static GLuint opengl_uniform_buffer_uniform_block_index( GfxUniformBufferResource *resource, int slot )
{
	// Get hash "key" from uniformBuffer index, slot, and shader program
	char buffer[64];
	const u32 a = static_cast<u32>( resource->index );
	const u32 b = static_cast<u32>( CoreGfx::state.pipeline.shader->program );
	const u32 c = static_cast<u32>( slot );
	snprintf( buffer, sizeof( buffer ), "%u_%u_%u", a, b, c ); // TODO: Refactor this?
	const u32 key = checksum_xcrc32( buffer, strlen( buffer ), 0 );

	// Block index already found?
	if( uniformBufferUniformBlockIndices.contains( key ) )
	{
		return uniformBufferUniformBlockIndices.get( key );
	}

	// Find & cache block index
	const GLuint index = nglGetUniformBlockIndex( CoreGfx::state.pipeline.shader->program,
		static_cast<const GLchar *>( resource->name ) );

	ErrorReturnIf( index == GL_INVALID_OPERATION, false,
		"OpenGL: Unable to get uniform block index (Invalid operation) (%s)",
		resource->name );

	ErrorReturnIf( index == GL_INVALID_INDEX, false,
		"OpenGL: Unable to get uniform block index (Invalid index) (%s) %d",
		resource->name, CoreGfx::state.pipeline.shader->program );

	uniformBufferUniformBlockIndices.add( key, index );
	return index;
}


static GLint opengl_texture_uniform_location( HashMap<u32, GLint> &cache, int slot )
{
	// Get hash "key" from uniformBuffer index, slot, and shader program
	char buffer[64];
	const u32 a = static_cast<u32>( CoreGfx::state.pipeline.shader->program );
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
	const GLint location = nglGetUniformLocation( CoreGfx::state.pipeline.shader->program,
		static_cast<const GLchar *>( name ) );

	ErrorReturnIf( location == GL_INVALID_OPERATION, false,
		"OpenGL: Unable to get texture uniform index (Invalid operation)" );
	ErrorReturnIf( location == GL_INVALID_VALUE, false,
		"OpenGL: Unable to get texture uniform index (Invalid value)" );

	cache.add( key, location );
	return location;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OpenGL Render State

static bool bind_shader( GfxShaderResource *resource )
{
	OPENGL_CHECK_ERRORS_SCOPE

	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	const bool dirty = BITFLAG_IS_SET( CoreGfx::state.dirtyFlags, GfxStateDirtyFlag_SHADER );
	if( !dirty && resource == CoreGfx::state.pipeline.shader ) { return true; }

	CoreGfx::state.pipeline.shader = resource;
	nglUseProgram( resource->program );
	OPENGL_CHECK_ERROR( "Failed to bind shader program: %u", resource->id );
	PROFILE_GFX( Gfx::stats.frame.shaderBinds++ );

	ErrorIf( !CoreGfx::shader_bind_uniform_buffers_vertex( resource->shaderID ),
		"Failed to bind vertex shader uniform buffers! (%u)", resource->shaderID );
	ErrorIf( !CoreGfx::shader_bind_uniform_buffers_fragment( resource->shaderID ),
		"Failed to bind fragment shader uniform buffers! (%u)", resource->shaderID );
	ErrorIf( !CoreGfx::shader_bind_uniform_buffers_compute( resource->shaderID ),
		"Failed to bind compute shader uniform buffers! (%u)", resource->shaderID );

	BITFLAG_UNSET( CoreGfx::state.dirtyFlags, GfxStateDirtyFlag_SHADER );
	return true;
}


static bool apply_pipeline_state_raster( const GfxPipelineDescription &state )
{
	OPENGL_CHECK_ERRORS_SCOPE

	GfxPipelineDescription &stateCurrent = CoreGfx::state.pipeline.description;
	const bool dirty = BITFLAG_IS_SET( CoreGfx::state.dirtyFlags, GfxStateDirtyFlag_RASTER );
	if( !dirty && state.equal_raster( stateCurrent ) )
	{
		return true;
	}

	if( dirty || state.rasterCullMode != stateCurrent.rasterCullMode )
	{
		glFrontFace( GL_CW );
		glSetEnabled( GL_CULL_FACE, state.rasterCullMode != GfxCullMode_NONE );
		glCullFace( OpenGLCullModes[state.rasterCullMode] );
	}

	if( dirty || state.rasterFillMode != stateCurrent.rasterFillMode )
	{
	#if 0 // TODO: Evaluate this, only GL_FRONT_AND_BACK appears to work
		glPolygonMode( OpenGLCullModes[state.rasterCullMode], OpenGLFillModes[state.rasterFillMode] );
	#else
		glPolygonMode( GL_FRONT_AND_BACK, OpenGLFillModes[state.rasterFillMode] );
	#endif
	}

	stateCurrent.raster_set( state );
	BITFLAG_UNSET( CoreGfx::state.dirtyFlags, GfxStateDirtyFlag_RASTER );
	return true;
}


static bool apply_pipeline_state_blend( const GfxPipelineDescription &state )
{
	OPENGL_CHECK_ERRORS_SCOPE

	GfxPipelineDescription &stateCurrent = CoreGfx::state.pipeline.description;
	const bool dirty = BITFLAG_IS_SET( CoreGfx::state.dirtyFlags, GfxStateDirtyFlag_BLEND );
	if( !dirty && state.equal_blend( stateCurrent ) ) { return true; }

	if( dirty || state.blendEnabled != stateCurrent.blendEnabled )
	{
		glSetEnabled( GL_BLEND, state.blendEnabled );
	}

	if( dirty || state.blendColorWriteMask != stateCurrent.blendColorWriteMask )
	{
		GLboolean r = static_cast<bool>( state.blendColorWriteMask & GfxBlendWrite_RED );
		GLboolean g = static_cast<bool>( state.blendColorWriteMask & GfxBlendWrite_GREEN );
		GLboolean b = static_cast<bool>( state.blendColorWriteMask & GfxBlendWrite_BLUE );
		GLboolean a = static_cast<bool>( state.blendColorWriteMask & GfxBlendWrite_ALPHA );
		glColorMask( r, g, b, a );
	}

	if( dirty ||
		state.blendSrcFactorColor != stateCurrent.blendDstFactorColor ||
		state.blendSrcFactorAlpha != stateCurrent.blendDstFactorAlpha ||
		state.blendDstFactorColor != stateCurrent.blendDstFactorColor ||
		state.blendDstFactorAlpha != stateCurrent.blendDstFactorAlpha )
	{
		nglBlendFuncSeparate( OpenGLBlendFactors[state.blendSrcFactorColor],
			OpenGLBlendFactors[state.blendDstFactorColor],
			OpenGLBlendFactors[state.blendSrcFactorAlpha],
			OpenGLBlendFactors[state.blendDstFactorAlpha] );
	}

	if( dirty ||
		state.blendOperationColor != stateCurrent.blendOperationColor ||
		state.blendOperationAlpha != stateCurrent.blendOperationAlpha )
	{
		nglBlendEquationSeparate( OpenGLBlendOperations[state.blendOperationColor],
			OpenGLBlendOperations[state.blendOperationAlpha] );
	}

	stateCurrent.blend_set( state );
	BITFLAG_UNSET( CoreGfx::state.dirtyFlags, GfxStateDirtyFlag_BLEND );
	return true;
}


static bool apply_pipeline_state_depth( const GfxPipelineDescription &state )
{
	OPENGL_CHECK_ERRORS_SCOPE

	GfxPipelineDescription &stateCurrent = CoreGfx::state.pipeline.description;
	const bool dirty = BITFLAG_IS_SET( CoreGfx::state.dirtyFlags, GfxStateDirtyFlag_DEPTH );
	if( !dirty && state.equal_depth( stateCurrent ) ) { return true; }

	if( dirty || state.depthFunction != stateCurrent.depthFunction )
	{
		const bool enabled = state.depthFunction != GfxDepthFunction_NONE;
		glSetEnabled( GL_DEPTH_TEST, enabled );
	}

	if( dirty || state.depthFunction != stateCurrent.depthFunction )
	{
		glDepthFunc( OpenGLDepthFunctions[state.depthFunction] );
	}

	if( dirty || state.depthWriteMask != stateCurrent.depthWriteMask )
	{
		glDepthMask( OpenGLDepthWriteMasks[state.depthWriteMask] );
	}

	stateCurrent.depth_set_state( state );
	BITFLAG_UNSET( CoreGfx::state.dirtyFlags, GfxStateDirtyFlag_DEPTH );
	return true;
}


static bool apply_pipeline_state_stencil( const GfxPipelineDescription &state )
{
	OPENGL_CHECK_ERRORS_SCOPE

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
	OPENGL_CHECK_ERRORS_SCOPE

	const bool dirty = BITFLAG_IS_SET( CoreGfx::state.dirtyFlags, GfxStateDirtyFlag_SCISSOR );
	if( !dirty && state == CoreGfx::state.scissor ) { return true; }

	if( state.is_enabled() )
	{
		glEnable( GL_SCISSOR_TEST );
		GLint x = static_cast<GLint>( state.x1 );
		GLint y = static_cast<GLint>( Window::height - state.y2 ); // Flip y
		GLint w = static_cast<GLint>( state.x2 - state.x1 );
		GLint h = static_cast<GLint>( state.y2 - state.y1 );
		glScissor( x * Window::scale, y * Window::scale, w * Window::scale, h * Window::scale );
	}
	else
	{
		glDisable( GL_SCISSOR_TEST );
	}

	CoreGfx::state.scissor = state;
	BITFLAG_UNSET( CoreGfx::state.dirtyFlags, GfxStateDirtyFlag_SCISSOR );
	return true;
}


static void render_target_resolve_msaa( GfxRenderTargetResource *const resource )
{
	OPENGL_CHECK_ERRORS_SCOPE
	if( resource == nullptr ) { return; }
	if( resource->desc.sampleCount <= 1 ) { return; }

	GLbitfield flags = 0;
	if( resource->desc.colorFormat != GfxColorFormat_NONE ) { flags |= GL_COLOR_BUFFER_BIT; }
	if( resource->desc.depthFormat != GfxDepthFormat_NONE ) { flags |= GL_DEPTH_BUFFER_BIT; }
	if( resource->desc.depthFormat == GfxDepthFormat_R24_UINT_G8_UINT ) { flags |= GL_STENCIL_BUFFER_BIT; }

	GLint fboReadPrevious, fboDrawPrevious;
	glGetIntegerv( GL_READ_FRAMEBUFFER_BINDING, &fboReadPrevious );
	glGetIntegerv( GL_DRAW_FRAMEBUFFER_BINDING, &fboDrawPrevious );
	nglBindFramebuffer( GL_READ_FRAMEBUFFER, resource->fboMS );
	nglBindFramebuffer( GL_DRAW_FRAMEBUFFER, resource->fboSS );
	nglBlitFramebuffer( 0, 0, resource->width, resource->height, 0, 0,
		resource->width, resource->height, flags, GL_NEAREST );
	nglBindFramebuffer( GL_READ_FRAMEBUFFER, fboReadPrevious );
	nglBindFramebuffer( GL_DRAW_FRAMEBUFFER, fboDrawPrevious );
}


static bool opengl_texture_type_is_2d( GLuint type )
{
	switch( type )
	{
		case GL_TEXTURE_2D:
		case GL_TEXTURE_2D_MULTISAMPLE:
		case GL_TEXTURE_CUBE_MAP:
			return true;

		default:
			return false;
	}
}


static bool opengl_texture_type_is_layered( GLuint type )
{
	switch( type )
	{
		case GL_TEXTURE_2D_ARRAY:
		case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
		case GL_TEXTURE_3D:
		case GL_TEXTURE_CUBE_MAP_ARRAY:
			return true;

		default:
			return false;
	}
}


static bool bind_targets( GfxRenderTargetResource *const *resources, const int *layers )
{
	OPENGL_CHECK_ERRORS_SCOPE
	static double_m44 cacheMatrixModel;
	static double_m44 cacheMatrixView;
	static double_m44 cacheMatrixPerspective;

	bool dirtySlot[GFX_RENDER_TARGET_SLOT_COUNT] = { false };
	bool dirtyState = BITFLAG_IS_SET( CoreGfx::state.dirtyFlags, GfxStateDirtyFlag_TARGETS );
	bool hasCustomRenderTargets = false;

	// FBO State Cache
	for( int slot = 0; slot < GFX_RENDER_TARGET_SLOT_COUNT; slot++ )
	{
		GfxRenderTargetResource *const resource = resources[slot];
		const int layer = layers[slot];

		if( stateBoundTargetResources[slot] != nullptr && stateBoundTargetResources[slot] != resource )
		{
			render_target_resolve_msaa( stateBoundTargetResources[slot] );
		}

		hasCustomRenderTargets |= ( resource != nullptr );
		dirtySlot[slot] |= ( stateBoundTargetResources[slot] != resource );
		stateBoundTargetResources[slot] = resource;

		if( resource == nullptr )
		{
			dirtySlot[slot] |= ( stateBoundColorTextures[slot] != 0 );
			stateBoundColorTextures[slot] = 0;

			if( slot == 0 )
			{
				dirtySlot[slot] |= ( stateBoundDepthTexture != 0 );
				stateBoundDepthTexture = 0;
			}
		}
		else
		{
			const bool hasMultiSample = resource->desc.sampleCount > 1;
			const bool hasColor = resource->resourceColor != nullptr;
			hasCustomRenderTargets |= hasColor;
			const bool hasDepth = resource->resourceDepth != nullptr;
			hasCustomRenderTargets |= hasDepth;

			// Color
			if( hasColor )
			{
				const GLuint colorTexture = hasMultiSample ?
					resource->resourceColor->textureMS : resource->resourceColor->textureSS;
				dirtySlot[slot] |= ( stateBoundColorTextures[slot] != colorTexture );
				stateBoundColorTextures[slot] = colorTexture;

				if( resource->resourceColor->type == GfxTextureType_2D )
				{
					Assert( layer == 0 );
					const GLuint colorType = hasMultiSample ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
					dirtySlot[slot] |= ( stateBoundColorTypes[slot] != colorType );
					stateBoundColorTypes[slot] = colorType;
					dirtySlot[slot] |= ( stateBoundColorLayer[slot] != 0 );
					stateBoundColorLayer[slot] = 0;
				}
				else if( resource->resourceColor->type == GfxTexturetype_2D_ARRAY )
				{
					Assert( layer < resource->resourceColor->layers );
					const GLuint colorType = hasMultiSample ? GL_TEXTURE_2D_MULTISAMPLE_ARRAY : GL_TEXTURE_2D_ARRAY;
					dirtySlot[slot] |= ( stateBoundColorTypes[slot] != colorType );
					stateBoundColorTypes[slot] = colorType;
					dirtySlot[slot] |= ( stateBoundColorLayer[slot] != layer );
					stateBoundColorLayer[slot] = layer;
				}
				else
				{
					AssertMsg( false, "Unexpected GfxTextureType!", resource->resourceColor->type );
				}
			}
			else
			{
				dirtySlot[slot] |= ( stateBoundColorTextures[slot] != 0 );
				stateBoundColorTextures[slot] = 0;
				dirtySlot[slot] |= ( stateBoundColorLayer[slot] != 0 );
				stateBoundColorLayer[slot] = 0;
			}

			// Depth
			if( slot == 0 )
			{
				if( hasDepth )
				{
					const GLuint depthTexture = hasMultiSample ?
						resource->resourceDepth->textureMS : resource->resourceDepth->textureSS;
					dirtySlot[slot] |= ( stateBoundDepthTexture != depthTexture );
					stateBoundDepthTexture = depthTexture;

					if( resource->resourceDepth->type == GfxTextureType_2D )
					{
						Assert( layer == 0 );
						const GLuint depthType = hasMultiSample ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
						dirtySlot[slot] |= ( stateBoundDepthType != depthType );
						stateBoundDepthType = depthType;
						dirtySlot[slot] |= ( stateBoundDepthLayer != 0 );
						stateBoundDepthLayer = 0;
					}
					else if( resource->resourceDepth->type == GfxTexturetype_2D_ARRAY )
					{
						Assert( layer < resource->resourceDepth->layers );
						const GLuint depthType = hasMultiSample ? GL_TEXTURE_2D_MULTISAMPLE_ARRAY : GL_TEXTURE_2D_ARRAY;
						dirtySlot[slot] |= ( stateBoundDepthType != depthType );
						stateBoundDepthType = depthType;
						dirtySlot[slot] |= ( stateBoundDepthLayer != layer );
						stateBoundDepthLayer = layer;
					}
				}
				else
				{
					dirtySlot[slot] |= ( stateBoundDepthTexture != 0 );
					stateBoundDepthTexture = 0;
					dirtySlot[slot] |= ( stateBoundDepthLayer != 0 );
					stateBoundDepthLayer = 0;
				}
			}
		}

		dirtyState |= dirtySlot[slot];
	}

	isDefaultTargetActive = !hasCustomRenderTargets;
	if( !dirtyState ) { return true; }

#if !SWAPCHAIN_DEPTH_ENABLED
	if( isDefaultTargetActive )
	{
		// NOTE: When returning to the default swapchain and not SWAPCHAIN_DEPTH_ENABLED, we explicitly disable depth
		GfxPipelineDescription desc = CoreGfx::state.pipeline.description;
		desc.depthFunction = GfxDepthFunction_NONE;
		desc.depthWriteMask = GfxDepthWrite_NONE;
		apply_pipeline_state_depth( desc );
	}
#endif

	if( hasCustomRenderTargets )
	{
		nglBindFramebuffer( GL_FRAMEBUFFER, stateBoundFBO );

		// Color Textures
		for( int slot = 0; slot < GFX_RENDER_TARGET_SLOT_COUNT; slot++ )
		{
			if( !dirtySlot[slot] ) { continue; }

			const GLenum attachment = GL_COLOR_ATTACHMENT0 + slot;

			if( opengl_texture_type_is_2d( stateBoundColorTypes[slot] ) )
			{
				nglFramebufferTexture2D( GL_FRAMEBUFFER, attachment,
					stateBoundColorTypes[slot], stateBoundColorTextures[slot], 0 );
			}
			else if( opengl_texture_type_is_layered( stateBoundColorTypes[slot] ) )
			{
				nglFramebufferTextureLayer( GL_FRAMEBUFFER, attachment,
					stateBoundColorTextures[slot], 0, stateBoundColorLayer[slot] );
			}
			else
			{
				AssertMsg( false, "Unexpected texture type! %u", stateBoundColorTypes[slot] );
			}
		}

		// Depth Texture
		if( dirtySlot[0] )
		{
			if( opengl_texture_type_is_2d( stateBoundDepthType ) )
			{
				nglFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
					stateBoundDepthType, stateBoundDepthTexture, 0 );
			}
			else if( opengl_texture_type_is_layered( stateBoundDepthType ) )
			{
				nglFramebufferTextureLayer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
					stateBoundDepthTexture, 0, stateBoundDepthLayer );
			}
			else
			{
				AssertMsg( false, "Unexpected texture type! %u", stateBoundDepthType );
			}

		}

		GLenum buffers[GFX_RENDER_TARGET_SLOT_COUNT];
		for( int slot = 0; slot < GFX_RENDER_TARGET_SLOT_COUNT; ++slot )
		{
			buffers[slot] = stateBoundColorTextures[slot] != 0 ? GL_COLOR_ATTACHMENT0 + slot : GL_NONE;
		}
		nglDrawBuffers( GFX_RENDER_TARGET_SLOT_COUNT, buffers );

		if( nglCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
		{
			ErrorReturnMsg( false, "%s: Failed to create framebuffer", __FUNCTION__ );
		}
	}
	else
	{
		nglBindFramebuffer( GL_FRAMEBUFFER, 0 );
	}

	// OpenGL: When binding to a FBO, we need to update the viewport and MVP matrices
	if( hasCustomRenderTargets )
	{
		cacheMatrixModel = Gfx::get_matrix_model();
		cacheMatrixView = Gfx::get_matrix_view();
		cacheMatrixPerspective = Gfx::get_matrix_perspective();

		if( resources[0] != nullptr )
		{
			const u16 width = resources[0]->width;
			const u16 height = resources[0]->height;
			Gfx::set_matrix_model( double_m44_build_identity() );
			Gfx::set_matrix_view( double_m44_build_identity() );
			Gfx::set_matrix_perspective( double_m44_build_orthographic( 0.0, width, 0.0, height, 0.0, 1.0 ) );
			Gfx::viewport_set_size( width, height );
		}
	}
	else
	{
		Gfx::set_matrix_mvp( cacheMatrixModel, cacheMatrixView, cacheMatrixPerspective );
		Gfx::viewport_set_size( Gfx::swapchain_width(), Gfx::swapchain_height() );
	}

	BITFLAG_UNSET( CoreGfx::state.dirtyFlags, GfxStateDirtyFlag_TARGETS );
	return true;
}


static void render_targets_reset()
{
	OPENGL_CHECK_ERRORS_SCOPE

	GfxRenderTargetResource *targets[GFX_RENDER_TARGET_SLOT_COUNT] = { nullptr };
	int layers[GFX_RENDER_TARGET_SLOT_COUNT] = { 0 };
	bind_targets( targets, layers );
}


static void render_pass_validate()
{
	OPENGL_CHECK_ERRORS_SCOPE

	if( CoreGfx::state.renderPassActive ) { return; }
	if( isDefaultTargetActive ) { return; }
	render_targets_reset();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// GFX System

bool CoreGfx::init_api()
{
#if SWAPCHAIN_DPI_SCALED
	const u16 w = Window::width;
	const u16 h = Window::height;
#else
	const u16 w = Window::width * Window::scale;
	const u16 h = Window::height * Window::scale;
#endif

	ErrorReturnIf( !opengl_init(), false, "%s: Failed to init OpenGL", __FUNCTION__ );
	OPENGL_CHECK_ERRORS_SCOPE
	ErrorReturnIf( !api_swapchain_init( w, h ), false, "%s: Failed to create swap chain", __FUNCTION__ );
	ErrorReturnIf( !resources_init(), false, "%s: Failed to create gfx resources", __FUNCTION__ );

	uniformBufferUniformBlockIndices.init();

	uniformLocationsTexture.init();

	nglGenVertexArrays( 1, &drawCallVAOEmpty );

	nglGenFramebuffers( 1, &stateBoundFBO );

	return true;
}


bool CoreGfx::free_api()
{
	nglDeleteFramebuffers( 1, &stateBoundFBO );
	stateBoundFBO = 0;

	nglDeleteVertexArrays( 1, &drawCallVAOEmpty );
	drawCallVAOEmpty = 0U;

	uniformLocationsTexture.free();
	uniformBufferUniformBlockIndices.free();

	resources_free();
	api_swapchain_free();
	opengl_free();
	resources_free();

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Frame

void CoreGfx::api_frame_begin()
{
	OPENGL_CHECK_ERRORS_SCOPE

	bind_shader( CoreGfx::shaders[Shader::SHADER_DEFAULT].resource );

	render_targets_reset();

	CoreGfx::state.scissor = GfxStateScissor { };
	BITFLAG_SET( CoreGfx::state.dirtyFlags, GfxStateDirtyFlag_SCISSOR );

	api_sampler_set_state( GfxStateSampler { } );

	BITFLAG_SET( CoreGfx::state.dirtyFlags, GfxStateDirtyFlag_RASTER );
	BITFLAG_SET( CoreGfx::state.dirtyFlags, GfxStateDirtyFlag_BLEND );
	BITFLAG_SET( CoreGfx::state.dirtyFlags, GfxStateDirtyFlag_DEPTH );
	BITFLAG_SET( CoreGfx::state.dirtyFlags, GfxStateDirtyFlag_STENCIL );
}


void CoreGfx::api_frame_end()
{
	OPENGL_CHECK_ERRORS_SCOPE

#if COMPILE_DEBUG
	for( ;; )
	{
		int error = glGetError();
		if( error == GL_NO_ERROR ) { break; }
		Error( "OpenGL Error: %d\n", error );
	}
#endif

	const bool swap = opengl_swap();
	Assert( swap );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Clear

void CoreGfx::api_clear_color( Color color )
{
	OPENGL_CHECK_ERRORS_SCOPE
	static constexpr float INV_255 = 1.0f / 255.0f;
	glClearColor( color.r * INV_255, color.g * INV_255, color.b * INV_255, color.a * INV_255 );
	glClear( GL_COLOR_BUFFER_BIT );
}


void CoreGfx::api_clear_depth( float depth )
{
	OPENGL_CHECK_ERRORS_SCOPE
	glDepthMask( true );
	glClear( GL_DEPTH_BUFFER_BIT );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Render State

bool CoreGfx::api_swapchain_init( u16 width, u16 height )
{
	OPENGL_CHECK_ERRORS_SCOPE

	CoreGfx::state.swapchain.width = width;
	CoreGfx::state.swapchain.height = height;

#if SWAPCHAIN_DEPTH_ENABLED
	PROFILE_GFX( Gfx::stats.gpuMemorySwapchain +=
		GFX_SIZE_IMAGE_DEPTH_BYTES( width, height, 1, SWAPCHAIN_DEPTH_FORMAT ) * 2 );
#endif

	PROFILE_GFX( Gfx::stats.gpuMemorySwapchain +=
		GFX_SIZE_IMAGE_COLOR_BYTES( width, height, 1, GfxColorFormat_R8G8B8A8_FLOAT ) * 2 );

	// NOTE:
	// Since we use do not explicitly create any swapchain color/depth attachments, OpenGL falls back to the
	// system supplied swapchain for the window. Therefore, we do not have to do anything resource-wise here.

	CoreGfx::api_viewport_set_size( width, height );
	return true;
}


bool CoreGfx::api_swapchain_free()
{
	OPENGL_CHECK_ERRORS_SCOPE

	PROFILE_GFX( Gfx::stats.gpuMemorySwapchain -= GFX_SIZE_IMAGE_COLOR_BYTES( CoreGfx::state.swapchain.width,
		CoreGfx::state.swapchain.height, 1, GfxColorFormat_R8G8B8A8_FLOAT ) * 2 );

#if SWAPCHAIN_DEPTH_ENABLED
	PROFILE_GFX( Gfx::stats.gpuMemorySwapchain -= GFX_SIZE_IMAGE_DEPTH_BYTES( CoreGfx::state.swapchain.width,
		CoreGfx::state.swapchain.height, 1, SWAPCHAIN_DEPTH_FORMAT ) * 2 );
#endif

	// NOTE:
	// Since we use do not explicitly create any swapchain color/depth attachments, OpenGL falls back to the
	// system supplied swapchain for the window. Therefore, we do not have to do anything resource-wise here.

	return true;
}


bool CoreGfx::api_swapchain_set_size( u16 width, u16 height )
{
	OPENGL_CHECK_ERRORS_SCOPE

	const u16 widthPrevious = CoreGfx::state.swapchain.width;
	const u16 heightPrevious = CoreGfx::state.swapchain.height;

	CoreGfx::state.swapchain.width = width;
	CoreGfx::state.swapchain.height = height;

	PROFILE_GFX( Gfx::stats.gpuMemorySwapchain -=
		GFX_SIZE_IMAGE_COLOR_BYTES( widthPrevious, heightPrevious, 1, GfxColorFormat_R8G8B8A8_FLOAT ) * 2 );
	PROFILE_GFX( Gfx::stats.gpuMemorySwapchain +=
		GFX_SIZE_IMAGE_COLOR_BYTES( width, height, 1, GfxColorFormat_R8G8B8A8_FLOAT ) * 2 );

#if SWAPCHAIN_DEPTH_ENABLED
	PROFILE_GFX( Gfx::stats.gpuMemorySwapchain -=
		GFX_SIZE_IMAGE_DEPTH_BYTES( widthPrevious, heightPrevious, 1, SWAPCHAIN_DEPTH_FORMAT ) * 2 );
	PROFILE_GFX( Gfx::stats.gpuMemorySwapchain +=
		GFX_SIZE_IMAGE_DEPTH_BYTES( width, height, 1, SWAPCHAIN_DEPTH_FORMAT ) * 2 );
#endif

	// NOTE:
	// Since we use do not explicitly create any swapchain color/depth attachments, OpenGL falls back to the
	// system supplied swapchain for the window. Therefore, resizing is automatically handled when the window
	// resizes so we do not have to do anything resource-wise here.

	opengl_update();
	render_targets_reset();
	return true;
}


bool CoreGfx::api_viewport_init( u16 width, u16 height )
{
	OPENGL_CHECK_ERRORS_SCOPE
	return api_viewport_set_size( width, height );
}


bool CoreGfx::api_viewport_free()
{
	OPENGL_CHECK_ERRORS_SCOPE
	return true;
}


bool CoreGfx::api_viewport_set_size( u16 width, u16 height )
{
	OPENGL_CHECK_ERRORS_SCOPE

	CoreGfx::state.viewport.width = width;
	CoreGfx::state.viewport.height = height;

	glViewport( 0, 0, width, height );

	return true;
}


bool CoreGfx::api_sampler_set_state( const GfxStateSampler &state )
{
	OPENGL_CHECK_ERRORS_SCOPE

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
		OpenGLFilteringModes[state.filterMode][0] );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		OpenGLFilteringModes[state.filterMode][isFilterModeMipmapping ? 2 : 1] );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
		OpenGLUVWrapModes[state.wrapMode] );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
		OpenGLUVWrapModes[state.wrapMode] );

	CoreGfx::state.sampler = state;
	return true;
}


bool CoreGfx::api_scissor_set_state( const GfxStateScissor &state )
{
	OPENGL_CHECK_ERRORS_SCOPE
	return apply_state_scissor( state );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Pipeline State

bool CoreGfx::api_set_raster( const GfxPipelineDescription &description )
{
	OPENGL_CHECK_ERRORS_SCOPE
	return apply_pipeline_state_raster( description );
}


bool CoreGfx::api_set_blend( const GfxPipelineDescription &description )
{
	OPENGL_CHECK_ERRORS_SCOPE
	return apply_pipeline_state_blend( description );
}


bool CoreGfx::api_set_depth( const GfxPipelineDescription &description )
{
	OPENGL_CHECK_ERRORS_SCOPE
	return apply_pipeline_state_depth( description );
}


bool CoreGfx::api_set_stencil( const GfxPipelineDescription &description )
{
	OPENGL_CHECK_ERRORS_SCOPE
	return apply_pipeline_state_stencil( description );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Shader

void GfxShaderResource::release( GfxShaderResource *&resource )
{
	OPENGL_CHECK_ERRORS_SCOPE
	if( resource == nullptr || resource->id == GFX_RESOURCE_ID_NULL ) { return; }

	if( resource->program != 0 ) { nglDeleteProgram( resource->program ); }

	shaderResources.remove( resource->id );
	resource = nullptr;
}


bool CoreGfx::api_shader_init( GfxShaderResource *&resource, u32 shaderID,
	const struct ShaderEntry &shaderEntry )
{
	OPENGL_CHECK_ERRORS_SCOPE
	Assert( resource == nullptr );
	if( shaderEntry.stages == 0 ) { return true; }

	char info[1024];
	int status;

	resource = shaderResources.make_new();
	resource->shaderID = shaderID;
	resource->program = nglCreateProgram();

	// Vertex Shader
	if( CoreGfx::shader_has_stage( shaderID, GfxShaderStage_Vertex ) )
	{
		GLuint shaderVertex = nglCreateShader( GL_VERTEX_SHADER );

		const byte *codeVertex = Assets::binary.data + shaderEntry.offsetVertex + BINARY_OFFSET_GFX;
		nglShaderSource( shaderVertex, 1, reinterpret_cast<const GLchar **>( &codeVertex ),
			reinterpret_cast<const GLint *>( &shaderEntry.sizeVertex ) );

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

		nglAttachShader( resource->program, shaderVertex );
		nglDeleteShader( shaderVertex );

		// Vertex / Instance Formats
		GLuint inputLayoutElementsCount = 0U;
		if( shaderEntry.vertexFormat != U32_MAX )
		{
			Assert( shaderEntry.vertexFormat < inputLayoutFormatsVertexCount );
			const OpenGLInputLayoutFormats &table = inputLayoutFormatsVertex[shaderEntry.vertexFormat];
			for( GLuint i = 0; i < table.attributesCount; i++ )
			{
				const OpenGLInputLayoutAttributes &attributes = table.attributes[i];
				nglBindAttribLocation( resource->program, inputLayoutElementsCount, attributes.name );
				inputLayoutElementsCount++;
			}
		}

		if( shaderEntry.instanceFormat != U32_MAX )
		{
			Assert( shaderEntry.instanceFormat < inputLayoutFormatsInstanceCount );
			const OpenGLInputLayoutFormats &table = inputLayoutFormatsInstance[shaderEntry.instanceFormat];
			for( GLuint i = 0; i < table.attributesCount; i++ )
			{
				const OpenGLInputLayoutAttributes &attributes = table.attributes[i];
				nglBindAttribLocation( resource->program, inputLayoutElementsCount, attributes.name );
				inputLayoutElementsCount++;
			}
		}
	}

	// Fragment Shader
	if( CoreGfx::shader_has_stage( shaderID, GfxShaderStage_Vertex ) )
	{
		GLuint shaderFragment = nglCreateShader( GL_FRAGMENT_SHADER );

		const byte *codeFragment = Assets::binary.data + shaderEntry.offsetFragment + BINARY_OFFSET_GFX;
		nglShaderSource( shaderFragment, 1, reinterpret_cast<const GLchar **>( &codeFragment ),
			reinterpret_cast<const GLint *>( &shaderEntry.sizeFragment ) );

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

		nglAttachShader( resource->program, shaderFragment );
		nglDeleteShader( shaderFragment );
	}

	// Compute Shader
	if( CoreGfx::shader_has_stage( shaderID, GfxShaderStage_Compute ) )
	{
		// TODO: compute shaders (must upgrade to Opengl 4.5 & deprecate it on macOS)
		GLuint shaderCompute;
	}

	// Link Shader
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
	OPENGL_CHECK_ERRORS_SCOPE
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	PROFILE_GFX( Gfx::stats.gpuMemoryShaders -= resource->sizeVS );
	PROFILE_GFX( Gfx::stats.gpuMemoryShaders -= resource->sizePS );
	PROFILE_GFX( Gfx::stats.gpuMemoryShaders -= resource->sizeCS );

	GfxShaderResource::release( resource );

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Buffer

bool CoreGfx::api_buffer_init( GfxBufferResource *&resource, usize capacity )
{
	return true;
}


bool CoreGfx::api_buffer_free( GfxBufferResource *&resource )
{
	return true;
}


void CoreGfx::api_buffer_write_begin( GfxBufferResource *resource )
{
	// ...
}


void CoreGfx::api_buffer_write_end( GfxBufferResource *resource )
{
	// ...
}


void CoreGfx::api_buffer_write( GfxBufferResource *resource, const void *data, usize size, usize offset )
{
	// ...
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Vertex Buffer

void GfxVertexBufferResource::release( GfxVertexBufferResource *&resource )
{
	OPENGL_CHECK_ERRORS_SCOPE
	if( resource == nullptr || resource->id == GFX_RESOURCE_ID_NULL ) { return; }

	if( resource->vao != 0 ) { nglDeleteVertexArrays( 1, &resource->vao ); }
	if( resource->vbo != 0 ) { nglDeleteBuffers( 1, &resource->vbo ); }

	vertexBufferResources.remove( resource->id );
	resource = nullptr;
}


bool CoreGfx::api_vertex_buffer_init( GfxVertexBufferResource *&resource, u32 vertexFormatID,
	GfxWriteMode writeMode, u32 capacity, u32 stride )
{
	OPENGL_CHECK_ERRORS_SCOPE
	Assert( resource == nullptr );
	resource = vertexBufferResources.make_new();
	resource->writeMode = writeMode == GfxWriteMode_RING ? GfxWriteMode_OVERWRITE : writeMode; // TODO
	resource->mapped = false;
	resource->vertexFormat = vertexFormatID;
	resource->size = capacity;
	resource->stride = stride;

	nglGenVertexArrays( 1, &resource->vao );
	nglGenBuffers( 1, &resource->vbo );

	nglBindBuffer( GL_ARRAY_BUFFER, resource->vbo );
	nglBufferData( GL_ARRAY_BUFFER, capacity, nullptr, GL_STATIC_DRAW ); // TODO: GL_DYNAMIC_DRAW

	if( OPENGL_ERROR() )
	{
		GfxVertexBufferResource::release( resource );
		ErrorReturnMsg( false, "%s: failed to init vertex buffer (%u)", __FUNCTION__, glGetError() );
	}

	PROFILE_GFX( Gfx::stats.gpuMemoryVertexBuffers += resource->size );
	return true;
}


bool CoreGfx::api_vertex_buffer_free( GfxVertexBufferResource *&resource )
{
	OPENGL_CHECK_ERRORS_SCOPE
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	PROFILE_GFX( Gfx::stats.gpuMemoryVertexBuffers -= resource->size );

	GfxVertexBufferResource::release( resource );

	return true;
}


void CoreGfx::api_vertex_buffer_write_begin( GfxVertexBufferResource *resource )
{
	OPENGL_CHECK_ERRORS_SCOPE
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	nglBindBuffer( GL_ARRAY_BUFFER, resource->vbo );
	OPENGL_CHECK_ERROR( "Failed to bind vertex buffer for write begin (resource: %u)", resource->id )

	GLbitfield access = 0;
	switch( resource->writeMode )
	{
		default:
		case GfxWriteMode_ONCE:
			access = GL_MAP_WRITE_BIT;
		break;

		case GfxWriteMode_OVERWRITE:
			access = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT;
		break;

		case GfxWriteMode_RING:
			access = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT; // TODO: Temporary
			//access = GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT;
		break;
	}

	Assert( !resource->mapped );
	resource->data = reinterpret_cast<byte *>( nglMapBufferRange( GL_ARRAY_BUFFER, 0, resource->size, access ) );
	OPENGL_CHECK_ERROR( "Failed to map vertex buffer for write begin (resource: %u)", resource->id )
	resource->mapped = true;

	resource->current = 0;
	resource->offset = 0;

	resource->writeRangeStart = USIZE_MAX;
	resource->writeRangeEnd = 0LLU;
}


void CoreGfx::api_vertex_buffer_write_end( GfxVertexBufferResource *resource )
{
	OPENGL_CHECK_ERRORS_SCOPE
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	nglBindBuffer( GL_ARRAY_BUFFER, resource->vbo );
	OPENGL_CHECK_ERROR( "Failed to bind vertex buffer for write end (resource: %u)", resource->id );

	Assert( resource->mapped );
	nglUnmapBuffer( GL_ARRAY_BUFFER );
	OPENGL_CHECK_ERROR( "Failed to unmap vertex buffer for write end (resource: %u)", resource->id );
	resource->mapped = false;

#if 0 // TODO: Temporary
	if( resource->writeMode == GfxWriteMode_RING )
	{
		if( resource->writeRangeEnd <= resource->writeRangeStart ) { return; }
		const GLintptr offset = static_cast<GLintptr>( resource->writeRangeStart );
		const GLsizeiptr length = static_cast<GLsizeiptr>( resource->writeRangeEnd - resource->writeRangeStart );
		nglFlushMappedBufferRange( GL_ARRAY_BUFFER, offset, length );
	}
#endif
}


void CoreGfx::api_vertex_buffer_write( GfxVertexBufferResource *resource, const void *data, u32 size )
{
	OPENGL_CHECK_ERRORS_SCOPE
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	Assert( resource->mapped );
	Assert( data != nullptr );
	memory_copy( resource->data + resource->current, data, size );
	resource->current += size;
}


u32 CoreGfx::api_vertex_buffer_current( const GfxVertexBufferResource *resource )
{
	OPENGL_CHECK_ERRORS_SCOPE
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	return resource->current;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Instance Buffer

void GfxInstanceBufferResource::release( GfxInstanceBufferResource *&resource )
{
	OPENGL_CHECK_ERRORS_SCOPE
	if( resource == nullptr || resource->id == GFX_RESOURCE_ID_NULL ) { return; }

	if( resource->vao != 0 ) { nglDeleteVertexArrays( 1, &resource->vao ); }
	if( resource->vbo != 0 ) { nglDeleteBuffers( 1, &resource->vbo ); }

	instanceBufferResources.remove( resource->id );
	resource = nullptr;
}


bool CoreGfx::api_instance_buffer_init( GfxInstanceBufferResource *&resource,
	u32 instanceFormatID, GfxWriteMode writeMode, u32 capacity, u32 stride )
{
	OPENGL_CHECK_ERRORS_SCOPE
	Assert( resource == nullptr );
	resource = instanceBufferResources.make_new();
	resource->writeMode = writeMode == GfxWriteMode_RING ? GfxWriteMode_OVERWRITE : writeMode;
	resource->mapped = false;
	resource->instanceFormat = instanceFormatID;
	resource->size = capacity;
	resource->stride = stride;

	nglGenVertexArrays( 1, &resource->vao );
	nglGenBuffers( 1, &resource->vbo );

	nglBindBuffer( GL_ARRAY_BUFFER, resource->vbo );
	nglBufferData( GL_ARRAY_BUFFER, capacity, nullptr, GL_STATIC_DRAW ); // TODO: GL_DYNAMIC_DRAW

	if( OPENGL_ERROR() )
	{
		GfxInstanceBufferResource::release( resource );
		ErrorReturnMsg( false, "%s: failed to init instance buffer (%u)", __FUNCTION__, glGetError() );
	}

	PROFILE_GFX( Gfx::stats.gpuMemoryInstanceBuffers += resource->size );
	return true;
}


bool CoreGfx::api_instance_buffer_free( GfxInstanceBufferResource *&resource )
{
	OPENGL_CHECK_ERRORS_SCOPE
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	PROFILE_GFX( Gfx::stats.gpuMemoryInstanceBuffers -= resource->size );

	GfxInstanceBufferResource::release( resource );

	return true;
}


void CoreGfx::api_instance_buffer_write_begin( GfxInstanceBufferResource *resource )
{
	OPENGL_CHECK_ERRORS_SCOPE
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	nglBindBuffer( GL_ARRAY_BUFFER, resource->vbo );
	OPENGL_CHECK_ERROR( "Failed to bind instance buffer for write begin (resource: %u)", resource->id )

	GLbitfield access = 0;
	switch( resource->writeMode )
	{
		default:
		case GfxWriteMode_ONCE:
			access = GL_MAP_WRITE_BIT;
		break;

		case GfxWriteMode_OVERWRITE:
			access = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT;
		break;

		case GfxWriteMode_RING:
			access = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT; // TODO: Temporary
			//access = GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT;
		break;
	}

	Assert( !resource->mapped );
	resource->data = reinterpret_cast<byte *>( nglMapBufferRange( GL_ARRAY_BUFFER, 0, resource->size, access ) );
	OPENGL_CHECK_ERROR( "Failed to map instance buffer for write begin (resource: %u)", resource->id )
	resource->mapped = true;

	resource->current = 0;
	resource->offset = 0;

	resource->writeRangeStart = USIZE_MAX;
	resource->writeRangeEnd = 0LLU;
}


void CoreGfx::api_instance_buffer_write_end( GfxInstanceBufferResource *resource )
{
	OPENGL_CHECK_ERRORS_SCOPE
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	nglBindBuffer( GL_ARRAY_BUFFER, resource->vbo );
	OPENGL_CHECK_ERROR( "Failed to bind instance buffer for write end (resource: %u)", resource->id )

	Assert( resource->mapped );
	nglUnmapBuffer( GL_ARRAY_BUFFER );
	OPENGL_CHECK_ERROR( "Failed to unmap instance buffer for write end (resource: %u)", resource->id )
	resource->mapped = false;

#if 0 // TODO: Temporary
	if( resource->writeMode == GfxWriteMode_RING )
	{
		if( resource->writeRangeEnd <= resource->writeRangeStart ) { return; }
		const GLintptr offset = static_cast<GLintptr>( resource->writeRangeStart );
		const GLsizeiptr length = static_cast<GLsizeiptr>( resource->writeRangeEnd - resource->writeRangeStart );
		nglFlushMappedBufferRange( GL_ARRAY_BUFFER, offset, length );
	}
#endif
}


void CoreGfx::api_instance_buffer_write( GfxInstanceBufferResource *resource, const void *data, u32 size )
{
	OPENGL_CHECK_ERRORS_SCOPE
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	Assert( resource->mapped );
	Assert( data != nullptr );
	memory_copy( resource->data + resource->current, data, size );
	resource->current += size;
}


u32 CoreGfx::api_instance_buffer_current( const GfxInstanceBufferResource *resource )
{
	OPENGL_CHECK_ERRORS_SCOPE
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	return resource->current;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Index Buffer

void GfxIndexBufferResource::release( GfxIndexBufferResource *&resource )
{
	OPENGL_CHECK_ERRORS_SCOPE
	if( resource == nullptr || resource->id == GFX_RESOURCE_ID_NULL ) { return; }

	if( resource->ebo != 0 ) { nglDeleteBuffers( 1, &resource->ebo ); }

	indexBufferResources.remove( resource->id );
	resource = nullptr;
}


bool CoreGfx::api_index_buffer_init( GfxIndexBufferResource *&resource, void *data, u32 size,
	double indToVertRatio, GfxIndexBufferFormat format, GfxWriteMode writeMode )
{
	OPENGL_CHECK_ERRORS_SCOPE
	Assert( resource == nullptr );
	Assert( format != GfxIndexBufferFormat_NONE );
	Assert( format < GFXINDEXBUFFERFORMAT_COUNT );

	resource = indexBufferResources.make_new();
	resource->writeMode = writeMode;
	resource->format = format;
	resource->indicesToVerticesRatio = indToVertRatio;
	resource->size = size;

	nglGenBuffers( 1, &resource->ebo );
	nglBindBuffer( GL_ELEMENT_ARRAY_BUFFER, resource->ebo );
	nglBufferData( GL_ELEMENT_ARRAY_BUFFER, size, data, GL_STATIC_DRAW );

	if( OPENGL_ERROR() )
	{
		GfxIndexBufferResource::release( resource );
		ErrorReturnMsg( false, "%s: failed to init index buffer (%u)", __FUNCTION__, glGetError() );
	}

	PROFILE_GFX( Gfx::stats.gpuMemoryIndexBuffers += resource->size );
	return true;
}


bool CoreGfx::api_index_buffer_free( GfxIndexBufferResource *&resource )
{
	OPENGL_CHECK_ERRORS_SCOPE
	Assert( resource != nullptr );

	PROFILE_GFX( Gfx::stats.gpuMemoryIndexBuffers -= resource->size );

	GfxIndexBufferResource::release( resource );

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Uniform Buffer

void GfxUniformBufferResource::release( GfxUniformBufferResource *&resource )
{
	OPENGL_CHECK_ERRORS_SCOPE
	if( resource == nullptr || resource->id == GFX_RESOURCE_ID_NULL ) { return; }

	if( resource->ubo != 0 ) { nglDeleteBuffers( 1, &resource->ubo ); }

	uniformBufferResources.remove( resource->id );
	resource = nullptr;
}


bool CoreGfx::api_uniform_buffer_init( GfxUniformBufferResource *&resource, const char *name,
	int index, u32 size )
{
	OPENGL_CHECK_ERRORS_SCOPE
	Assert( resource == nullptr );

	resource = uniformBufferResources.make_new();
	resource->name = name;
	resource->index = index;
	resource->size = size;

	nglGenBuffers( 1, &resource->ubo );
	OPENGL_CHECK_ERROR( "Failed to generate constant buffer (resource: %u)", resource->id );
	if( OPENGL_ERROR() )
	{
		GfxUniformBufferResource::release( resource );
		ErrorReturnMsg( false, "%s: failed to init uniform buffer ubo (%u)", __FUNCTION__, glGetError() );
	}

	nglBindBuffer( GL_UNIFORM_BUFFER, resource->ubo );
	nglBufferData( GL_UNIFORM_BUFFER, size, nullptr, GL_DYNAMIC_DRAW );
	nglBindBuffer( GL_UNIFORM_BUFFER, 0 );
	OPENGL_CHECK_ERROR( "Failed to bind constant buffer for init (resource: %u)", resource->id );

	PROFILE_GFX( Gfx::stats.gpuMemoryUniformBuffers += resource->size );
	return true;
}


bool CoreGfx::api_uniform_buffer_free( GfxUniformBufferResource *&resource )
{
	OPENGL_CHECK_ERRORS_SCOPE
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	PROFILE_GFX( Gfx::stats.gpuMemoryUniformBuffers -= resource->size );

	GfxUniformBufferResource::release( resource );

	return true;
}


void CoreGfx::api_uniform_buffer_write_begin( GfxUniformBufferResource *resource )
{
	OPENGL_CHECK_ERRORS_SCOPE
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	Assert( !resource->mapped );
	resource->mapped = true;
}


void CoreGfx::api_uniform_buffer_write_end( GfxUniformBufferResource *resource )
{
	OPENGL_CHECK_ERRORS_SCOPE
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	Assert( resource->mapped );
	resource->mapped = false;
}


void CoreGfx::api_uniform_buffer_write( GfxUniformBufferResource *resource, const void *data )
{
	OPENGL_CHECK_ERRORS_SCOPE
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	Assert( resource->mapped );
	Assert( data != nullptr );
	nglBindBuffer( GL_UNIFORM_BUFFER, resource->ubo );
	nglBufferSubData( GL_UNIFORM_BUFFER, 0, static_cast<GLsizeiptr>( resource->size ), data );
	nglBindBuffer( GL_UNIFORM_BUFFER, 0 );
}


bool CoreGfx::api_uniform_buffer_bind_vertex( GfxUniformBufferResource *resource, int slot )
{
	OPENGL_CHECK_ERRORS_SCOPE
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( slot >= 0 );

	nglBindBuffer( GL_UNIFORM_BUFFER, resource->ubo );
	nglBindBufferBase( GL_UNIFORM_BUFFER, static_cast<GLuint>( resource->index ), resource->ubo );

	const GLuint blockIndex = opengl_uniform_buffer_uniform_block_index( resource, slot );
	nglUniformBlockBinding( CoreGfx::state.pipeline.shader->program, blockIndex,
		static_cast<GLuint>( resource->index ) );

	return true;
}


bool CoreGfx::api_uniform_buffer_bind_fragment( GfxUniformBufferResource *resource, int slot )
{
	OPENGL_CHECK_ERRORS_SCOPE
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( slot >= 0 );

	nglBindBuffer( GL_UNIFORM_BUFFER, resource->ubo );
	nglBindBufferBase( GL_UNIFORM_BUFFER, static_cast<GLuint>( resource->index ), resource->ubo );

	const GLuint blockIndex = opengl_uniform_buffer_uniform_block_index( resource, slot );
	nglUniformBlockBinding( CoreGfx::state.pipeline.shader->program, blockIndex,
		static_cast<GLuint>( resource->index ) );

	return true;
}


bool CoreGfx::api_uniform_buffer_bind_compute( GfxUniformBufferResource *resource, int slot )
{
	OPENGL_CHECK_ERRORS_SCOPE
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( slot >= 0 );

	nglBindBuffer( GL_UNIFORM_BUFFER, resource->ubo );
	nglBindBufferBase( GL_UNIFORM_BUFFER, static_cast<GLuint>( resource->index ), resource->ubo );

	const GLuint blockIndex = opengl_uniform_buffer_uniform_block_index( resource, slot );
	nglUniformBlockBinding( CoreGfx::state.pipeline.shader->program, blockIndex,
		static_cast<GLuint>( resource->index ) );

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// TODO: GfxConstantBuffer
// TODO: GfxMutableBuffer

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Texture

void GfxTextureResource::release( GfxTextureResource *&resource )
{
	OPENGL_CHECK_ERRORS_SCOPE

	if( resource == nullptr || resource->id == GFX_RESOURCE_ID_NULL ) { return; }
	if( resource->textureSS != 0 ) { glDeleteTextures( 1, &resource->textureSS ); }

	textureResources.remove( resource->id );
	resource = nullptr;
}


bool CoreGfx::api_texture_init( GfxTextureResource *&resource, void *pixels,
	u16 width, u16 height, u16 levels, const GfxColorFormat &format )
{
	OPENGL_CHECK_ERRORS_SCOPE
	Assert( resource == nullptr );

	GLint TEXTURE_CURRENT = 0;
	glGetIntegerv( GL_TEXTURE_BINDING_2D, &TEXTURE_CURRENT );

	resource = textureResources.make_new();
	resource->type = GfxTextureType_2D;
	resource->colorFormat = format;
	resource->width = width;
	resource->height = height;
	resource->depth = 1U;
	resource->levels = levels;
	resource->layers = 1U;

	const usize pixelSizeBytes = CoreGfx::colorFormatPixelSizeBytes[format];
	const GLint glFormatInternal = OpenGLColorFormats[format].formatInternal;
	const GLenum glFormat = OpenGLColorFormats[format].format;
	const GLenum glFormatType = OpenGLColorFormats[format].formatType;

	if( Gfx::mip_level_count_2d( width, height ) < levels )
	{
		GfxTextureResource::release( resource );
		ErrorReturnMsg( false, "%s: requested mip levels is more than texture size supports (attempted: %u)", levels );
	}

	if( levels >= GFX_MIP_DEPTH_MAX )
	{
		GfxTextureResource::release( resource );
		ErrorReturnMsg( false, "%s: exceeded max mip level count (has: %u, max: %u)", levels, GFX_MIP_DEPTH_MAX );
	}

	glGenTextures( 1, &resource->textureSS );
	if( OPENGL_ERROR() )
	{
		GfxTextureResource::release( resource );
		ErrorReturnMsg( false, "%s: failed to init texture (%d)", __FUNCTION__, glGetError() );
	}

	glBindTexture( GL_TEXTURE_2D, resource->textureSS );
	{
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			( levels > 1 ) ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, levels - 1 );

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

			if( OPENGL_ERROR() )
			{
				GfxTextureResource::release( resource );
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
	glBindTexture( GL_TEXTURE_2D, TEXTURE_CURRENT );

	PROFILE_GFX( Gfx::stats.gpuMemoryTextures += resource->size );
	return true;
}


bool CoreGfx::api_texture_free( GfxTextureResource *&resource )
{
	OPENGL_CHECK_ERRORS_SCOPE
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	PROFILE_GFX( Gfx::stats.gpuMemoryTextures -= resource->size );

	GfxTextureResource::release( resource );

	return true;
}


bool CoreGfx::api_texture_bind( GfxTextureResource *resource, int slot )
{
	OPENGL_CHECK_ERRORS_SCOPE
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( slot >= 0 && slot < GFX_TEXTURE_SLOT_COUNT );

	// Bind Texture2D
	const GLint location = opengl_texture_uniform_location( uniformLocationsTexture, slot );
	nglUniform1i( location, slot );
	nglActiveTexture( GL_TEXTURE0 + slot );
	glBindTexture( OpenGLTextureTypes[resource->type], resource->textureSS );

	// OpenGL requires us to explitily set the sampler state on textures binds (TODO: Refactor)
	isFilterModeMipmapping = resource->levels > 1;
	CoreGfx::api_sampler_set_state( CoreGfx::state.sampler );
	isFilterModeMipmapping = false;

	PROFILE_GFX( Gfx::stats.frame.textureBinds++ );
	return true;
}


bool CoreGfx::api_texture_release( GfxTextureResource *resource, int slot )
{
	OPENGL_CHECK_ERRORS_SCOPE
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( slot >= 0 && slot < GFX_TEXTURE_SLOT_COUNT );

	// Do nothing...

	PROFILE_GFX( Gfx::stats.frame.textureBinds-- );
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Render Target

void GfxRenderTargetResource::release( GfxRenderTargetResource *&resource )
{
	OPENGL_CHECK_ERRORS_SCOPE
	if( resource == nullptr || resource->id == GFX_RESOURCE_ID_NULL ) { return; }

	if( resource->fboSS != 0 ) { nglDeleteFramebuffers( 1, &resource->fboSS ); }
	if( resource->fboMS != 0 ) { nglDeleteFramebuffers( 1, &resource->fboMS ); }
	if( resource->pboColor != 0 ) { nglDeleteBuffers( 1, &resource->pboColor ); }
	if( resource->pboDepth != 0 ) { nglDeleteBuffers( 1, &resource->pboDepth ); }

	renderTargetResources.remove( resource->id );
	resource = nullptr;
}


bool CoreGfx::api_render_target_init( GfxRenderTargetResource *&resource,
	GfxTextureResource *&resourceColor, GfxTextureResource *&resourceDepth,
	u16 width, u16 height, u16 layers, const GfxRenderTargetDescription &desc )
{
	OPENGL_CHECK_ERRORS_SCOPE
	Assert( resource == nullptr );
	Assert( resourceColor == nullptr );
	Assert( resourceDepth == nullptr );

	const bool isArrayTexture = layers > 1;
	const GLenum targetSS = isArrayTexture ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D;
	const GLenum targetMS = isArrayTexture ? GL_TEXTURE_2D_MULTISAMPLE_ARRAY : GL_TEXTURE_2D_MULTISAMPLE;

	GLint cacheBoundTexture2D = 0;
	glGetIntegerv( GL_TEXTURE_BINDING_2D, &cacheBoundTexture2D );

	resource = renderTargetResources.make_new();
	resource->width = width;
	resource->height = height;
	resource->layers = layers;
	resource->desc = desc;

	// Color Texture
	if( desc.colorFormat != GfxColorFormat_NONE )
	{
		resourceColor = textureResources.make_new();
		resourceColor->type = layers > 1 ? GfxTexturetype_2D_ARRAY : GfxTextureType_2D;
		resourceColor->layers = layers;
		resource->resourceColor = resourceColor;

		const GLint glFormatInternal = OpenGLColorFormats[desc.colorFormat].formatInternal;
		const GLenum glFormat = OpenGLColorFormats[desc.colorFormat].format;
		const GLenum glFormatType = OpenGLColorFormats[desc.colorFormat].formatType;

		// Texture (Single-Sample)
		glGenTextures( 1, &resourceColor->textureSS );
		if( OPENGL_ERROR() )
		{
			GfxTextureResource::release( resourceColor );
			GfxRenderTargetResource::release( resource );
			ErrorReturnMsg( false, "%s: failed to init color single-sample texture (%d)",
				__FUNCTION__, glGetError() );
		}

		// Sampler (Single-Sample)
		{
			glBindTexture( targetSS, resourceColor->textureSS );
			glTexParameteri( targetSS, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
			glTexParameteri( targetSS, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
			glTexParameteri( targetSS, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
			glTexParameteri( targetSS, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

			resource->size += GFX_SIZE_IMAGE_COLOR_BYTES( width, height, layers, desc.colorFormat );

			if( isArrayTexture )
			{
				nglTexImage3D( targetSS, 0, glFormatInternal,
					width, height, layers, 0, glFormat, glFormatType, nullptr );
			}
			else
			{
				glTexImage2D( targetSS, 0, glFormatInternal,
					width, height, 0, glFormat, glFormatType, nullptr );
			}

			if( OPENGL_ERROR() )
			{
				GfxTextureResource::release( resourceColor );
				GfxRenderTargetResource::release( resource );
				ErrorReturnMsg( false, "%s: failed to upload color single-sample texture data (%d)",
					__FUNCTION__, glGetError() );
			}
		}

		if( desc.sampleCount > 1 )
		{
			// Texture (Multi-Sample)
			glGenTextures( 1, &resourceColor->textureMS );
			if( OPENGL_ERROR() )
			{
				GfxTextureResource::release( resourceColor );
				GfxRenderTargetResource::release( resource );
				ErrorReturnMsg( false, "%s: failed to init color multi-sample texture (%d)",
					__FUNCTION__, glGetError() );
			}

			// Sampler (Multi-Sample)
			{
				glBindTexture( targetMS, resourceColor->textureMS );

				resource->size += GFX_SIZE_IMAGE_COLOR_BYTES( width, height, layers, desc.colorFormat ) *
					desc.sampleCount;

				if( isArrayTexture )
				{
					nglTexImage3DMultisample( targetMS, desc.sampleCount, glFormatInternal,
						width, height, layers, GL_TRUE );
				}
				else
				{
					nglTexImage2DMultisample( targetMS, desc.sampleCount, glFormatInternal,
						width, height, GL_TRUE );
				}

				if( OPENGL_ERROR() )
				{
					GfxTextureResource::release( resourceColor );
					GfxRenderTargetResource::release( resource );
					ErrorReturnMsg( false, "%s: failed to upload color multi-sample texture data (%d)",
						__FUNCTION__, glGetError() );
				}
			}
		}

		// CPU Readback
		if( desc.cpuAccess )
		{
			nglGenBuffers( 1, &resource->pboColor );
			nglBindBuffer( GL_PIXEL_PACK_BUFFER, resource->pboColor );
			{
				resource->size += GFX_SIZE_IMAGE_COLOR_BYTES( width, height, 1, desc.colorFormat );
				nglBufferData( GL_PIXEL_PACK_BUFFER,
					GFX_SIZE_IMAGE_COLOR_BYTES( width, height, 1, desc.colorFormat ), nullptr, GL_STREAM_READ );
			}
			nglBindBuffer( GL_PIXEL_PACK_BUFFER, 0 );

			if( OPENGL_ERROR() )
			{
				GfxTextureResource::release( resourceColor );
				GfxRenderTargetResource::release( resource );
				ErrorReturnMsg( false, "%s: failed to init color readback pbo (%d)",
					__FUNCTION__, glGetError() );
			}
		}
	}

	// Depth Texture
	if( desc.depthFormat != GfxDepthFormat_NONE )
	{
		resourceDepth = textureResources.make_new();
		resourceDepth->type = layers > 1 ? GfxTexturetype_2D_ARRAY : GfxTextureType_2D;
		resourceDepth->layers = layers;
		resource->resourceDepth = resourceDepth;

		const GLint glFormatInternal = OpenGLDepthStencilFormats[desc.depthFormat].formatInternal;
		const GLenum glFormat = OpenGLDepthStencilFormats[desc.depthFormat].format;
		const GLenum glFormatType = OpenGLDepthStencilFormats[desc.depthFormat].formatType;

		// Texture (Single-Sample)
		glGenTextures( 1, &resourceDepth->textureSS );
		if( OPENGL_ERROR() )
		{
			GfxTextureResource::release( resourceColor );
			GfxTextureResource::release( resourceDepth );
			GfxRenderTargetResource::release( resource );
			ErrorReturnMsg( false, "%s: failed to init depth single-sample texture (%d)",
				__FUNCTION__, glGetError() );
		}

		// Sampler (Single-Sample)
		{
			glBindTexture( targetSS, resourceDepth->textureSS );
			glTexParameteri( targetSS, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
			glTexParameteri( targetSS, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
			glTexParameteri( targetSS, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
			glTexParameteri( targetSS, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

			resource->size += GFX_SIZE_IMAGE_DEPTH_BYTES( width, height, layers, desc.depthFormat );

			if( isArrayTexture )
			{
				nglTexImage3D( targetSS, 0, glFormatInternal,
					width, height, layers, 0, glFormat, glFormatType, nullptr );
			}
			else
			{
				glTexImage2D( targetSS, 0, glFormatInternal,
					width, height, 0, glFormat, glFormatType, nullptr );
			}

			if( OPENGL_ERROR() )
			{
				GfxTextureResource::release( resourceColor );
				GfxTextureResource::release( resourceDepth );
				GfxRenderTargetResource::release( resource );
				ErrorReturnMsg( false, "%s: failed to upload depth single-sample texture data (%d)",
					__FUNCTION__, glGetError() );
			}
		}

		if( desc.sampleCount > 1 )
		{
			// Texture (Multi-Sample)
			glGenTextures( 1, &resourceDepth->textureMS );
			if( OPENGL_ERROR() )
			{
				GfxTextureResource::release( resourceColor );
				GfxTextureResource::release( resourceDepth );
				GfxRenderTargetResource::release( resource );
				ErrorReturnMsg( false, "%s: failed to init color multi-sample texture (%d)",
					__FUNCTION__, glGetError() );
			}

			// Sampler (Multi-Sample)
			{
				glBindTexture( targetMS, resourceDepth->textureMS );

				resource->size += GFX_SIZE_IMAGE_DEPTH_BYTES( width, height, layers, desc.depthFormat ) *
					desc.sampleCount;

				if( isArrayTexture )
				{
					nglTexImage3DMultisample( targetMS, desc.sampleCount, glFormatInternal,
						width, height, layers, GL_TRUE );
				}
				else
				{
					nglTexImage2DMultisample( targetMS, desc.sampleCount, glFormatInternal,
						width, height, GL_TRUE );
				}

				if( OPENGL_ERROR() )
				{
					GfxTextureResource::release( resourceColor );
					GfxTextureResource::release( resourceDepth );
					GfxRenderTargetResource::release( resource );
					ErrorReturnMsg( false, "%s: failed to upload depth multi-sample texture data (%d)",
						__FUNCTION__, glGetError() );
				}
			}
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

			if( OPENGL_ERROR() )
			{
				GfxTextureResource::release( resourceColor );
				GfxRenderTargetResource::release( resource );
				ErrorReturnMsg( false, "%s: failed to init depth readback pbo (%d)", __FUNCTION__, glGetError() );
			}
		}
	}

	// FBO (Single-Sample)
	GLint fboPrevious;
	glGetIntegerv( GL_FRAMEBUFFER_BINDING, &fboPrevious );
	nglGenFramebuffers( 1, &resource->fboSS );
	nglBindFramebuffer( GL_FRAMEBUFFER, resource->fboSS );
	{
		// Attach Color
		if( desc.colorFormat != GfxColorFormat_NONE )
		{
			if( isArrayTexture )
			{
				nglFramebufferTextureLayer( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
					resourceColor->textureSS, 0, 0 );
			}
			else
			{
				nglFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
					targetSS, resourceColor->textureSS, 0 );
			}
		}

		// Attach Depth
		if( desc.depthFormat != GfxDepthFormat_NONE )
		{
			if( isArrayTexture )
			{
				nglFramebufferTextureLayer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
					resourceDepth->textureSS, 0, 0 );
			}
			else
			{
				nglFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
					targetSS, resourceDepth->textureSS, 0 );
			}
		}

		// Check Status
		if( nglCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
		{
			GfxTextureResource::release( resourceColor );
			GfxTextureResource::release( resourceDepth );
			GfxRenderTargetResource::release( resource );
			ErrorReturnMsg( false, "%s: failed to create framebuffer", __FUNCTION__ );
		}
	}
	nglBindFramebuffer( GL_FRAMEBUFFER, fboPrevious );

	// FBO (Multi-Sample)
	if( desc.sampleCount > 1 )
	{
		glGetIntegerv( GL_FRAMEBUFFER_BINDING, &fboPrevious );
		nglGenFramebuffers( 1, &resource->fboMS );
		nglBindFramebuffer( GL_FRAMEBUFFER, resource->fboMS );
		{
			// Attach Color
			if( desc.colorFormat != GfxColorFormat_NONE )
			{
				if( isArrayTexture )
				{
					nglFramebufferTextureLayer( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
						resourceColor->textureMS, 0, 0 );
				}
				else
				{
					nglFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
						targetMS, resourceColor->textureMS, 0 );
				}
			}

			// Attach Depth
			if( desc.depthFormat != GfxDepthFormat_NONE )
			{
				if( isArrayTexture )
				{
					nglFramebufferTextureLayer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
						resourceDepth->textureMS, 0, 0 );
				}
				else
				{
					nglFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
						targetMS, resourceDepth->textureMS, 0 );
				}
			}

			// Check Status
			if( nglCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
			{
				GfxTextureResource::release( resourceColor );
				GfxTextureResource::release( resourceDepth );
				GfxRenderTargetResource::release( resource );
				ErrorReturnMsg( false, "%s: failed to create framebuffer", __FUNCTION__ );
			}
		}
		nglBindFramebuffer( GL_FRAMEBUFFER, fboPrevious );
	}

	glBindTexture( GL_TEXTURE_2D, cacheBoundTexture2D );
	PROFILE_GFX( Gfx::stats.gpuMemoryRenderTargets += resource->size );
	return true;
}


bool CoreGfx::api_render_target_free( GfxRenderTargetResource *&resource,
	GfxTextureResource *&resourceColor,GfxTextureResource *&resourceDepth )
{
	OPENGL_CHECK_ERRORS_SCOPE
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	PROFILE_GFX( Gfx::stats.gpuMemoryRenderTargets -= resource->size );

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

	GfxRenderTargetResource::release( resource );

	return true;
}


bool CoreGfx::api_render_target_copy_color(
	GfxRenderTargetResource *&srcResource, GfxTextureResource *&srcResourceDepth,
	GfxRenderTargetResource *&dstResource, GfxTextureResource *&dstResourceDepth )
{
	OPENGL_CHECK_ERRORS_SCOPE
	if( srcResource == nullptr || dstResource == nullptr ) { return false; }

	// Ensure equal dimensions
	if( srcResource->width != dstResource->width ||
		srcResource->height != dstResource->height )
	{
		return false;
	}

	GLbitfield flags = 0;

	if( srcResource->desc.colorFormat != GfxColorFormat_NONE &&
		dstResource->desc.colorFormat != GfxColorFormat_NONE )
	{
		flags |= GL_COLOR_BUFFER_BIT;
	}

	if( flags )
	{
		GLint fboReadPrevious, fboDrawPrevious;
		glGetIntegerv( GL_READ_FRAMEBUFFER_BINDING, &fboReadPrevious );
		glGetIntegerv( GL_DRAW_FRAMEBUFFER_BINDING, &fboDrawPrevious );
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
		nglBindFramebuffer( GL_READ_FRAMEBUFFER, fboReadPrevious );
		nglBindFramebuffer( GL_DRAW_FRAMEBUFFER, fboDrawPrevious );
	}

	return true;
}


bool CoreGfx::api_render_target_copy_depth(
	GfxRenderTargetResource *&srcResource, GfxTextureResource *&srcResourceColor,
	GfxRenderTargetResource *&dstResource, GfxTextureResource *&dstResourceColor )
{
	OPENGL_CHECK_ERRORS_SCOPE
	if( srcResource == nullptr || dstResource == nullptr ) { return false; }

	// Ensure equal dimensions
	if( srcResource->width != dstResource->width ||
		srcResource->height != dstResource->height )
	{
		return false;
	}

	GLbitfield flags = 0;

	if( srcResource->desc.depthFormat != GfxDepthFormat_NONE &&
		dstResource->desc.depthFormat != GfxDepthFormat_NONE )
	{
		flags |= GL_DEPTH_BUFFER_BIT;
	}

	if( flags )
	{
		GLint fboReadPrevious, fboDrawPrevious;
		glGetIntegerv( GL_READ_FRAMEBUFFER_BINDING, &fboReadPrevious );
		glGetIntegerv( GL_DRAW_FRAMEBUFFER_BINDING, &fboDrawPrevious );
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
		nglBindFramebuffer( GL_READ_FRAMEBUFFER, fboReadPrevious );
		nglBindFramebuffer( GL_DRAW_FRAMEBUFFER, fboDrawPrevious );
	}

	return true;
}


bool CoreGfx::api_render_target_copy(
	GfxRenderTargetResource *&srcResource,
	GfxTextureResource *&srcResourceColor, GfxTextureResource *&srcResourceDepth,
	GfxRenderTargetResource *&dstResource,
	GfxTextureResource *&dstResourceColor, GfxTextureResource *&dstResourceDepth )
{
	OPENGL_CHECK_ERRORS_SCOPE
	if( srcResource == nullptr || dstResource == nullptr ) { return false; }

	// Ensure equal dimensions
	if( srcResource->width != dstResource->width ||
		srcResource->height != dstResource->height )
	{
		return false;
	}

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
		GLint fboReadPrevious, fboDrawPrevious;
		glGetIntegerv( GL_READ_FRAMEBUFFER_BINDING, &fboReadPrevious );
		glGetIntegerv( GL_DRAW_FRAMEBUFFER_BINDING, &fboDrawPrevious );
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
		nglBindFramebuffer( GL_READ_FRAMEBUFFER, fboReadPrevious );
		nglBindFramebuffer( GL_DRAW_FRAMEBUFFER, fboDrawPrevious );
	}

	return true;
}


bool CoreGfx::api_render_target_copy_part(
	GfxRenderTargetResource *&srcResource,
	GfxTextureResource *&srcResourceColor, GfxTextureResource *&srcResourceDepth,
	GfxRenderTargetResource *&dstResource,
	GfxTextureResource *&dstResourceColor, GfxTextureResource *&dstResourceDepth,
	u16 srcX, u16 srcY, u16 dstX, u16 dstY, u16 width, u16 height )
{
	OPENGL_CHECK_ERRORS_SCOPE
	if( srcResource == nullptr || dstResource == nullptr ) { return false; }

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
		GLint fboReadPrevious, fboDrawPrevious;
		glGetIntegerv( GL_READ_FRAMEBUFFER_BINDING, &fboReadPrevious );
		glGetIntegerv( GL_DRAW_FRAMEBUFFER_BINDING, &fboDrawPrevious );
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
		nglBindFramebuffer( GL_READ_FRAMEBUFFER, fboReadPrevious );
		nglBindFramebuffer( GL_DRAW_FRAMEBUFFER, fboDrawPrevious );
	}

	return true;
}


bool CoreGfx::api_render_target_buffer_read_color( GfxRenderTargetResource *&resource,
	GfxTextureResource *&resourceColor,
	void *buffer, u32 size )
{
	OPENGL_CHECK_ERRORS_SCOPE
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( resourceColor != nullptr && resourceColor->id != GFX_RESOURCE_ID_NULL );
	Assert( resource->pboColor != 0 );

	ErrorIf( !resource->desc.cpuAccess,
		"Trying to CPU access a render target that does not have CPU access flag!" );

	const u32 sizeSource = GFX_SIZE_IMAGE_COLOR_BYTES(
		resource->width, resource->height, 1, resource->desc.colorFormat );

	Assert( sizeSource <= size );
	Assert( buffer != nullptr );

	GLint fboReadPrevious;
	glGetIntegerv( GL_READ_FRAMEBUFFER_BINDING, &fboReadPrevious );
	nglBindFramebuffer( GL_READ_FRAMEBUFFER, resource->fboSS );
	GLint bufferReadPrevious;
	glGetIntegerv( GL_READ_BUFFER, &bufferReadPrevious );
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

	nglUnmapBuffer( GL_PIXEL_PACK_BUFFER );
	nglBindBuffer( GL_PIXEL_PACK_BUFFER, 0 );
	nglBindFramebuffer( GL_READ_FRAMEBUFFER, fboReadPrevious );
	glReadBuffer( bufferReadPrevious );

	return true;
}


bool CoreGfx::api_render_target_buffer_read_color_async_request( GfxRenderTargetResource *&resource,
	GfxTextureResource *&resourceColor,
	void *buffer, u32 size )
{
	OPENGL_CHECK_ERRORS_SCOPE

	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( resourceColor != nullptr && resourceColor->id != GFX_RESOURCE_ID_NULL );
	Assert( resource->pboColor != 0 );

	ErrorIf( !resource->desc.cpuAccess,
		"Trying to CPU access a render target that does not have CPU access flag!" );

	// Bind read framebuffer
	GLint fboReadPrevious;
	glGetIntegerv( GL_READ_FRAMEBUFFER_BINDING, &fboReadPrevious );
	nglBindFramebuffer( GL_READ_FRAMEBUFFER, resource->fboSS );
	GLint bufferReadPrevious;
	glGetIntegerv( GL_READ_BUFFER, &bufferReadPrevious );
	glReadBuffer( GL_COLOR_ATTACHMENT0 );

	// Bind PBO
	nglBindBuffer( GL_PIXEL_PACK_BUFFER, resource->pboColor );
	const GLenum glFormat = OpenGLColorFormats[resource->desc.colorFormat].format;
	const GLenum glFormatType = OpenGLColorFormats[resource->desc.colorFormat].formatType;

	// Launch async DMA transfer to PBO
	glReadPixels( 0, 0, resource->width, resource->height, glFormat, glFormatType, nullptr );

	// Insert a fence for completion
	if( resource->syncObj ) { nglDeleteSync( resource->syncObj ); }
	resource->syncObj = nglFenceSync( GL_SYNC_GPU_COMMANDS_COMPLETE, 0 );

	nglBindBuffer( GL_PIXEL_PACK_BUFFER, 0 );
	nglBindFramebuffer( GL_READ_FRAMEBUFFER, fboReadPrevious );
	glReadBuffer( bufferReadPrevious );

	Assert( !resource->asyncReadback );
	resource->asyncReadback = true;
	return true;
}


bool CoreGfx::api_render_target_buffer_read_color_async_poll( GfxRenderTargetResource *&resource,
	GfxTextureResource *&resourceColor,
	void *buffer, u32 size )
{
	OPENGL_CHECK_ERRORS_SCOPE

	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( resourceColor != nullptr && resourceColor->id != GFX_RESOURCE_ID_NULL );
	Assert( resource->pboColor != 0 );

	ErrorIf( !resource->desc.cpuAccess,
		"Trying to CPU access a render target that does not have CPU access flag!" );

	const u32 sizeSource = GFX_SIZE_IMAGE_COLOR_BYTES(
		resource->width, resource->height, 1, resource->desc.colorFormat );

	Assert( sizeSource <= size );
	Assert( buffer != nullptr );

	GLint fboReadPrevious;
	glGetIntegerv( GL_READ_FRAMEBUFFER_BINDING, &fboReadPrevious );
	nglBindFramebuffer( GL_READ_FRAMEBUFFER, resource->fboSS );
	GLint bufferReadPrevious;
	glGetIntegerv( GL_READ_BUFFER, &bufferReadPrevious );
	glReadBuffer( GL_COLOR_ATTACHMENT0 );

	Assert( resource->asyncReadback );
	GLenum r = nglClientWaitSync( resource->syncObj, 0, 0 );
	if( r == GL_TIMEOUT_EXPIRED || r == GL_WAIT_FAILED ) { return false; }

	nglBindBuffer( GL_PIXEL_PACK_BUFFER, resource->pboColor);
	void *data = nglMapBuffer( GL_PIXEL_PACK_BUFFER, GL_READ_ONLY );
	if( !data )
	{
		nglBindBuffer( GL_PIXEL_PACK_BUFFER, 0 );
		return false;
	}

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

	nglUnmapBuffer( GL_PIXEL_PACK_BUFFER );
	nglBindBuffer( GL_PIXEL_PACK_BUFFER, 0 );
	nglBindFramebuffer( GL_READ_FRAMEBUFFER, fboReadPrevious );
	glReadBuffer( bufferReadPrevious );

	nglDeleteSync( resource->syncObj );
	resource->syncObj = nullptr;
	resource->asyncReadback = false;
	return true;
}


bool api_render_target_buffer_read_depth( GfxRenderTargetResource *&resource,
	GfxTextureResource *&resourceDepth,
	void *buffer, const u32 size )
{
	OPENGL_CHECK_ERRORS_SCOPE
	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this
	return true;
}


bool CoreGfx::api_render_target_buffer_read_depth_async_request( GfxRenderTargetResource *&resource,
	GfxTextureResource *&resourceDepth,
	void *buffer, u32 size )
{
	OPENGL_CHECK_ERRORS_SCOPE
	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this
	return true;
}


bool CoreGfx::api_render_target_buffer_read_depth_async_poll( GfxRenderTargetResource *&resource,
	GfxTextureResource *&resourceDepth,
	void *buffer, u32 size )
{
	OPENGL_CHECK_ERRORS_SCOPE
	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this
	return true;
}


bool CoreGfx::api_render_target_buffer_write_color( GfxRenderTargetResource *&resource,
	GfxTextureResource *&resourceColor,
	const void *buffer, u32 size )
{
	OPENGL_CHECK_ERRORS_SCOPE
	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this
	return true;
}


bool CoreGfx::api_render_target_buffer_write_depth( GfxRenderTargetResource *&resource,
	GfxTextureResource *&resourceDepth,
	const void *buffer, u32 size )
{
	OPENGL_CHECK_ERRORS_SCOPE
	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Draw

static void opengl_draw( GLsizei vertexCount, GLuint startVertexLocation, GfxPrimitiveType type )
{
	OPENGL_CHECK_ERRORS_SCOPE
	Assert( type < GFXPRIMITIVETYPE_COUNT );

	glDrawArrays( OpenGLPrimitiveTypes[type], startVertexLocation, vertexCount );

	PROFILE_GFX( Gfx::stats.frame.drawCalls++ );
	PROFILE_GFX( Gfx::stats.frame.vertexCount += vertexCount );
}


static void opengl_draw_instanced( GLsizei vertexCount, GLuint startVertexLocation,
	GLsizei instanceCount, GfxPrimitiveType type )
{
	OPENGL_CHECK_ERRORS_SCOPE
	Assert( type < GFXPRIMITIVETYPE_COUNT );

	nglDrawArraysInstanced( OpenGLPrimitiveTypes[type], startVertexLocation, vertexCount, instanceCount );

	PROFILE_GFX( Gfx::stats.frame.drawCalls++ );
	PROFILE_GFX( Gfx::stats.frame.vertexCount += vertexCount * instanceCount );
}


static void opengl_draw_indexed( GLsizei vertexCount, GLuint startVertexLocation,
	GLuint baseVertexLocation, GfxIndexBufferFormat format, GfxPrimitiveType type )
{
	OPENGL_CHECK_ERRORS_SCOPE
	Assert( type < GFXPRIMITIVETYPE_COUNT );

	glDrawElements( OpenGLPrimitiveTypes[type], vertexCount, OpenGLIndexBufferFormats[format], 0 );

	PROFILE_GFX( Gfx::stats.frame.drawCalls++ );
	PROFILE_GFX( Gfx::stats.frame.vertexCount += vertexCount );
}


static void opengl_draw_instanced_indexed( GLsizei vertexCount, GLuint startVertexLocation,
	GLsizei instanceCount, GLuint baseVertexLocation,
	GfxIndexBufferFormat format, GfxPrimitiveType type )
{
	OPENGL_CHECK_ERRORS_SCOPE
	Assert( type < GFXPRIMITIVETYPE_COUNT );

	nglDrawElementsInstanced( OpenGLPrimitiveTypes[type], vertexCount,
		OpenGLIndexBufferFormats[format], 0, instanceCount );

	PROFILE_GFX( Gfx::stats.frame.drawCalls++ );
	PROFILE_GFX( Gfx::stats.frame.vertexCount += vertexCount * instanceCount );
}


bool CoreGfx::api_draw(
	const GfxVertexBufferResource *resourceVertex, u32 vertexCount,
	const GfxInstanceBufferResource *resourceInstance, u32 instanceCount,
	const GfxIndexBufferResource *resourceIndex,
	GfxPrimitiveType type )
{
	OPENGL_CHECK_ERRORS_SCOPE
	Assert( resourceVertex == nullptr || resourceVertex->id != GFX_RESOURCE_ID_NULL );
	Assert( resourceInstance == nullptr || resourceInstance->id != GFX_RESOURCE_ID_NULL );
	Assert( resourceIndex == nullptr || resourceIndex->id != GFX_RESOURCE_ID_NULL );

	const GfxShaderResource *const shader = CoreGfx::state.pipeline.shader;

	AssertMsg( resourceVertex == nullptr ||
		CoreGfx::shaderEntries[shader->shaderID].vertexFormat == resourceVertex->vertexFormat,
		"Attempting to draw a vertex buffer with a shader of a different vertex format!" );
	AssertMsg( resourceInstance == nullptr ||
		CoreGfx::shaderEntries[shader->shaderID].instanceFormat == resourceInstance->instanceFormat,
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
	else
	{
		nglBindVertexArray( drawCallVAOEmpty );
	}

	// Bind Vertex Buffer
	GLuint inputLayoutElementsCount = 0U;

	if( resourceVertex != nullptr && resourceVertex->vertexFormat != U32_MAX )
	{
		nglBindBuffer( GL_ARRAY_BUFFER, resourceVertex->vbo );

		Assert( resourceVertex->vertexFormat < inputLayoutFormatsVertexCount );
		const OpenGLInputLayoutFormats &table = inputLayoutFormatsVertex[resourceVertex->vertexFormat];
		for( GLuint i = 0; i < table.attributesCount; i++ )
		{
			const OpenGLInputLayoutAttributes &attributes = table.attributes[i];

			switch( attributes.type )
			{
				case OpenGLInputAttributeType_FLOAT:
					nglVertexAttribPointer( inputLayoutElementsCount, attributes.components, attributes.format,
						attributes.normalized, table.stepStride, reinterpret_cast<void *>( attributes.offset ) );
				break;

				case OpenGLInputAttributeType_DOUBLE:
					nglVertexAttribLPointer( inputLayoutElementsCount, attributes.components, attributes.format,
						table.stepStride, reinterpret_cast<void *>( attributes.offset ) );
				break;

				case OpenGLInputAttributeType_INTEGER:
					nglVertexAttribIPointer( inputLayoutElementsCount, attributes.components, attributes.format,
						table.stepStride, reinterpret_cast<void *>( attributes.offset ) );
				break;
			}

			nglEnableVertexAttribArray( inputLayoutElementsCount );
			nglVertexAttribDivisor( inputLayoutElementsCount, 0 );
			inputLayoutElementsCount++;
		}
	}

	if( resourceInstance != nullptr && resourceInstance->instanceFormat != U32_MAX )
	{
		nglBindBuffer( GL_ARRAY_BUFFER, resourceInstance->vbo );

		Assert( resourceInstance->instanceFormat < inputLayoutFormatsInstanceCount );
		const OpenGLInputLayoutFormats &table = inputLayoutFormatsInstance[resourceInstance->instanceFormat];
		for( GLuint i = 0; i < table.attributesCount; i++ )
		{
			const OpenGLInputLayoutAttributes &attributes = table.attributes[i];

			switch( attributes.type )
			{
				case OpenGLInputAttributeType_FLOAT:
					nglVertexAttribPointer( inputLayoutElementsCount, attributes.components, attributes.format,
						attributes.normalized, table.stepStride, reinterpret_cast<void *>( attributes.offset ) );
				break;

				case OpenGLInputAttributeType_DOUBLE:
					nglVertexAttribLPointer( inputLayoutElementsCount, attributes.components, attributes.format,
						table.stepStride, reinterpret_cast<void *>( attributes.offset ) );
				break;

				case OpenGLInputAttributeType_INTEGER:
					nglVertexAttribIPointer( inputLayoutElementsCount, attributes.components, attributes.format,
						table.stepStride, reinterpret_cast<void *>( attributes.offset ) );
				break;
			}

			nglEnableVertexAttribArray( inputLayoutElementsCount );
			nglVertexAttribDivisor( inputLayoutElementsCount, 1 );
			inputLayoutElementsCount++;
		}
	}

	// Bind Index Buffer
	if( resourceIndex != nullptr )
	{
		nglBindBuffer( GL_ELEMENT_ARRAY_BUFFER, resourceIndex->ebo );
		OPENGL_CHECK_ERROR( "Failed to bind index buffer for indexed draw (resource: %u)", resourceVertex->id )
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
// Dispatch

bool CoreGfx::api_dispatch( u32 x, u32 y, u32 z )
{
#if !OS_MACOS
	OPENGL_CHECK_ERRORS_SCOPE
	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this
	return true;
#else
	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Compute not supported on MacOS!
	return true;
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Render Command

void CoreGfx::api_render_command_execute( const GfxRenderCommand &command )
{
	OPENGL_CHECK_ERRORS_SCOPE

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


void CoreGfx::api_render_command_execute_post( const GfxRenderCommand &command )
{
}


void CoreGfx::api_render_pass_begin( const GfxRenderPass &pass )
{
	OPENGL_CHECK_ERRORS_SCOPE

	bind_targets( pass.targets, pass.layers );
}


void CoreGfx::api_render_pass_end( const GfxRenderPass &pass )
{
	OPENGL_CHECK_ERRORS_SCOPE

	// NOTE: Do nothing here, since subsequent api_render_pass_begin() will rebind targets
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////