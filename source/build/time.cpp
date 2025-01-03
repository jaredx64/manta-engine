#include <build/time.hpp>

#include <config.hpp>

#include <core/types.hpp>
#include <core/debug.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if PIPELINE_OS_WINDOWS

	#include <vendor/windows.hpp>

	thread_local static LARGE_INTEGER offset;
	thread_local static double frequency;

	bool Time::init()
	{
		LARGE_INTEGER result;

		// Set Timer Resolution
		if( timeBeginPeriod( 1 ) != TIMERR_NOERROR ) { ErrorReturnMsg( false, "WIN: Failed to set timer resolution" ); }

		// Get Timer Offset
		if( !QueryPerformanceCounter( &offset ) ) { ErrorReturnMsg( false, "WIN: Failed to get timer offset" ); }

		// Get Timer Frequency
		if( !QueryPerformanceFrequency( &result ) ) { ErrorReturnMsg( false, "WIN: Failed to get timer frequency" ); }

		frequency = static_cast<double>( result.QuadPart );

		// Success
		return true;
	}


	double Time::value()
	{
		LARGE_INTEGER current;
		QueryPerformanceCounter( &current );
		return ( current.QuadPart - offset.QuadPart ) / frequency;
	}


	u64 Time::seed()
	{
		LARGE_INTEGER current;
		QueryPerformanceCounter( &current );
		return static_cast<u64>( current.LowPart );
	}

#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if PIPELINE_OS_MACOS

	#include <mach/mach_time.h>

	static u64 offset;
	static double frequency;

	bool Time::init()
	{
		mach_timebase_info_data_t info;
		if( mach_timebase_info( &info ) != KERN_SUCCESS ) { ErrorReturnMsg( false, "MACH: Failed get mach timer info" ); }

		frequency = info.denom * 1e+9 / static_cast<double>( info.numer );
		offset = mach_absolute_time();
		return true;
	}


	double Time::value()
	{
		return ( mach_absolute_time() - offset ) / frequency;
	}


	u64 Time::seed()
	{
		return static_cast<u64>( mach_absolute_time() );
	}

#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if PIPELINE_OS_LINUX

	#include <time.h>

	thread_local static timespec offset;

	bool Time::init()
	{
		// Success
		return clock_gettime( CLOCK_MONOTONIC, &offset ) == 0;
	}


	double Time::value()
	{
		timespec current;

		// Get Absolute Offset
		clock_gettime( CLOCK_MONOTONIC, &current );

		// Get Relative Offset
		return ( current.tv_sec - offset.tv_sec ) + ( current.tv_nsec - offset.tv_nsec ) / 1e+9;
	}


	u64 Time::seed()
	{
		return static_cast<u64>( offset.tv_nsec );
	}

#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////