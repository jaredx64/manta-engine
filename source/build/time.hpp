#pragma once

#include <core/types.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Time
{
	extern bool init();
	extern double value();
	extern u64 seed();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Timer
{
	Timer() { start(); }

	double timeStart = 0.0;
	double timeEnd = 0.0;

	void start() { timeStart = Time::value(); }
	void stop() { timeEnd = Time::value(); }

	double elapsed_s()
	{
		stop();
		return ( timeEnd - timeStart );
	}

	double elapsed_ms()
	{
		stop();
		return ( timeEnd - timeStart ) * 1000.0;
	}

	double elapsed_us()
	{
		stop();
		return ( timeEnd - timeStart ) * 1000.0 * 1000.0;
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////