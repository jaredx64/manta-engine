#pragma once

#include <vendor/vendor.hpp>
#include <vendor/new.hpp>

#include <core/debug.hpp>
#include <core/traits.hpp>
#include <core/memory.hpp>
#include <core/buffer.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define HASHMAP_LOAD_FACTOR 0.75

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Types used as keys need to have the following functions:
//
// namespace Hash
// {
//     u32 hash( T key );
//     bool equals( T a, T b );
//     bool is_null( T key );
//     bool set_null( T &key );
// };
//
// See traits.hpp

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename K, typename V>
struct HashMap
{
public:
#if MEMORY_RAII
	HashMap( u32 reserve = 32, const V &nullElement = { } ) { init( reserve, nullElement ); }
	HashMap( const HashMap<K, V> &other ) { copy( other ); }
	HashMap( HashMap<K, V> &&other ) { move( static_cast<HashMap<K, V> &&>( other ) ); }
	~HashMap() { free(); }

	HashMap<K, V> &operator=( const HashMap<K, V> &other ) { return copy( other ); }
	HashMap<K, V> &operator=( HashMap<K, V> &&other ) { return move( static_cast<HashMap<K, V> &&>( other ) ); }
#else
	HashMap<K, V> &operator=( const HashMap<K, V> &other ) { Error( "HashMap: assignment disabled" ); return *this; }
	HashMap<K, V> &operator=( HashMap<K, V> &&other ) { Error( "HashMap: assignment disabled" ); return *this; }

#if MEMORY_ASSERTS
	~HashMap()
	{
		if( Debug::memoryLeakDetection && Debug::exitCode == 0 )
		{
			MemoryAssertMsg( data == nullptr, "ERROR: Memory leak in HashMap (%p) (size: %.2f kb)",
				this, KB( size_allocated_bytes() ) );
		}
	}
#endif
#endif

private:
	struct KeyValue { K key; V value; };

	void grow()
	{
		ErrorIf( capacity == U32_MAX, "HashMap grow exceeded maximum capacity: %llu", U32_MAX );
		Assert( capacity >= 1 );

		// Grow capacity
		const u32 capacityPrevious = capacity;
		capacity = capacity > U32_MAX / 2 ? U32_MAX : capacity * 2;

		// Allocate memory
		MemoryAssert( data != nullptr );
		KeyValue *dataOld = data;
		data = reinterpret_cast<KeyValue *>( memory_alloc( capacity * sizeof( KeyValue ) ) );
		MemoryAssert( deadslot != nullptr );
		deadslot = reinterpret_cast<bool *>( memory_realloc( deadslot, capacity * sizeof( bool ) ) );

		// Initialize new slots
		for( u32 i = 0; i < capacity; i++ )
		{
			deadslot[i] = false;
			Hash::set_null( data[i].key );
		}

		// Rehash keys from the old 'data' table
		for( u32 indexOld = 0; indexOld < capacityPrevious; indexOld++ )
		{
			// Skip null keys
			if( Hash::is_null( dataOld[indexOld].key ) ) { continue; }

			u32 indexNew = U32_MAX;
			find( dataOld[indexOld].key, indexNew );
			new ( &data[indexNew].key ) K( static_cast<K &&>( dataOld[indexOld].key ) );
			new ( &data[indexNew].value ) V( static_cast<V &&>( dataOld[indexOld].value ) );
		}

		// Free old data
		memory_free( dataOld );
	}

	bool find( const K &key, u32 &outIndex ) const
	{
		// NOTE: returns true if key exists, and false if not
		//       outIndex is the index of the existing key or nearest open slot

		MemoryAssert( data != nullptr && deadslot != nullptr );
		const u32 indexHash = Hash::hash( key ) ;
		const u32 indexEnd = ( indexHash - 1 ) & ( capacity - 1 );
		u32 indexCurrent = ( indexHash ) & ( capacity - 1 );
		u32 indexDead = U32_MAX; // First null slot that was recently deleted

		for( ;; )
		{
			// Have we looped around?
			if( indexCurrent == indexEnd )
			{
				outIndex = indexDead == U32_MAX ? indexCurrent : indexDead;
				return false;
			}

			// Check if slot is empty and not marked as dead
			if( Hash::is_null( data[indexCurrent].key ) && deadslot[indexCurrent] == false )
			{
				outIndex = indexDead == U32_MAX ? indexCurrent : indexDead;
				return false;
			}

			// Check if slot matches the search key
			if( Hash::equals( key, data[indexCurrent].key ) )
			{
				outIndex = indexCurrent;
				return true;
			}

			// Move to the next slot
			if( deadslot[indexCurrent] == true && indexDead == U32_MAX ) { indexDead = indexCurrent; }
			indexCurrent = ( indexCurrent + 1 ) & ( capacity - 1 );
		}

		// Unreachable!
		Assert( false ); return false;
	}

public:
	void init( const u32 reserve = 32, const V &nullElement = { } )
	{
		Assert( reserve >= 1 );
		capacity = align_pow2( reserve );
		size = 0;
		new ( &null ) V( nullElement );

		MemoryAssert( data == nullptr );
		data = reinterpret_cast<KeyValue *>( memory_alloc( capacity * sizeof( KeyValue ) ) );
		MemoryAssert( deadslot == nullptr );
		deadslot = reinterpret_cast<bool *>( memory_alloc( capacity * sizeof( bool ) ) );

		for( u32 i = 0; i < capacity; i++ )
		{
			deadslot[i] = false;
			Hash::set_null( data[i].key );
		}
	}

