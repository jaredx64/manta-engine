#pragma once
#include <vendor/config.hpp>

#if !USE_CUSTOM_C_HEADERS
	#include <vendor/conflicts.hpp>
		#include <string.h>
	#include <vendor/conflicts.hpp>
#else
	#include <vendor/stddef.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// string.h

	extern "C" void *memset( void *, int, size_t );
	extern "C" void *memcpy( void *, const void *, size_t );
	extern "C" void *memmove( void *, const void *, size_t );
	extern "C" int memcmp( const void *, const void *, size_t );
	extern "C" char *strcpy( char *, const char * );
	extern "C" char *strncpy( char *, const char *, size_t );
	extern "C" int strcmp( const char *, const char * );
	extern "C" int strncmp( const char *, const char *, size_t );
	extern "C" size_t strlen( const char * );
	extern "C" char *strcat( char *, const char * );
	extern "C" char *strncat( char *, const char *, size_t );
	extern "C" char *strstr( const char *, const char * );
	extern "C" char *strchr( const char *, int );
	extern "C" char *strrchr( const char *, int );
	extern "C" void *memchr( const void *, int, size_t );
	extern "C" size_t strspn( const char *, const char * );
	extern "C" size_t strcspn( char const *, char const * );

	#if PIPELINE_COMPILER_MSVC
		// NOTE: MSVC wants to know that the above declarations are intended to be used as compiler intrinsics
		#pragma intrinsic( memset )
		#pragma intrinsic( memcpy )
		#pragma intrinsic( memmove )
		#pragma intrinsic( memcmp )
		#pragma intrinsic( strcpy )
		#pragma intrinsic( strcmp )
		#pragma intrinsic( strlen )
		#pragma intrinsic( strcat )
	#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif