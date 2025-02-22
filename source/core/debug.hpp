#pragma once

#include <config.hpp>

#include <core/color.hpp>

#include <vendor/stdarg.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef HEADER_ONLY_DEBUG
	// Note: HEADER_ONLY_DEBUG used by boot.exe
	#define HEADER_ONLY_DEBUG ( 0 )
#endif

#ifndef COMPILE_DEBUG
	#define COMPILE_DEBUG ( 0 )
#endif

#ifndef COMPILE_DEBUG_SYMBOLS
	#define COMPILE_DEBUG_SYMBOLS ( 0 )
#endif

#ifndef MEMORY_ASSERTS
	#define MEMORY_ASSERTS ( 0 )
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if COMPILE_DEBUG
	#define DEBUG(code) code
#else
	#define DEBUG(code)
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Debug
{
	extern int exitCode;
	extern bool memoryLeakDetection;
}

namespace Debug
{
	extern void exit( const int code );

	extern void print_formatted_variadic( const bool newline, const char *format, va_list args );
	extern void print_formatted_variadic_color( const bool newline, int color, const char *format, va_list args );
	extern void print_formatted( const bool newline, const char *format, ... );
	extern void print_formatted_color( const bool newline, int color, const char *format, ... );

	extern void manta_assert( const char *label, const char *caller, const char *file, const char *func,
		const int line, const char *condition );

	extern void manta_assert_message( const char *label, const char *caller, const char *file, const char *func,
		const int line, const char *condition, const char *msg, ... );

