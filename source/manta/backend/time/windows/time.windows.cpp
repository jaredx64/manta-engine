#include <manta/time.hpp>

#include <core/debug.hpp>

#include <vendor/windows.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

thread_local static LARGE_INTEGER offset;
thread_local static double frequency;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CoreTime::init()
{
	LARGE_INTEGER result;

	if( timeBeginPeriod( 1 ) != TIMERR_NOERROR ) { ErrorReturnMsg( false, "WIN: Failed to set timer resolution" ); }
	if( !QueryPerformanceCounter( &offset ) ) { ErrorReturnMsg( false, "WIN: Failed to get timer offset" ); }
	if( !QueryPerformanceFrequency( &result ) ) { ErrorReturnMsg( false, "WIN: Failed to get timer frequency" ); }

	frequency = static_cast<double>( result.QuadPart );

	return true;
}


bool CoreTime::free()
{
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////