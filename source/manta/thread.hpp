#pragma once

#include <config.hpp>

#include <core/debug.hpp>
#include <core/types.hpp>
#include <core/memory.hpp>

#include <vendor/vendor.hpp>

#include <manta/time.hpp>

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

struct Thread_ID
{
#if THREAD_WINDOWS
	DWORD id;
	Thread_ID() { };
	Thread_ID( DWORD id ) : id { id } { }
	bool operator==( const Thread_ID &other ) const { return ( id == other.id ); }
	bool operator!=( const Thread_ID &other ) const { return ( id != other.id ); }
#elif THREAD_POSIX
	pthread_t id;
	Thread_ID() { };
	Thread_ID( const pthread_t id ) : id { id } { }
	bool operator==( const Thread_ID &other ) const { return ( id == other.id ); }
	bool operator!=( const Thread_ID &other ) const { return ( id != other.id ); }
#else
	Thread_ID() { };
	Thread_ID( const pthread_t id ) : id { id } { }
	bool operator==( const Thread_ID &other ) const { return true; }
	bool operator!=( const Thread_ID &other ) const { return true; }
#endif
};

extern Thread_ID THREAD_ID_MAIN;

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
	extern void pause();
	extern void yield();
	extern struct Thread_ID id();
	extern void *create( ThreadFunction function );
	extern void free( void *thread );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Thread
{
	template <typename F> bool wait_until( F condition, int timeoutMS )
	{
		Assert( timeoutMS > 0 );

		Timer timer;
		timer.start();

		constexpr int SPIN_ITERATIONS = 2000;
		constexpr int YIELD_ITERATIONS = 10000;
		int iteration = 0;

		while( !condition() )
		{
			if( iteration < SPIN_ITERATIONS )
			{
				pause(); // Tight spin for very short waits
			}
			else if( iteration < YIELD_ITERATIONS )
			{

				yield(); // Yield to scheduler
			}
			else
			{
				sleep( 0 ); // Sleep briefly to avoid pegging CPU
			}

			iteration++;

			timer.stop();
			if( timer.elapsed_ms() >= timeoutMS ) { return false; }
		}

		return true;
	}

	template <typename F> void wait_until( F condition )
	{
		constexpr int SPIN_ITERATIONS = 2000;
		constexpr int YIELD_ITERATIONS = 10000;
		int iteration = 0;

		while( !condition() )
		{
			if( iteration < SPIN_ITERATIONS )
			{

				pause(); // Tight spin for very short waits
			}
			else if( iteration < YIELD_ITERATIONS )
			{

				yield(); // Yield to scheduler
			}
			else
			{
				sleep( 0 ); // Sleep briefly to avoid pegging CPU
			}

			iteration++;
		}
	}
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

struct Atomic_U16
{
	void init( u16 value = 0 ) { v = value; }
	u16 load() const;
	void store( u16 value );
	u16 fetch_add( u16 value );
	u16 fetch_sub( u16 value );
	u16 fetch_or( u16 value );
	u16 fetch_and( u16 value );
	u16 fetch_xor( u16 value );
	u16 exchange( u16 value );
	bool compare_exchange_strong( u16 &expected, u16 desired );
	void increment() { fetch_add( 1 ); }
	void decrement() { fetch_sub( 1 ); }
	alignas( 2 ) volatile u16 v;
};


struct Atomic_U32
{
	void init( u32 value = 0 ) { v = value; }
	u32 load() const;
	void store( u32 value );
	u32 fetch_add( u32 value );
	u32 fetch_sub( u32 value );
	u32 fetch_or( u32 value );
	u32 fetch_and( u32 value );
	u32 fetch_xor( u32 value );
	u32 exchange( u32 value );
	bool compare_exchange_strong( u32 &expected, u32 desired );
	void increment() { fetch_add( 1 ); }
	void decrement() { fetch_sub( 1 ); }
	alignas( 4 ) volatile u32 v;
};


struct Atomic_U64
{
	void init( u64 value = 0 ) { v = value; }
	u64 load() const;
	void store( u64 value);
	u64 fetch_add( u64 value );
	u64 fetch_sub( u64 value );
	u64 fetch_or( u64 value );
	u64 fetch_and( u64 value );
	u64 fetch_xor( u64 value );
	u64 exchange( u64 value );
	bool compare_exchange_strong( u64 &expected, u64 desired );
	void increment() { fetch_add( 1LLU ); }
	void decrement() { fetch_sub( 1LLU ); }
#if defined( _MSC_VER )
	alignas( 8 ) volatile long long v;
#else
	alignas( 8 ) volatile u64 v;
#endif
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Semaphore
{
	Mutex mutex;
	Condition condition;
	int count;

	void init( int initialCount );
	void free();

	void wait();
	void post();
	void post_all();
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T> struct ConcurrentQueue
{
public:
	bool init( usize reserve = 1 )
	{
		Assert( reserve > 0 );
		capacity = reserve;
		count = 0;
		front = 0;
		back = 0;

		Assert( data == nullptr );
		data = reinterpret_cast<T *>( memory_alloc( capacity * sizeof( T ) ) );

		mutex.init();
		notEmpty.init();
		notFull.init();

		return true;
	}

	bool free()
	{
		if( data == nullptr ) { return true; }

		notEmpty.free();
		notFull.free();
		mutex.free();

		memory_free( data );
		data = nullptr;
		return true;
	}

	bool enqueue( const T &element )
	{
		mutex.lock();
		{
			while( count >= capacity ) { notFull.sleep( mutex ); }

			data[back] = element;
			back = ( back + 1 ) % capacity;
			count++;

			notEmpty.wake();
		}
		mutex.unlock();
		return true;
	}

	bool dequeue( T &outElement, bool blocking = false )
	{
		mutex.lock();
		{
			if( blocking )
			{
				while( count == 0 ) { notEmpty.sleep( mutex ); }
			}
			else if( count == 0 )
			{
				mutex.unlock();
				return false;
			}

			outElement = data[front];
			front = ( front + 1 ) % capacity;
			count--;

			notFull.wake();
		}
		mutex.unlock();
		return true;
	}

public:
	T *data = nullptr;
	usize capacity = 1;
	usize count = 0;
	usize front = 0;
	usize back = 0;

	Mutex mutex;
	Condition notEmpty;
	Condition notFull;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum_type( WriteMode, u32 )
{
	WriteMode_PERSISTENT = 0,
	WriteMode_OVERWRITE = 1,
	WriteMode_RING = 2,
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T> class ThreadedWriterSPSC
{
public:
	static constexpr int MAX_RESOURCE_COUNT = 16;
	static constexpr int MAX_COMMIT_TRACKING = 256;
	DEBUG( static constexpr int STALL_TIMEOUT_MS = 2000; )

public:
	struct alignas( 64 ) ResourceState
	{
		T *resource = nullptr;
		usize capacity = 0LLU;
		Atomic_U64 offset;
		Atomic_U32 isWritten;
		Atomic_U32 numCommitsInFlight;
	};

	struct CommitResult
	{
		T *resource = nullptr;
		usize offset = 0LLU;
		usize size = 0LLU;
		u64 commitID = 0LLU;
	};

	struct CommitTracking
	{
		Atomic_U64 commitID;
		Atomic_U32 resourceIndex;
		Atomic_U32 valid; // 1 if this slot is valid, 0 otherwise
	};

public:
	bool init( T *resources, int resourceCount, usize resourceCapacity, WriteMode writeMode,
		bool bufferedWrites = true )
	{
		constexpr usize s = sizeof( ResourceState );

		Assert( !initialized );
		Assert( resourceCount > 0 && resourceCount <= MAX_RESOURCE_COUNT );
		Assert( resourceCapacity > 0 );
		Assert( resources != nullptr );

		this->resourceCount = resourceCount;
		this->resourceCapacity = resourceCapacity;
		this->writeMode = writeMode;
		this->writeBuffered = bufferedWrites;
		this->resourceIndexCurrent = 0;
		this->resourceIndexNext = 0;

		for( int i = 0; i < resourceCount; i++ )
		{
			if( !resources[i].init( resourceCapacity ) )
			{
				Error( "ThreadedWriter: failed to initialize resource %d", i );
				return false;
			}

			resourceStates[i].resource = &resources[i];
			resourceStates[i].capacity = resourceCapacity;
			resourceStates[i].offset.init( 0LLU );
			resourceStates[i].isWritten.init( 0U );
			resourceStates[i].numCommitsInFlight.init( 0U );
		}

		for( int i = 0; i < MAX_COMMIT_TRACKING; i++ )
		{
			commitTracking[i].commitID.init( 0LLU );
			commitTracking[i].resourceIndex.init( 0U );
			commitTracking[i].valid.init( 0U );
		}

		if( bufferedWrites )
		{
			scratchBuffer = memory_alloc( resourceCapacity );
			Assert( scratchBuffer != nullptr );
		}

		globalCommitCounter.init( 0LLU );

		initialized = true;
		return true;
	}

	bool free( bool stall = true )
	{
		Assert( initialized );

	#if COMPILE_DEBUG
		const bool success = Thread::wait_until( [&]()
			{
				for( int i = 0; i < resourceCount; i++ )
				{
					if( resourceStates[i].numCommitsInFlight.load() > 0U ) { return false; }
				}
				return true;
			}, STALL_TIMEOUT_MS );
		ErrorIf( !success, "ThreadedWriter: free() timeout -- resources still in flight" );
	#else
		Thread::wait_until( [&]()
			{
				for( int i = 0; i < resourceCount; i++ )
				{
					if( resourceStates[i].numCommitsInFlight.load() > 0U ) { return false; }
				}
				return true;
			} );
	#endif

		if( scratchBuffer != nullptr )
		{
			memory_free( scratchBuffer );
			scratchBuffer = nullptr;
		}

		for( int i = 0; i < resourceCount; i++ )
		{
			const bool succeeded = resourceStates[i].resource->free();
			if( !succeeded ) { Warning( "ThreadedWriter: failed to free resource %d", i ); }
		}

		initialized = false;
		return true;
	}

	bool write_begin( bool stall = true )
	{
		const int resourceIndex = static_cast<int>( resourceIndexNext % resourceCount );

		if( writeMode == WriteMode_PERSISTENT )
		{
			// PERSISTENT: Ensure we only allow a single write_begin
			if( resourceStates[resourceIndex].isWritten.load() != 0U )
			{
				Error( "ThreadedWriter: PERSISTENT mode resource has already been written to!" );
				return false;
			}

			resourceIndexCurrent = resourceIndex;
		}
		else if( writeMode == WriteMode_OVERWRITE )
		{
			// OVERWRITE: Find an available resource
			resourceIndexCurrent = find_available_resource();

			if( resourceIndexCurrent < 0 )
			{
				if( !stall ) { return false; }

			#if COMPILE_DEBUG
				const bool success = Thread::wait_until( [&]()
					{
						resourceIndexCurrent = find_available_resource();
						return resourceIndexCurrent >= 0;
					}, STALL_TIMEOUT_MS );
				ErrorIf( !success, "ThreadedWriter: write_begin() timeout -- no available resources" );
			#else
				Thread::wait_until( [&]()
					{
						resourceIndexCurrent = find_available_resource();
						return resourceIndexCurrent >= 0;
					} );
			#endif
			}

			// OVERWRITE resets scratch buffer
			if( writeBuffered ) { scratchOffset = 0LLU; }
		}
		else if( writeMode == WriteMode_RING )
		{
			// RING: Continue on current resource from current offset
			resourceIndexCurrent = resourceIndex;
		}

		// Non-buffered mode: Start the write session immediately on the underlying resource
		if( !writeBuffered )
		{
			if( writeMode == WriteMode_OVERWRITE )
			{
				writeStartOffset = 0LLU;
				resourceStates[resourceIndexCurrent].offset.store( 0LLU );
			}
			else
			{
				writeStartOffset = resourceStates[resourceIndexCurrent].offset.load();
			}

			scratchOffset = 0LLU;
			resourceStates[resourceIndexCurrent].resource->write_begin();
		}

		return true;
	}

	bool write( const void *data, usize size, int alignment = 1 )
	{
		Assert( initialized );

		if( alignment > 1 )
		{
			const usize alignMask = static_cast<usize>( alignment - 1 );
			scratchOffset = ( scratchOffset + alignMask ) & ~alignMask;
		}

		if( scratchOffset + size > resourceCapacity )
		{
			Error( "ThreadedWriter: write exceeds capacity! (offset=%llu, size=%llu, capacity=%llu)",
				scratchOffset, size, resourceCapacity );
			return false;
		}

		if( writeBuffered )
		{
			byte *destination = static_cast<byte *>( scratchBuffer ) + scratchOffset;
			memory_copy( destination, data, size );
		}
		else
		{
			const usize offset = writeStartOffset + scratchOffset;
			resourceStates[resourceIndexCurrent].resource->write( data, size, offset );
		}

		scratchOffset += size;
		return true;
	}


	bool write_end()
	{
		Assert( initialized );

		if( writeBuffered )
		{
			// NOTE: In buffered mode, actual resource writes happen at commit()
		}
		else
		{
			resourceStates[resourceIndexCurrent].resource->write_end();
			resourceStates[resourceIndexCurrent].offset.store( writeMode == WriteMode_RING ?
				writeStartOffset + scratchOffset : 0LLU );
		}

		return true;
	}

	CommitResult commit( u64 commitID = U64_MAX, bool stall = true )
	{
		Assert( initialized );

		CommitResult result;
		const usize commitSize = scratchOffset;

		if( writeBuffered )
		{
			// Find a resource that can fit the accumulated writes
			int resourceIndexTarget = resourceIndexCurrent;
			usize writeOffset = 0LLU;

			if( writeMode == WriteMode_PERSISTENT || writeMode == WriteMode_OVERWRITE )
			{
				// For PERSISTENT/OVERWRITE, always write at offset 0
				writeOffset = 0LLU;
			}
			else if( writeMode == WriteMode_RING )
			{
				// For RING, try to fit in the current resource
				writeOffset = resourceStates[resourceIndexTarget].offset.load();
				if( writeOffset + commitSize > resourceStates[resourceIndexTarget].capacity )
				{
					int index = find_next_available_resource( resourceIndexTarget );

					if( index < 0 )
					{
						if( !stall )
						{
							return result;
						}

						// Stall until a resource becomes available
					#if COMPILE_DEBUG
						const bool success = Thread::wait_until( [&]()
							{
								index = find_next_available_resource(resourceIndexTarget);
								return index >= 0;
							}, STALL_TIMEOUT_MS );
						ErrorIf( !success, "ThreadedWriter: RING commit() timeout -- no available resources" );
					#else
						Thread::wait_until( [&]()
						{
							index = find_next_available_resource(resourceIndexTarget);
							return index >= 0;
						} );
					#endif
					}

					resourceIndexTarget = index;
					resourceIndexCurrent = index;
					resourceIndexNext = index;
					writeOffset = 0LLU;
				}
			}

			if( commitSize > 0LLU )
			{
				T *resource = resourceStates[resourceIndexTarget].resource;
				resource->write_begin();
				resource->write( scratchBuffer, commitSize, writeOffset );
				resource->write_end();
			}

			resourceStates[resourceIndexTarget].offset.store( writeMode == WriteMode_RING ?
				writeOffset + commitSize : 0LLU );
			resourceIndexCurrent = resourceIndexTarget;
		}

		resourceStates[resourceIndexCurrent].numCommitsInFlight.increment();

		if( commitID == U64_MAX ) { commitID = globalCommitCounter.fetch_add( 1LLU ); }
		track_commit( commitID, resourceIndexCurrent );
		resourceStates[resourceIndexCurrent].isWritten.store( 1U );

		usize commitOffset = 0LLU;
		if( writeBuffered )
		{
			commitOffset = writeMode == WriteMode_RING ?
				resourceStates[resourceIndexCurrent].offset.load() - commitSize : 0LLU;
		}
		else
		{
			commitOffset = writeStartOffset;
		}

		result.resource = resourceStates[resourceIndexCurrent].resource;
		result.offset = commitOffset;
		result.size = commitSize;
		result.commitID = commitID;

		if( writeMode == WriteMode_OVERWRITE ) { resourceIndexNext = ( resourceIndexCurrent + 1 ) % resourceCount; }

		if( writeBuffered ) { scratchOffset = 0LLU; }

		return result;
	}

	void release( u64 commitID )
	{
		const int index = find_commit_resource( commitID );
		if( index < 0 ) { return; }

		const u64 numCommitsInFlight = resourceStates[index].numCommitsInFlight.fetch_sub( 1U );
		if( numCommitsInFlight == 0U )
		{
			Error( "ThreadedWriter: numCommitsInFlight underflow for resource %d", index );
			return;
		}

		// Clear in-flight flag when no more commits are using this resource
		if( ( numCommitsInFlight - 1LLU ) == 0LLU )
		{
			if( writeMode != WriteMode_PERSISTENT )
			{
				resourceStates[index].offset.store( 0LLU );
				resourceStates[index].isWritten.store( 0U );
			}
		}

		untrack_commit( commitID );
	}

private:
	void track_commit( u64 commitID, int resourceIndex )
	{
		const int slot = static_cast<int>( commitID % MAX_COMMIT_TRACKING );

		for( int i = 0; i < MAX_COMMIT_TRACKING; i++ )
		{
			const int idx = ( slot + i ) % MAX_COMMIT_TRACKING;

			u32 expected = 0U;
			if( commitTracking[idx].valid.load() == 0U )
			{
				commitTracking[idx].commitID.store( commitID );
				commitTracking[idx].resourceIndex.store( static_cast<u32>( resourceIndex ) );
				commitTracking[idx].valid.store( 1U );
				return;
			}
		}

		AssertMsg( false, "ThreadedWriter: Commit tracking full! Increase MAX_COMMIT_TRACKING (currently %d)",
			MAX_COMMIT_TRACKING );
	}

	int find_commit_resource( u64 commitID )
	{
		const int slot = static_cast<int>( commitID % MAX_COMMIT_TRACKING );

		for( int i = 0; i < MAX_COMMIT_TRACKING; i++ )
		{
			const int idx = ( slot + i ) % MAX_COMMIT_TRACKING;

			if( commitTracking[idx].valid.load() == 1U &&
				commitTracking[idx].commitID.load() == commitID )
			{
				return static_cast<int>( commitTracking[idx].resourceIndex.load() );
			}
		}

		return -1;
	}

	void untrack_commit( u64 commitID )
	{
		const int slot = static_cast<int>( commitID % MAX_COMMIT_TRACKING );

		for( int i = 0; i < MAX_COMMIT_TRACKING; i++ )
		{
			const int idx = ( slot + i ) % MAX_COMMIT_TRACKING;

			if( commitTracking[idx].valid.load() == 1U &&
				commitTracking[idx].commitID.load() == commitID )
			{
				commitTracking[idx].valid.store( 0U );
				return;
			}
		}
	}

	int find_available_resource()
	{
		// Try to find a resource that is not in-flight
		for( int i = 0; i < resourceCount; i++ )
		{
			const int index = ( resourceIndexNext + i ) % resourceCount;
			if( resourceStates[index].numCommitsInFlight.load() == 0U ) { return index; }
		}

		return -1;
	}

	int find_next_available_resource( int currentIndex )
	{
		for( int i = 1; i < resourceCount; i++ )
		{
			const int index = ( currentIndex + i ) % resourceCount;
			if( resourceStates[index].numCommitsInFlight.load() == 0U ) { return index; }
		}
		return -1;
	}

private:
	ResourceState resourceStates[MAX_RESOURCE_COUNT];
	CommitTracking commitTracking[MAX_COMMIT_TRACKING];

	usize resourceCapacity = 0LLU;
	int resourceCount = 0;
	int resourceIndexCurrent = 0;
	int resourceIndexNext = 0;

	void *scratchBuffer = nullptr;
	usize scratchOffset = 0LLU;

	usize writeStartOffset = 0LLU; // Non-buffered mode only
	WriteMode writeMode = WriteMode_PERSISTENT;
	bool writeBuffered = true;

	Atomic_U64 globalCommitCounter;

	bool initialized = false;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////