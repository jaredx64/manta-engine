#include <manta/thread.hpp>

#include <core/debug.hpp>

#include <vendor/windows.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if defined( _MSC_VER ) && ( defined( _M_X64 ) || defined( _M_IX86 ) )
	extern "C" void __cdecl _mm_pause( void );
#endif
#if defined( _MSC_VER ) && ( defined( _M_ARM64 ) || defined( _M_ARM ) )
	extern "C" void __yield(void);
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Thread::sleep( const u32 milliseconds )
{
	Sleep( milliseconds );
}


void Thread::yield()
{
	SwitchToThread();
}


void Thread::pause()
{
#if defined( _M_X64 ) || defined( _M_IX86 ) || defined( __i386__ ) || defined( __x86_64__ )
	#if defined( _MSC_VER )
		_mm_pause();
	#else
		__builtin_ia32_pause();
	#endif
#elif defined( _M_ARM64 ) || defined( _M_ARM ) || defined( __aarch64__ ) || defined( __arm__ )
	#if defined( _MSC_VER )
		__yield();
	#else
		__asm__ __volatile__( "yield" ::: "memory" );
	#endif
#else
	#if defined( _MSC_VER )
		_ReadWriteBarrier();
	#else
		__asm__ __volatile__( "" ::: "memory" );
	#endif
#endif
}


Thread_ID Thread::id()
{
	return Thread_ID( GetCurrentThreadId() );
}


void *Thread::create( ThreadFunction function )
{
	// Create the thread
	void *handle = CreateThread( nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>( function ), nullptr, 0, nullptr );
	ErrorIf( handle == nullptr, "WIN: Failed to create thread!" );
	return handle;
}


void Thread::free( void *thread )
{
	if( thread == nullptr ) { return; }
	memory_free( thread );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Mutex::init()
{
	InitializeCriticalSection( &mutex );
}


void Mutex::free()
{
	DeleteCriticalSection( &mutex );
}


void Mutex::lock()
{
	EnterCriticalSection( &mutex );
}


void Mutex::unlock()
{
	LeaveCriticalSection( &mutex );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Condition::init()
{
	InitializeConditionVariable( &condition );
}


void Condition::free()
{
	// Do nothing
}


void Condition::sleep( Mutex &mutex )
{
	SleepConditionVariableCS( &condition, &mutex.mutex, 0xFFFFFFFF );
}


void Condition::wake()
{
	WakeConditionVariable( &condition );
}


void Condition::wake_all()
{
	WakeAllConditionVariable( &condition );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////