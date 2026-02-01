#pragma once
#include <vendor/config.hpp>
#include <pipeline.hpp>

#if USE_OFFICIAL_HEADERS
	#include <vendor/conflicts.hpp>
		#include <execinfo.h>
	#include <vendor/conflicts.hpp>
#else
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// execinfo.h

	extern "C" int backtrace( void **, int );
	extern "C" char **backtrace_symbols( void *const *, int );
	extern "C" void backtrace_symbols_fd( void *const *, int, int );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif