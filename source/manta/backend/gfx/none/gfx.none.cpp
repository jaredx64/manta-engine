#include <manta/gfx.hpp>
#include <config.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace GfxCore
{
	bool init()
	{
		return true;
	}


	bool free()
	{
		return true;
	}


	void update()
	{
		// ...
	}


	void viewport_set_size( const int width, const int height, const bool fullscreen )
	{
		// ...
	}


	void swapchain_resize( const int width, const int height, const bool fullscreen )
	{
		// ...
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Gfx
{
	void frame_begin()
	{
		// ...
	}


	void frame_end()
	{
		// ...
	}


	void clear_color( Color color )
	{
		// ...
	}


	void clear_depth()
	{
		// ...
	}


	void quad_batch_begin()
	{
		// ...
	}


	void quad_batch_end()
	{
		// ...
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if WINDOW_X11

	#include <manta/window.hpp>
	#include <manta/backend/window/x11/window.x11.hpp>

	XVisualInfo * SysWindow::x11_create_visual()
	{
		return nullptr;
	}

#endif