#pragma once

#include <core/types.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace SysTime
{
	extern bool init();
	extern bool free();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Time
{
	extern double value();
	extern u64 seed();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Timer
{
_PUBLIC:
	Timer() { start(); }

	void start() { timeStart = Time::value(); timing = true; }
	void stop() { timeEnd = Time::value(); timing = false; }
	bool running() const { return timing; }

	double s()
	{
		return ( Time::value() - timeStart );
	}

	double ms()
	{
		return ( Time::value() - timeStart ) * 1000.0;
	}

	double us()
	{
		return ( Time::value() - timeStart ) * 1000.0 * 1000.0;
	}

	double elapsed_s()
	{
		stop();
		return ( timeEnd - timeStart );
	}

	double elapsed_ms()
	{
		//stop();
		return ( timeEnd - timeStart ) * 1000.0;
	}

	double elapsed_us()
	{
		//stop();
		return ( timeEnd - timeStart ) * 1000.0 * 1000.0;
	}

_PRIVATE:
	double timeStart = 0.0;
	double timeEnd = 0.0;
	bool timing = false;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Frame
{
	extern u32 fps;
	extern u32 fpsLimit;
	extern Delta delta;
	extern double frameTimeMS;
	extern bool tickSecond;
	extern bool tickFrame;

	extern void start();
	extern void end();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////