#include <core/json.hpp>

#include <vendor/string.hpp>
#include <vendor/stdlib.hpp>

#include <core/debug.hpp>
#include <core/types.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

JSON::JSON( String &string )
{
	MemoryAssert( string.data != nullptr );

	// Skip UTF-8 BOM if present
	if( string.length_bytes() >= 3 &&
		static_cast<u8>( string[0] ) == 0xEF &&
		static_cast<u8>( string[1] ) == 0xBB &&
		static_cast<u8>( string[2] ) == 0xBF)
	{
		string.remove( 0, 3 );
	}

    // Strip Whitespace
    bool inQuotes = false;
    for( usize i = string.length_bytes(); i > 0; i-- )
    {
        if( string[i-1] == '"' && ( i <= 1 || string[i - 2] != '\\' ) ) { inQuotes = !inQuotes; continue; }
        if( !inQuotes && char_is_whitespace( string[i - 1] ) ) { string.remove( i - 1, 1 ); continue; }
    }

    this->start = 1;
    AssertMsg( string[0] == '{', "JSON has invalid root scope (no open {)\n\nJSON:\n%s", string.cstr() );
    this->end = string.length_bytes() - 1;
    AssertMsg( string[this->end] == '}', "JSON has invalid root scope (no closing })\n\nJSON:\n%s", string.cstr() );
    this->string = &string;
}


JSON JSON::object( const char *key )
{
	// Find Element
	JSONElement element = find_element_key( key );
	if( !element ) { return JSON { string, 0, 0 }; }

	// Get Object
	if( (*string)[element.start] != '{' ) { return JSON { string, 0, 0 }; }
	if( (*string)[element.end-1] != '}' ) { return JSON { string, 0, 0 }; }

	return JSON { string, element.start + 1, element.end - 1 };
}


JSON JSON::object_at( usize index )
{
	// Find Element
	JSONElement element = find_element_index( index );
	if( !element ) { return JSON { string, 0, 0 }; }

	// Get Object
	if( (*string)[element.start] != '{' ) { return JSON { string, 0, 0 }; }
	if( (*string)[element.end-1] != '}' ) { return JSON { string, 0, 0 }; }

	return JSON { string, element.start + 1, element.end - 1 };
}

JSON JSON::array( const char *key )
{
	// Find Element
	JSONElement element = find_element_key( key );
	if( !element ) { return JSON { string, 0, 0 }; }

	// Get Array
	if( (*string)[element.start] != '[' ) { return JSON { string, 0, 0 }; }
	if( (*string)[element.end-1] != ']' ) { return JSON { string, 0, 0 }; }
	return JSON { string, element.start + 1, element.end - 1 };
}


JSON JSON::array_at( usize index )
{
	// Find Element
	JSONElement element = find_element_index( index );
	if( !element ) { return JSON { string, 0, 0 }; }

	// Get Array
	if( (*string)[element.start] != '[' ) { return JSON { string, 0, 0 }; }
	if( (*string)[element.end-1] != ']' ) { return JSON { string, 0, 0 }; }
	return JSON { string, element.start + 1, element.end - 1 };
}


String JSON::get_string( const char *key, const char *defaultValue )
{
	// Find Element
	JSONElement element = find_element_key( key );
	if( !element ) { return defaultValue; }

	// Get String
	if( (*string)[element.start] != '"' ) { return defaultValue; }
	if( (*string)[element.end-1] != '"' ) { return defaultValue; }
	String out = string->substr( element.start + 1, element.end - 1 );
	return out.replace( "\\\"", "\"" ).replace( "\\n", "\n" ).replace( "\\t", "\t" );
}


String JSON::get_string_at( usize index, const char *defaultValue )
{
	// Find Element
	JSONElement element = find_element_index( index );
	if( !element ) { return defaultValue; }

	// Get String
	if( (*string)[element.start] != '"' ) { return defaultValue; }
	if( (*string)[element.end-1] != '"' ) { return defaultValue; }
	String out = string->substr( element.start + 1, element.end - 1 );
	return out.replace( "\\\"", "\"" ).replace( "\\n", "\n" ).replace( "\\t", "\t" );
}


