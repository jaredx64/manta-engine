#include <manta/thread.hpp>

#include <vendor/pthread.hpp>
#include <vendor/posix.hpp>

#include <core/debug.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Thread::sleep( u32 milliseconds )
{
	usleep( milliseconds * 1000 );
}


void Thread::yield()
{
	sched_yield();
}


void Thread::pause()
{
#if defined( __x86_64__ ) || defined( __i386__ )
	__builtin_ia32_pause();
#elif defined( __aarch64__ ) || defined( __arm__ )
	__asm__ __volatile__( "yield" ::: "memory" );
#else
	__asm__ __volatile__( "" ::: "memory" );
#endif
}


Thread_ID Thread::id()
{
	return Thread_ID { pthread_self() };
}


void *Thread::create( ThreadFunction function )
{
	pthread_t *thread = reinterpret_cast<pthread_t *>( memory_alloc( sizeof( pthread_t ) ) );
	int result = pthread_create( thread, nullptr, function, nullptr );
	if( result != 0 )
	{
		memory_free( thread );
		ErrorReturnMsg( nullptr, "POSIX: Failed to create thread!" );
	}
	return reinterpret_cast<void *>( thread );
}


void Thread::free( void *thread )
{
	if( thread == nullptr ) { return; }
	memory_free( thread );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Mutex::init()
{
	pthread_mutexattr_t attr;
	pthread_mutexattr_init( &attr ) ;
	pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_NORMAL );
	int result = pthread_mutex_init( &mutex, &attr );
	pthread_mutexattr_destroy( &attr );
	ErrorIf( result != 0, "pthread_mutex_init failed! %d", result );
}


void Mutex::free()
{
	pthread_mutex_destroy( &mutex );
}


void Mutex::lock()
{
	pthread_mutex_lock( &mutex );
}


void Mutex::unlock()
{
	pthread_mutex_unlock( &mutex );
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