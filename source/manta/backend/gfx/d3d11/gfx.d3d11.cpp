#include <manta/gfx.hpp>

#include <vendor/d3d11.hpp>
#include <vendor/d3dcompiler.hpp> // TODO: Remove from runtime

#include <manta/backend/gfx/gfxfactory.hpp>
#include <manta/backend/window/windows/window.windows.hpp>

#include <gfx.api.generated.hpp>

#include <config.hpp>

#include <core/buffer.hpp>
#include <core/memory.hpp>
#include <core/hashmap.hpp>
#include <core/math.hpp>

#include <manta/window.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static GUID D3D11_IID_IDXGIFactory { 0x7B7166EC, 0x21C7, 0x44AE, { 0xB2, 0x1A, 0xC9, 0xAE, 0x32, 0x1A, 0xE3, 0x69 } };
static GUID D3D11_IID_ID3D11Texture2D { 0x6F15AAF2, 0xD208, 0x4E89, { 0x9A, 0xB4, 0x48, 0x95, 0x35, 0xD3, 0x4F, 0x9C } };

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
	DXGI_FORMAT_R16G16B16A16_FLOAT, // GfxColorFormat_R16G16F_FLOAT
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
	ID3D11Buffer *buffer = nullptr;
	GfxCPUAccessMode accessMode = GfxCPUAccessMode_NONE;
	GfxIndexBufferFormat format = GfxIndexBufferFormat_U32;
	double indToVertRatio = 1.0;
	UINT size = 0; // buffer size in bytes
};


struct GfxConstantBufferResource : public GfxResource
{
	ID3D11Buffer *buffer = nullptr;
	bool mapped = false;
	byte *data = nullptr;
	const char *name = "";
	int index = 0;
	UINT size = 0; // buffer size in bytes
};


struct GfxTexture2DResource : public GfxResource
{
	ID3D11ShaderResourceView *view = nullptr;
	GfxColorFormat colorFormat;
	u32 width = 0;
	u32 height = 0;
};


struct GfxRenderTarget2DResource : public GfxResource
{
	ID3D11RenderTargetView *targetColor = nullptr;
	ID3D11DepthStencilView *targetDepth = nullptr;
	ID3D11Texture2D *stagingColor = nullptr;
	ID3D11Texture2D *stagingDepth = nullptr;
	GfxRenderTargetDescription desc = { };
	u16 width = 0;
	u16 height = 0;
};


struct GfxShaderResource : public GfxResource
{
	ID3D11VertexShader *vs = nullptr;
	ID3D11PixelShader *ps = nullptr;
	ID3D11InputLayout *il = nullptr; // TODO: Generate these from shader code?
	UINT sizeVS, sizePS, sizeCS;
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

	// Success
	return true;
}


