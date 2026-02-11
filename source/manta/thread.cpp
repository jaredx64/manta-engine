#include <manta/thread.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Thread_ID THREAD_ID_MAIN;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CoreThread::init()
{
	THREAD_ID_MAIN = Thread::id();
	return true;
}


bool CoreThread::free()
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Semaphore::init( int initialCount )
{
	count = initialCount;
	mutex.init();
	condition.init();
}

void Semaphore::free()
{
	condition.free();
	mutex.free();
}

void Semaphore::wait()
{
	mutex.lock();
	while( count <= 0 ) { condition.sleep( mutex ); }
	--count;
	mutex.unlock();
}

void Semaphore::post()
{
	mutex.lock();
	++count;
	condition.wake();
	mutex.unlock();
}

void Semaphore::post_all()
{
	mutex.lock();
	++count;
	condition.wake_all();
	mutex.unlock();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if defined( _MSC_VER )
	static_assert( PIPELINE_OS_WINDOWS, "MSVC detected but not on Windows!" );
	#define ATOMIC_MSVC
	#include <vendor/intrin.hpp>
#elif defined( __GNUC__ ) || defined( __clang__ )
	#define ATOMIC_GCC_CLANG
#else
	static_assert( false, "Unsupported compiler!" );
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

u16 Atomic_U16::load() const
{
#if defined( ATOMIC_MSVC )
	return v; // 16-bit loads are atomic on all modern x86/x64
#elif defined( ATOMIC_GCC_CLANG )
	return __atomic_load_n( &v, __ATOMIC_ACQUIRE );
#endif
}


void Atomic_U16::store( u16 value )
{
#if defined( ATOMIC_MSVC )
	v = value;
#elif defined( ATOMIC_GCC_CLANG )
	__atomic_store_n( &v, value, __ATOMIC_RELEASE );
#endif
}


u16 Atomic_U16::fetch_add( u16 value )
{
#if defined( ATOMIC_MSVC )
	return _InterlockedExchangeAdd16( reinterpret_cast<volatile short*>( &v ), static_cast<short>( value ) );
#elif defined( ATOMIC_GCC_CLANG )
	return __atomic_fetch_add(&v, value, __ATOMIC_ACQ_REL);
#endif
}

u16 Atomic_U16::fetch_sub( u16 value )
{
#if defined( ATOMIC_MSVC )
	return _InterlockedExchangeAdd16( reinterpret_cast<volatile short*>( &v ), -static_cast<short>( value ) );
#elif defined( ATOMIC_GCC_CLANG )
	return __atomic_fetch_sub( &v, value, __ATOMIC_ACQ_REL );
#endif
}


u16 Atomic_U16::fetch_or( u16 value)
{
#if defined( ATOMIC_MSVC )
	short o;
	do { o = v; } while( _InterlockedCompareExchange16( reinterpret_cast<volatile short*>( &v ), o | value, o ) != o );
	return o;
#elif defined( ATOMIC_GCC_CLANG )
	return __atomic_fetch_or( &v, value, __ATOMIC_ACQ_REL );
#endif
}


u16 Atomic_U16::fetch_and( u16 value )
{
#if defined( ATOMIC_MSVC )
	short o;
	do { o = v; } while ( _InterlockedCompareExchange16( reinterpret_cast<volatile short*>( &v ), o & value, o ) != o );
	return o;
#elif defined( ATOMIC_GCC_CLANG )
	return __atomic_fetch_and(&v, value, __ATOMIC_ACQ_REL);
#endif
}


u16 Atomic_U16::fetch_xor( u16 value )
{
#if defined( ATOMIC_MSVC )
	short o;
	do { o = v; } while ( _InterlockedCompareExchange16( reinterpret_cast<volatile short*>( &v ), o ^ value, o ) != o );
	return o;
#elif defined( ATOMIC_GCC_CLANG )
	return __atomic_fetch_xor(&v, value, __ATOMIC_ACQ_REL);
#endif
}


u16 Atomic_U16::exchange( u16 value )
{
#if defined( ATOMIC_MSVC )
	return _InterlockedExchange16( reinterpret_cast<volatile short*>( &v ), static_cast<short>( value ) );
#elif defined( ATOMIC_GCC_CLANG )
	return __atomic_exchange_n( &v, value, __ATOMIC_ACQ_REL );
#endif
}


bool Atomic_U16::compare_exchange_strong( u16 &expected, u16 desired )
{
#if defined( ATOMIC_MSVC )
	u16 e = expected;
	u16 o = _InterlockedCompareExchange16( reinterpret_cast<volatile short *>( &v ), desired, e );
	if( o == e ) { return true; }
	expected = o;
	return false;
#elif defined( ATOMIC_GCC_CLANG )
	u16 e = expected;
	bool result = __atomic_compare_exchange_n( &v, &e, desired, false, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE );
	expected = e;
	return result;
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

u32 Atomic_U32::load() const
{
#if defined( ATOMIC_MSVC )
	return v;
#elif defined( ATOMIC_GCC_CLANG )
	return __atomic_load_n( &v, __ATOMIC_ACQUIRE );
#endif
}

void Atomic_U32::store( u32 value )
{
#if defined( ATOMIC_MSVC )
	v = value;
#elif defined( ATOMIC_GCC_CLANG )
	__atomic_store_n( &v, value, __ATOMIC_RELEASE );
#endif
}


u32 Atomic_U32::fetch_add( u32 value )
{
#if defined( ATOMIC_MSVC )
	return _InterlockedExchangeAdd( reinterpret_cast<volatile long*>( &v ), static_cast<long>( value ) );
#elif defined( ATOMIC_GCC_CLANG )
	return __atomic_fetch_add( &v, value, __ATOMIC_ACQ_REL );
#endif
}


u32 Atomic_U32::fetch_sub( u32 value )
{
#if defined( ATOMIC_MSVC )
	return _InterlockedExchangeAdd( reinterpret_cast<volatile long*>( &v ), -static_cast<long>( value ) );
#elif defined( ATOMIC_GCC_CLANG )
	return __atomic_fetch_sub( &v, value, __ATOMIC_ACQ_REL );
#endif
}


u32 Atomic_U32::fetch_or( u32 value )
{
#if defined( ATOMIC_MSVC )
	return _InterlockedOr( reinterpret_cast<volatile long*>( &v ), static_cast<long>( value ) );
#elif defined( ATOMIC_GCC_CLANG )
	return __atomic_fetch_or( &v, value, __ATOMIC_ACQ_REL );
#endif
}


u32 Atomic_U32::fetch_and( u32 value )
{
#if defined( ATOMIC_MSVC )
	return _InterlockedAnd( reinterpret_cast<volatile long*>( &v ), static_cast<long>( value ) );
#elif defined( ATOMIC_GCC_CLANG )
	return __atomic_fetch_and( &v, value, __ATOMIC_ACQ_REL );
#endif
}


u32 Atomic_U32::fetch_xor( u32 value )
{
#if defined( ATOMIC_MSVC )
	return _InterlockedXor( reinterpret_cast<volatile long*>( &v ), static_cast<long>( value ) );
#elif defined( ATOMIC_GCC_CLANG )
	return __atomic_fetch_xor( &v, value, __ATOMIC_ACQ_REL );
#endif
}


u32 Atomic_U32::exchange( u32 value )
{
#if defined( ATOMIC_MSVC )
	return _InterlockedExchange( reinterpret_cast<volatile long*>( &v ), static_cast<long>( value ) );
#elif defined( ATOMIC_GCC_CLANG )
	return __atomic_exchange_n( &v, value, __ATOMIC_ACQ_REL );
#endif
}


bool Atomic_U32::compare_exchange_strong( u32 &expected, u32 desired )
{
#if defined( ATOMIC_MSVC )
	u32 e = expected;
	u32 o = _InterlockedCompareExchange( reinterpret_cast<volatile long*>( &v ), desired, e );
	if( o == e ) { return true; }
	expected = o;
	return false;
#elif defined( ATOMIC_GCC_CLANG )
	u32 e = expected;
	bool result = __atomic_compare_exchange_n( &v, &e, desired, false, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE );
	expected = e;
	return result;
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

u64 Atomic_U64::load() const
{
#if defined( ATOMIC_MSVC )
	return v;
#elif defined( ATOMIC_GCC_CLANG )
	return __atomic_load_n( &v, __ATOMIC_ACQUIRE );
#endif
}


void Atomic_U64::store( u64 value )
{
#if defined( ATOMIC_MSVC )
	_InterlockedExchange64( reinterpret_cast<volatile long long*>( &v ), static_cast<long long>( value ) );
#elif defined( ATOMIC_GCC_CLANG )
	__atomic_store_n( &v, value, __ATOMIC_RELEASE );
#endif
}


u64 Atomic_U64::fetch_add( u64 value )
{
#if defined( ATOMIC_MSVC )
	return _InterlockedExchangeAdd64( reinterpret_cast<volatile long long*>( &v ), static_cast<long long>( value ) );
#elif defined( ATOMIC_GCC_CLANG )
	return __atomic_fetch_add( &v, value, __ATOMIC_ACQ_REL );
#endif
}


u64 Atomic_U64::fetch_sub( u64 value )
{
#if defined( ATOMIC_MSVC )
	return _InterlockedExchangeAdd64( reinterpret_cast<volatile long long*>( &v ), -static_cast<long long>( value ) );
#elif defined( ATOMIC_GCC_CLANG )
	return __atomic_fetch_sub( &v, value, __ATOMIC_ACQ_REL );
#endif
}


u64 Atomic_U64::fetch_or( u64 value )
{
#if defined( ATOMIC_MSVC )
	return _InterlockedOr64( reinterpret_cast<volatile long long*>( &v ), static_cast<long long>( value ) );
#elif defined( ATOMIC_GCC_CLANG )
	return __atomic_fetch_or( &v, value, __ATOMIC_ACQ_REL );
#endif
}


u64 Atomic_U64::fetch_and( u64 value )
{
#if defined( ATOMIC_MSVC )
	return _InterlockedAnd64( reinterpret_cast<volatile long long*>( &v ), static_cast<long long>( value ) );
#elif defined( ATOMIC_GCC_CLANG )
	return __atomic_fetch_and( &v, value, __ATOMIC_ACQ_REL );
#endif
}


u64 Atomic_U64::fetch_xor( u64 value )
{
#if defined( ATOMIC_MSVC )
	return _InterlockedXor64( reinterpret_cast<volatile long long*>( &v ), static_cast<long long>( value ) );
#elif defined( ATOMIC_GCC_CLANG )
	return __atomic_fetch_xor( &v, value, __ATOMIC_ACQ_REL );
#endif
}


u64 Atomic_U64::exchange( u64 value )
{
#if defined( ATOMIC_MSVC )
	return _InterlockedExchange64( reinterpret_cast<volatile long long*>( &v ), static_cast<long long>( value ) );
#elif defined( ATOMIC_GCC_CLANG )
	return __atomic_exchange_n( &v, value, __ATOMIC_ACQ_REL );
#endif
}


bool Atomic_U64::compare_exchange_strong( u64 &expected, u64 desired )
{
#if defined( ATOMIC_MSVC )
	long long e = static_cast<long long>( expected );
	long long o = _InterlockedCompareExchange64( &v, desired, e );
	if( o == e ) { return true; }
	expected = static_cast<u64>( o );
	return false;
#elif defined( ATOMIC_GCC_CLANG )
	u64 e = expected;
	bool result = __atomic_compare_exchange_n( &v, &e, desired, false, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE );
	expected = e;
	return result;
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////