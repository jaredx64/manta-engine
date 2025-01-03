#pragma once

#include <vendor/new.hpp>

#include <core/memory.hpp>
#include <core/types.hpp>
#include <core/debug.hpp>
#include <core/buffer.hpp>
#include <core/serializer.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T>
class List
{
_PUBLIC:
#if MEMORY_RAII
	List( const usize reserve = 1 ) { init( reserve ); }
	List( const List<T> &other ) { copy( other ); }
	List( List<T> &&other ) { move( static_cast<List<T> &&>( other ) ); }
	~List() { free(); }

	List<T> &operator=( const List<T> &other ) { return copy( other ); }
	List<T> &operator=( List<T> &&other ) { return move( static_cast<List<T> &&>( other ) ); }
#else
	List<T> &operator=( const List<T> &other ) { Error( "List: assignment disabled" ); return *this; }
	List<T> &operator=( List<T> &&other ) { Error( "List: assignment disabled" ); return *this; }

#if MEMORY_ASSERTS
	~List()
	{
		// Memory Leak Detection
		if( Debug::memoryLeakDetection && Debug::exitCode == 0 )
		{
			MemoryAssertMsg( data == nullptr, "ERROR: Memory leak in List (%p) (size: %.2f kb)",
			                 this, KB( size_allocated_bytes() ) );
		}
	}
#endif
#endif

_PRIVATE:
	void grow()
	{
		MemoryAssert( data != nullptr );
		Assert( capacity >= 1 && capacity < USIZE_MAX );

		// Reallocate memory
		capacity = capacity > USIZE_MAX / 2 ? USIZE_MAX : capacity * 2;
		data = reinterpret_cast<T *>( memory_realloc( data, capacity * sizeof( T ) ) );
		ErrorIf( data == nullptr, "Failed to reallocate memory for grow List (%p: realloc %d bytes)",
		         data, capacity * sizeof( T ) );
	}

_PUBLIC:
	void init( const usize reserve = 1 )
	{
		// Set state
		capacity = reserve;
		current = 0;

		// Allocate memory
		MemoryAssert( data == nullptr );
		Assert( capacity >= 1 );
		data = reinterpret_cast<T *>( memory_alloc( capacity * sizeof( T ) ) );
		ErrorIf( data == nullptr, "Failed to allocate memory for init List (%p: alloc %d bytes)",
		         data, capacity * sizeof( T ) );
		memory_set( data, 0, capacity * sizeof( T ) );
	}

	void free()
	{
		if( data == nullptr )
		{
		#if MEMORY_RAII
			return;
		#else
			MemoryWarning( "List: attempting to free() list that is already freed!" ); return;
		#endif
		}

		// Free elements
		for( usize i = 0; i < current; i++ ) { data[i].~T(); }

		// Free memory
		memory_free( data );
		data = nullptr;

		// Reset state
		capacity = 0;
		current = 0;
	}

	List<T> &copy( const List<T> &other )
	{
		MemoryAssert( other.data != nullptr );
		if( this == &other ) { return *this; }
		if( data != nullptr ) { free(); }

		// Copy state
		capacity = other.capacity;
		current = other.current;

		// Allocate memory
		MemoryAssert( data == nullptr );
		Assert( capacity >= 1 );
		data = reinterpret_cast<T *>( memory_alloc( capacity * sizeof( T ) ) );
		ErrorIf( data == nullptr, "Failed to allocate memory for copy List (%p: alloc %d bytes)",
		         data, capacity * sizeof( T ) );

		// Copy memory
		for( usize i = 0; i < current; i++ ) { new ( &data[i] ) T( other.data[i] ); }

		// Return this
		return *this;
	}

	List<T> &move( List<T> &&other )
	{
		MemoryAssert( other.data != nullptr );
		if( this == &other ) { return *this; }
		if( data != nullptr ) { free(); }

		// Move the other Lists's resources
		data = other.data;
		capacity = other.capacity;
		current = other.current;

		// Reset other List to null state
		other.data = nullptr;
		other.capacity = 0;
		other.current = 0;

		// Return this
		return *this;
	}

	bool save( const char *path )
	{
		Buffer buffer;
		buffer.init( sizeof( usize ) + current * sizeof( T ), true );
		List<T>::write( buffer, *this );
		const bool success = buffer.save( path );
		buffer.free();
		return success;
	}

	bool load( const char *path )
	{
		Buffer buffer;
		if( !buffer.load( path, false ) ) { return false; }
		List<T>::read( buffer, *this );
		buffer.free();
		return true;
	}