bool GfxCore::rb_free()
{
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
		DECL_ZERO( D3D11_TEXTURE2D_DESC, texDesc );
		texDesc.Width = width;
		texDesc.Height = height;
		texDesc.MipLevels = 1;
		texDesc.ArraySize = 1;
		texDesc.Format = D3D11DepthStencilFormats[DEPTH_BUFFER_FORMAT].texture2DDescFormat; // TODO
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		texDesc.CPUAccessFlags = 0;
		texDesc.MiscFlags = 0;

		ID3D11Texture2D *textureDepth = nullptr;
		PROFILE_GFX( Gfx::stats.gpuMemorySwapchain +=
			GFX_SIZE_IMAGE_DEPTH_BYTES( width, height, 1, DEPTH_BUFFER_FORMAT ) * 2 );
		ErrorReturnIf( FAILED( device->CreateTexture2D( &texDesc, nullptr, &textureDepth ) ), false,
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

		DECL_ZERO( D3D11_TEXTURE2D_DESC, texDesc );
		texDesc.Width = width;
		texDesc.Height = height;
		texDesc.MipLevels = 1;
		texDesc.ArraySize = 1;
		texDesc.Format = D3D11DepthStencilFormats[DEPTH_BUFFER_FORMAT].texture2DDescFormat; // TODO
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		texDesc.CPUAccessFlags = 0;
		texDesc.MiscFlags = 0;

		ID3D11Texture2D *textureDepth = nullptr;
		PROFILE_GFX( Gfx::stats.gpuMemorySwapchain +=
			GFX_SIZE_IMAGE_DEPTH_BYTES( width, height, 1, DEPTH_BUFFER_FORMAT ) * 2 );
		ErrorReturnIf( FAILED( device->CreateTexture2D( &texDesc, nullptr, &textureDepth ) ), false,
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
	ssDesc.MaxAnisotropy = 1;
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

	DECL_ZERO( D3D11_BUFFER_DESC, bfDesc );
	bfDesc.ByteWidth = size;
	bfDesc.Usage = D3D11CPUAccessModes[accessMode].d3d11Usage;
	bfDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bfDesc.CPUAccessFlags = D3D11CPUAccessModes[accessMode].cpuAccessFlag;
	bfDesc.MiscFlags = 0;
	bfDesc.StructureByteStride = 0;

	DECL_ZERO( D3D11_SUBRESOURCE_DATA, bfData );
	bfData.pSysMem = data;

	PROFILE_GFX( Gfx::stats.gpuMemoryIndexBuffers += size );
	if( FAILED( device->CreateBuffer( &bfDesc, &bfData, &resource->buffer ) ) )
	{
		ErrorReturnMsg( false, "%s: Failed to create index buffer", __FUNCTION__ );
	}

	// Success
	return true;
}


bool GfxCore::rb_index_buffer_free( GfxIndexBufferResource *&resource )
{
	Assert( resource != nullptr );

	PROFILE_GFX( Gfx::stats.gpuMemoryIndexBuffers -= resource->size );

	resource->buffer->Release();
	resource->buffer = nullptr;
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

	DECL_ZERO( D3D11_BUFFER_DESC, bfDesc );
	bfDesc.ByteWidth = size;
	bfDesc.Usage = D3D11CPUAccessModes[accessMode].d3d11Usage;
	bfDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bfDesc.CPUAccessFlags = D3D11CPUAccessModes[accessMode].cpuAccessFlag;
	bfDesc.MiscFlags = 0;
	bfDesc.StructureByteStride = 0;

	PROFILE_GFX( Gfx::stats.gpuMemoryVertexBuffers += size );
	if( FAILED( device->CreateBuffer( &bfDesc, nullptr, &resource->buffer ) ) )
	{
		ErrorReturnMsg( false, "%s: Failed to create vertex buffer", __FUNCTION__ );
	}

	// Success
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

	DECL_ZERO( D3D11_BUFFER_DESC, bfDesc );
	bfDesc.ByteWidth = size;
	bfDesc.Usage = D3D11CPUAccessModes[accessMode].d3d11Usage;
	bfDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bfDesc.CPUAccessFlags = D3D11CPUAccessModes[accessMode].cpuAccessFlag;
	bfDesc.MiscFlags = 0;
	bfDesc.StructureByteStride = 0;

	PROFILE_GFX( Gfx::stats.gpuMemoryVertexBuffers += size );
	if( FAILED( device->CreateBuffer( &bfDesc, nullptr, &resource->buffer ) ) )
	{
		ErrorReturnMsg( false, "%s: Failed to create vertex buffer", __FUNCTION__ );
	}

	// Success
	return true;
}


bool GfxCore::rb_vertex_buffer_free( GfxVertexBufferResource *&resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	PROFILE_GFX( Gfx::stats.gpuMemoryVertexBuffers -= resource->size );

	resource->buffer->Release();
	resource->buffer = nullptr;
	vertexBufferResources.remove( resource->id );
	resource = nullptr;

	// Success
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

	// Success
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

	// Success
	return true;
}


void GfxCore::rb_vertex_buffered_write_begin( GfxVertexBufferResource *&resource )
{
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

	DECL_ZERO( D3D11_BUFFER_DESC, bfDesc );
	bfDesc.ByteWidth = size;
	bfDesc.Usage = D3D11_USAGE_DYNAMIC;
	bfDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bfDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bfDesc.MiscFlags = 0;
	bfDesc.StructureByteStride = 0;

	PROFILE_GFX( Gfx::stats.gpuMemoryConstantBuffers += size );
	if( FAILED( device->CreateBuffer( &bfDesc, nullptr, &resource->buffer ) ) )
	{
		ErrorReturnMsg( false, "%s: Failed to create constant buffer", __FUNCTION__ );
	}

	// Success
	return true;
}


bool GfxCore::rb_constant_buffer_free( GfxConstantBufferResource *&resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	PROFILE_GFX( Gfx::stats.gpuMemoryConstantBuffers -= resource->size );

	resource->buffer->Release();
	resource->buffer = nullptr;
	constantBufferResources.remove( resource->id );
	resource = nullptr;

	// Success
	return true;
}


void GfxCore::rb_constant_buffered_write_begin( GfxConstantBufferResource *&resource )
{
	if( resource->mapped == true ) { return; }

 	DECL_ZERO( D3D11_MAPPED_SUBRESOURCE, mappedResource );
	context->Map( resource->buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource );

	resource->data = reinterpret_cast<byte *>( mappedResource.pData );
	resource->mapped = true;
}


void GfxCore::rb_constant_buffered_write_end( GfxConstantBufferResource *&resource )
{
	if( resource->mapped == false ) { return; }
	context->Unmap( resource->buffer, 0 );
	resource->mapped = false;
}


bool GfxCore::rb_constant_buffered_write( GfxConstantBufferResource *&resource, const void *data )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( resource->mapped );
	Assert( resource->data != nullptr );

	memory_copy( resource->data, data, resource->size );

	// Success
	return true;
}


bool GfxCore::rb_constant_buffer_bind_vertex( GfxConstantBufferResource *&resource, const int slot )
{
	Assert( slot >= 0 );
	context->VSSetConstantBuffers( static_cast<UINT>( slot ), 1, &resource->buffer );

	// Success
	return true;
}


bool GfxCore::rb_constant_buffer_bind_fragment( GfxConstantBufferResource *&resource, const int slot )
{
	Assert( slot >= 0 );
	context->PSSetConstantBuffers( static_cast<UINT>( slot ), 1, &resource->buffer );

	// Success
	return true;
}


bool GfxCore::rb_constant_buffer_bind_compute( GfxConstantBufferResource *&resource, const int slot )
{
	Assert( slot >= 0 );
	context->CSSetConstantBuffers( static_cast<UINT>( slot ), 1, &resource->buffer );

	// Success
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GfxCore::rb_texture_2d_init( GfxTexture2DResource *&resource, void *pixels,
                                  const u16 width, const u16 height, const GfxColorFormat &format )
{
#if false
	// Flip the texture vertically
	const usize stride = width * colorFormatPixelSizeBytes[format];
	u8 *flipped = SysGfx::scratch_buffer( stride * height );
	u8 *source = static_cast<u8 *>( pixels );
	for( u16 y = 0; y < height; y++ )
	{
		memory_copy( &flipped[y * stride], &source[( height - 1 - y ) * stride], stride );
	}
	pixels = reinterpret_cast<void *>( flipped );
#endif

	// Register Texture2D
	Assert( resource == nullptr );
	resource = texture2DResources.make_new();
	resource->colorFormat = format;
	resource->width = width;
	resource->height = height;

	// Setup Texture Description
	DECL_ZERO( D3D11_TEXTURE2D_DESC, desc );
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = D3D11ColorFormats[format];
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_IMMUTABLE;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	// Setup Texture Data
	DECL_ZERO( D3D11_SUBRESOURCE_DATA, data );
	data.pSysMem = pixels;
	data.SysMemPitch = width * 4;
	data.SysMemSlicePitch = 0;

	// Create Texture
	ID3D11Texture2D *peer = nullptr;
	PROFILE_GFX( Gfx::stats.gpuMemoryTextures += GFX_SIZE_IMAGE_COLOR_BYTES( width, height, 1, format ) );
	if( FAILED( device->CreateTexture2D( &desc, &data, &peer ) ) )
	{
		ErrorReturnMsg( false, "%s: Failed to create texture 2D", __FUNCTION__ );
	}

	// Create Texture View
	if( FAILED( device->CreateShaderResourceView( peer, nullptr, &resource->view ) ) )
	{
		ErrorReturnMsg( false, "%s: Failed to create shader resource view", __FUNCTION__ );
	}

	// Release
	peer->Release();

	// Success
	return true;
}


bool GfxCore::rb_texture_2d_free( GfxTexture2DResource *&resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	PROFILE_GFX( Gfx::stats.gpuMemoryTextures -=
		GFX_SIZE_IMAGE_COLOR_BYTES( resource->width, resource->height, 1, resource->colorFormat ) );

	resource->view->Release();
	resource->view = nullptr;
	texture2DResources.remove( resource->id );
	resource = nullptr;

	// Success
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


bool GfxCore::rb_texture_2d_release( const int slot )
{
	Assert( slot > 0 && slot < GFX_TEXTURE_SLOT_COUNT );
	Assert( textureSlots[slot] != nullptr );

	textureSlots[slot] = nullptr;
	PROFILE_GFX( Gfx::stats.frame.textureBinds-- );
	context->VSSetShaderResources( 0, GFX_TEXTURE_SLOT_COUNT, textureSlots );
	context->PSSetShaderResources( 0, GFX_TEXTURE_SLOT_COUNT, textureSlots );

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

	// Color
	{
		// Register Resource
		resourceColor = texture2DResources.make_new();

		// Create Texture2D
		DECL_ZERO( D3D11_TEXTURE2D_DESC, texDesc );
		texDesc.Width = width;
		texDesc.Height = height;
		texDesc.MipLevels = 1;
		texDesc.ArraySize = 1;
		texDesc.Format = D3D11ColorFormats[desc.colorFormat];
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		texDesc.CPUAccessFlags = 0;
		texDesc.MiscFlags = 0;

		ID3D11Texture2D *textureColor = nullptr;
		ErrorReturnIf( FAILED( device->CreateTexture2D( &texDesc, nullptr, &textureColor ) ), false,
		               "%s: Failed to create color texture 2d", __FUNCTION__ );
		PROFILE_GFX( Gfx::stats.gpuMemoryRenderTargets +=
			GFX_SIZE_IMAGE_COLOR_BYTES( width, height, 1, desc.colorFormat ) );

		// Create Render Target View
		DECL_ZERO( D3D11_RENDER_TARGET_VIEW_DESC, rtvDesc );
		rtvDesc.Format = texDesc.Format;
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		rtvDesc.Texture2D.MipSlice = 0;

		Assert( resource->targetColor == nullptr );
		ErrorReturnIf( FAILED( device->CreateRenderTargetView( textureColor, &rtvDesc, &resource->targetColor ) ), false,
		               "%s: Failed to create color render target view", __FUNCTION__ );

		// Crate Shader Resource View
		DECL_ZERO( D3D11_SHADER_RESOURCE_VIEW_DESC, srvDesc );
		srvDesc.Format = D3D11ColorFormats[desc.colorFormat];
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;

		ErrorReturnIf( FAILED( device->CreateShaderResourceView( textureColor, &srvDesc, &resourceColor->view ) ), false,
		               "%s: Failed to create color shader resource view", __FUNCTION__ );

		// CPU Access
		if( desc.cpuAccess )
		{
			DECL_ZERO( D3D11_TEXTURE2D_DESC, stagingDesc );
			textureColor->GetDesc( &stagingDesc );
			stagingDesc.Usage = D3D11_USAGE_STAGING;
			stagingDesc.BindFlags = 0;
			stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

			ErrorReturnIf( FAILED( device->CreateTexture2D( &stagingDesc, nullptr, &resource->stagingColor ) ), false,
			               "%s: Failed to create CPU access staging texture for render target", __FUNCTION__ );
			PROFILE_GFX( Gfx::stats.gpuMemoryRenderTargets +=
				GFX_SIZE_IMAGE_COLOR_BYTES( width, height, 1, desc.colorFormat ) );
		}
	}

	// Depth
	if( desc.depthFormat != GfxDepthFormat_NONE )
	{
		// Register Resource
		resourceDepth = texture2DResources.make_new();

		// Create Texture2D
		DECL_ZERO( D3D11_TEXTURE2D_DESC, texDesc );
		texDesc.Width = width;
		texDesc.Height = height;
		texDesc.MipLevels = 1;
		texDesc.ArraySize = 1;
		texDesc.Format = D3D11DepthStencilFormats[desc.depthFormat].texture2DDescFormat;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		texDesc.CPUAccessFlags = 0;
		texDesc.MiscFlags = 0;

		ID3D11Texture2D *textureDepth = nullptr;
		ErrorReturnIf( FAILED( device->CreateTexture2D( &texDesc, nullptr, &textureDepth ) ), false,
		               "%s: Failed to create depth texture 2d", __FUNCTION__ );

		// Depth-Stencil View
		DECL_ZERO( D3D11_DEPTH_STENCIL_VIEW_DESC, dsvDesc );
		dsvDesc.Format = D3D11DepthStencilFormats[desc.depthFormat].depthStencilViewDescFormat;
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Flags = 0;
		dsvDesc.Texture2D.MipSlice = 0;

		Assert( resource->targetDepth == nullptr );
		ErrorReturnIf( FAILED( device->CreateDepthStencilView( textureDepth, &dsvDesc, &resource->targetDepth ) ), false,
		               "%s: Failed to create depth render target view", __FUNCTION__ );
		PROFILE_GFX( Gfx::stats.gpuMemoryRenderTargets +=
			GFX_SIZE_IMAGE_DEPTH_BYTES( width, height, 1, desc.depthFormat ) );

		// Shader Resource
		DECL_ZERO( D3D11_SHADER_RESOURCE_VIEW_DESC, srvDesc );
		srvDesc.Format = D3D11DepthStencilFormats[desc.depthFormat].shaderResourceViewDescFormat;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;

		ErrorReturnIf( FAILED( device->CreateShaderResourceView( textureDepth, &srvDesc, &resourceDepth->view ) ), false,
		               "%s: Failed to create depth shader resource view", __FUNCTION__ );

		// CPU Access
		/*
		if( desc.cpuAccess )
		{
			DECL_ZERO( D3D11_TEXTURE2D_DESC, stagingDesc );
			textureDepth->GetDesc( &stagingDesc );
			stagingDesc.Usage = D3D11_USAGE_STAGING;
			stagingDesc.BindFlags = 0;
			stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

			PROFILE_GFX( Gfx::stats.gpuMemoryTextures +=
				GFX_SIZE_IMAGE_DEPTH_BYTES( width, height, 1, desc.depthFormat ) );
			ErrorReturnIf( FAILED( device->CreateTexture2D( &stagingDesc, nullptr, &resource->stagingDepth ) ), false,
			               "%s: Failed to create CPU access staging texture for render target", __FUNCTION__ );
		}
		*/
	}

	// Success
	return true;
}


bool GfxCore::rb_render_target_2d_free( GfxRenderTarget2DResource *&resource,
	GfxTexture2DResource *&resourceColor,
	GfxTexture2DResource *&resourceDepth )
{
	// Depth
	if( resource->desc.depthFormat != GfxDepthFormat_NONE )
	{
		Assert( resourceDepth != nullptr && resourceDepth->id != GFX_RESOURCE_ID_NULL );

		resourceDepth->view->Release();
		PROFILE_GFX( Gfx::stats.gpuMemoryRenderTargets -=
			GFX_SIZE_IMAGE_DEPTH_BYTES( resource->width, resource->height, 1, resource->desc.depthFormat ) );
		resourceDepth->view = nullptr;

		texture2DResources.remove( resourceDepth->id );
		resourceDepth = nullptr;

		if( resource->desc.cpuAccess )
		{
			Assert( resource->stagingDepth != nullptr );

			resource->stagingDepth->Release();
			PROFILE_GFX( Gfx::stats.gpuMemoryRenderTargets -=
				GFX_SIZE_IMAGE_DEPTH_BYTES( resource->width, resource->height, 1, resource->desc.depthFormat ) );
			resource->stagingDepth = nullptr;
		}
	}

	// Color
	{
		Assert( resourceColor != nullptr && resourceColor->id != GFX_RESOURCE_ID_NULL );

		resourceColor->view->Release();
		PROFILE_GFX( Gfx::stats.gpuMemoryRenderTargets -=
			GFX_SIZE_IMAGE_COLOR_BYTES( resource->width, resource->height, 1, resource->desc.colorFormat ) );
		resourceColor->view = nullptr;

		texture2DResources.remove( resourceColor->id );
		resourceColor = nullptr;

		if( resource->desc.cpuAccess )
		{
			Assert( resource->stagingColor != nullptr );

			resource->stagingColor->Release();
			PROFILE_GFX( Gfx::stats.gpuMemoryRenderTargets -=
				GFX_SIZE_IMAGE_COLOR_BYTES( resource->width, resource->height, 1, resource->desc.colorFormat ) );
			resource->stagingColor = nullptr;
		}
	}

	// RenderTarget2D
	{
		Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
		resource->targetColor->Release();
		resource->targetColor = nullptr;
		if( resource->targetDepth != nullptr )
		{
			resource->targetDepth->Release();
			resource->targetDepth = nullptr;
		}
		renderTarget2DResources.remove( resource->id );
		resource = nullptr;
	}

	// Success
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

	// Success
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

	// Success
	return true;
}


bool GfxCore::rb_render_target_2d_buffer_read_color( GfxRenderTarget2DResource *&resource,
		GfxTexture2DResource *&resourceColor,
		void *buffer, const u32 size )
{
	ErrorIf( !resource->desc.cpuAccess, "Trying to CPU access a render target that does not have CPU access flag!" );
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( resource->stagingColor != nullptr );
	Assert( resourceColor != nullptr && resourceColor->id != GFX_RESOURCE_ID_NULL );

	const u32 sizeSource = GFX_SIZE_IMAGE_COLOR_BYTES( resource->width, resource->height, 1, resource->desc.colorFormat );
	Assert( size > 0 && sizeSource <= size );
	Assert( buffer != nullptr );

	// Copy Data to Staging Texture
	ID3D11Resource *srcResource;
	resourceColor->view->GetResource( &srcResource );
	context->CopyResource( resource->stagingColor, srcResource );
	srcResource->Release();

	// Map Staging Texture
	DECL_ZERO( D3D11_MAPPED_SUBRESOURCE, mappedResource );
	context->Map( resource->stagingColor, 0, D3D11_MAP_READ, 0, &mappedResource );

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
	context->Unmap( resource->stagingColor, 0 );

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


bool GfxCore::rb_render_target_2d_buffered_write_color( GfxRenderTarget2DResource *&resource,
	GfxTexture2DResource *&resourceColor,
	const void *const buffer, const u32 size )
{
	ErrorIf( !resource->desc.cpuAccess,
		"Trying to CPU access a render target that does not have CPU access flag!" );
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );
	Assert( resource->stagingColor != nullptr );
	Assert( resourceColor != nullptr && resourceColor->id != GFX_RESOURCE_ID_NULL );

	const u32 srcSize = resource->width * resource->height *
	                    GfxCore::colorFormatPixelSizeBytes[resource->desc.colorFormat];
	Assert( size > 0 && size <= srcSize );
	Assert( buffer != nullptr );

	// Map Staging Texture
	DECL_ZERO( D3D11_MAPPED_SUBRESOURCE, mappedResource );
	context->Map( resource->stagingColor, 0, D3D11_MAP_READ, 0, &mappedResource );

	// Copy Texture Data
	memory_copy( mappedResource.pData, buffer, srcSize );

	// Unmap Staging Texture
	context->Unmap( resource->stagingColor, 0 );

	// Copy Data to Staging Texture
	ID3D11Resource *dstResource;
	resourceColor->view->GetResource( &dstResource );
	context->CopyResource( dstResource, resource->stagingColor );
	dstResource->Release();

	// Success
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


bool GfxCore::rb_render_target_2d_release( const int slot )
{
	Assert( slot >= 0 && slot < GFX_RENDER_TARGET_SLOT_COUNT );

	RT_TEXTURE_COLOR[slot] = ( slot == 0 ) ? RT_TEXTURE_COLOR_CACHE : nullptr;
	if( slot == 0 ) { RT_TEXTURE_DEPTH = RT_TEXTURE_DEPTH_CACHE; }

	context->OMSetRenderTargets( GFX_RENDER_TARGET_SLOT_COUNT, RT_TEXTURE_COLOR, RT_TEXTURE_DEPTH );

	// Success
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GfxCore::rb_shader_init( GfxShaderResource *&resource, const u32 shaderID, const struct DiskShader &diskShader )
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
	const void *codeVertex = reinterpret_cast<const void *>( Assets::binary.data + diskShader.offsetVertex );
	if( FAILED( D3DCompile( codeVertex, diskShader.sizeVertex, nullptr, nullptr, nullptr,
	                        "vs_main", "vs_4_0", D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &vsCode, &info ) ) )
	{
		ErrorReturnMsg( false,
			"%s: Failed to compile vertex shader (%s)\n\n%s",
			__FUNCTION__, diskShader.name, reinterpret_cast<const char *>( info->GetBufferPointer() ) );
	}

	// Compile Fragment Shader
	const void *codeFragment = reinterpret_cast<const void *>( Assets::binary.data + diskShader.offsetFragment );
	if( FAILED( D3DCompile( codeFragment, diskShader.sizeFragment, nullptr, nullptr, nullptr,
	                        "ps_main", "ps_4_0", D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &psCode, &info ) ) )
	{
		ErrorReturnMsg( false,
			"%s: Failed to compile fragment shader (%s)\n\n%s",
			__FUNCTION__, diskShader.name, reinterpret_cast<const char *>( info->GetBufferPointer() ) );
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
	PROFILE_GFX( Gfx::stats.gpuMemoryShaderPrograms += resource->sizeVS );
	if( FAILED( device->CreateVertexShader( vsStripped->GetBufferPointer(),
	                                        vsStripped->GetBufferSize(), nullptr, &resource->vs ) ) )
	{
		ErrorReturnMsg( false, "%s: Failed to create vertex shader (%s)",
			__FUNCTION__, diskShader.name );
	}

	// Create Pixel Shader
	resource->sizePS = psStripped->GetBufferSize();
	PROFILE_GFX( Gfx::stats.gpuMemoryShaderPrograms += resource->sizePS );
	if( FAILED( device->CreatePixelShader( psStripped->GetBufferPointer(),
	                                       psStripped->GetBufferSize(), nullptr, &resource->ps ) ) )
	{
		ErrorReturnMsg( false, "%s: Failed to create pixel shader (%s)",
			__FUNCTION__, diskShader.name );
	}

	// Create Input Layout
	D3D11VertexInputLayoutDescription desc;
	GfxCore::d3d11_vertex_input_layout_desc[Gfx::diskShaders[shaderID].vertexFormat]( desc );
	if( FAILED( device->CreateInputLayout( desc.desc, desc.count,
	                                       vsStripped->GetBufferPointer(),
	                                       vsStripped->GetBufferSize(), &resource->il ) ) )
	{
		ErrorReturnMsg( false, "%s: Failed to create input layout (%s)",
			__FUNCTION__, diskShader.name );
	}

	// Success
	return true;
}


bool GfxCore::rb_shader_free( GfxShaderResource *&resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	PROFILE_GFX( Gfx::stats.gpuMemoryShaderPrograms -= resource->sizeVS );
	PROFILE_GFX( Gfx::stats.gpuMemoryShaderPrograms -= resource->sizePS );

	resource->vs->Release();
	resource->vs = nullptr;
	resource->ps->Release();
	resource->ps = nullptr;
	resource->il->Release();
	resource->il = nullptr;
	shaderResources.remove( resource->id );
	resource = nullptr;

	// Success
	return true;
}


bool GfxCore::rb_shader_bind( GfxShaderResource *&resource )
{
	Assert( resource != nullptr && resource->id != GFX_RESOURCE_ID_NULL );

	context->VSSetShader( resource->vs, nullptr, 0 );
	context->PSSetShader( resource->ps, nullptr, 0 );
	context->IASetInputLayout( resource->il );
	PROFILE_GFX( Gfx::stats.frame.shaderBinds++ );

	// Success
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////