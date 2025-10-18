#include <manta/window.hpp>
#include <manta/backend/window/windows/window.windows.hpp>

#include <config.hpp>

#include <core/debug.hpp>
#include <core/types.hpp>

#include <vendor/stdlib.hpp>

#include <manta/engine.hpp>
#include <manta/input.hpp>
#include <manta/gfx.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define WINDOW_CLASS ( TEXT( BUILD_PROJECT ) )
#define WINDOW_TITLE ( TEXT( PROJECT_CAPTION ) )
#define WINDOW_STYLE ( WS_OVERLAPPEDWINDOW )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace CoreWindow
{
	HWND handle;

	static LRESULT CALLBACK procedure( HWND wnd, UINT msg, WPARAM wp, LPARAM lp )
	{
	#if WINDOW_ENABLED
		switch( msg )
		{
			// Close Button
			case WM_CLOSE:
			{
				Engine::exit();
				return 0;
			}

			// Min/Max Info
			case WM_GETMINMAXINFO:
			{
				MINMAXINFO *info = reinterpret_cast<MINMAXINFO *>( lp );
				info->ptMinTrackSize.x = Window::widthMin > 0 ? static_cast<LONG>( Window::widthMin ) : 0;
				info->ptMinTrackSize.y = Window::heightMin > 0 ? static_cast<LONG>( Window::heightMin ) : 0;
				info->ptMaxTrackSize.x = Window::widthMax > 0 ? static_cast<LONG>( Window::widthMax ) : I32_MAX;
				info->ptMaxTrackSize.y = Window::heightMax > 0 ? static_cast<LONG>( Window::heightMax ) : I32_MAX;
				return 0;
			}

			// Focus
			case WM_ACTIVATEAPP:
			{
				Window::hasFocus = static_cast<bool>( wp );
				return 0;
			}

			// Resize
			case WM_SIZE:
			{
				BringWindowToTop( CoreWindow::handle );
				int width = LOWORD( lp );
				int height = HIWORD( lp );
				if( width != 0 && height != 0 )
				{
					Window::width = static_cast<int>( width );
					Window::height = static_cast<int>( height );
					Window::resized = true;
					CoreGfx::swapchain_viewport_update();
				}
				return 0;
			}


			// Position
			case WM_MOVE:
			{
				// Reset fullscreen
				if( Window::fullscreen ) { Window::set_fullscreen( true ); }
				return 0;
			}

			// Key Presses
			case WM_KEYDOWN:
			case WM_KEYUP:
			{
				Keyboard::state().keyCurrent[wp] = ( msg == WM_KEYDOWN );
				Keyboard::state().keyRepeat[wp] = ( msg == WM_KEYDOWN );
				//if( msg == WM_KEYDOWN ) { PrintLn( "Key Hex: 0x%02X", static_cast<u8>( wp ) ); }
				return 0;
			}

			// Printable Character
			case WM_CHAR:
			{
				static WCHAR hs;

				if( IS_HIGH_SURROGATE( wp ) )
				{
					// This is the first part of the message: the high surrogate. We can't do
					// anything with this alone, so just save it for later, return, and wait
					// until we process the second message.
					hs = static_cast<WCHAR>( wp );
					return 0;
				}
				else if( IS_SURROGATE_PAIR( hs, wp ) )
				{
					wp = ( hs << 10 ) + static_cast<u32>( wp ) + ( 0x10000 - ( 0xD800 << 10 ) - 0xDC00 );
					hs = 0;
				}
				else
				{
					hs = 0;
				}

				// Keyboard Input Buffer
				char *buffer = Keyboard::state().inputBuffer;
				int size = WideCharToMultiByte( CP_UTF8, 0, reinterpret_cast<WCHAR *>( &wp ), 1, buffer, 10, nullptr, nullptr );
				buffer[size] = '\0';

				return 0;
			}

			// I've never actually seen this message get sent, but apparently it's
			// used by a few third party IMEs. It's also trivial to implement since
			// it's already in UTF-32, so let's just be good boys and support it!
			case WM_UNICHAR:
			{
				// Announce support for WM_UNICHAR to the sender of this message.
				if( wp == UNICODE_NOCHAR ) { return 1; }

				// Keyboard Input Buffer
				char *buffer = Keyboard::state().inputBuffer;
				int size = WideCharToMultiByte( CP_UTF8, 0, reinterpret_cast<WCHAR *>( &wp ), 1, buffer, 10, nullptr, nullptr );
				buffer[size] = '\0';

				return 0;
			}

			// Mouse Button
			case WM_LBUTTONDOWN:
			case WM_LBUTTONUP:
			case WM_RBUTTONDOWN:
			case WM_RBUTTONUP:
			case WM_MBUTTONDOWN:
			case WM_MBUTTONUP:
			{
				// Update State
				Mouse::state().buttonCurrent = static_cast<int>( wp );

				// Update Capture
				if( wp & ( MK_LBUTTON | MK_RBUTTON | MK_MBUTTON ) )
				{
					SetCapture( CoreWindow::handle );
				}
				else
				{
					ReleaseCapture();
				}
				return 0;
			}

			case WM_MOUSEWHEEL:
			{
				if( GET_WHEEL_DELTA_WPARAM( wp ) > 0 )
				{
					Mouse::state().wheelY = -1; // up
					Mouse::state().wheelYVelocity = 1.0f;
				}
				else
				{
					Mouse::state().wheelY = 1; // down
					Mouse::state().wheelYVelocity = 1.0f;
				}

				return 0;
			}

			case WM_MOUSEHWHEEL:
			{
				if( GET_WHEEL_DELTA_WPARAM( wp ) > 0 )
				{
					Mouse::state().wheelX = 1; // right
					Mouse::state().wheelXVelocity = 1.0f;
				}
				else
				{
					Mouse::state().wheelX = -1; // left
					Mouse::state().wheelXVelocity = 1.0f;
				}

				return 0;
			}

			// Mouse Movement
			case WM_MOUSEMOVE:
			{
				Mouse::state().positionXPrevious = Mouse::state().positionX;
				Mouse::state().positionYPrevious = Mouse::state().positionY;
				Mouse::state().positionX = static_cast<float>( GET_X_LPARAM( lp ) );
				Mouse::state().positionY = static_cast<float>( GET_Y_LPARAM( lp ) );
				return 0;
			}

			// Let Windows handle everything else
			default: { return DefWindowProcW( wnd, msg, wp, lp ); }
		}
	#endif
		return 0;
	}


	bool init( const int defaultWidth, const int defaultHeight )
	{
	#if WINDOW_ENABLED
		// Set Dimensions
		Window::width = defaultWidth;
		Window::height = defaultHeight;
		Window::resized = true;

		// Get Application Instance
		HINSTANCE instance;
		instance = GetModuleHandleW( nullptr );

		// Get Application Path
		WCHAR path[MAX_PATH];
		GetModuleFileNameW( nullptr, path, MAX_PATH );

		// Get Window Area
		RECT area;
		area.left = 0;
		area.top = 0;
		area.right = Window::width;
		area.bottom = Window::height;

		AdjustWindowRect( &area, WINDOW_STYLE, false );
		Window::width = area.right - area.left;
		Window::height = area.bottom - area.top;

		// Register Class
		WNDCLASSW wclass;

	#if GRAPHICS_OPENGL // TODO: GRAPHICS_VULKAN, GRAPHICS_D3D12 -- consider these?
		wclass.style = CS_OWNDC;
	#else // Everything Else
		wclass.style = 0;
	#endif
		wclass.lpfnWndProc = procedure;
		wclass.cbClsExtra = 0;
		wclass.cbWndExtra = 0;
		wclass.hInstance = instance;
		wclass.hIcon = ExtractIconW( nullptr, path, 0 );
		wclass.hCursor = LoadCursorW( nullptr, IDC_ARROW );
		wclass.hbrBackground = nullptr;
		wclass.lpszMenuName = nullptr;
		wclass.lpszClassName = WINDOW_CLASS;

		// Check Error
		if( !RegisterClassW( &wclass ) ) { ErrorReturnMsg( false, "Window: Failed to register window class" ); }

		// Create Window
		handle = CreateWindowExW(
	#if GRAPHICS_D3D11 // TODO: GRAPHICS_VULKAN, GRAPHICS_D3D12 -- consider these?
			WS_EX_NOREDIRECTIONBITMAP,
	#else
			0,
	#endif
			WINDOW_CLASS,
			WINDOW_TITLE,
			WINDOW_STYLE,
			( GetSystemMetrics( SM_CXSCREEN ) - Window::width ) >> 1,
			( GetSystemMetrics( SM_CYSCREEN ) - Window::height ) >> 1,
			Window::width,
			Window::height,
			nullptr,
			nullptr,
			instance,
			nullptr );

		// Check Error
		if( handle == nullptr ) { ErrorReturnMsg( false, "Window: Failed to get window handle" ); }
	#endif

		// Success
		return true;
	}


	bool free()
	{
	#if WINDOW_ENABLED
		// ...
	#endif
		return true;
		// Success
	}


	void show()
	{
	#if WINDOW_ENABLED
		ShowWindow( CoreWindow::handle, SW_NORMAL );
	#endif
	}


	void poll()
	{
	#if WINDOW_ENABLED
		MSG msg;

		while( PeekMessageW( &msg, nullptr, 0, 0, PM_REMOVE ) )
		{
			TranslateMessage( &msg );
			DispatchMessageW( &msg );
		}
	#endif
	}


	void mouse_get_position( double &x, double &y )
	{
	#if WINDOW_ENABLED
		// On Windows, this function doesn't need to do anything special
		x = Mouse::x();
		y = Mouse::y();
	#endif
	}


	void mouse_set_position( const int x, const int y )
	{
	#if WINDOW_ENABLED
		POINT p;
		p.x = x;
		p.y = y;
		ClientToScreen( CoreWindow::handle, &p );
		SetCursorPos( p.x, p.y );
	#endif
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Window
{
	void show_message( const char* title, const char* message )
	{
	#if WINDOW_ENABLED
		MessageBoxA( NULL, message, title, MB_OK );
	#endif
	}


	void show_message_error( const char* title, const char* message )
	{
	#if WINDOW_ENABLED
		MessageBoxA( NULL, message, title, MB_OK | MB_ICONERROR );
	#endif
	}


	void set_size( int width, int height )
	{
	#if WINDOW_ENABLED
		RECT screenRect;
		SystemParametersInfoA( SPI_GETWORKAREA, 0, &screenRect, 0 );

		const int x = ( screenRect.right - width ) / 2;
		const int y = ( screenRect.bottom - height ) / 2;
		SetWindowPos( CoreWindow::handle, NULL, x, y, width, height, SWP_NOZORDER | SWP_NOACTIVATE );
	#endif
	}


	void set_fullscreen( bool enabled )
	{
	#if WINDOW_ENABLED
		MONITORINFO monitor;
		int mw, mh; // Monitor Dimensions
		int cw, ch; // Client Dimensions
		RECT area;

		// ???
		monitor.cbSize = sizeof( MONITORINFO );

		// Get Nearest Monitor
		GetMonitorInfoW( MonitorFromWindow( CoreWindow::handle, MONITOR_DEFAULTTONEAREST ), &monitor );

		// Get Monitor Dimensions
		mw = monitor.rcMonitor.right  - monitor.rcMonitor.left;
		mh = monitor.rcMonitor.bottom - monitor.rcMonitor.top;

		if( enabled )
		{
			// Enter Fullscreen
			Window::fullscreen = true;

			// Update Window Style
			SetWindowLongPtrW( CoreWindow::handle, GWL_STYLE, WINDOW_STYLE & ~( WS_CAPTION | WS_THICKFRAME ) );

			// Update Window Size
			SetWindowPos(
				CoreWindow::handle,
				nullptr,
				monitor.rcMonitor.left,
				monitor.rcMonitor.top,
				mw,
				mh,
				SWP_FRAMECHANGED | SWP_SHOWWINDOW
			);

		}
		else
		{
			// Exit Fullscreen
			Window::fullscreen = false;

			// Get Client Dimensions
			area.left = 0;
			area.top = 0;
			area.right = WINDOW_WIDTH_DEFAULT; // TODO: Cache previous window size?
			area.bottom = WINDOW_HEIGHT_DEFAULT;

			AdjustWindowRect(&area, WINDOW_STYLE, false);

			cw = area.right  - area.left;
			ch = area.bottom - area.top;

			// Update Window Style
			SetWindowLongPtrW( CoreWindow::handle, GWL_STYLE, WINDOW_STYLE );

			// Update Window Size
			SetWindowPos(
				CoreWindow::handle,
				nullptr,
				monitor.rcMonitor.left + ( ( mw - cw ) >> 1 ),
				monitor.rcMonitor.top  + ( ( mh - ch ) >> 1 ),
				cw,
				ch,
				SWP_FRAMECHANGED | SWP_SHOWWINDOW
			);
		}
	#endif
	}


	void show_cursor( const bool enabled )
	{
	#if WINDOW_ENABLED
		ShowCursor( enabled );
	#endif
	}


	void set_caption( const char *caption )
	{
	#if WINDOW_ENABLED
		static wchar_t wstr[512];

		int length = static_cast<int>( strlen( caption ) );
		int size = MultiByteToWideChar( CP_UTF8, 0, caption, length, NULL, 0 );

		if( size < static_cast<int>( ARRAY_LENGTH( wstr ) ) )
		{
			MultiByteToWideChar( CP_UTF8, 0, caption, length, wstr, size );
			wstr[size] = L'\0';

			SetWindowTextW( CoreWindow::handle, wstr );
		}
	#endif
	}


	bool set_clipboard( const char *buffer )
	{
	#if WINDOW_ENABLED
		// Open & clear the clipboard
		if( !OpenClipboard( 0 ) ) { return false; }
		if( !EmptyClipboard() ) { CloseClipboard(); return false; }

		// Allocate global memory for our text
		const usize size = strlen( buffer ) + 1;
		HGLOBAL hGlobal = GlobalAlloc( GMEM_MOVEABLE, size );
		if( !hGlobal ) { CloseClipboard(); return false; }

		// Transfer the text
		char *destination = reinterpret_cast<char *>( GlobalLock( hGlobal ) );
		memory_copy( destination, buffer, size );
		GlobalUnlock( hGlobal );

		// Set clipboard data
		if( !SetClipboardData( CF_TEXT, hGlobal ) )
		{
			GlobalFree( hGlobal );
			CloseClipboard();
			return false;
		}

		// Close clipboard
		CloseClipboard();
	#endif

		return true;
	}


	bool get_clipboard( char *buffer, const usize size )
	{
	#if WINDOW_ENABLED
		// Open the clipboard
		buffer[0] = '\0';
		if( !OpenClipboard( 0 ) ) { return false; }

		// Check if the clipboard has text
		if( !IsClipboardFormatAvailable( CF_TEXT ) ) { CloseClipboard(); return false; }

		// Transfer the clipboard data
		HGLOBAL hGlobal = GetClipboardData( CF_TEXT );
		if( !hGlobal ) { CloseClipboard(); return false; }
		char *source = reinterpret_cast<char *>( GlobalLock( hGlobal ) );
		if( source == nullptr ) { CloseClipboard(); return false; }
		snprintf( buffer, size, "%s", source );
		GlobalUnlock( hGlobal );

		// Close clipboard
		CloseClipboard();
	#endif
		return true;
	}


	bool set_selection( const char *buffer )
	{
	#if WINDOW_ENABLED
		// Do nothing on Windows
	#endif
		return true;
	}


	bool get_selection( char *buffer, const usize size )
	{
	#if WINDOW_ENABLED
		// Do nothing on Windows
		buffer[0] = '\0';
	#endif
		return true;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////