#include <core/string.hpp>

#include <vendor/string.hpp>
#include <vendor/stdarg.hpp>
#include <vendor/stdio.hpp>

#include <core/memory.hpp>
#include <core/buffer.hpp>
#include <core/utf8.hpp>

#if COMPILE_BUILD
#include <build/filesystem.hpp>
#elif COMPILE_ENGINE
#include <manta/filesystem.hpp>
#include <manta/input.hpp>
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CoreText::macro_strjoin( usize size, char *buffer, ... )
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


void CoreText::macro_strjoin_path( usize size, char *buffer, ... )
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


void CoreText::macro_strappend( usize size, char *buffer, const char *string )
{
	if( buffer == nullptr || string == nullptr || size == 0 ) { return; }
	const usize bufferLength = strlen( buffer );
	strncat( buffer, string, size - bufferLength - 1 );
	buffer[size - 1] = '\0';
}


char chrupper( char c )
{
	return ( c >= 'a' && c <= 'z' ) ? ( c & 0xDF ) : c;
}


char chrlower( char c )
{
	return ( c >= 'A' && c <= 'Z' ) ? ( c | 0x20 ) : c;
}


void strlower( char *buffer )
{
	while( *buffer != '\0' ) { *buffer = chrlower( *buffer ); buffer++; }
}


void strupper( char *buffer )
{
	while( *buffer != '\0' ) { *buffer = chrupper( *buffer ); buffer++; }
}


bool streq( const char *str1, const char *str2 )
{
	while( *str1 != '\0' && *str2 != '\0' )
	{
		if( *str1 != *str2 ) { return false; }
		str1++;
		str2++;
	}
	return *str1 == *str2;
}


bool strneq( const char *str1, const char *str2, usize size )
{
	for( usize i = 0; i < size; i++ )
	{
		if( str1[i] != str2[i] ) { return false; }
		if( str1[i] == '\0' ) { break; }
	}
	return true;
}


bool streq_case_insensitive( const char *str1, const char *str2 )
{
	while( *str1 != '\0' && *str2 != '\0' )
	{
		if( chrlower( *str1 ) != chrlower( *str2 ) ) { return false; }
		str1++;
		str2++;
	}
	return *str1 == *str2;
}


bool strneq_case_insensitive( const char *str1, const char *str2, usize size )
{
	for( usize i = 0; i < size; i++ )
	{
		if( chrlower( str1[i] ) != chrlower( str2[i] ) ) { return false; }
		if( str1[i] == '\0' ) { break; }
	}
	return true;
}


bool strstr_case_insensitive( const char *str1, const char *str2 )
{
	if( !*str2 ) return true;

	for( ; *str1; ++str1 )
	{
		const char* h = str1;
		const char* n = str2;

		while( *h && *n && chrlower( *h ) == chrlower( *n ) )
		{
			++h;
			++n;
		}

		if( !( *n ) ) { return true; }
	}

	return false;
}


bool strcontains( const char *str, const char *substr )
{
	if( *substr == '\0' ) { return true; }

	for( ; *str != '\0'; str++ )
	{
		const char *h = str;
		const char *n = substr;

		while( *h != '\0' && *n != '\0' && *h == *n )
		{
			h++;
			n++;
		}

		if( *n == '\0' )
		{
			return true;
		}
	}

	return false;
}


bool strcontains_case_insensitive( const char *str, const char *substr )
{
	if( *substr == '\0' ) { return true; }

	for( ; *str != '\0'; str++ )
	{
		const char *h = str;
		const char *n = substr;

		while( *h != '\0' && *n != '\0' && chrlower( *h ) == chrlower( *n ) )
		{
			h++;
			n++;
		}

		if( *n == '\0' )
		{
			return true;
		}
	}

	return false;
}


