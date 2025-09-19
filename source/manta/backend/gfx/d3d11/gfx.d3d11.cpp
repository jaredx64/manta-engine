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

static GUID D3D11_IID_IDXGIFactory { 0x7B7166EC, 0x21C7, 0x44AE, { 0xB2, 0x1A, 0xC9, 0xAE, 0x32, 0x1A, 0xE3, 0x69 } };
static GUID D3D11_IID_ID3D11Texture2D { 0x6F15AAF2, 0xD208, 0x4E89, { 0x9A, 0xB4, 0x48, 0x95, 0x35, 0xD3, 0x4F, 0x9C } };

#define MSAA_SAMPLE_COUNT 4
#define MSAA_QUALITY_LEVEL 0

#if 1
#define DECL_ZERO(type, name) type name; memory_set( &name, 0, sizeof( type ) )
#else
#define DECL_ZERO(type, name) type name
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static ID3D11Device *device = nullptr;

static ID3D11DeviceContext *context = nullptr;

static IDXGIFactory *factory = nullptr;

static IDXGISwapChain *swapChain = nullptr;
static UINT swapChainFlagsCreate;
static UINT swapChainFlagsPresent;

#define GFX_RENDER_TARGET_SLOT_COUNT ( 8 )
static ID3D11RenderTargetView *RT_TEXTURE_COLOR[GFX_RENDER_TARGET_SLOT_COUNT];
static ID3D11DepthStencilView *RT_TEXTURE_DEPTH;

static ID3D11RenderTargetView *RT_TEXTURE_COLOR_CACHE; // HACK: change this?
static ID3D11DepthStencilView *RT_TEXTURE_DEPTH_CACHE; // HACK: change this?

#define GFX_TEXTURE_SLOT_COUNT ( 8 )
static ID3D11ShaderResourceView *textureSlots[GFX_TEXTURE_SLOT_COUNT];

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
static_assert( ARRAY_LENGTH( D3D11ColorFormats ) == GFXCOLORFORMAT_COUNT, "Missing GfxColorFormat!" );


struct D3D11DepthStencilFormat
{
	D3D11DepthStencilFormat( const DXGI_FORMAT t2DFormat, const DXGI_FORMAT dsvFormat, const DXGI_FORMAT srvFormat ) :
		texture2DDescFormat{ t2DFormat },
		depthStencilViewDescFormat{ dsvFormat },
		shaderResourceViewDescFormat{ srvFormat }{ }

	DXGI_FORMAT texture2DDescFormat, depthStencilViewDescFormat, shaderResourceViewDescFormat;
};

static const D3D11DepthStencilFormat D3D11DepthStencilFormats[] =
{
	{ DXGI_FORMAT_UNKNOWN,        DXGI_FORMAT_UNKNOWN,           DXGI_FORMAT_UNKNOWN },               // GfxDepthFormat_NONE
	{ DXGI_FORMAT_R16_TYPELESS,   DXGI_FORMAT_D16_UNORM,         DXGI_FORMAT_R16_UNORM },             // GfxDepthFormat_R16_FLOAT
	{ DXGI_FORMAT_R16_TYPELESS,   DXGI_FORMAT_D16_UNORM,         DXGI_FORMAT_R16_UNORM },             // GfxDepthFormat_R16_UINT
	{ DXGI_FORMAT_R32_TYPELESS,   DXGI_FORMAT_D32_FLOAT,         DXGI_FORMAT_R32_FLOAT },             // GfxDepthFormat_R32_FLOAT
	{ DXGI_FORMAT_R32_TYPELESS,   DXGI_FORMAT_D32_FLOAT,         DXGI_FORMAT_R32_UINT },              // GfxDepthFormat_R32_UINT
	{ DXGI_FORMAT_R24G8_TYPELESS, DXGI_FORMAT_D24_UNORM_S8_UINT, DXGI_FORMAT_R24_UNORM_X8_TYPELESS }, // GfxDepthFormat_R24_UINT_G8_UINT
};
static_assert( ARRAY_LENGTH( D3D11DepthStencilFormats ) == GFXDEPTHFORMAT_COUNT, "Missing GfxDepthFormat!" );


static const D3D11_FILL_MODE D3D11FillModes[] =
{
	D3D11_FILL_SOLID,     // GfxFillMode_SOLID
	D3D11_FILL_WIREFRAME, // GfxFillMode_WIREFRAME
};
static_assert( ARRAY_LENGTH( D3D11FillModes ) == GFXFILLMODE_COUNT, "Missing GfxFillMode!" );


static const D3D11_CULL_MODE D3D11CullModes[] =
{
	D3D11_CULL_NONE,  // GfxCullMode_NONE
	D3D11_CULL_FRONT, // GfxCullMode_FRONT
	D3D11_CULL_BACK,  // GfxCullMode_BACK
};
static_assert( ARRAY_LENGTH( D3D11CullModes ) == GFXCULLMODE_COUNT, "Missing GfxCullMode!" );


static const D3D11_FILTER D3D11FilteringModes[] =
{
	D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR, // GfxFilteringMode_NEAREST
	D3D11_FILTER_MIN_MAG_MIP_LINEAR,       // GfxFilteringMode_LINEAR
	D3D11_FILTER_ANISOTROPIC,              // GfxFilteringMode_ANISOTROPIC
};
static_assert( ARRAY_LENGTH( D3D11FilteringModes ) == GFXFILTERINGMODE_COUNT, "Missing GfxFilteringMode!" );


static const D3D11_TEXTURE_ADDRESS_MODE D3D11UVWrapModes[] =
{
	D3D11_TEXTURE_ADDRESS_WRAP,   // GfxUVWrapMode_WRAP
	D3D11_TEXTURE_ADDRESS_MIRROR, // GfxUVWrapMode_MIRROR
	D3D11_TEXTURE_ADDRESS_CLAMP,  // GfxUVWrapMode_CLAMP
};
static_assert( ARRAY_LENGTH( D3D11UVWrapModes ) == GFXUVWRAPMODE_COUNT, "Missing GfxUVWrapMode!" );


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
static_assert( ARRAY_LENGTH( D3D11BlendFactors ) == GFXBLENDFACTOR_COUNT, "Missing GfxBlendFactor!" );


static const D3D11_BLEND_OP D3D11BlendOperations[] =
{
	D3D11_BLEND_OP_ADD,          // GfxBlendOperation_ADD
	D3D11_BLEND_OP_SUBTRACT,     // GfxBlendOperation_SUBTRACT
	D3D11_BLEND_OP_REV_SUBTRACT, // GfxBlendOperation_REV_SUBTRACT
	D3D11_BLEND_OP_MIN,          // GfxBlendOperation_MIN
	D3D11_BLEND_OP_MAX,          // GfxBlendOperation_MAX
};
static_assert( ARRAY_LENGTH( D3D11BlendOperations ) == GFXBLENDOPERATION_COUNT, "Missing GfxBlendOperation!" );


static const D3D11_DEPTH_WRITE_MASK D3D11DepthWriteMasks[] =
{
	D3D11_DEPTH_WRITE_MASK_ZERO, // GfxDepthWriteFlag_NONE
	D3D11_DEPTH_WRITE_MASK_ALL,  // GfxDepthWriteFlag_ALL
};
static_assert( ARRAY_LENGTH( D3D11DepthWriteMasks ) == GFXDEPTHWRITEFLAG_COUNT, "Missing GfxDepthWriteFlag!" );


static const D3D11_COMPARISON_FUNC D3D11DepthTestModes[] =
{
	D3D11_COMPARISON_ALWAYS,        // GfxDepthTestMode_NONE
	D3D11_COMPARISON_LESS,          // GfxDepthTestMode_LESS
	D3D11_COMPARISON_LESS_EQUAL,    // GfxDepthTestMode_LESS_EQUALS
	D3D11_COMPARISON_GREATER,       // GfxDepthTestMode_GREATER
	D3D11_COMPARISON_GREATER_EQUAL, // GfxDepthTestMode_GREATER_EQUALS
	D3D11_COMPARISON_EQUAL,         // GfxDepthTestMode_EQUAL
	D3D11_COMPARISON_NOT_EQUAL,     // GfxDepthTestMode_NOT_EQUAL
	D3D11_COMPARISON_ALWAYS,        // GfxDepthTestMode_ALWAYS
};
static_assert( ARRAY_LENGTH( D3D11DepthTestModes ) == GFXDEPTHTESTMODE_COUNT, "Missing GfxDepthTestMode!" );


static const DXGI_FORMAT D3D11IndexBufferFormats[] =
{
	DXGI_FORMAT_UNKNOWN,  // GfxIndexBufferFormat_NONE
	DXGI_FORMAT_R8_UINT,  // GfxIndexBufferFormat_U8
	DXGI_FORMAT_R16_UINT, // GfxIndexBufferFormat_U16
	DXGI_FORMAT_R32_UINT, // GfxIndexBufferFormat_U32
};
static_assert( ARRAY_LENGTH( D3D11IndexBufferFormats ) == GFXINDEXBUFFERFORMAT_COUNT, "Missing GfxIndexBufferFormat!" );


