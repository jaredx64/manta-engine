#pragma once

#include <core/types.hpp>
#include <core/debug.hpp>
#include <core/traits.hpp>

#include <vendor/string.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define TAB "    "
#define COMMENT_BREAK "////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace CoreText
{
	extern void macro_strjoin( const usize size, char *buffer, ... );
	extern void macro_strjoin_path( const usize size, char *buffer, ... );
	extern void macro_strappend( const usize size, char *buffer, const char *string );
}

#define strjoin( buffer, ... ) CoreText::macro_strjoin( sizeof( buffer ), buffer, __VA_ARGS__, nullptr )
#define strjoin_path( buffer, ... ) CoreText::macro_strjoin_path( sizeof( buffer ), buffer, __VA_ARGS__, nullptr );
#define strappend( buffer, string ) CoreText::macro_strappend( sizeof( buffer ), buffer, string )

extern char chrlower( char c );
extern char chrupper( char c );
extern void strlower( char *buffer );
extern void strupper( char *buffer );

extern bool streq( const char *str1, const char *str2 );
extern bool strneq( const char *str1, const char *str2, const usize size );
extern bool streq_case_insensitive( const char *str1, const char *str2 );
extern bool strneq_case_insensitive( const char *str1, const char *str2, const usize size );

extern bool char_whitespace( const char c );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class StringView
{
public:
	StringView() = default;
	StringView( const char *data ) : data{ data }, length{ strlen( data ) } { }
	StringView( const char *data, usize length ) : data{ data }, length{ length } { }

	StringView &set( const char *data );
	StringView &set( const char *data, usize length );

	u32 hash();
	char *cstr( char *buffer, const usize size );

public:
	const char *data = nullptr;
	usize length = 0;
};

namespace Hash
{
	inline extern u32 hash( const StringView stringView ) { return hash( stringView.data, stringView.length ); }

	inline bool equals( const StringView a, const StringView b )
	{
		return a.length == b.length && strncmp( a.data, b.data, a.length ) == 0;
	}

