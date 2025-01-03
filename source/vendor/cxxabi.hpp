#pragma once
#include <vendor/config.hpp>

#if USE_OFFICIAL_HEADERS
	#include <vendor/conflicts.hpp>
		#include <cxxabi.h>
	#include <vendor/conflicts.hpp>
#else

	namespace abi
	{
		extern "C" char *__cxa_demangle( const char *, char *, size_t *, int * );
	}

#endif