struct D3D11MapAccessMode
{
	D3D11MapAccessMode( const UINT cpuAccessFlag, const D3D11_MAP d3d11Map, const D3D11_USAGE d3d11Usage ) :
		cpuAccessFlag{ cpuAccessFlag }, d3d11Map{ d3d11Map }, d3d11Usage{ d3d11Usage } { }

	UINT cpuAccessFlag;
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct GfxVertexBufferResource : public GfxResource
{
	static void release( GfxVertexBufferResource *&resource );
	ID3D11Buffer *buffer = nullptr;
	GfxCPUAccessMode accessMode;
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
	double indToVertRatio = 1.0;
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


struct GfxTexture2DResource : public GfxResource
{
	static void release( GfxTexture2DResource *&resource );
	ID3D11ShaderResourceView *view = nullptr;
	GfxColorFormat colorFormat;
	u32 width = 0;
	u32 height = 0;
	u16 levels = 0;
	usize size = 0;
};


struct GfxRenderTarget2DResource : public GfxResource
{
	static void release( GfxRenderTarget2DResource *&resource );
	ID3D11RenderTargetView *targetColor = nullptr;
	ID3D11DepthStencilView *targetDepth = nullptr;
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


struct GfxShaderResource : public GfxResource
{
	static void release( GfxShaderResource *&resource );
	ID3D11VertexShader *vs = nullptr;
	ID3D11PixelShader *ps = nullptr;
	ID3D11InputLayout *il = nullptr; // TODO: Generate these from shader code?
	UINT sizeVS = 0;
	UINT sizePS = 0;
	UINT sizeCS = 0;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static GfxResourceFactory<GfxVertexBufferResource, GFX_RESOURCE_COUNT_VERTEX_BUFFER> vertexBufferResources;
static GfxResourceFactory<GfxIndexBufferResource, GFX_RESOURCE_COUNT_INDEX_BUFFER> indexBufferResources;
static GfxResourceFactory<GfxUniformBufferResource, GFX_RESOURCE_COUNT_CONSTANT_BUFFER> uniformBufferResources;
static GfxResourceFactory<GfxShaderResource, GFX_RESOURCE_COUNT_SHADER> shaderResources;
static GfxResourceFactory<GfxTexture2DResource, GFX_RESOURCE_COUNT_TEXTURE_2D> texture2DResources;
static GfxResourceFactory<GfxRenderTarget2DResource, GFX_RESOURCE_COUNT_RENDER_TARGET_2D> renderTarget2DResources;


static bool resources_init()
{
	vertexBufferResources.init();
	indexBufferResources.init();
	uniformBufferResources.init();
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
	uniformBufferResources.free();
	texture2DResources.free();
	renderTarget2DResources.free();
	shaderResources.free();

	// Success
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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


static void msaa_resolver_color( ID3D11Texture2D *textureColorMS, ID3D11Texture2D *textureColorSS,
	GfxColorFormat format )
{
	if( format == GfxColorFormat_NONE ) { return; }
    if( !textureColorMS || !textureColorSS ) { return; }
    context->ResolveSubresource( textureColorSS, 0, textureColorMS, 0, D3D11ColorFormats[format] );
}


static void msaa_resolver_depth( ID3D11Texture2D *textureDepthMS, ID3D11Texture2D *textureDepthSS,
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
		return;

		// TODO
	#if 0
        // SRV for depth
		DECL_ZERO( D3D11_SHADER_RESOURCE_VIEW_DESC, srvDepth );
        srvDepth.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
        srvDepth.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
        ID3D11ShaderResourceView *depthSRV = nullptr;
        device->CreateShaderResourceView( textureDepthMS, &srvDepth, &depthSRV );

        // SRV for stencil
        DECL_ZERO( D3D11_SHADER_RESOURCE_VIEW_DESC, srvStencil );
        srvStencil.Format = DXGI_FORMAT_X24_TYPELESS_G8_UINT;
        srvStencil.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
        ID3D11ShaderResourceView *stencilSRV = nullptr;
        device->CreateShaderResourceView( textureDepthMS, &srvStencil, &stencilSRV );

        // UAVs
        DECL_ZERO( D3D11_UNORDERED_ACCESS_VIEW_DESC, uavDepth );
        uavDepth.Format = DXGI_FORMAT_R32_FLOAT;
        uavDepth.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
        uavDepth.Texture2D.MipSlice = 0;
        ID3D11UnorderedAccessView *depthUAV = nullptr;
        device->CreateUnorderedAccessView( textureDepthSS, &uavDepth, &depthUAV );

        DECL_ZERO( D3D11_UNORDERED_ACCESS_VIEW_DESC, uavStencil );
        uavStencil.Format = DXGI_FORMAT_R8_UINT;
        uavStencil.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
        uavStencil.Texture2D.MipSlice = 0;
        ID3D11UnorderedAccessView *stencilUAV = nullptr;
        device->CreateUnorderedAccessView( textureStencilSS, &uavStencil, &stencilUAV );

        // Bind and dispatch
        context->CSSetShader( MSAA_RESOLVER_CS_DEPTH_STENCIL, nullptr, 0 );
        ID3D11ShaderResourceView *srvs[] = { depthSRV, stencilSRV };
        context->CSSetShaderResources( 0, 2, srvs );
        ID3D11UnorderedAccessView *uavs[] = { depthUAV, stencilUAV };
        context->CSSetUnorderedAccessViews( 0, 2, uavs, nullptr );

        context->Dispatch( groupsX, groupsY, 1 );

        // Unbind
        ID3D11ShaderResourceView *nullSRVs[] = { nullptr, nullptr };
        context->CSSetShaderResources( 0, 2, nullSRVs );
        ID3D11UnorderedAccessView *nullUAVs[] = { nullptr, nullptr };
        context->CSSetUnorderedAccessViews( 0, 2, nullUAVs, nullptr );
        context->CSSetShader( nullptr, nullptr, 0 );

        depthSRV->Release();
        stencilSRV->Release();
        depthUAV->Release();
        stencilUAV->Release();
	#endif
	}
	// Depth
	{
        // SRV
        DECL_ZERO( D3D11_SHADER_RESOURCE_VIEW_DESC, srvDesc );
        srvDesc.Format = D3D11DepthStencilFormats[format].shaderResourceViewDescFormat;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
        ID3D11ShaderResourceView *depthSRV = nullptr;
        if( FAILED( device->CreateShaderResourceView( textureDepthMS, &srvDesc, &depthSRV ) ) )
		{
			return;
		}

		// UAV
        DECL_ZERO( D3D11_UNORDERED_ACCESS_VIEW_DESC, uavDesc );
        uavDesc.Format = D3D11DepthStencilFormats[format].shaderResourceViewDescFormat;
        uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
        uavDesc.Texture2D.MipSlice = 0;
        ID3D11UnorderedAccessView *depthUAV = nullptr;
      	if( FAILED( device->CreateUnorderedAccessView( textureDepthSS, &uavDesc, &depthUAV ) ) )
		{
			depthSRV->Release();
			return;
		}

		// Dispatch CS
        context->CSSetShader( MSAA_RESOLVER_CS_DEPTH, nullptr, 0 );
        context->CSSetShaderResources( 0, 1, &depthSRV );
        context->CSSetUnorderedAccessViews( 0, 1, &depthUAV, nullptr );
        context->Dispatch( groupsX, groupsY, 1 );

        // Unbind
        ID3D11ShaderResourceView *nullSRV[] = { nullptr };
        context->CSSetShaderResources( 0, 1, nullSRV );
        ID3D11UnorderedAccessView *nullUAV[] = { nullptr };
        context->CSSetUnorderedAccessViews( 0, 1, nullUAV, nullptr );
        context->CSSetShader( nullptr, nullptr, 0 );

        depthSRV->Release();
        depthUAV->Release();
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool d3d11_device_init()
{
	// Feature Levels
	D3D_FEATURE_LEVEL featureLevel;
	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
	};
	UINT numFeatureLevels = sizeof( featureLevels ) / sizeof( featureLevels[0] );

	// Flags
#if COMPILE_DEBUG
	UINT flags = 0; // D3D11_CREATE_DEVICE_SINGLETHREADED | D3D11_CREATE_DEVICE_DEBUG
#else
	UINT flags = 0; // D3D11_CREATE_DEVICE_SINGLETHREADED;
#endif

	// Device
	if( FAILED( D3D11CreateDevice( nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, featureLevels, numFeatureLevels,
	                               D3D11_SDK_VERSION, &device, &featureLevel, &context ) ) )
	{
		ErrorReturnMsg( false, "%s: Failed to create device", __FUNCTION__ );
	}

	// Success
	return true;
}


static bool d3d11_device_free()
{
	device->Release();
	return true;
}


static bool d3d11_factory_init()
{
	// Factory
	if( FAILED( CreateDXGIFactory( D3D11_IID_IDXGIFactory, reinterpret_cast<void **>( &factory ) ) ) )
	{
		ErrorReturnMsg( false, "%s: Failed to create DXGI factory", __FUNCTION__ );
	}

	// Success
	return true;
}


static bool d3d11_factory_free()
{
	factory->Release();
	return true;
}


static void d3d11_draw( const UINT vertexCount, const UINT startVertexLocation )
{
	SysGfx::state_apply();
	context->Draw( vertexCount, startVertexLocation );
	PROFILE_GFX( Gfx::stats.frame.drawCalls++ );
	PROFILE_GFX( Gfx::stats.frame.vertexCount += vertexCount );
}


static void d3d11_draw_indexed( const UINT vertexCount, const UINT startVertexLocation, const INT baseVertexLocation )
{
	SysGfx::state_apply();
	context->DrawIndexed( vertexCount, startVertexLocation, baseVertexLocation );
	PROFILE_GFX( Gfx::stats.frame.drawCalls++ );
	PROFILE_GFX( Gfx::stats.frame.vertexCount += vertexCount );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GfxCore::rb_init()
{
	// GfxState
	GfxState &state = Gfx::state();
	const u16 w = Window::width;
	const u16 h = Window::height;
	const u16 f = Window::fullscreen;

	// Internal
	for( int i = 0; i < GFX_TEXTURE_SLOT_COUNT; i++ ) { textureSlots[i] = nullptr; }
	for( int i = 0; i < GFX_RENDER_TARGET_SLOT_COUNT; i++ ) { RT_TEXTURE_COLOR[i] = nullptr; };
	RT_TEXTURE_DEPTH = nullptr;

	// Resources
	ErrorReturnIf( !resources_init(), false, "%s: Failed to create gfx resources", __FUNCTION__ );

	// D3D11 Device
	ErrorReturnIf( !d3d11_device_init(), false, "%s: Failed to create D3D11 device", __FUNCTION__ );

	// D3D11 Factory
	ErrorReturnIf( !d3d11_factory_init(), false, "%s: Failed to create D3D11 factory", __FUNCTION__ );

	// Swapchain
	ErrorReturnIf( !GfxCore::rb_swapchain_init( w, h, f ), false, "%s: Failed to create swap chain", __FUNCTION__ );

	// MSAA Resolver
	ErrorReturnIf( !msaa_resolver_init(), false, "%s: Failed to init MSAA resolver", __FUNCTION__ );

	// Success
	return true;
}


bool GfxCore::rb_free()
{
	// MSAA Resolver
	msaa_resolver_free();

	// Device
	d3d11_device_free();

	// Factory
	d3d11_factory_free();

	// Resources
	resources_free();

	// Success
	return true;
}


void GfxCore::rb_frame_begin()
{
	// Reset Render Targets
#if DEPTH_BUFFER_ENABLED
	context->OMSetRenderTargets( 1, &RT_TEXTURE_COLOR[0], RT_TEXTURE_DEPTH );
#else
	context->OMSetRenderTargets( 1, &RT_TEXTURE_COLOR[0], nullptr );
#endif
}


void GfxCore::rb_frame_end()
{
	// Swap Buffers
	const bool swap = SUCCEEDED( swapChain->Present( 0, swapChainFlagsPresent ) );
	Assert( swap );
}


void GfxCore::rb_clear_color( const Color color )
{
	FLOAT rgba[4];
	static constexpr float INV_255 = 1.0f / 255.0f;
	rgba[0] = color.r * INV_255;
	rgba[1] = color.g * INV_255;
	rgba[2] = color.b * INV_255;
	rgba[3] = color.a * INV_255;

	for( int i = 0; i < GFX_RENDER_TARGET_SLOT_COUNT; i++ )
	{
		if( RT_TEXTURE_COLOR[i] == nullptr ) { continue; }
		context->ClearRenderTargetView( RT_TEXTURE_COLOR[i], rgba );
	}
}


void GfxCore::rb_clear_depth( const float depth )
{
	if( RT_TEXTURE_DEPTH == nullptr ) { return; }
	context->ClearDepthStencilView( RT_TEXTURE_DEPTH, D3D11_CLEAR_DEPTH, depth, 0 );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GfxCore::rb_swapchain_init( const u16 width, const u16 height, const bool fullscreen )
{
	// Cache State
	GfxSwapChain &swapChainCache = GfxCore::swapchain;
	swapChainCache.width = width;
	swapChainCache.height = height;
	swapChainCache.fullscreen = fullscreen;

	// Setup Swap Chain Description
	DECL_ZERO( DXGI_SWAP_CHAIN_DESC, swapChainDesc );
	swapChainDesc.BufferDesc.Width = width;
	swapChainDesc.BufferDesc.Height = height;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferDesc.Format = D3D11ColorFormats[GfxColorFormat_R8G8B8A8_FLOAT];
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.OutputWindow = SysWindow::handle;
	swapChainDesc.Windowed = true;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

	// Create Swap Chain
	Assert( swapChain == nullptr );
	PROFILE_GFX( Gfx::stats.gpuMemorySwapchain =
		GFX_SIZE_IMAGE_COLOR_BYTES( width, height, 1, GfxColorFormat_R8G8B8A8_FLOAT ) * 2 );

	if( FAILED( factory->CreateSwapChain( device, &swapChainDesc, &swapChain ) ) )
	{
		// If we fail to create a swap chain that should be compatible with Windows 10,
		// we'll falll back to a windows 8.1 compatible one...
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;

		if( FAILED( factory->CreateSwapChain( device, &swapChainDesc, &swapChain ) ) )
		{
			// If we fail to create a swap chain that should be compatible with Windows 8.1,
			// we'll fall back to a standard compatible one...
			swapChainDesc.BufferCount = 1;
			swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
			swapChainDesc.Flags = 0;

			if( FAILED( factory->CreateSwapChain( device, &swapChainDesc, &swapChain ) ) )
			{
				// We were unable to make any swap chain... this is an error
				ErrorReturnMsg( false, "%s: Failed to create swap chain", __FUNCTION__ );
			}
		}
	}

	// Cache Flags
	swapChainFlagsCreate = swapChainDesc.Flags;
	swapChainFlagsPresent = swapChainDesc.Flags == DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING ? DXGI_PRESENT_ALLOW_TEARING : 0;

	// Disable Alt + Enter (TODO: Move somewhere else?)
	IDXGIFactory *parent;
	if( SUCCEEDED( swapChain->GetParent( D3D11_IID_IDXGIFactory, reinterpret_cast<void **>( &parent ) ) ) )
	{
		parent->MakeWindowAssociation( SysWindow::handle, DXGI_MWA_NO_ALT_ENTER );
		parent->Release();
	}

	// Swapchain Render Targets
	ID3D11Texture2D *buffer;
	{
		// Get Swapchain Back Buffer Pointer
		if( FAILED( swapChain->GetBuffer( 0, D3D11_IID_ID3D11Texture2D, reinterpret_cast<void **>( &buffer ) ) ) )
		{
			ErrorReturnMsg( false, "%s: Failed to get back buffer pointer", __FUNCTION__ );
		}

		// Create Render Target View
		Assert( RT_TEXTURE_COLOR[0] == nullptr );
		if( FAILED( device->CreateRenderTargetView( buffer, nullptr, &RT_TEXTURE_COLOR[0] ) ) )
		{
			ErrorReturnMsg( false, "%s: Failed to create render target view", __FUNCTION__ );
		}

		// TODO: Depth buffer on swap chain
		// ...
	}
	buffer->Release();

	// Depth Buffer
	#if DEPTH_BUFFER_ENABLED
	{
		DECL_ZERO( D3D11_TEXTURE2D_DESC, tDesc );
		tDesc.Width = width;
		tDesc.Height = height;
		tDesc.MipLevels = 1;
		tDesc.ArraySize = 1;
		tDesc.Format = D3D11DepthStencilFormats[DEPTH_BUFFER_FORMAT].texture2DDescFormat; // TODO
		tDesc.SampleDesc.Count = 1;
		tDesc.SampleDesc.Quality = 0;
		tDesc.Usage = D3D11_USAGE_DEFAULT;
		tDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		tDesc.CPUAccessFlags = 0;
		tDesc.MiscFlags = 0;

		ID3D11Texture2D *textureDepth = nullptr;
		PROFILE_GFX( Gfx::stats.gpuMemorySwapchain +=
			GFX_SIZE_IMAGE_DEPTH_BYTES( width, height, 1, DEPTH_BUFFER_FORMAT ) * 2 );
		ErrorReturnIf( FAILED( device->CreateTexture2D( &tDesc, nullptr, &textureDepth ) ), false,
						"%s: Failed to create depth texture 2d", __FUNCTION__ );

		// Depth-Stencil View
		DECL_ZERO( D3D11_DEPTH_STENCIL_VIEW_DESC, dsvDesc );
		dsvDesc.Format = D3D11DepthStencilFormats[DEPTH_BUFFER_FORMAT].depthStencilViewDescFormat;
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Flags = 0;
		dsvDesc.Texture2D.MipSlice = 0;

		ErrorReturnIf( FAILED( device->CreateDepthStencilView( textureDepth, &dsvDesc, &RT_TEXTURE_DEPTH ) ), false,
						"%s: Failed to create depth render target view", __FUNCTION__ );
	}
	#endif

	// Success
	return true;
}


bool GfxCore::rb_swapchain_free()
{
	Assert( swapChain != nullptr );

	swapChain->Release();
	swapChain = nullptr;

	return true;
}


bool GfxCore::rb_swapchain_resize( u16 width, u16 height, bool fullscreen )
{
	// Cache State
	GfxSwapChain &swapChainCache = GfxCore::swapchain;
	swapChainCache.width = width;
	swapChainCache.height = height;
	swapChainCache.fullscreen = fullscreen;

	// Unbind Render Target
	context->OMSetRenderTargets( 0, nullptr, nullptr );

	// Release Render Target
	RT_TEXTURE_COLOR[0]->Release();

	// Resize Buffers
	PROFILE_GFX( Gfx::stats.gpuMemorySwapchain =
		GFX_SIZE_IMAGE_COLOR_BYTES( width, height, 1, GfxColorFormat_R8G8B8A8_FLOAT ) * 2 );
	if( FAILED( swapChain->ResizeBuffers( 0, width, height, DXGI_FORMAT_UNKNOWN, swapChainFlagsCreate ) ) )
	{
		Error( "%s: Failed to resize swap chain buffers", __FUNCTION__ );
	}

	// Rebuild Render Target & Depth Buffer
	ID3D11Texture2D *buffer;
	{
		// Get Back Buffer Pointer
		if( FAILED( swapChain->GetBuffer( 0, D3D11_IID_ID3D11Texture2D, reinterpret_cast<void **>( &buffer ) ) ) )
		{
			Error( "%s: Failed to get swap chain buffer", __FUNCTION__ );
		}

		// Create Render Target
		if( FAILED( device->CreateRenderTargetView( buffer, nullptr, &RT_TEXTURE_COLOR[0] ) ) )
		{
			Error( "%s: Failed to create device render target view", __FUNCTION__ );
		}

		// Bind Render Target
		context->OMSetRenderTargets( 1, &RT_TEXTURE_COLOR[0], nullptr );

		// TODO: Depth buffer
		// ...
	}
	buffer->Release();

	// Depth Buffer
	#if DEPTH_BUFFER_ENABLED
	{
		RT_TEXTURE_DEPTH->Release();

		DECL_ZERO( D3D11_TEXTURE2D_DESC, tDesc );
		tDesc.Width = width;
		tDesc.Height = height;
		tDesc.MipLevels = 1;
		tDesc.ArraySize = 1;
		tDesc.Format = D3D11DepthStencilFormats[DEPTH_BUFFER_FORMAT].texture2DDescFormat; // TODO
		tDesc.SampleDesc.Count = 1;
		tDesc.SampleDesc.Quality = 0;
		tDesc.Usage = D3D11_USAGE_DEFAULT;
		tDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		tDesc.CPUAccessFlags = 0;
		tDesc.MiscFlags = 0;

		ID3D11Texture2D *textureDepth = nullptr;
		PROFILE_GFX( Gfx::stats.gpuMemorySwapchain +=
			GFX_SIZE_IMAGE_DEPTH_BYTES( width, height, 1, DEPTH_BUFFER_FORMAT ) * 2 );
		ErrorReturnIf( FAILED( device->CreateTexture2D( &tDesc, nullptr, &textureDepth ) ), false,
						"%s: Failed to create depth texture 2d", __FUNCTION__ );

		// Depth-Stencil View
		DECL_ZERO( D3D11_DEPTH_STENCIL_VIEW_DESC, dsvDesc );
		dsvDesc.Format = D3D11DepthStencilFormats[DEPTH_BUFFER_FORMAT].depthStencilViewDescFormat;
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Flags = 0;
		dsvDesc.Texture2D.MipSlice = 0;

		ErrorReturnIf( FAILED( device->CreateDepthStencilView( textureDepth, &dsvDesc, &RT_TEXTURE_DEPTH ) ), false,
						"%s: Failed to create depth render target view", __FUNCTION__ );
	}
	#endif

	// Update Viewport
	GfxCore::rb_viewport_resize( width, height, fullscreen );

	// Success
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GfxCore::rb_viewport_init( const u16 width, const u16 height, const bool fullscreen )
{
	// TODO: Multiple viewports
	GfxViewport &viewport = GfxCore::viewport;
	viewport.width = width;
	viewport.height = height;
	viewport.fullscreen = fullscreen;

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


bool GfxCore::rb_viewport_free()
{
	// TODO
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
	// State Description
	DECL_ZERO( D3D11_RASTERIZER_DESC, rsDesc );
	Assert( state.fillMode < GFXFILLMODE_COUNT );
	rsDesc.FillMode = D3D11FillModes[state.fillMode];
	Assert( state.cullMode < GFXCULLMODE_COUNT );
	rsDesc.CullMode = D3D11CullModes[state.cullMode];
	rsDesc.FrontCounterClockwise = false;
	rsDesc.DepthBias = 0;
	rsDesc.DepthBiasClamp = 0.0f;
	rsDesc.SlopeScaledDepthBias = 0.0f;
	rsDesc.DepthClipEnable = false;
	rsDesc.MultisampleEnable = false;
	rsDesc.AntialiasedLineEnable = false;
	rsDesc.ScissorEnable = state.scissor;

	// Create State
	ID3D11RasterizerState *rasterizerState = nullptr;
	if( FAILED( device->CreateRasterizerState( &rsDesc, &rasterizerState ) ) )
	{
		ErrorReturnMsg( false, "%s: Failed to create rasterizer state", __FUNCTION__ );
	}
	context->RSSetState( rasterizerState );

	// Scissor
	if( state.scissor )
	{
		D3D11_RECT rect;
		rect.left = state.scissorX1;
		rect.top = state.scissorY1;
		rect.right = state.scissorX2;
		rect.bottom = state.scissorY2;
		context->RSSetScissorRects( 1, &rect );
	}
	else
	{
		context->RSSetScissorRects( 0, nullptr );
	}

	// Success
	return true;
}


bool GfxCore::rb_set_sampler_state( const GfxSamplerState &state )
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

	// TODO: Multiple sampler states (per-texture?)
	// Right now, every texture has the same sampler which may not be desired -- could be a parameter to
	// Texture.bind()?
	context->PSSetSamplers( 0, 1, &samplerState );
	return true;
}


bool GfxCore::rb_set_blend_state( const GfxBlendState &state )
{
	DECL_ZERO( D3D11_BLEND_DESC, bsDesc );
	bsDesc.AlphaToCoverageEnable = false;
	bsDesc.IndependentBlendEnable = false;
	bsDesc.RenderTarget[0].BlendEnable = state.blendEnable && state.colorWriteMask;
	bsDesc.RenderTarget[0].SrcBlend = D3D11BlendFactors[state.srcFactorColor];
	bsDesc.RenderTarget[0].DestBlend = D3D11BlendFactors[state.dstFactorColor];
	bsDesc.RenderTarget[0].BlendOp = D3D11BlendOperations[state.blendOperationColor];
	bsDesc.RenderTarget[0].SrcBlendAlpha = D3D11BlendFactors[state.srcFactorAlpha];
	bsDesc.RenderTarget[0].DestBlendAlpha = D3D11BlendFactors[state.dstFactorAlpha];
	bsDesc.RenderTarget[0].BlendOpAlpha = D3D11BlendOperations[state.blendOperationAlpha];
	bsDesc.RenderTarget[0].RenderTargetWriteMask = state.colorWriteMask;

	ID3D11BlendState *blendState = nullptr;
	if( FAILED( device->CreateBlendState( &bsDesc, &blendState ) ) )
	{
		ErrorReturnMsg( false, "%s: Failed to create blend state", __FUNCTION__ );
	}

	context->OMSetBlendState( blendState, nullptr, 0xFFFFFFFF );
	return true;
}


bool GfxCore::rb_set_depth_state( const GfxDepthState &state )
{
	DECL_ZERO( D3D11_DEPTH_STENCIL_DESC, dsDesc );
	dsDesc.DepthEnable = state.depthTestMode != GfxDepthTestMode_NONE;
	dsDesc.DepthWriteMask = D3D11DepthWriteMasks[state.depthWriteMask];
	dsDesc.DepthFunc = D3D11DepthTestModes[state.depthTestMode];
	dsDesc.StencilEnable = false;
	dsDesc.StencilReadMask = 0;
	dsDesc.StencilWriteMask = 0;

	ID3D11DepthStencilState *depthState = nullptr;
	if( FAILED( device->CreateDepthStencilState( &dsDesc, &depthState ) ) )
	{
		ErrorReturnMsg( false, "%s: Failed to create depth stencil state", __FUNCTION__ );
	}

	context->OMSetDepthStencilState( depthState, 0 );
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GfxIndexBufferResource::release( GfxIndexBufferResource *&resource )
{
	if( resource == nullptr ) { return; }
	if( resource->id == GFX_RESOURCE_ID_NULL ) { return; }

	if( resource->buffer != nullptr ) { resource->buffer->Release(); }

	indexBufferResources.remove( resource->id );
	resource = nullptr;
}


bool GfxCore::rb_index_buffer_init( GfxIndexBufferResource *&resource,
	void *data, const u32 size, const double indToVertRatio,
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

	// Buffer Description
	DECL_ZERO( D3D11_BUFFER_DESC, bfDesc );
	bfDesc.ByteWidth = size;
	bfDesc.Usage = D3D11CPUAccessModes[accessMode].d3d11Usage;
	bfDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bfDesc.CPUAccessFlags = D3D11CPUAccessModes[accessMode].cpuAccessFlag;
	bfDesc.MiscFlags = 0;
	bfDesc.StructureByteStride = 0;

	// Buffer Data
	DECL_ZERO( D3D11_SUBRESOURCE_DATA, bfData );
	bfData.pSysMem = data;

	// Create Buffer
	if( FAILED( device->CreateBuffer( &bfDesc, &bfData, &resource->buffer ) ) )
	{
		GfxIndexBufferResource::release( resource );
		ErrorReturnMsg( false, "%s: Failed to create index buffer", __FUNCTION__ );
	}

	PROFILE_GFX( Gfx::stats.gpuMemoryIndexBuffers += size );
	return true;
}


bool GfxCore::rb_index_buffer_free( GfxIndexBufferResource *&resource )
{
	Assert( resource != nullptr );

	PROFILE_GFX( Gfx::stats.gpuMemoryIndexBuffers -= resource->size );

	GfxIndexBufferResource::release( resource );

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GfxVertexBufferResource::release( GfxVertexBufferResource *&resource )
{
	if( resource == nullptr ) { return; }
	if( resource->id == GFX_RESOURCE_ID_NULL ) { return; }

	if( resource->buffer != nullptr ) { resource->buffer->Release(); }

	vertexBufferResources.remove( resource->id );
	resource = nullptr;
}


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

	// Buffer Description
	DECL_ZERO( D3D11_BUFFER_DESC, bfDesc );
	bfDesc.ByteWidth = size;
	bfDesc.Usage = D3D11CPUAccessModes[accessMode].d3d11Usage;
	bfDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bfDesc.CPUAccessFlags = D3D11CPUAccessModes[accessMode].cpuAccessFlag;
	bfDesc.MiscFlags = 0;
	bfDesc.StructureByteStride = 0;

	// Create Buffer
	if( FAILED( device->CreateBuffer( &bfDesc, nullptr, &resource->buffer ) ) )
	{
		GfxVertexBufferResource::release( resource );
		ErrorReturnMsg( false, "%s: Failed to create vertex buffer", __FUNCTION__ );
	}

	PROFILE_GFX( Gfx::stats.gpuMemoryVertexBuffers += resource->size );
	return true;
}


bool GfxCore::rb_vertex_buffer_init_static( GfxVertexBufferResource *&resource, const u32 vertexFormatID,
	const GfxCPUAccessMode accessMode, const void *const data,
	const u32 size, const u32 stride )
{
	// TODO: Implement this

	// Register VertexBuffer
	Assert( accessMode == GfxCPUAccessMode_WRITE_DISCARD || accessMode == GfxCPUAccessMode_WRITE_NO_OVERWRITE );
	Assert( resource == nullptr );
	resource = vertexBufferResources.make_new();
	resource->size = size;
	resource->stride = stride;
	resource->accessMode = accessMode;

	// Buffer Description
	DECL_ZERO( D3D11_BUFFER_DESC, bfDesc );
	bfDesc.ByteWidth = size;
	bfDesc.Usage = D3D11CPUAccessModes[accessMode].d3d11Usage;
	bfDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bfDesc.CPUAccessFlags = D3D11CPUAccessModes[accessMode].cpuAccessFlag;
	bfDesc.MiscFlags = 0;
	bfDesc.StructureByteStride = 0;

	// Create Buffer
	if( FAILED( device->CreateBuffer( &bfDesc, nullptr, &resource->buffer ) ) )
	{
		GfxVertexBufferResource::release( resource );
		ErrorReturnMsg( false, "%s: Failed to create vertex buffer", __FUNCTION__ );
	}

	PROFILE_GFX( Gfx::stats.gpuMemoryVertexBuffers += resource->size );
	return true;
}


bool GfxCore::rb_vertex_buffer_free( GfxVertexBufferResource *&resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	PROFILE_GFX( Gfx::stats.gpuMemoryVertexBuffers -= resource->size );

	GfxVertexBufferResource::release( resource );

	return true;
}


bool GfxCore::rb_vertex_buffer_draw( GfxVertexBufferResource *&resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	if( resource->mapped )
	{
		context->Unmap( resource->buffer, 0 );
		resource->mapped = false;
	}

	// Submit Vertex Buffer
	context->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
	context->IASetVertexBuffers( 0, 1, &resource->buffer, &resource->stride, &resource->offset );
	const UINT count = static_cast<UINT>( resource->current / resource->stride );
	d3d11_draw( count, 0 );

	return true;
}


bool GfxCore::rb_vertex_buffer_draw_indexed( GfxVertexBufferResource *&resource, GfxIndexBufferResource *&resourceIndexBuffer )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( resourceIndexBuffer != nullptr && resourceIndexBuffer->id != GFX_RESOURCE_ID_NULL );

	if( resource->mapped )
	{
		context->Unmap( resource->buffer, 0 );
		resource->mapped = false;
	}

	// Submit Vertex Buffer
	context->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
	context->IASetVertexBuffers( 0, 1, &resource->buffer, &resource->stride, &resource->offset );

	// TODO: Cache this?
	context->IASetIndexBuffer( resourceIndexBuffer->buffer, D3D11IndexBufferFormats[resourceIndexBuffer->format], 0 );
	const UINT count = static_cast<UINT>( resource->current / resource->stride * resourceIndexBuffer->indToVertRatio );
	d3d11_draw_indexed( count, 0, 0 );

	return true;
}


void GfxCore::rb_vertex_buffered_write_begin( GfxVertexBufferResource *&resource )
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


void GfxCore::rb_vertex_buffered_write_end( GfxVertexBufferResource *&resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	if( resource->mapped == false ) { return; }

	context->Unmap( resource->buffer, 0 );
	resource->mapped = false;
}


bool GfxCore::rb_vertex_buffered_write( GfxVertexBufferResource *&resource, const void *const data, const u32 size )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( resource->mapped );

	memory_copy( resource->data + resource->current, data, size );
	resource->current += size;

	return true;
}


u32 GfxCore::rb_vertex_buffer_current( GfxVertexBufferResource *&resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	return resource->current;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GfxUniformBufferResource::release( GfxUniformBufferResource *&resource )
{
	if( resource == nullptr ) { return; }
	if( resource->id == GFX_RESOURCE_ID_NULL ) { return; }

	if( resource->buffer != nullptr ) { resource->buffer->Release(); }

	uniformBufferResources.remove( resource->id );
	resource = nullptr;
}


bool GfxCore::rb_uniform_buffer_init( GfxUniformBufferResource *&resource, const char *name,
	const int index, const u32 size )
{
	// Register Constant Buffer
	Assert( resource == nullptr );
	resource = uniformBufferResources.make_new();
	resource->name = name;
	resource->index = index;
	resource->size = size;

	// Buffer Description
	DECL_ZERO( D3D11_BUFFER_DESC, bfDesc );
	bfDesc.ByteWidth = size;
	bfDesc.Usage = D3D11_USAGE_DYNAMIC;
	bfDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bfDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bfDesc.MiscFlags = 0;
	bfDesc.StructureByteStride = 0;

	// Create Buffer
	if( FAILED( device->CreateBuffer( &bfDesc, nullptr, &resource->buffer ) ) )
	{
		GfxUniformBufferResource::release( resource );
		ErrorReturnMsg( false, "%s: Failed to create constant buffer", __FUNCTION__ );
	}

	PROFILE_GFX( Gfx::stats.gpuMemoryUniformBuffers += resource->size );
	return true;
}


bool GfxCore::rb_uniform_buffer_free( GfxUniformBufferResource *&resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	PROFILE_GFX( Gfx::stats.gpuMemoryUniformBuffers -= resource->size );

	GfxUniformBufferResource::release( resource );

	return true;
}


void GfxCore::rb_constant_buffered_write_begin( GfxUniformBufferResource *&resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	if( resource->mapped == true ) { return; }

 	DECL_ZERO( D3D11_MAPPED_SUBRESOURCE, mappedResource );
	context->Map( resource->buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource );

	resource->data = reinterpret_cast<byte *>( mappedResource.pData );
	resource->mapped = true;
}


void GfxCore::rb_constant_buffered_write_end( GfxUniformBufferResource *&resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	if( resource->mapped == false ) { return; }

	context->Unmap( resource->buffer, 0 );
	resource->mapped = false;
}


bool GfxCore::rb_constant_buffered_write( GfxUniformBufferResource *&resource, const void *data )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( resource->mapped );
	Assert( resource->data != nullptr );

	memory_copy( resource->data, data, resource->size );

	return true;
}


bool GfxCore::rb_uniform_buffer_bind_vertex( GfxUniformBufferResource *&resource, const int slot )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( slot >= 0 );

	context->VSSetConstantBuffers( static_cast<UINT>( slot ), 1, &resource->buffer );

	return true;
}


bool GfxCore::rb_uniform_buffer_bind_fragment( GfxUniformBufferResource *&resource, const int slot )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( slot >= 0 );

	context->PSSetConstantBuffers( static_cast<UINT>( slot ), 1, &resource->buffer );

	return true;
}


bool GfxCore::rb_uniform_buffer_bind_compute( GfxUniformBufferResource *&resource, const int slot )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( slot >= 0 );

	context->CSSetConstantBuffers( static_cast<UINT>( slot ), 1, &resource->buffer );

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GfxTexture2DResource::release( GfxTexture2DResource *&resource )
{
	if( resource == nullptr ) { return; }
	if( resource->id == GFX_RESOURCE_ID_NULL ) { return; }

	if( resource->view != nullptr ) { resource->view->Release(); }

	texture2DResources.remove( resource->id );
	resource = nullptr;
}


bool GfxCore::rb_texture_2d_init( GfxTexture2DResource *&resource, void *pixels,
	const u16 width, const u16 height, const u16 levels, const GfxColorFormat &format )
{
	const usize pixelSizeBytes = GfxCore::colorFormatPixelSizeBytes[format];
	const byte *source = reinterpret_cast<const byte *>( pixels );

	ErrorReturnIf( Gfx::mip_level_count_2d( width, height ) < levels, false,
		"%s: requested mip levels is more than texture size supports (attempted: %u)", levels )
	ErrorReturnIf( levels >= GFX_MIP_DEPTH_MAX, false,
		"%s: exceeded max mip level count (has: %u, max: %u)", levels, GFX_MIP_DEPTH_MAX );

	// Register Texture2D
	Assert( resource == nullptr );
	resource = texture2DResources.make_new();
	resource->colorFormat = format;
	resource->width = width;
	resource->height = height;
	resource->levels = levels;
	resource->size = 0;

	// Texture Description
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

	// Mip Chain Data
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

	// Texture
	ID3D11Texture2D *texture = nullptr;
	if( FAILED( device->CreateTexture2D( &tDesc, subresources, &texture ) ) )
	{
		GfxTexture2DResource::release( resource );
		ErrorReturnMsg( false, "%s: Failed to create texture 2D", __FUNCTION__ );
	}

	// Shader Resource View
	if( FAILED( device->CreateShaderResourceView( texture, nullptr, &resource->view ) ) )
	{
		texture->Release();
		GfxTexture2DResource::release( resource );
		ErrorReturnMsg( false, "%s: Failed to create shader resource view", __FUNCTION__ );
	}
	texture->Release(); // Resource owned by resource->view

	PROFILE_GFX( Gfx::stats.gpuMemoryTextures += resource->size );

	return true;
}


bool GfxCore::rb_texture_2d_free( GfxTexture2DResource *&resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	PROFILE_GFX( Gfx::stats.gpuMemoryTextures -= resource->size );

	GfxTexture2DResource::release( resource );

	return true;
}


bool GfxCore::rb_texture_2d_bind( const GfxTexture2DResource *const &resource, const int slot )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( slot >= 0 && slot < GFX_TEXTURE_SLOT_COUNT );

	textureSlots[slot] = resource->view;
	PROFILE_GFX( Gfx::stats.frame.textureBinds++ );
	context->VSSetShaderResources( 0, GFX_TEXTURE_SLOT_COUNT, textureSlots );
	context->PSSetShaderResources( 0, GFX_TEXTURE_SLOT_COUNT, textureSlots );

	// Success
	return true;
}


bool GfxCore::rb_texture_2d_release( const GfxTexture2DResource *const &resource, const int slot )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( slot > 0 && slot < GFX_TEXTURE_SLOT_COUNT );
	Assert( textureSlots[slot] != nullptr );

	textureSlots[slot] = nullptr;
	PROFILE_GFX( Gfx::stats.frame.textureBinds-- );
	context->VSSetShaderResources( 0, GFX_TEXTURE_SLOT_COUNT, textureSlots );
	context->PSSetShaderResources( 0, GFX_TEXTURE_SLOT_COUNT, textureSlots );

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GfxRenderTarget2DResource::release( GfxRenderTarget2DResource *&resource )
{
	if( resource == nullptr ) { return; }
	if( resource->id == GFX_RESOURCE_ID_NULL ) { return; }

	if( resource->targetColor != nullptr ) { resource->targetColor->Release(); }
	if( resource->targetDepth != nullptr ) { resource->targetDepth->Release(); }
	if( resource->textureColorSS != nullptr ) { resource->textureColorSS->Release(); }
	if( resource->textureColorMS != nullptr ) { resource->textureColorMS->Release(); }
	if( resource->textureDepthSS != nullptr ) { resource->textureDepthSS->Release(); }
	if( resource->textureDepthMS != nullptr ) { resource->textureDepthMS->Release(); }
	if( resource->textureColorStaging != nullptr ) { resource->textureColorStaging->Release(); }
	if( resource->textureDepthStaging != nullptr ) { resource->textureDepthStaging->Release(); }

	renderTarget2DResources.remove( resource->id );
	resource = nullptr;
}


bool GfxCore::rb_render_target_2d_init( GfxRenderTarget2DResource *&resource,
	GfxTexture2DResource *&resourceColor, GfxTexture2DResource *&resourceDepth,
	const u16 width, const u16 height,
	const GfxRenderTargetDescription &desc )
{
#if PROFILING_GFX
	usize CACHE_GPU_MEMORY_RENDER_TARGETS = Gfx::stats.gpuMemoryRenderTargets;
#endif

	// Register RenderTarget2D
	Assert( resource == nullptr );
	resource = renderTarget2DResources.make_new();
	resource->width = width;
	resource->height = height;
	resource->desc = desc;
	resource->size = 0;

	// Color
	if( desc.colorFormat != GfxColorFormat_NONE )
	{
		// Register Resource
		Assert( resourceColor == nullptr );
		resourceColor = texture2DResources.make_new();

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
			GfxTexture2DResource::release( resourceColor );
			GfxRenderTarget2DResource::release( resource );
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
			if( FAILED(hr) || msaaQualityLevels == 0 ) { msaaSampleCount = 1; msaaQualityLevels = 1; }

			tDesc.SampleDesc.Count = msaaSampleCount;
			tDesc.SampleDesc.Quality = msaaQualityLevels - 1;

			Assert( resource->textureColorMS == nullptr );
			if( FAILED( device->CreateTexture2D( &tDesc, nullptr, &resource->textureColorMS ) ) )
			{
				GfxTexture2DResource::release( resourceColor );
				GfxRenderTarget2DResource::release( resource );
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

		Assert( resource->targetColor == nullptr );
		if( FAILED( device->CreateRenderTargetView( desc.sampleCount > 1 ?
			resource->textureColorMS : resource->textureColorSS,
			&rtvDesc, &resource->targetColor ) ) )
		{
			GfxTexture2DResource::release( resourceColor );
			GfxRenderTarget2DResource::release( resource );
			ErrorReturnMsg( false, "%s: Failed to create color render target view", __FUNCTION__ );
		}

		// Shader Resource View
		DECL_ZERO( D3D11_SHADER_RESOURCE_VIEW_DESC, srvDesc );
		srvDesc.Format = D3D11ColorFormats[desc.colorFormat];
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;

		Assert( resourceColor->view == nullptr );
		if( FAILED( device->CreateShaderResourceView( resource->textureColorSS, &srvDesc, &resourceColor->view ) ) )
		{
			GfxTexture2DResource::release( resourceColor );
			GfxRenderTarget2DResource::release( resource );
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
				GfxTexture2DResource::release( resourceColor );
				GfxRenderTarget2DResource::release( resource );
				ErrorReturnMsg( false, "%s: Failed to create color CPU staging texture", __FUNCTION__ );
			}
			resource->size += GFX_SIZE_IMAGE_COLOR_BYTES( width, height, 1, desc.colorFormat );
		}
	}

	// Depth
	if( desc.depthFormat != GfxDepthFormat_NONE )
	{
		// Register Resource
		Assert( resourceDepth == nullptr );
		resourceDepth = texture2DResources.make_new();
		const bool hasMSAA = desc.sampleCount > 1;

		// Texture2D (Single-Sample)
		DECL_ZERO( D3D11_TEXTURE2D_DESC, tDesc );
		tDesc.Width = width;
		tDesc.Height = height;
		tDesc.MipLevels = 1;
		tDesc.ArraySize = 1;
		tDesc.Format = hasMSAA ? D3D11DepthStencilFormats[desc.depthFormat].shaderResourceViewDescFormat : // MSAA: Typed
			D3D11DepthStencilFormats[desc.depthFormat].texture2DDescFormat; // Single-Sample: Untyped
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
			GfxTexture2DResource::release( resourceColor );
			GfxTexture2DResource::release( resourceDepth );
			GfxRenderTarget2DResource::release( resource );
			ErrorReturnMsg( false, "%s: Failed to create depth texture 2d", __FUNCTION__ );
		}
		resource->size += GFX_SIZE_IMAGE_DEPTH_BYTES( width, height, 1, desc.depthFormat );

		// Texture2D (Multi-Sample)
		if( desc.sampleCount > 1 )
		{
			UINT msaaSampleCount = desc.sampleCount;
			UINT msaaQualityLevels = 0;
			HRESULT hr = device->CheckMultisampleQualityLevels(
				D3D11DepthStencilFormats[desc.depthFormat].texture2DDescFormat, msaaSampleCount, &msaaQualityLevels );
			if( FAILED( hr ) || msaaQualityLevels == 0 ) { msaaSampleCount = 1; msaaQualityLevels = 1; }

			tDesc.Format = D3D11DepthStencilFormats[desc.depthFormat].texture2DDescFormat;
			tDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
			tDesc.SampleDesc.Count = msaaSampleCount;
			tDesc.SampleDesc.Quality = msaaQualityLevels - 1;

			Assert( resource->textureDepthMS == nullptr );
			if( FAILED( device->CreateTexture2D( &tDesc, nullptr, &resource->textureDepthMS ) ) )
			{
				GfxTexture2DResource::release( resourceColor );
				GfxTexture2DResource::release( resourceDepth );
				GfxRenderTarget2DResource::release( resource );
				ErrorReturnMsg( false, "%s: Failed to create depth texture 2d", __FUNCTION__ );
			}
			resource->size += GFX_SIZE_IMAGE_DEPTH_BYTES( width, height, 1, desc.depthFormat ) * msaaSampleCount;
		}

		// Depth-Stencil View
		DECL_ZERO( D3D11_DEPTH_STENCIL_VIEW_DESC, dsvDesc );
		dsvDesc.Format = D3D11DepthStencilFormats[desc.depthFormat].depthStencilViewDescFormat;
		dsvDesc.ViewDimension = desc.sampleCount > 1 ?
			D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Flags = 0;
		dsvDesc.Texture2D.MipSlice = 0;

		Assert( resource->targetDepth == nullptr );
		if( FAILED( device->CreateDepthStencilView( desc.sampleCount > 1 ?
			resource->textureDepthMS : resource->textureDepthSS,
			&dsvDesc, &resource->targetDepth ) ) )
		{
			GfxTexture2DResource::release( resourceColor );
			GfxTexture2DResource::release( resourceDepth );
			GfxRenderTarget2DResource::release( resource );
			ErrorReturnMsg( false, "%s: Failed to create depth render target view", __FUNCTION__ );
		}

		// Shader Resource
		DECL_ZERO( D3D11_SHADER_RESOURCE_VIEW_DESC, srvDesc );
		srvDesc.Format = D3D11DepthStencilFormats[desc.depthFormat].shaderResourceViewDescFormat;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;

		Assert( resourceDepth->view == nullptr );
		if( FAILED( device->CreateShaderResourceView( resource->textureDepthSS, &srvDesc, &resourceDepth->view ) ) )
		{
			GfxTexture2DResource::release( resourceColor );
			GfxTexture2DResource::release( resourceDepth );
			GfxRenderTarget2DResource::release( resource );
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
				GfxTexture2DResource::release( resourceColor );
				GfxTexture2DResource::release( resourceDepth );
				GfxRenderTarget2DResource::release( resource );
				ErrorReturnMsg( false, "%s: Failed to create depth CPU staging texture", __FUNCTION__ );
			}
			resource->size += GFX_SIZE_IMAGE_DEPTH_BYTES( width, height, 1, desc.depthFormat );
		}
	}

	PROFILE_GFX( Gfx::stats.gpuMemoryRenderTargets += resource->size );
	return true;
}


bool GfxCore::rb_render_target_2d_free( GfxRenderTarget2DResource *&resource,
	GfxTexture2DResource *&resourceColor,
	GfxTexture2DResource *&resourceDepth )
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


bool GfxCore::rb_render_target_2d_copy(
	GfxRenderTarget2DResource *&srcResource,
	GfxTexture2DResource *&srcResourceColor, GfxTexture2DResource *&srcResourceDepth,
	GfxRenderTarget2DResource *&dstResource,
	GfxTexture2DResource *&dstResourceColor, GfxTexture2DResource *&dstResourceDepth )
{
	// Validate resources
	if( srcResource == nullptr || dstResource == nullptr ) { return false; }

	// Ensure equal dimensions
	if( srcResource->width != dstResource->width || srcResource->height != dstResource->height ) { return false; }

	// Copy Color
	if( srcResource->targetColor && dstResource->targetColor )
	{
		ID3D11Resource *srcD3D11ResourceColor;
		ID3D11Resource *dstD3D11ResourceColor;
		srcResourceColor->view->GetResource( &srcD3D11ResourceColor );
		dstResourceColor->view->GetResource( &dstD3D11ResourceColor );
		context->CopySubresourceRegion( dstD3D11ResourceColor, 0, 0, 0, 0, srcD3D11ResourceColor, 0, nullptr );
		srcD3D11ResourceColor->Release();
		dstD3D11ResourceColor->Release();
	}

	// Copy Depth Texture
	if( srcResource->targetDepth && dstResource->targetDepth )
	{
		ID3D11Resource *srcD3D11ResourceDepth;
		ID3D11Resource *dstD3D11ResourceDepth;
		srcResourceDepth->view->GetResource( &srcD3D11ResourceDepth );
		dstResourceDepth->view->GetResource( &dstD3D11ResourceDepth );
		context->CopySubresourceRegion( dstD3D11ResourceDepth, 0, 0, 0, 0, srcD3D11ResourceDepth, 0, nullptr );
		srcD3D11ResourceDepth->Release();
		dstD3D11ResourceDepth->Release();
	}

	return true;
}


bool GfxCore::rb_render_target_2d_copy_part(
	GfxRenderTarget2DResource *&srcResource,
	GfxTexture2DResource *&srcResourceColor, GfxTexture2DResource *&srcResourceDepth,
	GfxRenderTarget2DResource *&dstResource,
	GfxTexture2DResource *&dstResourceColor, GfxTexture2DResource *&dstResourceDepth,
	u16 srcX, u16 srcY, u16 dstX, u16 dstY, u16 width, u16 height )
{
	// Validate resources
	if( srcResource == nullptr || dstResource == nullptr ) { return false; }

	// Create D3D11_BOX
	D3D11_BOX sourceRegion;
	sourceRegion.left = static_cast<UINT>( srcX );
	sourceRegion.right = static_cast<UINT>( srcX ) + static_cast<UINT>( width );
	sourceRegion.top = static_cast<UINT>( srcY );
	sourceRegion.bottom = static_cast<UINT>( srcY ) + static_cast<UINT>( height );
	sourceRegion.front = 0;
	sourceRegion.back = 1;

	// Copy Color
	if( srcResource->targetColor && dstResource->targetColor )
	{
		ID3D11Resource *srcD3D11ResourceColor;
		ID3D11Resource *dstD3D11ResourceColor;
		srcResourceColor->view->GetResource( &srcD3D11ResourceColor );
		dstResourceColor->view->GetResource( &dstD3D11ResourceColor );
		context->CopySubresourceRegion( dstD3D11ResourceColor, 0, static_cast<UINT>( dstX ), static_cast<UINT>( dstY ),
			0, srcD3D11ResourceColor, 0, &sourceRegion );
		srcD3D11ResourceColor->Release();
		dstD3D11ResourceColor->Release();
	}

	// Copy Depth Texture
	#if 0
	if( srcResource->targetDepth && dstResource->targetDepth )
	{
		// No native API support for this, have to implement manually
		// See: GfxRenderTarget2D::copy_part()
	}
	#endif

	return true;
}


bool GfxCore::rb_render_target_2d_buffer_read_color( GfxRenderTarget2DResource *&resource,
		GfxTexture2DResource *&resourceColor,
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
	resourceColor->view->GetResource( &srcResource );
	context->CopyResource( resource->textureColorStaging, srcResource );
	srcResource->Release();

	// Map Staging Texture
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

	// Unmap Staging Texture
	context->Unmap( resource->textureColorStaging, 0 );

	return true;
}


bool rb_render_target_2d_buffer_read_depth( GfxRenderTarget2DResource *&resource,
	GfxTexture2DResource *&resourceDepth,
	void *buffer, const u32 size )
{
	// TODO ...

	return true;
}


bool GfxCore::rb_render_target_2d_buffered_write_color( GfxRenderTarget2DResource *&resource,
	GfxTexture2DResource *&resourceColor,
	const void *const buffer, const u32 size )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( resource->textureColorStaging != nullptr );
	Assert( resourceColor != nullptr && resourceColor->id != GFX_RESOURCE_ID_NULL );

	ErrorIf( !resource->desc.cpuAccess,
		"Trying to CPU access a render target that does not have CPU access flag!" );

	const u32 sizeSource = resource->width * resource->height *
		GfxCore::colorFormatPixelSizeBytes[resource->desc.colorFormat];
	Assert( size > 0 && size <= sizeSource );
	Assert( buffer != nullptr );

	// Map Staging Texture
	DECL_ZERO( D3D11_MAPPED_SUBRESOURCE, mappedResource );
	context->Map( resource->textureColorStaging, 0, D3D11_MAP_READ, 0, &mappedResource );

	// Copy Texture Data
	memory_copy( mappedResource.pData, buffer, sizeSource );

	// Unmap Staging Texture
	context->Unmap( resource->textureColorStaging, 0 );

	// Copy Data to Staging Texture
	ID3D11Resource *dstResource;
	resourceColor->view->GetResource( &dstResource );
	context->CopyResource( dstResource, resource->textureColorStaging );
	dstResource->Release();

	return true;
}


bool GfxCore::rb_render_target_2d_bind( const GfxRenderTarget2DResource *const &resource, int slot )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( slot >= 0 && slot < GFX_RENDER_TARGET_SLOT_COUNT );

	if( slot == 0 )
	{
		RT_TEXTURE_COLOR_CACHE = RT_TEXTURE_COLOR[0];
		RT_TEXTURE_DEPTH_CACHE = RT_TEXTURE_DEPTH;
	}

	RT_TEXTURE_COLOR[slot] = resource->targetColor;
	if( slot == 0 ) { RT_TEXTURE_DEPTH = resource->targetDepth; }

	context->OMSetRenderTargets( GFX_RENDER_TARGET_SLOT_COUNT, RT_TEXTURE_COLOR, RT_TEXTURE_DEPTH );

	// Success
	return true;
}


bool GfxCore::rb_render_target_2d_release( const GfxRenderTarget2DResource *const &resource, const int slot )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( slot >= 0 && slot < GFX_RENDER_TARGET_SLOT_COUNT );

	RT_TEXTURE_COLOR[slot] = ( slot == 0 ) ? RT_TEXTURE_COLOR_CACHE : nullptr;
	if( slot == 0 ) { RT_TEXTURE_DEPTH = RT_TEXTURE_DEPTH_CACHE; }

	context->OMSetRenderTargets( GFX_RENDER_TARGET_SLOT_COUNT, RT_TEXTURE_COLOR, RT_TEXTURE_DEPTH );

	// MSAA: Resolve
	const GfxRenderTargetDescription &desc = resource->desc;
	if( desc.sampleCount > 1 )
	{
		// Color Texture
		if( resource->textureColorSS != nullptr && resource->textureColorMS != nullptr )
		{
			msaa_resolver_color( resource->textureColorMS, resource->textureColorSS, desc.colorFormat );
		}

		// Depth Texture
		if( resource->textureDepthSS != nullptr && resource->textureDepthMS != nullptr )
		{
			msaa_resolver_depth( resource->textureDepthMS, resource->textureDepthSS, nullptr, desc.depthFormat );
		}
	}

	// Success
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GfxShaderResource::release( GfxShaderResource *&resource )
{
	if( resource == nullptr ) { return; }
	if( resource->id == GFX_RESOURCE_ID_NULL ) { return; }

	if( resource->vs != nullptr ) { resource->vs->Release(); }
	if( resource->ps != nullptr ) { resource->ps->Release(); }
	if( resource->il != nullptr ) { resource->il->Release(); }

	shaderResources.remove( resource->id );
	resource = nullptr;
}


