#include <manta/backend/gfx/opengl/opengl.hpp>

#include <config.hpp>

#include <core/debug.hpp>
#include <core/types.hpp>

#include <manta/window.hpp>
#include <manta/backend/window/x11/window.x11.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Define GLX Procedures
#undef  META
#define META(type, name, ...)                         \
	using  name##proc = type (GL_API *)(__VA_ARGS__); \
	static name##proc name;

#include "glx.procedures.hpp"

// the framebuffer config used to create the window
static GLXFBConfig config;

// The pixel format descriptor.
static const int pfd[]
{
	GLX_X_RENDERABLE,   true,
	GLX_DRAWABLE_TYPE,  GLX_WINDOW_BIT,
	GLX_RENDER_TYPE,    GLX_RGBA_BIT,
	GLX_X_VISUAL_TYPE,  GLX_TRUE_COLOR,
	GLX_RED_SIZE,       8,
	GLX_GREEN_SIZE,     8,
	GLX_BLUE_SIZE,      8,
	GLX_ALPHA_SIZE,     8,
	GLX_DEPTH_SIZE,     bitsDepth,
	GLX_STENCIL_SIZE,   bitsStencil,
	GLX_DOUBLEBUFFER,   true,
	0
};

// The OpenGL context attributes.
static const int attributes[]
{
    // OpenGL Version
    GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
    GLX_CONTEXT_MINOR_VERSION_ARB, 1,

    // OpenGL Profile
    GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,

    // Enable Debug
#if COMPILE_DEBUG
    GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_DEBUG_BIT_ARB,
#endif

    // Null-terminate
    0,
};


bool opengl_init()
{
    GLXContext context;

    // Load GLX Procedures
	#undef  META
	#define META(type, name, ...) \
		if( ( name = reinterpret_cast<name##proc>( opengl_proc( #name ) ) ) == nullptr ) \
			{ ErrorReturnMsg( false, "OpenGL: Failed to load GLX procedure: %s", #name ); }

    #include "glx.procedures.hpp"

    // Create Context
    if( ( context = glXCreateContextAttribsARB( CoreWindow::display, config, nullptr, true, attributes ) ) == nullptr )
        { ErrorReturnMsg( false, "OpenGL: Failed to create OpenGL context" ); }

    // ???
    XSync( CoreWindow::display, false );

    // Bind Context
    if( !glXMakeCurrent( CoreWindow::display, CoreWindow::handle, context ) )
        { ErrorReturnMsg( false, "OpenGL: Failed to bind OpenGL context" ); }

    // Load OpenGL Procedures
    if( !opengl_load() )
        { ErrorReturnMsg( false, "OpenGL: Failed to load OpenGL procedures" ); }

    // Disable VSync (TODO)
    glXSwapIntervalEXT( CoreWindow::display, CoreWindow::handle, 0 );

	// Success
	return true;
}


bool opengl_swap()
{
    // Swap Buffers
    glXSwapBuffers( CoreWindow::display, CoreWindow::handle );

    // Success
	return true;
}


void *opengl_proc( const char *name )
{
    return reinterpret_cast<void *>( glXGetProcAddress( reinterpret_cast<const GLubyte *>( name ) ) );
}


void opengl_update()
{
	// NOTE: Nothing to do on glx
}


XVisualInfo *x11_create_visual()
{
    int          major, minor, count;
    GLXFBConfig *configs;

    // Check that GLX is at least version 1.3
    if( !glXQueryVersion( CoreWindow::display, &major, &minor ) || ( major <= 1 && minor < 3 ) )
        return nullptr;

    // Get a list of matching configurations
    configs = glXChooseFBConfig( CoreWindow::display, DefaultScreen( CoreWindow::display ), pfd, &count );

    // Use the first configuration (TODO?)
    config = configs[0];

    // Free Config List
    XFree( configs );

    // Return Visual
    return glXGetVisualFromFBConfig( CoreWindow::display, config );
}


XVisualInfo * CoreWindow::x11_create_visual()
{
    int major;
    int minor;
    int count;
    GLXFBConfig *configs;

    // Check that GLX is at least version 1.3
    if( !glXQueryVersion( CoreWindow::display, &major, &minor ) || ( major <= 1 && minor < 3 ) )
    {
        return nullptr;
    }

    Print( "%d, %d", bitsDepth, bitsStencil );

    // Get a list of matching configurations
    configs = glXChooseFBConfig( CoreWindow::display, DefaultScreen( CoreWindow::display ), pfd, &count );

    // Use the first configuration (TODO?)
    config = configs[0];

    // Free Config List
    XFree( configs );

    // Return Visual
    return glXGetVisualFromFBConfig( CoreWindow::display, config );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////