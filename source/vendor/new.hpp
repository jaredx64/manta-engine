#pragma once
#include <vendor/config.hpp>


#if USE_OFFICIAL_HEADERS
	#include <vendor/conflicts.hpp>
		#include <new> // uh oh
	#include <vendor/conflicts.hpp>
#else
	#include <vendor/stddef.hpp>
	inline void *operator new( size_t size, void *where ) { return where; }
#endif