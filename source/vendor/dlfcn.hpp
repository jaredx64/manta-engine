#pragma once
#include <vendor/config.hpp>

#if USE_OFFICIAL_HEADERS
	#include <vendor/conflicts.hpp>
		#include <dlfcn.h>
	#include <vendor/conflicts.hpp>
#else
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// dlfcn.h

	struct Dl_info
	{
		const char *dli_fname;
		void *dli_fbase;
		const char *dli_sname;
		void *dli_saddr;
	};

	extern "C" int dladdr( const void *, Dl_info * );
	extern "C" int dladdr1( const void *, Dl_info *, void **, int );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif