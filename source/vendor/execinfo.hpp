#pragma once
#include <vendor/config.hpp>
#include <pipeline.hpp>

#if USE_OFFICIAL_HEADERS
	#include <vendor/conflicts.hpp>
		#include <execinfo.h>
	#include <vendor/conflicts.hpp>
#else
	extern "C" int backtrace( void **buffer, int size );
	extern "C" char **backtrace_symbols( void *const *buffer, int size );
	extern "C" void backtrace_symbols_fd( void *const *buffer, int size, int fd );
#endif