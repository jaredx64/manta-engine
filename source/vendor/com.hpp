#pragma once
#include <vendor/config.hpp>

#include <vendor/windows.hpp>

#if PIPELINE_COMPILER_MSVC
	#define MIDL_INTERFACE struct __declspec(novtable)
#else
	#define MIDL_INTERFACE struct
#endif

#define CLSCTX_ALL 0x17

enum COINIT
{
	COINIT_APARTMENTTHREADED = 0x2,
	COINIT_DISABLE_OLE1DDE = 0x4,
	COINIT_SPEED_OVER_MEMORY = 0x8,
};

struct GUID
{
	unsigned int Data1;
	unsigned short Data2;
	unsigned short Data3;
	unsigned char Data4[8];
};

MIDL_INTERFACE IUnknown
{
	virtual HRESULT STD_CALL QueryInterface(const GUID &, void **) = 0;
	virtual ULONG STD_CALL AddRef() = 0;
	virtual ULONG STD_CALL Release() = 0;
};

extern "C" DLL_IMPORT HRESULT STD_CALL CoInitializeEx(void *, DWORD);
extern "C" DLL_IMPORT HRESULT STD_CALL CoCreateInstance(const GUID &, IUnknown *, DWORD, const GUID &, void **);