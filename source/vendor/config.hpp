#pragma once

#include <config.hpp>
#include <pipeline.hpp>

#if PIPELINE_OS_MACOS// || PIPELINE_OS_LINUX
	// On macOS, we don't support unofficial headers (TODO: support them)
	#define USE_OFFICIAL_HEADERS ( 1 )
#else
	#ifdef OFFICIAL_HEADERS
		#define USE_OFFICIAL_HEADERS ( OFFICIAL_HEADERS )
	#else
		#define USE_OFFICIAL_HEADERS ( 1 )
	#endif
#endif
