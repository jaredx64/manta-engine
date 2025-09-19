#include <manta/thread.hpp>

#include <core/debug.hpp>

#include <vendor/windows.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Thread::sleep( u32 milliseconds )
{
	Sleep( milliseconds );
}


ThreadID Thread::id()
{
	return ThreadID( GetCurrentThreadId() );
}


void *Thread::create( ThreadFunction function )
{
	// Create the thread
	void *handle = CreateThread( nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>( function ), nullptr, 0, nullptr );
	ErrorIf( handle == nullptr, "WIN: Failed to create thread!" );
	return handle;
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////