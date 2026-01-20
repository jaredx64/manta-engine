#include <manta/time.hpp>

#include <time.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

thread_local static timespec offset;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CoreTime::init()
{
	return clock_gettime( CLOCK_MONOTONIC, &offset ) == 0;
}


bool CoreTime::free()
{
	return true;
}


double Time::value()
{
	timespec current;
	clock_gettime( CLOCK_MONOTONIC, &current );
	return ( current.tv_sec - offset.tv_sec ) + ( current.tv_nsec - offset.tv_nsec ) / 1e+9;
}


u64 Time::seed()
{
	timespec current;
	clock_gettime( CLOCK_MONOTONIC, &current );
	return static_cast<u64>( current.tv_nsec ^ current.tv_sec );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////