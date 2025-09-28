#pragma once

#include <core/debug.hpp>
#include <core/types.hpp>

#include <vendor/stdarg.hpp>
#include <vendor/string.hpp>

#include <boot/platform.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace CoreText
{
	inline void macro_strjoin( const usize size, char *buffer, ... )
	{
		if( buffer == nullptr || size == 0 ) { return; }

		va_list args;
		va_start( args, buffer );
		usize length = 0;

		const char *input;
		while( ( input = va_arg( args, const char * ) ) != nullptr )
		{
			usize inputLength = strlen( input );
			if( length + inputLength >= size )
			{
				buffer[length] = '\0';
				AssertMsg( false, "strjoin - length exceeded buffer size" )
				va_end( args );
				return;
			}
			strcpy( buffer + length, input );
			length += inputLength;
		}

		va_end( args );
		buffer[length] = '\0';
	}


	inline void _filepath( const usize size, char *buffer, ... )
	{
		if( buffer == nullptr || size == 0 ) { return; }
		const usize slashLength = strlen( SLASH );

		va_list args;
		va_start( args, buffer );

		// Copy the first string
		buffer[0] = '\0';
		strncat( buffer, va_arg( args, const char * ), size - 1 );
		buffer[size - 1] = '\0';
		usize length = strlen( buffer );

		const char *input;
		while( ( input = va_arg( args, const char * ) ) != nullptr )
		{
			usize inputLength = slashLength + strlen( input );
			if( length + inputLength >= size )
			{
				buffer[length] = '\0';
				AssertMsg( false, "strjoin_path - length exceeded buffer size" )
				va_end( args );
				return;
			}
			strcpy( buffer + length, SLASH );
			strcpy( buffer + length + slashLength, input );
			length += inputLength;
		}

		va_end( args );
		buffer[length] = '\0';
	}


	inline void macro_strappend( const usize size, char *buffer, const char *string )
	{
		if( buffer == nullptr || string == nullptr || size == 0 ) { return; }

		usize bufferLength = strlen( buffer );
		usize stringLength = strlen( string );

		AssertMsg( bufferLength + stringLength < size, "strappend - length exceeded buffer size" )
		if( bufferLength + stringLength < size ) { strncat( buffer, string, size - bufferLength - 1 ); }

		buffer[size - 1] = '\0';
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define strjoin( buffer, ... ) CoreText::macro_strjoin( sizeof( buffer ), buffer, __VA_ARGS__, nullptr )
#define strjoin_path( buffer, ... ) CoreText::_filepath( sizeof( buffer ), buffer, __VA_ARGS__, nullptr );
#define strappend( buffer, string ) CoreText::macro_strappend( sizeof( buffer ), buffer, string )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////