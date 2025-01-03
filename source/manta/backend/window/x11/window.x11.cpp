#include <manta/window.hpp>
#include <manta/backend/window/x11/window.x11.hpp>

#include <config.hpp>

#include <vendor/stdlib.hpp>
#include <core/debug.hpp>
#include <core/types.hpp>

#include <manta/engine.hpp>
#include <manta/input.hpp>
#include <manta/gfx.hpp>
#include <manta/time.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define Button6 ( 6 )
#define Button7 ( 7 )

static char clipboard[4096];
static char primary[4096];

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static XAtom WM_DELETE_WINDOW;
static XAtom _NET_WM_STATE;
static XAtom _NET_WM_STATE_FULLSCREEN;

static XAtom CLIPBOARD;
static XAtom PRIMARY;
static XAtom TARGETS;
static XAtom UTF8_STRING;
static XAtom target;

namespace SysWindow
{
	XDisplay *display;
	XWindow handle;

	bool init( const int defaultWidth, const int defaultHeight )
	{
		XVisualInfo *visual;
		XSetWindowAttributes attributes;
		XSizeHints hints;

		Window::width = defaultWidth;
		Window::height = defaultHeight;
		Window::resized = true;

		// Open XDisplay
		if( ( SysWindow::display = XOpenDisplay( nullptr ) ) == nullptr )
			{ ErrorReturnMsg( false, "X11: Failed to open X11 display" ); }

		// Setup Window Visual
		if( ( visual = SysWindow::x11_create_visual() ) == nullptr )
			{ ErrorReturnMsg( false, "X11: Failed to create X11 visual" ); }

		// Setup Window Attribute
		attributes.background_pixel = BlackPixel( SysWindow::display, DefaultScreen( SysWindow::display ) );
		attributes.border_pixel = BlackPixel( SysWindow::display, DefaultScreen( SysWindow::display ) );
		attributes.event_mask = KeyPressMask | KeyReleaseMask | ButtonPressMask |
		                        ButtonReleaseMask | PointerMotionMask | FocusChangeMask | StructureNotifyMask;
		attributes.colormap = XCreateColormap( SysWindow::display, DefaultRootWindow( SysWindow::display ),
		                                       visual->visual, AllocNone );

		// Create Window
		SysWindow::handle =
			XCreateWindow(
				SysWindow::display,
				DefaultRootWindow( SysWindow::display ),
				0,
				0,
				Window::width,
				Window::height,
				0,
				visual->depth,
				InputOutput,
				visual->visual,
				CWBackPixel | CWBorderPixel | CWEventMask | CWColormap,
				&attributes
			);

		// Setup Atoms
		WM_DELETE_WINDOW = XInternAtom( SysWindow::display, "WM_DELETE_WINDOW", false );
		_NET_WM_STATE = XInternAtom( SysWindow::display, "_NET_WM_STATE", false );
		_NET_WM_STATE_FULLSCREEN = XInternAtom( SysWindow::display, "_NET_WM_STATE_FULLSCREEN", false );

		CLIPBOARD = XInternAtom( SysWindow::display , "CLIPBOARD", false );
		PRIMARY = XInternAtom( SysWindow::display, "PRIMARY", false );
		TARGETS = XInternAtom( SysWindow::display , "TARGETS", false );
		UTF8_STRING = XInternAtom( SysWindow::display, "UTF8_STRING", false );
		target = None;

		// Setup Protocols
		if( !XSetWMProtocols( SysWindow::display, SysWindow::handle, &WM_DELETE_WINDOW, 1 ) )
			{ ErrorReturnMsg( false, "X11: Failed to set X11 WM protocols" ); }

		// Setup Size
		hints.flags = PMinSize;
		hints.min_width = WINDOW_WIDTH_MIN;
		hints.min_height = WINDOW_HEIGHT_MIN;
		hints.max_width = WINDOW_WIDTH_MAX;
		hints.max_height = WINDOW_HEIGHT_MAX;
		XSetWMNormalHints( SysWindow::display, SysWindow::handle, &hints );

		// Set Window Title
		XStoreName( SysWindow::display, SysWindow::handle, PROJECT_CAPTION );

		// Enable detectable key auto-repeat #include <X11/XKBlib.h>
		XkbSetDetectableAutoRepeat( SysWindow::display, true, 0 );

		// Success
		return true;
	}


	bool free()
	{
		// Success
		return true;
	}


