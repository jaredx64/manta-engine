#pragma once
#include <vendor/config.hpp>

#if USE_OFFICIAL_HEADERS
	#include <vendor/conflicts.hpp>
		#include <windows.h>
		#include <gl/gl.h>
	#include <vendor/conflicts.hpp>
#else
	#include <vendor/vendor.hpp>
	#include <vendor/windows.hpp>

	#define PFD_DOUBLEBUFFER 0x01
	#define PFD_STEREO 0x02
	#define PFD_DRAW_TO_WINDOW 0x04
	#define PFD_SUPPORT_OPENGL 0x20
	#define PFD_DEPTH_DONTCARE 0x20000000
	#define PFD_TYPE_RGBA 0
	#define PFD_MAIN_PLANE 0

	#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
	#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
	#define WGL_CONTEXT_FLAGS_ARB 0x2094
	#define WGL_CONTEXT_PROFILE_MASK_ARB 0x9126
	#define WGL_CONTEXT_DEBUG_BIT_ARB 0x1
	#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x1

	struct PIXELFORMATDESCRIPTOR
	{
		WORD nSize;
		WORD nVersion;
		DWORD dwFlags;
		BYTE iPixelType;
		BYTE cColorBits;
		BYTE cRedBits;
		BYTE cRedShift;
		BYTE cGreenBits;
		BYTE cGreenShift;
		BYTE cBlueBits;
		BYTE cBlueShift;
		BYTE cAlphaBits;
		BYTE cAlphaShift;
		BYTE cAccumBits;
		BYTE cAccumRedBits;
		BYTE cAccumGreenBits;
		BYTE cAccumBlueBits;
		BYTE cAccumAlphaBits;
		BYTE cDepthBits;
		BYTE cStencilBits;
		BYTE cAuxBuffers;
		BYTE iLayerType;
		BYTE bReserved;
		DWORD dwLayerMask;
		DWORD dwVisibleMask;
		DWORD dwDamageMask;
	};

	// OPENGL32.dll
	extern "C" DLL_IMPORT HGLRC STD_CALL wglCreateContext(HDC);
	extern "C" DLL_IMPORT BOOL STD_CALL wglDeleteContext(HGLRC);
	extern "C" DLL_IMPORT PROC STD_CALL wglGetProcAddress(LPCSTR);
	extern "C" DLL_IMPORT BOOL STD_CALL wglMakeCurrent(HDC, HGLRC);

	// GDI32.dll
	extern "C" DLL_IMPORT int STD_CALL ChoosePixelFormat(HDC, const PIXELFORMATDESCRIPTOR *);
	extern "C" DLL_IMPORT BOOL STD_CALL SetPixelFormat(HDC, int, const PIXELFORMATDESCRIPTOR *);
	extern "C" DLL_IMPORT BOOL STD_CALL SwapBuffers(HDC);
#endif