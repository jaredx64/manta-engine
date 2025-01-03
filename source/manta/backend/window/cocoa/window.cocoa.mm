#include <manta/window.hpp>
#include <manta/backend/window/cocoa/window.cocoa.hpp>

#import <Cocoa/Cocoa.h>

#include <config.hpp>

#include <core/debug.hpp>
#include <core/types.hpp>

#include <vendor/stdlib.hpp>
#include <vendor/string.hpp>

#include <manta/engine.hpp>
#include <manta/input.hpp>
#include <manta/gfx.hpp>
#include <manta/math.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define WINDOW_TITLE PROJECT_CAPTION
#define WINDOW_STYLE \
	( NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | \
	  NSWindowStyleMaskResizable | NSWindowStyleMaskMiniaturizable )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Subclass Application
@interface GameApplication : NSApplication @end
@implementation GameApplication

//- (void)terminate:(id)sender { Engine::exit(); }

@end

// Subclass Window
@interface GameWindow : NSWindow @end
@implementation GameWindow

- (BOOL)canBecomeKeyWindow  { return YES; }
- (BOOL)canBecomeMainWindow { return YES; }

@end

// Window Delegate
@interface GameWindowDelegate : NSObject @end
@implementation GameWindowDelegate

- (BOOL)windowShouldClose:(NSWindow *)sender
{
	// Terminate the application when window is closed
	[ NSApp terminate:nil ];

	// The window will be closed implicitly when the applicaiton exists
	return NO;
}

// Toggling fullscreen is asynch in Cocoa, so these callbacks tell us
- (void)windowDidEnterFullScreen:(NSNotification *)notification { Window::fullscreen = true; }
- (void)windowDidExitFullScreen:(NSNotification *)notification  { Window::fullscreen = false; }

// Called when the window gains/loses focus
- (void)windowDidBecomeKey:(NSNotification *)notification { Window::hasFocus = true; }
- (void)windowDidResignKey:(NSNotification *)notification { Window::hasFocus = false; }

@end

// Subclass View
@interface GameView : NSView @end
@implementation GameView

- (BOOL)canBecomeKeyView { return YES; }
- (BOOL)acceptsFirstResponder { return YES; }
- (BOOL)acceptsFirstMouse:(NSEvent *)event { return YES; }

// A regular key was pressed or released
- (void)keyDown: (NSEvent *)event
	{
		BOOL isRepeat = [event isARepeat];
		Keyboard::state().keyCurrent[event.keyCode] = true;
		Keyboard::state().keyRepeat[event.keyCode] = isRepeat;
		//PrintLn( "Key Hex: 0x%02X", static_cast<u8>( event.keyCode ) );

		// IME
		char *buffer = Keyboard::state().inputBuffer;
		const char *input = [[event characters] UTF8String];
		const usize length = strlen( input );
		memory_copy( buffer, input, length < 8 ? length : 8 );
		buffer[8] = '\0';
	}

- (void)keyUp: (NSEvent *)event
	{
		Keyboard::state().keyCurrent[event.keyCode] = false;
		Keyboard::state().keyRepeat[event.keyCode] = false;
	}

// A modifier key was pressed or released
- (void)flagsChanged: (NSEvent *)event
{
	static NSUInteger previousFlags = 0;
	NSUInteger currentFlags = event.modifierFlags & NSEventModifierFlagDeviceIndependentFlagsMask;

	// Detect key presses
	if( currentFlags & NSEventModifierFlagShift )
	{
		Keyboard::state().keyCurrent[vk_shift] = true;
	}
	else if( previousFlags & NSEventModifierFlagShift )
	{
		Keyboard::state().keyCurrent[vk_shift] = false;
	}

	if( currentFlags & NSEventModifierFlagControl )
	{
		Keyboard::state().keyCurrent[vk_control] = true;
	}
	else if( previousFlags & NSEventModifierFlagControl )
	{
		Keyboard::state().keyCurrent[vk_control] = false;
	}

	if( currentFlags & NSEventModifierFlagOption )
	{
		Keyboard::state().keyCurrent[vk_alt] = true;
	}
	else if( previousFlags & NSEventModifierFlagOption )
	{
		Keyboard::state().keyCurrent[vk_alt] = false;
	}

	if( currentFlags & NSEventModifierFlagCommand )
	{
		Keyboard::state().keyCurrent[vk_command] = true;

	}
	else if( previousFlags & NSEventModifierFlagCommand )
	{
		Keyboard::state().keyCurrent[vk_command] = false;
	}

	// Update the previous flags
	previousFlags = currentFlags;
}

// Left Mouse Button
- (void)mouseDown: (NSEvent *)event
	{
		if( !Keyboard::state().keyCurrent[vk_control] )
		{
			Mouse::state().buttonCurrent |= mb_left;
		}
		else
		{
			Mouse::state().buttonCurrent |= mb_middle;
		}
	}

