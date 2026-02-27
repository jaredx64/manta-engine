#pragma once
#include <vendor/config.hpp>

#if !USE_CUSTOM_C_HEADERS
	#include <vendor/conflicts.hpp>
		#include <stdio.h>
	#include <vendor/conflicts.hpp>
#else
	#include <vendor/stddef.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// corecrt.h
	using errno_t = int;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// stdio.h

	using FILE = void *;

	#define SEEK_CUR ( 1 )
	#define SEEK_END ( 2 )
	#define SEEK_SET ( 0 )

	#define EOF ( -1 )

	extern "C" FILE *fopen( char const *, char const * );
	extern "C" errno_t fopen_s( FILE **, const char *, const char * );
	extern "C" errno_t freopen_s( FILE **, char const*, char const *, FILE * );
	extern "C" int fclose( FILE * );
	extern "C" size_t fwrite( void const *, size_t, size_t, FILE * );
	extern "C" size_t fread( void *, size_t, size_t, FILE * );
	extern "C" int fseek( FILE *, size_t, int );
	extern "C" size_t ftell( FILE * );
	extern "C" int fgetc( FILE * );
	extern "C" int getc( FILE * );
	extern "C" int ungetc( int, FILE * );
	extern "C" int ungetc( int, FILE * );
	extern "C" int feof( FILE * );
	extern "C" int ferror( FILE * );
	extern "C" char *fgets( char *, int, FILE );
	extern "C" int fputs( char const *, FILE * );
	extern "C" int fflush( FILE * );

	#if PIPELINE_OS_WINDOWS
		extern "C" FILE *_popen( const char *, const char * );
		extern "C" int _pclose( FILE * );
	#else
		extern "C" FILE *popen( const char *, const char * );
		extern "C" int pclose( FILE * );
	#endif

	#if PIPELINE_OS_WINDOWS
		#include <vendor/vendor.hpp>
		#include <vendor/stdarg.hpp>

		using _locale_t = void *;

		#define _CRT_INTERNAL_LOCAL_PRINTF_OPTIONS (*__local_stdio_printf_options())
		inline unsigned long long *__local_stdio_printf_options()
		{
			static unsigned long long _OptionsStorage;
			return &_OptionsStorage;
		}

		#define _CRT_INTERNAL_LOCAL_SCANF_OPTIONS (*__local_stdio_scanf_options())
		inline unsigned long long *__local_stdio_scanf_options()
		{
			static unsigned long long _OptionsStorage;
			return &_OptionsStorage;
		}

		#define _CRT_INTERNAL_PRINTF_LEGACY_VSPRINTF_NULL_TERMINATION (1ULL << 0)
		#define stdin ( __acrt_iob_func( 0 ) )
		#define stdout ( __acrt_iob_func( 1 ) )
		#define stderr ( __acrt_iob_func( 2 ) )

		extern "C" int __stdio_common_vsscanf(unsigned long long, char const *, size_t, char const *,
			_locale_t, va_list);
		extern "C" int __stdio_common_vsprintf(unsigned long long, char *, size_t, char const *,
			_locale_t, va_list);
		extern "C" int __stdio_common_vfprintf(unsigned long long, FILE *, const char *,
			_locale_t, va_list);
		extern "C" FILE *__acrt_iob_func(unsigned);

		inline int sscanf(char const *const buffer, char const *const format, ...)
		{
			int result;
			va_list args;
			va_start(args, format);
			result = __stdio_common_vsscanf(_CRT_INTERNAL_LOCAL_SCANF_OPTIONS, buffer, (size_t)-1,
				format, nullptr, args);
			va_end(args);
			return result;
		}

		inline int printf(const char *format, ...)
		{
			int result;
			va_list args;
			va_start(args, format);
			result = __stdio_common_vfprintf(_CRT_INTERNAL_LOCAL_PRINTF_OPTIONS, stdout, format, nullptr, args);
			va_end(args);
			return result;
		}

		inline int sprintf(char *buffer, const char *format, ...)
		{
			int result;
			va_list args;
			va_start(args, format);
			result = __stdio_common_vsprintf(
				_CRT_INTERNAL_LOCAL_PRINTF_OPTIONS | _CRT_INTERNAL_PRINTF_LEGACY_VSPRINTF_NULL_TERMINATION,
				buffer,
				static_cast<size_t>( -1 ),
				format,
				nullptr,
				args
			);
			va_end(args);
			return result < 0 ? -1 : result;
		}

		inline int snprintf(char *buffer, size_t size, const char *format, ...)
		{
			int result;
			va_list args;
			va_start(args, format);
			result = __stdio_common_vsprintf(
				_CRT_INTERNAL_LOCAL_PRINTF_OPTIONS | _CRT_INTERNAL_PRINTF_LEGACY_VSPRINTF_NULL_TERMINATION,
				buffer,
				size,
				format,
				nullptr,
				args
			);
			va_end(args);
			return result < 0 ? -1 : result;
		}

		inline int vprintf(const char *format, va_list args)
		{
			int result;
			result = __stdio_common_vfprintf(_CRT_INTERNAL_LOCAL_PRINTF_OPTIONS, stdout, format, nullptr, args);
			return result;
		}

		inline int vsprintf(char *buffer, const char *format, va_list args)
		{
			int result;
			result = __stdio_common_vsprintf(
				_CRT_INTERNAL_LOCAL_PRINTF_OPTIONS | _CRT_INTERNAL_PRINTF_LEGACY_VSPRINTF_NULL_TERMINATION,
				buffer,
				static_cast<size_t>( -1 ),
				format,
				nullptr,
				args
			);
			return result < 0 ? -1 : result;
		}

		inline int vsnprintf( char *buffer, size_t size, const char *format, va_list args )
		{
			int result;
			result = __stdio_common_vsprintf(
				_CRT_INTERNAL_LOCAL_PRINTF_OPTIONS | _CRT_INTERNAL_PRINTF_LEGACY_VSPRINTF_NULL_TERMINATION,
				buffer,
				size,
				format,
				nullptr,
				args
			);
			return result < 0 ? -1 : result;
		}
	#else
		extern "C" int sscanf( const char *, const char *, ... );
		extern "C" int printf( const char *, ... );
		extern "C" int sprintf( char *, const char *, ... );
		extern "C" int snprintf( char *, size_t, const char *, ... );
		extern "C" int vprintf( const char *, __builtin_va_list );
		extern "C" int vsprintf( char *, const char *, __builtin_va_list );
		extern "C" int vsnprintf( char *, size_t, const char *, __builtin_va_list );

		extern "C" FILE *stdin;
		extern "C" FILE *stdout;
		extern "C" FILE *stderr;

		extern "C" int remove( const char * );
		extern "C" int rename( const char *, const char * );
	#endif

	#if PIPELINE_OS_WINDOWS
		#define popen(command, mode) _popen( command, mode )
		#define pclose(stream) _pclose( stream )
	#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif