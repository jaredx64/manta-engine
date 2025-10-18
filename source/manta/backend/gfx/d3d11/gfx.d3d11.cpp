#include <manta/gfx.hpp>
#include <gfx.api.generated.hpp>

#include <vendor/d3d11.hpp>
#include <vendor/d3dcompiler.hpp>

#include <config.hpp>

#include <core/buffer.hpp>
#include <core/memory.hpp>
#include <core/hashmap.hpp>
#include <core/math.hpp>

#include <manta/window.hpp>

#include <manta/backend/gfx/gfxfactory.hpp>
#include <manta/backend/window/windows/window.windows.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define GRAPHICS_API_NAME "D3D11"

#define COMPUTE_SHADERS 0 // TODO: Temporary

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if 1
	#define DECL_ZERO(type, name) type name; memory_set( &name, 0, sizeof( type ) )
#else
	#define DECL_ZERO(type, name) type name
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static const GUID IID_IDXGIFactory
	{ 0x7B7166EC, 0x21C7, 0x44AE, { 0xB2, 0x1A, 0xC9, 0xAE, 0x32, 0x1A, 0xE3, 0x69 } };

static const GUID IID_ID3D11Texture2D
	{ 0x6F15AAF2, 0xD208, 0x4E89, { 0x9A, 0xB4, 0x48, 0x95, 0x35, 0xD3, 0x4F, 0x9C } };

static const GUID IID_ID3DUserDefinedAnnotation
	{ 0xB2DAAD8B, 0x03D4, 0x4DBF, { 0x95, 0xEB, 0x32, 0xAB, 0x4B, 0x63, 0xD0, 0xAB } };

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// D3D11 Backend State

static ID3D11Device *device = nullptr;
static ID3D11DeviceContext *context = nullptr;
static IDXGIFactory *factory = nullptr;
static ID3DUserDefinedAnnotation *annotation = nullptr;

static IDXGISwapChain *swapchain = nullptr;
static UINT swapchainFlagsCreate;
static UINT swapchainFlagsPresent;
static ID3D11RenderTargetView *swapchainViewColor = nullptr;
static ID3D11DepthStencilView *swapchainViewDepth = nullptr;

static ID3D11RenderTargetView *stateBoundViewColor[GFX_RENDER_TARGET_SLOT_COUNT];
static ID3D11DepthStencilView *stateBoundViewDepth = nullptr;
static GfxRenderTargetResource *stateBoundTargetResources[GFX_RENDER_TARGET_SLOT_COUNT] = { nullptr };

static bool isDefaultTargetActive = true;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// D3D11 Enums

static const DXGI_FORMAT D3D11ColorFormats[] =
{
	DXGI_FORMAT_UNKNOWN,            // GfxColorFormat_NONE
	DXGI_FORMAT_R8G8B8A8_UNORM,     // GfxColorFormat_R8G8B8A8_FLOAT
	DXGI_FORMAT_R8G8B8A8_UINT,      // GfxColorFormat_R8G8B8A8_UINT
	DXGI_FORMAT_R10G10B10A2_UNORM,  // GfxColorFormat_R10G10B10A2_FLOAT
	DXGI_FORMAT_R8_UNORM,           // GfxColorFormat_R8
	DXGI_FORMAT_R8G8_UNORM,         // GfxColorFormat_R8G8
	DXGI_FORMAT_R16_UNORM,          // GfxColorFormat_R16
	DXGI_FORMAT_R16_FLOAT,          // GfxColorFormat_R16_FLOAT
	DXGI_FORMAT_R16G16_UNORM,       // GfxColorFormat_R16G16
	DXGI_FORMAT_R16G16_FLOAT,       // GfxColorFormat_R16G16F_FLOAT
	DXGI_FORMAT_R16G16B16A16_FLOAT, // GfxColorFormat_R16G16B16A16_FLOAT
	DXGI_FORMAT_R16G16B16A16_UINT,  // GfxColorFormat_R16G16B16A16_UINT
	DXGI_FORMAT_R32_FLOAT,          // GfxColorFormat_R32_FLOAT
	DXGI_FORMAT_R32G32_FLOAT,       // GfxColorFormat_R32G32_FLOAT
	DXGI_FORMAT_R32G32B32A32_FLOAT, // GfxColorFormat_R32G32B32A32_FLOAT
	DXGI_FORMAT_R32G32B32A32_UINT,  // GfxColorFormat_R32G32B32A32_UINT
};
static_assert( ARRAY_LENGTH( D3D11ColorFormats ) == GFXCOLORFORMAT_COUNT,
	"Missing GfxColorFormat!" );


struct D3D11DepthStencilFormat { DXGI_FORMAT texFormat, dsvFormat, srvFormat, uavFormat; };
static const D3D11DepthStencilFormat D3D11DepthStencilFormats[] =
{
	// GfxDepthFormat_NONE
	{ DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN },
	// GfxDepthFormat_R16_FLOAT
	{ DXGI_FORMAT_R16_TYPELESS, DXGI_FORMAT_D16_UNORM, DXGI_FORMAT_R16_UNORM, DXGI_FORMAT_R16_FLOAT },
	// GfxDepthFormat_R16_UINT
	{ DXGI_FORMAT_R16_TYPELESS, DXGI_FORMAT_D16_UNORM, DXGI_FORMAT_R16_UNORM, DXGI_FORMAT_R16_UINT },
	// GfxDepthFormat_R32_FLOAT
	{ DXGI_FORMAT_R32_TYPELESS, DXGI_FORMAT_D32_FLOAT, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R32_FLOAT },
	// GfxDepthFormat_R32_UINT
	{ DXGI_FORMAT_R32_TYPELESS, DXGI_FORMAT_D32_FLOAT, DXGI_FORMAT_R32_UINT, DXGI_FORMAT_R32_UINT },
	// GfxDepthFormat_R24_UINT_G8_UINT
	{ DXGI_FORMAT_R24G8_TYPELESS, DXGI_FORMAT_D24_UNORM_S8_UINT,
	  DXGI_FORMAT_R24_UNORM_X8_TYPELESS, DXGI_FORMAT_R24_UNORM_X8_TYPELESS },
};
static_assert( ARRAY_LENGTH( D3D11DepthStencilFormats ) == GFXDEPTHFORMAT_COUNT,
	"Missing GfxDepthFormat!" );


static const D3D11_PRIMITIVE_TOPOLOGY D3D11PrimitiveTypes[] =
{
	D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,  // GfxPrimitiveType_TriangleList
	D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, // GfxPrimitiveType_TriangleStrip
	D3D11_PRIMITIVE_TOPOLOGY_LINELIST,      // GfxPrimitiveType_LineList
	D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP,     // GfxPrimitiveType_LineStrip
	D3D11_PRIMITIVE_TOPOLOGY_POINTLIST,     // GfxPrimitiveType_PointList
};
static_assert( ARRAY_LENGTH( D3D11PrimitiveTypes ) == GFXPRIMITIVETYPE_COUNT,
	"Missing GfxPrimitiveType!" );


static const D3D11_FILL_MODE D3D11FillModes[] =
{
	D3D11_FILL_SOLID,     // GfxRasterFillMode_SOLID
	D3D11_FILL_WIREFRAME, // GfxRasterFillMode_WIREFRAME
};
static_assert( ARRAY_LENGTH( D3D11FillModes ) == GFXRASTERFILLMODE_COUNT,
	"Missing GfxRasterFillMode!" );


static const D3D11_CULL_MODE D3D11CullModes[] =
{
	D3D11_CULL_NONE,  // GfxRasterCullMode_NONE
	D3D11_CULL_FRONT, // GfxRasterCullMode_FRONT
	D3D11_CULL_BACK,  // GfxRasterCullMode_BACK
};
static_assert( ARRAY_LENGTH( D3D11CullModes ) == GFXRASTERCULLMODE_COUNT,
	"Missing GfxRasterCullMode!" );


static const D3D11_FILTER D3D11FilteringModes[] =
{
	D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR, // GfxSamplerFilteringMode_NEAREST
	D3D11_FILTER_MIN_MAG_MIP_LINEAR,       // GfxSamplerFilteringMode_LINEAR
	D3D11_FILTER_ANISOTROPIC,              // GfxSamplerFilteringMode_ANISOTROPIC
};
static_assert( ARRAY_LENGTH( D3D11FilteringModes ) == GFXSAMPLERFILTERINGMODE_COUNT,
	"Missing GfxSamplerFilteringMode!" );


static const D3D11_TEXTURE_ADDRESS_MODE D3D11UVWrapModes[] =
{
	D3D11_TEXTURE_ADDRESS_WRAP,   // GfxSamplerWrapMode_WRAP
	D3D11_TEXTURE_ADDRESS_MIRROR, // GfxSamplerWrapMode_MIRROR
	D3D11_TEXTURE_ADDRESS_CLAMP,  // GfxSamplerWrapMode_CLAMP
};
static_assert( ARRAY_LENGTH( D3D11UVWrapModes ) == GFXSAMPLERWRAPMODE_COUNT,
	"Missing GfxSamplerWrapMode!" );


static const D3D11_BLEND D3D11BlendFactors[] =
{
	D3D11_BLEND_ZERO,           // GfxBlendFactor_ZERO
	D3D11_BLEND_ONE,            // GfxBlendFactor_ONE
	D3D11_BLEND_SRC_COLOR,      // GfxBlendFactor_SRC_COLOR
	D3D11_BLEND_INV_SRC_COLOR,  // GfxBlendFactor_INV_SRC_COLOR
	D3D11_BLEND_SRC_ALPHA,      // GfxBlendFactor_SRC_ALPHA
	D3D11_BLEND_INV_SRC_ALPHA,  // GfxBlendFactor_INV_SRC_ALPHA
	D3D11_BLEND_DEST_ALPHA,     // GfxBlendFactor_DEST_ALPHA
	D3D11_BLEND_INV_DEST_ALPHA, // GfxBlendFactor_INV_DEST_ALPHA
	D3D11_BLEND_DEST_COLOR,     // GfxBlendFactor_DEST_COLOR
	D3D11_BLEND_INV_DEST_COLOR, // GfxBlendFactor_INV_DEST_COLOR
	D3D11_BLEND_SRC_ALPHA_SAT,  // GfxBlendFactor_SRC_ALPHA_SAT
	D3D11_BLEND_SRC1_COLOR,     // GfxBlendFactor_SRC1_COLOR
	D3D11_BLEND_INV_SRC1_COLOR, // GfxBlendFactor_INV_SRC1_COLOR
	D3D11_BLEND_SRC1_ALPHA,     // GfxBlendFactor_SRC1_ALPHA
	D3D11_BLEND_INV_SRC1_ALPHA, // GfxBlendFactor_INV_SRC1_ALPHA
};
static_assert( ARRAY_LENGTH( D3D11BlendFactors ) == GFXBLENDFACTOR_COUNT,
	"Missing GfxBlendFactor!" );


static const D3D11_BLEND_OP D3D11BlendOperations[] =
{
	D3D11_BLEND_OP_ADD,          // GfxBlendOperation_ADD
	D3D11_BLEND_OP_SUBTRACT,     // GfxBlendOperation_SUBTRACT
	D3D11_BLEND_OP_MIN,          // GfxBlendOperation_MIN
	D3D11_BLEND_OP_MAX,          // GfxBlendOperation_MAX
};
static_assert( ARRAY_LENGTH( D3D11BlendOperations ) == GFXBLENDOPERATION_COUNT,
	"Missing GfxBlendOperation!" );


static const D3D11_DEPTH_WRITE_MASK D3D11DepthWriteMasks[] =
{
	D3D11_DEPTH_WRITE_MASK_ZERO, // GfxDepthWrite_NONE
	D3D11_DEPTH_WRITE_MASK_ALL,  // GfxDepthWrite_ALL
};
static_assert( ARRAY_LENGTH( D3D11DepthWriteMasks ) == GFXDEPTHWRITE_COUNT,
	"Missing GfxDepthWrite!" );


