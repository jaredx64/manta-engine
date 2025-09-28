#include <manta/thread.hpp>

#include <vendor/pthread.hpp>
#include <vendor/posix.hpp>

#include <core/debug.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Thread::sleep( u32 milliseconds )
{
	usleep( milliseconds * 1000 );
}


ThreadID Thread::id()
{
	return ThreadID( pthread_self() );
}


void *Thread::create( ThreadFunction function )
{
	// Create the thread
	void *handle;
	int result = pthread_create( reinterpret_cast<pthread_t *>( &handle ), nullptr, function, nullptr );
	ErrorIf( result != 0, "POSIX: Failed to create thread!" );
	return handle;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void Mutex::init()
{
	pthread_mutex_init( &mutex, nullptr );
}


void Mutex::free()
{
	pthread_mutex_destroy( &mutex );
}


void Mutex::lock()
{
	//pthread_mutex_lock( &mutex );
}


void Mutex::unlock()
{
	//pthread_mutex_unlock( &mutex );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Condition::init()
{
	pthread_cond_init( &condition, nullptr );
}


void Condition::free()
{
	pthread_cond_destroy( &condition );
}


void Condition::sleep( Mutex &mutex )
{
	pthread_cond_wait( &condition, &mutex.mutex );
}


void Condition::wake()
{
	pthread_cond_signal( &condition );
}


void Condition::wake_all()
{
	pthread_cond_broadcast( &condition );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////