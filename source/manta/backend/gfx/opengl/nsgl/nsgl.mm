#define GL_SILENCE_DEPRECATION
#include <manta/backend/gfx/opengl/opengl.hpp>
#include <manta/backend/window/cocoa/window.cocoa.hpp>

#include <vendor/conflicts.hpp>
	#import <Cocoa/Cocoa.h>
	#import <OpenGL/gl3.h>
#include <vendor/conflicts.hpp>

#include <core/debug.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static NSOpenGLContext *context;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static const NSOpenGLPixelFormatAttribute attributes[]
{
	// Double buffered context
	NSOpenGLPFADoubleBuffer,

	// OpenGL 4.1 (Core Profile) context
	NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion4_1Core,

	// 32-bit color buffer
	NSOpenGLPFAColorSize, 32,

	// Depth size
	NSOpenGLPFADepthSize, bitsDepth,

	// Stencil size
	NSOpenGLPFAStencilSize, bitsStencil,

	// NOTE: Apple says to do this
	NSOpenGLPFAAccelerated,
	NSOpenGLPFANoRecovery,
	NSOpenGLPFAAllowOfflineRenderers,

	// Null-terminate
	0,
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool opengl_init()
{
	// Create Pixel Format
	NSOpenGLPixelFormat *format;
	if( ( format = [[NSOpenGLPixelFormat alloc] initWithAttributes:attributes]) == nil )
	{
		ErrorReturnMsg( false, "OpenGL: Failed to create pixel format" );
	}

	// Create OpenGL Context
	if( ( context = [[NSOpenGLContext alloc] initWithFormat:format shareContext:nil] ) == nil )
	{
		ErrorReturnMsg( false, "OpenGL: Failed to create OpenGL context" );
	}

	// Disable Retina Framebuffer
	[CoreWindow::view setWantsBestResolutionOpenGLSurface: YES];

	// Bind Context
	[context setView: CoreWindow::view];
	[context makeCurrentContext];

	// Disable Vsync (TODO)
	int vsync = 0;
	[context setValues: &vsync forParameter: NSOpenGLContextParameterSwapInterval];

	return true;
}


bool opengl_free()
{
	return true;
}


bool opengl_swap()
{
	[context flushBuffer];
	return true;
}


void opengl_update()
{
	[context update];
}


void *opengl_proc( const char *name )
{
	return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////