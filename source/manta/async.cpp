#include <manta/async.hpp>

#include <core/list.hpp>

#include <manta/engine.hpp>
#include <manta/time.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AsyncQueue::init()
{
	events.init();
	return true;
}


bool AsyncQueue::free()
{
	events.free();
	return true;
}


bool AsyncQueue::update( Delta delta )
{
	if( events.count() == 0 ) { return true; }

	AsyncEvent &event = events.at( 0 );

	if( event.function != nullptr && !event.function( event ) ) { return false; }

	events.remove( 0 );
	return true;
}


void AsyncQueue::push_front( const AsyncEvent &event )
{
	events.insert( 0, event );
}


void AsyncQueue::push_back( const AsyncEvent &event )
{
	events.add( event );
}


void AsyncQueue::push( const AsyncEvent &event, bool skipQueue )
{
	if( skipQueue ) { push_front( event ); } else { push_back( event ); }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////