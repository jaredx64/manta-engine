#pragma once

#ifndef _MSC_VER
	static_assert( false, "winnt.h intrinsics only supported by MSVC compiler!" );
#else

#include <vendor/config.hpp>

#if USE_OFFICIAL_HEADERS
	#include <vendor/conflicts.hpp>
		#include <winnt.h>
		#include <intrin.h>
	#include <vendor/conflicts.hpp>
#else

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

#endif

#endif