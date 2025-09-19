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
public:
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

private:
	double timeStart = 0.0;
	double timeEnd = 0.0;
	bool timing = false;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T>
class TimedInterpolator
{
public:
	void set( const T start, const T end, const float timeMS )
	{
		vStart = start;
		vEnd = end;
		tStart = static_cast<float>( Time::value() );
		tDurationInv = 1000.0f / timeMS;
	}

	T value() const
	{
		const float progress = []( const float a, const float b ) { return a < b ? a : b; } (
			( static_cast<float>( Time::value() ) - tStart ) * tDurationInv, 1.0f );
		return vStart + ( vEnd - vStart ) * progress;
	}

private:
	T vStart, vEnd;
	float tStart;
	float tDurationInv = 0.0f;
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