	void free()
	{
		if( data == nullptr )
		{
		#if MEMORY_RAII
			return;
		#else
			MemoryWarning( "HashMap: attempting to free() hashmap that is already freed!" ); return;
		#endif
		}

		for( u32 i = 0; i < capacity; i++ )
		{
			if( Hash::is_null( data[i].key ) ) { continue; }
			data[i].~KeyValue();
		}

		memory_free( data );
		data = nullptr;
		memory_free( deadslot );
		deadslot = nullptr;

		capacity = 0LLU;
		size = 0LLU;
		null.~V();
	}

	HashMap<K, V> &copy( const HashMap<K, V> &other )
	{
		MemoryAssert( other.data != nullptr && other.deadslot != nullptr );
		if( this == &other ) { return *this; }
		if( data != nullptr ) { free(); }

		capacity = other.capacity;
		Assert( capacity >= 1 );
		size = other.size;
		null.~V();
		new ( &null ) V( other.null );

		MemoryAssert( data == nullptr );
		data = reinterpret_cast<KeyValue *>( memory_alloc( capacity * sizeof( KeyValue ) ) );
		deadslot = reinterpret_cast<bool *>( memory_alloc( capacity * sizeof( bool ) ) );

		for( u32 i = 0; i < capacity; i++ )
		{
			deadslot[i] = other.deadslot[i];
			if( Hash::is_null( other.data[i].key ) ) { Hash::set_null( data[i].key ); continue; }
			new ( &data[i].key ) K( other.data[i].key );
			new ( &data[i].value ) V( other.data[i].value );
		}

		return *this;
	}

	HashMap<K, V> &move( HashMap<K, V> &&other )
	{
		MemoryAssert( other.data != nullptr && other.deadslot != nullptr );
		if( this == &other ) { return *this; }
		if( data != nullptr ) { free(); }

		data = other.data;
		deadslot = other.deadslot;
		capacity = other.capacity;
		size = other.size;
		null.~V();
		null = static_cast<V &&>( other.null );

		other.data = nullptr;
		other.deadslot = nullptr;
		other.capacity = 0;
		other.size = 0;
		other.null.~V();

		return *this;
	}

	void clear()
	{
		MemoryAssert( data != nullptr && deadslot != nullptr );
		for( u32 i = 0; i < capacity; i++ )
		{
			deadslot[i] = false;
			if( Hash::is_null( data[i].key ) ) { continue; }
			data[i].~KeyValue();
			Hash::set_null( data[i].key );
		}

		size = 0;
	}

	void shrink()
	{
		// TODO
	}

	void rehash()
	{
		// Allocate new 'data' memory
		Assert( capacity >= 1 );
		MemoryAssert( data != nullptr );
		KeyValue *dataOld = data;
		data = reinterpret_cast<KeyValue *>( memory_alloc( capacity * sizeof( KeyValue ) ) );

		// Initialize new slots
		for( u32 i = 0; i < capacity; i++ )
		{
			deadslot[i] = false;
			Hash::set_null( data[i].key );
		}

		// Rehash keys from the old 'data' table
		for( u32 indexOld = 0; indexOld < capacity; indexOld++ )
		{
			// Skip null keys
			if( Hash::is_null( dataOld[indexOld].key ) ) { continue; }

			u32 indexNew = U32_MAX;
			find( dataOld[indexOld].key, indexNew );
			new ( &data[indexNew].key ) K( static_cast<K &&>( dataOld[indexOld].key ) );
			new ( &data[indexNew].value ) V( static_cast<V &&>( dataOld[indexOld].value ) );
		}

		// Free old data
		memory_free( dataOld );
	}

	bool is_initialized() const
	{
		return data != nullptr;
	}

	V &get( const K &key )
	{
		MemoryAssert( data != nullptr && deadslot != nullptr );
		if( UNLIKELY( size + 1 > capacity * HASHMAP_LOAD_FACTOR ) ) { grow(); }
		u32 index = U32_MAX;

		if( !find( key, index ) )
		{
			size++;
			deadslot[index] = false;
			new ( &data[index].key ) K( key );
			new ( &data[index].value ) V( null );
		}

		return data[index].value;
	}

	V &set( const K &key, const V &value )
	{
		MemoryAssert( data != nullptr && deadslot != nullptr );
		V &outValue = get( key );
		new ( &outValue ) V( value );
		return outValue;
	}

	V &set( const K &key, V &&value )
	{
		MemoryAssert( data != nullptr && deadslot != nullptr );
		V &outValue = get( key );
		new ( &outValue ) V( static_cast<V &&>( value ) );
		return outValue;
	}

