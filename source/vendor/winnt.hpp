#pragma once
#include <vendor/config.hpp>

#if !defined( _MSC_VER )
	static_assert( false, "winnt.h intrinsics only supported by MSVC compiler!" );
#endif

#if USE_OFFICIAL_HEADERS
	#include <vendor/conflicts.hpp>
		#include <winnt.h>
		#include <intrin.h>
	#include <vendor/conflicts.hpp>
#else

	// intrin.h
	#if !defined( _INTRIN_DEFINED )
		extern "C" short __cdecl _InterlockedCompareExchange16( short volatile *, short, short );
		#pragma intrinsic( _InterlockedCompareExchange16 )
		extern "C" long __cdecl _InterlockedCompareExchange( long volatile *, long, long );
		#pragma intrinsic( _InterlockedCompareExchange )
		extern "C" long long __cdecl _InterlockedCompareExchange64( long long volatile *, long long, long long );
		#pragma intrinsic( _InterlockedCompareExchange64 )

		extern "C" short __cdecl _InterlockedExchange16( short volatile *, short );
		#pragma intrinsic( _InterlockedExchange16 )
		extern "C" long __cdecl _InterlockedExchange( long volatile *, long );
		#pragma intrinsic( _InterlockedExchange )
		extern "C" long long __cdecl _InterlockedExchange64( long long volatile *, long long );
		#pragma intrinsic( _InterlockedExchange64 )

		extern "C" short __cdecl _InterlockedExchangeAdd16( short volatile *, short );
		#pragma intrinsic( _InterlockedExchangeAdd16 )
		extern "C" long __cdecl _InterlockedExchangeAdd( long volatile *, long );
		#pragma intrinsic( _InterlockedExchangeAdd )
		extern "C" long long __cdecl _InterlockedExchangeAdd64( long long volatile *, long long );
		#pragma intrinsic( _InterlockedExchangeAdd64 )

		extern "C" long __cdecl _InterlockedOr( long volatile *, long );
		#pragma intrinsic( _InterlockedOr )
		extern "C" long __cdecl _InterlockedAnd( long volatile *, long );
		#pragma intrinsic( _InterlockedAnd )
		extern "C" long __cdecl _InterlockedXor( long volatile *, long );
		#pragma intrinsic( _InterlockedXor )

		extern "C" long long __cdecl _InterlockedOr64( long long volatile *, long long );
		#pragma intrinsic( _InterlockedOr64 )
		extern "C" long long __cdecl _InterlockedAnd64( long long volatile *, long long );
		#pragma intrinsic( _InterlockedAnd64 )
		extern "C" long long __cdecl _InterlockedXor64( long long volatile *, long long );
		#pragma intrinsic( _InterlockedXor64 )
	#endif

	// winnt.h
	#define _MM_HINT_T0 1
	#define _MM_HINT_T1 2
	#define _MM_HINT_T2 3
	#define _MM_HINT_NTA 0

	extern "C" void __cdecl _mm_lfence( void );
	#pragma intrinsic( _mm_lfence )
	extern "C" void __cdecl _mm_mfence( void );
	#pragma intrinsic( _mm_mfence )
	extern "C" void __cdecl _mm_sfence( void );
	#pragma intrinsic( _mm_sfence )
	extern "C" void __cdecl _mm_pause( void );
	#pragma intrinsic( _mm_pause )
	extern "C" void __cdecl _mm_prefetch( char const *, int );
	#pragma intrinsic( _mm_prefetch )

#endif