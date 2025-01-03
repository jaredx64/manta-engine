#pragma once
#include <vendor/config.hpp>

#if USE_OFFICIAL_HEADERS
	#include <vendor/conflicts.hpp>
		#include <string.h>
	#include <vendor/conflicts.hpp>
#else
	#include <vendor/stddef.hpp>

	extern "C" void *memset(void *dest, int ch, size_t count);
	extern "C" void *memcpy(void *dest, const void *src, size_t count);
	extern "C" void *memmove(void *dest, const void *src, size_t count);
	extern "C" int memcmp(const void *lhs, const void *rhs, size_t count);
	extern "C" char *strcpy(char *dest, const char *src);
	extern "C" char *strncpy(char *dest, const char *src, size_t count);
	extern "C" int strcmp(const char *lhs, const char *rhs);
	extern "C" int strncmp(const char *lhs, const char *rhs, size_t count);
	extern "C" size_t strlen(const char *str);
	extern "C" char *strcat(char *destination, const char *source);
	extern "C" char *strncat(char *destination, const char *source, size_t count);
	extern "C" char *strstr(const char *str, const char *substr);
	extern "C" char *strchr(const char *str, int ch);
	extern "C" char *strrchr(const char *str, int ch);
	extern "C" void *memchr(const void *ptr, int ch, size_t count);
	extern "C" size_t strspn( const char *dest, const char *src );

	#if PIPELINE_COMPILER_MSVC
		// MSVC wants to know that the above declarations are intended
		// to be used as compiler intrinsics.
		#pragma intrinsic(memset)
		#pragma intrinsic(memcpy)
		#pragma intrinsic(memmove)
		#pragma intrinsic(memcmp)
		#pragma intrinsic(strcpy)
		#pragma intrinsic(strcmp)
		#pragma intrinsic(strlen)
		#pragma intrinsic(strcat)
	#endif
#endif