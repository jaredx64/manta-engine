#pragma once

#include <core/types.hpp>

#include <vendor/initializer_list.hpp> // TODO: only used for choose(), so maybe get rid of it?

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Random
{
public:
	Random();
	Random( u64 seed );

	void seed( u64 seed );
	u32 base();

	int next_int( int min, int max );
	int next_int( int max );
	u64 next_u64( u64 min, u64 max );
	u64 next_u64( u64 max );
	u32 next_u32( u32 min, u32 max );
	u32 next_u32( u32 max );
	u16 next_u16( u16 min, u16 max );
	u16 next_u16( u16 max );
	u8 next_u8( u8 min, u8 max );
	u8 next_u8( u8 max );
	float next_float( float min, float max );
	float next_float( float max );
	double next_double( double min, double max );
	double next_double( double max );

	// Usage: random.choose( { value1, ..., valueN } );
	template <typename T> T choose( std::initializer_list<T> values )
	{
		return *( values.begin() + this->next_int( static_cast<int>( values.size() ) - 1 ) );
	}

private:
	u64 state = 0;
	u64 where = 0;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////