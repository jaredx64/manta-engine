#pragma once
#include <vendor/config.hpp>
#include <pipeline.hpp>

#if USE_OFFICIAL_HEADERS
	#include <vendor/conflicts.hpp>
		#include <stdlib.h>
	#include <vendor/conflicts.hpp>
#else
	#include <vendor/vendor.hpp>
	#include <vendor/stddef.hpp>

	#if PIPELINE_OS_WINDOWS
		extern "C" int* __cdecl __p___argc(void);
		extern "C" char*** __cdecl __p___argv(void);
		extern "C" wchar_t*** __cdecl __p___wargv(void);

		#ifdef _CRT_DECLARE_GLOBAL_VARIABLES_DIRECTLY
			extern int __argc;
			extern char**__argv;
			extern wchar_t** __wargv;
		#else
			#define __argc (*__p___argc())
			#define __argv (*__p___argv())
			#define __wargv (*__p___wargv())
		#endif
	#endif

	extern "C" MALLOC_LIKE void *malloc(size_t);
	extern "C" void *realloc(void *, size_t);
	extern "C" void free(void *);
	extern "C" NO_RETURN void exit(int);
	extern "C" int system(const char *);
	extern "C" int atoi(const char *);
	extern "C" double atof(const char *);
	extern "C" long atol(const char *);
	extern "C" long long atoll(const char *);
	extern "C" long int strtol(const char *, char **, int);
	extern "C" unsigned long long int strtoull(const char *str, char **endptr, int base);
	extern "C" char * getenv(char const *);

	#if PIPELINE_OS_WINDOWS
		extern "C" char *_fullpath(const char *, char const *, size_t);
	#else
		extern "C" char *realpath(const char *, char *);
	#endif
#endif

#if PIPELINE_OS_WINDOWS
	#define realpath(path, outPath) _fullpath(outPath, path, sizeof(path))
#endif