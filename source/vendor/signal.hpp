#pragma once
#include <vendor/config.hpp>

#if USE_OFFICIAL_HEADERS
	#include <vendor/conflicts.hpp>
		#include <signal.h>
	#include <vendor/conflicts.hpp>
#else

	// TODO: signal
	extern "C"
	{
		void (*signal(int s, void (*handler)(int)))(int);
	}

	#define SIGINT 2	// interrupt
	#define SIGILL 4	// illegal instruction - invalid function image
	#define SIGFPE 8	// floating point exception
	#define SIGSEGV 11	// segment violation
	#define SIGTERM 15	// Software termination signal from kill
	#define SIGBREAK 21 // Ctrl-Break sequence
	#define SIGABRT 22	// abnormal termination triggered by abort call

#endif