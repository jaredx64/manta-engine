#pragma once
#include <vendor/config.hpp>

#include <vendor/x11.hpp>

#if !USE_CUSTOM_C_HEADERS
	#include <vendor/conflicts.hpp>
		#include <GL/glx.h>
	#include <vendor/conflicts.hpp>
#else
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// glxext.h

	#define GLX_CONTEXT_MAJOR_VERSION_ARB 0x2091
	#define GLX_CONTEXT_MINOR_VERSION_ARB 0x2092
	#define GLX_CONTEXT_FLAGS_ARB 0x2094
	#define GLX_CONTEXT_PROFILE_MASK_ARB 0x9126
	#define GLX_CONTEXT_DEBUG_BIT_ARB 0x01
	#define GLX_CONTEXT_CORE_PROFILE_BIT_ARB 0x01

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// glx.h

	#define GLX_X_RENDERABLE 0x8012
	#define GLX_DRAWABLE_TYPE 0x8010
	#define GLX_RENDER_TYPE 0x8011
	#define GLX_X_VISUAL_TYPE 0x22
	#define GLX_RED_SIZE 8
	#define GLX_GREEN_SIZE 9
	#define GLX_BLUE_SIZE 10
	#define GLX_ALPHA_SIZE 11
	#define GLX_DEPTH_SIZE 12
	#define GLX_STENCIL_SIZE 13
	#define GLX_DOUBLEBUFFER 5
	#define GLX_WINDOW_BIT 1
	#define GLX_RGBA_BIT 1
	#define GLX_TRUE_COLOR 0x8002
	#define GLX_SAMPLE_BUFFERS 100000
	#define GLX_SAMPLES 100001

	using GLXDrawable = XID;
	using GLXContext = void *;
	using GLXFBConfig = void *;

	extern "C" GLXFBConfig *glXChooseFBConfig( XDisplay *, int, const int *, int * );
	extern "C" void *glXGetProcAddress( const unsigned char * );
	extern "C" XVisualInfo *glXGetVisualFromFBConfig( XDisplay *, GLXFBConfig );
	extern "C" void glXSwapBuffers( XDisplay *, GLXDrawable );
	extern "C" int glXMakeCurrent( XDisplay *, GLXDrawable, GLXContext );
	extern "C" int glXQueryVersion( XDisplay *, int *, int * );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif