#pragma once

#include <core/types.hpp>

#include <vendor/initializer_list.hpp> // TODO: only used for choose(), so maybe get rid of it?

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Random
{
public:
	Random();
	Random( const u64 seed );

	void seed( const u64 seed );
	u32 base();

	int next_int( const int min, const int max );
	int next_int( const int max );
	u64 next_u64( const u64 min, const u64 max );
	u64 next_u64( const u64 max );
	u32 next_u32( const u32 min, const u32 max );
	u32 next_u32( const u32 max );
	u16 next_u16( const u16 min, const u16 max );
	u16 next_u16( const u16 max );
	u8 next_u8( const u8 min, const u8 max );
	u8 next_u8( const u8 max );
	float next_float( const float min, const float max );
	float next_float( const float max );
	double next_double( const double min, const double max );
	double next_double( const double max );

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