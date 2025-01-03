#include <manta/time.hpp>

#include <core/debug.hpp>
#include <core/types.hpp>

#include <mach/mach_time.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static u64 offset;
static double frequency;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SysTime::init()
{
	mach_timebase_info_data_t info;
	if( mach_timebase_info( &info ) != KERN_SUCCESS ) { ErrorReturnMsg( false, "MACH: Failed get mach timer info" ); }

	frequency = info.denom * 1e+9 / static_cast<double>( info.numer );
	offset = mach_absolute_time();
	return true;
}


bool SysTime::free()
{
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////