	void show()
	{
		XMapRaised( SysWindow::display, SysWindow::handle );
	}

	void poll()
	{
		//PROFILE_FUNCTION();
		while( XPending( SysWindow::display ) )
		{
			XEvent event;

			XNextEvent( SysWindow::display, &event );

			if( XFilterEvent( &event, None ) ) { continue; }

			switch( event.type )
			{
				// Window Resize
				case ConfigureNotify:
				{
					// Resize
					if( event.xconfigure.width != Window::width || event.xconfigure.height != Window::height )
					{
						Window::width = event.xconfigure.width;
						Window::height = event.xconfigure.height;
						Window::resized = true;
						Gfx::viewport_update();
					}
				}
				break;

				// The focus changed
				case FocusIn:
				{
					Window::hasFocus = true;
				}
				break;

				case FocusOut:
				{
					Window::hasFocus = false;
				}
				break;

				// Key Press
				case KeyPress:
				{
					KeySym keysym = XLookupKeysym( &event.xkey, 0 );
					Keyboard::state().keyCurrent[keysym & 0xFF] = true;
					Keyboard::state().keyRepeat[keysym & 0xFF] = Keyboard::state().keyCurrent[keysym & 0xFF];
					//PrintLn( "Key Hex: 0x%02X", static_cast<u8>( keysym & 0xFF ) );

					// Keyboard Input Text
					char *buffer = Keyboard::state().inputBuffer;
					XLookupString( &event.xkey, buffer, 8, &keysym, nullptr );
					buffer[8] = '\0';
				}
				break;

				// Key Release
				case KeyRelease:
				{
					// Detect auto-repeat explicitly
					if( XEventsQueued( SysWindow::display, QueuedAfterReading ) )
					{
						XEvent nextEvent;
						XPeekEvent( SysWindow::display, &nextEvent );
						if( nextEvent.type == KeyPress &&
							nextEvent.xkey.time == event.xkey.time &&
							nextEvent.xkey.keycode == event.xkey.keycode )
						{
							// Ignore the KeyRelease; it's part of an auto-repeat
							break;
						}
					}

					KeySym keysym = XLookupKeysym( &event.xkey, 0 );
					Keyboard::state().keyCurrent[keysym & 0xFF] = false;
					Keyboard::state().keyRepeat[keysym & 0xFF] = false;
				}
				break;

				// Mouse Press
				case ButtonPress:
				{
					Mouse::state().buttonCurrent |= ( 1 << event.xbutton.button );

					// Mouse wheel
					if( event.xbutton.button == Button4 )
					{
						Mouse::state().wheelY = -1;
						Mouse::state().wheelYVelocity = 1.0f;
					}

					if( event.xbutton.button == Button5 )
					{
						Mouse::state().wheelY = 1;
						Mouse::state().wheelYVelocity = 1.0f;
					}

					if( event.xbutton.button == Button6 )
					{
						Mouse::state().wheelX = -1;
						Mouse::state().wheelXVelocity = 1.0f;
					}

					if( event.xbutton.button == Button7 )
					{
						Mouse::state().wheelX = 1;
						Mouse::state().wheelXVelocity = 1.0f;
					}
				}
				break;

				// Mouse Release
				case ButtonRelease:
				{
					Mouse::state().buttonCurrent &= ~( 1 << event.xbutton.button );
				}
				break;

				// Mouse Position
				case MotionNotify:
				{
					Mouse::state().positionXPrevious = Mouse::state().positionX;
					Mouse::state().positionYPrevious = Mouse::state().positionY;
					Mouse::state().positionX = static_cast<float>( event.xmotion.x );
					Mouse::state().positionY = static_cast<float>( event.xmotion.y );
				}
				break;


				// Another client (likely the window manager) has sent us a message
				case ClientMessage:
				{
					// Gracefully handle a close request from the window
					if( event.xclient.data.l[0] == static_cast<long int>( WM_DELETE_WINDOW ) )
					{
						Engine::exit();
						return;
					}
				}
				break;


				// Clipboard
				case SelectionRequest:
				{
					XSelectionRequestEvent request = event.xselectionrequest;

					// CLIPBOARD / PRIMARY selections
					XAtom selection = None;
					const char *data = nullptr;

					if( XGetSelectionOwner( SysWindow::display, CLIPBOARD ) == SysWindow::handle &&
						request.selection == CLIPBOARD )
					{
						selection = CLIPBOARD;
						data = clipboard;
					}
					else if( XGetSelectionOwner( SysWindow::display, PRIMARY ) == SysWindow::handle &&
						request.selection == PRIMARY )
					{
						selection = PRIMARY;
						data = primary;
					}

					if( selection != None )
					{
						if( request.target == TARGETS && request.property != None )
						{
							XChangeProperty( request.display, request.requestor, request.property,
								XA_ATOM, 32, PropModeReplace, (unsigned char *)&UTF8_STRING, 1 );
						}
						else if( request.target == UTF8_STRING && request.property != None )
						{
							XChangeProperty( request.display, request.requestor, request.property,
								request.target, 8, PropModeReplace, (unsigned char *)data, strlen( data ) );
						}

						XSelectionEvent sendEvent;
						sendEvent.type = SelectionNotify;
						sendEvent.serial = request.serial;
						sendEvent.send_event = request.send_event;
						sendEvent.display = request.display;
						sendEvent.requestor = request.requestor;
						sendEvent.selection = request.selection;
						sendEvent.target = request.target;
						sendEvent.property = request.property;
						sendEvent.time = request.time;

						XSendEvent( SysWindow::display, request.requestor, 0, 0, (XEvent *)&sendEvent );
					}
				}
				break;
			}
		}
	}


