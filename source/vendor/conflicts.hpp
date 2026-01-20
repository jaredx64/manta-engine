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

	#define min CONFLICT_min
	#define max CONFLICT_max

	// gl.h
	#define GLuint64 CONFLICT_GLuint64

	// stdlib.h
	#define random CONFLICT_random

	// docobj.h
	#define Print CONFLICT_Print

	// Finder.h
	#define FileInfo CONFLICT_FileInfo

	// x11.hpp
	#define Atom XAtom
	#define Font XFont
	#define Time XTime
	#define time xtime
	#define Window XWindow
	#define Display XDisplay
	#define Screen XScreen
	#define Visual XVisual
	#define VisualID XVisualID
	#define Cursor XCursor
	#define Colormap XColormap

#else

	#undef min
	#undef max

	// gl.h
	#undef GLuint64

	// stdlib.h
	#undef random

	// docobj.h
	#undef Print

	// Finder.h
	#undef FileInfo

	// x11.h
	#undef Font
	#undef Time
	#undef time
	#undef Window
	#undef Display
	#undef Screen
	#undef Visual
	#undef Cursor
	#undef Colormap

#undef CONFLICT_GUARD
#endif