double JSON::get_double( const char *key, const double defaultValue )
{
	// Find Element
	JSONElement element = find_element_key( key );
	if( !element ) { return defaultValue; }

	// Return Double
	return atof( string->substr( element.start, element.end ).cstr() );
}


double JSON::get_double_at( usize index, const double defaultValue )
{
	// Find Element
	JSONElement element = find_element_index( index );
	if( !element ) { return defaultValue; }

	// Return Double
	return atof( string->substr( element.start, element.end ).cstr() );
}


float JSON::get_float( const char *key, const float defaultValue )
{
	return static_cast<float>( get_double( key, static_cast<double>( defaultValue ) ) );
}


float JSON::get_float_at( usize index, const float defaultValue )
{
	return static_cast<float>( get_double_at( index, static_cast<double>( defaultValue ) ) );
}


int JSON::get_int( const char *key, const int defaultValue )
{
	// Find Element
	JSONElement element = find_element_key( key );
	if( !element ) { return defaultValue; }

	// Return Integer
	return atoi( string->substr( element.start, element.end ).cstr() );
}


int JSON::get_int_at( usize index, const int defaultValue )
{
	// Find Element
	JSONElement element = find_element_index( index );
	if( !element ) { return defaultValue; }

	// Return Integer
	return atoi( string->substr( element.start, element.end ).cstr() );
}


bool JSON::get_bool( const char *key, const bool defaultValue )
{
	// Find Element
	JSONElement element = find_element_key( key );
	if( !element ) { return defaultValue; }

	// Return Boolean
	return string->contains_at( "true", element.start ) || string->contains_at( "1", element.start );
}


bool JSON::get_bool_at( usize index, const bool defaultValue )
{
	// Find Element
	JSONElement element = find_element_index( index );
	if( !element ) { return defaultValue; }

	// Return Boolean
	return string->contains_at( "true", element.start ) || string->contains_at( "1", element.start );
}


