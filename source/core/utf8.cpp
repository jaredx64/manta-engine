#include <core/utf8.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Maximum of 4 bytes per utf8 char, UTF8_BYTES + 1 for the null termiator
#define UTF8_BYTES ( 4 )
#define UTF8_BYTES_MAX ( UTF8_BYTES + 1 )

// 0xC0 = 11000000
// 0x80 = 10000000
#define UTF8_IS_CONTINUATION_BYTE( x ) ( ( ( x ) & 0xC0 ) == 0x80 )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

usize utf8_length_codepoints( const char *string )
{
	char c;
	usize i = 0;
	while( ( c = *string++ ) != '\0' ) { i += !UTF8_IS_CONTINUATION_BYTE( c ); }
	return i;
}


usize utf8_length_bytes( const char *string )
{
	char c;
	usize i = 0;
	while( ( c = *string++ ) != '\0' ) { i++; }
	return i;
}


usize utf8_substr_length_bytes( const char *string, usize start, usize end )
{
	usize i = 0;
	usize bytes = 0;
	bool counting = false;

	char c;
	while( ( c = *string++ ) != '\0' )
	{
		if( !UTF8_IS_CONTINUATION_BYTE( c ) )
		{
			if( i == start ) { counting = true; }
			if( i == end ) { break; }
			i++;
 		}

		// Count bytes
		if( counting ) { bytes++; }
	}

	return bytes;
}


u32 utf8_codepoint_at( const char *string, usize index )
{
	u32 state = UTF8_ACCEPT;
	u32 codepoint;
	usize i = 0;
	char c;

	while( ( c = *string++ ) != '\0' )
	{
		if( utf8_decode( &state, &codepoint, c ) != UTF8_ACCEPT ) { continue; }
		if( i == index ) { return codepoint; }
		i++;
	}

	// Index out of bounds?
	return 0;
}


int utf8_codepoint_size( u32 codepoint )
{
	if( codepoint <= 0x7F ) { return 1; }
	if( codepoint <= 0x07FF ) { return 2; }
	if( codepoint <= 0xFFFF ) { return 3; }
	if( codepoint <= 0x10FFFF ) { return 4; }
	return 0;
}


int utf8_encode( char *buffer, u32 codepoint )
{
	if( codepoint <= 0x7F )
	{
		// ASCII
		buffer[0] = static_cast<char>( codepoint );
		return 1;
	}
	else if( codepoint <= 0x07FF )
	{
		// 2-bytes
		buffer[0] = static_cast<char>( ( ( codepoint >> 6 ) & 0x1F ) | 0xC0 );
		buffer[1] = static_cast<char>( ( ( codepoint )      & 0x3F ) | 0x80 );
		return 2;
	}
	else if( codepoint <= 0xFFFF )
	{
		// 3-bytes
		buffer[0] = static_cast<char>( ( ( codepoint >> 12 ) & 0x0F ) | 0xE0 );
		buffer[1] = static_cast<char>( ( ( codepoint >>  6 ) & 0x3F ) | 0x80 );
		buffer[2] = static_cast<char>( ( ( codepoint )       & 0x3F ) | 0x80 );
		return 3;
	}
	else if( codepoint <= 0x10FFFF )
	{
		// 4-bytes
		buffer[0] = static_cast<char>( ( ( codepoint >> 18 ) & 0x07 ) | 0xF0 );
		buffer[1] = static_cast<char>( ( ( codepoint >> 12 ) & 0x3F ) | 0x80 );
		buffer[2] = static_cast<char>( ( ( codepoint >>  6 ) & 0x3F ) | 0x80 );
		buffer[3] = static_cast<char>( ( ( codepoint )       & 0x3F ) | 0x80 );
		return 4;
	}

	return 0;
}


u32 utf8_decode( u32 *state, u32 *codepoint, char byte )
{
    // Bjoern Hoehrmann's UTF-8 decoder:
    // http://bjoern.hoehrmann.de/utf-8/decoder/dfa/

    static const u8 table[]
    {
        // The first part of the table maps bytes to character classes
        // to reduce the size of the transition table and create bitmasks.
         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
         1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
         7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
         8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
        10,3,3,3,3,3,3,3,3,3,3,3,3,4,3,3, 11,6,6,6,5,8,8,8,8,8,8,8,8,8,8,8,

        // The second part is a transition table that maps a combination
        // of a state of the automaton and a character class to a state.
         0,12,24,36,60,96,84,12,12,12,48,72, 12,12,12,12,12,12,12,12,12,12,12,12,
        12, 0,12,12,12,12,12, 0,12, 0,12,12, 12,24,12,12,12,12,12,24,12,24,12,12,
        12,12,12,12,12,12,12,24,12,12,12,12, 12,24,12,12,12,12,12,12,12,24,12,12,
        12,12,12,12,12,12,12,36,12,36,12,12, 12,36,12,12,12,12,12,36,12,36,12,12,
        12,36,12,12,12,12,12,12,12,12,12,12,
    };

    u32 type = table[static_cast<u8>( byte )];

    *codepoint = ( *state != UTF8_ACCEPT ) ?
	             ( byte  & 0x3F ) | ( *codepoint << 6 ) :
	             ( 0xFF >> type ) & ( byte );

    return *state = table[256 + *state + type];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////