	inline bool is_null( const StringView a ) { return a.data == nullptr; }
	inline void set_null( StringView &a ) { a.data = nullptr; }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class String
{
public:
#if true // MEMORY_RAII
	String( const char *string = "" ) { init( string ); }
	String( const StringView &other ) { init( other.data, other.length ); }
	String( const String &other ) { copy( other ); }
	String( const String &other, const usize start, const usize end ) { copy_part( other, start, end ); }
	String( String &&other ) { move( static_cast<String &&>( other ) ); }
	~String() { free(); }

	String &operator=( const String &other ) { return copy( other ); }
	String &operator=( String &&other ) { return move( static_cast<String &&>( other ) ); }
#else
	String<T> &operator=( const String &other ) { Error( "String: assignment disabled" ); return *this; }
	String<T> &operator=( String &&other ) { Error( "String: assignment disabled" ); return *this; }

#if MEMORY_ASSERTS
	~String()
	{
		// Memory Leak Detection
		if( Debug::memoryLeakDetection && Debug::exitCode == 0 )
		{
			MemoryAssertMsg( data == nullptr, "ERROR: Memory leak in String (%p) (size: %.2f kb)",
			                 this, KB( /*size_allocated_bytes()*/capacity ) );
		}
	}
#endif
#endif
private:
	void grow();

public:
	void init( const char *string = "", const usize length = USIZE_MAX );
	void free();
	String &copy( const String &other );
	String &copy_part( const String &other, usize start, usize end );
	String &move( String &&other );

	bool save( const char *path );
	bool load( const char *path );

#if true || MEMORY_RAII
	String substr( const usize start = 0, const usize end = USIZE_MAX ) const;
#endif
	StringView view( const usize start = 0, const usize end = USIZE_MAX ) const;

	String &clear();
	String &trim();
	String &append( const char *string );
	String &append( const String &string );
	String &append( const StringView &string );
	String &append( char c );
	String &append( int integer );
	String &append( u32 integer );
	String &append( u64 integer );
	String &append( float number );
	String &append( double number );
	String &insert( const usize index, const char *string );
	String &remove( const usize index, const usize count );
	String &replace( const char *substr, const char *str, usize start = 0, usize end = USIZE_MAX );
	String &to_upper();
	String &to_lower();

	String operator+( const String &rhs ) const;
	String &operator+=( const String &rhs );

	u32 hash() const;
	usize length_bytes() const;
	usize length_codepoints() const;
	bool is_empty() const { return current == 0; }

	usize find( const char *substr, usize start = 0, usize end = USIZE_MAX ) const;
	bool contains( const char *substr ) const;
	bool contains_at( const char *substr, usize index ) const;

	bool equals( const char *string ) const;
	bool equals( const String &string ) const;
	bool equals( const StringView &string ) const;

	// C-style char strings
	char &char_at( const usize index );
	const char &char_at( const usize index ) const;
	const char *get_pointer( const usize index ) const;
	const char *cstr() const;
	operator const char *() const { return cstr(); }

	// Equality
	bool operator==( const char *string ) const { return equals( string ); }
	bool operator==( const String &string ) const { return equals( string ); }
	bool operator==( const StringView &string ) const { return equals( string ); }
	bool operator!=( const char *string ) const { return !equals( string ); }
	bool operator!=( const String &string ) const { return !equals( string ); }
	bool operator!=( const StringView &string ) const { return !equals( string ); }

	// Indexer
	char &operator[]( const usize index ) { return char_at( index ); }
	const char &operator[]( const usize index ) const { return char_at( index ); }

	// Null string check
	explicit operator bool() const { return data != nullptr && data[0] != '\0'; }

	// Forward iterator
	class forward_iterator
	{
	public:
		forward_iterator( char *element ) : element( element ) { }
		forward_iterator &operator++() { ++element; return *this; }
		bool operator!=( const forward_iterator &other ) const { return element != other.element; }
		char &operator*() { return *element; }
		const char &operator*() const { return *element; }
	private:
		char *element;
	};

	forward_iterator begin() { MemoryAssert( data != nullptr ); return forward_iterator( &data[0] ); }
	forward_iterator end() { MemoryAssert( data != nullptr ); return forward_iterator( &data[current] ); }
	forward_iterator begin() const { MemoryAssert( data != nullptr ); return forward_iterator( &data[0] ); }
	forward_iterator end() const { MemoryAssert( data != nullptr ); return forward_iterator( &data[current] ); }

	// Reverse iterator
	class reverse_iterator
	{
	public:
		reverse_iterator( char *element ) : element{ element } { }
		reverse_iterator &operator++() { element--; return *this; }
		bool operator!=( const reverse_iterator &other ) const { return element != other.element; }
		char &operator*() { return *element; }
		const char &operator*() const { return *element; }
	private:
		char *element;
	};

	reverse_iterator rbegin() { MemoryAssert( data != nullptr ); return reverse_iterator( &data[current] - 1 ); }
	reverse_iterator rend() { MemoryAssert( data != nullptr ); return reverse_iterator( &data[0] - 1 ); }
	reverse_iterator rbegin() const { MemoryAssert( data != nullptr ); return reverse_iterator( &data[current] - 1 ); }
	reverse_iterator rend() const { MemoryAssert( data != nullptr ); return reverse_iterator( &data[0] - 1 ); }

public:
	static usize write( class Buffer &buffer, const String &string );
	static void read( class Buffer &buffer, String &string );
	static void serialize( class Buffer &buffer, const String &string );
	static void deserialize( class Buffer &buffer, String &string );

public:
	char *data = nullptr;
	usize capacity = 0;
	usize current = 0;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// TODO: Stack-allocated String
template <usize N>
class StringBuffer
{
private:
	// ...
public:
	// ...
public:
	char data[N];
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////