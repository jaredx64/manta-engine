#include <manta/random.hpp>
#include <manta/time.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Random::Random()
{
	this->seed( Time::seed() );
}


Random::Random( const u64 seed )
{
	this->seed( seed );
}


void Random::seed( const u64 seed )
{
	state = 0;
	where = ( seed << 1 ) | 1;
	base();
	state += seed;
	base();
}


u32 Random::base()
{
    // Advance State
	u64 oldstate = state;
    state = oldstate * 6364136223846793005 + where;

    // Calculate Output
    u32 xorshifted = static_cast<u32>( ( ( oldstate >> 18 ) ^ oldstate ) >> 27 );
    u32 rot = oldstate >> 59;
    return ( xorshifted >> rot ) | ( xorshifted << ( ( ~rot + 1 ) & 31 ) );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Random::next_int( const int min, const int max )
{
	u32 x = base();
	u64 m = static_cast<u64>( x ) * static_cast<u64>( max - min + 1 );
	return ( m >> 32 ) + min;
}


int Random::next_int( const int max )
{
	u32 x = base();
	u64 m = static_cast<u64>( x ) * static_cast<u64>( max + 1 );
	return ( m >> 32 );
}

u64 Random::next_u64( const u64 min, const u64 max )
{
	u32 x = base();
	u64 m = static_cast<u64>( x ) * static_cast<u64>( max - min + 1 );
	return m + min;
}


u64 Random::next_u64( const u64 max )
{
	u32 x = base();
	u64 m = static_cast<u64>( x ) * static_cast<u64>( max + 1 );
	return m;
}


u32 Random::next_u32( const u32 min, const u32 max )
{
	u32 x = base();
	u64 m = static_cast<u64>( x ) * static_cast<u64>( max - min + 1 );
	return ( m >> 32 ) + min;
}


u32 Random::next_u32( const u32 max )
{
	u32 x = base();
	u64 m = static_cast<u64>( x ) * static_cast<u64>( max + 1 );
	return ( m >> 32 );
}


u16 Random::next_u16( const u16 min, const u16 max )
{
	u32 x = base();
	u64 m = static_cast<u64>( x ) * static_cast<u64>( max - min + 1 );
	return static_cast<u16>( ( m >> 32 ) + min );
}


u16 Random::next_u16( const u16 max )
{
	u32 x = base();
	u64 m = static_cast<u64>( x ) * static_cast<u64>( max + 1 );
	return static_cast<u16>( m >> 32 );
}


u8 Random::next_u8( const u8 min, const u8 max )
{
	u32 x = base();
	u64 m = static_cast<u64>( x ) * static_cast<u64>( max - min + 1 );
	return static_cast<u8>( ( m >> 32 ) + min );
}


u8 Random::next_u8( const u8 max )
{
	u32 x = base();
	u64 m = static_cast<u64>( x ) * static_cast<u64>( max + 1 );
	return static_cast<u8>( m >> 32 );
}


float Random::next_float( const float min, const float max )
{
	return base() * 0x1.0p-32f * ( max - min ) + min;
}


float Random::next_float( const float max )
{
	return base() * 0x1.0p-32f * max;
}


double Random::next_double( const double min, const double max )
{
	return base() * 0x1.0p-32 * ( max - min ) + min;
}


double Random::next_double( const double max )
{
	return base() * 0x1.0p-32 * max;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////