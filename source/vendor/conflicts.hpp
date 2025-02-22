// For compile times, there are optional defines 'OFFICIAL_HEADERS' and
// 'USE_OFFICIAL_HEADERS' that wrap around C headers with our own implentations
//
// Doing so significantly reduces the size and complexity of headers,
// which in turn reduces preprocessing and compile times
//
// Since we're already wrapping C headers in this way, it also allows us to
// circumvent global namespace conflicts such as stdlib's monopoly of 'random'

#ifndef CONFLICT_GUARD
#define CONFLICT_GUARD

	// ...
	#define min CONFLICT_min
	#define max CONFLICT_max

	// time.h (MacOS)
	#define time CONFLICT_time

	// stdlib.h
	#define random CONFLICT_random

	// x11.hpp
	#define Atom XAtom
	#define Font XFont
	#define Time XTime
	#define Window XWindow
	#define Display XDisplay
	#define Screen XScreen
	#define Visual XVisual
	#define VisualID XVisualID
	#define Cursor XCursor
	#define Colormap XColormap

#else

	// ...
	#undef min
	#undef max

	// time.h (MacOS)
	#undef time

	// stdlib.h
	#undef random

	// x11.h
	#undef Font
	#undef Time
	#undef Window
	#undef Display
	#undef Screen
	#undef Visual
	#undef Cursor

#undef CONFLICT_GUARD
#endif