	bool shrink()
	{
		// Shrink to current
		MemoryAssert( data != nullptr );
		if( current == capacity ) { return false; }
		capacity = current > 1 ? current : 1;
		data = reinterpret_cast<T *>( memory_realloc( data, capacity * sizeof( T ) ) );
		ErrorIf( data == nullptr, "Failed to reallocate memory for shrink List (%p: alloc %d bytes)",
		         data, capacity * sizeof( T ) );

		// Success
		return true;
	}

	void clear()
	{
		// Clear all elements
		MemoryAssert( data != nullptr );
		for( usize i = 0; i < current; i++ ) { data[i].~T(); }

		// Reset state
		current = 0;
	}

	T &add( const T &element )
	{
		MemoryAssert( data != nullptr );
		ErrorIf( current == USIZE_MAX, "List exceeded maximum possible capacity" );
		if( current == capacity ) { grow(); }

		// Add element
		const usize index = current++;
		new ( &data[index] ) T( element );
		return data[index];
	}

	T &add( T &&element )
	{
		MemoryAssert( data != nullptr );
		ErrorIf( current == USIZE_MAX, "List exceeded maximum possible capacity" );
		if( current == capacity ) { grow(); }

		// Add element
		const usize index = current++;
		new ( &data[index] ) T( static_cast<T &&>( element ) );
		return data[index];
	}

	T &insert( const usize index, const T &element )
	{
		MemoryAssert( data != nullptr );
		ErrorIf( index > current, "List insertion index out of bounds" );
		ErrorIf( current == USIZE_MAX, "List exceeded maximum possible capacity" );
		if( current == capacity ) { grow(); }

		// Shift data right
		const usize size = ( current++ ) - index;
		if( size != 0 ) { memory_move( &data[index + 1], &data[index], size * sizeof( T ) ); }

		// Insert element
		new ( &data[index] ) T( element );
		return data[index];
	}

	T &insert( const usize index, T &&element )
	{
		MemoryAssert( data != nullptr );
		ErrorIf( index > current, "List insertion index out of bounds" );
		ErrorIf( current == USIZE_MAX, "List exceeded maximum possible capacity" );
		if( current == capacity ) { grow(); }

		// Shift data right
		const usize size = ( current++ ) - index;
		if( size != 0 ) { memory_move( &data[index + 1], &data[index], size * sizeof( T ) ); }

		// Insert element
		new ( &data[index] ) T( static_cast<T &&>( element ) );
		return data[index];
	}

	T &replace( const usize index, const T &element )
	{
		MemoryAssert( data != nullptr );
		ErrorIf( index >= current, "List replace index out of bounds" );

		// Replace existing element
		data[index].~T();
		new ( &data[index] ) T( element );
		return data[index];
	}

	T &replace( const usize index, T &&element )
	{
		MemoryAssert( data != nullptr );
		ErrorIf( index >= current, "List replace index out of bounds" );

		// Replace existing element
		data[index].~T();
		new ( &data[index] ) T( static_cast<T &&>( element ) );
		return data[index];
	}

	void remove( const usize index )
	{
		MemoryAssert( data != nullptr );
		ErrorIf( index >= current, "List removal index out of bounds" );

		// Remove element (shift index + 1 left)
		data[index].~T();
		const usize size = ( --current ) - index;
		if( size > 0 ) { memory_move( &data[index], &data[index + 1], size * sizeof( T ) ); }
	}

	void remove_swap( const usize index )
	{
		MemoryAssert( data != nullptr );
		ErrorIf( index >= current, "List removal index out of bounds" );

		// Remove element (swap with last element)
		data[index].~T();
		if( --current != index ) { memory_copy( &data[index], &data[current], sizeof( T ) ); }
	}

	T &at( const usize index )
	{
		MemoryAssert( data != nullptr );
		ErrorIf( index >= current, "List at( %llu ) out of bounds [0,%llu)", index, current );
		return data[index];
	}

	const T &at( const usize index ) const
	{
		MemoryAssert( data != nullptr );
		ErrorIf( index >= current, "List at( %llu ) out of bounds [0,%llu)", index, current );
		return data[index];
	}

	T &front()
	{
		MemoryAssert( data != nullptr );
		ErrorIf( current == 0, "List front() called on empty list" );
		return data[0];
	}

	const T &front() const
	{
		MemoryAssert( data != nullptr );
		ErrorIf( current == 0, "List front() called on empty list" );
		return data[0];
	}

	T &back()
	{
		MemoryAssert( data != nullptr );
		ErrorIf( current == 0, "List back() called on empty list" );
		return data[current - 1];
	}

	const T &back() const
	{
		MemoryAssert( data != nullptr );
		ErrorIf( current == 0, "List back() called on empty list" );
		return data[current - 1];
	}

	bool contains( const T &element ) const
	{
		MemoryAssert( data != nullptr );
		for( usize i = 0; i < current; i++ )
		{
			if( data[i] == element ) { return true; }
		}

		// List does not contain element
		return false;
	}

	bool equals( const List<T> &other ) const
	{
		// NOTE: Equality here checks whether the elements match between containers

		// Ensure initialization
		if( data == nullptr && other.data == nullptr ) { return true; }
		if( data == nullptr || other.data == nullptr ) { return false; }

		// Compare elements
		if( current != other.current ) { return false; }
		for( usize i = 0; i < current; i++ )
		{
			if( data[i] != other.data[i] ) { return false; }
		}

		// Success
		return true;
	}

	usize count() const
	{
		return current;
	}

	usize size() const
	{
		return current;
	}

	usize size_allocated_bytes() const
	{
		return capacity * sizeof( T );
	}

	// Equality
    bool operator==( const List<T> &other ) const { return equals( other ); }
	bool operator!=( const List<T> &other ) const { return !equals( other ); }

	// Indexer
    T &operator[]( const usize index ) { return at( index ); }
	const T &operator[]( const usize index ) const { return at( index ); }

	// Initialization check
    explicit operator bool() const { return data != nullptr; }

	// Forward Iterator
	class forward_iterator
	{
	_PUBLIC:
		forward_iterator( T *element ) : element{ element } { }
		forward_iterator &operator++() { element++; return *this; }
		bool operator!=( const forward_iterator &other ) const { return element != other.element; }
		T &operator*() { return *element; }

	_PRIVATE:
		T *element;
	};

	forward_iterator begin() { MemoryAssert( data != nullptr ); return forward_iterator( &data[0] ); }
	forward_iterator end() { MemoryAssert( data != nullptr ); return forward_iterator( &data[current] ); }
	forward_iterator begin() const { MemoryAssert( data != nullptr ); return forward_iterator( &data[0] ); }
	forward_iterator end() const { MemoryAssert( data != nullptr ); return forward_iterator( &data[current] ); }

	struct reverse_iterator
	{
	_PUBLIC:
		reverse_iterator( T *element ) : element{ element } { }
		reverse_iterator &operator++() { element--; return *this; }
		bool operator!=( const reverse_iterator &other ) const { return element != other.element; }
		T &operator*() { return *element; }

	_PRIVATE:
		T *element;
	};

	reverse_iterator rbegin() { MemoryAssert( data != nullptr ); return reverse_iterator( &data[current] - 1 ); }
	reverse_iterator rend() { MemoryAssert( data != nullptr ); return reverse_iterator( &data[0] - 1 ); }
	reverse_iterator rbegin() const { MemoryAssert( data != nullptr ); return reverse_iterator( &data[current] - 1 ); }
	reverse_iterator rend() const { MemoryAssert( data != nullptr ); return reverse_iterator( &data[0] - 1 ); }

_PUBLIC:
	static void write( Buffer &buffer, const List<T> &list )
	{
		MemoryAssert( buffer.data != nullptr );
		MemoryAssert( list.data != nullptr );

		// Save current
		buffer.write<usize>( list.current );

		// Save elements
		for( usize i = 0; i < list.current; i++ )
		{
			buffer.write<T>( list.data[i] );
		}
	}

	static void read( Buffer &buffer, List<T> &list )
	{
		if( list.data != nullptr ) { list.free(); }
		MemoryAssert( list.data == nullptr );

		// Initialize memory
		list.init( buffer.read<usize>() );
		list.current = list.capacity;

		// Read elements
		for( usize i = 0; i < list.current; i++ )
		{
			buffer.read<T>( list.data[i] );
		}
	}

	static void serialize( Buffer &buffer, const List<T> &list )
	{
		MemoryAssert( list.data != nullptr );

		Serializer serializer;
		serializer.begin( buffer, 0 );
		serializer.write<usize>( "current", list.current );
		serializer.write_array( "elements", list.data, list.current );
		serializer.end();
	}

	static void deserialize( Buffer &buffer, List<T> &list )
	{
		if( list.data != nullptr ) { list.free(); }

		Deserializer deserializer;
		deserializer.begin( buffer, 0 );
		{
			usize current;
			deserializer.read<usize>( "current", current );
			list.init( current );
			list.current = current;

			deserializer.read_array<T>( "elements", list.data, list.current );
		}
		deserializer.end();
	}

_PRIVATE:
	T *data = nullptr;
	usize capacity = 0;
	usize current = 0;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////