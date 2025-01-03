#pragma once
#include <vendor/config.hpp>

#if USE_OFFICIAL_HEADERS
	#include <vendor/conflicts.hpp>
		#include <stddef.h>
	#include <vendor/conflicts.hpp>
#else
	#ifndef NULL
		#ifdef __cplusplus
			#define NULL 0
		#else
			#define NULL ((void *)0)
		#endif
	#endif

	#if PIPELINE_COMPILER_MSVC
		// On MSVC, size_t acts as a built-in type (usable without a header)
	#elif PIPELINE_COMPILER_CLANG || PIPELINE_COMPILER_GCC
		// However, on other compilers we need to define it
		#if PIPELINE_ARCHITECTURE_X64 || PIPELINE_ARCHITECTURE_ARM64
			#ifdef _WIN64
				using size_t = unsigned long long;
			#else
				using size_t = unsigned long;
			#endif
			static_assert( sizeof( size_t ) == 8, "size_t not 8 bytes!" );
		#else
			static_assert( false, "Unsupported architecture!" );
		#endif
	#endif

#endif