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

void Window::update( const Delta delta )
{
#if WINDOW_ENABLED
	Window::resized = false;
	CoreWindow::poll();
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////