static const D3D11_COMPARISON_FUNC D3D11DepthFunctions[] =
{
	D3D11_COMPARISON_ALWAYS,        // GfxDepthFunction_NONE
	D3D11_COMPARISON_LESS,          // GfxDepthFunction_LESS
	D3D11_COMPARISON_LESS_EQUAL,    // GfxDepthFunction_LESS_EQUALS
	D3D11_COMPARISON_GREATER,       // GfxDepthFunction_GREATER
	D3D11_COMPARISON_GREATER_EQUAL, // GfxDepthFunction_GREATER_EQUALS
	D3D11_COMPARISON_EQUAL,         // GfxDepthFunction_EQUAL
	D3D11_COMPARISON_NOT_EQUAL,     // GfxDepthFunction_NOT_EQUAL
	D3D11_COMPARISON_ALWAYS,        // GfxDepthFunction_ALWAYS
};
static_assert( ARRAY_LENGTH( D3D11DepthFunctions ) == GFXDEPTHFUNCTION_COUNT,
	"Missing GfxDepthFunction!" );


static const DXGI_FORMAT D3D11IndexBufferFormats[] =
{
	DXGI_FORMAT_UNKNOWN,  // GfxIndexBufferFormat_NONE
	DXGI_FORMAT_R8_UINT,  // GfxIndexBufferFormat_U8
	DXGI_FORMAT_R16_UINT, // GfxIndexBufferFormat_U16
	DXGI_FORMAT_R32_UINT, // GfxIndexBufferFormat_U32
};
static_assert( ARRAY_LENGTH( D3D11IndexBufferFormats ) == GFXINDEXBUFFERFORMAT_COUNT,
	"Missing GfxIndexBufferFormat!" );


enum
{
	D3D11_CPU_ACCESS_NONE = 0,
	D3D11_CPU_ACCESS_READ_WRITE = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE,
};

struct D3D11MapAccessMode { UINT cpuAccessFlag; D3D11_MAP d3d11Map; D3D11_USAGE d3d11Usage; };
static const D3D11MapAccessMode D3D11CPUAccessModes[] =
{
	// GfxCPUAccessMode_NONE
	{ D3D11_CPU_ACCESS_NONE, D3D11_MAP_WRITE_NO_OVERWRITE, D3D11_USAGE_IMMUTABLE },
	// GfxCPUAccessMode_READ
	{ D3D11_CPU_ACCESS_READ, D3D11_MAP_READ, D3D11_USAGE_IMMUTABLE },
	// GfxCPUAccessMode_READ_WRITE
	{ D3D11_CPU_ACCESS_READ_WRITE, D3D11_MAP_READ_WRITE, D3D11_USAGE_DYNAMIC },
	// GfxCPUAccessMode_WRITE
	{ D3D11_CPU_ACCESS_WRITE, D3D11_MAP_WRITE, D3D11_USAGE_DYNAMIC },
	// GfxCPUAccessMode_WRITE_DISCARD
	{ D3D11_CPU_ACCESS_WRITE, D3D11_MAP_WRITE_DISCARD, D3D11_USAGE_DYNAMIC },
	// GfxCPUAccessMode_WRITE_NO_OVERWRITE
	{ D3D11_CPU_ACCESS_WRITE, D3D11_MAP_WRITE_NO_OVERWRITE, D3D11_USAGE_DYNAMIC },
};
static_assert( ARRAY_LENGTH( D3D11CPUAccessModes ) == GFXCPUACCESSMODE_COUNT,
	"Missing GfxCPUAccessMode!" );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Resources

struct GfxShaderResource : public GfxResource
{
	static void release( GfxShaderResource *&resource );
	ID3D11ComputeShader *cs = nullptr;
	ID3D11VertexShader *vs = nullptr;
	ID3D11PixelShader *ps = nullptr;
	ID3D11InputLayout *il = nullptr;
	UINT sizeVS = 0;
	UINT sizePS = 0;
	UINT sizeCS = 0;
	u32 shaderID = 0;
};


struct GfxVertexBufferResource : public GfxResource
{
	static void release( GfxVertexBufferResource *&resource );
	ID3D11Buffer *buffer = nullptr;
	GfxCPUAccessMode accessMode;
	u32 vertexFormat = 0;
	bool mapped = false;
	byte *data = nullptr;
	UINT size = 0; // buffer size in bytes
	UINT stride = 0; // vertex size in bytes
	UINT offset = 0;
	UINT current = 0;
};


struct GfxInstanceBufferResource : public GfxResource
{
	static void release( GfxInstanceBufferResource *&resource );
	ID3D11Buffer *buffer = nullptr;
	GfxCPUAccessMode accessMode;
	u32 instanceFormat = 0;
	bool mapped = false;
	byte *data = nullptr;
	UINT size = 0; // buffer size in bytes
	UINT stride = 0; // vertex size in bytes
	UINT offset = 0;
	UINT current = 0;
};


struct GfxIndexBufferResource : public GfxResource
{
	static void release( GfxIndexBufferResource *&resource );
	ID3D11Buffer *buffer = nullptr;
	GfxCPUAccessMode accessMode = GfxCPUAccessMode_NONE;
	GfxIndexBufferFormat format = GfxIndexBufferFormat_U32;
	double indicesToVerticesRatio = 1.0;
	UINT size = 0; // buffer size in bytes
};


struct GfxUniformBufferResource : public GfxResource
{
	static void release( GfxUniformBufferResource *&resource );
	ID3D11Buffer *buffer = nullptr;
	bool mapped = false;
	byte *data = nullptr;
	const char *name = "";
	int index = 0;
	UINT size = 0; // buffer size in bytes
};


struct GfxTextureResource : public GfxResource
{
	static void release( GfxTextureResource *&resource );
	ID3D11ShaderResourceView *srv = nullptr;
	ID3D11UnorderedAccessView *uav = nullptr;
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
	ID3D11RenderTargetView *rtv = nullptr;
	ID3D11DepthStencilView *dsv = nullptr;
	ID3D11Texture2D *textureColorSS = nullptr;
	ID3D11Texture2D *textureColorMS = nullptr;
	ID3D11Texture2D *textureDepthSS = nullptr;
	ID3D11Texture2D *textureDepthMS = nullptr;
	ID3D11Texture2D *textureColorStaging = nullptr;
	ID3D11Texture2D *textureDepthStaging = nullptr;
	GfxRenderTargetDescription desc = { };
	u16 width = 0;
	u16 height = 0;
	usize size = 0;
};


static GfxResourceFactory<GfxShaderResource, GFX_RESOURCE_COUNT_SHADER> shaderResources;
static GfxResourceFactory<GfxVertexBufferResource, GFX_RESOURCE_COUNT_VERTEX_BUFFER> vertexBufferResources;
static GfxResourceFactory<GfxInstanceBufferResource, GFX_RESOURCE_COUNT_INDEX_BUFFER> instanceBufferResources;
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
// MSAA Resolver

// NOTE: D3D11 does not have a built-in implemention for resolving multi-sample depth/stencil textures to a
// single-sample texture. For color textures, this is possible with context->ResolveSubresource( ... ).
//
// The purpose of these msaa_resolver functions is to allow for resolving of depth/stencil textures implemented via
// custom compute shaders. We compile these shaders at startup and store them statically here in the D3D11 backend.

static ID3D11ComputeShader *MSAA_RESOLVER_CS_DEPTH;
static ID3D11ComputeShader *MSAA_RESOLVER_CS_DEPTH_STENCIL;


static bool msaa_resolver_init()
{
	HRESULT hr;
	ID3D10Blob *csBlob = nullptr;
	ID3D10Blob *errBlob = nullptr;

	// Depth
	static const char* hlslResolveDepth = R"(
		Texture2DMS<float> DepthMS : register( t0 );
		RWTexture2D<float> DepthSS : register( u0 );

		[numthreads(8,8,1)]
		void main( uint3 DTid : SV_DispatchThreadID )
		{
			uint2 coord = DTid.xy;
			DepthSS[coord] = DepthMS.Load( coord, 0 );
		}
		)";

	hr = D3DCompile( hlslResolveDepth, strlen( hlslResolveDepth ),
					nullptr, nullptr, nullptr, "main", "cs_5_0", 0, 0, &csBlob, &errBlob );
	if( FAILED( hr ) ) { return false; }
	device->CreateComputeShader( csBlob->GetBufferPointer(), csBlob->GetBufferSize(),
		nullptr, &MSAA_RESOLVER_CS_DEPTH );
	csBlob->Release();

	// Depth Stencil
	static const char* hlslResolveDepthStencil = R"(
		Texture2DMS<float> DepthMS : register( t0 );
		Texture2DMS<uint> StencilMS : register( t1 );
		RWTexture2D<float> DepthSS : register( u0 );
		RWTexture2D<uint> StencilSS : register( u1 );

		[numthreads(8,8,1)]
		void main( uint3 DTid : SV_DispatchThreadID )
		{
			uint2 coord = DTid.xy;
			DepthSS[coord] = DepthMS.Load( coord, 0 );
			StencilSS[coord] = StencilMS.Load( coord, 0 );
		}
		)";

	hr = D3DCompile( hlslResolveDepthStencil, strlen( hlslResolveDepthStencil ),
		nullptr, nullptr, nullptr, "main", "cs_5_0", 0, 0, &csBlob, &errBlob );
	if( FAILED( hr ) ) { return false; }
	device->CreateComputeShader( csBlob->GetBufferPointer(), csBlob->GetBufferSize(),
		nullptr, &MSAA_RESOLVER_CS_DEPTH_STENCIL );
	csBlob->Release();

	return true;
}


static void msaa_resolver_free()
{
	if( MSAA_RESOLVER_CS_DEPTH )
	{
		MSAA_RESOLVER_CS_DEPTH->Release();
		MSAA_RESOLVER_CS_DEPTH = nullptr;
	}

	if( MSAA_RESOLVER_CS_DEPTH_STENCIL )
	{
		MSAA_RESOLVER_CS_DEPTH_STENCIL->Release();
		MSAA_RESOLVER_CS_DEPTH_STENCIL = nullptr;
	}
}


static void msaa_resolve_color( ID3D11Texture2D *textureColorMS, ID3D11Texture2D *textureColorSS,
	GfxColorFormat format )
{
	if( format == GfxColorFormat_NONE ) { return; }
	if( !textureColorMS || !textureColorSS ) { return; }
	context->ResolveSubresource( textureColorSS, 0, textureColorMS, 0, D3D11ColorFormats[format] );
}


static void msaa_resolve_depth( ID3D11Texture2D *textureDepthMS, ID3D11Texture2D *textureDepthSS,
	ID3D11Texture2D *textureStencilSS, GfxDepthFormat format )
{
	if( format == GfxDepthFormat_NONE ) { return; }

	D3D11_TEXTURE2D_DESC descSS;
	textureDepthSS->GetDesc( &descSS );
	UINT groupsX = ( descSS.Width + 7 ) / 8;
	UINT groupsY = ( descSS.Height + 7 ) / 8;

	// Depth-Stencil
	if( format == GfxDepthFormat_R24_UINT_G8_UINT )
	{
		GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this
		return;
	}
	// Depth
	{
		DECL_ZERO( D3D11_SHADER_RESOURCE_VIEW_DESC, srvDesc );
		srvDesc.Format = D3D11DepthStencilFormats[format].srvFormat;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
		ID3D11ShaderResourceView *depthSRV = nullptr;
		if( FAILED( device->CreateShaderResourceView( textureDepthMS, &srvDesc, &depthSRV ) ) )
		{
			return;
		}

		DECL_ZERO( D3D11_UNORDERED_ACCESS_VIEW_DESC, uavDesc );
		uavDesc.Format = D3D11DepthStencilFormats[format].uavFormat;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Texture2D.MipSlice = 0;
		ID3D11UnorderedAccessView *depthUAV = nullptr;
	  	if( FAILED( device->CreateUnorderedAccessView( textureDepthSS, &uavDesc, &depthUAV ) ) )
		{
			depthSRV->Release();
			return;
		}

		context->CSSetShader( MSAA_RESOLVER_CS_DEPTH, nullptr, 0 );
		context->CSSetShaderResources( 0, 1, &depthSRV );
		context->CSSetUnorderedAccessViews( 0, 1, &depthUAV, nullptr );
		context->Dispatch( groupsX, groupsY, 1 );

		ID3D11ShaderResourceView *nullSRV[] = { nullptr };
		context->CSSetShaderResources( 0, 1, nullSRV );
		ID3D11UnorderedAccessView *nullUAV[] = { nullptr };
		context->CSSetUnorderedAccessViews( 0, 1, nullUAV, nullptr );
		context->CSSetShader( nullptr, nullptr, 0 );

		depthSRV->Release();
		depthUAV->Release();
	}
}


