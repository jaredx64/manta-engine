#pragma once

#include <core/types.hpp>
#include <core/debug.hpp>
#include <core/memory.hpp>

#include <vendor/new.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Buffer
{
public:
#if MEMORY_RAII
	Buffer( usize reserve = 1, const bool grow = true ) { init( reserve, grow ); }
	Buffer( const char *path, const bool grow = true ) { load( path, grow ); }
	Buffer( const Buffer &other ) { copy( other ); }
	Buffer( Buffer &&other ) { move( static_cast<Buffer &&>( other ) ); }
	~Buffer() { free(); }

	// Copy & Move Assignment
	Buffer &operator=( const Buffer &other ) { return copy( other ); }
	Buffer &operator=( Buffer &&other ) { return move( static_cast<Buffer &&>( other ) ); }
#else
	Buffer &operator=( const Buffer &other ) { Error( "Buffer: assignment disabled" ); return *this; }
	Buffer &operator=( Buffer &&other ) { Error( "Buffer: assignment disabled" ); return *this; }

#if MEMORY_ASSERTS
	~Buffer()
	{
		// Memory Leak Detection
		if( Debug::memoryLeakDetection && Debug::exitCode == 0 )
		{
			MemoryAssertMsg( data == nullptr, "ERROR: Memory leak in Buffer (%p) (size: %.2f kb)",
			                 this, KB( capacity ) );
		}
	}
#endif
#endif

private:
	// custom write() must be: static void T::write( Buffer &buffer, const T &type ) { ... }
	template <typename T> struct HasCustomWrite
	{
		template <typename U> static auto test( int ) ->
			decltype( static_cast<void (*)( Buffer &, const T & )>( &U::write ), true_type() );
		template <typename U> static false_type test( ... );
		static const bool value = decltype( test<T>( 0 ) )::value;
	};

	// custom read() must be: static void T::read( Buffer &buffer, T &type ) { ... }
	template <typename T> struct HasCustomRead
	{
		template <typename U> static auto test( int ) ->
			decltype( static_cast<void (*)( Buffer &, T & )>( &U::read ), true_type() );
		template <typename U> static false_type test( ... );
		static const bool value = decltype( test<T>( 0 ) )::value;
	};

	void grow();

public:
	void init( usize reserve = 1, const bool grow = true );
	void free();
	bool load( const char *path, const bool grow = false );
	bool save( const char *path );
	Buffer &copy( const Buffer &other );
	Buffer &move( Buffer &&other );

	bool shrink();
	void clear();

	usize write( void *bytes, const usize size );
	usize write_from_file( const char *path, const usize offset, const usize size );

	template <typename T> usize write( const T &element )
	{
		MemoryAssert( data != nullptr );
		using Type = typename remove_reference<T>::type;

		if constexpr ( Buffer::HasCustomWrite<Type>::value )
		{
			return Type::write( *this, element );
		}
		else
		{
			// Align & grow memory
			while( !fixed )
			{
				if( tell + sizeof( Type ) <= capacity ) { break; }
				grow();
			}

			// Write element
			const usize writeIndex = tell;
			ErrorIf( writeIndex + sizeof( Type ) > capacity, "Buffer: write exceeded buffer capacity" );
			new ( reinterpret_cast<Type *>( &data[writeIndex] ) ) Type( element );
			tell += sizeof( Type );
			current = tell > current ? tell : current;
			return writeIndex;
		}
	}

	template <typename T> usize poke( usize offset, const T &element )
	{
		MemoryAssert( data != nullptr );
		using Type = typename remove_reference<T>::type;

		if constexpr ( Buffer::HasCustomWrite<Type>::value )
		{
			const usize tellCache = tell;
			seek_to( offset );
			const usize writeIndex = write<Type>( element );
			seek_to( tellCache );
			return writeIndex;
		}
		else
		{
			// Align & grow memory
			while( !fixed )
			{
				if( offset + sizeof( Type ) <= capacity ) { break; }
				grow();
			}

			// Write element
			const usize writeIndex = offset;
			ErrorIf( writeIndex + sizeof( Type ) > capacity, "Buffer: poke exceeded buffer capacity" );
			new ( reinterpret_cast<Type *>( &data[writeIndex] ) ) Type( element );
			return writeIndex;
		}
	}

	template <typename T> void read( T &element )
	{
		MemoryAssert( data != nullptr );
		using Type = typename remove_reference<T>::type;

		if constexpr ( Buffer::HasCustomRead<Type>::value )
		{
			Type::read( *this, element );
		}
		else
		{
			using Type = typename remove_reference<T>::type;
			ErrorIf( tell + sizeof( Type ) > current, "Buffer: read exceeded buffer capacity" );
			element = *reinterpret_cast<Type *>( &data[tell] );
			tell += sizeof( Type );
		}
	}

	template <typename T> T &read()
	{
		static_assert( !Buffer::HasCustomRead<T>::value,
		               "Type has custom T::read(), must use buffer.read<T>( T &type ) syntax" );

		MemoryAssert( data != nullptr );
		ErrorIf( tell + sizeof( T ) > current, "Buffer: read exceeded buffer capacity" );
		byte *element =  &data[tell];
		tell += sizeof( T );
		return *reinterpret_cast<T *>( element );
	}

	void *read_bytes( const usize count )
	{
		MemoryAssert( data != nullptr );
		ErrorIf( tell + count > current, "Buffer: read_bytes exceeded buffer capacity" );
		void *pointer = reinterpret_cast<void *>( &data[tell] );
		tell += count;
		return pointer;
	}

	template <typename T> void peek( T &element )
	{
		MemoryAssert( data != nullptr );
		using Type = typename remove_reference<T>::type;
		if constexpr ( Buffer::HasCustomRead<Type>::value )
		{
			const usize tellCache = tell;
			Type::read( *this, element );
			seek_to( tellCache );
		}
		else
		{
			ErrorIf( tell + sizeof( Type ) > current, "Buffer: peek exceeded buffer capacity" );
			element = *reinterpret_cast<Type *>( &data[tell] );
		}
	}

	template <typename T> T &peek() const
	{
		static_assert( !Buffer::HasCustomRead<T>::value,
		               "Type has custom T::read(), must use buffer.peek<T>( T &type ) syntax" );
		MemoryAssert( data != nullptr );
		ErrorIf( tell + sizeof( T ) > current, "Buffer: peek exceeded buffer capacity" );
		return *reinterpret_cast<T *>( &data[tell] );
	}

	template <typename T> bool has_next() const
	{
		MemoryAssert( data != nullptr );
		return tell + sizeof( T ) <= current;
	}

	void seek_start() { tell = 0; }
	void seek_end() { tell = current; }
	void seek_to( const usize offset ) { Assert( tell <= current ); tell = offset; }

	usize size() const { return current; }
	usize size_allocated_bytes() const { return capacity; }

public:
	byte *data = nullptr;
	usize tell = 0LLU;

private:
	usize capacity = 0LLU;
	usize current = 0LLU;
	bool fixed = false;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////