	extern bool snprint_callstack( char *buffer, const unsigned int size, int skip = 0,
		const char *prefix = "", const char *caller = "" );
	extern bool print_callstack( int skip = 0, const char *prefix = "", const char *caller = "" );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum
{
	LOG_DEFAULT = 0,
	LOG_BLACK = 30,
	LOG_RED = 31,
	LOG_GREEN = 32,
	LOG_YELLOW = 33,
	LOG_BLUE = 34,
	LOG_MAGENTA = 35,
	LOG_CYAN = 36,
	LOG_WHITE = 37,
};

#define Print( message, ... ) Debug::print_formatted( false, message, ##__VA_ARGS__ )
#define PrintColor( color, message, ... ) Debug::print_formatted_color( false, color, message, ##__VA_ARGS__ )
#define PrintLn( message, ... ) Debug::print_formatted( true, message, ##__VA_ARGS__ )
#define PrintLnColor( color, message, ... ) Debug::print_formatted_color( true, color, message, ##__VA_ARGS__ )

#if COMPILE_DEBUG
	#define DebugPrint( message, ... ) Debug::print_formatted( false, message, ##__VA_ARGS__ )
	#define DebugPrintColor( color, message, ... ) Debug::print_formatted_color( false, color, message, ##__VA_ARGS__ )
	#define DebugPrintLn( message, ... ) Debug::print_formatted( true, message, ##__VA_ARGS__ )
	#define DebugPrintLnColor( color, message, ... ) Debug::print_formatted_color( true, color, message, ##__VA_ARGS__ )
#else
	#define DebugPrint( message, ... )
	#define DebugPrintColor( color, message, ... )
	#define DebugPrintLn( message, ... )
	#define DebugPrintLnColor( color, message, ... )
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class EnumInfo
{
#if COMPILE_DEBUG
public:
	constexpr EnumInfo( const char *str, const Color col ) : str{ str }, col{ col } { };
	const char *name() const { return str; }
	const Color color() const { return col; }
private:
	const char *str = "";
	const Color col = c_white;
#else
public:
	constexpr EnumInfo( const char *str, const Color col ) { }
	const char *name() const { return "..."; }
	const Color color() const { return c_white; }
#endif
};

#define ENUM_INFO( name, count, ... ) \
	constexpr EnumInfo name[] = { __VA_ARGS__ }; \
	static_assert( ( sizeof( name ) / sizeof( name[0] ) ) == count, "Missing an enum(s)!" );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ERRORS

#define ERROR_HANDLER_FUNCTION __Error
#define ERROR_HANDLER_FUNCTION_DECL \
	ERROR_HANDLER_FUNCTION( const char *caller, const char *file, const char *func, const int line, \
		const char *condition, const char *message, ... ) \

extern void ERROR_HANDLER_FUNCTION_DECL;

#define Error( message, ... ) \
	ERROR_HANDLER_FUNCTION( "Error", __FILE__, __FUNCTION__, __LINE__, "", message, ##__VA_ARGS__ );

#define ErrorIf( condition, message, ... ) \
	if( condition ) \
	{ \
		ERROR_HANDLER_FUNCTION( "ErrorIf", __FILE__, __FUNCTION__, __LINE__, #condition, message, ##__VA_ARGS__ ); \
	}

#define ErrorReturn( value ) return value

#if COMPILE_DEBUG
	#define ErrorReturnIf( condition, value, message, ... ) \
		if( condition ) \
		{ \
			Debug::print_formatted_color( true, LOG_RED, "ERROR: " message " (%s)", ##__VA_ARGS__, __FILE__ ); \
			return value; \
		}

	#define ErrorReturnMsg( value, message, ... ) \
		Debug::print_formatted_color( true, LOG_RED, "ERROR: " message " (%s)", ##__VA_ARGS__, __FILE__ ); return value

	#define Warning( message, ... ) \
		Debug::print_formatted_color( true, LOG_YELLOW, "WARNING: " message, ##__VA_ARGS__ );
#else
	#define ErrorReturnIf( condition, value, message, ... ) \
		if( condition ) \
		{ \
			Debug::print_formatted_color( true, LOG_RED, "ERROR: " message, ##__VA_ARGS__ ); \
			return value; \
		}

	#define ErrorReturnMsg( value, message, ... ) \
		Debug::print_formatted_color( true, LOG_RED, "ERROR: " message, ##__VA_ARGS__ ); return value

	#define Warning( message, ... )
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ASSERTIONS

#define ASSERT_HANLDER_FUNCTION __Assert
#define ASSERT_HANLDER_FUNCTION_DECL \
	ASSERT_HANLDER_FUNCTION( const char *label, const char *caller, const char *file, const char *func, \
		const int line, const char *condition, const char *message, ... )

extern void ASSERT_HANLDER_FUNCTION_DECL;


#if COMPILE_DEBUG
	#define Assert( condition ) \
		if( !( condition ) ) \
		{ \
			ASSERT_HANLDER_FUNCTION( "ASSERTION:", "Assert", \
				__FILE__, __FUNCTION__, __LINE__, #condition, "" ); \
		}

	#define AssertMsg( condition, message, ... ) \
		if( !( condition ) ) \
		{ \
			ASSERT_HANLDER_FUNCTION( "ASSERTION:", "AssertMsg", \
				__FILE__, __FUNCTION__, __LINE__, #condition, message, ##__VA_ARGS__ ); \
		}

	#define MemoryWarning( message, ... ) \
		Debug::print_formatted_color( true, LOG_YELLOW, "WARNING: " message, ##__VA_ARGS__ );
#else
	#define Assert( condition )
	#define AssertMsg( condition, message, ... )
	#define MemoryWarning( message, ... )
#endif


#if MEMORY_ASSERTS
	#define MemoryAssert( condition ) \
		if( !( condition ) ) \
		{ \
			ASSERT_HANLDER_FUNCTION( "MEMORY ASSERTION:", "MemoryAssert", \
				__FILE__, __FUNCTION__, __LINE__, #condition, "" ); \
		}

	#define MemoryAssertMsg( condition, message, ... ) \
		if( !( condition ) ) \
		{ \
			ASSERT_HANLDER_FUNCTION( "MEMORY ASSERTION:", "MemoryAssertMsg", \
				__FILE__, __FUNCTION__, __LINE__, #condition, message, ##__VA_ARGS__ ); \
		}
#else
	#define MemoryAssert( condition )
	#define MemoryAssertMsg( condition, message, ... )
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SEGMENTATION FAULTS

#define SEGMENTATION_FAULT_HANLDER_FUNCTION __SegFault
#define SEGMENTATION_FAULT_HANLDER_FUNCTION_DECL \
	SEGMENTATION_FAULT_HANLDER_FUNCTION( const int code )

extern void SEGMENTATION_FAULT_HANLDER_FUNCTION_DECL;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if HEADER_ONLY_DEBUG
	// It's a bit gross to include a .cpp here, but build/project compile times
	// benefit from separating implementations from debug.hpp
	#include <core/debug.cpp>
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////