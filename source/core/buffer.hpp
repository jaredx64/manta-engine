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
	Buffer( usize reserve = 1, bool grow = true ) { init( reserve, grow ); }
	Buffer( const char *path, bool grow = true ) { load( path, grow ); }
	Buffer( const Buffer &other ) { copy( other ); }
	Buffer( Buffer &&other ) { move( static_cast<Buffer &&>( other ) ); }
	~Buffer() { free(); }

	Buffer &operator=( const Buffer &other ) { return copy( other ); }
	Buffer &operator=( Buffer &&other ) { return move( static_cast<Buffer &&>( other ) ); }
#else
	Buffer &operator=( const Buffer &other ) { Error( "Buffer: assignment disabled" ); return *this; }
	Buffer &operator=( Buffer &&other ) { Error( "Buffer: assignment disabled" ); return *this; }

#if MEMORY_ASSERTS
	~Buffer()
	{
		if( Debug::memoryLeakDetection && Debug::exitCode == 0 )
		{
			MemoryAssertMsg( data == nullptr, "ERROR: Memory leak in Buffer (%p) (size: %.2f kb)",
				this, KB( capacity ) );
		}
	}
#endif
#endif

private:
	// custom write() must be: static bool T::write( Buffer &buffer, const T &type ) { ... }
	template <typename T> struct HasCustomWrite
	{
		template <typename U> static auto test( int ) ->
			decltype( static_cast<void (*)( Buffer &, const T & )>( &U::write ), true_type() );
		template <typename U> static false_type test( ... );
		static const bool value = decltype( test<T>( 0 ) )::value;
	};

	// custom read() must be: static bool T::read( Buffer &buffer, T &type ) { ... }
	template <typename T> struct HasCustomRead
	{
		template <typename U> static auto test( int ) ->
			decltype( static_cast<bool (*)( Buffer &, T & )>( &U::read ), true_type() );
		template <typename U> static false_type test( ... );
		static const bool value = decltype( test<T>( 0 ) )::value;
	};

	void grow();

public:
	void init( usize reserve = 1, bool grow = true );
	void free();
	bool load( const char *path, bool grow = false );
	bool save( const char *path );
	Buffer &copy( const Buffer &other );
	Buffer &move( Buffer &&other );

	bool shrink();
	void clear();

	usize compress();
	usize compress( void *tempBuffer, usize tempBufferSize );
	usize compress_into( void *buffer, usize size );
	bool decompress( const void *dataCompressed, usize sizeCompressed, usize sizeOriginal );

	usize write( const void *bytes, usize size );
	usize write_from_file( const char *path, usize offset, usize size );

	void shift( int amount );

	template <typename T> usize write( const T &element )
	{
		MemoryAssert( data != nullptr );
		using Type = typename remove_reference<T>::type;

		if constexpr ( Buffer::HasCustomWrite<Type>::value )
		{
			const usize writeIndex = tell;
			Type::write( *this, element );
			return writeIndex;
		}
		else
		{
			// Grow memory
			for( ; !fixed && tell + sizeof( Type ) > capacity; grow() ) { }

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
			// Grow Memory
			for( ; !fixed && offset + sizeof( Type ) > capacity; grow() ) { }

			// Write element
			const usize writeIndex = offset;
			ErrorIf( writeIndex + sizeof( Type ) > capacity, "Buffer: poke exceeded buffer capacity" );
			new ( reinterpret_cast<Type *>( &data[writeIndex] ) ) Type( element );
			return writeIndex;
		}
	}

	template <typename T> bool read( T &element )
	{
		MemoryAssert( data != nullptr );
		using Type = typename remove_reference<T>::type;

		if constexpr ( Buffer::HasCustomRead<Type>::value )
		{
			return Type::read( *this, element );
		}
		else
		{
			using Type = typename remove_reference<T>::type;
			ErrorIf( tell + sizeof( Type ) > current, "Buffer: read exceeded buffer capacity" );
			element = *reinterpret_cast<Type *>( &data[tell] );
			tell += sizeof( Type );
			return true;
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

	template <typename T> bool peek( T &element )
	{
		MemoryAssert( data != nullptr );
		using Type = typename remove_reference<T>::type;
		if constexpr ( Buffer::HasCustomRead<Type>::value )
		{
			const usize tellCache = tell;
			const bool success = Type::read( *this, element );
			seek_to( tellCache );
			return success;
		}
		else
		{
			ErrorIf( tell + sizeof( Type ) > current, "Buffer: peek exceeded buffer capacity" );
			element = *reinterpret_cast<Type *>( &data[tell] );
			return true;
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
	void seek_to( usize offset ) { Assert( offset <= current ); tell = offset; }

	byte *tell_ptr() const { MemoryAssert( data != nullptr ); return &data[tell]; }
	usize size() const { return current; }
	usize size_allocated_bytes() const { return capacity; }
	usize bytes_remaining() const { return current - tell; }

public:
	byte *data = nullptr;
	usize tell = 0LLU;

private:
	usize capacity = 0LLU;
	usize current = 0LLU;
	bool fixed = false;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////