static void msaa_resolve_render_target_2d( GfxRenderTargetResource *const resource )
{
	if( resource == nullptr ) { return; }
	const GfxRenderTargetDescription &desc = resource->desc;
	if( desc.sampleCount <= 1 ) { return; }

	if( resource->textureColorSS != nullptr && resource->textureColorMS != nullptr )
	{
		msaa_resolve_color( resource->textureColorMS, resource->textureColorSS, desc.colorFormat );
	}

	if( resource->textureDepthSS != nullptr && resource->textureDepthMS != nullptr )
	{
		msaa_resolve_depth( resource->textureDepthMS, resource->textureDepthSS, nullptr, desc.depthFormat );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// D3D11 System

static bool d3d11_device_init()
{
	D3D_FEATURE_LEVEL featureLevel;
	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
	};
	UINT numFeatureLevels = sizeof( featureLevels ) / sizeof( featureLevels[0] );

#if COMPILE_DEBUG
	UINT flags = 0; // D3D11_CREATE_DEVICE_SINGLETHREADED | D3D11_CREATE_DEVICE_DEBUG
#else
	UINT flags = 0; // D3D11_CREATE_DEVICE_SINGLETHREADED;
#endif

	if( FAILED( D3D11CreateDevice( nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, featureLevels, numFeatureLevels,
		D3D11_SDK_VERSION, &device, &featureLevel, &context ) ) )
	{
		ErrorReturnMsg( false, "%s: Failed to create device", __FUNCTION__ );
	}

	return true;
}


static bool d3d11_device_free()
{
	device->Release();
	return true;
}


static bool d3d11_factory_init()
{
	if( FAILED( CreateDXGIFactory( IID_IDXGIFactory, reinterpret_cast<void **>( &factory ) ) ) )
	{
		ErrorReturnMsg( false, "%s: Failed to create DXGI factory", __FUNCTION__ );
	}

	return true;
}


static bool d3d11_factory_free()
{
	factory->Release();
	return true;
}


static CoreGfx::D3D11InputLayoutDescription d3d11_build_input_layout_description(
	const u32 vertexFormatID, const u32 instanceFormatID )
{
	static D3D11_INPUT_ELEMENT_DESC scratch[255];
	int count = 0;

	// Per-Vertex Input Layouts
	if( vertexFormatID != U32_MAX )
	{
		CoreGfx::D3D11InputLayoutDescription vertex;
		CoreGfx::d3d11_input_layout_desc_vertex[vertexFormatID]( vertex );
		if( vertex.desc != nullptr )
		{
			memory_copy( &scratch[count], vertex.desc, vertex.count * sizeof( D3D11_INPUT_ELEMENT_DESC ) );
		}
		count += vertex.count;
	}

	// Per-Instance
	if( instanceFormatID != U32_MAX )
	{
		CoreGfx::D3D11InputLayoutDescription instance;
		CoreGfx::d3d11_input_layout_desc_instance[instanceFormatID]( instance );
		if( instance.desc != nullptr )
		{
			memory_copy( &scratch[count], instance.desc, instance.count * sizeof( D3D11_INPUT_ELEMENT_DESC ) );
		}
		count += instance.count;
	}

	CoreGfx::D3D11InputLayoutDescription inputLayoutDescription;
	inputLayoutDescription.desc = &scratch[0];
	inputLayoutDescription.count = count;
	return inputLayoutDescription;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// D3D11 Render State

static bool bind_shader( GfxShaderResource *const resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	const bool dirty = BITFLAG_IS_SET( CoreGfx::state.dirtyFlags, GfxStateDirtyFlag_SHADER );
	if( !dirty && resource == CoreGfx::state.pipeline.shader ) { return true; }

	CoreGfx::state.pipeline.shader = resource;
	context->VSSetShader( resource->vs, nullptr, 0 );
	context->PSSetShader( resource->ps, nullptr, 0 );
	context->IASetInputLayout( resource->il );
	PROFILE_GFX( Gfx::stats.frame.shaderBinds++ );

	ErrorIf( !CoreGfx::api_shader_bind_uniform_buffers_vertex[resource->shaderID](),
		"Failed to bind vertex shader uniform buffers! (%u)", resource->shaderID );
	ErrorIf( !CoreGfx::api_shader_bind_uniform_buffers_fragment[resource->shaderID](),
		"Failed to bind fragment shader uniform buffers! (%u)", resource->shaderID );
	ErrorIf( !CoreGfx::api_shader_bind_uniform_buffers_compute[resource->shaderID](),
		"Failed to bind compute shader uniform buffers! (%u)", resource->shaderID );

	BITFLAG_UNSET( CoreGfx::state.dirtyFlags, GfxStateDirtyFlag_SHADER );
	return true;
}


static bool apply_pipeline_state_raster( const GfxPipelineDescription &state )
{
	GfxPipelineDescription &stateCurrent = CoreGfx::state.pipeline.description;
	const bool dirty = BITFLAG_IS_SET( CoreGfx::state.dirtyFlags, GfxStateDirtyFlag_RASTER );
	if( !dirty && state.equal_raster( stateCurrent ) ) { return true; }

	DECL_ZERO( D3D11_RASTERIZER_DESC, rsDesc );
	Assert( state.rasterFillMode < GFXRASTERFILLMODE_COUNT );
	rsDesc.FillMode = D3D11FillModes[state.rasterFillMode];
	Assert( state.rasterCullMode < GFXRASTERCULLMODE_COUNT );
	rsDesc.CullMode = D3D11CullModes[state.rasterCullMode];
	rsDesc.FrontCounterClockwise = false;
	rsDesc.DepthBias = 0;
	rsDesc.DepthBiasClamp = 0.0f;
	rsDesc.SlopeScaledDepthBias = 0.0f;
	rsDesc.DepthClipEnable = false;
	rsDesc.MultisampleEnable = false;
	rsDesc.AntialiasedLineEnable = false;
	rsDesc.ScissorEnable = CoreGfx::state.scissor.is_enabled();

	ID3D11RasterizerState *rs = nullptr;
	if( FAILED( device->CreateRasterizerState( &rsDesc, &rs ) ) )
	{
		ErrorReturnMsg( false, "%s: Failed to create rasterizer state", __FUNCTION__ );
	}
	context->RSSetState( rs );
	rs->Release();

	stateCurrent.raster_set( state );
	BITFLAG_UNSET( CoreGfx::state.dirtyFlags, GfxStateDirtyFlag_RASTER );
	return true;
}


static bool apply_pipeline_state_blend( const GfxPipelineDescription &state )
{
	GfxPipelineDescription &stateCurrent = CoreGfx::state.pipeline.description;
	const bool dirty = BITFLAG_IS_SET( CoreGfx::state.dirtyFlags, GfxStateDirtyFlag_BLEND );
	if( !dirty && state.equal_blend( stateCurrent ) ) { return true; }

	DECL_ZERO( D3D11_BLEND_DESC, bsDesc );
	bsDesc.AlphaToCoverageEnable = false;
	bsDesc.IndependentBlendEnable = false;
	bsDesc.RenderTarget[0].BlendEnable = state.blendEnabled && state.blendColorWriteMask;
	bsDesc.RenderTarget[0].SrcBlend = D3D11BlendFactors[state.blendSrcFactorColor];
	bsDesc.RenderTarget[0].DestBlend = D3D11BlendFactors[state.blendDstFactorColor];
	bsDesc.RenderTarget[0].BlendOp = D3D11BlendOperations[state.blendOperationColor];
	bsDesc.RenderTarget[0].SrcBlendAlpha = D3D11BlendFactors[state.blendSrcFactorAlpha];
	bsDesc.RenderTarget[0].DestBlendAlpha = D3D11BlendFactors[state.blendDstFactorAlpha];
	bsDesc.RenderTarget[0].BlendOpAlpha = D3D11BlendOperations[state.blendOperationAlpha];
	bsDesc.RenderTarget[0].RenderTargetWriteMask = state.blendColorWriteMask;

	ID3D11BlendState *bs = nullptr;
	if( FAILED( device->CreateBlendState( &bsDesc, &bs ) ) )
	{
		ErrorReturnMsg( false, "%s: Failed to create blend state", __FUNCTION__ );
	}
	context->OMSetBlendState( bs, nullptr, 0xFFFFFFFF );
	bs->Release();

	stateCurrent.blend_set( state );
	BITFLAG_UNSET( CoreGfx::state.dirtyFlags, GfxStateDirtyFlag_BLEND );
	return true;
}


static bool apply_pipeline_state_depth( const GfxPipelineDescription &state )
{
	GfxPipelineDescription &stateCurrent = CoreGfx::state.pipeline.description;
	const bool dirty = BITFLAG_IS_SET( CoreGfx::state.dirtyFlags, GfxStateDirtyFlag_DEPTH );
	if( !dirty && state.equal_depth( stateCurrent ) ) { return true; }

	DECL_ZERO( D3D11_DEPTH_STENCIL_DESC, dsDesc );
	dsDesc.DepthEnable = state.depthFunction != GfxDepthFunction_NONE;
	dsDesc.DepthWriteMask = D3D11DepthWriteMasks[state.depthWriteMask];
	dsDesc.DepthFunc = D3D11DepthFunctions[state.depthFunction];
	dsDesc.StencilEnable = false;
	dsDesc.StencilReadMask = 0;
	dsDesc.StencilWriteMask = 0;

	ID3D11DepthStencilState *dss = nullptr;
	if( FAILED( device->CreateDepthStencilState( &dsDesc, &dss ) ) )
	{
		ErrorReturnMsg( false, "%s: Failed to create depth stencil state", __FUNCTION__ );
	}
	context->OMSetDepthStencilState( dss, 0 );
	dss->Release();

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

	if( state.is_enabled() )
	{
		D3D11_RECT rect;
		rect.left = state.x1;
		rect.top = state.y1;
		rect.right = state.x2;
		rect.bottom = state.y2;
		context->RSSetScissorRects( 1, &rect );
	}
	else
	{
		context->RSSetScissorRects( 0, nullptr );
	}

	CoreGfx::state.scissor = state;
	BITFLAG_UNSET( CoreGfx::state.dirtyFlags, GfxStateDirtyFlag_SCISSOR );
	return true;
}


static bool bind_targets( GfxRenderTargetResource *const resources[] )
{
	static double_m44 cacheMatrixModel;
	static double_m44 cacheMatrixView;
	static double_m44 cacheMatrixPerspective;

	bool dirtySlots[GFX_RENDER_TARGET_SLOT_COUNT] = { false };
	bool dirtyState = BITFLAG_IS_SET( CoreGfx::state.dirtyFlags, GfxStateDirtyFlag_TARGETS );

	bool hasCustomRenderTargets = false;

	for( int slot = 0; slot < GFX_RENDER_TARGET_SLOT_COUNT; slot++ )
	{
		GfxRenderTargetResource *const resource = resources[slot];

		if( resource != nullptr && stateBoundTargetResources[slot] != resource )
		{
			msaa_resolve_render_target_2d( stateBoundTargetResources[slot] );
		}

		hasCustomRenderTargets |= ( resource != nullptr );
		dirtySlots[slot] |= ( stateBoundTargetResources[slot] != resource );
		stateBoundTargetResources[slot] = resource;

		if( resource == nullptr )
		{
			// Color
			ID3D11RenderTargetView *viewColor = slot == 0 ? swapchainViewColor : nullptr;
			dirtySlots[slot] |= ( stateBoundViewColor[slot] != viewColor );
			stateBoundViewColor[slot] = viewColor;

			// Depth
			if( slot == 0 )
			{
				ID3D11DepthStencilView *viewDepth = swapchainViewDepth;
				dirtySlots[slot] |= ( stateBoundViewDepth != viewDepth );
				stateBoundViewDepth = viewDepth;
			}
		}
		else
		{
			// Color
			ID3D11RenderTargetView *viewColor = resource->rtv;
			hasCustomRenderTargets |= ( viewColor != nullptr );
			dirtySlots[slot] |= ( stateBoundViewColor[slot] != viewColor );
			stateBoundViewColor[slot] = viewColor;

			// Depth
			if( slot == 0 )
			{
				ID3D11DepthStencilView *viewDepth = resource->dsv;
				hasCustomRenderTargets |= ( viewDepth != nullptr );
				dirtySlots[slot] |= ( stateBoundViewDepth != viewDepth );
				stateBoundViewDepth = viewDepth;
			}
		}

		dirtyState |= dirtySlots[slot];
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

	context->OMSetRenderTargets( GFX_RENDER_TARGET_SLOT_COUNT, stateBoundViewColor, stateBoundViewDepth );

	// D3D11: When binding to a FBO, we need to update the viewport and MVP matrices
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
		Gfx::viewport_set_size( Window::width * Window::scale, Window::height * Window::scale );
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
	const u16 w = Window::width;
	const u16 h = Window::height;
	const u16 f = Window::fullscreen;

	ErrorReturnIf( !d3d11_device_init(), false, "%s: Failed to create D3D11 device", __FUNCTION__ );
	ErrorReturnIf( !d3d11_factory_init(), false, "%s: Failed to create D3D11 factory", __FUNCTION__ );
	ErrorReturnIf( !api_swapchain_init( w, h, f ), false, "%s: Failed to create swap chain", __FUNCTION__ );
	ErrorReturnIf( !msaa_resolver_init(), false, "%s: Failed to init MSAA resolver", __FUNCTION__ );
	ErrorReturnIf( !resources_init(), false, "%s: Failed to create gfx resources", __FUNCTION__ );

#if COMPILE_DEBUG
	context->QueryInterface( IID_ID3DUserDefinedAnnotation, reinterpret_cast<void **>( &annotation ) );
#endif

	return true;
}


bool CoreGfx::api_free()
{
#if COMPILE_DEBUG
	annotation->Release();
#endif

	resources_free();
	msaa_resolver_free();
	resources_free();
	d3d11_factory_free();
	d3d11_device_free();

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Frame

void CoreGfx::api_frame_begin()
{
	bind_shader( CoreGfx::shaders[Shader::SHADER_DEFAULT].resource );

	BITFLAG_SET( CoreGfx::state.dirtyFlags, GfxStateDirtyFlag_TARGETS );
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
	BITFLAG_SET( CoreGfx::state.dirtyFlags, GfxStateDirtyFlag_TARGETS );
	render_targets_reset();

	const bool swap = SUCCEEDED( swapchain->Present( 0, swapchainFlagsPresent ) );
	Assert( swap );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Clear

void CoreGfx::api_clear_color( const Color color )
{
	static constexpr float INV_255 = 1.0f / 255.0f;
	const FLOAT rgba[4] = { color.r * INV_255, color.g * INV_255, color.b * INV_255, color.a * INV_255 };

	for( int slot = 0; slot < GFX_RENDER_TARGET_SLOT_COUNT; slot++ )
	{
		if( stateBoundViewColor[slot] == nullptr ) { continue; }
		context->ClearRenderTargetView( stateBoundViewColor[slot], rgba );
	}
}


void CoreGfx::api_clear_depth( const float depth )
{
	if( stateBoundViewDepth == nullptr ) { return; }
	context->ClearDepthStencilView( stateBoundViewDepth, D3D11_CLEAR_DEPTH, depth, 0 );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Render State

bool CoreGfx::api_swapchain_init( const u16 width, const u16 height, const float dpi )
{
	const GfxColorFormat colorFormat = GfxColorFormat_R8G8B8A8_FLOAT; // TODO: Support other formats

	CoreGfx::state.swapchain.width = width;
	CoreGfx::state.swapchain.height = height;
	CoreGfx::state.swapchain.dpi = dpi;

	DECL_ZERO( DXGI_SWAP_CHAIN_DESC, swapChainDesc );
	swapChainDesc.BufferDesc.Width = width;
	swapChainDesc.BufferDesc.Height = height;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferDesc.Format = D3D11ColorFormats[colorFormat];
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.OutputWindow = CoreWindow::handle;
	swapChainDesc.Windowed = true;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING; // TODO: This can fail on unsupported systems

	Assert( swapchain == nullptr );
	if( FAILED( factory->CreateSwapChain( device, &swapChainDesc, &swapchain ) ) )
	{
		// If we fail to create a swap chain that should be compatible with Windows 10,
		// we'll falll back to a windows 8.1 compatible one...
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;

		if( FAILED( factory->CreateSwapChain( device, &swapChainDesc, &swapchain ) ) )
		{
			// If we fail to create a swap chain that should be compatible with Windows 8.1,
			// we'll fall back to a standard compatible one...
			swapChainDesc.BufferCount = 1;
			swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
			swapChainDesc.Flags = 0;

			if( FAILED( factory->CreateSwapChain( device, &swapChainDesc, &swapchain ) ) )
			{
				// We were unable to make any swap chain... this is an error
				ErrorReturnMsg( false, "%s: Failed to create swapchain", __FUNCTION__ );
			}
		}
	}

	// Cache Flags
	swapchainFlagsCreate = swapChainDesc.Flags;
	swapchainFlagsPresent = swapChainDesc.Flags == DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING ?
		DXGI_PRESENT_ALLOW_TEARING : 0;

	// Disable Alt + Enter (TODO: Move somewhere else?)
	IDXGIFactory *parent;
	if( SUCCEEDED( swapchain->GetParent( IID_IDXGIFactory, reinterpret_cast<void **>( &parent ) ) ) )
	{
		parent->MakeWindowAssociation( CoreWindow::handle, DXGI_MWA_NO_ALT_ENTER );
		parent->Release();
	}

	// Swapchain Color Target
	{
		ID3D11Texture2D *texture = nullptr;

		if( FAILED( swapchain->GetBuffer( 0, IID_ID3D11Texture2D, reinterpret_cast<void **>( &texture ) ) ) )
		{
			ErrorReturnMsg( false, "%s: Failed to get back swapchain buffer pointer", __FUNCTION__ );
		}

		PROFILE_GFX( Gfx::stats.gpuMemorySwapchain +=
			GFX_SIZE_IMAGE_COLOR_BYTES( width, height, 1, colorFormat ) * 2 );

		Assert( swapchainViewColor == nullptr );
		if( FAILED( device->CreateRenderTargetView( texture, nullptr, &swapchainViewColor ) ) )
		{
			ErrorReturnMsg( false, "%s: Failed to create swapchain color render target view", __FUNCTION__ );
		}

		texture->Release();
	}

#if SWAPCHAIN_DEPTH_ENABLED
	// Swapchain Depth Target
	{
		ID3D11Texture2D *texture = nullptr;

		DECL_ZERO( D3D11_TEXTURE2D_DESC, tDesc );
		tDesc.Width = width;
		tDesc.Height = height;
		tDesc.MipLevels = 1;
		tDesc.ArraySize = 1;
		tDesc.Format = D3D11DepthStencilFormats[SWAPCHAIN_DEPTH_FORMAT].texFormat;
		tDesc.SampleDesc.Count = 1;
		tDesc.SampleDesc.Quality = 0;
		tDesc.Usage = D3D11_USAGE_DEFAULT;
		tDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		tDesc.CPUAccessFlags = 0;
		tDesc.MiscFlags = 0;

		if( FAILED( device->CreateTexture2D( &tDesc, nullptr, &texture ) ) )
		{
			ErrorReturnMsg( false, "%s: Failed to create swapchain depth texture", __FUNCTION__ );
		}

		PROFILE_GFX( Gfx::stats.gpuMemorySwapchain +=
			GFX_SIZE_IMAGE_DEPTH_BYTES( width, height, 1, SWAPCHAIN_DEPTH_FORMAT ) * 2 );

		DECL_ZERO( D3D11_DEPTH_STENCIL_VIEW_DESC, dsvDesc );
		dsvDesc.Format = D3D11DepthStencilFormats[SWAPCHAIN_DEPTH_FORMAT].dsvFormat;
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Flags = 0;
		dsvDesc.Texture2D.MipSlice = 0;

		Assert( swapchainViewDepth == nullptr );
		if( FAILED( device->CreateDepthStencilView( texture, &dsvDesc, &swapchainViewDepth ) ) )
		{
			ErrorReturnMsg( false, "%s: Failed to create swapchain depth render target view", __FUNCTION__ );
		}

		texture->Release();
	}
#endif

	CoreGfx::api_viewport_set_size( width, height, dpi );
	return true;
}


bool CoreGfx::api_swapchain_free()
{
	Assert( swapchainViewColor != nullptr );
	swapchainViewColor->Release();

#if SWAPCHAIN_DEPTH_ENABLED
	Assert( swapchainViewDepth != nullptr );
	swapchainViewDepth->Release();
#endif

	Assert( swapchain != nullptr );
	swapchain->Release();
	swapchain = nullptr;

	return true;
}


bool CoreGfx::api_swapchain_set_size( const u16 width, const u16 height, const float dpi )
{
	const GfxColorFormat colorFormat = GfxColorFormat_R8G8B8A8_FLOAT; // TODO: Support other formats

	const u16 widthPrevious = CoreGfx::state.swapchain.width;
	const u16 heightPrevious = CoreGfx::state.swapchain.height;
	const float dpiPrevious = CoreGfx::state.swapchain.dpi;

	CoreGfx::state.swapchain.width = width;
	CoreGfx::state.swapchain.height = height;
	CoreGfx::state.swapchain.dpi = dpi;

	context->VSSetShader( nullptr, nullptr, 0 );
	context->PSSetShader( nullptr, nullptr, 0 );
	context->IASetInputLayout( nullptr );
	context->OMSetRenderTargets( 0, nullptr, nullptr );

	Assert( swapchainViewColor != nullptr );
	swapchainViewColor->Release();
	swapchainViewColor = nullptr;

#if SWAPCHAIN_DEPTH_ENABLED
	Assert( swapchainViewDepth != nullptr );
	swapchainViewDepth->Release();
	swapchainViewDepth = nullptr;
#endif

	// Resize Swapchain
	HRESULT hr = swapchain->ResizeBuffers( 2, width, height, D3D11ColorFormats[colorFormat], swapchainFlagsCreate );
	if( FAILED( hr ) )
	{
		ErrorReturnMsg( false, "%s: Failed to resize swapchain buffers", __FUNCTION__ );
	}

	// Color Target
	{
		ID3D11Texture2D *texture = nullptr;

		if( FAILED( swapchain->GetBuffer( 0, IID_ID3D11Texture2D, reinterpret_cast<void **>( &texture ) ) ) )
		{
			ErrorReturnMsg( false, "%s: Failed to get back swapchain buffer pointer", __FUNCTION__ );
		}

		Assert( swapchainViewColor == nullptr );
		if( FAILED( device->CreateRenderTargetView( texture, nullptr, &swapchainViewColor ) ) )
		{
			ErrorReturnMsg( false, "%s: Failed to create swapchain color render target view", __FUNCTION__ );
		}

		PROFILE_GFX( Gfx::stats.gpuMemorySwapchain -=
			GFX_SIZE_IMAGE_COLOR_BYTES( widthPrevious, heightPrevious, 1, colorFormat ) * 2 );
		PROFILE_GFX( Gfx::stats.gpuMemorySwapchain +=
			GFX_SIZE_IMAGE_COLOR_BYTES( width, height, 1, colorFormat ) * 2 );

		texture->Release();
	}

#if SWAPCHAIN_DEPTH_ENABLED
	// Depth Target
	{
		ID3D11Texture2D *texture = nullptr;

		DECL_ZERO( D3D11_TEXTURE2D_DESC, tDesc );
		tDesc.Width = width;
		tDesc.Height = height;
		tDesc.MipLevels = 1;
		tDesc.ArraySize = 1;
		tDesc.Format = D3D11DepthStencilFormats[SWAPCHAIN_DEPTH_FORMAT].texFormat;
		tDesc.SampleDesc.Count = 1;
		tDesc.SampleDesc.Quality = 0;
		tDesc.Usage = D3D11_USAGE_DEFAULT;
		tDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		tDesc.CPUAccessFlags = 0;
		tDesc.MiscFlags = 0;

		if( FAILED( device->CreateTexture2D( &tDesc, nullptr, &texture ) ) )
		{
			ErrorReturnMsg( false, "%s: Failed to create swapchain depth texture", __FUNCTION__ );
		}

		DECL_ZERO( D3D11_DEPTH_STENCIL_VIEW_DESC, dsvDesc );
		dsvDesc.Format = D3D11DepthStencilFormats[SWAPCHAIN_DEPTH_FORMAT].dsvFormat;
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Flags = 0;
		dsvDesc.Texture2D.MipSlice = 0;

		Assert( swapchainViewDepth == nullptr );
		if( FAILED( device->CreateDepthStencilView( texture, &dsvDesc, &swapchainViewDepth ) ) )
		{
			ErrorReturnMsg( false, "%s: Failed to create swapchain depth render target view", __FUNCTION__ );
		}

		PROFILE_GFX( Gfx::stats.gpuMemorySwapchain -=
			GFX_SIZE_IMAGE_DEPTH_BYTES( widthPrevious, heightPrevious, 1, SWAPCHAIN_DEPTH_FORMAT ) * 2 );
		PROFILE_GFX( Gfx::stats.gpuMemorySwapchain +=
			GFX_SIZE_IMAGE_DEPTH_BYTES( width, height, 1, SWAPCHAIN_DEPTH_FORMAT ) * 2 );

		texture->Release();
	}
#endif

	BITFLAG_SET( CoreGfx::state.dirtyFlags, GfxStateDirtyFlag_TARGETS );
	render_targets_reset();
	BITFLAG_SET( CoreGfx::state.dirtyFlags, GfxStateDirtyFlag_SHADER );
	bind_shader( CoreGfx::state.pipeline.shader );

	return true;
}


bool CoreGfx::api_viewport_init( const u16 width, const u16 height, const float dpi )
{
	return api_viewport_set_size( width, height, dpi );
}


bool CoreGfx::api_viewport_free()
{
	GRAPHICS_API_IMPLEMENTATION_WARNING
	return true;
}


bool CoreGfx::api_viewport_set_size( const u16 width, const u16 height, const float dpi )
{
	CoreGfx::state.viewport.width = width;
	CoreGfx::state.viewport.height = height;
	CoreGfx::state.viewport.dpi = dpi;

	DECL_ZERO( D3D11_VIEWPORT, d3d11Viewport );
	d3d11Viewport.TopLeftX = 0.0f;
	d3d11Viewport.TopLeftY = 0.0f;
	d3d11Viewport.Width = static_cast<FLOAT>( width );
	d3d11Viewport.Height = static_cast<FLOAT>( height );
	d3d11Viewport.MinDepth = 0.0f;
	d3d11Viewport.MaxDepth = 1.0f;

	context->RSSetViewports( 1, &d3d11Viewport );
	return true;
}


bool CoreGfx::api_scissor_set_state( const GfxStateScissor &state )
{
	const bool dirty = BITFLAG_IS_SET( CoreGfx::state.dirtyFlags, GfxStateDirtyFlag_SCISSOR );
	if( !dirty && state == CoreGfx::state.scissor ) { return true; }

	if( apply_state_scissor( state ) ) { return false; }

	// D3D11: Scissor state requires updating the raster state, so we force it here too
	BITFLAG_SET( CoreGfx::state.dirtyFlags, GfxStateDirtyFlag_RASTER );
	if( !apply_pipeline_state_raster( CoreGfx::state.pipeline.description ) ) { return false; }

	return true;
}


bool CoreGfx::api_sampler_set_state( const GfxStateSampler &state )
{
	DECL_ZERO( D3D11_SAMPLER_DESC, ssDesc );
	ssDesc.Filter = D3D11FilteringModes[state.filterMode];
	ssDesc.AddressU = D3D11UVWrapModes[state.wrapMode];
	ssDesc.AddressV = D3D11UVWrapModes[state.wrapMode];
	ssDesc.AddressW = D3D11UVWrapModes[state.wrapMode];
	ssDesc.MipLODBias = 0.0f;
	ssDesc.MaxAnisotropy = state.anisotropy;
	ssDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	ssDesc.BorderColor[0] = 0.0f;
	ssDesc.BorderColor[1] = 0.0f;
	ssDesc.BorderColor[2] = 0.0f;
	ssDesc.BorderColor[3] = 0.0f;
	ssDesc.MinLOD = 0.0f;
	ssDesc.MaxLOD = D3D11_FLOAT32_MAX;

	ID3D11SamplerState *samplerState = nullptr;
	if( FAILED( device->CreateSamplerState( &ssDesc, &samplerState ) ) )
	{
		ErrorReturnMsg( false, "%s: Failed to create sampler state", __FUNCTION__ );
	}

	context->PSSetSamplers( 0, 1, &samplerState ); // TODO: Sampler objects per texture

	CoreGfx::state.sampler = state;
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Pipeline State

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

	if( resource->cs != nullptr ) { resource->cs->Release(); }
	if( resource->vs != nullptr ) { resource->vs->Release(); }
	if( resource->ps != nullptr ) { resource->ps->Release(); }
	if( resource->il != nullptr ) { resource->il->Release(); }

	shaderResources.remove( resource->id );
	resource = nullptr;
}


bool CoreGfx::api_shader_init( GfxShaderResource *&resource, const u32 shaderID,
	const struct ShaderEntry &shaderEntry )
{
	Assert( resource == nullptr );
	Assert( shaderID < CoreGfx::shaderCount );

	// Register Shader
	resource = shaderResources.make_new();
	resource->shaderID = shaderID;

	ID3D10Blob *info;
	ID3D10Blob *csCode, *csStripped;
	ID3D10Blob *vsCode, *vsStripped;
	ID3D10Blob *psCode, *psStripped;
	u32 csSize, vsSize, psSize;

	// Vertex Shader
	if( shaderEntry.sizeVertex > 0 )
	{
		const void *codeVertex = reinterpret_cast<const void *>( Assets::binary.data + shaderEntry.offsetVertex );
		if( FAILED( D3DCompile( codeVertex, shaderEntry.sizeVertex, nullptr, nullptr, nullptr,
			"vs_main", "vs_4_0", D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &vsCode, &info ) ) )
		{
			GfxShaderResource::release( resource );
			ErrorReturnMsg( false,
				"%s: Failed to compile vertex shader (%s)\n\n%s",
				__FUNCTION__, shaderEntry.name, reinterpret_cast<const char *>( info->GetBufferPointer() ) );
		}

		D3DStripShader( vsCode->GetBufferPointer(), vsCode->GetBufferSize(),
			D3DCOMPILER_STRIP_REFLECTION_DATA |
			D3DCOMPILER_STRIP_DEBUG_INFO |
			D3DCOMPILER_STRIP_PRIVATE_DATA, &vsStripped );

		resource->sizeVS = vsStripped->GetBufferSize();
		if( FAILED( device->CreateVertexShader( vsStripped->GetBufferPointer(),
			vsStripped->GetBufferSize(), nullptr, &resource->vs ) ) )
		{
			GfxShaderResource::release( resource );
			ErrorReturnMsg( false, "%s: Failed to create vertex shader (%s)",
				__FUNCTION__, shaderEntry.name );
		}

		D3D11InputLayoutDescription inputLayoutDescription = d3d11_build_input_layout_description(
			CoreGfx::shaderEntries[shaderID].vertexFormat, CoreGfx::shaderEntries[shaderID].instanceFormat );

		if( FAILED( device->CreateInputLayout( inputLayoutDescription.desc, inputLayoutDescription.count,
			vsStripped->GetBufferPointer(),
			vsStripped->GetBufferSize(), &resource->il ) ) )
		{
			GfxShaderResource::release( resource );
			ErrorReturnMsg( false, "%s: Failed to create input layout (%s)",
				__FUNCTION__, shaderEntry.name );
		}
	}

	// Fragment Shader
	if( shaderEntry.sizeFragment > 0 )
	{
		const void *codeFragment = reinterpret_cast<const void *>( Assets::binary.data + shaderEntry.offsetFragment );
		if( FAILED( D3DCompile( codeFragment, shaderEntry.sizeFragment, nullptr, nullptr, nullptr,
			"ps_main", "ps_4_0", D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &psCode, &info ) ) )
		{
			GfxShaderResource::release( resource );
			ErrorReturnMsg( false,
				"%s: Failed to compile fragment shader (%s)\n\n%s",
				__FUNCTION__, shaderEntry.name, reinterpret_cast<const char *>( info->GetBufferPointer() ) );
		}

		D3DStripShader( psCode->GetBufferPointer(), psCode->GetBufferSize(),
			D3DCOMPILER_STRIP_REFLECTION_DATA |
			D3DCOMPILER_STRIP_DEBUG_INFO |
			D3DCOMPILER_STRIP_PRIVATE_DATA, &psStripped );

		resource->sizePS = psStripped->GetBufferSize();
		if( FAILED( device->CreatePixelShader( psStripped->GetBufferPointer(),
			psStripped->GetBufferSize(), nullptr, &resource->ps ) ) )
		{
			GfxShaderResource::release( resource );
			ErrorReturnMsg( false, "%s: Failed to create pixel shader (%s)",
				__FUNCTION__, shaderEntry.name );
		}
	}

	// Compute Shader
#if COMPUTE_SHADERS
	if( shaderEntry.sizeFragment > 0 )
	{
		const void *codeCompute = reinterpret_cast<const void *>( Assets::binary.data + shaderEntry.offsetCompute );
		if( FAILED( D3DCompile( codeCompute, shaderEntry.sizeCompute, nullptr, nullptr, nullptr,
			"vs_main", "vs_4_0", D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &csCode, &info ) ) )
		{
			GfxShaderResource::release( resource );
			ErrorReturnMsg( false,
				"%s: Failed to compile compute shader (%s)\n\n%s",
				__FUNCTION__, shaderEntry.name, reinterpret_cast<const char *>( info->GetBufferPointer() ) );
		}

		D3DStripShader( csCode->GetBufferPointer(), csCode->GetBufferSize(),
			D3DCOMPILER_STRIP_REFLECTION_DATA |
			D3DCOMPILER_STRIP_DEBUG_INFO |
			D3DCOMPILER_STRIP_PRIVATE_DATA, &csStripped );

		resource->sizeCS = csStripped->GetBufferSize();
		if( FAILED( device->CreateComputeShader( csStripped->GetBufferPointer(),
			csStripped->GetBufferSize(), nullptr, &resource->cs ) ) )
		{
			GfxShaderResource::release( resource );
			ErrorReturnMsg( false, "%s: Failed to create compute shader (%s)",
				__FUNCTION__, shaderEntry.name );
		}
	}
#endif

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
// Vertex Buffer

void GfxVertexBufferResource::release( GfxVertexBufferResource *&resource )
{
	if( resource == nullptr || resource->id == GFX_RESOURCE_ID_NULL ) { return; }

	if( resource->buffer != nullptr ) { resource->buffer->Release(); }

	vertexBufferResources.remove( resource->id );
	resource = nullptr;
}


bool CoreGfx::api_vertex_buffer_init_dynamic( GfxVertexBufferResource *&resource, const u32 vertexFormatID,
	const GfxCPUAccessMode accessMode, const u32 size, const u32 stride )
{
	Assert( resource == nullptr );
	Assert( accessMode == GfxCPUAccessMode_WRITE_DISCARD || accessMode == GfxCPUAccessMode_WRITE_NO_OVERWRITE );

	resource = vertexBufferResources.make_new();
	resource->size = size;
	resource->stride = stride;
	resource->accessMode = accessMode;
	resource->vertexFormat = vertexFormatID;

	DECL_ZERO( D3D11_BUFFER_DESC, bfDesc );
	bfDesc.ByteWidth = size;
	bfDesc.Usage = D3D11CPUAccessModes[accessMode].d3d11Usage;
	bfDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bfDesc.CPUAccessFlags = D3D11CPUAccessModes[accessMode].cpuAccessFlag;
	bfDesc.MiscFlags = 0;
	bfDesc.StructureByteStride = 0;

	if( FAILED( device->CreateBuffer( &bfDesc, nullptr, &resource->buffer ) ) )
	{
		GfxVertexBufferResource::release( resource );
		ErrorReturnMsg( false, "%s: Failed to create vertex buffer", __FUNCTION__ );
	}

	PROFILE_GFX( Gfx::stats.gpuMemoryVertexBuffers += resource->size );
	return true;
}


bool CoreGfx::api_vertex_buffer_init_static( GfxVertexBufferResource *&resource, const u32 vertexFormatID,
	const GfxCPUAccessMode accessMode, const void *const data,
	const u32 size, const u32 stride )
{
	Assert( resource == nullptr );
	Assert( accessMode == GfxCPUAccessMode_WRITE_DISCARD || accessMode == GfxCPUAccessMode_WRITE_NO_OVERWRITE );
	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this
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

	const GfxCPUAccessMode accessMode = resource->accessMode;
	Assert( accessMode == GfxCPUAccessMode_WRITE ||
		accessMode == GfxCPUAccessMode_WRITE_DISCARD ||
		accessMode == GfxCPUAccessMode_WRITE_NO_OVERWRITE )

 	DECL_ZERO( D3D11_MAPPED_SUBRESOURCE, mappedResource );
	context->Map( resource->buffer, 0, D3D11CPUAccessModes[accessMode].d3d11Map, 0, &mappedResource );
	resource->mapped = true;

	if( accessMode == GfxCPUAccessMode_WRITE_DISCARD || resource->data == nullptr )
	{
		resource->data = reinterpret_cast<byte *>( mappedResource.pData );
		resource->current = 0;
		resource->offset = 0;
	}
}


void CoreGfx::api_vertex_buffer_write_end( GfxVertexBufferResource *const resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	if( resource->mapped == false ) { return; }

	context->Unmap( resource->buffer, 0 );
	resource->mapped = false;
}


bool CoreGfx::api_vertex_buffer_write( GfxVertexBufferResource *const resource,
	const void *const data, const u32 size )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
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
// Instance Buffer

void GfxInstanceBufferResource::release( GfxInstanceBufferResource *&resource )
{
	if( resource == nullptr || resource->id == GFX_RESOURCE_ID_NULL ) { return; }

	if( resource->buffer != nullptr ) { resource->buffer->Release(); }

	instanceBufferResources.remove( resource->id );
	resource = nullptr;
}


bool CoreGfx::api_instance_buffer_init_dynamic( GfxInstanceBufferResource *&resource, const u32 instanceFormatID,
	const GfxCPUAccessMode accessMode, const u32 size, const u32 stride )
{
	Assert( resource == nullptr );
	Assert( accessMode == GfxCPUAccessMode_WRITE_DISCARD || accessMode == GfxCPUAccessMode_WRITE_NO_OVERWRITE );

	resource = instanceBufferResources.make_new();
	resource->size = size;
	resource->stride = stride;
	resource->accessMode = accessMode;
	resource->instanceFormat = instanceFormatID;

	DECL_ZERO( D3D11_BUFFER_DESC, bfDesc );
	bfDesc.ByteWidth = size;
	bfDesc.Usage = D3D11CPUAccessModes[accessMode].d3d11Usage;
	bfDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bfDesc.CPUAccessFlags = D3D11CPUAccessModes[accessMode].cpuAccessFlag;
	bfDesc.MiscFlags = 0;
	bfDesc.StructureByteStride = 0;

	if( FAILED( device->CreateBuffer( &bfDesc, nullptr, &resource->buffer ) ) )
	{
		GfxInstanceBufferResource::release( resource );
		ErrorReturnMsg( false, "%s: Failed to create instance buffer", __FUNCTION__ );
	}

	PROFILE_GFX( Gfx::stats.gpuMemoryInstanceBuffers += resource->size );
	return true;
}


bool CoreGfx::api_instance_buffer_init_static( GfxInstanceBufferResource *&resource, const u32 instanceFormatID,
	const GfxCPUAccessMode accessMode, const void *const data,
	const u32 size, const u32 stride )
{
	Assert( resource == nullptr );
	Assert( accessMode == GfxCPUAccessMode_WRITE_DISCARD || accessMode == GfxCPUAccessMode_WRITE_NO_OVERWRITE );
	GRAPHICS_API_IMPLEMENTATION_WARNING // TODO: Implement this
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

	const GfxCPUAccessMode accessMode = resource->accessMode;
	Assert( accessMode == GfxCPUAccessMode_WRITE ||
		accessMode == GfxCPUAccessMode_WRITE_DISCARD ||
		accessMode == GfxCPUAccessMode_WRITE_NO_OVERWRITE )

 	DECL_ZERO( D3D11_MAPPED_SUBRESOURCE, mappedResource );
	context->Map( resource->buffer, 0, D3D11CPUAccessModes[accessMode].d3d11Map, 0, &mappedResource );
	resource->mapped = true;

	if( accessMode == GfxCPUAccessMode_WRITE_DISCARD || resource->data == nullptr )
	{
		resource->data = reinterpret_cast<byte *>( mappedResource.pData );
		resource->current = 0;
		resource->offset = 0;
	}
}


void CoreGfx::api_instance_buffer_write_end( GfxInstanceBufferResource *const resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	if( resource->mapped == false ) { return; }

	context->Unmap( resource->buffer, 0 );
	resource->mapped = false;
}


bool CoreGfx::api_instance_buffer_write( GfxInstanceBufferResource *const resource,
	const void *const data, const u32 size )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
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
// Index Buffer

void GfxIndexBufferResource::release( GfxIndexBufferResource *&resource )
{
	if( resource == nullptr || resource->id == GFX_RESOURCE_ID_NULL ) { return; }

	if( resource->buffer != nullptr ) { resource->buffer->Release(); }

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

	resource = indexBufferResources.make_new();
	resource->accessMode = accessMode;
	resource->format = format;
	resource->indicesToVerticesRatio = indicesToVerticesRatio;
	resource->size = size;

	DECL_ZERO( D3D11_BUFFER_DESC, bfDesc );
	bfDesc.ByteWidth = size;
	bfDesc.Usage = D3D11CPUAccessModes[accessMode].d3d11Usage;
	bfDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bfDesc.CPUAccessFlags = D3D11CPUAccessModes[accessMode].cpuAccessFlag;
	bfDesc.MiscFlags = 0;
	bfDesc.StructureByteStride = 0;

	DECL_ZERO( D3D11_SUBRESOURCE_DATA, bfData );
	bfData.pSysMem = data;

	if( FAILED( device->CreateBuffer( &bfDesc, &bfData, &resource->buffer ) ) )
	{
		GfxIndexBufferResource::release( resource );
		ErrorReturnMsg( false, "%s: Failed to create index buffer", __FUNCTION__ );
	}

	PROFILE_GFX( Gfx::stats.gpuMemoryIndexBuffers += size );
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

	if( resource->buffer != nullptr ) { resource->buffer->Release(); }

	uniformBufferResources.remove( resource->id );
	resource = nullptr;
}


bool CoreGfx::api_uniform_buffer_init( GfxUniformBufferResource *&resource, const char *name,
	const int index, const u32 size )
{
	Assert( resource == nullptr );

	resource = uniformBufferResources.make_new();
	resource->name = name;
	resource->index = index;
	resource->size = size;

	DECL_ZERO( D3D11_BUFFER_DESC, bfDesc );
	bfDesc.ByteWidth = size;
	bfDesc.Usage = D3D11_USAGE_DYNAMIC;
	bfDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bfDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bfDesc.MiscFlags = 0;
	bfDesc.StructureByteStride = 0;

	if( FAILED( device->CreateBuffer( &bfDesc, nullptr, &resource->buffer ) ) )
	{
		GfxUniformBufferResource::release( resource );
		ErrorReturnMsg( false, "%s: Failed to create constant buffer", __FUNCTION__ );
	}

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

 	DECL_ZERO( D3D11_MAPPED_SUBRESOURCE, mappedResource );
	context->Map( resource->buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource );

	resource->data = reinterpret_cast<byte *>( mappedResource.pData );
	resource->mapped = true;
}


void CoreGfx::api_uniform_buffer_write_end( GfxUniformBufferResource *const resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	if( resource->mapped == false ) { return; }

	context->Unmap( resource->buffer, 0 );
	resource->mapped = false;
}


bool CoreGfx::api_uniform_buffer_write( GfxUniformBufferResource *const resource, const void *data )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( resource->mapped );
	Assert( resource->data != nullptr );

	memory_copy( resource->data, data, resource->size );

	return true;
}


bool CoreGfx::api_uniform_buffer_bind_vertex( GfxUniformBufferResource *const resource, const int slot )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( slot >= 0 );

	context->VSSetConstantBuffers( static_cast<UINT>( slot ), 1, &resource->buffer );

	return true;
}


bool CoreGfx::api_uniform_buffer_bind_fragment( GfxUniformBufferResource *const resource, const int slot )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( slot >= 0 );

	context->PSSetConstantBuffers( static_cast<UINT>( slot ), 1, &resource->buffer );

	return true;
}


bool CoreGfx::api_uniform_buffer_bind_compute( GfxUniformBufferResource *const resource, const int slot )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( slot >= 0 );

	context->CSSetConstantBuffers( static_cast<UINT>( slot ), 1, &resource->buffer );

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

	if( resource->srv != nullptr ) { resource->srv->Release(); }

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

	resource = textureResources.make_new();
	resource->type = GfxTextureType_2D;
	resource->colorFormat = format;
	resource->width = width;
	resource->height = height;
	resource->depth = 1U;
	resource->levels = levels;
	resource->size = 0U;

	DECL_ZERO( D3D11_TEXTURE2D_DESC, tDesc );
	tDesc.Width = width;
	tDesc.Height = height;
	tDesc.MipLevels = levels;
	tDesc.ArraySize = 1;
	tDesc.Format = D3D11ColorFormats[format];
	tDesc.SampleDesc.Count = 1;
	tDesc.SampleDesc.Quality = 0;
	tDesc.Usage = D3D11_USAGE_IMMUTABLE;
	tDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	tDesc.CPUAccessFlags = 0;
	tDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA subresources[GFX_MIP_DEPTH_MAX];
	u16 mipWidth = width;
	u16 mipHeight = height;

	for( u16 level = 0; level < levels; level++ )
	{
		D3D11_SUBRESOURCE_DATA &data = subresources[level];
		data.pSysMem = source;
		data.SysMemPitch = mipWidth * pixelSizeBytes;
		data.SysMemSlicePitch = 0;

		const usize mipLevelSizeBytes = mipWidth * mipHeight * pixelSizeBytes;
		source += mipLevelSizeBytes;
		resource->size += mipLevelSizeBytes;

		mipWidth = ( mipWidth > 1 ) ? ( mipWidth >> 1 ) : 1;
		mipHeight = ( mipHeight > 1 ) ? ( mipHeight >> 1 ) : 1;
	}

	ID3D11Texture2D *texture = nullptr;
	if( FAILED( device->CreateTexture2D( &tDesc, subresources, &texture ) ) )
	{
		GfxTextureResource::release( resource );
		ErrorReturnMsg( false, "%s: Failed to create texture 2D", __FUNCTION__ );
	}

	if( FAILED( device->CreateShaderResourceView( texture, nullptr, &resource->srv ) ) )
	{
		texture->Release();
		GfxTextureResource::release( resource );
		ErrorReturnMsg( false, "%s: Failed to create shader resource view", __FUNCTION__ );
	}
	texture->Release(); // Resource owned by resource->view

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


bool CoreGfx::api_texture_bind( GfxTextureResource *const resource, const int slot )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( slot >= 0 && slot < GFX_TEXTURE_SLOT_COUNT );

	static ID3D11ShaderResourceView *srvSlots[GFX_TEXTURE_SLOT_COUNT] = { nullptr };
	srvSlots[slot] = resource->srv;

	PROFILE_GFX( Gfx::stats.frame.textureBinds++ );
	context->VSSetShaderResources( 0, GFX_TEXTURE_SLOT_COUNT, srvSlots );
	context->PSSetShaderResources( 0, GFX_TEXTURE_SLOT_COUNT, srvSlots );

	return true;
}


bool CoreGfx::api_texture_release( GfxTextureResource *const resource, const int slot )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( slot >= 0 && slot < GFX_TEXTURE_SLOT_COUNT );

	// Do nothing...

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Render Target

void GfxRenderTargetResource::release( GfxRenderTargetResource *&resource )
{
	if( resource == nullptr || resource->id == GFX_RESOURCE_ID_NULL ) { return; }

	if( resource->rtv != nullptr ) { resource->rtv->Release(); }
	if( resource->dsv != nullptr ) { resource->dsv->Release(); }
	if( resource->textureColorSS != nullptr ) { resource->textureColorSS->Release(); }
	if( resource->textureColorMS != nullptr ) { resource->textureColorMS->Release(); }
	if( resource->textureDepthSS != nullptr ) { resource->textureDepthSS->Release(); }
	if( resource->textureDepthMS != nullptr ) { resource->textureDepthMS->Release(); }
	if( resource->textureColorStaging != nullptr ) { resource->textureColorStaging->Release(); }
	if( resource->textureDepthStaging != nullptr ) { resource->textureDepthStaging->Release(); }

	renderTargetResources.remove( resource->id );
	resource = nullptr;
}


bool CoreGfx::api_render_target_init( GfxRenderTargetResource *&resource,
	GfxTextureResource *&resourceColor, GfxTextureResource *&resourceDepth,
	const u16 width, const u16 height,
	const GfxRenderTargetDescription &desc )
{
	Assert( resource == nullptr );
	Assert( resourceColor == nullptr );
	Assert( resourceDepth == nullptr );

	resource = renderTargetResources.make_new();
	resource->width = width;
	resource->height = height;
	resource->desc = desc;
	resource->size = 0;

	// Color
	if( desc.colorFormat != GfxColorFormat_NONE )
	{
		resourceColor = textureResources.make_new();
		resourceColor->type = GfxTextureType_2D;

		// Texture2D (Single-Sample)
		DECL_ZERO( D3D11_TEXTURE2D_DESC, tDesc );
		tDesc.Width = width;
		tDesc.Height = height;
		tDesc.MipLevels = 1;
		tDesc.ArraySize = 1;
		tDesc.Format = D3D11ColorFormats[desc.colorFormat];
		tDesc.SampleDesc.Count = 1;
		tDesc.SampleDesc.Quality = 0;
		tDesc.Usage = D3D11_USAGE_DEFAULT;
		tDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		tDesc.CPUAccessFlags = 0;
		tDesc.MiscFlags = 0;

		Assert( resource->textureColorSS == nullptr );
		if( FAILED( device->CreateTexture2D( &tDesc, nullptr, &resource->textureColorSS ) ) )
		{
			GfxTextureResource::release( resourceColor );
			GfxRenderTargetResource::release( resource );
			ErrorReturnMsg( false, "%s: Failed to create color texture 2d", __FUNCTION__ );
		}
		resource->size += GFX_SIZE_IMAGE_COLOR_BYTES( width, height, 1, desc.colorFormat );

		// Texture2D (Multi-Sample)
		if( desc.sampleCount > 1 )
		{
			UINT msaaSampleCount = desc.sampleCount;
			UINT msaaQualityLevels = 0;
			HRESULT hr = device->CheckMultisampleQualityLevels( D3D11ColorFormats[desc.colorFormat],
				msaaSampleCount, &msaaQualityLevels );
			if( FAILED( hr ) || msaaQualityLevels == 0 ) { msaaSampleCount = 1; msaaQualityLevels = 1; }

			tDesc.SampleDesc.Count = msaaSampleCount;
			tDesc.SampleDesc.Quality = msaaQualityLevels - 1;

			Assert( resource->textureColorMS == nullptr );
			if( FAILED( device->CreateTexture2D( &tDesc, nullptr, &resource->textureColorMS ) ) )
			{
				GfxTextureResource::release( resourceColor );
				GfxRenderTargetResource::release( resource );
				ErrorReturnMsg( false, "%s: Failed to create color texture 2d", __FUNCTION__ );
			}
			resource->size += GFX_SIZE_IMAGE_COLOR_BYTES( width, height, 1, desc.colorFormat ) * msaaSampleCount;
		}

		// Render Target View
		DECL_ZERO( D3D11_RENDER_TARGET_VIEW_DESC, rtvDesc );
		rtvDesc.Format = tDesc.Format;
		rtvDesc.ViewDimension = desc.sampleCount > 1 ?
			D3D11_RTV_DIMENSION_TEXTURE2DMS : D3D11_RTV_DIMENSION_TEXTURE2D;
		rtvDesc.Texture2D.MipSlice = 0;

		Assert( resource->rtv == nullptr );
		if( FAILED( device->CreateRenderTargetView( desc.sampleCount > 1 ?
			resource->textureColorMS : resource->textureColorSS,
			&rtvDesc, &resource->rtv ) ) )
		{
			GfxTextureResource::release( resourceColor );
			GfxRenderTargetResource::release( resource );
			ErrorReturnMsg( false, "%s: Failed to create color render target view", __FUNCTION__ );
		}

		// Shader Resource View
		DECL_ZERO( D3D11_SHADER_RESOURCE_VIEW_DESC, srvDesc );
		srvDesc.Format = D3D11ColorFormats[desc.colorFormat];
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;

		Assert( resourceColor->srv == nullptr );
		if( FAILED( device->CreateShaderResourceView( resource->textureColorSS, &srvDesc, &resourceColor->srv ) ) )
		{
			GfxTextureResource::release( resourceColor );
			GfxRenderTargetResource::release( resource );
			ErrorReturnMsg( false, "%s: Failed to create color shader resource view", __FUNCTION__ );
		}

		// CPU Access
		if( desc.cpuAccess )
		{
			DECL_ZERO( D3D11_TEXTURE2D_DESC, stagingDesc );
			resource->textureColorSS->GetDesc( &stagingDesc );
			stagingDesc.Usage = D3D11_USAGE_STAGING;
			stagingDesc.BindFlags = 0;
			stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
			stagingDesc.MiscFlags = 0;

			Assert( resource->textureColorStaging == nullptr );
			if( FAILED( device->CreateTexture2D( &stagingDesc, nullptr, &resource->textureColorStaging ) ) )
			{
				GfxTextureResource::release( resourceColor );
				GfxRenderTargetResource::release( resource );
				ErrorReturnMsg( false, "%s: Failed to create color CPU staging texture", __FUNCTION__ );
			}
			resource->size += GFX_SIZE_IMAGE_COLOR_BYTES( width, height, 1, desc.colorFormat );
		}
	}

	// Depth
	if( desc.depthFormat != GfxDepthFormat_NONE )
	{
		resourceDepth = textureResources.make_new();
		resourceDepth->type = GfxTextureType_2D;

		const bool hasMSAA = desc.sampleCount > 1;

		// Texture2D (Single-Sample)
		DECL_ZERO( D3D11_TEXTURE2D_DESC, tDesc );
		tDesc.Width = width;
		tDesc.Height = height;
		tDesc.MipLevels = 1;
		tDesc.ArraySize = 1;
		tDesc.Format = hasMSAA ? D3D11DepthStencilFormats[desc.depthFormat].srvFormat : // MSAA: Typed
			D3D11DepthStencilFormats[desc.depthFormat].texFormat; // Single-Sample: Untyped
		tDesc.SampleDesc.Count = 1;
		tDesc.SampleDesc.Quality = 0;
		tDesc.Usage = D3D11_USAGE_DEFAULT;
		tDesc.BindFlags = hasMSAA ? ( D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS ) : // MSAA: Resolver UAV
			( D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE ); // Single-Sample: SRV
		tDesc.CPUAccessFlags = 0;
		tDesc.MiscFlags = 0;

		Assert( resource->textureDepthSS == nullptr );
		if( FAILED( device->CreateTexture2D( &tDesc, nullptr, &resource->textureDepthSS ) ) )
		{
			GfxTextureResource::release( resourceColor );
			GfxTextureResource::release( resourceDepth );
			GfxRenderTargetResource::release( resource );
			ErrorReturnMsg( false, "%s: Failed to create depth texture 2d", __FUNCTION__ );
		}
		resource->size += GFX_SIZE_IMAGE_DEPTH_BYTES( width, height, 1, desc.depthFormat );

		// Texture2D (Multi-Sample)
		if( desc.sampleCount > 1 )
		{
			UINT msaaSampleCount = desc.sampleCount;
			UINT msaaQualityLevels = 0;
			HRESULT hr = device->CheckMultisampleQualityLevels(
				D3D11DepthStencilFormats[desc.depthFormat].texFormat, msaaSampleCount, &msaaQualityLevels );
			if( FAILED( hr ) || msaaQualityLevels == 0 ) { msaaSampleCount = 1; msaaQualityLevels = 1; }

			tDesc.Format = D3D11DepthStencilFormats[desc.depthFormat].texFormat;
			tDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
			tDesc.SampleDesc.Count = msaaSampleCount;
			tDesc.SampleDesc.Quality = msaaQualityLevels - 1;

			Assert( resource->textureDepthMS == nullptr );
			if( FAILED( device->CreateTexture2D( &tDesc, nullptr, &resource->textureDepthMS ) ) )
			{
				GfxTextureResource::release( resourceColor );
				GfxTextureResource::release( resourceDepth );
				GfxRenderTargetResource::release( resource );
				ErrorReturnMsg( false, "%s: Failed to create depth texture 2d", __FUNCTION__ );
			}
			resource->size += GFX_SIZE_IMAGE_DEPTH_BYTES( width, height, 1, desc.depthFormat ) * msaaSampleCount;
		}

		// Depth-Stencil View
		DECL_ZERO( D3D11_DEPTH_STENCIL_VIEW_DESC, dsvDesc );
		dsvDesc.Format = D3D11DepthStencilFormats[desc.depthFormat].dsvFormat;
		dsvDesc.ViewDimension = desc.sampleCount > 1 ?
			D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Flags = 0;
		dsvDesc.Texture2D.MipSlice = 0;

		Assert( resource->dsv == nullptr );
		if( FAILED( device->CreateDepthStencilView( desc.sampleCount > 1 ?
			resource->textureDepthMS : resource->textureDepthSS,
			&dsvDesc, &resource->dsv ) ) )
		{
			GfxTextureResource::release( resourceColor );
			GfxTextureResource::release( resourceDepth );
			GfxRenderTargetResource::release( resource );
			ErrorReturnMsg( false, "%s: Failed to create depth render target view", __FUNCTION__ );
		}

		// Shader Resource
		DECL_ZERO( D3D11_SHADER_RESOURCE_VIEW_DESC, srvDesc );
		srvDesc.Format = D3D11DepthStencilFormats[desc.depthFormat].srvFormat;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;

		Assert( resourceDepth->srv == nullptr );
		if( FAILED( device->CreateShaderResourceView( resource->textureDepthSS, &srvDesc, &resourceDepth->srv ) ) )
		{
			GfxTextureResource::release( resourceColor );
			GfxTextureResource::release( resourceDepth );
			GfxRenderTargetResource::release( resource );
			ErrorReturnMsg( false, "%s: Failed to create depth shader resource view", __FUNCTION__ );
		}

		// CPU Access
		if( desc.cpuAccess )
		{
			DECL_ZERO( D3D11_TEXTURE2D_DESC, stagingDesc );
			resource->textureDepthSS->GetDesc( &stagingDesc );
			stagingDesc.Usage = D3D11_USAGE_STAGING;
			stagingDesc.BindFlags = 0;
			stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
			stagingDesc.MiscFlags = 0;

			Assert( resource->textureDepthStaging == nullptr );
			if( FAILED( device->CreateTexture2D( &stagingDesc, nullptr, &resource->textureDepthStaging ) ) )
			{
				GfxTextureResource::release( resourceColor );
				GfxTextureResource::release( resourceDepth );
				GfxRenderTargetResource::release( resource );
				ErrorReturnMsg( false, "%s: Failed to create depth CPU staging texture", __FUNCTION__ );
			}
			resource->size += GFX_SIZE_IMAGE_DEPTH_BYTES( width, height, 1, desc.depthFormat );
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


bool CoreGfx::api_render_target_copy(
	GfxRenderTargetResource *&srcResource,
	GfxTextureResource *&srcResourceColor, GfxTextureResource *&srcResourceDepth,
	GfxRenderTargetResource *&dstResource,
	GfxTextureResource *&dstResourceColor, GfxTextureResource *&dstResourceDepth )
{
	if( srcResource == nullptr || dstResource == nullptr ) { return false; }
	if( srcResource->width != dstResource->width || srcResource->height != dstResource->height ) { return false; }

	if( srcResource->rtv && dstResource->rtv )
	{
		ID3D11Resource *srcD3D11ResourceColor;
		ID3D11Resource *dstD3D11ResourceColor;
		srcResourceColor->srv->GetResource( &srcD3D11ResourceColor );
		dstResourceColor->srv->GetResource( &dstD3D11ResourceColor );
		context->CopySubresourceRegion( dstD3D11ResourceColor, 0, 0, 0, 0, srcD3D11ResourceColor, 0, nullptr );
		srcD3D11ResourceColor->Release();
		dstD3D11ResourceColor->Release();
	}

	if( srcResource->dsv && dstResource->dsv )
	{
		ID3D11Resource *srcD3D11ResourceDepth;
		ID3D11Resource *dstD3D11ResourceDepth;
		srcResourceDepth->srv->GetResource( &srcD3D11ResourceDepth );
		dstResourceDepth->srv->GetResource( &dstD3D11ResourceDepth );
		context->CopySubresourceRegion( dstD3D11ResourceDepth, 0, 0, 0, 0, srcD3D11ResourceDepth, 0, nullptr );
		srcD3D11ResourceDepth->Release();
		dstD3D11ResourceDepth->Release();
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
	if( srcResource == nullptr || dstResource == nullptr ) { return false; }

	D3D11_BOX sourceRegion;
	sourceRegion.left = static_cast<UINT>( srcX );
	sourceRegion.right = static_cast<UINT>( srcX ) + static_cast<UINT>( width );
	sourceRegion.top = static_cast<UINT>( srcY );
	sourceRegion.bottom = static_cast<UINT>( srcY ) + static_cast<UINT>( height );
	sourceRegion.front = 0;
	sourceRegion.back = 1;

	if( srcResource->rtv && dstResource->rtv )
	{
		ID3D11Resource *srcD3D11ResourceColor;
		ID3D11Resource *dstD3D11ResourceColor;
		srcResourceColor->srv->GetResource( &srcD3D11ResourceColor );
		dstResourceColor->srv->GetResource( &dstD3D11ResourceColor );
		context->CopySubresourceRegion( dstD3D11ResourceColor, 0, static_cast<UINT>( dstX ), static_cast<UINT>( dstY ),
			0, srcD3D11ResourceColor, 0, &sourceRegion );
		srcD3D11ResourceColor->Release();
		dstD3D11ResourceColor->Release();
	}

#if 0
	if( srcResource->targetDepth && dstResource->targetDepth )
	{
		// No native API support for this, have to implement manually
		// See: GfxRenderTarget::copy_part()
	}
#endif

	return true;
}


bool CoreGfx::api_render_target_buffer_read_color( GfxRenderTargetResource *&resource,
	GfxTextureResource *&resourceColor,
	void *buffer, const u32 size )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( resource->textureColorStaging != nullptr );
	Assert( resourceColor != nullptr && resourceColor->id != GFX_RESOURCE_ID_NULL );

	ErrorIf( !resource->desc.cpuAccess,
		"Trying to CPU access a render target that does not have CPU access flag!" );

	const u32 sizeSource = GFX_SIZE_IMAGE_COLOR_BYTES( resource->width, resource->height, 1, resource->desc.colorFormat );
	Assert( size > 0 && sizeSource <= size );
	Assert( buffer != nullptr );

	// Copy Data to Staging Texture
	ID3D11Resource *srcResource;
	resourceColor->srv->GetResource( &srcResource );
	context->CopyResource( resource->textureColorStaging, srcResource );
	srcResource->Release();

	DECL_ZERO( D3D11_MAPPED_SUBRESOURCE, mappedResource );
	context->Map( resource->textureColorStaging, 0, D3D11_MAP_READ, 0, &mappedResource );
#if false
	// Flip the texture vertically
	const usize stride = resource->width * colorFormatPixelSizeBytes[resource->desc.colorFormat];
	byte *flipped = reinterpret_cast<byte *>( buffer );
	byte *source = static_cast<byte *>( mappedResource.pData );
	for( u16 y = 0; y < resource->height; y++ )
	{
		memory_copy( &flipped[y * stride], &source[( resource->height - 1 - y ) * stride], stride );
	}
#else
	memory_copy( buffer, mappedResource.pData, size );
#endif
	context->Unmap( resource->textureColorStaging, 0 );

	return true;
}


bool api_render_target_buffer_read_depth( GfxRenderTargetResource *&resource,
	GfxTextureResource *&resourceDepth,
	void *buffer, const u32 size )
{
	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING
	return true;
}


bool CoreGfx::api_render_target_buffer_write_color( GfxRenderTargetResource *&resource,
	GfxTextureResource *&resourceColor,
	const void *const buffer, const u32 size )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( resource->textureColorStaging != nullptr );
	Assert( resourceColor != nullptr && resourceColor->id != GFX_RESOURCE_ID_NULL );

	ErrorIf( !resource->desc.cpuAccess,
		"Trying to CPU access a render target that does not have CPU access flag!" );

	const u32 sizeSource = resource->width * resource->height *
		CoreGfx::colorFormatPixelSizeBytes[resource->desc.colorFormat];
	Assert( size > 0 && size <= sizeSource );
	Assert( buffer != nullptr );

	DECL_ZERO( D3D11_MAPPED_SUBRESOURCE, mappedResource );
	context->Map( resource->textureColorStaging, 0, D3D11_MAP_READ, 0, &mappedResource );
	memory_copy( mappedResource.pData, buffer, sizeSource );
	context->Unmap( resource->textureColorStaging, 0 );

	ID3D11Resource *dstResource;
	resourceColor->srv->GetResource( &dstResource );
	context->CopyResource( dstResource, resource->textureColorStaging );
	dstResource->Release();

	return true;
}


bool CoreGfx::api_render_target_buffer_write_depth( GfxRenderTargetResource *&resource,
	GfxTextureResource *&resourceDepth,
	const void *const buffer, const u32 size )
{
	// TODO: Implement this
	GRAPHICS_API_IMPLEMENTATION_WARNING
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Draw

static void d3d11_draw( const UINT vertexCount, const UINT vertexStartLocation )
{
	context->Draw( vertexCount, vertexStartLocation );
	PROFILE_GFX( Gfx::stats.frame.drawCalls++ );
	PROFILE_GFX( Gfx::stats.frame.vertexCount += vertexCount );
}


static void d3d11_draw_instanced( const UINT vertexCount, const UINT instanceCount,
	const UINT vertexStartLocation, const UINT instanceStartLocation )
{
	context->DrawInstanced( vertexCount, instanceCount, vertexStartLocation, instanceStartLocation );
	PROFILE_GFX( Gfx::stats.frame.drawCalls++ );
	PROFILE_GFX( Gfx::stats.frame.vertexCount += vertexCount * instanceCount );
}


static void d3d11_draw_indexed( const UINT vertexCount, const UINT vertexStartLocation, const INT baseVertexLocation )
{
	context->DrawIndexed( vertexCount, vertexStartLocation, baseVertexLocation );
	PROFILE_GFX( Gfx::stats.frame.drawCalls++ );
	PROFILE_GFX( Gfx::stats.frame.vertexCount += vertexCount );
}


static void d3d11_draw_instanced_indexed( const UINT vertexCount, const UINT instanceCount,
	const UINT vertexStartLocation, const UINT instanceStartLocation, const INT baseVertexLocation )
{
	context->DrawIndexedInstanced( vertexCount, instanceCount, vertexStartLocation,
		baseVertexLocation, instanceStartLocation );
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

	// Set Vertex Topology
	Assert( type < GFXPRIMITIVETYPE_COUNT );
	context->IASetPrimitiveTopology( D3D11PrimitiveTypes[type] );

	// Set Vertex & Instance Buffers
	ID3D11Buffer *buffers[2] = { nullptr, nullptr };
	UINT strides[2] = { 0, 0 };
	UINT offsets[2] = { 0, 0 };
	UINT slot = 0;
	UINT count = 0;

	if( resourceVertex != nullptr )
	{
		buffers[count] = resourceVertex->buffer;
		strides[count] = resourceVertex->stride;
		offsets[count] = resourceVertex->offset;
		count++;
	}

	if( resourceInstance != nullptr )
	{
		if( resourceVertex == nullptr ) { slot = 1; }
		buffers[count] = resourceInstance->buffer;
		strides[count] = resourceInstance->stride;
		offsets[count] = resourceInstance->offset;
		count++;
	}

	if( count > 0 )
	{
		context->IASetVertexBuffers( slot, count, buffers, strides, offsets );
	}

	// Submit Index Buffer
	if( resourceIndex )
	{
		context->IASetIndexBuffer( resourceIndex->buffer, D3D11IndexBufferFormats[resourceIndex->format], 0 );
	}

	// Submit Draw
	if( resourceIndex == nullptr )
	{
		if( instanceCount > 0 )
		{
			d3d11_draw_instanced( static_cast<UINT>( vertexCount ), static_cast<UINT>( instanceCount ), 0, 0 );
		}
		else
		{
			d3d11_draw( static_cast<UINT>( vertexCount ), 0 );
		}
	}
	else
	{
		const UINT count = static_cast<UINT>( vertexCount * resourceIndex->indicesToVerticesRatio );

		if( instanceCount > 0 )
		{
			d3d11_draw_instanced_indexed( count, static_cast<UINT>( instanceCount ), 0, 0, 0 );
		}
		else
		{
			d3d11_draw_indexed( count, static_cast<UINT>( instanceCount ), 0 );
		}
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Dispatch

bool CoreGfx::api_dispatch( const u32 x, const u32 y, const u32 z )
{
	GRAPHICS_API_IMPLEMENTATION_WARNING
	//context->Dispatch( static_cast<UINT>( x ), static_cast<UINT>( y ), static_cast<UINT>( z ) );
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
#if COMPILE_DEBUG
	if( pass.name[0] != '\0' )
	{
		wchar_t wname[PATH_SIZE];
		int len = MultiByteToWideChar( CP_UTF8, 0, pass.name, -1, wname, PATH_SIZE );
		if( len > 0 ) { wname[len - 1] = L'\0'; }
		annotation->BeginEvent( wname );
	}
#endif

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