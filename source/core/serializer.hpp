#pragma once

#include <vendor/vendor.hpp>

#include <core/types.hpp>
#include <core/buffer.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct SerializerKey
{
	SerializerKey() = delete;

	consteval SerializerKey( const char *key ) : hash { 5381 }
	{
		while( *key ) { hash = ( ( hash << 5 ) + hash ) + static_cast<u32>( *key++ ); }
	}

	SerializerKey( char *key ) : hash { 5381 }
	{
		while( *key ) { hash = ( ( hash << 5 ) + hash ) + static_cast<u32>( *key++ ); }
	}

	u32 hash;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Serialization { };

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

	template <typename T> void write( u32 hash, const T &variable )
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

	template <typename T> void write( SerializerKey name, const T &variable )
	{
		write<T>( name.hash, variable );
	}

	template <typename T> void write_array( u32 hash, const T *array, usize length )
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

	template <typename T> void write_array( SerializerKey name, const T *array, usize length )
	{
		write_array<T>( name.hash, array, length );
	}

	void write_data( u32 hash, const void *data, usize size )
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

		// Write data & size
		buffer->write<usize>( size );
		buffer->write( data, size );
	}

	void write_data( SerializerKey name, const void *data, usize size )
	{
		write_data( name.hash, data, size );
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Deserializer
{
private:
	// custom deserialize() must be: static bool T::deserialize( Buffer &buffer, T &type ) { ... }
	template <typename T> struct HasCustomDeserialize
	{
		template <typename U> static auto test( int ) ->
			decltype( static_cast<bool (*)( Buffer &, T & )>( &U::deserialize ), true_type() );
		template <typename U> static false_type test( ... );
		static const bool value = decltype( test<T>( 0 ) )::value;
	};

	Buffer *buffer = nullptr;
	usize endTell = USIZE_MAX;
	usize firstTell = USIZE_MAX;

public:
	u32 version = 0;

	void begin( Buffer &buffer, u32 version )
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
		// Update buffer tell
		Assert( this->buffer != nullptr );
		buffer->seek_to( endTell );
	}

	template <typename T> NO_DISCARD bool read( u32 hash, T &variable )
	{
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
					return Type::deserialize( *buffer, variable );
				}
				else
				{
					return buffer->read<Type>( variable );
				}
			}
			else
			// This isn't our element, continue or return
			{
				if( nextTell == USIZE_MAX ) { return false; }
				buffer->seek_to( nextTell );
				continue;
			}
		}

		// Element not found
		return false;
	}

	template <typename T> NO_DISCARD bool read( const SerializerKey name, T &variable )
	{
		return read<T>( name.hash, variable );
	}

	template <typename T> NO_DISCARD bool read_array( u32 hash, T *array, usize length )
	{
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
					for( usize i = 0; i < count; i++ )
					{
						if( !Type::deserialize( *buffer, array[i] ) ) { return false; };
					}
				}
				else
				{
					for( usize i = 0; i < count; i++ )
					{
						if( !buffer->read<Type>( array[i] ) ) { return false; };
					}
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

		// Array not found
		return false;
	}

	template <typename T> NO_DISCARD bool read_array( SerializerKey name, T *array, usize length )
	{
		return read_array<T>( name.hash, array, length );
	}

	NO_DISCARD bool read_data( u32 hash, void *data, usize size )
	{
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
				// Read size
				const usize sizeSerialized = buffer->read<usize>();
				ErrorIf( sizeSerialized != size, "Deserializing data buffer of mismatched size (e %llu, a: %llu)!",
					size, sizeSerialized );

				// Read data
				void *dataSerialized = buffer->read_bytes( sizeSerialized );
				memory_copy( data, dataSerialized, sizeSerialized );
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

		// Data not found
		return false;
	}

	NO_DISCARD bool read_data( SerializerKey name, void *data, usize size )
	{
		return read_data( name.hash, data, size );
	}

	template <typename TypePrevious, typename TypeCurrent>
	NO_DISCARD bool reinterpret( u32 hash, TypeCurrent &variable )
	{
		TypePrevious previous;
		if( !read<TypePrevious>( hash, previous ) ) { return false; }
		variable = static_cast<TypeCurrent>( previous );
		return true;
	}

	template <typename TypePrevious, typename TypeCurrent>
	NO_DISCARD bool reinterpret( SerializerKey name, TypeCurrent &variable )
	{
		TypePrevious previous;
		if( !read<TypePrevious>( name.hash, previous ) ) { return false; }
		variable = static_cast<TypeCurrent>( previous );
		return true;
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////