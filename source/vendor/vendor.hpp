#pragma once
#include <vendor/config.hpp>

#if defined( _MSC_VER )

	// MSVC
	#ifndef STD_CALL
	#define STD_CALL __stdcall
	#endif

	#ifndef FORCE_INLINE
	#define FORCE_INLINE __forceinline
	#endif

	#ifndef NO_INLINE
	#define NO_INLINE __declspec(noinline)
	#endif

	#ifndef NO_RETURN
	#define NO_RETURN __declspec(noreturn)
	#endif

	#ifndef NO_STRIP
	#define NO_STRIP
	#endif

	#ifndef MALLOC_LIKE
	#define MALLOC_LIKE __declspec(restrict)
	#endif

	#ifndef DLL_IMPORT
	#define DLL_IMPORT __declspec(dllimport)
	#endif

	#ifndef MAY_ALIAS
	#define MAY_ALIAS
	#endif

	#ifndef UNREACHABLE
	#define UNREACHABLE __assume(0)
	#endif

	#ifndef ASSUME
	#define ASSUME(x) __assume(x)
	#endif

	#ifndef LIKELY
	#define LIKELY(x) (x)
	#endif

	#ifndef UNLIKELY
	#define UNLIKELY(x) (x)
	#endif

#elif defined( __GNUC__ )

	// GCC/G++
	#ifndef STD_CALL
	#define STD_CALL __attribute__ ((stdcall))
	#endif

	#ifndef FORCE_INLINE
	#define FORCE_INLINE __attribute__ ((always_inline))
	#endif

	#ifndef NO_INLINE
	#define NO_INLINE __attribute__ ((noinline))
	#endif

	#ifndef NO_RETURN
	#define NO_RETURN __attribute__ ((noreturn))
	#endif

	#ifndef NO_STRIP
	#define NO_STRIP __attribute__ ((used))
	#endif

	#ifndef MALLOC_LIKE
	#define MALLOC_LIKE __attribute__ ((malloc))
	#endif

	#ifndef DLL_IMPORT
	#define DLL_IMPORT __attribute__ ((dllimport))
	#endif

	#ifndef MAY_ALIAS
	#define MAY_ALIAS __attribute__ ((may_alias))
	#endif

	#ifndef UNREACHABLE
	#define UNREACHABLE __builtin_unreachable ()
	#endif

	#ifndef ASSUME
	#define ASSUME(x) if( !( x ) ) __builtin_unreachable()
	#endif

	#ifndef LIKELY
	#define LIKELY(x) __builtin_expect ((x), 1)
	#endif

	#ifndef UNLIKELY
	#define UNLIKELY(x) __builtin_expect ((x), 0)
	#endif

#elif defined( __clang__ )

	// CLANG
	#ifndef STD_CALL
	#define STD_CALL __attribute__ ((stdcall))
	#endif

	#ifndef FORCE_INLINE
	#define FORCE_INLINE __attribute__ ((always_inline))
	#endif

	#ifndef NO_INLINE
	#define NO_INLINE __attribute__ ((noinline))
	#endif

	#ifndef NO_RETURN
	#define NO_RETURN __attribute__ ((noreturn))
	#endif

	#ifndef NO_STRIP
	#define NO_STRIP __attribute__ ((used))
	#endif

	#ifndef MALLOC_LIKE
	#define MALLOC_LIKE __attribute__ ((malloc))
	#endif

	#ifndef DLL_IMPORT
	#define DLL_IMPORT __attribute__ ((dllimport))
	#endif

	#ifndef MAY_ALIAS
	#define MAY_ALIAS __attribute__ ((may_alias))
	#endif

	#ifndef UNREACHABLE
	#define UNREACHABLE __builtin_unreachable ()
	#endif

	#ifndef ASSUME
	#define ASSUME(x) __builtin_assume (x)
	#endif

	#ifndef LIKELY
	#define LIKELY(x) __builtin_expect ((x), 1)
	#endif

	#ifndef UNLIKELY
	#define UNLIKELY(x) __builtin_expect ((x), 0)
	#endif

#endif

	// All
	#ifndef UNUSED
	#define UNUSED [[maybe_unused]]
	#endif

	#ifndef NO_DISCARD
	#define NO_DISCARD [[nodiscard]]
	#endif

	#ifndef DEPRECATED
	#define DEPRECATED(str) [[deprecated(str)]]
	#endif