- (void)mouseUp: (NSEvent *)event
	{
		Mouse::state().buttonCurrent &= ~mb_left;

		if( Keyboard::state().keyCurrent[vk_control] )
		{
			Mouse::state().buttonCurrent  &= ~mb_middle;
		}
	}

- (void)rightMouseDown:(NSEvent *)event
	{
		Mouse::state().buttonCurrent |= mb_right;
	}

- (void)rightMouseUp: (NSEvent *)event
	{
		Mouse::state().buttonCurrent &= ~mb_right;
	}

- (void)otherMouseDown: (NSEvent *)event
	{
		Mouse::state().buttonCurrent |= mb_middle;
	}

- (void)otherMouseUp: (NSEvent *)event
	{
		Mouse::state().buttonCurrent &= ~mb_middle;
	}

- (void)scrollWheel: (NSEvent *)event
	{
		if( event.deltaY > 0 )
		{
			Mouse::state().wheelY = -1; // up
			Mouse::state().wheelYVelocity = static_cast<float>( event.deltaY ) * 0.25f;
		}

		else if( event.deltaY < 0 )
		{
			Mouse::state().wheelY = 1; // down
			Mouse::state().wheelYVelocity = static_cast<float>( -event.deltaY ) * 0.25f;
		}

		if( event.deltaX > 0 )
		{
			Mouse::state().wheelX = 1; // right
			Mouse::state().wheelXVelocity = static_cast<float>( event.deltaX ) * 0.25f;
		}

		else if( event.deltaX < 0 )
		{
			Mouse::state().wheelX = -1; // left
			Mouse::state().wheelXVelocity = static_cast<float>( -event.deltaX ) * 0.25f;
		}
	}

// Mouse Position was changed
- (void)mouseMoved:(NSEvent *)event
{
	NSRect frame = [self frame];
	NSPoint point = [event locationInWindow];

	if( point.x < 0.0 || point.x >= frame.size.width ||
		point.y < 0.0 || point.y >= frame.size.height )
	{
		return;
	}

	Mouse::state().positionXPrevious = Mouse::state().positionX;
	Mouse::state().positionYPrevious = Mouse::state().positionY;
	Mouse::state().positionX = static_cast<float>( point.x );
	Mouse::state().positionY = static_cast<float>( frame.size.height - point.y );
}

// For some reason, when a mouse button is being held down, cocoa doesn't send
// mouse move events, instead sends us "dragged" events corresponding to the
// buttons being held. Let's just forward these to regular mouseMoved events
- (void)mouseDragged:     (NSEvent *)event { [self mouseMoved:event]; }
- (void)rightMouseDragged:(NSEvent *)event { [self mouseMoved:event]; }
- (void)otherMouseDragged:(NSEvent *)event { [self mouseMoved:event]; }

// The view was resized
- (void)drawRect:(NSRect)dirtyRect
{
	Window::width = static_cast<int>( dirtyRect.size.width );
	Window::height = static_cast<int>( dirtyRect.size.height );
	Window::resized = true;
	Gfx::viewport_update();
}

