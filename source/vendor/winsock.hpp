#pragma once
#include <vendor/config.hpp>


#if USE_OFFICIAL_HEADERS
	#include <vendor/conflicts.hpp>
		#include <winsock2.h>
		#include <ws2tcpip.h>
	#include <vendor/conflicts.hpp>
#else
	// TODO?
#endif