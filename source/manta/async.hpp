#pragma once

#include <vendor/new.hpp>

#include <core/list.hpp>
#include <core/types.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class AsyncEvent
{
public:
	AsyncEvent( bool ( *function )( AsyncEvent & ) ) : function{ function }, payload{ 0 } { }

	AsyncEvent( const AsyncEvent &other ) : function{ other.function }
	{
		memory_copy( payload, other.payload, sizeof( payload ) );
	}

	template <typename State>
	AsyncEvent( bool ( *process )( AsyncEvent & ), const State &state ) : function{ process }
	{
		static_assert( sizeof( State ) <= sizeof( payload ), "State is too big!" );
		memory_copy( payload, &state, sizeof( State ) );
	}

	template <typename State>
	AsyncEvent( const AsyncEvent &other, const State &state ) : function{ other.function }
	{
		Assert( sizeof( State ) <= sizeof( payload ) );
		memory_copy( payload, &state, sizeof( State ) );
	}

	template <typename State>
	State &state()
	{
		static_assert( sizeof( State ) <= sizeof( payload ), "State is too big!" );
		return *reinterpret_cast<State *>( payload );
	}

private:
	friend class AsyncQueue;
	bool ( *const function )( AsyncEvent &event ) = nullptr;
	byte payload[4096] = { 0 };
};

#define ASYNC_EVENT_LAMBDA []( AsyncEvent &event ) -> bool
#define ASYNC_EVENT_FUNCTION( function ) bool function( AsyncEvent &event )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class AsyncQueue
{
public:
	bool init();
	bool free();
	bool update( const Delta delta );

	void push_front( const AsyncEvent &event );
	void push_back( const AsyncEvent &event );
	void push( const AsyncEvent &event, const bool skipQueue );

private:
	List<AsyncEvent> events;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////