bool char_is_whitespace( const char c )
{
	return c == ' ' || c == '\n' || c == '\r' || c == '\t';
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

StringView &StringView::set( const char *string )
{
	this->data = string;
	this->length = strlen( string );
	return *this;
}


StringView &StringView::set( const char *string, usize length )
{
	this->data = string;
	this->length = length;
	return *this;
}


char *StringView::cstr( char *buffer, usize size )
{
	MemoryAssert( buffer != nullptr );
	Assert( size > length );
	memory_copy( buffer, data, length );
	buffer[length] = '\0';
	return buffer;
}


u32 StringView::hash()
{
	MemoryAssert( data != nullptr );
	return Hash::hash( data, length );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void String::grow()
{
	MemoryAssert( data != nullptr );
	if( capacity == 0 ) { capacity = 1; } else { capacity = capacity > USIZE_MAX / 2 ? USIZE_MAX : capacity * 2; }

	data = reinterpret_cast<char *>( memory_realloc( data, capacity + 1 ) );
	ErrorIf( data == nullptr, "Failed to allocate memory for grow String (%p: alloc %d bytes)", data, capacity + 1 );
	data[capacity] = '\0';
}


void String::init( const char *string, usize length )
{
	capacity = length == USIZE_MAX ? strlen( string ) : length;
	current = capacity;

	MemoryAssert( data == nullptr );
	data = reinterpret_cast<char *>( memory_alloc( capacity + 1 ) );
	memory_copy( data, string, current );
	data[current] = '\0';
}


void String::free()
{
	if( data == nullptr )
	{
	#if true // MEMORY_RAII
		return;
	#else
		MemoryWarning( "String: attempting to free() string that is already freed!" ); return;
	#endif
	}

	memory_free( data );
	data = nullptr;

	capacity = 0LLU;
	current = 0LLU;
}


bool String::save( const char *path )
{
	Assert( data != nullptr );
	FILE *file = fopen( path, "wb" );
	if( file == nullptr ) { return false; }

	bool returnCode = true;
	if( fwrite( data, current, 1, file ) < 1 ) { returnCode = false; goto cleanup; }

cleanup:
	if( fclose( file ) != 0 ) { return false; }
	return returnCode;
}


bool String::load( const char *path )
{
	Assert( data != nullptr );
	FILE *file = fopen( path, "rb" );
	if( file == nullptr ) { return false; }

	bool returnCode = true;
	const usize size = fsize( file );
	if( size == 0 ) { returnCode = false; goto cleanup; }

	capacity = size + 1;
	current = size;
	data = reinterpret_cast<char *>( memory_realloc( data, capacity ) );
	ErrorIf( data == nullptr,
		"Failed to allocate memory for load String (%p: alloc %d bytes)", data, capacity );

	if( fread( data, size, 1, file ) < 1 ) { returnCode = false; goto cleanup; }
	data[size] = '\0';

cleanup:
	if( fclose( file ) != 0 ) { return false; }
	return returnCode;
}


String &String::copy( const String &other )
{
	MemoryAssert( other.data != nullptr );
	if( this == &other ) { return *this; }
	if( data != nullptr ) { free(); }

	capacity = other.capacity;
	current = other.current;

	MemoryAssert( data == nullptr );
	data = reinterpret_cast<char *>( memory_alloc( capacity + 1 ) );
	memory_copy( data, other.data, current );
	data[current] = '\0';

	return *this;
}


String &String::copy_part( const String &other, usize start, usize end )
{
	MemoryAssert( other.data != nullptr );
	if( this == &other ) { return *this; }
	if( data != nullptr ) { free(); }

	Assert( start <= end && start <= other.current );
	if( end == USIZE_MAX ) { end = other.current; }
	Assert( end <= other.current );
	capacity = end - start;
	current = capacity;

	MemoryAssert( data == nullptr );
	data = reinterpret_cast<char *>( memory_alloc( capacity + 1 ) );
	memory_copy( data, other.data + start, current );
	data[current] = '\0';

	return *this;
}


String &String::move( String &&other )
{
	MemoryAssert( other.data != nullptr );
	if( this == &other ) { return *this; }
	if( data != nullptr ) { free(); }

	data = other.data;
	capacity = other.capacity;
	current = other.current;

	other.data = nullptr;
	other.capacity = 0LLU;
	other.current = 0LLU;

	return *this;
}


String &String::clear()
{
	MemoryAssert( data != nullptr );
	data[0] = '\0';
	current = 0LLU;
	return *this;
}


#if true || MEMORY_RAII
String String::substr( usize start, usize end ) const
{
	MemoryAssert( data != nullptr );
	Assert( start <= end );
	return String { *this, start, end };
}
#endif


StringView String::view( usize start, usize end ) const
{
	MemoryAssert( data != nullptr );
	Assert( start <= end && start <= current );
	Assert( end == USIZE_MAX || end <= current );
	const usize length = ( end == USIZE_MAX ? current : end ) - start;
	return StringView { &data[start], length };
}


String &String::trim()
{
	MemoryAssert( data != nullptr );
	if( current == 0 ) { return *this; }

	usize leadingSpaces = 0;
	while( leadingSpaces < current && char_is_whitespace( data[leadingSpaces] ) ) { leadingSpaces++; }
	usize trailingSpaces = 0;
	while( trailingSpaces < current && char_is_whitespace(data[current - trailingSpaces - 1] ) ) { trailingSpaces++; }

	if( leadingSpaces + trailingSpaces >= current )
	{
		// String is entirely whitespace, return empty string
		data[0] = '\0';
		current = 0LLU;
	}
	else if( leadingSpaces > 0 || trailingSpaces > 0 )
	{
		// There are leading or trailing spaces to trim
		current -= ( leadingSpaces + trailingSpaces );
		for( usize i = 0; i < current; i++ ) { data[i] = data[i + leadingSpaces]; }
		data[current] = '\0';
	}

	return *this;
}


String &String::append( const char *string )
{
	MemoryAssert( data != nullptr );
	if( string == nullptr ) { return *this; }
	const usize length = strlen( string );
	for( ; current + length > capacity; grow() ) { }
	memory_copy( data + current, string, length );
	current += length;
	data[current] = '\0';
	return *this;
}


String &String::append( const String &string )
{
	return append( string.cstr() );
}


String &String::append( const StringView &string )
{
	MemoryAssert( data != nullptr );
	if( string.data == nullptr || string.length == 0 || string.data[0] == '\0' ) { return *this; }
	for( ; current + string.length > capacity; grow() ) { }
	memory_copy( data + current, string.data, string.length );
	current += string.length;
	data[current] = '\0';
	return *this;
}


String &String::append( char c )
{
	MemoryAssert( data != nullptr );
	for( ; current + 1 > capacity; grow() ) { }
	data[current++] = c;
	data[current] = '\0';
	return *this;
}


String &String::append( int integer )
{
	char buffer[32];
	snprintf( buffer, 32, "%d", integer );
	return append( buffer );
}


String &String::append( u32 integer )
{
	char buffer[32];
	snprintf( buffer, 32, "%u", integer );
	return append( buffer );
}



String &String::append( u64 integer )
{
	char buffer[32];
	snprintf( buffer, 32, "%llu", integer );
	return append( buffer );
}


String &String::append( float number )
{
	char buffer[32];
	snprintf( buffer, 32, "%f", number );
	return append( buffer );
}


String &String::append( double number )
{
	char buffer[32];
	snprintf( buffer, 32, "%f", number );
	return append( buffer );
}


String &String::insert( usize index, const char *string )
{
	MemoryAssert( data != nullptr );
	if( string == nullptr || string[0] == '\0' ) { return *this; }

	// Grow data (if necessary)
	Assert( index <= current );
	const usize length = strlen( string );
	for( ; current + length > capacity; grow() ) { }

	// Move chars after index to the right & insert string
	const usize shift = current - index;
	memory_move( &data[index + length], &data[index], shift * sizeof( char ) );
	memory_copy( &data[index], string, length );
	current += length;
	data[current] = '\0';
	return *this;
}


String &String::remove( usize index, usize count )
{
	MemoryAssert( data != nullptr );
	Assert( index < current && index + count <= current );

	// Shift the right side of the string over
	const usize shift = current - ( index + count );
	memory_move( &data[index], &data[index + count], shift * sizeof( char ) );
	current -= count;
	data[current] = '\0';
	return *this;
}


String &String::replace( const char *substr, const char *str, usize start, usize end )
{
	MemoryAssert( data != nullptr );
	Assert( start <= end && start <= current );
	if( end == USIZE_MAX ) { end = current; }
	Assert( end <= current );

	// Empty strings?
	if( substr == nullptr || substr[0] == '\0' || str == nullptr)
	{
		return *this;
	}

	const usize lenSubstr = strlen( substr );
	const usize lenString = strlen( str );
	const i64 lenDiff = ( lenString - lenSubstr );

	// Replace substrings
	usize index = find( substr, start, end );
	while( index != USIZE_MAX && index + lenSubstr <= end )
	{
		remove( index, lenSubstr );
		insert( index, str );

		// Find next
		start = index + lenString;
		end += lenDiff;
		index = find( substr, start, end );
	}
	return *this;
}


String &String::to_upper()
{
	MemoryAssert( data != nullptr );
	for( usize i = 0; i < current && data[i] != '\0'; i++ )
	{
		data[i] = ( data[i] >= 'a' && data[i] <= 'z' ) ? ( data[i] & 0xDF ) : data[i];
	}
	return *this;
}


String &String::to_lower()
{
	MemoryAssert( data != nullptr );
	for( usize i = 0; i < current && data[i] != '\0'; i++ )
	{
		data[i] = ( data[i] >= 'A' && data[i] <= 'Z' ) ? ( data[i] | 0x20 ) : data[i];
	}
	return *this;
}


String String::operator+( const String &rhs ) const
{
	MemoryAssert( data != nullptr );
	String result { cstr() }; // TODO: copy constructor
	result.append( rhs.cstr() );
	return result;
}


String &String::operator+=( const String &rhs )
{
	return append( rhs.cstr() );
}


u32 String::hash() const
{
	MemoryAssert( data != nullptr );
	data[current] = '\0';
	return Hash::hash( data );
}


usize String::length_bytes() const
{
	return current;
}


usize String::length_codepoints() const
{
	data[current] = '\0';
	return utf8_length_codepoints( data );
}


usize String::find( const char *substr, usize start, usize end ) const
{
	MemoryAssert( data != nullptr );
	if( start >= current || start >= end ) { return USIZE_MAX; }
	if( end == USIZE_MAX ) { end = current; }
	Assert( end <= current );
	if( substr == nullptr || substr[0] == '\0' ) { return USIZE_MAX; }

	const usize length = strlen( substr );
	end = current - length < end ? current - length + 1 : end;
	for( usize i = start; i < end; i++ )
	{
		bool found = true;
		for( usize j = 0; substr[j] != '\0'; j++ )
		{
			if( data[i + j] != substr[j] )
			{
				found = false;
				break;
			}
		}

		if( found ) { return i; }
	}

	return USIZE_MAX;
}


bool String::contains( const char *substr ) const
{
	return find( substr, 0, current ) != USIZE_MAX;
}


bool String::contains_at( const char *substr, usize index ) const
{
	MemoryAssert( data != nullptr );
	if( index >= current ) { return false; }
	if( substr == nullptr || substr[0] == '\0' ) { return false; }

	const usize length = strlen( substr );
	if( index + length > current ) { return false; }
	return strncmp( &data[index], substr, length ) == 0;
}


bool String::equals( const char *string ) const
{
	MemoryAssert( data != nullptr );
	data[current] = '\0';
	return strcmp( data, string ) == 0;
}


bool String::equals( const String &string ) const
{
	MemoryAssert( data != nullptr && string.data != nullptr );
	if( current != string.current ) { return false; }
	return strncmp( data, string.data, current ) == 0;
}


bool String::equals( const StringView &string ) const
{
	MemoryAssert( data != nullptr && string.data != nullptr );
	if( current != string.length ) { return false; }
	return strncmp( data, string.data, string.length ) == 0;
}


const char *String::get_pointer( usize index ) const
{
	MemoryAssert( data != nullptr );
	Assert( index <= current );
	return &data[index];
}


const char &String::char_at( usize index ) const
{
	MemoryAssert( data != nullptr );
	Assert( index <= current );
	return data[index];
}


const char *String::cstr() const
{
	MemoryAssert( data != nullptr );
	data[current] = '\0';
	return data;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

usize String::write( Buffer &buffer, const String &string )
{
	MemoryAssert( string.data != nullptr );
	const usize offset = buffer.write<usize>( string.current + 1 );
	buffer.write( string.data, string.current + 1 );
	return offset;
}


bool String::read( Buffer &buffer, String &string )
{
	if( string.data != nullptr ) { string.free(); }

	buffer.read<usize>( string.capacity );
	ErrorIf( string.capacity == 0, "Failed to Buffer read String: invalid capacity" );
	string.capacity -= 1;
	string.current = string.capacity;

	MemoryAssert( string.data == nullptr );
	string.data = reinterpret_cast<char *>( memory_alloc( string.capacity + 1 ) );
	memory_copy( string.data, buffer.read_bytes( string.capacity ), string.capacity );
	string.data[string.current] = '\0';
	return true;
}


void String::serialize( Buffer &buffer, const String &string )
{
	buffer.write<String>( string );
}


bool String::deserialize( Buffer &buffer, String &string )
{
	return buffer.read<String>( string );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////