#include <core/debug.hpp>

#include <core/types.hpp>

#include <vendor/stdio.hpp>
#include <vendor/stdlib.hpp>
#include <vendor/string.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef COMPILE_ENGINE
	#if defined( SHOW_ERROR_MESSAGES ) && SHOW_ERROR_MESSAGES
		#include <manta/window.hpp> // For show_message_error()
	#else
		#define SHOW_ERROR_MESSAGES ( false )
	#endif
#else
	#ifdef SHOW_ERROR_MESSAGES
		#undef SHOW_ERROR_MESSAGES
	#endif
	#define SHOW_ERROR_MESSAGES ( false )
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Debug
{
	int exitCode = 0;
	bool memoryLeakDetection = true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void str_append( char *buffer, const usize size, const char *string )
{
	if( buffer == nullptr || string == nullptr || size == 0 ) { return; }
	const usize bufferLength = strlen( buffer );
	strncat( buffer, string, size - bufferLength - 1 );
	buffer[size - 1] = '\0';
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Debug::exit( int code )
{
	Debug::exitCode = code;
	::exit( code );
}


void Debug::print_formatted( const bool newline, const char *format, ... )
{
	va_list args;
	va_start( args, format );
	vprintf( format, args );
	if( newline ) { printf( "\n" ); }
	va_end( args );
}


void Debug::print_formatted_variadic( const bool newline, const char *format, va_list args )
{
	vprintf( format, args );
	if( newline ) { printf( "\n" ); }
}


void Debug::print_formatted_color( const bool newline, int color, const char *format, ... )
{
	va_list args;
	va_start( args, format );
	printf( "\x1b[%dm", color );
	vprintf( format, args );
	printf( newline ? "\x1b[%dm\n" : "\x1b[%dm", LOG_DEFAULT );
	va_end( args );
}


void Debug::print_formatted_variadic_color( const bool newline, int color, const char *format, va_list args )
{
	printf( "\x1b[%dm", color );
	vprintf( format, args );
	printf( newline ? "\x1b[%dm\n" : "\x1b[%dm", LOG_DEFAULT );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ERROR_HANDLER_FUNCTION_DECL
{
	if( Debug::exitCode != 0 ) { return; }

	char callerAlias[256];
	char conditionLine[1024]; conditionLine[0] = '\0';
	char messageLine[1024]; messageLine[0] = '\0';
	char callstack[2048]; callstack[0] = '\0';

	// Caller Alias
	if( caller[0] != '\0' )
	{
		if( condition[0] != '\0' )
		{
			snprintf( callerAlias, sizeof( callerAlias ), "%s( %s )", caller, condition );
		}
		else
		{
			snprintf( callerAlias, sizeof( callerAlias ), "%s", caller );
		}
	}

	// Print Condition
	PrintColor( LOG_RED, "\n\nERROR:\n\n" );
	if( condition[0] != '\0' )
	{
		snprintf( conditionLine, sizeof( conditionLine ), "%s( %s )", caller, condition );
		PrintColor( LOG_YELLOW, "    %s\n\n", conditionLine );
	}

	// Print Message
	if( message[0] != '\0' )
	{
		va_list args;
		va_start( args, message );
		vsnprintf( messageLine, sizeof( messageLine ), message, args );
		va_end( args );

		PrintColor( LOG_WHITE, "    " );
		PrintColor( LOG_WHITE, messageLine );
		PrintColor( LOG_WHITE, "\n\n" );
	}

	// Retrieve Callstack
	if( !Debug::snprint_callstack( callstack, sizeof( callstack ), 1, "    > ", callerAlias ) )
	{
		snprintf( callstack, sizeof( callstack ), "    > %s (%s:%d)\n", func, file, line );
	}

	// Print Callstack
	PrintColor( LOG_RED, "    CALLSTACK:\n" );
	PrintColor( LOG_RED, callstack );
	PrintColor( LOG_RED, "\n" );

	// Error Message Box
#if SHOW_ERROR_MESSAGES
	char errorMsg[4096]; errorMsg[0] = '\0';
	str_append( errorMsg, sizeof( errorMsg ), "ERROR:\n\n" );

	if( conditionLine[0] != '\0' )
	{
		str_append( errorMsg, sizeof( errorMsg ), "    " );
		str_append( errorMsg, sizeof( errorMsg ), conditionLine );
		str_append( errorMsg, sizeof( errorMsg ), "\n\n" );
	}

	if( messageLine[0] != '\0' )
	{
		str_append( errorMsg, sizeof( errorMsg ), "    " );
		str_append( errorMsg, sizeof( errorMsg ), messageLine );
		str_append( errorMsg, sizeof( errorMsg ), "\n\n" );
	}

	if( callstack[0] != '\0' )
	{
		str_append( errorMsg, sizeof( errorMsg ), "CALLSTACK:\n" );
		str_append( errorMsg, sizeof( errorMsg ), callstack );
		str_append( errorMsg, sizeof( errorMsg ), "\n" );
	}

	Window::show_message_error( "ERROR!", errorMsg );
#endif

	// Exit
	Debug::exit( 1 );
}


void ASSERT_HANLDER_FUNCTION_DECL
{
	if( Debug::exitCode != 0 ) { return; }

	char callerAlias[256];
	char conditionLine[1024]; conditionLine[0] = '\0';
	char messageLine[1024]; messageLine[0] = '\0';
	char callstack[2048]; callstack[0] = '\0';

	// Caller Alias
	if( caller[0] != '\0' )
	{
		if( condition[0] != '\0' )
		{
			snprintf( callerAlias, sizeof( callerAlias ), "%s( %s )", caller, condition );
		}
		else
		{
			snprintf( callerAlias, sizeof( callerAlias ), "%s", caller );
		}
	}

	// Print Condition
	PrintColor( LOG_RED, "\n\n%s\n\n", label );
	if( condition[0] != '\0' )
	{
		snprintf( conditionLine, sizeof( conditionLine ), "%s( %s )", caller, condition );
		PrintColor( LOG_YELLOW, "    %s\n\n", conditionLine );
	}

	// Print Message
	if( message[0] != '\0' )
	{
		va_list args;
		va_start( args, message );
		vsnprintf( messageLine, sizeof( messageLine ), message, args );
		va_end( args );

		PrintColor( LOG_WHITE, "    " );
		PrintColor( LOG_WHITE, messageLine );
		PrintColor( LOG_WHITE, "\n\n" );
	}

	// Retrieve Callstack
	if( !Debug::snprint_callstack( callstack, sizeof( callstack ), 1, "    > ", callerAlias ) )
	{
		snprintf( callstack, sizeof( callstack ), "    > %s (%s:%d)\n", func, file, line );
	}

	// Print Callstack
	PrintColor( LOG_RED, "    CALLSTACK:\n" );
	PrintColor( LOG_RED, callstack );
	PrintColor( LOG_RED, "\n" );

	// Error Message Box
#if SHOW_ERROR_MESSAGES
	char errorMsg[4096]; errorMsg[0] = '\0';
	str_append( errorMsg, sizeof( errorMsg ), "ASSERTION:\n\n" );

	if( conditionLine[0] != '\0' )
	{
		str_append( errorMsg, sizeof( errorMsg ), "    " );
		str_append( errorMsg, sizeof( errorMsg ), conditionLine );
		str_append( errorMsg, sizeof( errorMsg ), "\n\n" );
	}

	if( messageLine[0] != '\0' )
	{
		str_append( errorMsg, sizeof( errorMsg ), "    " );
		str_append( errorMsg, sizeof( errorMsg ), messageLine );
		str_append( errorMsg, sizeof( errorMsg ), "\n\n" );
	}

	if( callstack[0] != '\0' )
	{
		str_append( errorMsg, sizeof( errorMsg ), "CALLSTACK:\n" );
		str_append( errorMsg, sizeof( errorMsg ), callstack );
		str_append( errorMsg, sizeof( errorMsg ), "\n" );
	}

	Window::show_message_error( "ASSERTION!", errorMsg );
#endif

	// Exit
	Debug::exit( 1 );
}


void SEGMENTATION_FAULT_HANLDER_FUNCTION_DECL
{
	if( Debug::exitCode != 0 ) { return; }

	char callstack[2048]; callstack[0] = '\0';

	// Print Condition
	PrintColor( LOG_RED, "\n\nSEGMENTATION FAULT %d\n\n", code );

	// Retrieve Callstack
	if( !Debug::snprint_callstack( callstack, sizeof( callstack ), 2, "    > ", "SEGMENTATION FAULT" ) )
	{
		snprintf( callstack, sizeof( callstack ), "    > unable to retrieve callstack" );
	}

	// Print Callstack
#if COMPILE_DEBUG
	PrintColor( LOG_RED, "    CALLSTACK:\n" );
	PrintColor( LOG_RED, callstack );
	PrintColor( LOG_RED, "\n" );
#endif

	// Error Message Box
#if SHOW_ERROR_MESSAGES
	char errorMsg[2048];
	snprintf( errorMsg, sizeof( errorMsg ), "CALLSTACK:\n%s", callstack );
	Window::show_message_error( "SEGMENTATION FAULT!", errorMsg );
#endif

	// Exit
	Debug::exit( code );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if COMPILE_DEBUG
#include <vendor/stdlib.hpp>
#include <vendor/stdio.hpp>

struct SymbolInfo
{
	char name[256];
	char path[256];
	int line;
};

static bool callstack_symbol_clean_path(const char *path, char *buffer, usize size )
{
	buffer[0] = '\0';
	char *resolved = realpath( path, nullptr );

	if( resolved != nullptr )
	{
		snprintf( buffer, size, "%s", resolved );
		free( resolved );
		return true;
	}

	return false;
}

#define CALLSTACK_DEPTH ( 128 )


#if OS_WINDOWS
	#include <vendor/windows.hpp>
	#include <vendor/dbghelp.hpp>

	struct CallstackContext
	{
		HANDLE process;
		HMODULE dbghelp;
		SymInitializeFunc SymInitialize;
		RtlCaptureStackBackTraceFunc RtlCaptureStackBackTrace;
		SymFromAddrFunc SymFromAddr;
		SymGetLineFromAddrFunc SymGetLineFromAddr;
		SymCleanupFunc SymCleanup;
	};

	static bool callstack_context_init( CallstackContext &context )
	{
		// Dynamic link to dbghelp.dll
		context.dbghelp = LoadLibraryA( "dbghelp.dll" );
		if( !context.dbghelp ) { return false; }

		context.SymInitialize =
			reinterpret_cast<SymInitializeFunc>( GetProcAddress( context.dbghelp, "SymInitialize" ) );

		context.RtlCaptureStackBackTrace =
			reinterpret_cast<RtlCaptureStackBackTraceFunc>(
				GetProcAddress( GetModuleHandleA( "ntdll.dll" ), "RtlCaptureStackBackTrace" ) );

		context.SymFromAddr =
			reinterpret_cast<SymFromAddrFunc>( GetProcAddress( context.dbghelp, "SymFromAddr" ) );

		context.SymGetLineFromAddr =
			reinterpret_cast<SymGetLineFromAddrFunc>( GetProcAddress( context.dbghelp, "SymGetLineFromAddr64" ) );

		context.SymCleanup =
			reinterpret_cast<SymCleanupFunc>( GetProcAddress( context.dbghelp, "SymCleanup" ) );

		// Validate dynamic linkage
		if( !context.SymInitialize || !context.RtlCaptureStackBackTrace ||
			!context.SymFromAddr || !context.SymGetLineFromAddr || !context.SymCleanup )
		{
			FreeLibrary( context.dbghelp );
			return false;
		}

		// Get Current Process
		context.process = GetCurrentProcess();
		if( !context.SymInitialize( context.process, nullptr, true ) )
		{
			context.SymCleanup( context.process );
			FreeLibrary( context.dbghelp );
			return false;
		}

		// Success
		return true;
	}

	static void callstack_context_free( CallstackContext &context )
	{
		context.SymCleanup( context.process );
		FreeLibrary( context.dbghelp );
	}

	static bool callstack_filter_symbol( const char *name )
	{
		const char *exclusions[] =
		{
			"dynamic atexit destructor",
			"std::",
			"VCCRT\\vcruntime",
			"minkernel\\crts",
			"common_exit",
			"RtlFindCharInUnicodeString",
			"_chkstk",
			"_seh_filter_exe",
			"_C_specific_handler",
			"__C_specific_handler",
			"__crt_seh_guarded_call",
			"__scrt_common_main_seh",
		};
		const usize exclusionsCount = sizeof( exclusions ) / sizeof( exclusions[0] );

		// Test symbol name against exclusions
		for( usize i = 0; i < exclusionsCount; ++i )
		{
			if( strstr( name, exclusions[i] ) != nullptr ) { return true; }
		}

		// Success - no need to filter
		return false;
	};

	static bool callstack_get_symbol_info( const CallstackContext &context,
		void *address, SYMBOL_INFO *symbol, SymbolInfo &info )
	{
		// Nullify SymbolInfo
		info.name[0] = '\0';
		info.path[0] = '\0';
		info.line = 0;

		// Get symbol name
		if( !context.SymFromAddr( context.process, reinterpret_cast<DWORD64>( address ), nullptr, symbol ) )
		{
			return false;
		}
		if( callstack_filter_symbol( symbol->Name ) ) { return false; }
		snprintf( info.name, sizeof( info.name ), "%s", symbol->Name );

		// Get path and line number
		DWORD displacement = 0;
		IMAGEHLP_LINE line;
		memset( &line, 0, sizeof( IMAGEHLP_LINE ) );
		line.SizeOfStruct = sizeof( IMAGEHLP_LINE );
		if( context.SymGetLineFromAddr( context.process, reinterpret_cast<DWORD64>( address ), &displacement, &line ) )
		{
			snprintf( info.path, sizeof( info.path ), line.FileName );
			info.line = line.LineNumber;
		}

		// Success
		return true;
	}

	static bool callstack_symbols_windows( SymbolInfo *&symbols, int &count )
	{
		CallstackContext context;
		if( !callstack_context_init( context ) ) { return false; }

		// Get Callstack
		void *callstack[CALLSTACK_DEPTH];
		const WORD frames = context.RtlCaptureStackBackTrace( 0, CALLSTACK_DEPTH, callstack, nullptr );

		// Allocate symbol buffer
		SYMBOL_INFO *symbol = reinterpret_cast<SYMBOL_INFO *>( malloc( sizeof( SYMBOL_INFO ) + 256 ) );
		symbol->MaxNameLen = 255;
		symbol->SizeOfStruct = sizeof( SYMBOL_INFO );

		// Retrieve Callstack
		count = 0;
		for( int i = 0; i < static_cast<int>( frames ) - 3; i++ )
		{
			if( !callstack_get_symbol_info( context, callstack[i], symbol, symbols[count] ) ) { continue; }
			count++;
		}

		// Success
		free( symbol );
		callstack_context_free( context );
		return true;
	}

#elif OS_MACOS && USE_OFFICIAL_HEADERS
	#include <vendor/execinfo.hpp>
	#include <mach-o/dyld.h>

	struct CallstackContext
	{
		const char *exe;
		void *address;
	};

	static bool callstack_context_init( CallstackContext &context )
	{
		const char *executable = getprogname();
		const u32 images = _dyld_image_count();

		for( u32 i = 0; i < images; ++i )
		{
			context.exe = _dyld_get_image_name( i );
			if( strstr( context.exe, executable ) != nullptr )
			{
				context.address = reinterpret_cast<void *>( const_cast<mach_header *>( _dyld_get_image_header( i ) ) );
				return true;
			}
		}

		return false;
	}

	static bool callstack_get_symbol_info( const CallstackContext &context, const char *symbol, SymbolInfo &info )
	{
		// Nullify SymbolInfo
		info.name[0] = '\0';
		info.path[0] = '\0';
		info.line = 0;

		// backtrace_symbols() produces symbols like: "2 exe 0x0000000100997374 _ZN7Project6updateEd + 416"
		// We need to extract the address ('0x...') in order to fetch it later with 'atos'
		const char *addressBegin = strstr( symbol, "0x" );
		if( addressBegin == nullptr ) { return false; }
		const char *addressEnd = strchr( addressBegin, ' ' );
		if( addressEnd == nullptr ) { return false; }
		const int addressLength = static_cast<int>( addressEnd - addressBegin );
		char symbolAddress[256];
		snprintf( symbolAddress, sizeof( symbolAddress ), "%.*s", addressLength, addressBegin );

		// Execute 'atos' command
		char command[256];
		snprintf( command, 256, "atos -o %s -arch arm64 -l %p -fullPath %s", context.exe, context.address, symbolAddress );
		FILE *fp = popen( command, "r" );
		if( fp == nullptr ) { return false; }

		// Parse Symbol Info
		char line[1024];
		while( fgets( line, sizeof( line ), fp ) != nullptr )
		{
			// Extract Function Name
			const char *functionBegin = line;
			if( functionBegin == nullptr ) { pclose( fp ); return true; }
			const char *functionEndSpace = strchr( functionBegin, ' ' );
			const char *functionEndParen = strchr( functionBegin, '(' );
			const char *functionEnd = functionEndSpace < functionEndParen ? functionEndSpace : functionEndParen;
			if( functionEnd == nullptr ) { pclose( fp ); return true; }
			const int functionLength = static_cast<int>( functionEnd - functionBegin );
			snprintf( info.name, sizeof( info.name ), "%.*s", functionLength, functionBegin );

			// Extract Filepath
			const char *filepathBegin = strchr( functionEnd, '/' );
			if( filepathBegin == nullptr ) { pclose( fp ); return true; }
			const char *filepathEnd = strchr( filepathBegin, ':' );
			if( filepathEnd == nullptr ) { pclose( fp ); return true; }
			const int filepathLength = static_cast<int>( filepathEnd - filepathBegin );
			char filepath[256];
			snprintf( filepath, sizeof( filepath ), "%.*s", filepathLength, filepathBegin );
			if( !callstack_symbol_clean_path( filepath, info.path, sizeof( info.path ) ) )
			{
				pclose( fp );
				return true;
			}

			// Extract Line
			const char *lineBegin = filepathEnd + 1;
			if( lineBegin == nullptr || *lineBegin == '\0' ) { pclose( fp ); return true; }
			const char *lineEnd = strchr( lineBegin, ')' );
			if( lineEnd == nullptr ) { pclose( fp ); return true; }
			const int lineLength = static_cast<int>( lineEnd - lineBegin );
			char line[16];
			snprintf( line, sizeof( line ), "%.*s", lineLength, lineBegin );
			info.line = atoi( line );

			// Success
			pclose( fp );
			return true;
		}

		// Failure
		pclose( fp );
		return true;
	}

	static bool callstack_symbols_macos( SymbolInfo *&symbols, int &count )
	{
		// Initialize Context
		CallstackContext context;
		if( !callstack_context_init( context ) ) { return false; }

		// Get backtrace
		void *callstack[CALLSTACK_DEPTH];
		usize frames = backtrace( callstack, CALLSTACK_DEPTH );
		if( frames == 0 ) { return false; }

		// Get backtrace symbols
		char **backtraceSymbols = backtrace_symbols( callstack, frames );
		if( backtraceSymbols == nullptr ) { return false; }

		// Retrieve Callstack
		count = 0;
		for( int i = 0; i < frames - 1; i++ )
		{
			if( !callstack_get_symbol_info( context, backtraceSymbols[i], symbols[count] ) ) { continue; }
			count++;
		}

		// Success
		free( backtraceSymbols );
		return true;
	}

#elif OS_LINUX
	#include <vendor/execinfo.hpp>
	#include <vendor/dlfcn.hpp>
	#include <vendor/cxxabi.hpp>

	struct CallstackContext
	{
		char exe[256];
	};

	static bool callstack_context_init( CallstackContext &context )
	{
		// Get executable path
		Dl_info info;
		if( dladdr( reinterpret_cast<void *>( callstack_context_init ), &info ) )
		{
			snprintf( context.exe, sizeof( context.exe ), "%s", info.dli_fname );
			return true;
		}

		// Failure
		return false;
	}

	static bool callstack_get_symbol_info( const CallstackContext &context, const char *symbol, SymbolInfo &info )
	{
		// Nullify SymbolInfo
		info.name[0] = '\0';
		info.path[0] = '\0';
		info.line = 0;

		// On Linux, backtrace_symbols() produces symbols like: "path/to/exe(+0x4e493) [0x57d11b7d5493]"
		// We need to extract the sybol address ('0x...') in order to fetch it later with 'addr2line'

		const char *addressBegin = strstr( symbol, "0x" );
		if( addressBegin == nullptr ) { return false; }
		const char *addressEnd = strchr( addressBegin, ')' );
		if( addressEnd == nullptr ) { return false; }
		const int addressLength = static_cast<int>( addressEnd - addressBegin );
		char symbolAddress[256];
		snprintf( symbolAddress, sizeof( symbolAddress ), "%.*s", addressLength, addressBegin );

		// Execute 'addr2line' command
		char command[256];
		snprintf( command, 256, "addr2line -e %s %s -f", context.exe, symbolAddress );
		FILE *fp = popen( command, "r" );
		if( fp == nullptr ) { return false; }

		// Parse Symbol Info
		int i = 0;
		char line[1024];
		while( fgets( line, sizeof( line ), fp ) != nullptr )
		{
			// First line is mangled function name
			if( i == 0 )
			{
				// Demangle symbol
				char symbolMangled[256];
				const int length = static_cast<int>( strlen( line ) - 1 );
				snprintf( symbolMangled, sizeof( symbolMangled ), "%.*s", length, line );
				int status;
				char *symbolDemangled = abi::__cxa_demangle( symbolMangled, nullptr, nullptr, &status );
				if( status ) { pclose( fp ); return true; }

				// Extract just the function name
				const char *functionBegin = symbolDemangled;
				if( functionBegin == nullptr ) { pclose( fp ); return true; }
				const char *functionEnd = strchr( functionBegin, '(' );
				if( functionEnd == nullptr ) { pclose( fp ); return true; }
				const int functionLength = static_cast<int>( functionEnd - functionBegin );
				snprintf( info.name, sizeof( info.name ), "%.*s", functionLength, functionBegin );
				free( symbolDemangled );

				// Success
				i++;
				continue;
			}
			else if( i == 1 )
			// Second line is symbol path + line number
			{
				// Extract path
				const char *filepathBegin = line;
				if( filepathBegin == nullptr ) { pclose( fp ); return true; }
				const char *filepathEnd = strchr( filepathBegin, ':' );
				if( filepathEnd == nullptr ) { pclose( fp ); return true; }
				const int filepathLength = static_cast<int>( filepathEnd - filepathBegin );
				char filepath[256];
				snprintf( filepath, sizeof( filepath ), "%.*s", filepathLength, filepathBegin );

				if( !callstack_symbol_clean_path( filepath, info.path, sizeof( info.path ) ) )
				{
					pclose( fp );
					return true;
				}

				// Extract Line
				const char *lineBegin = filepathEnd + 1;
				if( lineBegin == nullptr || *lineBegin == '\0' ) { pclose( fp ); return true; }
				const char *lineEndSpace = strchr( lineBegin, ' ' );
				const char *lineEndNewline = strchr( lineBegin, '\n' );
				const char *lineEnd = lineEndSpace < lineEndNewline ? lineEndNewline : lineEndSpace;
				if( lineEnd == nullptr ) { pclose( fp ); return true; }
				const int lineLength = static_cast<int>( lineEnd - lineBegin );
				char line[16];
				snprintf( line, sizeof( line ), "%.*s", lineLength, lineBegin );
				info.line = atoi( line );

				// Success
				pclose( fp );
				return true;
			}
		}

		// Success
		pclose( fp );
		return true;
	}

	static bool callstack_symbols_linux( SymbolInfo *&symbols, int &count )
	{
		// Initialize Context
		CallstackContext context;
		if( !callstack_context_init( context ) ) { return false; }

		// Get backtrace
		void *callstack[CALLSTACK_DEPTH];
		const int frames = backtrace( callstack, CALLSTACK_DEPTH );
		if( frames == 0 ) { return false; }

		// Get backtrace symbols
		char **backtraceSymbols = backtrace_symbols( callstack, frames );
		if( backtraceSymbols == nullptr ) { return false; }

		// Retrieve Callstack
		count = 0;
		for( int i = 0; i < frames - 4; i++ )
		{
			if( !callstack_get_symbol_info( context, backtraceSymbols[i], symbols[count] ) ) { continue; }
			count++;
		}

		// Success
		free( backtraceSymbols );
		return true;
	}
#endif
#endif


bool Debug::snprint_callstack( char *buffer, const unsigned int size, const int skip,
	const char *prefix, const char *caller )
{
#if COMPILE_DEBUG
	int count = 0;
	SymbolInfo *symbols = reinterpret_cast<SymbolInfo *>( malloc( sizeof( SymbolInfo ) * CALLSTACK_DEPTH ) );
	if( symbols == nullptr ) { return false; }

	// Retrive Symbols
	{
	#if OS_WINDOWS
		if( !callstack_symbols_windows( symbols, count ) ) { return false; }
	#elif OS_MACOS && USE_OFFICIAL_HEADERS
		if( !callstack_symbols_macos( symbols, count ) ) { return false; }
	#elif OS_LINUX
		if( !callstack_symbols_linux( symbols, count ) ) { return false; }
	#else
		return false;
	#endif
	}

	// Print Symbols
	char scratch[256];
	buffer[0] = '\0';
	for( int i = count - 1; i > skip; i-- )
	{
		SymbolInfo &function = symbols[i];
		SymbolInfo &callsite = symbols[i + ( i < count )];

		const char *name = ( i == skip + 1 && caller[0] != '\0' ) ?
			( caller ) : ( function.name[0] == '\0' ? "..." : function.name );

		if( callsite.path[0] != '\0' )
		{
			snprintf( scratch, sizeof( scratch ), "%s%s (%s:%d)\n", prefix, name, callsite.path, callsite.line );
			str_append( buffer, size, scratch );
		}
		else if( name[0] != '\0' )
		{
			snprintf( scratch, sizeof( scratch ), "%s%s\n", prefix, name );
			str_append( buffer, size, scratch );
		}
	}

	free( symbols );
	return true;
#else
	// Callstacks only available in debug builds
	return false;
#endif
}


bool Debug::print_callstack( const int skip, const char *prefix, const char *caller )
{
#if COMPILE_DEBUG
	char callstack[4096];
	snprint_callstack( callstack, sizeof( callstack ), skip, prefix, caller );
	PrintColor( LOG_RED, callstack );
	return true;
#else
	// Callstacks only available in debug builds
	return false;
#endif
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////