@end

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace SysWindow
{
	NSView *view;
	NSWindow *window;
	static NSAutoreleasePool *pool;

	bool init( const int defaultWidth, const int defaultHeight )
	{
		// TODO
		Window::width = defaultWidth;
		Window::height = defaultHeight;
		Window::resized = true;

		// Lazy-initialize the application singleton into the global 'NSApp' variable
		[GameApplication sharedApplication];

		// Turning ARC on in the compiler (default) is nto the end of the story
		// Objects marked 'autorelease' still require an active autorelease pool if you
		// don't want to leak memory
		pool = [NSAutoreleasePool new];

		// Get Screen Size
		NSScreen *screen = [NSScreen mainScreen];
		NSRect area = [screen frame];
		Window::scale = [screen backingScaleFactor];

		// Create Window
		SysWindow::window = [[GameWindow alloc] initWithContentRect: NSMakeRect( static_cast<int>( area.size.width - defaultWidth ) >> 1,
																				 static_cast<int>( area.size.height - defaultHeight ) >> 1,
																				 defaultWidth, defaultHeight )
																				 styleMask: WINDOW_STYLE
																				 backing: NSBackingStoreBuffered
																				 defer: NO ];

		// Check Window
		if( SysWindow::window == nil )
			{ ErrorReturnMsg( false, "COCOA: Failed to create cocoa window" ); }

		// Create Window View
		SysWindow::view = [GameView new];

		// Setup Window Properties
		[SysWindow::window setContentView: SysWindow::view];
		[SysWindow::window makeFirstResponder: SysWindow::view];
		[SysWindow::window setDelegate: (id)[GameWindowDelegate new]];
		[SysWindow::window setAcceptsMouseMovedEvents: YES];
		[SysWindow::window setTitle: @WINDOW_TITLE];
		[SysWindow::window setRestorable: NO];
		[SysWindow::window setContentMinSize: NSMakeSize( WINDOW_WIDTH_MIN, WINDOW_WIDTH_MAX ) ];

		// Setup Apple Menu
		// TODO

		// macOS applications that are not bundled must be explicitly made into UI
		// apps by calling this function otherwise we won't appear in the dock and
		// the window will not be visible
		[NSApp setActivationPolicy: NSApplicationActivationPolicyRegular];

		[NSApp finishLaunching];

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
		// Bring the Application into focus
		[NSApp activateIgnoringOtherApps: YES];

		// Bring the Window into focus
		[SysWindow::window makeKeyAndOrderFront: nil];
	}


	void poll()
	{
		//PROFILE_FUNCTION();

		for( ;; )
		{
			// Get the next event in the queue
			NSEvent *event = [NSApp nextEventMatchingMask: NSEventMaskAny
									untilDate: nil
									inMode: NSDefaultRunLoopMode
									dequeue: YES ];

			// There are no more events, break
			if( event == nil ) { break; }

			// Forward the event to the rest of the application
			[NSApp sendEvent:event];
		}

		// All of the memory management for Cocoa is done here
		// First, drain the pool (one always exists from window_init() )
		[pool drain];

		// Next, create the pool for the next frame
		pool = [NSAutoreleasePool new];
	}


	void set_caption( const char *caption )
	{
		// TODO
		//[SysWindow::window setTitle: @caption ];
	}


	void mouse_get_position( double &x, double &y )
	{
		NSRect frame = [SysWindow::view frame];
		NSPoint point = [SysWindow::window mouseLocationOutsideOfEventStream];
		x = point.x;
		y = frame.size.height - point.y;
	}


	void mouse_set_position( const int x, const int y )
	{
		NSRect rect = [SysWindow::window frame];
		CGWarpMouseCursorPosition( CGPointMake( rect.origin.x + x, rect.origin.y + y ) );

		// HACK
		CGAssociateMouseAndMouseCursorPosition( true );
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Window
{
	void show_message( const char *title, const char *message )
	{
		NSApplication *app = [NSApplication sharedApplication];
		NSAlert *alert = [[NSAlert alloc] init];
		[alert setIcon:nil];
		[alert setMessageText:[NSString stringWithUTF8String:title]];
		[alert setInformativeText:[NSString stringWithUTF8String:message]];
		[alert runModal];
	}


	void show_message_error( const char *title, const char *message )
	{
		NSApplication *app = [NSApplication sharedApplication];
		NSAlert *alert = [[NSAlert alloc] init];
		[alert setAlertStyle:NSAlertStyleCritical];
		[alert setMessageText:[NSString stringWithUTF8String:title]];
		[alert setInformativeText:[NSString stringWithUTF8String:message]];
		[alert runModal];
	}


	bool resize( int width, int height )
	{
		// Resize Window
		NSRect contentRect = { { 0, 0 }, { static_cast<CGFloat>( width ), static_cast<CGFloat>( height ) } };
		NSRect frame = [SysWindow::window frameRectForContentRect:contentRect];

		// Receneter window
		NSRect screenRect = [[NSScreen mainScreen] frame];
		frame.origin.x = ( screenRect.size.width - frame.size.width ) / 2;
		frame.origin.y = ( screenRect.size.height - frame.size.height ) / 2;

		[SysWindow::window setFrame:frame display:YES animate:YES];

		// Success
		return true;
	}


	void set_fullscreen( bool enabled )
	{
		if( enabled != Window::fullscreen )
		{
			[SysWindow::window toggleFullScreen:nil];

			// HACK
			for( int i = 0; i < 255; i++ )
			{
				Keyboard::state().keyCurrent[i] = false;
			}
		}
	}


	void set_caption( const char *caption )
	{
		// TODO
	}


	bool set_clipboard( const char *buffer )
	{
		// Get the clipboard
		NSPasteboard *pasteboard = [NSPasteboard generalPasteboard];
		if( !pasteboard ) { return false; }

		// Clear the clipboard
		[pasteboard clearContents];

		// Transfer buffer to clipboard
		NSString *text = [NSString stringWithUTF8String:buffer];
		if( !text ) { return false; }
		BOOL success = [pasteboard setString:text forType:NSPasteboardTypeString];
		return success;
	}


	bool get_clipboard( char *buffer, const usize size )
	{
		// Get the cipboard
		buffer[0] = '\0';
		NSPasteboard *pasteboard = [NSPasteboard generalPasteboard];
		if( !pasteboard ) { return false; }

		// Transfer text from the clipboard
		NSString *text = [pasteboard stringForType:NSPasteboardTypeString];
		if( !text ) { return false; }
		const char *clipboardText = [text UTF8String];
		if( !clipboardText ) { return false; }
		snprintf( buffer, size, "%s", clipboardText );
		return true;
	}


	bool set_selection( const char *buffer )
	{
		// Do nothing on MacOS
		return true;
	}


	bool get_selection( char *buffer, const usize size )
	{
		// Do nothing on MacOS
		buffer[0] = '\0';
		return true;
	}

}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////