usize JSON::count() const
{
	// Early out if empty
	if( end - start <= 1 ) { return 0; }

	bool inQuotes = false;
	int depthBraces = 0;
	int depthBrackets = 0;
	usize count = 1;
	for( usize i = start; i < end; i++ )
	{
		// Length check / null terminator
		if( i >= string->length_bytes() || (*string)[i] == '\0' ) { break; }

		// Skip quotes
		if( (*string)[i] == '"' && ( i == 0 || (*string)[i-1] != '\\' ) ) { inQuotes = !inQuotes; }
		if( inQuotes ) { continue; }

		// Process Braces/Brackets
		if( (*string)[i] == '{' ) { depthBraces++; continue; }
		if( (*string)[i] == '[' ) { depthBrackets++; continue; }
		if( (*string)[i] == '}' ) { if( depthBraces > 0 ) { depthBraces--; } else { break; } }
		if( (*string)[i] == ']' ) { if( depthBrackets > 0 ) { depthBrackets--; } else { break; } }
		if( depthBraces > 0 || depthBrackets > 0 ) { continue; }

		// Process ',' delimiter
		if( (*string)[i] == ',' ) { count++; }
	}

	// Failure
	return count;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static usize json_find_element_key( const String &buffer, const char *key, usize start = 0, usize end = USIZE_MAX )
{
	bool inQuotes = false;
	int depthBraces = 0;
	int depthBrackets = 0;

	for( usize i = start; i < end; i++ )
	{
		// Length check / null terminator
		if( i >= buffer.length_bytes() || buffer[i] == '\0' ) { break; }

		// Find Key / Skip Quotes
		if( depthBraces == 0 && depthBrackets == 0 )
		{
			if( buffer[i] == '"' && ( i == 0 || buffer[i-1] != '\\' ) )
			{
				if( inQuotes ) { inQuotes = false; continue; }
				if( buffer.contains_at( key, i + 1 ) && buffer.contains_at( "\":", i + 1 + strlen( key ) ) )
				{
					return i + strlen( key ) + 3;
				}
				inQuotes = true;
			}
			if( inQuotes ) { continue; }
		}

		// Process Braces/Brackets
		if( buffer[i] == '}' ) { if( depthBraces > 0 ) { depthBraces--; } else { break; } }
		if( buffer[i] == ']' ) { if( depthBrackets > 0 ) { depthBrackets--; } else { break; } }
		if( buffer[i] == '{' ) { depthBraces++; continue; }
		if( buffer[i] == '[' ) { depthBrackets++; continue; }
	}

	// Failure
	return USIZE_MAX;
}

static usize json_find_element_index( const String &buffer, usize index, usize start = 0, usize end = USIZE_MAX )
{
	bool inQuotes = false;
	int depthBraces = 0;
	int depthBrackets = 0;

	usize last = start;
	usize count = 0;
	for( usize i = start; i < end; i++ )
	{
		// Length check / null terminator
		if( i >= buffer.length_bytes() || buffer[i] == '\0' ) { break; }

		// Skip quotes
		if( buffer[i] == '"' && ( i == 0 || buffer[i-1] != '\\' ) ) { inQuotes = !inQuotes; }
		if( inQuotes ) { continue; }

		// Process Braces/Brackets
		if( buffer[i] == '{' ) { depthBraces++; continue; }
		if( buffer[i] == '[' ) { depthBrackets++; continue; }
		if( buffer[i] == '}' ) { if( depthBraces > 0 ) { depthBraces--; } else { break; } }
		if( buffer[i] == ']' ) { if( depthBrackets > 0 ) { depthBrackets--; } else { break; } }
		if( depthBraces > 0 || depthBrackets > 0 ) { continue; }

		// Process ',' delimiter
		if( buffer[i] == ',' )
		{
			if( count == index ) { return last; }
			last = i + 1;
			count++;
		}
	}

	// Failure
	return count == index ? last : USIZE_MAX;
}

static usize json_find_element_delimiter( const String &buffer, usize start = 0, usize end = USIZE_MAX )
{
	bool inQuotes = false;
	int depthBraces = 0;
	int depthBrackets = 0;

	for( usize i = start; i < end; i++ )
	{
		// Length check / null terminator
		if( i >= buffer.length_bytes() || buffer[i] == '\0' ) { return i; }

		// Skip quotes
		if( buffer[i] == '"' && ( i == 0 || buffer[i-1] != '\\' ) ) { inQuotes = !inQuotes; }
		if( inQuotes ) { continue; }

		// Process Braces/Brackets
		if( buffer[i] == '{' ) { depthBraces++; continue; }
		if( buffer[i] == '[' ) { depthBrackets++; continue; }
		if( buffer[i] == '}' ) { if( depthBraces > 0 ) { depthBraces--; } else { return i; } }
		if( buffer[i] == ']' ) { if( depthBrackets > 0 ) { depthBrackets--; } else { return i; } }
		if( depthBraces > 0 || depthBrackets > 0 ) { continue; }

		// Process ',' delimiter
		if( buffer[i] == ',' ) { return i; }
	}

	// Failure
	return end;
}


JSON::JSONElement JSON::find_element_key( const char *key )
{
	// Locate Start
	const usize indexStart = json_find_element_key( *string, key, start, end );
	if( indexStart > end ) { return JSONElement { 0, 0 }; }

	// Locate End
	const usize indexEnd = json_find_element_delimiter( *string, indexStart, end );
	if( indexEnd > end || indexEnd <= indexStart ) { return JSONElement { 0, 0 }; }

	return JSONElement { indexStart, indexEnd };
}


JSON::JSONElement JSON::find_element_index( usize index )
{
	const usize indexStart = json_find_element_index( *string, index, start, end );
	if( indexStart > end ) { return JSONElement { 0, 0 }; }
	const usize indexEnd = json_find_element_delimiter( *string, indexStart, end );
	if( indexEnd > end || indexEnd <= indexStart ) { return JSONElement { 0, 0 }; }
	return JSONElement { indexStart, indexEnd };
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////