	void mouse_get_position( double &x, double &y )
	{
		// On X11, this function doesn't need to do anything special
		x = Mouse::x();
		y = Mouse::y();
	}


	void mouse_set_position( const int x, const int y )
	{
		XWarpPointer( SysWindow::display, None, SysWindow::handle, 0, 0, 0, 0, x, y );
		XSync( SysWindow::display, false );
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Window
{
	void show_message( const char *title, const char *message )
	{
		// TODO: implement this on linux
	}


	void show_message_error( const char *title, const char *message )
	{
		// TODO: implement this on linux
	}


	bool resize( int width, int height )
	{
		const XScreen *const screen = DefaultScreenOfDisplay( SysWindow::display );
		if( !screen ) { return false; }

		const int x = ( screen->width - width ) / 2;
		const int y = ( screen->height - height ) / 2;
		XResizeWindow( SysWindow::display, SysWindow::handle, width, height );
		XMoveWindow( SysWindow::display, SysWindow::handle, x, y );
		XFlush( SysWindow::display );
		return true;
	}


	void set_fullscreen( bool enabled )
	{
		// TODO:
		// This is based on the 'Extended Window Manager Hints' specification, so
		// maybe you need a "modern" window manager for anything to happen.
		XEvent event = { };

		// Write a client message telling the window manager to add or remove
		// the _NET_WM_STATE_FULLSCREEN atom from the _NET_WM_STATE depending
		// on whether we're entering or exiting fullscreen.
		event.xclient.type = ClientMessage;
		event.xclient.window = SysWindow::handle;
		event.xclient.message_type = _NET_WM_STATE;
		event.xclient.format = 32;
		event.xclient.data.l[0] = fullscreen;
		event.xclient.data.l[1] = _NET_WM_STATE_FULLSCREEN;
		event.xclient.data.l[2] = 0;
		event.xclient.data.l[3] = 0;

		// Send the message to the window manager
		XSendEvent( SysWindow::display, DefaultRootWindow( SysWindow::display ),
		            false, SubstructureRedirectMask | SubstructureNotifyMask, &event );

		// Don't forget to update the internal fullscreen state!
		Window::fullscreen = enabled;
	}


	void set_caption( const char *caption )
	{
		XStoreName( SysWindow::display, SysWindow::handle, caption );
	}


	bool set_clipboard( const char *buffer )
	{
		// Copy 'buffer' to our local clipboard cache
		snprintf( clipboard, sizeof( clipboard ), "%s", buffer );

		// Trigger SelectionRequest events (processed in Window::poll() above)
		XSetSelectionOwner( SysWindow::display, CLIPBOARD, SysWindow::handle, CurrentTime );
		if( XGetSelectionOwner( SysWindow::display, CLIPBOARD ) != SysWindow::handle ) { return false; }
		return true;
	}


	bool get_clipboard( char *buffer, const usize size )
	{
		// Request the clipboard
		buffer[0] = '\0';
		if( XGetSelectionOwner( SysWindow::display, CLIPBOARD ) == None ) { return false; }

		// If we're the current owner, short circuit to our own clipboard cache
		if( XGetSelectionOwner( SysWindow::display, CLIPBOARD ) == SysWindow::handle )
		{
			snprintf( buffer, size, "%s", clipboard );
			return true;
		}

		// If the owner is a different app, request and wait for a SelectionNotify event
		XConvertSelection( SysWindow::display, CLIPBOARD, UTF8_STRING, CLIPBOARD, SysWindow::handle, CurrentTime );

		Timer timer;
		while( timer.ms() < 100.0 ) // 100 ms timeout
		{
			XEvent event;
			XNextEvent( SysWindow::display, &event );
			if( XFilterEvent( &event, None ) ) { continue; }

			if( event.type == SelectionNotify )
			{
				XSelectionEvent selection = event.xselection;
				if( selection.property == None ) { break; }

				XAtom actualType;
				int actualFormat;
				unsigned long bytesAfter;
				unsigned char *data;
				unsigned long count;

				XGetWindowProperty( SysWindow::display, SysWindow::handle, CLIPBOARD, 0, I32_MAX, false,
					AnyPropertyType, &actualType, &actualFormat, &count, &bytesAfter, &data );

				if( selection.target == TARGETS )
				{
					XAtom *list = reinterpret_cast<XAtom *>( data );
					for( unsigned long i = 0; i < count; i++ )
					{
						if( list[i] == XA_STRING ) { target = XA_STRING; } else
						if( list[i] == UTF8_STRING ) { target = UTF8_STRING; break; }
					}

					if( target != None )
					{
						XConvertSelection( SysWindow::display, CLIPBOARD, target, CLIPBOARD,
							SysWindow::handle, CurrentTime );
					}
				}

				// Transfer the clipboard data to our output buffer
				if( data )
				{
					snprintf( buffer, size, "%s", data );
					XFree( data );
				}
				return true;
			}
		}

		// Failure
		return false;
	}


	bool set_selection( const char *buffer )
	{
		// Copy 'buffer' to our local selection cache
		snprintf( primary, sizeof( primary ), "%s", buffer );

		// Trigger SelectionRequest events (processed in Window::poll() above)
		XSetSelectionOwner( SysWindow::display, PRIMARY, SysWindow::handle, CurrentTime );
		if( XGetSelectionOwner( SysWindow::display, PRIMARY ) != SysWindow::handle ) { return false; }
		return true;
	}


	bool get_selection( char *buffer, const usize size )
	{
		// Request the selection
		buffer[0] = '\0';
		if( XGetSelectionOwner( SysWindow::display, PRIMARY ) == None ) { return false; }

		// If we're the current owner, short circuit to our own selection cache
		if( XGetSelectionOwner( SysWindow::display, PRIMARY ) == SysWindow::handle )
		{
			snprintf( buffer, size, "%s", primary );
			return true;
		}

		// If the owner is a different app, request and wait for a SelectionNotify event
		XConvertSelection( SysWindow::display, PRIMARY, UTF8_STRING, PRIMARY, SysWindow::handle, CurrentTime );

		Timer timer;
		while( timer.ms() < 100.0 ) // 100 ms timeout
		{
			XEvent event;
			XNextEvent( SysWindow::display, &event );
			if( XFilterEvent( &event, None ) ) { continue; }

			if( event.type == SelectionNotify )
			{
				XSelectionEvent selection = event.xselection;
				if( selection.property == None ) { break; }

				XAtom actualType;
				int actualFormat;
				unsigned long bytesAfter;
				unsigned char *data;
				unsigned long count;

				XGetWindowProperty( SysWindow::display, SysWindow::handle, PRIMARY, 0, I32_MAX, false,
					AnyPropertyType, &actualType, &actualFormat, &count, &bytesAfter, &data );

				if( selection.target == TARGETS )
				{
					XAtom *list = reinterpret_cast<XAtom *>( data );
					for( unsigned long i = 0; i < count; i++ )
					{
						if( list[i] == XA_STRING ) { target = XA_STRING; } else
						if( list[i] == UTF8_STRING ) { target = UTF8_STRING; break; }
					}

					if( target != None )
					{
						XConvertSelection( SysWindow::display, PRIMARY, target, PRIMARY,
							SysWindow::handle, CurrentTime );
					}
				}

				// Transfer the selection data to our output buffer
				if( data )
				{
					snprintf( buffer, size, "%s", data );
					XFree( data );
				}
				return true;
			}
		}

		// Failure
		return false;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////