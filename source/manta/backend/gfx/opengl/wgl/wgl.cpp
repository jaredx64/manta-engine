#include <manta/backend/gfx/opengl/opengl.hpp>

#include <config.hpp>

#include <vendor/windows.hpp>
#include <vendor/wgl.hpp>

#include <core/debug.hpp>
#include <core/types.hpp>

#include <manta/window.hpp>
#include <manta/backend/window/windows/window.windows.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Define WGL Constants
#define WGL_CONTEXT_MAJOR_VERSION_ARB    0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB    0x2092
#define WGL_CONTEXT_FLAGS_ARB            0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB     0x9126
#define WGL_CONTEXT_DEBUG_BIT_ARB        0x1
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x1

// Define WGL Procedures
#undef  META
#define META(type, name, ...)                         \
	using  name##proc = type (GL_API *)(__VA_ARGS__); \
	static name##proc name;

#include "wgl.procedures.hpp"

// Win32 Device Context
static HDC device;

// The pixel format descriptor.
static const PIXELFORMATDESCRIPTOR pfd
{
    sizeof( PIXELFORMATDESCRIPTOR ),
    1,
    PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER /*| PFD_DEPTH_DONTCARE*/,
    PFD_TYPE_RGBA,
    32, // rgba bits
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0,
	bitsDepth,
	bitsStencil,
    0,
    PFD_MAIN_PLANE,
    0, 0, 0, 0,
};

// The OpenGL context attributes
static const int attributes[]
{
    // OpenGL Version
    WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
    WGL_CONTEXT_MINOR_VERSION_ARB, 1,

    // OpenGL Profile
    WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,

    // Enable Debug
#if ENGINE_DEBUG
    WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
#endif

    // Null-terminate
    0,
};


bool opengl_init()
{
	int   format;
	HGLRC context;

	// Get Device Context
	device = GetDC( CoreWindow::handle );

	// Choose Pixel Format
	if( ( format = ChoosePixelFormat( device, &pfd ) ) == 0 )
		{ ErrorReturnMsg( false, "OpenGL: Failed to choose pixel format" ); }

	// Set Pixel Format
	if( !SetPixelFormat( device, format, &pfd ) )
		{ ErrorReturnMsg( false, "OpenGL: Failed to set pixel format" ); }

	// Create False Context
	if( ( context = wglCreateContext( device ) ) == nullptr )
		{ ErrorReturnMsg( false, "OpenGL: Failed to create false context" ); }

	// Bind False Context
	if( !wglMakeCurrent( device, context ) )
		{ ErrorReturnMsg( false, "OpenGL: Failed to bind false context" ); }

	// Load WGL Procedures
	#undef  META
	#define META(type, name, ...)                                                             \
		if( ( name = reinterpret_cast<name##proc>( opengl_proc( #name ) ) ) == nullptr )      \
			{ ErrorReturnMsg( false, "OpenGL: Failed to load WGL procedure: %s", #name ); }

	#include "wgl.procedures.hpp"

	// Unbind False Context
	if( !wglMakeCurrent( nullptr, nullptr ) )
		{ ErrorReturnMsg( false, "OpenGL: Failed to unbind false context" ); }

	// Delete False Context
	if( !wglDeleteContext( context ) )
		{ ErrorReturnMsg( false, "OpenGL: Failed to delete false context" ); }

	// Create Final Context
	if( ( context = wglCreateContextAttribsARB( device, nullptr, attributes ) ) == nullptr )
		{ ErrorReturnMsg( false, "OpenGL: Failed to create final context" ); }

	// Bind Final Context
	if( !wglMakeCurrent( device, context ) )
		{ ErrorReturnMsg( false, "OpenGL: Failed to bind final context" ); }

	// Load OpenGL Procedures
	if( !opengl_load() )
		{ ErrorReturnMsg( false, "OpenGL: Failed to load final context" ); }

	// Disable VSync (TODO)
	wglSwapIntervalEXT( 0 );

	// Success
	return true;
}

bool opengl_swap()
{
	return SwapBuffers( device );
}

void *opengl_proc( const char *name )
{
	return reinterpret_cast<void *>( wglGetProcAddress( name ) );
}

void opengl_update()
{
	// NOTE: Nothing to do on wgl
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////