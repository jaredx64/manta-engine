#pragma once
#include <vendor/config.hpp>

#if !USE_CUSTOM_C_HEADERS
	#include <vendor/conflicts.hpp>
		#include <stdarg.h>
	#include <vendor/conflicts.hpp>
#else
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// stdarg.h

	#if defined(__i386__) || defined(_M_IX86)
		using va_list = char *;

		#define va_start(a, v) (a = ((va_list)&(v)) + ((sizeof(v) + 3) & ~3))
		#define va_arg(a, t) (a += (sizeof(t) + 3) & ~3, *(t *)(a - ((sizeof(t) + 3) & ~3)))
		#define va_end(a)
	#elif defined(__x86_64__) || defined(_M_X64)
		#ifdef _WIN64
			using va_list = char *;

			#ifdef _MSC_VER
				extern "C" void __va_start(va_list *, ...);
				#define va_start(a, v) __va_start(&a, v)
			#else
				#define va_start(a, v) __builtin_va_start(a, v)
			#endif
			#define va_arg(a, t) ((sizeof(t) > 8 || (sizeof(t) & (sizeof(t) - 1))) ? \
				**(t **)((a += 8) - 8) : *(t *)((a += 8) - 8))
			#define va_end(a)
		#else
			using va_list = __builtin_va_list;

			#define va_start(a, v) __builtin_va_start(a, v)
			#define va_arg(a, t) __builtin_va_arg(a, t)
			#define va_end(a) __builtin_va_end(a)
		#endif
	#elif defined(__arm__) || defined(_M_ARM)
		// TODO: ARM32
		// ...
		static_assert( false, "No unofficial arm32 va_list implementation!" );
	#elif defined(__aarch64__) || defined(_M_ARM64)
		// TODO: ARM64
		// ...
		static_assert( false, "No unofficial arm64 va_list implementation!" );
	#else
		#error "Please provide a 'va_list' implementation for this platform!"
	#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif