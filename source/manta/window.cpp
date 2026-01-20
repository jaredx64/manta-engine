#include <manta/window.hpp>

#include <manta/input.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace CoreWindow
{
	// Implementations: manta/backend/window/...
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Window
{
	int width = WINDOW_WIDTH_DEFAULT;
	int height = WINDOW_HEIGHT_DEFAULT;
	int widthDefault = WINDOW_WIDTH_DEFAULT;
	int heightDefault = WINDOW_HEIGHT_DEFAULT;
	int widthMin = WINDOW_WIDTH_MIN;
	int heightMin = WINDOW_HEIGHT_MIN;
	int widthMax = WINDOW_WIDTH_MAX;
	int heightMax = WINDOW_HEIGHT_MAX;

	float scale = 1.0f;

	bool hasFocus = true;
	bool allowFullscreen = true;
	bool allowResize = true;
	bool fullscreen = false;
	bool resized = false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Window::update( Delta delta )
{
#if WINDOW_ENABLED
	Window::resized = false;
	CoreWindow::poll();
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Window::width_logical()
{
	return Window::width;
}


int Window::height_logical()
{
	return Window::height;
}


int Window::width_pixels()
{
	return static_cast<int>( Window::width * Window::scale );
}


int Window::height_pixels()
{
	return static_cast<int>( Window::height * Window::scale );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////