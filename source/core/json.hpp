#pragma once

#include <core/debug.hpp>
#include <core/string.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Custom JSON reader; may not be perfect, but works for current needs
// TODO: Revisit this module

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class JSON
{
public:
	JSON( String &string );
	JSON( String *string, usize start = U64_MAX, usize end = U64_MAX ) :
		string { string }, start { start }, end { end  }
	{
		MemoryAssert( string != nullptr );
		MemoryAssert( string->data != nullptr );
	}

	JSON object( const char *key );
	JSON object_at( usize index );

	JSON array( const char *key );
	JSON array_at( usize index );

	String get_string( const char *key, const char *defaultValue = "" );
	String get_string_at( usize index, const char *defaultValue = "" );

	double get_double( const char *key, const double defaultValue = 0.0 );
	double get_double_at( usize index, const double defaultValue = 0.0 );

	float get_float( const char *key, const float defaultValue = 0.0f );
	float get_float_at( usize index, const float defaultValue = 0.0f );

	int get_int( const char *key, const int defaultValue = 0 );
	int get_int_at( usize index, const int defaultValue = 0 );

	bool get_bool( const char *key, const bool defaultValue = false );
	bool get_bool_at( usize index, const bool defaultValue = false );

	usize count() const;

	explicit operator bool() const { return start < end; }

private:
	struct JSONElement
	{
		JSONElement( usize start, usize end ) : start( start ), end( end ) { }
		usize start = 0;
		usize end = 0;
		explicit operator bool() const { return start < end; }
	};

	JSONElement find_element_key( const char *key );
	JSONElement find_element_index( usize index );

public:
	String *string;
	usize start;
	usize end;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////