	bool add( const K &key, const V &value )
	{
		MemoryAssert( data != nullptr && deadslot != nullptr );
		if( UNLIKELY( size + 1 > capacity * HASHMAP_LOAD_FACTOR ) ) { grow(); }
		u32 index = U32_MAX;

		if( !find( key, index ) )
		{
			size++;
			deadslot[index] = false;
			new ( &data[index].key ) K( key );
			new ( &data[index].value ) V( value );
			return true;
		}

		return false;
	}

	bool add( const K &key, V &&value )
	{
		MemoryAssert( data != nullptr && deadslot != nullptr );
		if( UNLIKELY( size + 1 > capacity * HASHMAP_LOAD_FACTOR ) ) { grow(); }
		u32 index = U32_MAX;

		if( !find( key, index ) )
		{
			size++;
			deadslot[index] = false;
			new ( &data[index].key ) K( key );
			new ( &data[index].value ) V( static_cast<V &&>( value ) );
			return true;
		}

		return false;
	}

	bool remove( const K &key )
	{
		MemoryAssert( data != nullptr && deadslot != nullptr );
		u32 index = U32_MAX;

		if( find( key, index ) )
		{
			size--;// TODO: decrement size?
			deadslot[index] = true;
			Hash::set_null( data[index].key );
			data[index].value.~V();
			return true;
		}

		return false;
	}

	bool contains( const K &key ) const
	{
		u32 index = U32_MAX;
		return find( key, index );
	}

	bool equals( const HashMap<K, V> &other ) const
	{
		// TODO: Implement
		return false;
	}

	usize count() const
	{
		// TODO: fix?
		return size;
	}

	usize size_allocated_bytes() const
	{
		return capacity * ( sizeof( KeyValue ) + sizeof( bool ) );
	}

    bool operator==( const HashMap<K, V> &other ) const { return  equals( other ); }
	bool operator!=( const HashMap<K, V> &other ) const { return !equals( other ); }

	V &operator[]( const K &key ) { return get( key ); }

	class forward_iterator
	{
	public:
		forward_iterator( KeyValue *ptr, KeyValue *end ) : end{ end } { this->ptr = find_next( ptr ); } // begin();
		forward_iterator( KeyValue *end ) : end{ end }, ptr{ end } { } // end();
		KeyValue &operator*() { return *ptr; }
		forward_iterator &operator++() { ptr = find_next( ptr + 1 ); return *this; }
		bool operator!=( const forward_iterator &other ) const { return ptr != other.ptr; }

	private:
		KeyValue *find_next( KeyValue *ptr )
		{
			while( ptr != end )
			{
				if( !Hash::is_null( ptr->key ) ) { break; }
				ptr++;
			}
			return ptr;
		}

		KeyValue *end;
		KeyValue *ptr;
	};

	forward_iterator begin() { return forward_iterator( &data[0], &data[capacity] ); }
	forward_iterator end() { return forward_iterator( &data[capacity] ); }
	forward_iterator begin() const { return forward_iterator( &data[0], &data[capacity] ); }
	forward_iterator end() const { return forward_iterator( &data[capacity] ); }

    explicit operator bool() const { return data != nullptr; }

public:
	static void write( Buffer &buffer, const HashMap<K, V> &hashmap )
	{
		MemoryAssert( buffer.data != nullptr );
		MemoryAssert( hashmap.data != nullptr );

		buffer.write<u32>( hashmap.size );
		for( usize i = 0; i < hashmap.capacity; i++ )
		{
			if( Hash::is_null( hashmap.data[i].key ) ) { continue; }
			buffer.write<K>( hashmap.data[i].key );
			buffer.write<V>( hashmap.data[i].value );
		}
	}

	NO_DISCARD static bool read( Buffer &buffer, HashMap<K, V> &hashmap )
	{
		u32 size;
		if( !buffer.read<u32>( size ) ) { return false; }

		const bool hashmapAlreadyInitialized = hashmap.data != nullptr;
		auto cleanup_failure = [&]() { if( !hashmapAlreadyInitialized ) { hashmap.free(); } };
		if( hashmapAlreadyInitialized ) { hashmap.free(); }
		hashmap.init( size );

		for( usize i = 0; i < size; i++ )
		{
			K key;
			if( !buffer.read<K>( key ) ) { cleanup_failure(); return false; };
			if( !buffer.read<V>( hashmap.get( key ) ) ) { cleanup_failure(); return false; };
		}

		return true;
	}

	static void serialize( Buffer &buffer, const HashMap<K, V> &hashmap )
	{
		// TODO: Implement HashMap serialize() function
		Error( "TODO: Implement this!" );
	}

	NO_DISCARD static bool deserialize( Buffer &buffer, const HashMap<K, V> &hashmap )
	{
		// TODO: Implement HashMap deserialize() function
		Error( "TODO: Implement this!" );
		return true;
	}

private:
	KeyValue *data = nullptr;
	bool *deadslot = nullptr;
	u32 capacity = 0;
	u32 size = 0;
	V null;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////