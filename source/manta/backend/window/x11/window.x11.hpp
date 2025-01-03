#pragma once

#include <vendor/x11.hpp>
#include <vendor/glx.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace SysWindow
{
	extern XDisplay *display;
	extern XWindow handle;

	// X11 has a kinda weird coupling between window and context creation.
	// Basically, any graphics API that wants to support X11 will need to
	// implement this function and return a visual for the window. This is
	// just called from the X11 window implementation.
	extern XVisualInfo *x11_create_visual();
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////