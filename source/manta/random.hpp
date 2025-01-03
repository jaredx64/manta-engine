#pragma once

#include <core/types.hpp>

#include <vendor/initializer_list.hpp> // TODO: only used for choose(), so maybe get rid of it?

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct RandomContext
{
	RandomContext() { } // default
	RandomContext( const u64 seed ) { this->seed( seed ); }

	void seed( const u64 seed );
	u32 base();

	u64 state = 0;
	u64 where = 0;

	// Usage: randomContext.value<int>(...), randomContext.value<double>(...), etc
	template <typename T> T random( const T min, const T max );
	template <typename T> T random( const T max );

	// Usage: randomContext.choose_value( { value1, ..., valueN } );
	template <typename T> T choose_value( std::initializer_list<T> values )
	{
		return *( values.begin() + this->random<int>( static_cast<int>( values.size() ) - 1 ) );
	}
};

template <> int RandomContext::random<int>( const int min, const int max );
template <> int RandomContext::random<int>( const int max );

template <> u32 RandomContext::random<u32>( const u32 min, const u32 max );
template <> u32 RandomContext::random<u32>( const u32 max );

template <> float RandomContext::random<float>( const float min, const float max );
template <> float RandomContext::random<float>( const float max );

template <> double RandomContext::random<double>( const double min, const double max );
template <> double RandomContext::random<double>( const double max );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace SysRandom
{
	// thread_local ensures calls to Random:: do not conflict states between threads
	extern thread_local RandomContext context;
}


namespace Random
{
	inline void seed( const u64 seed ) { SysRandom::context.seed( seed ); }
	inline u32 base() { return SysRandom::context.base(); }
	inline RandomContext &context() { return SysRandom::context; }
}


template <typename T> T random( T min, T max );
template <typename T> T random( T max );


template <> inline int random<int>( const int min, const int max )
{
	return SysRandom::context.random<int>( min, max );
}


template <> inline int random<int>( const int max )
{
	return SysRandom::context.random<int>( max );
}


template <> inline u32 random<u32>( const u32 min, const u32 max )
{
	return SysRandom::context.random<u32>( min, max );
}


template <> inline u32 random<u32>( const u32 max )
{
	return SysRandom::context.random<u32>( max );
}


template <> inline float random<float>( const float min, const float max )
{
	return SysRandom::context.random<float>( min, max );
}


template <> inline float random<float>( const float max )
{
	return SysRandom::context.random<float>( max );
}


template <> inline double random<double>( const double min, const double max )
{
	return SysRandom::context.random<double>( min, max );
}


template <> inline double random<double>( const double max )
{
	return SysRandom::context.random<double>( max );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define choose(...) SysRandom::context.choose_value( { __VA_ARGS__ } )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////