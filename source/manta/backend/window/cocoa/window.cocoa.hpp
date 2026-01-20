#pragma once

#include <pipeline.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef __OBJC__
	#include <vendor/stddef.hpp>

	#include <vendor/conflicts.hpp>
		#import <Cocoa/Cocoa.h>
	#include <vendor/conflicts.hpp>

#if GRAPHICS_METAL
	@interface GameView : NSView <CALayerDelegate> @end
#else
	@interface GameView : NSView @end
#endif

	namespace CoreWindow
	{
		extern GameView *view;
		extern NSWindow *window;
		extern NSAutoreleasePool *pool;
	}
#else
	static_assert( false, "Not Objective-C!" );
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////