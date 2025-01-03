#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// This header detects the local operating system & the compiler compiling the code
// In this context, "pipeline" refers to the *build* pipeline (where these defines are relevant)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if defined( __clang__ )
	#define PIPELINE_COMPILER_NAME  "clang"
	#define PIPELINE_COMPILER_CLANG ( 1 )
	#define PIPELINE_COMPILER_MSVC  ( 0 )
	#define PIPELINE_COMPILER_GCC   ( 0 )
#elif defined( __GNUC__ )
	#define PIPELINE_COMPILER_NAME  "gcc"
	#define PIPELINE_COMPILER_CLANG ( 0 )
	#define PIPELINE_COMPILER_MSVC  ( 0 )
	#define PIPELINE_COMPILER_GCC   ( 1 )
#elif defined( _MSC_VER )
	#define PIPELINE_COMPILER_NAME  "msvc"
	#define PIPELINE_COMPILER_CLANG ( 0 )
	#define PIPELINE_COMPILER_MSVC  ( 1 )
	#define PIPELINE_COMPILER_GCC   ( 0 )
#else
	static_assert( false, "Unsupported compiler!" );
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if defined( _WIN32 )
	#define PIPELINE_OS_NAME    "windows"
	#define PIPELINE_OS_WINDOWS ( 1 )
	#define PIPELINE_OS_LINUX   ( 0 )
	#define PIPELINE_OS_MACOS   ( 0 )
#elif defined( __linux__ )
	#define PIPELINE_OS_NAME    "linux"
	#define PIPELINE_OS_WINDOWS ( 0 )
	#define PIPELINE_OS_LINUX   ( 1 )
	#define PIPELINE_OS_MACOS   ( 0 )
#elif defined( __APPLE__ )
	#define PIPELINE_OS_NAME    "macOS"
	#define PIPELINE_OS_WINDOWS ( 0 )
	#define PIPELINE_OS_LINUX   ( 0 )
	#define PIPELINE_OS_MACOS   ( 1 )
#else
	static_assert( false, "Unsupported compiler OS!" );
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if PIPELINE_COMPILER_CLANG || PIPELINE_COMPILER_GCC
	#if defined( __x86_64__ )
		#define PIPELINE_ARCHITECTURE_NAME "x64"
		#define PIPELINE_ARCHITECTURE_64BIT ( true )
		#define PIPELINE_ARCHITECTURE_X64   ( 1 )
		#define PIPELINE_ARCHITECTURE_ARM64 ( 0 )
	#elif defined( __aarch64__ )
		#define PIPELINE_ARCHITECTURE_NAME "arm64"
		#define PIPELINE_ARCHITECTURE_64BIT ( true )
		#define PIPELINE_ARCHITECTURE_X64   ( 0 )
		#define PIPELINE_ARCHITECTURE_ARM64 ( 1 )
	#else
		static_assert( false, "Unsupported architecture!" );
	#endif
#elif PIPELINE_COMPILER_MSVC
	#if defined( _M_IX86_FP )
		static_assert( false, "Unsupported architecture!" );
	#elif defined( _M_X64 )
		#define PIPELINE_ARCHITECTURE_NAME  "x64"
		#define PIPELINE_ARCHITECTURE_64BIT ( true )
		#define PIPELINE_ARCHITECTURE_X64   ( 1 )
		#define PIPELINE_ARCHITECTURE_ARM64 ( 0 )
	#elif defined( _M_ARM )
		#if defined( _WIN64 )
			#define PIPELINE_ARCHITECTURE_NAME  "arm64"
			#define PIPELINE_ARCHITECTURE_64BIT ( true )
			#define PIPELINE_ARCHITECTURE_X64   ( 0 )
			#define PIPELINE_ARCHITECTURE_ARM64 ( 1 )
		#else
			static_assert( false, "Unsupported architecture!" );
		#endif
	#else
		static_assert( false, "Unsupported architecture!" );
	#endif
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////