bool GfxCore::rb_shader_init( GfxShaderResource *&resource, const u32 shaderID, const struct BinShader &binShader )
{
	// Register Shader
	Assert( resource == nullptr );
	Assert( shaderID < Gfx::shadersCount );
	resource = shaderResources.make_new();

	ID3D10Blob *info;
	ID3D10Blob *vsCode, *vsStripped;
	ID3D10Blob *psCode, *psStripped;
	u32 vsSize, psSize;

	// Compile Vertex Shader
	const void *codeVertex = reinterpret_cast<const void *>( Assets::binary.data + binShader.offsetVertex );
	if( FAILED( D3DCompile( codeVertex, binShader.sizeVertex, nullptr, nullptr, nullptr,
	                        "vs_main", "vs_4_0", D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &vsCode, &info ) ) )
	{
		GfxShaderResource::release( resource );
		ErrorReturnMsg( false,
			"%s: Failed to compile vertex shader (%s)\n\n%s",
			__FUNCTION__, binShader.name, reinterpret_cast<const char *>( info->GetBufferPointer() ) );
	}

	// Compile Fragment Shader
	const void *codeFragment = reinterpret_cast<const void *>( Assets::binary.data + binShader.offsetFragment );
	if( FAILED( D3DCompile( codeFragment, binShader.sizeFragment, nullptr, nullptr, nullptr,
	                        "ps_main", "ps_4_0", D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &psCode, &info ) ) )
	{
		GfxShaderResource::release( resource );
		ErrorReturnMsg( false,
			"%s: Failed to compile fragment shader (%s)\n\n%s",
			__FUNCTION__, binShader.name, reinterpret_cast<const char *>( info->GetBufferPointer() ) );
	}

	// Strip Vertex Shader
	D3DStripShader( vsCode->GetBufferPointer(), vsCode->GetBufferSize(),
	                D3DCOMPILER_STRIP_REFLECTION_DATA |
	                D3DCOMPILER_STRIP_DEBUG_INFO |
	                D3DCOMPILER_STRIP_PRIVATE_DATA, &vsStripped );

	// Strip Fragment Shader
	D3DStripShader( psCode->GetBufferPointer(), psCode->GetBufferSize(),
	                D3DCOMPILER_STRIP_REFLECTION_DATA |
	                D3DCOMPILER_STRIP_DEBUG_INFO |
	                D3DCOMPILER_STRIP_PRIVATE_DATA, &psStripped );

	// Create Vertex Shader
	resource->sizeVS = vsStripped->GetBufferSize();
	if( FAILED( device->CreateVertexShader( vsStripped->GetBufferPointer(),
	                                        vsStripped->GetBufferSize(), nullptr, &resource->vs ) ) )
	{
		GfxShaderResource::release( resource );
		ErrorReturnMsg( false, "%s: Failed to create vertex shader (%s)",
			__FUNCTION__, binShader.name );
	}

	// Create Pixel Shader
	resource->sizePS = psStripped->GetBufferSize();
	if( FAILED( device->CreatePixelShader( psStripped->GetBufferPointer(),
	                                       psStripped->GetBufferSize(), nullptr, &resource->ps ) ) )
	{
		GfxShaderResource::release( resource );
		ErrorReturnMsg( false, "%s: Failed to create pixel shader (%s)",
			__FUNCTION__, binShader.name );
	}

	// Create Input Layout
	D3D11VertexInputLayoutDescription desc;
	GfxCore::d3d11_vertex_input_layout_desc[Gfx::binShaders[shaderID].vertexFormat]( desc );
	if( FAILED( device->CreateInputLayout( desc.desc, desc.count,
	                                       vsStripped->GetBufferPointer(),
	                                       vsStripped->GetBufferSize(), &resource->il ) ) )
	{
		GfxShaderResource::release( resource );
		ErrorReturnMsg( false, "%s: Failed to create input layout (%s)",
			__FUNCTION__, binShader.name );
	}

	PROFILE_GFX( Gfx::stats.gpuMemoryShaderPrograms += resource->sizeVS );
	PROFILE_GFX( Gfx::stats.gpuMemoryShaderPrograms += resource->sizePS );
	PROFILE_GFX( Gfx::stats.gpuMemoryShaderPrograms += resource->sizeCS );
	return true;
}


bool GfxCore::rb_shader_free( GfxShaderResource *&resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	PROFILE_GFX( Gfx::stats.gpuMemoryShaderPrograms -= resource->sizeVS );
	PROFILE_GFX( Gfx::stats.gpuMemoryShaderPrograms -= resource->sizePS );
	PROFILE_GFX( Gfx::stats.gpuMemoryShaderPrograms -= resource->sizeCS );

	GfxShaderResource::release( resource );

	return true;
}


bool GfxCore::rb_shader_bind( GfxShaderResource *&resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	context->VSSetShader( resource->vs, nullptr, 0 );
	context->PSSetShader( resource->ps, nullptr, 0 );
	context->IASetInputLayout( resource->il );
	PROFILE_GFX( Gfx::stats.frame.shaderBinds++ );

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////