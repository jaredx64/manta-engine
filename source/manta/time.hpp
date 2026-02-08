#pragma once

#include <core/types.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace CoreTime
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

	double s() const
	{
		return ( Time::value() - timeStart );
	}

	double ms() const
	{
		return ( Time::value() - timeStart ) * 1000.0;
	}

	double us() const
	{
		return ( Time::value() - timeStart ) * 1000.0 * 1000.0;
	}

	double elapsed_s()
	{
		return ( timeEnd - timeStart );
	}

	double elapsed_ms()
	{
		return ( timeEnd - timeStart ) * 1000.0;
	}

	double elapsed_us()
	{
		return ( timeEnd - timeStart ) * 1000.0 * 1000.0;
	}

private:
	double timeStart = 0.0;
	double timeEnd = 0.0;
	bool timing = false;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Ticker
{
public:
	void start();
	int tick( double intervalMS );

	template <typename T> int tick( double intervalMS, T &callback )
	{
		const int ticks = tick( intervalMS );
		for( int i = 0; i < ticks; i++ ) { callback(); }
		return ticks;
	}

private:
	double lastTimeS = 0.0;
	double accumulatorMS = 0.0;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T> class TimedInterpolator
{
public:
	void set( T start, T end, float timeMS )
	{
		vStart = start;
		vEnd = end;
		tStart = static_cast<float>( Time::value() );
		tDurationInv = 1000.0f / timeMS;
	}

	T value() const
	{
		const float progress = []( float a, float b ) { return a < b ? a : b; } (
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