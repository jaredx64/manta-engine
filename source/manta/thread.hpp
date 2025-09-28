#pragma once

#include <config.hpp>

#include <core/types.hpp>
#include <core/memory.hpp>

#include <vendor/vendor.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if THREAD_WINDOWS
	// Windows
	#define THREAD_FUNCTION( name ) unsigned int STD_CALL name( void * )
	using ThreadFunction = unsigned int (STD_CALL *)( void * );
	#include <vendor/windows.hpp>
#elif THREAD_POSIX
	// POSIX
	#define THREAD_FUNCTION( name ) void * name( void * )
	using ThreadFunction = void *(*)( void * );
	#include <vendor/pthread.hpp>
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct ThreadID
{
#if THREAD_WINDOWS
	DWORD id;
	ThreadID() { };
	ThreadID( const DWORD id ) { this->id = id; }
	bool operator==( const ThreadID &other ) const { return ( id == other.id ); }
	bool operator!=( const ThreadID &other ) const { return ( id != other.id ); }
#elif THREAD_POSIX
	pthread_t id;
	ThreadID() { };
	ThreadID( const pthread_t id ) { this->id = id; }
	bool operator==( const ThreadID &other ) const { return ( id == other.id ); }
	bool operator!=( const ThreadID &other ) const { return ( id != other.id ); }
#else
	ThreadID() { };
	ThreadID( const pthread_t id ) { this->id = id; }
	bool operator==( const ThreadID &other ) const { return true; }
	bool operator!=( const ThreadID &other ) const { return true; }
#endif
};

extern ThreadID THREAD_ID_MAIN;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace CoreThread
{
	extern bool init();
	extern bool free();
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Thread
{
	extern void sleep( u32 milliseconds );
	extern struct ThreadID id();
	extern void *create( ThreadFunction function );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Mutex
{
#if THREAD_WINDOWS
	CRITICAL_SECTION mutex;
#elif THREAD_POSIX
	pthread_mutex_t mutex;
#endif

	void init();
	void free();
	void lock();
	void unlock();
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Condition
{
#if THREAD_WINDOWS
	CONDITION_VARIABLE condition;
#elif THREAD_POSIX
	pthread_cond_t condition;
#endif

	void init();
	void free();
	void sleep( Mutex &mutex );
	void wake();
	void wake_all();
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T> struct ConcurrentQueue
{
    T *data = nullptr;
    usize capacity = 1;
    usize count = 0;
    usize front = 0;
    usize back = 0;

    Mutex mutex;
    Condition empty;

	bool init( const int reserve );
	bool free();
	bool clear();
	bool enqueue( const T &element );

	bool dequeue( T &outElement );
	bool dequeue_lock( T &outElement );
	bool dequeue_unlock();
};


template <typename T> bool ConcurrentQueue<T>::init( const int reserve )
{
	capacity = reserve;
	count = 0;
	front = 0;
	back = -1;

	Assert( data == nullptr );
	data = reinterpret_cast<T *>( memory_alloc( capacity * sizeof( T ) ) );

	mutex.init();
	empty.init();

	return true;
}


template <typename T> bool ConcurrentQueue<T>::free()
{
	if( data == nullptr ) { return true; }

	empty.free();
	mutex.free();

	memory_free( data );
	data = nullptr;
	return true;
}


template <typename T> bool ConcurrentQueue<T>::clear()
{
	Assert( data != nullptr );

	mutex.lock();
    {
        count = 0;
        front = 0;
        back = -1;
    }
	mutex.unlock();

	return true;
}


template <typename T> bool ConcurrentQueue<T>::enqueue( const T &element )
{
	Assert( data != nullptr );

	mutex.lock();
	{
		// If the queue is full, wait until a spot opens up (maybe we should just bail?)
		while( count >= capacity )
		{
			empty.sleep( mutex );
		}

		back = ( back + 1 ) % capacity;
		data[back] = element;
		count++;

		empty.wake();
	}
	mutex.unlock();

	return true;
}


template <typename T> bool ConcurrentQueue<T>::dequeue( T &outElement )
{
	Assert( data != nullptr );

	mutex.lock();
	{
		// If the queue is empty, abort
		if( count <= 0 )
		{
			mutex.unlock();
			return false;
		}

		outElement = data[front];
		front = ( front + 1 ) % capacity;
		count--;

		empty.wake();
	}
	mutex.unlock();

	return true;
}


template <typename T> bool ConcurrentQueue<T>::dequeue_lock( T &outElement )
{
	Assert( data != nullptr );

	mutex.lock();

	// If the queue is empty, abort
	if( count <= 0 )
	{
		mutex.unlock();
		return false;
	}

	outElement = data[front];
	front = ( front + 1 ) % capacity;
	count--;

	return true;
}


template <typename T> bool ConcurrentQueue<T>::dequeue_unlock()
{
	Assert( data != nullptr );

	empty.wake();
	mutex.unlock();

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////