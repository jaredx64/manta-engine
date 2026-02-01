#pragma once
#include <vendor/config.hpp>

#if USE_OFFICIAL_HEADERS
	#include <vendor/conflicts.hpp>
		#include <signal.h>
	#include <vendor/conflicts.hpp>
#else

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// signal.h

	// TODO: signal
	extern "C"
	{
		void (*signal(int s, void (*handler)(int)))(int);
	}

	#define SIGINT 2
	#define SIGILL 4
	#define SIGFPE 8
	#define SIGSEGV 11
	#define SIGTERM 15
	#define SIGBREAK 21
	#define SIGABRT 22

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif