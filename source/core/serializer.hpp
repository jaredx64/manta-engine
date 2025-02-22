#pragma once

#include <core/types.hpp>
#include <core/buffer.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct SerializerKey
{
	SerializerKey() = delete;

	consteval SerializerKey( const char *key ) : hash{ 5381 }
	{
		while( *key ) { hash = ( ( hash << 5 ) + hash ) + static_cast<u32>( *key++ ); }
	}

	SerializerKey( char *key ) : hash{ 5381 }
	{
		while( *key ) { hash = ( ( hash << 5 ) + hash ) + static_cast<u32>( *key++ ); }
	}

	u32 hash;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Serializer
{
private:
	// custom serialize() must be: static void T::serialize( Buffer &buffer, const T &type ) { ... }
	template <typename T> struct HasCustomSerialize
	{
		template <typename U> static auto test( int ) ->
			decltype( static_cast<void (*)( Buffer &, const T & )>( &U::serialize ), true_type() );
		template <typename U> static false_type test( ... );
		static const bool value = decltype( test<T>( 0 ) )::value;
	};

	Buffer *buffer = nullptr;
	usize endTell = USIZE_MAX;
	usize nextTell = USIZE_MAX;

public:
	void begin( Buffer &buffer, const u32 version )
	{
		// Set state
		this->buffer = &buffer;
		Assert( this->buffer != nullptr );

		// Write version & temporary end offset
		this->buffer->write<u32>( version );
		endTell = this->buffer->tell;
		this->buffer->write<usize>( USIZE_MAX );
	}

	void end()
	{
		// Update 'end' offset
		Assert( endTell != USIZE_MAX );
		buffer->poke<usize>( endTell, buffer->tell );
	}

	template <typename T> void write( const u32 hash, const T &variable )
	{
		// Update the previous element's 'next' offset
		if( nextTell != USIZE_MAX )
		{
			buffer->poke<usize>( nextTell, buffer->tell );
		}

		// Write name hash & temporary 'next' offset
		buffer->write<u32>( hash );
		nextTell = buffer->tell;
		buffer->write<usize>( USIZE_MAX );

		// Write element
		using Type = typename remove_reference<T>::type;
		if constexpr ( Serializer::HasCustomSerialize<Type>::value )
		{
			Type::serialize( *buffer, variable );
		}
		else
		{
			buffer->write<Type>( variable );
		}
	}

	template <typename T> void write( const SerializerKey name, const T &variable )
	{
		write<T>( name.hash, variable );
	}

	template <typename T> void write_array( const u32 hash, const T *array, const usize length )
	{
		// Update the previous element's 'next' offset
		if( nextTell != USIZE_MAX )
		{
			buffer->poke<usize>( nextTell, buffer->tell );
		}

		// Write name hash & temporary 'next' offset
		buffer->write<u32>( hash );
		nextTell = buffer->tell;
		buffer->write<usize>( USIZE_MAX );

		// Write array length
		buffer->write<usize>( length );

		// Write elements
		using Type = typename remove_reference<T>::type;
		if constexpr ( Serializer::HasCustomSerialize<Type>::value )
		{
			for( usize i = 0; i < length; i++ ) { Type::serialize( *buffer, array[i] ); }
		}
		else
		{
			for( usize i = 0; i < length; i++ ) { buffer->write<Type>( array[i] ); }
		}
	}

	template <typename T> void write_array( const SerializerKey name, const T *array, const usize length )
	{
		write_array<T>( name.hash, array, length );
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Deserializer
{
private:
	// custom deserialize() must be: static void T::deserialize( Buffer &buffer, T &type ) { ... }
	template <typename T> struct HasCustomDeserialize
	{
		template <typename U> static auto test( int ) ->
			decltype( static_cast<void (*)( Buffer &, T & )>( &U::deserialize ), true_type() );
		template <typename U> static false_type test( ... );
		static const bool value = decltype( test<T>( 0 ) )::value;
	};

	Buffer *buffer = nullptr;
	bool failure = false;
	usize endTell = USIZE_MAX;
	usize firstTell = USIZE_MAX;

public:
	u32 version = 0;

	void begin( Buffer &buffer, const u32 version )
	{
		// Set state
		this->buffer = &buffer;
		Assert( this->buffer != nullptr );

		// Read version & end offset
		this->version = this->buffer->read<u32>();
		this->endTell = this->buffer->read<usize>();
		this->firstTell = this->buffer->tell;
	}

	void end()
	{
		// Skip if failure
		if( failure ) { return; }

		// Update buffer tell
		Assert( this->buffer != nullptr );
		buffer->seek_to( endTell );
	}

	template <typename T> bool read( const u32 hash, T &variable )
	{
		// Skip if failure
		if( failure ) { return false; }

		// Seek buffer to first element
		buffer->seek_to( firstTell );
		if( firstTell == endTell ) { return false; }

		// Search for element by name
		for( ;; )
		{
			// Read hash
			const u32 thisHash = buffer->read<u32>();
			const usize nextTell = buffer->read<usize>();

			// Found our element?
			if( thisHash == hash )
			{
				// Read element
				using Type = typename remove_reference<T>::type;
				if constexpr ( Deserializer::HasCustomDeserialize<Type>::value )
				{
					Type::deserialize( *buffer, variable );
				}
				else
				{
					buffer->read<Type>( variable );
				}
				return true;
			}
			else
			// This isn't our element, continue or return
			{
				if( nextTell == USIZE_MAX ) { return false; }
				buffer->seek_to( nextTell );
				continue;
			}
		}
	}

	template <typename T> bool read( const SerializerKey name, T &variable )
	{
		return read<T>( name.hash, variable );
	}

	template <typename TypePrevious, typename TypeCurrent>
	bool reinterpret( const u32 hash, TypeCurrent &variable )
	{
		// Skip if failure
		if( failure ) { return false; }

		TypePrevious previous;
		if( !read<TypePrevious>( hash, previous ) ) { return false; }
		variable = static_cast<TypeCurrent>( previous );
		return true;
	}

	template <typename TypePrevious, typename TypeCurrent>
	bool reinterpret( const SerializerKey name, TypeCurrent &variable )
	{
		// Skip if failure
		if( failure ) { return false; }

		TypePrevious previous;
		if( !read<TypePrevious>( name.hash, previous ) ) { return false; }
		variable = static_cast<TypeCurrent>( previous );
		return true;
	}

	template <typename T> bool read_array( const u32 hash, T *array, const usize length )
	{
		// Skip if failure
		if( failure ) { return false; }

		// Seek buffer to first element
		buffer->seek_to( firstTell );
		if( firstTell == endTell ) { return false; }

		// Search for element by name
		for( ;; )
		{
			// Read hash
			const u32 thisHash = buffer->read<u32>();
			const usize nextTell = buffer->read<usize>();

			// Found our element?
			if( thisHash == hash )
			{
				// Read array length
				const usize count = buffer->read<usize>();
				ErrorIf( count != length, "Deserializing an array of mismatched lengths (e %llu, a: %llu)!",
					length, count );

				// Read element
				using Type = typename remove_reference<T>::type;
				if constexpr ( Deserializer::HasCustomDeserialize<Type>::value )
				{
					for( usize i = 0; i < count; i++ ) { Type::deserialize( *buffer, array[i] ); }
				}
				else
				{
					for( usize i = 0; i < count; i++ ) { buffer->read<Type>( array[i] ); }
				}
				return true;
			}
			else
			// This isn't our element, continue or return
			{
				if( nextTell == USIZE_MAX ) { return false; }
				buffer->seek_to( nextTell );
				continue;
			}
		}
	}

	template <typename T> bool read_array( const SerializerKey name, T *array, const usize length )
	{
		return read_array<T>( name.hash, array, length );
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////