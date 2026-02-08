#pragma once

#include <config.hpp>
#include <pipeline.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef USE_CUSTOM_C_HEADERS
	#if PIPELINE_OS_MACOS
		// TODO: We don't support custom C headers on MacOS yet
		#define USE_CUSTOM_C_HEADERS ( 0 )
	#else
		#ifdef CUSTOM_C_HEADERS
			#define USE_CUSTOM_C_HEADERS ( CUSTOM_C_HEADERS )
		#else
			#define USE_CUSTOM_C_HEADERS ( 0 )
		#endif
	#endif
#elif
	// TODO: We don't support custom C headers on MacOS yet
	#if PIPELINE_OS_MACOS
		#undef USE_CUSTOM_C_HEADERS
		#define USE_CUSTOM_C_HEADERS ( 0 )
	#endif
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////