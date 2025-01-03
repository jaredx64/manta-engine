#pragma once
#include <vendor/config.hpp>


#if USE_OFFICIAL_HEADERS
	#include <vendor/conflicts.hpp>
		#include <d3d11_1.h>
		#include <dxgi1_3.h>
		#include <windows.h>
	#include <vendor/conflicts.hpp>
#else
	#include <vendor/dxgi.hpp>

	#define	D3D11_SDK_VERSION 7
	#define D3D11_FLOAT32_MAX 3.402823466e+38f

	enum D3D_DRIVER_TYPE
	{
		D3D_DRIVER_TYPE_UNKNOWN,
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_REFERENCE,
		D3D_DRIVER_TYPE_NULL,
		D3D_DRIVER_TYPE_SOFTWARE,
		D3D_DRIVER_TYPE_WARP,
	};

	enum D3D11_CREATE_DEVICE_FLAG
	{
		D3D11_CREATE_DEVICE_SINGLETHREADED = 1,
		D3D11_CREATE_DEVICE_DEBUG = 2,
		D3D11_CREATE_DEVICE_SWITCH_TO_REF = 4,
		D3D11_CREATE_DEVICE_PREVENT_INTERNAL_THREADING_OPTIMIZATIONS = 8,
		D3D11_CREATE_DEVICE_BGRA_SUPPORT = 32,
		D3D11_CREATE_DEVICE_DEBUGGABLE = 64,
		D3D11_CREATE_DEVICE_PREVENT_ALTERING_LAYER_SETTINGS_FROM_REGISTRY = 128,
		D3D11_CREATE_DEVICE_DISABLE_GPU_TIMEOUT = 256,
		D3D11_CREATE_DEVICE_VIDEO_SUPPORT = 2048
	};

	enum D3D11_FEATURE
	{
		D3D11_FEATURE_THREADING,
		D3D11_FEATURE_DOUBLES,
	};

	enum D3D11_USAGE
	{
		D3D11_USAGE_DEFAULT,
		D3D11_USAGE_IMMUTABLE,
		D3D11_USAGE_DYNAMIC,
		D3D11_USAGE_STAGING,
	};

	enum D3D11_BIND_FLAG
	{
		D3D11_BIND_VERTEX_BUFFER = 1,
		D3D11_BIND_INDEX_BUFFER = 2,
		D3D11_BIND_CONSTANT_BUFFER = 4,
		D3D11_BIND_SHADER_RESOURCE = 8,
		D3D11_BIND_STREAM_OUTPUT = 16,
		D3D11_BIND_RENDER_TARGET = 32,
		D3D11_BIND_DEPTH_STENCIL = 64,
		D3D11_BIND_UNORDERED_ACCESS = 128,
		D3D11_BIND_DECODER = 512,
		D3D11_BIND_VIDEO_ENCODER = 1024,
	};

	enum D3D11_CPU_ACCESS_FLAG
	{
		D3D11_CPU_ACCESS_WRITE = 0x10000,
		D3D11_CPU_ACCESS_READ = 0x20000,
	};

	enum D3D11_INPUT_CLASSIFICATION
	{
		D3D11_INPUT_PER_VERTEX_DATA,
		D3D11_INPUT_PER_INSTANCE_DATA,
	};

	enum D3D11_COMPARISON_FUNC
	{
		D3D11_COMPARISON_NEVER = 1,
		D3D11_COMPARISON_LESS,
		D3D11_COMPARISON_EQUAL,
		D3D11_COMPARISON_LESS_EQUAL,
		D3D11_COMPARISON_GREATER,
		D3D11_COMPARISON_NOT_EQUAL,
		D3D11_COMPARISON_GREATER_EQUAL,
		D3D11_COMPARISON_ALWAYS,
	};

	enum D3D11_CULL_MODE
	{
		D3D11_CULL_NONE = 1,
		D3D11_CULL_FRONT,
		D3D11_CULL_BACK,
	};

	enum D3D11_FILL_MODE
	{
		D3D11_FILL_WIREFRAME = 2,
		D3D11_FILL_SOLID,
	};

	enum D3D11_FILTER
	{
		D3D11_FILTER_MIN_MAG_MIP_POINT = 0,
		D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR = 0x1,
		D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT = 0x4,
		D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR = 0x5,
		D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT = 0x10,
		D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR = 0x11,
		D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT = 0x14,
		D3D11_FILTER_MIN_MAG_MIP_LINEAR = 0x15,
		D3D11_FILTER_ANISOTROPIC = 0x55,
		D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT = 0x80,
		D3D11_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR = 0x81,
		D3D11_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT = 0x84,
		D3D11_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR = 0x85,
		D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT = 0x90,
		D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR = 0x91,
		D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT = 0x94,
		D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR = 0x95,
		D3D11_FILTER_COMPARISON_ANISOTROPIC = 0xD5,
		D3D11_FILTER_MINIMUM_MIN_MAG_MIP_POINT = 0x100,
		D3D11_FILTER_MINIMUM_MIN_MAG_POINT_MIP_LINEAR = 0x101,
		D3D11_FILTER_MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT = 0x104,
		D3D11_FILTER_MINIMUM_MIN_POINT_MAG_MIP_LINEAR = 0x105,
		D3D11_FILTER_MINIMUM_MIN_LINEAR_MAG_MIP_POINT = 0x110,
		D3D11_FILTER_MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR = 0x111,
		D3D11_FILTER_MINIMUM_MIN_MAG_LINEAR_MIP_POINT = 0x114,
		D3D11_FILTER_MINIMUM_MIN_MAG_MIP_LINEAR = 0x115,
		D3D11_FILTER_MINIMUM_ANISOTROPIC = 0x155,
		D3D11_FILTER_MAXIMUM_MIN_MAG_MIP_POINT = 0x180,
		D3D11_FILTER_MAXIMUM_MIN_MAG_POINT_MIP_LINEAR = 0x181,
		D3D11_FILTER_MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT = 0x184,
		D3D11_FILTER_MAXIMUM_MIN_POINT_MAG_MIP_LINEAR = 0x185,
		D3D11_FILTER_MAXIMUM_MIN_LINEAR_MAG_MIP_POINT = 0x190,
		D3D11_FILTER_MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR = 0x191,
		D3D11_FILTER_MAXIMUM_MIN_MAG_LINEAR_MIP_POINT = 0x194,
		D3D11_FILTER_MAXIMUM_MIN_MAG_MIP_LINEAR = 0x195,
		D3D11_FILTER_MAXIMUM_ANISOTROPIC = 0x1D5,
	};

	enum D3D11_TEXTURE_ADDRESS_MODE
	{
		D3D11_TEXTURE_ADDRESS_WRAP = 1,
		D3D11_TEXTURE_ADDRESS_MIRROR,
		D3D11_TEXTURE_ADDRESS_CLAMP,
		D3D11_TEXTURE_ADDRESS_BORDER,
		D3D11_TEXTURE_ADDRESS_MIRROR_ONCE,
	};

	enum D3D11_BLEND
	{
		D3D11_BLEND_ZERO = 1,
		D3D11_BLEND_ONE,
		D3D11_BLEND_SRC_COLOR,
		D3D11_BLEND_INV_SRC_COLOR,
		D3D11_BLEND_SRC_ALPHA,
		D3D11_BLEND_INV_SRC_ALPHA,
		D3D11_BLEND_DEST_ALPHA,
		D3D11_BLEND_INV_DEST_ALPHA,
		D3D11_BLEND_DEST_COLOR,
		D3D11_BLEND_INV_DEST_COLOR,
		D3D11_BLEND_SRC_ALPHA_SAT,
		D3D11_BLEND_BLEND_FACTOR = 14,
		D3D11_BLEND_INV_BLEND_FACTOR,
		D3D11_BLEND_SRC1_COLOR,
		D3D11_BLEND_INV_SRC1_COLOR,
		D3D11_BLEND_SRC1_ALPHA,
		D3D11_BLEND_INV_SRC1_ALPHA,
	};

	enum D3D11_BLEND_OP
	{
		D3D11_BLEND_OP_ADD = 1,
		D3D11_BLEND_OP_SUBTRACT,
		D3D11_BLEND_OP_REV_SUBTRACT,
		D3D11_BLEND_OP_MIN,
		D3D11_BLEND_OP_MAX,
	};

	enum D3D11_COLOR_WRITE_ENABLE
	{
		D3D11_COLOR_WRITE_ENABLE_RED = 1,
		D3D11_COLOR_WRITE_ENABLE_GREEN = 2,
		D3D11_COLOR_WRITE_ENABLE_BLUE = 4,
		D3D11_COLOR_WRITE_ENABLE_ALPHA = 8,
		D3D11_COLOR_WRITE_ENABLE_ALL = 15,
	};

	enum D3D11_MAP
	{
		D3D11_MAP_READ = 1,
		D3D11_MAP_WRITE,
		D3D11_MAP_READ_WRITE,
		D3D11_MAP_WRITE_DISCARD,
		D3D11_MAP_WRITE_NO_OVERWRITE,
	};

	enum D3D11_PRIMITIVE_TOPOLOGY
	{
		D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED,
		D3D11_PRIMITIVE_TOPOLOGY_POINTLIST,
		D3D11_PRIMITIVE_TOPOLOGY_LINELIST,
		D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP,
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,
		D3D11_PRIMITIVE_TOPOLOGY_LINELIST_ADJ = 10,
		D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ,
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ,
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ,
	};

	enum D3D11_DEVICE_CONTEXT_TYPE
	{
		D3D11_DEVICE_CONTEXT_IMMEDIATE,
		D3D11_DEVICE_CONTEXT_DEFERRED,
	};

	enum D3D11_COUNTER_TYPE
	{
		D3D11_COUNTER_TYPE_FLOAT32,
		D3D11_COUNTER_TYPE_UINT16,
		D3D11_COUNTER_TYPE_UINT32,
		D3D11_COUNTER_TYPE_UINT64,
	};

	enum D3D11_DEPTH_WRITE_MASK
	{
		D3D11_DEPTH_WRITE_MASK_ZERO,
		D3D11_DEPTH_WRITE_MASK_ALL,
	};

	enum D3D11_STENCIL_OP
	{
		D3D11_STENCIL_OP_KEEP = 1,
		D3D11_STENCIL_OP_ZERO,
		D3D11_STENCIL_OP_REPLACE,
		D3D11_STENCIL_OP_INCR_SAT,
		D3D11_STENCIL_OP_DECR_SAT,
		D3D11_STENCIL_OP_INVERT,
		D3D11_STENCIL_OP_INCR,
		D3D11_STENCIL_OP_DECR,
	};

	enum D3D11_SRV_DIMENSION
	{
		D3D11_SRV_DIMENSION_UNKNOWN,
		D3D11_SRV_DIMENSION_BUFFER,
		D3D11_SRV_DIMENSION_TEXTURE1D,
		D3D11_SRV_DIMENSION_TEXTURE1DARRAY,
		D3D11_SRV_DIMENSION_TEXTURE2D,
		D3D11_SRV_DIMENSION_TEXTURE2DARRAY,
		D3D11_SRV_DIMENSION_TEXTURE2DMS,
		D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY,
		D3D11_SRV_DIMENSION_TEXTURE3D,
		D3D11_SRV_DIMENSION_TEXTURECUBE,
		D3D11_SRV_DIMENSION_TEXTURECUBEARRAY,
		D3D11_SRV_DIMENSION_BUFFEREX,
	};

	enum D3D11_RTV_DIMENSION
	{
		D3D11_RTV_DIMENSION_UNKNOWN,
		D3D11_RTV_DIMENSION_BUFFER,
		D3D11_RTV_DIMENSION_TEXTURE1D,
		D3D11_RTV_DIMENSION_TEXTURE1DARRAY,
		D3D11_RTV_DIMENSION_TEXTURE2D,
		D3D11_RTV_DIMENSION_TEXTURE2DARRAY,
		D3D11_RTV_DIMENSION_TEXTURE2DMS,
		D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY,
		D3D11_RTV_DIMENSION_TEXTURE3D8
	};

	enum D3D11_CLEAR_FLAG
	{
		D3D11_CLEAR_DEPTH = 0x1L,
		D3D11_CLEAR_STENCIL = 0x2L,
	};

	enum D3D11_DSV_DIMENSION
	{
		D3D11_DSV_DIMENSION_UNKNOWN,
		D3D11_DSV_DIMENSION_TEXTURE1D,
		D3D11_DSV_DIMENSION_TEXTURE1DARRAY,
		D3D11_DSV_DIMENSION_TEXTURE2D,
		D3D11_DSV_DIMENSION_TEXTURE2DARRAY,
		D3D11_DSV_DIMENSION_TEXTURE2DMS,
		D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY6
	};

	using D3D11_RECT = RECT;

	struct ID3D11Asynchronous;
	struct ID3D11BlendState;
	struct ID3D11Buffer;
	struct ID3D11ClassInstance;
	struct ID3D11ClassLinkage;
	struct ID3D11CommandList;
	struct ID3D11ComputeShader;
	struct ID3D11Counter;
	struct ID3D11DepthStencilState;
	struct ID3D11DepthStencilView;
	struct ID3D11DeviceContext;
	struct ID3D11DomainShader;
	struct ID3D11GeometryShader;
	struct ID3D11HullShader;
	struct ID3D11InputLayout;
	struct ID3D11PixelShader;
	struct ID3D11Predicate;
	struct ID3D11Query;
	struct ID3D11RasterizerState;
	struct ID3D11RenderTargetView;
	struct ID3D11Resource;
	struct ID3D11SamplerState;
	struct ID3D11ShaderResourceView;
	struct ID3D11Texture1D;
	struct ID3D11Texture2D;
	struct ID3D11Texture3D;
	struct ID3D11UnorderedAccessView;
	struct ID3D11VertexShader;
	struct D3D11_COUNTER_DESC;
	struct D3D11_COUNTER_INFO;
	struct D3D11_QUERY_DESC;
	struct D3D11_RENDER_TARGET_VIEW_DESC;
	struct D3D11_RESOURCE_DIMENSION;
	struct D3D11_SO_DECLARATION_ENTRY;
	struct D3D11_TEXTURE1D_DESC;
	struct D3D11_TEXTURE3D_DESC;
	struct D3D11_UNORDERED_ACCESS_VIEW_DESC;

	struct D3D11_SUBRESOURCE_DATA
	{
		const void *pSysMem;
		UINT SysMemPitch;
		UINT SysMemSlicePitch;
	};

	struct D3D11_MAPPED_SUBRESOURCE
	{
		void *pData;
		UINT RowPitch;
		UINT DepthPitch;
	};

	struct D3D11_VIEWPORT
	{
		FLOAT TopLeftX;
		FLOAT TopLeftY;
		FLOAT Width;
		FLOAT Height;
		FLOAT MinDepth;
		FLOAT MaxDepth;
	};

	struct D3D11_RASTERIZER_DESC
	{
		D3D11_FILL_MODE FillMode;
		D3D11_CULL_MODE CullMode;
		BOOL FrontCounterClockwise;
		INT DepthBias;
		FLOAT DepthBiasClamp;
		FLOAT SlopeScaledDepthBias;
		BOOL DepthClipEnable;
		BOOL ScissorEnable;
		BOOL MultisampleEnable;
		BOOL AntialiasedLineEnable;
	};

	struct D3D11_RENDER_TARGET_BLEND_DESC
	{
		BOOL BlendEnable;
		D3D11_BLEND SrcBlend;
		D3D11_BLEND DestBlend;
		D3D11_BLEND_OP BlendOp;
		D3D11_BLEND SrcBlendAlpha;
		D3D11_BLEND DestBlendAlpha;
		D3D11_BLEND_OP BlendOpAlpha;
		UINT8 RenderTargetWriteMask;
	};

	struct D3D11_SAMPLER_DESC
	{
		D3D11_FILTER Filter;
		D3D11_TEXTURE_ADDRESS_MODE AddressU;
		D3D11_TEXTURE_ADDRESS_MODE AddressV;
		D3D11_TEXTURE_ADDRESS_MODE AddressW;
		FLOAT MipLODBias;
		UINT MaxAnisotropy;
		D3D11_COMPARISON_FUNC ComparisonFunc;
		FLOAT BorderColor[4];
		FLOAT MinLOD;
		FLOAT MaxLOD;
	};

	struct D3D11_BLEND_DESC
	{
		BOOL AlphaToCoverageEnable;
		BOOL IndependentBlendEnable;
		D3D11_RENDER_TARGET_BLEND_DESC RenderTarget[8];
	};

	struct D3D11_BUFFER_DESC
	{
		UINT ByteWidth;
		D3D11_USAGE Usage;
		UINT BindFlags;
		UINT CPUAccessFlags;
		UINT MiscFlags;
		UINT StructureByteStride;
	};

	struct D3D11_TEXTURE2D_DESC
	{
		UINT Width;
		UINT Height;
		UINT MipLevels;
		UINT ArraySize;
		DXGI_FORMAT Format;
		DXGI_SAMPLE_DESC SampleDesc;
		D3D11_USAGE Usage;
		UINT BindFlags;
		UINT CPUAccessFlags;
		UINT MiscFlags;
	};

	struct D3D11_INPUT_ELEMENT_DESC
	{
		LPCSTR SemanticName;
		UINT SemanticIndex;
		DXGI_FORMAT Format;
		UINT InputSlot;
		UINT AlignedByteOffset;
		D3D11_INPUT_CLASSIFICATION InputSlotClass;
		UINT InstanceDataStepRate;
	};

	struct D3D11_DEPTH_STENCILOP_DESC
	{
		D3D11_STENCIL_OP StencilFailOp;
		D3D11_STENCIL_OP StencilDepthFailOp;
		D3D11_STENCIL_OP StencilPassOp;
		D3D11_COMPARISON_FUNC StencilFunc;
	};

	struct D3D11_DEPTH_STENCIL_DESC
	{
		BOOL DepthEnable;
		D3D11_DEPTH_WRITE_MASK DepthWriteMask;
		D3D11_COMPARISON_FUNC DepthFunc;
		BOOL StencilEnable;
		UINT8 StencilReadMask;
		UINT8 StencilWriteMask;
		D3D11_DEPTH_STENCILOP_DESC FrontFace;
		D3D11_DEPTH_STENCILOP_DESC BackFace;
	};

	struct D3D11_TEX2D_SRV
	{
		UINT MostDetailedMip;
		UINT MipLevels;
	};

	struct D3D11_SHADER_RESOURCE_VIEW_DESC
	{
		DXGI_FORMAT Format;
		D3D11_SRV_DIMENSION ViewDimension;

		union
		{
			D3D11_TEX2D_SRV Texture2D;
		};
	};

	struct D3D11_TEX2D_RTV
	{
		UINT MipSlice;
	};

	struct D3D11_RENDER_TARGET_VIEW_DESC
	{
		DXGI_FORMAT Format;
		D3D11_RTV_DIMENSION ViewDimension;
		union
		{
			D3D11_TEX2D_RTV Texture2D;
		};
	};

	struct D3D11_TEX2D_DSV
	{
		UINT MipSlice;
	};

	struct D3D11_DEPTH_STENCIL_VIEW_DESC
	{
		DXGI_FORMAT Format;
		D3D11_DSV_DIMENSION ViewDimension;
		UINT Flags;
		union
		{
			D3D11_TEX2D_DSV Texture2D;
		};
	};

	struct D3D11_BOX
	{
		UINT left;
		UINT top;
		UINT front;
		UINT right;
		UINT bottom;
		UINT back;
	};

	MIDL_INTERFACE ID3D11Device : IUnknown
	{
		virtual HRESULT STD_CALL CreateBuffer(const D3D11_BUFFER_DESC *, const D3D11_SUBRESOURCE_DATA *, ID3D11Buffer **) = 0;
		virtual HRESULT STD_CALL CreateTexture1D(const D3D11_TEXTURE1D_DESC *, const D3D11_SUBRESOURCE_DATA *, ID3D11Texture1D **) = 0;
		virtual HRESULT STD_CALL CreateTexture2D(const D3D11_TEXTURE2D_DESC *, const D3D11_SUBRESOURCE_DATA *, ID3D11Texture2D **) = 0;
		virtual HRESULT STD_CALL CreateTexture3D(const D3D11_TEXTURE3D_DESC *, const D3D11_SUBRESOURCE_DATA *, ID3D11Texture3D **) = 0;
		virtual HRESULT STD_CALL CreateShaderResourceView(ID3D11Resource *, const D3D11_SHADER_RESOURCE_VIEW_DESC *, ID3D11ShaderResourceView **) = 0;
		virtual HRESULT STD_CALL CreateUnorderedAccessView(ID3D11Resource *, const D3D11_UNORDERED_ACCESS_VIEW_DESC *, ID3D11UnorderedAccessView **) = 0;
		virtual HRESULT STD_CALL CreateRenderTargetView(ID3D11Resource *, const D3D11_RENDER_TARGET_VIEW_DESC *, ID3D11RenderTargetView **) = 0;
		virtual HRESULT STD_CALL CreateDepthStencilView(ID3D11Resource *, const D3D11_DEPTH_STENCIL_VIEW_DESC *, ID3D11DepthStencilView **) = 0;
		virtual HRESULT STD_CALL CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC *, UINT, const void *, SIZE_T, ID3D11InputLayout **) = 0;
		virtual HRESULT STD_CALL CreateVertexShader(const void *, SIZE_T, ID3D11ClassLinkage *, ID3D11VertexShader **) = 0;
		virtual HRESULT STD_CALL CreateGeometryShader(const void *, SIZE_T, ID3D11ClassLinkage *, ID3D11GeometryShader **) = 0;
		virtual HRESULT STD_CALL CreateGeometryShaderWithStreamOutput(const void *, SIZE_T, const D3D11_SO_DECLARATION_ENTRY *, UINT, const UINT *, UINT, UINT, ID3D11ClassLinkage *, ID3D11GeometryShader **) = 0;
		virtual HRESULT STD_CALL CreatePixelShader(const void *, SIZE_T, ID3D11ClassLinkage *, ID3D11PixelShader **) = 0;
		virtual HRESULT STD_CALL CreateHullShader(const void *, SIZE_T, ID3D11ClassLinkage *, ID3D11HullShader **) = 0;
		virtual HRESULT STD_CALL CreateDomainShader(const void *, SIZE_T, ID3D11ClassLinkage *, ID3D11DomainShader **) = 0;
		virtual HRESULT STD_CALL CreateComputeShader(const void *, SIZE_T, ID3D11ClassLinkage *, ID3D11ComputeShader **) = 0;
		virtual HRESULT STD_CALL CreateClassLinkage(ID3D11ClassLinkage **) = 0;
		virtual HRESULT STD_CALL CreateBlendState(const D3D11_BLEND_DESC *, ID3D11BlendState **) = 0;
		virtual HRESULT STD_CALL CreateDepthStencilState(const D3D11_DEPTH_STENCIL_DESC *, ID3D11DepthStencilState **) = 0;
		virtual HRESULT STD_CALL CreateRasterizerState(const D3D11_RASTERIZER_DESC *, ID3D11RasterizerState **) = 0;
		virtual HRESULT STD_CALL CreateSamplerState(const D3D11_SAMPLER_DESC *, ID3D11SamplerState **) = 0;
		virtual HRESULT STD_CALL CreateQuery(const D3D11_QUERY_DESC *, ID3D11Query **) = 0;
		virtual HRESULT STD_CALL CreatePredicate(const D3D11_QUERY_DESC *, ID3D11Predicate **) = 0;
		virtual HRESULT STD_CALL CreateCounter(const D3D11_COUNTER_DESC *, ID3D11Counter **) = 0;
		virtual HRESULT STD_CALL CreateDeferredContext(UINT, ID3D11DeviceContext **) = 0;
		virtual HRESULT STD_CALL OpenSharedResource(HANDLE, const GUID &, void **) = 0;
		virtual HRESULT STD_CALL CheckFormatSupport(DXGI_FORMAT, UINT *) = 0;
		virtual HRESULT STD_CALL CheckMultisampleQualityLevels(DXGI_FORMAT, UINT, UINT *) = 0;
		virtual void STD_CALL CheckCounterInfo(D3D11_COUNTER_INFO *) = 0;
		virtual HRESULT STD_CALL CheckCounter(const D3D11_COUNTER_DESC *, D3D11_COUNTER_TYPE *, UINT *, LPSTR, UINT *, LPSTR, UINT *, LPSTR, UINT *) = 0;
		virtual HRESULT STD_CALL CheckFeatureSupport(D3D11_FEATURE, void *, UINT) = 0;
		virtual HRESULT STD_CALL GetPrivateData(const GUID &, UINT *, void *) = 0;
		virtual HRESULT STD_CALL SetPrivateData(const GUID &, UINT, const void *) = 0;
		virtual HRESULT STD_CALL SetPrivateDataInterface(const GUID &, const IUnknown *) = 0;
		virtual D3D_FEATURE_LEVEL STD_CALL GetFeatureLevel() = 0;
		virtual UINT STD_CALL GetCreationFlags() = 0;
		virtual HRESULT STD_CALL GetDeviceRemovedReason() = 0;
		virtual void STD_CALL GetImmediateContext(ID3D11DeviceContext **) = 0;
		virtual HRESULT STD_CALL SetExceptionMode(UINT) = 0;
		virtual UINT STD_CALL GetExceptionMode() = 0;
	};

	MIDL_INTERFACE ID3D11DeviceChild : IUnknown
	{
		virtual void STD_CALL GetDevice(ID3D11Device **) = 0;
		virtual HRESULT STD_CALL GetPrivateData(const GUID &, UINT *, void *) = 0;
		virtual HRESULT STD_CALL SetPrivateData(const GUID &, UINT, const void *) = 0;
		virtual HRESULT STD_CALL SetPrivateDataInterface(const GUID &, const IUnknown *) = 0;
	};

	MIDL_INTERFACE ID3D11DeviceContext : ID3D11DeviceChild
	{
		virtual void STD_CALL VSSetConstantBuffers(UINT, UINT, ID3D11Buffer *const *) = 0;
		virtual void STD_CALL PSSetShaderResources(UINT, UINT, ID3D11ShaderResourceView *const *) = 0;
		virtual void STD_CALL PSSetShader(ID3D11PixelShader *, ID3D11ClassInstance *const *, UINT) = 0;
		virtual void STD_CALL PSSetSamplers(UINT, UINT, ID3D11SamplerState *const *) = 0;
		virtual void STD_CALL VSSetShader(ID3D11VertexShader *, ID3D11ClassInstance *const *, UINT) = 0;
		virtual void STD_CALL DrawIndexed(UINT, UINT, INT) = 0;
		virtual void STD_CALL Draw(UINT, UINT) = 0;
		virtual HRESULT STD_CALL Map(ID3D11Resource *, UINT, D3D11_MAP, UINT, D3D11_MAPPED_SUBRESOURCE *) = 0;
		virtual void STD_CALL Unmap(ID3D11Resource *, UINT) = 0;
		virtual void STD_CALL PSSetConstantBuffers(UINT, UINT, ID3D11Buffer *const *) = 0;
		virtual void STD_CALL IASetInputLayout(ID3D11InputLayout *) = 0;
		virtual void STD_CALL IASetVertexBuffers(UINT, UINT, ID3D11Buffer *const *, const UINT *, const UINT *) = 0;
		virtual void STD_CALL IASetIndexBuffer(ID3D11Buffer *, DXGI_FORMAT, UINT) = 0;
		virtual void STD_CALL DrawIndexedInstanced(UINT, UINT, UINT, INT, UINT) = 0;
		virtual void STD_CALL DrawInstanced(UINT, UINT, UINT, UINT) = 0;
		virtual void STD_CALL GSSetConstantBuffers(UINT, UINT, ID3D11Buffer *const *) = 0;
		virtual void STD_CALL GSSetShader(ID3D11GeometryShader *, ID3D11ClassInstance *const *, UINT) = 0;
		virtual void STD_CALL IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY) = 0;
		virtual void STD_CALL VSSetShaderResources(UINT, UINT, ID3D11ShaderResourceView *const *) = 0;
		virtual void STD_CALL VSSetSamplers(UINT, UINT, ID3D11SamplerState *const *) = 0;
		virtual void STD_CALL Begin(ID3D11Asynchronous *) = 0;
		virtual void STD_CALL End(ID3D11Asynchronous *) = 0;
		virtual HRESULT STD_CALL GetData(ID3D11Asynchronous *, void *, UINT, UINT) = 0;
		virtual void STD_CALL SetPredication(ID3D11Predicate *, BOOL) = 0;
		virtual void STD_CALL GSSetShaderResources(UINT, UINT, ID3D11ShaderResourceView *const *) = 0;
		virtual void STD_CALL GSSetSamplers(UINT, UINT, ID3D11SamplerState *const *) = 0;
		virtual void STD_CALL OMSetRenderTargets(UINT, ID3D11RenderTargetView *const *, ID3D11DepthStencilView *) = 0;
		virtual void STD_CALL OMSetRenderTargetsAndUnorderedAccessViews(UINT, ID3D11RenderTargetView *const *, ID3D11DepthStencilView *, UINT, UINT, ID3D11UnorderedAccessView *const *, const UINT *) = 0;
		virtual void STD_CALL OMSetBlendState(ID3D11BlendState *, const FLOAT[4], UINT) = 0;
		virtual void STD_CALL OMSetDepthStencilState(ID3D11DepthStencilState *, UINT) = 0;
		virtual void STD_CALL SOSetTargets(UINT, ID3D11Buffer *const *, const UINT *) = 0;
		virtual void STD_CALL DrawAuto() = 0;
		virtual void STD_CALL DrawIndexedInstancedIndirect(ID3D11Buffer *, UINT) = 0;
		virtual void STD_CALL DrawInstancedIndirect(ID3D11Buffer *, UINT) = 0;
		virtual void STD_CALL Dispatch(UINT, UINT, UINT) = 0;
		virtual void STD_CALL DispatchIndirect(ID3D11Buffer *, UINT) = 0;
		virtual void STD_CALL RSSetState(ID3D11RasterizerState *) = 0;
		virtual void STD_CALL RSSetViewports(UINT, const D3D11_VIEWPORT *) = 0;
		virtual void STD_CALL RSSetScissorRects(UINT, const D3D11_RECT *) = 0;
		virtual void STD_CALL CopySubresourceRegion(ID3D11Resource *, UINT, UINT, UINT, UINT, ID3D11Resource *, UINT, const D3D11_BOX *) = 0;
		virtual void STD_CALL CopyResource(ID3D11Resource *, ID3D11Resource *) = 0;
		virtual void STD_CALL UpdateSubresource(ID3D11Resource *, UINT, const D3D11_BOX *, const void *, UINT, UINT) = 0;
		virtual void STD_CALL CopyStructureCount(ID3D11Buffer *, UINT, ID3D11UnorderedAccessView *) = 0;
		virtual void STD_CALL ClearRenderTargetView(ID3D11RenderTargetView *, const FLOAT[4]) = 0;
		virtual void STD_CALL ClearUnorderedAccessViewUint(ID3D11UnorderedAccessView *, const UINT[4]) = 0;
		virtual void STD_CALL ClearUnorderedAccessViewFloat(ID3D11UnorderedAccessView *, const FLOAT[4]) = 0;
		virtual void STD_CALL ClearDepthStencilView(ID3D11DepthStencilView *, UINT, FLOAT, UINT8) = 0;
		virtual void STD_CALL GenerateMips(ID3D11ShaderResourceView *) = 0;
		virtual void STD_CALL SetResourceMinLOD(ID3D11Resource *, FLOAT) = 0;
		virtual FLOAT STD_CALL GetResourceMinLOD(ID3D11Resource *) = 0;
		virtual void STD_CALL ResolveSubresource(ID3D11Resource *, UINT, ID3D11Resource *, UINT, DXGI_FORMAT) = 0;
		virtual void STD_CALL ExecuteCommandList(ID3D11CommandList *, BOOL) = 0;
		virtual void STD_CALL HSSetShaderResources(UINT, UINT, ID3D11ShaderResourceView *const *) = 0;
		virtual void STD_CALL HSSetShader(ID3D11HullShader *, ID3D11ClassInstance *const *, UINT) = 0;
		virtual void STD_CALL HSSetSamplers(UINT, UINT, ID3D11SamplerState *const *) = 0;
		virtual void STD_CALL HSSetConstantBuffers(UINT, UINT, ID3D11Buffer *const *) = 0;
		virtual void STD_CALL DSSetShaderResources(UINT, UINT, ID3D11ShaderResourceView *const *) = 0;
		virtual void STD_CALL DSSetShader(ID3D11DomainShader *, ID3D11ClassInstance *const *, UINT) = 0;
		virtual void STD_CALL DSSetSamplers(UINT, UINT, ID3D11SamplerState *const *) = 0;
		virtual void STD_CALL DSSetConstantBuffers(UINT, UINT, ID3D11Buffer *const *) = 0;
		virtual void STD_CALL CSSetShaderResources(UINT, UINT, ID3D11ShaderResourceView *const *) = 0;
		virtual void STD_CALL CSSetUnorderedAccessViews(UINT, UINT, ID3D11UnorderedAccessView *const *, const UINT *) = 0;
		virtual void STD_CALL CSSetShader(ID3D11ComputeShader *, ID3D11ClassInstance *const *, UINT) = 0;
		virtual void STD_CALL CSSetSamplers(UINT, UINT, ID3D11SamplerState *const *) = 0;
		virtual void STD_CALL CSSetConstantBuffers(UINT, UINT, ID3D11Buffer *const *) = 0;
		virtual void STD_CALL VSGetConstantBuffers(UINT, UINT, ID3D11Buffer **) = 0;
		virtual void STD_CALL PSGetShaderResources(UINT, UINT, ID3D11ShaderResourceView **) = 0;
		virtual void STD_CALL PSGetShader(ID3D11PixelShader **, ID3D11ClassInstance **, UINT *) = 0;
		virtual void STD_CALL PSGetSamplers(UINT, UINT, ID3D11SamplerState **) = 0;
		virtual void STD_CALL VSGetShader(ID3D11VertexShader **, ID3D11ClassInstance **, UINT *) = 0;
		virtual void STD_CALL PSGetConstantBuffers(UINT, UINT, ID3D11Buffer **) = 0;
		virtual void STD_CALL IAGetInputLayout(ID3D11InputLayout **) = 0;
		virtual void STD_CALL IAGetVertexBuffers(UINT, UINT, ID3D11Buffer **, UINT *, UINT *) = 0;
		virtual void STD_CALL IAGetIndexBuffer(ID3D11Buffer **, DXGI_FORMAT *, UINT *) = 0;
		virtual void STD_CALL GSGetConstantBuffers(UINT, UINT, ID3D11Buffer **) = 0;
		virtual void STD_CALL GSGetShader(ID3D11GeometryShader **, ID3D11ClassInstance **, UINT *) = 0;
		virtual void STD_CALL IAGetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY *) = 0;
		virtual void STD_CALL VSGetShaderResources(UINT, UINT, ID3D11ShaderResourceView **) = 0;
		virtual void STD_CALL VSGetSamplers(UINT, UINT, ID3D11SamplerState **) = 0;
		virtual void STD_CALL GetPredication(ID3D11Predicate **, BOOL *) = 0;
		virtual void STD_CALL GSGetShaderResources(UINT, UINT, ID3D11ShaderResourceView **) = 0;
		virtual void STD_CALL GSGetSamplers(UINT, UINT, ID3D11SamplerState **) = 0;
		virtual void STD_CALL OMGetRenderTargets(UINT, ID3D11RenderTargetView **, ID3D11DepthStencilView **) = 0;
		virtual void STD_CALL OMGetRenderTargetsAndUnorderedAccessViews(UINT, ID3D11RenderTargetView **, ID3D11DepthStencilView **, UINT, UINT, ID3D11UnorderedAccessView **) = 0;
		virtual void STD_CALL OMGetBlendState(ID3D11BlendState **, FLOAT[4], UINT *) = 0;
		virtual void STD_CALL OMGetDepthStencilState(ID3D11DepthStencilState **, UINT *) = 0;
		virtual void STD_CALL SOGetTargets(UINT, ID3D11Buffer **) = 0;
		virtual void STD_CALL RSGetState(ID3D11RasterizerState **) = 0;
		virtual void STD_CALL RSGetViewports(UINT *, D3D11_VIEWPORT *) = 0;
		virtual void STD_CALL RSGetScissorRects(UINT *, D3D11_RECT *) = 0;
		virtual void STD_CALL HSGetShaderResources(UINT, UINT, ID3D11ShaderResourceView **) = 0;
		virtual void STD_CALL HSGetShader(ID3D11HullShader **, ID3D11ClassInstance **, UINT *) = 0;
		virtual void STD_CALL HSGetSamplers(UINT, UINT, ID3D11SamplerState **) = 0;
		virtual void STD_CALL HSGetConstantBuffers(UINT, UINT, ID3D11Buffer **) = 0;
		virtual void STD_CALL DSGetShaderResources(UINT, UINT, ID3D11ShaderResourceView **) = 0;
		virtual void STD_CALL DSGetShader(ID3D11DomainShader **, ID3D11ClassInstance **, UINT *) = 0;
		virtual void STD_CALL DSGetSamplers(UINT, UINT, ID3D11SamplerState **) = 0;
		virtual void STD_CALL DSGetConstantBuffers(UINT, UINT, ID3D11Buffer **) = 0;
		virtual void STD_CALL CSGetShaderResources(UINT, UINT, ID3D11ShaderResourceView **) = 0;
		virtual void STD_CALL CSGetUnorderedAccessViews(UINT, UINT, ID3D11UnorderedAccessView **) = 0;
		virtual void STD_CALL CSGetShader(ID3D11ComputeShader **, ID3D11ClassInstance **, UINT *) = 0;
		virtual void STD_CALL CSGetSamplers(UINT, UINT, ID3D11SamplerState **) = 0;
		virtual void STD_CALL CSGetConstantBuffers(UINT, UINT, ID3D11Buffer **) = 0;
		virtual void STD_CALL ClearState() = 0;
		virtual void STD_CALL Flush() = 0;
		virtual D3D11_DEVICE_CONTEXT_TYPE STD_CALL GetType() = 0;
		virtual UINT STD_CALL GetContextFlags() = 0;
		virtual HRESULT STD_CALL FinishCommandList(BOOL, ID3D11CommandList **) = 0;
	};

	MIDL_INTERFACE ID3D11Resource : ID3D11DeviceChild
	{
		virtual void STD_CALL GetType(D3D11_RESOURCE_DIMENSION *) = 0;
		virtual void STD_CALL SetEvictionPriority(UINT) = 0;
		virtual UINT STD_CALL GetEvictionPriority() = 0;
	};

	MIDL_INTERFACE ID3D11Buffer : ID3D11Resource
	{
		virtual void STD_CALL GetDesc(D3D11_BUFFER_DESC *) = 0;
	};

	MIDL_INTERFACE ID3D11Texture2D : ID3D11Resource
	{
		virtual void STD_CALL GetDesc(D3D11_TEXTURE2D_DESC *) = 0;
	};

	MIDL_INTERFACE ID3D11VertexShader : ID3D11DeviceChild
	{
	};

	MIDL_INTERFACE ID3D11PixelShader : ID3D11DeviceChild
	{
	};

	MIDL_INTERFACE ID3D11InputLayout : ID3D11DeviceChild
	{
	};

	MIDL_INTERFACE ID3D11View : ID3D11DeviceChild
	{
		virtual void STD_CALL GetResource(ID3D11Resource **) = 0;
	};

	MIDL_INTERFACE ID3D11RenderTargetView : ID3D11View
	{
		virtual void STD_CALL GetDesc(D3D11_RENDER_TARGET_VIEW_DESC *) = 0;
	};

	MIDL_INTERFACE ID3D11ShaderResourceView : ID3D11View
	{
		virtual void STD_CALL GetDesc(D3D11_SHADER_RESOURCE_VIEW_DESC *) = 0;
	};

	MIDL_INTERFACE ID3D11RasterizerState : ID3D11DeviceChild
	{
		virtual void STD_CALL GetDesc(D3D11_RASTERIZER_DESC *) = 0;
	};

	MIDL_INTERFACE ID3D11SamplerState : ID3D11DeviceChild
	{
		virtual void STD_CALL GetDesc(D3D11_SAMPLER_DESC *) = 0;
	};

	MIDL_INTERFACE ID3D11BlendState : ID3D11DeviceChild
	{
		virtual void STD_CALL GetDesc(D3D11_BLEND_DESC *) = 0;
	};

	MIDL_INTERFACE ID3D11DepthStencilView : ID3D11View
	{
		virtual void STD_CALL GetDesc(D3D11_DEPTH_STENCIL_VIEW_DESC *) = 0;
	};

	extern "C" DLL_IMPORT  HRESULT STD_CALL D3D11CreateDevice(IDXGIAdapter *, D3D_DRIVER_TYPE, HMODULE, UINT, const D3D_FEATURE_LEVEL *, UINT, UINT, ID3D11Device **, D3D_FEATURE_LEVEL *, ID3D11DeviceContext **);
#endif