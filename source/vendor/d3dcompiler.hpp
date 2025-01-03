#pragma once
#include <vendor/config.hpp>

#if USE_OFFICIAL_HEADERS
	#include <vendor/conflicts.hpp>
		#include <d3dcompiler.h>
	#include <vendor/conflicts.hpp>
#else
	#include <vendor/com.hpp>

	#define D3DCOMPILE_OPTIMIZATION_LEVEL3 ( 1 << 15 )

	enum D3DCOMPILER_STRIP_FLAGS
	{
		D3DCOMPILER_STRIP_REFLECTION_DATA = 0x1,
		D3DCOMPILER_STRIP_DEBUG_INFO = 0x2,
		D3DCOMPILER_STRIP_PRIVATE_DATA = 0x8,
	};

	MIDL_INTERFACE ID3D10Blob : IUnknown
	{
		virtual void * STD_CALL GetBufferPointer() = 0;
		virtual SIZE_T STD_CALL GetBufferSize() = 0;
	};

	extern "C" DLL_IMPORT HRESULT STD_CALL D3DCompile(const void *, SIZE_T, LPCSTR, const struct D3D_SHADER_MACRO *, struct ID3DInclude *, LPCSTR, LPCSTR, UINT, UINT, ID3D10Blob **, ID3D10Blob **);
	extern "C" DLL_IMPORT HRESULT STD_CALL D3DStripShader(const void *, SIZE_T, UINT, ID3D10Blob **);
#endif