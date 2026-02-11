#include <build/objects.hpp>

#include <build/build.hpp>
#include <build/system.hpp>

#include <core/hashmap.hpp>
#include <core/math.hpp>
#include <core/checksum.hpp>

#include <build/assets.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct CacheObject { usize dummy = 0; };
struct CacheObjects { usize fileCount = 0LLU; };

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ObjectFile::ERROR_HANDLER_FUNCTION_DECL
{
	// Early-out if we've already errored
	if( Debug::exitCode != 0 ) { return; }

	// Print Label
	Print( PrintColor_Red, "\n\nOBJECT FILE ERROR:\n\n" );

	// Print Message
	va_list args;
	va_start( args, message );
	Print( PrintColor_White, "    " );
	Debug::print_formatted_variadic_color( true, PrintColor_White, message, args );
	va_end( args );

	// Print File
	Print( PrintColor_Red, "    %s\n", name.cstr() );
	if( line >= 0 )
	{
		Print( PrintColor_Red, "    %s:%d\n", path.cstr(), line );
		Print( PrintColor_Red, "    line: %d\n", line );
	}
	else
	{
		Print( PrintColor_Red, "    %s:", path.cstr() );
	}
	Print( PrintColor_Red, "\n" );

	// Exit
	Debug::exit( 1 );
}


void Objects::ERROR_HANDLER_FUNCTION_DECL
{
	// Early-out if we've already errored
	if( Debug::exitCode != 0 ) { return; }

	// Print Label
	Print( PrintColor_Red, "\n\nOBJECT ERROR:\n\n" );

	// Print Message
	va_list args;
	va_start( args, message );
	Print( PrintColor_White, "    " );
	Debug::print_formatted_variadic_color( true, PrintColor_Red, message, args );
	va_end( args );

	// Exit
	Debug::exit( 1 );
}


#define ErrorLine( line, message, ... ) \
	ERROR_HANDLER_FUNCTION( "Error", __FILE__, __FUNCTION__, line, "", message, ##__VA_ARGS__ );

#define ErrorIfLine( condition, line, message, ... ) \
	if( condition ) \
	{ \
		ERROR_HANDLER_FUNCTION( "ErrorIf", __FILE__, __FUNCTION__, line, #condition, message, ##__VA_ARGS__ ); \
	}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static const char *s_DEFAULT_DEFINITION =
R"(
OBJECT( DEFAULT )
ABSTRACT( true )

PUBLIC ObjectInstance id;

EVENT_CREATE MANUAL
{
	/* do nothing */
}

EVENT_DESTROY MANUAL
{
	/* do nothing */
}
)";

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const char *g_KEYWORDS[] =
{
// Events
	"EVENT_CREATE",          // KeywordID_EVENT_CREATE
	"EVENT_DESTROY",         // KeywordID_EVENT_DESTROY
	"EVENT_INITIALIZE",      // KeywordID_EVENT_INITIALIZE

	"EVENT_FRAME_START",     // KeywordID_EVENT_FRAME_START
	"EVENT_FRAME_END",       // KeywordID_EVENT_FRAME_END

	"EVENT_UPDATE_CUSTOM",   // KeywordID_EVENT_UPDATE_CUSTOM
	"EVENT_UPDATE_GUI",      // KeywordID_EVENT_UPDATE_GUI
	"EVENT_UPDATE",          // KeywordID_EVENT_UPDATE

	"EVENT_RENDER_CUSTOM",   // KeywordID_EVENT_RENDER_CUSTOM
	"EVENT_RENDER_GUI",      // KeywordID_EVENT_RENDER_GUI
	"EVENT_RENDER",          // KeywordID_EVENT_RENDER

	"EVENT_CUSTOM",          // KeywordID_EVENT_CUSTOM
	"EVENT_PREPARE",         // KeywordID_EVENT_PREPARE
	"EVENT_TEST",            // KeywordID_EVENT_TEST

	"EVENT_SLEEP",           // KeywordID_EVENT_SLEEP
	"EVENT_WAKE",            // KeywordID_EVENT_WAKE

	"EVENT_FLAG",            // KeywordID_EVENT_FLAG
	"EVENT_PARTITION",       // KeywordID_EVENT_PARTITION

	"EVENT_NETWORK_SEND",    // KeywordID_EVENT_NETWORK_SEND
	"EVENT_NETWORK_RECEIVE", // KeywordID_EVENT_NETWORK_RECEIVE

// Keywords
	"INCLUDES",              // KeywordID_INCLUDES
	"HEADER_INCLUDES",       // KeywordID_HEADER_INCLUDES
	"SOURCE_INCLUDES",       // KeywordID_SOURCE_INCLUDES
	"OBJECT",                // KeywordID_OBJECT
	"PARENT",                // KeywordID_PARENT
	"COUNT",                 // KeywordID_COUNT
	"BUCKET_SIZE",           // KeywordID_BUCKET_SIZE
	"HASH",                  // KeywordID_HASH
	"CATEGORY",              // KeywordID_CATEGORY
	"VERSIONS",              // KeywordID_VERSIONS
	"ABSTRACT",              // KeywordID_ABSTRACT
	"NETWORKED",             // KeywordID_NETWORKED
	"CONSTRUCTOR",           // KeywordID_CONSTRUCTOR
	"WRITE",                 // KeywordID_WRITE
	"READ",                  // KeywordID_READ
	"SERIALIZE",             // KeywordID_SERIALIZE
	"DESERIALIZE",           // KeywordID_DESERIALIZE
	"PRIVATE",               // KeywordID_PRIVATE
	"PROTECTED",             // KeywordID_PROTECTED
	"PUBLIC",                // KeywordID_PUBLIC
	"GLOBAL",                // KeywordID_GLOBAL
	"FRIEND",                // KeywordID_FRIEND
};
static_assert( ARRAY_LENGTH( g_KEYWORDS ) == KEYWORD_COUNT, "Verify g_KEYWORDS matches KeywordID enum" );

int g_KEYWORD_COUNT[KEYWORD_COUNT];

KeywordRequirements g_KEYWORD_REQUIREMENTS[] =
{
// Events
//  { Required, Max Count }
	{ false,    1 }, // KeywordID_EVENT_CREATE
	{ false,    1 }, // KeywordID_EVENT_DESTROY
	{ false,    1 }, // KeywordID_EVENT_INITIALIZE

	{ false,    1 }, // KeywordID_EVENT_FRAME_START
	{ false,    1 }, // KeywordID_EVENT_FRAME_END

	{ false,    1 }, // KeywordID_EVENT_UPDATE_CUSTOM
	{ false,    1 }, // KeywordID_EVENT_UPDATE_GUI
	{ false,    1 }, // KeywordID_EVENT_UPDATE

	{ false,    1 }, // KeywordID_EVENT_RENDER_CUSTOM
	{ false,    1 }, // KeywordID_EVENT_RENDER_GUI
	{ false,    1 }, // KeywordID_EVENT_RENDER

	{ false,    1 }, // KeywordID_EVENT_CUSTOM
	{ false,    1 }, // KeywordID_EVENT_PREPARE
	{ false,    1 }, // KeywordID_EVENT_TEST

	{ false,    1 }, // KeywordID_EVENT_SLEEP
	{ false,    1 }, // KeywordID_EVENT_WAKE

	{ false,    1 }, // KeywordID_EVENT_FLAG
	{ false,    1 }, // KeywordID_EVENT_PARTITION

	{ false,    1 }, // KeywordID_EVENT_NETWORK_SEND
	{ false,    1 }, // KeywordID_EVENT_NETWORK_RECEIVE

// Keywords
//  { Required, Max Count }
	{ false,   -1 }, // KeywordID_INCLUDES
	{ false,   -1 }, // KeywordID_HEADER_INCLUDES
	{ false,   -1 }, // KeywordID_SOURCE_INCLUDES
	{  true,    1 }, // KeywordID_OBJECT
	{ false,    1 }, // KeywordID_PARENT
	{ false,    1 }, // KeywordID_COUNT
	{ false,    1 }, // KeywordID_BUCKET_SIZE
	{ false,    1 }, // KeywordID_HASH
	{ false,   -1 }, // KeywordID_CATEGORY
	{ false,    1 }, // KeywordID_VERSIONS
	{ false,    1 }, // KeywordID_ABSTRACT
	{ false,    1 }, // KeywordID_NETWORKED
	{ false,   -1 }, // KeywordID_CONSTRUCTOR
	{ false,    1 }, // KeywordID_WRITE
	{ false,    1 }, // KeywordID_READ
	{ false,    1 }, // KeywordID_SERIALIZE
	{ false,    1 }, // KeywordID_DESERIALIZE
	{ false,   -1 }, // KeywordID_PRIVATE
	{ false,   -1 }, // KeywordID_PROTECTED
	{ false,   -1 }, // KeywordID_PUBLIC
	{ false,   -1 }, // KeywordID_GLOBAL
	{ false,   -1 }, // KeywordID_FRIEND
};
static_assert( ARRAY_LENGTH( g_KEYWORD_REQUIREMENTS ) == KEYWORD_COUNT, "Verify g_KEYWORD_REQUIREMENTS matches KeywordID enum" );

const char *g_EVENT_FUNCTIONS[][EVENT_FUNCTION_DEFINITION_COUNT] =
{
//  { EventFunction_Name       EventFunction_ReturnType  EventFunction_ReturnValue  EventFunction_Parameters  EventFunction_ParametersCaller
	{ "event_create",          "void",                   "",                        "()",                     "()"         }, // KeywordID_EVENT_CREATE
	{ "event_destroy",         "void",                   "",                        "()",                     "()"         }, // KeywordID_EVENT_DESTROY
	{ "event_initialize",      "void",                   "",                        "()",                     "()"         }, // KeywordID_EVENT_INITIALIZE

	{ "event_frame_start",     "void",                   "",                        "( const Delta delta )",  "( delta )"  }, // KeywordID_EVENT_FRAME_START
	{ "event_frame_end",       "void",                   "",                        "( const Delta delta )",  "( delta )"  }, // KeywordID_EVENT_FRAME_END

	{ "event_update_custom",   "void",                   "",                        "( const Delta delta )",  "( delta )"  }, // KeywordID_EVENT_UPDATE_CUSTOM
	{ "event_update_gui",      "void",                   "",                        "( const Delta delta )",  "( delta )"  }, // KeywordID_EVENT_UPDATE_GUI
	{ "event_update",          "void",                   "",                        "( const Delta delta )",  "( delta )"  }, // KeywordID_EVENT_UPDATE

	{ "event_render_custom",   "void",                   "",                        "( const Delta delta )",  "( delta )"  }, // KeywordID_EVENT_RENDER_CUSTOM
	{ "event_render_gui",      "void",                   "",                        "( const Delta delta )",  "( delta )"  }, // KeywordID_EVENT_RENDER_GUI
	{ "event_render",          "void",                   "",                        "( const Delta delta )",  "( delta )"  }, // KeywordID_EVENT_RENDER

	{ "event_custom",          "void",                   "",                        "( const Delta delta )",  "( delta )"  }, // KeywordID_EVENT_CUSTOM
	{ "event_prepare",         "void",                   "",                        "()",                     "()"         }, // KeywordID_EVENT_PREPARE
	{ "event_test",            "bool",                   "",                        "()",                     "()"         }, // KeywordID_EVENT_TEST

	{ "event_sleep",           "void",                   "",                        "( const Delta delta )",  "( delta )"  }, // KeywordID_EVENT_SLEEP
	{ "event_wake",            "void",                   "",                        "( const Delta delta )",  "( delta )"  }, // KeywordID_EVENT_WAKE
	{ "event_flag",            "void",                   "",                        "( const u64 code )",     "( code )"   }, // KeywordID_EVENT_FLAG

	{ "event_partition",       "void",                   "",                        "( void *ptr )",          "( ptr )"    }, // KeywordID_EVENT_PARTITION

	{ "event_network_send",    "bool",                   "",                        "( Buffer &buffer )",     "( buffer )" }, // KeywordID_EVENT_NETWORK_SEND
	{ "event_network_receive", "bool",                   "",                        "( Buffer &buffer )",     "( buffer )" }, // KeywordID_EVENT_NETWORK_RECEIVE
};
static_assert( ARRAY_LENGTH( g_EVENT_FUNCTIONS ) == EVENT_COUNT, "Verify g_EVENT_FUNCTIONS matches EventID enum" );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static usize find_closing_brace( const String &buffer, usize start = 0, usize end = USIZE_MAX )
{
	// Nested scope traversal
	int depth = 0;
	for( ;; )
	{
		const usize nextOpen = buffer.find( "{", start, end );
		const usize nextClose = buffer.find( "}", start, end );
		if( nextOpen == nextClose ) { return USIZE_MAX; }

		if( nextOpen < nextClose )
		{
			depth++;
			start = nextOpen + 1;
		}
		else
		{
			depth--;
			if( depth <= -1 ) { return USIZE_MAX; }
			if( depth == 0 ) { return nextClose; }
			start = nextClose + 1;
		}
	}

	// Failure
	return USIZE_MAX;
}


static bool find_keyword_parentheses( const String &buffer, usize start, usize end,
	usize &parenOpen, usize &parenClose )
{
	parenOpen = buffer.find( "(", start, end );
	if( parenOpen > end ) { return false; }
	parenClose = buffer.find( ")", start, end );
	if( parenClose > end ) { return false; }
	if( parenOpen + 1 >= parenClose ) {  return false; }
	return buffer.substr( parenOpen + 1, parenClose ).trim().length_bytes() > 0;
}


static bool char_is_keyword_delimiter( char c )
{
	return !( ( c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' );
}


static usize line_at( const String &buffer, const usize position )
{
	usize line = 1;
	usize current = buffer.find( "\n", 0, position );
	while( current < position ) { line++; current = buffer.find( "\n", current + 1, position ); }
	return line;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ObjectFile::parse_keywords( const String &buffer )
{
	// Setup
	for( KeywordID keywordID = 0; keywordID < KEYWORD_COUNT; keywordID++ )
	{
		g_KEYWORD_COUNT[keywordID] = 0;
	}

	// Gather keywords
	const usize length = buffer.length_bytes();
	for( usize current = 0LLU; current < length; current++ )
	{
		// Comments
		if( buffer[current] == '/' )
		{
			// Single line comment
			if( buffer.contains_at( "//", current ) )
			{
				current = buffer.find( "\n", current );
				if( current == USIZE_MAX ) { break; }
				continue;
			}

			// Block comment
			if( buffer.contains_at( "/*", current ) )
			{
				current = buffer.find( "*/", current );
				if( current == USIZE_MAX ) { break; }
				current++;
				continue;
			}
		}

		// Shortcut common chars
		if( buffer[current] < 'A' || buffer[current] > 'Z' ) { continue; }

		// Find top keyword
		Keyword keyword { 0, buffer.length_bytes(), buffer.length_bytes() };
		for( KeywordID keywordID = 0; keywordID < KEYWORD_COUNT; keywordID++ )
		{
			if( buffer.contains_at( g_KEYWORDS[keywordID], current ) )
			{
				keyword.start = current;
				keyword.id = keywordID;
				break;
			}
		}
		if( keyword.start == keyword.end ) { continue; }

		// Check to make sure the keyword is a valid, stand-alone word
		const char prevChar = keyword.start > 0 ? buffer[keyword.start - 1] : ' ';
		const char postChar = keyword.start + strlen( g_KEYWORDS[keyword.id] ) < buffer.length_bytes() ?
			buffer[keyword.start + strlen( g_KEYWORDS[keyword.id] )] : ' ';
		if( !char_is_keyword_delimiter( prevChar ) || !char_is_keyword_delimiter( postChar ) ) { continue; }

		// Adjust previous keyword end
		if( keywords.size() > 0 )
		{
			Keyword &keywordPrev = keywords[keywords.size() - 1];
			keywordPrev.end = keyword.start;
		}

		// Register Keyword
		keyword.start += strlen( g_KEYWORDS[keyword.id] );
		keywords.add( keyword );
		g_KEYWORD_COUNT[keyword.id]++;
		current = keyword.start - 1;
	}

	// Validate Keyword Requirements
	for( KeywordID keywordID = 0; keywordID < KEYWORD_COUNT; keywordID++ )
	{
		const KeywordRequirements &requirements = g_KEYWORD_REQUIREMENTS[keywordID];

		// Requirements
		ErrorIf( requirements.required && g_KEYWORD_COUNT[keywordID] < requirements.count,
			"object must have at least %d '%s'", requirements.count, g_KEYWORDS[keywordID], keywordID );

		// Limit
		ErrorIf( requirements.count != -1 && g_KEYWORD_COUNT[keywordID] > requirements.count,
			"object can only have %d '%s'", requirements.count, g_KEYWORDS[keywordID] );
	}
}


void ObjectFile::parse_keywords_values( const String &buffer )
{
	// Loop through keywords
	for( Keyword &keyword : keywords )
	{
		const usize line = line_at( buffer, keyword.start );

		// Process Keyword
		switch( keyword.id )
		{
			// OBJECT
			case KeywordID_OBJECT:
			{
				name = keyword_PARENTHESES_string( buffer, keyword );
				ErrorIfLine( name.length_bytes() == 0, line,
				"%s() must be a valid string!", g_KEYWORDS[KeywordID_OBJECT] );
				type = name;
				type.append( "_t" );
				ErrorIfLine( !Objects::objectTypes.add( type.hash(), type ), line, "duplicate object name!" );
			}
			break;

			// PARENT
			case KeywordID_PARENT:
			{
				nameParent = keyword_PARENTHESES_string( buffer, keyword );
				if( nameParent.length_bytes() == 0 ) { nameParent = "DEFAULT"; }
				ErrorIfLine( nameParent.length_bytes() == 0, line,
				"%s() must be a valid string!", g_KEYWORDS[KeywordID_PARENT] );
				typeParent = nameParent;
				typeParent.append( "_t" );
				typeParentFull = "CoreObjects::";
				typeParentFull.append( typeParent );
			}
			break;

			// COUNT
			case KeywordID_COUNT:
			{
				const i64 value = keyword_PARENTHESES_i64( buffer, keyword );
				ErrorIfLine( value < 0 || value > U32_MAX, line,
					"%s() must be range 1 - %u", g_KEYWORDS[KeywordID_COUNT], U32_MAX );
				ErrorIfLine( value == 0, line,
					"%s() must be range 1 - %u. Use %s( true ) if desired size is 0",
					g_KEYWORDS[KeywordID_COUNT], U32_MAX, g_KEYWORDS[KeywordID_ABSTRACT] );
				countMax = static_cast<usize>( value );
			}
			break;

			// BUCKET_SIZE
			case KeywordID_BUCKET_SIZE:
			{
				const usize value = static_cast<usize>( keyword_PARENTHESES_int( buffer, keyword ) );
				ErrorIfLine( value > U16_MAX, line,
					"%s() must be range 1 - %u", g_KEYWORDS[KeywordID_BUCKET_SIZE], U16_MAX );
				ErrorIfLine( value == 0LLU, line,
					"%s() must be range 1 - %u. Use %s( true ) if desired size is 0",
					g_KEYWORDS[KeywordID_BUCKET_SIZE], U16_MAX, g_KEYWORDS[KeywordID_ABSTRACT] );
				bucketSize = value;
			}
			break;

			// HASH
			case KeywordID_HASH:
			{
				hash = keyword_PARENTHESES_string( buffer, keyword );
				ErrorIfLine( hash.length_bytes() == 0, line,
				"%s() must be a valid string!", g_KEYWORDS[KeywordID_HASH] );
			}
			break;

			// CATEGORY
			case KeywordID_CATEGORY:
			{
				keyword_CATEGORY( buffer, keyword );
			}
			break;

			// VERSIONS
			case KeywordID_VERSIONS:
			{
				keyword_VERSIONS( buffer, keyword );
			}
			break;

			// ABSTRACT
			case KeywordID_ABSTRACT:
			{
				abstract = keyword_PARENTHESES_bool( buffer, keyword );
			}
			break;

			// NETWORKED
			case KeywordID_NETWORKED:
			{
				networked = keyword_PARENTHESES_bool( buffer, keyword );
			}
			break;
		}
	}
}


void ObjectFile::parse_keywords_code( const String &buffer )
{
	// Loop through keywords
	for( Keyword &keyword : keywords )
	{
		// Event Keywords (KeywordID_EVENT_...)
		if( KEYWORD_IS_EVENT( keyword.id ) )
		{
			keyword_EVENT( buffer, keyword );
		}

		// Other Keywords
		switch( keyword.id )
		{
			// HEADER_INCLUDES / SOURCE_INCLUDES
			case KeywordID_INCLUDES:
			case KeywordID_HEADER_INCLUDES:
			case KeywordID_SOURCE_INCLUDES:
			{
				keyword_INCLUDES( buffer, keyword );
			}
			break;

			// CONSTRUCTOR
			case KeywordID_CONSTRUCTOR:
			{
				keyword_CONSTRUCTOR( buffer, keyword );
			}
			break;

			// WRITE
			case KeywordID_WRITE:
			{
				keyword_WRITE( buffer, keyword );
			}
			break;

			// READ
			case KeywordID_READ:
			{
				keyword_READ( buffer, keyword );
			}
			break;

			// SERIALIZE
			case KeywordID_SERIALIZE:
			{
				keyword_SERIALIZE( buffer, keyword );
			}
			break;

			// DESERIALIZE
			case KeywordID_DESERIALIZE:
			{
				keyword_DESERIALIZE( buffer, keyword );
			}
			break;

			// PRIVATE / PROTECTED / PUBLIC / GLOBAL
			case KeywordID_PRIVATE:
			case KeywordID_PROTECTED:
			case KeywordID_PUBLIC:
			case KeywordID_GLOBAL:
			{
				keyword_PRIVATE_PUBLIC_GLOBAL( buffer, keyword );
			}
			break;

			// FRIEND
			case KeywordID_FRIEND:
			{
				keyword_FRIEND( buffer, keyword );
			}
			break;
		}
	}
}


void ObjectFile::keyword_INCLUDES( const String &buffer, Keyword &keyword )
{
	String &includes = ( keyword.id == KeywordID_INCLUDES || keyword.id == KeywordID_HEADER_INCLUDES ) ?
		Objects::headerIncludes : Objects::sourceIncludes;

	usize start = buffer.find( "#i", keyword.start, keyword.end );
	while( start != USIZE_MAX )
	{
		const usize end = buffer.find( "\n", start );
		ErrorIfLine( end == USIZE_MAX, line_at( buffer, keyword.start ),
			 "%s #include has no endline", g_KEYWORDS[keyword.id] );

		String include = buffer.substr( start, end );
		if( includes.find( include ) == USIZE_MAX ) { includes.append( include ).append( "\n" ); }

		if( end == USIZE_MAX ) { break; }
		start = buffer.find( "#i", start + 1, keyword.end );
	}
}


void ObjectFile::keyword_CONSTRUCTOR( const String &buffer, Keyword &keyword )
{
	// Detect function scope braces
	const usize line = line_at( buffer, keyword.start );
	usize scopeEnd = find_closing_brace( buffer, keyword.start, keyword.end );
	ErrorIfLine( scopeEnd == USIZE_MAX, line, "'%s' has invalid scope", g_KEYWORDS[keyword.id] );
	usize scopeStart = buffer.find( "{", keyword.start, keyword.end );
	ErrorIfLine( scopeStart == USIZE_MAX, line, "'%s' has invalid scope", g_KEYWORDS[keyword.id] );
	String scope = buffer.substr( scopeStart, scopeEnd + 1 );

	// Detect function arguments
	usize argsStart, argsEnd;
	find_keyword_parentheses( buffer, keyword.start, scopeStart, argsStart, argsEnd );
	String arguments = buffer.substr( argsStart + 1, argsEnd ).trim();

	if( arguments.length_bytes() == 0 )
	{
		ErrorIfLine( constructorHasDefault, line, "Object already has a specified default constructor!" );
		constructorHasDefault = true;
		arguments.append( "()" );
	}
	else
	{
		arguments.insert( 0, "( " );
		arguments.append( " )" );
	}

	// Source
	String source;
	source.append( "CoreObjects::" ).append( type ).append( "::" ).append( type ).append( arguments ).append( "\n" );
	source.append( scope ).append( "\n" );
	constructorSource.add( static_cast<String &&>( source ) );

	// Header
	String header;
	header.append( type ).append( arguments ).append( ";" );
	constructorHeader.add( static_cast<String &&>( header ) );
}


void ObjectFile::keyword_WRITE( const String &buffer, Keyword &keyword )
{
	// Detect function scope braces
	const usize line = line_at( buffer, keyword.start );
	usize scopeEnd = find_closing_brace( buffer, keyword.start, keyword.end );
	ErrorIfLine( scopeEnd == USIZE_MAX, line, "'%s' has invalid scope", g_KEYWORDS[keyword.id] );
	usize scopeStart = buffer.find( "{", keyword.start, keyword.end );
	ErrorIfLine( scopeStart == USIZE_MAX, line, "'%s' has invalid scope", g_KEYWORDS[keyword.id] );
	String scope = buffer.substr( scopeStart + 1, scopeEnd );

	// Source
	String &source = writeSource;
	source.append( scope ).append( "\n" );

	// Header
	String &header = writeHeader;
	header.append( "void _write( class Buffer &buffer );" );

	// Flag
	hasWriteRead = true;
}


void ObjectFile::keyword_READ( const String &buffer, Keyword &keyword )
{
	// Detect function scope braces
	const usize line = line_at( buffer, keyword.start );
	usize scopeEnd = find_closing_brace( buffer, keyword.start, keyword.end );
	ErrorIfLine( scopeEnd == USIZE_MAX, line, "'%s' has invalid scope", g_KEYWORDS[keyword.id] );
	usize scopeStart = buffer.find( "{", keyword.start, keyword.end );
	ErrorIfLine( scopeStart == USIZE_MAX, line, "'%s' has invalid scope", g_KEYWORDS[keyword.id] );
	String scope = buffer.substr( scopeStart + 1, scopeEnd );

	// Source
	String &source = readSource;
	source.append( scope ).append( "\n" );

	// Header
	String &header = readHeader;
	header.append( "bool _read( class Buffer &buffer );" );

	// Flag
	hasWriteRead = true;
}


void ObjectFile::keyword_SERIALIZE( const String &buffer, Keyword &keyword )
{
	// Detect function scope braces
	const usize line = line_at( buffer, keyword.start );
	usize scopeEnd = find_closing_brace( buffer, keyword.start, keyword.end );
	ErrorIfLine( scopeEnd == USIZE_MAX, line, "'%s' has invalid scope", g_KEYWORDS[keyword.id] );
	usize scopeStart = buffer.find( "{", keyword.start, keyword.end );
	ErrorIfLine( scopeStart == USIZE_MAX, line, "'%s' has invalid scope", g_KEYWORDS[keyword.id] );

	// Source
	serializeSource.append( buffer.substr( scopeStart + 1, scopeEnd ).trim() );
	if( serializeSource.length_bytes() == 0 ) { serializeSource.append( "// ..." ); }

	// Header
	serializeHeader.append( "void _serialize( class Buffer &buffer );" );

	// Flag
	hasSerialize = true;
}


void ObjectFile::keyword_DESERIALIZE( const String &buffer, Keyword &keyword )
{
	// Detect function scope braces
	const usize line = line_at( buffer, keyword.start );
	usize scopeEnd = find_closing_brace( buffer, keyword.start, keyword.end );
	ErrorIfLine( scopeEnd == USIZE_MAX, line, "'%s' has invalid scope", g_KEYWORDS[keyword.id] );
	usize scopeStart = buffer.find( "{", keyword.start, keyword.end );
	ErrorIfLine( scopeStart == USIZE_MAX, line, "'%s' has invalid scope", g_KEYWORDS[keyword.id] );

	// Source
	deserializeSource.append( buffer.substr( scopeStart + 1, scopeEnd ).trim() );
	if( deserializeSource.length_bytes() == 0 ) { deserializeSource.append( "// ..." ); }

	// Header
	deserializeHeader.append( "bool _deserialize( class Buffer &buffer );" );

	// Flag
	hasSerialize = true;
}


void ObjectFile::keyword_EVENT( const String &buffer, Keyword &keyword )
{
	u8 eventID = keyword.id;
	events[eventID].inherits = true;
	events[eventID].implements = true;
	const usize modifiersStart = keyword.start;

	// Detect function scope braces
	const usize line = line_at( buffer, keyword.start );
	keyword.end = find_closing_brace( buffer, keyword.start, keyword.end );
	ErrorIfLine( keyword.end == USIZE_MAX, line, "'%s' has invalid scope", g_KEYWORDS[keyword.id] );
	keyword.start = buffer.find( "{", keyword.start, keyword.end );
	ErrorIfLine( keyword.start == USIZE_MAX, line, "'%s' has invalid scope", g_KEYWORDS[keyword.id] );

	// DISABLE?
	usize disabled = buffer.find( "DISABLE", modifiersStart, keyword.start );
	if( disabled != USIZE_MAX )
	{
		events[eventID].inherits = false;
		events[eventID].implements = false;
		events[eventID].disabled = true;
		events[eventID].manual = true;
		keyword_EVENT_NULL( buffer, keyword );
		return;
	}

	// MANUAL?
	usize manual = buffer.find( "MANUAL", modifiersStart, keyword.start );
	events[eventID].manual = manual != USIZE_MAX;

	// NOINHERIT?
	usize noinherit = buffer.find( "NOINHERIT", modifiersStart, keyword.start );
	events[eventID].noinherit = noinherit != USIZE_MAX;

	// Source
	String &source = events[eventID].source;
	source.append( buffer.substr( keyword.start + 1, keyword.end ) );

	// Header
	String &header = events[eventID].header;
	header.append( g_EVENT_FUNCTIONS[eventID][EventFunction_ReturnType] ).append( " " );
	header.append( g_EVENT_FUNCTIONS[eventID][EventFunction_Name] );
	header.append( g_EVENT_FUNCTIONS[eventID][EventFunction_Parameters] ).append( ";" );
}


void ObjectFile::keyword_EVENT_NULL( const String &buffer, Keyword &keyword )
{
	// Header -- null events are simply inline, private, and empty scope (prevents inheritance)
	u8 eventID = keyword.id;
	String &header = events[eventID].null;
	header.append( "inline " );
	header.append( g_EVENT_FUNCTIONS[eventID][EventFunction_ReturnType] );
	header.append( " " );
	header.append( g_EVENT_FUNCTIONS[eventID][EventFunction_Name] );
	header.append( g_EVENT_FUNCTIONS[eventID][EventFunction_Parameters] );
	header.append( " { " );
	header.append( strlen( g_EVENT_FUNCTIONS[eventID][EventFunction_ReturnValue] ) != 0 ? "return " : "return" );
	header.append( g_EVENT_FUNCTIONS[eventID][EventFunction_ReturnValue] );
	header.append( "; }" );
}


void ObjectFile::keyword_PRIVATE_PUBLIC_GLOBAL( const String &buffer, Keyword &keyword )
{
	usize start = keyword.start;
	usize end = keyword.end;

	// Determine if keyword is for a function or data
	bool isFunction = true;
	const usize semicolon = buffer.find( ";", start, end );
	const usize openingBrace = buffer.find( "{", start, end );
	const usize closingBrace = find_closing_brace( buffer, start, end );
	if( closingBrace == USIZE_MAX ) { isFunction = false; } else
	if( closingBrace > end ) { isFunction = false; } else
	if( semicolon < openingBrace ) { isFunction = false; } else
	if( buffer.contains_at( "};", closingBrace ) ) { isFunction = false; }

	// PUBLIC / PRIVATE
	if( keyword.id != KeywordID_GLOBAL )
	{
		// Helper Lambda
		auto list_ref = []( KeywordID id, List<String> &priv,
			List<String> &prot, List<String> &publ ) -> List<String> &
		{
			if( id == KeywordID_PRIVATE ) { return priv; }
			if( id == KeywordID_PROTECTED ) { return prot; }
			return publ;
		};

		// FUNCTION
		if( isFunction )
		{
			// Output
			List<String> &header = list_ref(
				keyword.id, privateFunctionHeader, protectedFunctionHeader, publicFunctionHeader );
			List<String> &source = list_ref(
				keyword.id, privateFunctionSource, protectedFunctionSource, publicFunctionSource );

			// Update 'end'
			end = closingBrace + 1;

			// Copy function
			String functionSource = buffer.substr( start, end ).trim();
			const usize openBrace = buffer.find( "{", start, end );
			ErrorIfLine( openBrace == USIZE_MAX, line_at( buffer, start ),
				"function declaration: invalid/missing scope" );
			String functionHeader = buffer.substr( start, openBrace ).trim();
			String functionInherited = functionHeader;

			// Format source function (e.g. source.cpp: "int OBJECT::foo() { ... }" )
			functionSource.append( "\n" );
			usize current = functionSource.find( "(", 0, functionSource.find( "{" ) ) - 1;
			ErrorIfLine( current == USIZE_MAX - 1, line_at( buffer, start ),
				"function declaration: invalid/missing parameter parentheses" );
			bool onName = false;
			for( ;; )
			{
				ErrorIfLine( current == 0, line_at( buffer, keyword.start ),
					"function declaration: no return type" );
				const bool whitespace = char_is_whitespace( functionSource[current] );
				if( whitespace && onName ) { current++; break; }
				current--;
				onName |= !whitespace;
			}
			functionSource.insert( current, "::" );
			functionSource.insert( current, type );
			functionSource.insert( current, "CoreObjects::" );

			// Format header function (e.g. header.hpp: "int foo();")
			functionHeader.append( ";" );

			// Register functions
			source.add( static_cast<String &&>( functionSource ) );
			header.add( static_cast<String &&>( functionHeader ) );
		}
		else
		// VARIABLE / DATA
		{
			// Output
			List<String> &header = list_ref(
				keyword.id, privateVariableHeader, protectedVariableHeader, publicVariableHeader );

			// Update 'end'
			end = buffer.find( ";", start, end );
			ErrorIfLine( end == USIZE_MAX, line_at( buffer, keyword.start ),
				"declaration: missing terminating semicolon" );
			end++;

			// Variable?
			const bool isVariable = !( buffer.find( "struct", start, end ) != USIZE_MAX ||
									   buffer.find( "class", start, end ) != USIZE_MAX ||
									   buffer.find( "enum", start, end ) != USIZE_MAX ||
									   buffer.find( "enum_type", start, end ) != USIZE_MAX );
			if( !isVariable )
			{
				end = find_closing_brace( buffer, start );
				ErrorIfLine( end == USIZE_MAX, line_at( buffer, keyword.start ),
					"declaration: missing scope braces" );
				end += 2;
			}

			// Copy expression
			String expression = buffer.substr( start, end ).trim();

			// Format expression
			expression.replace( "\n", "\n\t" );

			// Register expression
			header.add( static_cast<String &&>( expression ) );
		}
	}
	else
	// GLOBAL
	{
		// FUNCTION
		if( isFunction )
		{
			// Output
			List<String> &source = globalFunctionSource;
			List<String> &header = globalFunctionHeader;

			// Update 'end'
			end = closingBrace + 1;

			// Copy function
			String functionSource = buffer.substr( start, end ).trim();

			// Format source function (e.g. source.cpp: "int foo() { ... }" )
			functionSource.append( "\n" );

			// Format header function (e.g. header.hpp: "extern int foo();" )
			const usize openBrace = functionSource.find( "{" );
			ErrorIfLine( openBrace == USIZE_MAX, line_at( buffer, keyword.start ),
				"function declaration: invalid/missing scope" );
			String functionHeader = functionSource.substr( 0, openBrace ).trim();
			functionHeader.insert( 0, "extern " );
			functionHeader.append( ";\n" );

			// Register functions
			source.add( static_cast<String &&>( functionSource ) );
			header.add( static_cast<String &&>( functionHeader ) );
		}
		else
		// VARIABLE / DATA
		{
			// Output
			List<String> &source = globalVariableSource;
			List<String> &header = globalVariableHeader;

			// Update 'end'
			end = buffer.find( ";", start, end );
			ErrorIfLine( end == USIZE_MAX, line_at( buffer, keyword.start ),
				"declaration: missing terminating semicolon" );
			end++;

			// Variable?
			const bool isVariable = !( buffer.find( "struct", start, end ) != USIZE_MAX ||
				buffer.find( "class", start, end ) != USIZE_MAX ||
				buffer.find( "enum", start, end ) != USIZE_MAX ||
				buffer.find( "enum_type", start, end ) != USIZE_MAX );
			if( !isVariable )
			{
				end = find_closing_brace( buffer, start );
				ErrorIfLine( end == USIZE_MAX, line_at( buffer, keyword.start ),
					"declaration: missing scope braces" );
				end += 2;
			}

			// Copy expression
			String expressionSource = buffer.substr( start, end ).trim();
			String expressionHeader = expressionSource;

			// Variable expression
			if( isVariable )
			{
				// Format source expression (e.g. source.cpp: "int g_Variable = 10;" )
				// Format header expression (e.g. header.hpp: "extern int g_Variable;")

				const usize endBrace = expressionHeader.find( "{" );
				const usize endSemicolon = expressionHeader.find( ";" );
				const usize endAssignment = expressionHeader.find( "=" );
				ErrorIfLine(
					endAssignment == USIZE_MAX && endSemicolon == USIZE_MAX && endSemicolon == USIZE_MAX,
					line_at( buffer, keyword.start ), "declaration: missing '{ }', '=', or ';'" );
				const usize endIndex = min( endBrace, min( endSemicolon, endAssignment ) );
				expressionHeader.remove( endIndex, expressionHeader.length_bytes() - endIndex );

				// DVAR?
				PrintLn( PrintColor_Magenta, "%s", expressionHeader.cstr() );
				if( expressionHeader.contains_at( "DVAR", 0 ) )
				{
					// Handle DVAR macros
					expressionSource.insert( 0, "static " );
					source.add( static_cast<String &&>( expressionSource ) );
					return;
				}

				// Register Variable Expressions
				expressionHeader.trim();
				expressionHeader.insert( 0, "extern " );
				expressionHeader.append( ";\n" );
				source.add( static_cast<String &&>( expressionSource ) );
				header.add( static_cast<String &&>( expressionHeader ) );
			}
			else
			// Type expression (struct, class, enum, etc.)
			{
				// Format header expression
				expressionHeader.append( "\n" );

				// Register expression
				header.add( static_cast<String &&>( expressionHeader ) );
			}
		}
	}
}


void ObjectFile::keyword_FRIEND( const String &buffer, Keyword &keyword )
{
	usize start = 0;
	usize end = 0;

	const bool found = find_keyword_parentheses( buffer, keyword.start, keyword.end, start, end );
	ErrorIfLine( !found, line_at( buffer, keyword.start ),
		"%s(...) must not be empty!", g_KEYWORDS[KeywordID_CATEGORY] );

	String f = buffer.substr( start + 1, end ).trim();
	ErrorIfLine( f.length_bytes() == 0, line_at( buffer, keyword.start ),
		"%s(...) must not be empty!", g_KEYWORDS[KeywordID_CATEGORY] );

	// Register friend
	friendsHeader.add( static_cast<String &&>( f ) );
}


void ObjectFile::keyword_CATEGORY( const String &buffer, Keyword &keyword )
{
	usize current = 0LLU;
	usize end = 0LLU;
	const bool found = find_keyword_parentheses( buffer, keyword.start, keyword.end, current, end );
	ErrorIfLine( !found, line_at( buffer, keyword.start ),
		"%s(...) must not be empty!", g_KEYWORDS[KeywordID_CATEGORY] );

	usize delimiter = min( buffer.find( ",", current + 1, end ), end );
	for( ;; )
	{
		// Register category
		String category = buffer.substr( current + 1, delimiter ).trim();
		if( category.length_bytes() > 0 )
		{
			const u32 categoryHash = category.hash();
			categories.add( categoryHash, true );
			Objects::objectCategories.add( categoryHash, category );
		}

		// Loop
		if( delimiter == end ) { break; }
		current = delimiter + 1;
		delimiter = min( buffer.find( ",", current + 1, end ), end );
	}
}


void ObjectFile::keyword_VERSIONS( const String &buffer, Keyword &keyword )
{
	usize current = 0LLU;
	usize end = 0LLU;
	const bool found = find_keyword_parentheses( buffer, keyword.start, keyword.end, current, end );
	ErrorIfLine( !found, line_at( buffer, keyword.start ),
		"%s(...) must not be empty!", g_KEYWORDS[KeywordID_VERSIONS] );

	String &header = versionsHeader;
	header.append( "\tenum\n\t{\n\t\t" );
	usize delimiter = min( buffer.find( ",", current + 1, end ), end );
	for( ;; )
	{
		// Register Version
		String version = buffer.substr( current + 1, delimiter ).trim();
		if( version.length_bytes() > 0 ) { header.append( version ).append( ",\n\t\t" ); }

		// Loop
		if( delimiter == end ) { break; }
		current = delimiter + 1;
		delimiter = min( buffer.find( ",", current + 1, end ), end );
	}
	header.append( "VERSION_COUNT\n\t};\n" );
}


String ObjectFile::keyword_PARENTHESES_string( const String &buffer, Keyword &keyword, bool requireParentheses )
{
	// Find parentheses
	usize open = 0;
	usize close = 0;
	const bool found = find_keyword_parentheses( buffer, keyword.start, keyword.end, open, close );
	if( found ) { return buffer.substr( open + 1, close ).trim(); }
	ErrorIfLine( requireParentheses, line_at( buffer, keyword.start ),
		"%s(...) must not be empty!", g_KEYWORDS[keyword.id] );
	return "";
}


int ObjectFile::keyword_PARENTHESES_int( const String &buffer, Keyword &keyword, bool requireParentheses )
{
	const String string = keyword_PARENTHESES_string( buffer, keyword );
	return string.length_bytes() == 0 ? 0 : atoi( string.cstr() );
}


i64 ObjectFile::keyword_PARENTHESES_i64( const String &buffer, Keyword &keyword, bool requireParentheses )
{
	const String string = keyword_PARENTHESES_string( buffer, keyword );
	return string.length_bytes() == 0 ? 0 : atoll( string.cstr() );
}


double ObjectFile::keyword_PARENTHESES_double( const String &buffer, Keyword &keyword, bool requireParentheses )
{
	const String string = keyword_PARENTHESES_string( buffer, keyword );
	return string.length_bytes() == 0 ? 0.0 : atof( string.cstr() );
}


float ObjectFile::keyword_PARENTHESES_float( const String &buffer, Keyword &keyword, bool requireParentheses )
{
	const String string = keyword_PARENTHESES_string( buffer, keyword );
	return string.length_bytes() == 0 ? 0.0f : static_cast<float>( atof( string.cstr() ) );
}


bool ObjectFile::keyword_PARENTHESES_bool( const String &buffer, Keyword &keyword, bool requireParentheses )
{
	const String string = keyword_PARENTHESES_string( buffer, keyword );
	if( string.length_bytes() == 0 ) { return true; }
	if( string == "true" || string == "1" ) { return true; }
	if( string == "false" || string == "0" ) { return false; }
	ErrorIfLine( true, line_at( buffer, keyword.start ),
		"invalid boolean parentheses for keyword %s", string.cstr() );
	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ObjectFile::parse()
{
	// Buffer
	String buffer;

	// Skip DEFAULT
	if( name == "DEFAULT" )
	{
		// Read from s_DEFAULT_DEFINITION
		buffer = s_DEFAULT_DEFINITION;
	}
	else
	{
		// Read from file
		ErrorIf( !buffer.load( path ), "failed to open object file: %s", path.cstr() );
	}

	// Console
	if( verbose_output() )
	{
		Print( PrintColor_White, TAB TAB "Parse " );
		Print( PrintColor_Cyan, "%s", path.length_bytes() == 0 ? "DEFAULT" : path.cstr() );
	}

	Timer timer;
	{
		// Parse Keywords
		parse_keywords( buffer );

		// Process "Read" Keywords
		parse_keywords_values( buffer );

		// Write Keywords
		parse_keywords_code( buffer );
	}

	if( verbose_output() )
	{
		PrintLn( PrintColor_White, " (%.3f ms)", timer.elapsed_ms() );
	}
}


void ObjectFile::validate()
{
	// HASH
	{
		if( hash.length_bytes() == 0 ) { hash = name; }
		char hex[16]; snprintf( hex, 16, "0x%08X", hash.hash() );
		hashHex = hex;
	}

	// READ / WRITE validation
	{
		// Serialize / Deserialize
		ErrorIf( writeSource.length_bytes() != 0 && readSource.length_bytes() == 0,
			"%s has %s function but missing corresponding %s function",
			name.cstr(), g_KEYWORDS[KeywordID_WRITE], g_KEYWORDS[KeywordID_READ] );

		ErrorIf( readSource.length_bytes() != 0 && writeSource.length_bytes() == 0,
			"%s has %s function but missing corresponding %s function",
			name.cstr(), g_KEYWORDS[KeywordID_READ], g_KEYWORDS[KeywordID_WRITE] );
	}

	// SERIALIZE / DESERIALIZE / VERSIONS validation
	{
		// Versions
		if( versionsHeader.length_bytes() > 0 )
		{
			ErrorIf( serializeSource.length_bytes() == 0, "%s has %s(...) enums but missing %s function!",
				name.cstr(), g_KEYWORDS[KeywordID_VERSIONS], g_KEYWORDS[KeywordID_SERIALIZE] );

			ErrorIf( deserializeSource.length_bytes() == 0, "%s has %s(...) enums but missing %s function!",
				name.cstr(), g_KEYWORDS[KeywordID_VERSIONS], g_KEYWORDS[KeywordID_DESERIALIZE] );
		}
		else
		{
			ErrorIf( serializeSource.length_bytes() != 0, "%s has %s function but missing %s(...) enums!",
				name.cstr(), g_KEYWORDS[KeywordID_SERIALIZE], g_KEYWORDS[KeywordID_VERSIONS] );

			ErrorIf( deserializeSource.length_bytes() != 0, "%s has %s function but missing %s(...) enums!",
				name.cstr(), g_KEYWORDS[KeywordID_DESERIALIZE], g_KEYWORDS[KeywordID_VERSIONS] );
		}

		// Serialize / Deserialize
		ErrorIf( serializeSource.length_bytes() != 0 && deserializeSource.length_bytes() == 0,
			"%s has %s function but missing corresponding %s function",
			name.cstr(), g_KEYWORDS[KeywordID_SERIALIZE], g_KEYWORDS[KeywordID_DESERIALIZE] );

		ErrorIf( deserializeSource.length_bytes() != 0 && serializeSource.length_bytes() == 0,
			"%s has %s function but missing corresponding %s function",
			name.cstr(), g_KEYWORDS[KeywordID_DESERIALIZE], g_KEYWORDS[KeywordID_SERIALIZE] );

		// Networked Requirement
		ErrorIf( networked && ( serializeSource.length_bytes() == 0 || deserializeSource.length_bytes() == 0 ),
			"%s marked as %s, but missing required %s & %s functions",
			name.cstr(), g_KEYWORDS[KeywordID_NETWORKED], g_KEYWORDS[KeywordID_SERIALIZE],
			g_KEYWORDS[KeywordID_DESERIALIZE] );
	}
}


void ObjectFile::write_header()
{
	// Header
	String output;
	output.append( COMMENT_BREAK "\n\n" );

	// Global Variables / Data
	for( String &var : globalVariableHeader ) { output.append( var ); }
	if( globalVariableHeader.size() > 0 ) { output.append( "\n" ); }

	output.append( "__INTERNAL_OBJECT_SYSTEM_BEGIN\n" );

	// Object Struct
	{
		output.append( "class " ).append( type );
		if( parent != nullptr ) { output.append( " : public " ).append( typeParent ); }
		output.append( "\n{\n" );
		{
			// Friends
			output.append( "\tfriend struct CoreObjects::OBJECT_ENCODER<Object::" ).append( name ).append( ">;\n" );
			for( String &f : friendsHeader ) { output.append( "\tfriend " ).append( f ).append( ";\n" ); }

			// Public
			output.append( "public:\n" );
			{
				// Constructors
				if( !constructorHasDefault ) { output.append( "\t" ).append( type ).append( "() = default;\n" ); }
				for( String &ctor : constructorHeader ) { output.append( "\t" ).append( ctor ).append( "\n" ); }

				// Public Variables
				for( String &var : publicVariableHeader )
				{
					// Only write members that are unique to this object (factoring inheritance)
					if( parent == nullptr || !( parent->inheritedVariables.contains( var ) ) )
					{
						output.append( "\t" ).append( var ).append( "\n" );
					}
				}

				// Public Functions
				for( String &func : publicFunctionHeader )
				{
					output.append( "\tvirtual " ).append( func ).append( "\n" );
				}

				// Public Events
				for( u8 eventID = 0; eventID < EVENT_COUNT; eventID++ )
				{
					if( events[eventID].header.length_bytes() == 0 ) { continue; }
					output.append( "\tvirtual " ).append( events[eventID].header ).append( "\n" );
				}
			}

			// Protected
			output.append( "protected:\n" );
			{
				// Protected Variables
				for( String &var : protectedVariableHeader )
				{
					// Only write members that are unique to this object (factoring inheritance)
					if( parent == nullptr || !( parent->inheritedVariables.contains( var ) ) )
					{
						output.append( "\t" ).append( var ).append( "\n" );
					}
				}

				// Protected Functions
				for( String &func : protectedFunctionHeader )
				{
					output.append( "\tvirtual " ).append( func ).append( "\n" );
				}
			}

			// Private
			output.append( "private:\n" );
			{
				// Versions
				if( versionsHeader.length_bytes() > 0 ) { output.append( versionsHeader ); }\

				// Write & Read
				if( hasWriteRead )
				{
					output.append( "\t" ).append( writeHeader ).append( "\n" );
					output.append( "\t" ).append( readHeader ).append( "\n" );
				}

				// Serialize & Deserialize
				if( hasSerialize )
				{
					output.append( "\t" ).append( serializeHeader ).append( "\n" );
					output.append( "\t" ).append( deserializeHeader ).append( "\n" );
				}

				// Private Variables
				for( String &var : privateVariableHeader )
				{
					output.append( "\t" ).append( var ).append( "\n" );
				}

				// Private Functions
				for( String &func : privateFunctionHeader )
				{
					output.append( "\t" ).append( func ).append( "\n" );
				}

				// Disabled Events
				for( u8 eventID = 0; eventID < EVENT_COUNT; eventID++ )
				{
					if( events[eventID].null.length_bytes() == 0 ) { continue; }
					output.append( "\t" ).append( events[eventID].null ).append( "\n\n" );
				}
			}
		}
		output.append( "};\n\n" );
	}

	// Custom Constructor
	if( constructorHeader.size() > 0 )
	{
		output.append( "\ntemplate <typename... Args> struct TYPE_CONSTRUCT_VARIADIC<Object::" );
		output.append( name ).append( ", Args...>\n{\n" );
		output.append( "\tstatic void CONSTRUCT( void *object, Args... args ) { new ( object ) " );
		output.append( type ).append( "( args... ); }\n};\n" );
	}
	output.append( "__INTERNAL_OBJECT_SYSTEM_END\n\n" );

	// ObjectHandle
	output.append( "template <> struct ObjectHandle<Object::" ).append( name ).append( ">\n{\n" );
	output.append( "\tstatic CoreObjects::" ).append( type ).append( " stub;\n" );
	output.append( "\tCoreObjects::" ).append( type ).append( " *data = nullptr;\n" );
	output.append( "\tCoreObjects::" ).append( type );
	output.append( " *operator->() const { " );
	output.append( "return UNLIKELY( data == nullptr ) ? &ObjectHandle<Object::" ).append( name );
	output.append( ">::stub : data; }\n" );
	output.append( "\texplicit operator bool() const { return data != nullptr; }\n" );
	output.append( "\tObjectHandle( void *object ) { data = reinterpret_cast<CoreObjects::" );
	output.append( type ).append( " *>( object ); }\n" );
	if( hasWriteRead )
	{
		output.append( "\tstatic void write( class Buffer &buffer, const ObjectHandle<Object::" );
		output.append( name ).append( "> &handle );\n" );
		output.append( "\tstatic bool read( class Buffer &buffer, ObjectHandle<Object::" );
		output.append( name ).append( "> &handle );\n" );
	}
	if( hasSerialize )
	{
		output.append( "\tstatic void serialize( class Buffer &buffer, const ObjectHandle<Object::" );
		output.append( name ).append( "> &handle );\n" );
		output.append( "\tstatic bool deserialize( class Buffer &buffer, ObjectHandle<Object::" );
		output.append( name ).append( "> &handle );\n" );
	}
	output.append( "};\n" );

	// Global Functions
	if( globalFunctionHeader.size() > 0 )
	{
		for( String &str : globalFunctionHeader ) { output.append( str ); }
		output.append( "\n" );
	}
	output.append( "\n" );

	// Finish
	Objects::header.append( output );
}


void ObjectFile::write_source()
{
	// Header Break
	String output;
	output.append( COMMENT_BREAK "\n\n" );

	// ObjectHandle Stub
	output.append( "CoreObjects::" ).append( type ).append( " " );
	output.append( "ObjectHandle<Object::" ).append( name ).append(">::stub = { };\n\n" );

	// Global Data
	for( String &str : globalVariableSource ) { output.append( str ).append( "\n" ); }
	if( globalVariableSource.count() > 0 ) { output.append( "\n" ); }

	// Encoder
	output.append( "template <> struct CoreObjects::OBJECT_ENCODER<Object::" ).append( name );
	output.append(">\n{\n" );
	output.append( "\tOBJECT_ENCODER( void *data ) : object { *reinterpret_cast<" ).append( type );
	output.append( " *>( data ) } { } " ).append( type ).append( " &object;\n" );
	if( hasWriteRead )
	{
		output.append( "\tstatic void write( class Buffer &buffer, const OBJECT_ENCODER<Object::" );
		output.append( name ).append( "> &context ) { context.object._write( buffer ); }\n" );
		output.append( "\tstatic bool read( class Buffer &buffer, OBJECT_ENCODER<Object::" );
		output.append( name ).append( "> &context ) { return context.object._read( buffer ); }\n" );
	}
	if( hasSerialize )
	{
		output.append( "\tstatic void serialize( class Buffer &buffer, const OBJECT_ENCODER<Object::" );
		output.append( name ).append( "> &context ) { context.object._serialize( buffer ); }\n" );
		output.append( "\tstatic bool deserialize( class Buffer &buffer, OBJECT_ENCODER<Object::" );
		output.append( name ).append( "> &context ) { return context.object._deserialize( buffer ); }\n" );
	}
	output.append( "};\n\n" );

	// Constructors
	for( String &str : constructorSource )
	{
		output.append( str ).append( "\n" );
	}

	// Write / Read / Serialize / Deserialize
	if( writeSource.length_bytes() > 0 ) { output.append( writeSource ).append( "\n" ); }
	if( readSource.length_bytes() > 0 ) { output.append( readSource ).append( "\n" ); }

	if( writeSource.length_bytes() > 0 )
	{
		output.append( "void CoreObjects::" ).append( type ).append( "::_write( Buffer &buffer )\n{\n" );
		output.append( writeSource ).append( "}\n\n" );
	}

	if( readSource.length_bytes() > 0 )
	{
		output.append( "bool CoreObjects::" ).append( type ).append( "::_read( Buffer &buffer )\n{\n" );
		output.append( readSource ).append( "\treturn true;\n}\n\n" );
	}

	// Serializer
	if( hasSerialize )
	{
		// object_t::serialize
		output.append( "void CoreObjects::" ).append( type ).append( "::_serialize( Buffer &buffer )\n{\n" );
		output.append( "\tSerializer serializer;\n\tserializer.begin( buffer, VERSION_COUNT - 1 );\n\t" );
		output.append( serializeSource );
		output.append( "\n\tserializer.end();\n}\n\n" );

		// ObjectHandle<Object>::serialize
		output.append( "void ObjectHandle<Object::" ).append( name );
		output.append( ">::serialize( Buffer &buffer, const ObjectHandle<Object::" ).append( name );
		output.append( "> &handle )\n{\n" );
		output.append( "\tAssert( handle.data != nullptr );\n" );
		output.append( "\tSerializer serializer; serializer.begin( buffer, 0 );\n" );
		for( ObjectFile *par = this; par != nullptr; par = par->parent )
		{
			if( !par->hasSerialize ) { continue; }
			output.append( "\t{ CoreObjects::OBJECT_ENCODER<Object::" ).append( par->name );
			output.append( "> slice { handle.data }; serializer.write( " );
			output.append( par->hashHex ).append( ", slice ); }\n" );
		}
		output.append( "\tserializer.end();\n}\n\n" );
	}

	// Deserializer
	if( hasSerialize )
	{
		// object_t::serialize
		output.append( "bool CoreObjects::" ).append( type ).append( "::_deserialize( Buffer &buffer )\n{\n" );
		output.append( "\tDeserializer deserializer;\n\tdeserializer.begin( buffer, VERSION_COUNT - 1 );\n\t" );
		output.append( deserializeSource );
		output.append( "\n\tdeserializer.end();\n\treturn true;\n}\n\n" );

		// ObjectHandle<Object>::deserialize
		output.append( "bool ObjectHandle<Object::" ).append( name );
		output.append( ">::deserialize( Buffer &buffer, ObjectHandle<Object::" ).append( name );
		output.append( "> &handle )\n{\n" );
		output.append( "\tAssert( handle.data != nullptr );\n" );
		output.append( "\tDeserializer deserializer; deserializer.begin( buffer, 0 );\n" );
		for( ObjectFile *par = this; par != nullptr; par = par->parent )
		{
			if( !par->hasSerialize ) { continue; }
			output.append( "\t{ CoreObjects::OBJECT_ENCODER<Object::" ).append( par->name );
			output.append( "> slice { handle.data }; if( !deserializer.read( " );
			output.append( par->hashHex ).append( ", slice ) ) { return false; }; }\n" );
		}
		output.append( "\tdeserializer.end();\n\treturn true;\n}\n\n" );
	}

	// Events
	for( u8 eventID = 0; eventID < EVENT_COUNT; eventID++ )
	{
		if( !events[eventID].implements || events[eventID].disabled ) { continue; }

		output.append( g_EVENT_FUNCTIONS[eventID][EventFunction_ReturnType] );
		output.append( " CoreObjects::" );
		output.append( type );
		output.append( "::" );
		output.append( g_EVENT_FUNCTIONS[eventID][EventFunction_Name] );
		output.append( g_EVENT_FUNCTIONS[eventID][EventFunction_Parameters] ).append( "\n{" );
		for( ObjectFile *par = parent; !events[eventID].noinherit; )
		{
			if( par == nullptr || par->name.equals( "DEFAULT" ) ) { break; }
			if( !par->events[eventID].implements ) { par = par->parent; continue; }

			output.append( "\n\tCoreObjects::" ).append( par->type ).append( "::" );
			output.append( g_EVENT_FUNCTIONS[eventID][EventFunction_Name] );
			output.append( g_EVENT_FUNCTIONS[eventID][EventFunction_ParametersCaller] );
			output.append( events[eventID].source.length_bytes() > 0 ? ";" : ";\n" );
			break;
		}
		output.append( events[eventID].source );
		output.append( "}\n\n" );
	}

	// Private Functions
	for( String &string : privateFunctionSource ) { output.append( string ).append( "\n" ); }

	// Protected Functions
	for( String &string : protectedFunctionSource ) { output.append( string ).append( "\n" ); }

	// Public Functions
	for( String &string : publicFunctionSource ) { output.append( string ).append( "\n" ); }

	// Global Functions
	for( String &string : globalFunctionSource ) { output.append( string ).append( "\n" ); }

	// INHERIT
	const bool hasParent = ( nameParent != "DEFAULT" );
	if( hasParent ) { output.replace( "INHERIT", typeParentFull ); }

	// Finish
	Objects::source.append( output );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Objects
{
	// Output Paths
	char pathSourceObjects[PATH_SIZE];
	char pathHeaderObjects[PATH_SIZE];
	char pathHeaderSystem[PATH_SIZE];
	char pathIntelliSense[PATH_SIZE];

	// Output Contents
	String headerIncludes;
	String sourceIncludes;
	String header;
	String source;
	String system;
	String intellisense;

	// Object Files
	List<ObjectFile> objectFiles;
	List<ObjectFile *> objectFilesSorted;

	// Object Categories
	HashMap<u32, String> objectTypes;
	HashMap<u32, String> objectCategories;

	// Cache
	Cache cache;
	usize cacheFileCount = 0LLU;

	// Logging
	usize objectsBuilt = 0LLU;
}


void Objects::begin()
{
	// Initialize system with DEFAULT
	ObjectFile object;
	object.name = "DEFAULT";
	object.nameParent = "";
	objectFiles.add( object );

	// Output Paths
	strjoin( pathIntelliSense, Build::pathOutput, SLASH "generated" SLASH "objects.generated.intellisense" );
	strjoin( pathHeaderObjects, Build::pathOutput, SLASH "generated" SLASH "objects.generated.hpp" );
	strjoin( pathSourceObjects, Build::pathOutput, SLASH "generated" SLASH "objects.generated.cpp" );
	strjoin( pathHeaderSystem, Build::pathOutput, SLASH "generated" SLASH "objects.system.generated.hpp" );
}


void Objects::end()
{
	// ...
}


usize Objects::gather( const char *directory, bool recurse )
{
	// Iterate Directories
	Timer timer;
	List<FileInfo> objectFilesDisk;
	directory_iterate( objectFilesDisk, directory, ".object", true );

	for( FileInfo &file : objectFilesDisk )
	{
		u64 hash = file.time.as_u64();
		Hash::hash64_bytes( hash, file.path, strlen( file.path ) );
		const CacheKey cacheKey = static_cast<CacheKey>( hash );

		CacheObject cacheObject;
		if( !Objects::cache.dirty && !Objects::cache.fetch( cacheKey, cacheObject ) )
		{
			Objects::cache.dirty |= true;
		}

		Objects::cache.store( cacheKey, cacheObject );
		Objects::cacheFileCount++;

		// Add Object
		objectFiles.add( ObjectFile { file.path } );
	}

	return objectFilesDisk.size();
}


void Objects::parse()
{
	for( ObjectFile &object : objectFiles ) { object.parse(); Objects::objectsBuilt++; }
}


void Objects::resolve()
{
	if( verbose_output() )
	{
		Print( PrintColor_White, TAB TAB "Resolve Inheritance" );
	}

	Timer timer;

	// Resolve object dependencies (build inheritance N-tree)
	for( ObjectFile &object : objectFiles )
	{
		// Search for parent
		if( object.nameParent.length_bytes() == 0 ) { continue; }
		for( ObjectFile &parent : objectFiles )
		{
			// Parent found?
			if( object.nameParent == parent.name )
			{
				// Don't allow self-inheritance
				ErrorIf( parent.name == object.name, "%s(%s) attempting to inherit itself\n\t > %s",
					g_KEYWORDS[KeywordID_OBJECT], object.name.cstr(), object.path.cstr() );

				object.parent = &parent;
				parent.children.add( &object );
				break;
			}
		}

		// Make sure we resolved a parent
		ErrorIf( object.parent == nullptr, "No %s(%s) exists\n\t > %s",
			g_KEYWORDS[KeywordID_PARENT], object.nameParent.cstr(), object.path.cstr() );
	}

	// Sort objects based on inheritance depth
	objectFilesSorted.add( &objectFiles[0] ); // DEFAULT
	objectFiles[0].depth = 0;
	objectFiles[0].visited = true;
	sort_objects( &objectFiles[0], 1, objectFilesSorted );

	// Inherit variables, functions, & events
	for( ObjectFile &object : objectFiles )
	{
		ObjectFile *parent = object.parent;
		while( parent != nullptr )
		{
			// Categories
			for( auto &category : parent->categories )
			{
				object.categories.add( category.key, true );
			}

			// Friends
			for( String &f : parent->friendsHeader )
			{
				object.friendsHeader.add( f );
			}

			// Variables
			for( String &variable : parent->publicVariableHeader )
			{
				if( !( object.inheritedVariables.contains( variable ) ) )
				{
					object.inheritedVariables.add( variable );
				}
			}

			for( String &variable : parent->protectedVariableHeader )
			{
				if( !( object.inheritedVariables.contains( variable ) ) )
				{
					object.inheritedVariables.add( variable );
				}
			}

			// Functions
			for( String &function : parent->publicFunctionHeader )
			{
				if( !( object.inheritedFunctions.contains( function ) ) )
				{
					object.inheritedFunctions.add( function );
				}
			}

			for( String &function : parent->protectedFunctionHeader )
			{
				if( !( object.inheritedFunctions.contains( function ) ) )
				{
					object.inheritedFunctions.add( function );
				}
			}

			// Events
			for( u8 eventID = 0; eventID < EVENT_COUNT; eventID++ )
			{
				if( parent->events[eventID].header.length_bytes() == 0 ) { continue; }
				if( !( object.inheritedEvents.contains( parent->events[eventID].header ) ) )
				{
					object.inheritedEvents.add( parent->events[eventID].header );
				}
			}

			// Continue loop
			parent = parent->parent;
		}
	}

	// Log
	if( verbose_output() )
	{
		PrintLn( PrintColor_White, " (%.3f ms)", timer.elapsed_ms() );
	}
};


void Objects::sort_objects( ObjectFile *object, u16 depth, List<ObjectFile *> &outList )
{
	// This performs a "topological sort" of the ObjectFiles based on the inheritance tree
	// The output ensures that parents always precede children

	// Visit all children
	ErrorIf( depth == U16_MAX, "exceeded maximum inheritance depth (%u)\n\t > %s", U16_MAX, object->path.cstr() );
	for( ObjectFile *child : object->children )
	{
		// Skip already visited children
		if( child->visited ) { continue; }

		// Set inheritance depth
		child->depth = depth;
		child->visited = true;

		// Propogate events
		for( u8 eventID = 0; eventID < EVENT_COUNT; eventID++ )
		{
			Event &myEvent = object->events[eventID];
			Event &childEvent = child->events[eventID];

			if( ( ( myEvent.inherits || myEvent.implements ) && !myEvent.disabled ) && !childEvent.inherits )
			{
				childEvent.inherits = true;
				childEvent.manual = myEvent.manual;
			}
		}

		// Propogate inherited states
		// ...

		// Add child & recurse
		outList.add( child );
		if( child->children.size() > 0 ) { sort_objects( child, depth + 1, outList ); }
	}
}


void Objects::validate()
{
	// Validate Objects
	for( ObjectFile &object : objectFiles ) { object.validate(); }
}


void Objects::codegen()
{
	// Generates C++ source & header contents including:
	// 1.) Class definitions and member function implementations for each object type
	// 2.) Boilerplate datastructures & functions necessary for the object system (manta/objects.hpp)

	// IntelliSense
	codegen_intellisense( Objects::intellisense );
	{
		if( verbose_output() )
		{
			Print( PrintColor_White, TAB TAB "Write " );
			Print( PrintColor_Cyan, "%s", pathIntelliSense );
		}

		Timer timer;
		ErrorIf( !Objects::intellisense.save( pathIntelliSense ), "failed to write '%s'", pathIntelliSense );

		if( verbose_output() )
		{
			PrintLn( PrintColor_White, " (%.3f ms)", timer.elapsed_ms() );
		}
	}

	// System Header
	codegen_header_system( Objects::system );
	{
		if( verbose_output() )
		{
			Print( PrintColor_White, TAB TAB "Write " );
			Print( PrintColor_Cyan, "%s", pathHeaderSystem );
		}

		Timer timer;
		ErrorIf( !Objects::system.save( pathHeaderSystem ), "failed to write '%s'", pathHeaderSystem );

		if( verbose_output() )
		{
			PrintLn( PrintColor_White, " (%.3f ms)", timer.elapsed_ms() );
		}
	}

	// Objects Header
	codegen_header_objects( Objects::header );
	{
		if( verbose_output() )
		{
			Print( PrintColor_White, TAB TAB "Write " );
			Print( PrintColor_Cyan, "%s", pathHeaderObjects );
		}

		Timer timer;
		ErrorIf( !Objects::header.save( pathHeaderObjects ), "failed to write '%s'", pathHeaderObjects );

		if( verbose_output() )
		{
			PrintLn( PrintColor_White, " (%.3f ms)", timer.elapsed_ms() );
		}
	}

	// Objects Source
	codegen_source_objects( Objects::source );
	{
		if( verbose_output() )
		{
			Print( PrintColor_White, TAB TAB "Write " );
			Print( PrintColor_Cyan, "%s", pathSourceObjects );
		}

		Timer timer;
		ErrorIf( !Objects::source.save( pathSourceObjects ), "failed to write '%s'", pathSourceObjects );

		if( verbose_output() )
		{
			PrintLn( PrintColor_White, " (%.3f ms)", timer.elapsed_ms() );
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Objects::codegen_intellisense( String &output )
{
	if( verbose_output() )
	{
		Print( PrintColor_White, TAB TAB "Generate " );
		Print( PrintColor_Cyan, "output/generated/objects.generated.intellisense" );
	}

	Timer timer;

	// File Info
	output.append( COMMENT_BREAK "\n\n" );
	output.append( "/*\n" );
	output.append( " * File generated by build.exe\n" );
	output.append( " * Refer to: source/build/objects.cpp (Objects::generate_intellisense)\n" );
	output.append( " *\n" );
	output.append( " * This header should only be included in manta/source/objects_api.hpp\n" );
	output.append( " * This file enables IntelliSense/syntax highlighting for inherited objects\n" );
	output.append( " *\n" );
	output.append( " * This file is NOT compiled into the runtime executable!\n" );
	output.append( " */\n\n" );
	output.append( COMMENT_BREAK "\n\n" );

	// #pragma once
	output.append( "#pragma once\n\n" );

	// IntelliSense
	for( ObjectFile *object : objectFilesSorted )
	{
		output.append( COMMENT_BREAK "\n\n" );
		output.append( "namespace CoreObjects::ObjectIntelliSense_" ).append( object->name ).append( "\n{\n" );

		// Inherited Variables
		for( String &variable : object->inheritedVariables )
		{
			output.append( "\t" ).append( variable ).append( "\n" );
		}

		// Inherited Functions
		for( String &function : object->inheritedFunctions )
		{
			output.append( "\t" ).append( function ).append( "\n" );
		}

		// Inherited Events
		for( String &event : object->inheritedEvents )
		{
			output.append( "\t" ).append( event ).append( "\n" );
		}

		output.append( "}\n\n" );
	}

	// EOF
	output.append( COMMENT_BREAK );

	// Logging
	if( verbose_output() )
	{
		PrintLn( PrintColor_White, " (%.3f ms)", timer.elapsed_ms() );
	}
}


void Objects::codegen_header_system( String &output )
{
	// File Info
	output.append( "#pragma once\n\n" );

	output.append( COMMENT_BREAK "\n\n" );
	output.append( "/*\n" );
	output.append( " * File generated by build.exe\n" );
	output.append( " * Refer to: source/build/objects.cpp (Objects::generate_header_system)\n" );
	output.append( " */\n\n" );
	output.append( COMMENT_BREAK "\n\n" );
	output.append( "#include <core/types.hpp>\n\n" );
	output.append( COMMENT_BREAK "\n\n" );

	// Object Types
	output.append( "enum_class\n(\n\tObject, u16,\n\n" );
	for( ObjectFile *object : objectFilesSorted )
	{
		output.append( "\t" ).append( object->name ).append( ",\n" );
	}
	output.append( ");\n\n" );

	// TYPE_COUNT
	output.append( "namespace CoreObjects\n{\n" );
	output.append( "\tconstexpr u16 TYPE_COUNT = " );
	output.append( objectTypes.count() ).append( ";\n}\n\n" );

	// ObjectContext Categories
	output.append( COMMENT_BREAK "\n\n" );
	output.append( "enum_class\n(\n\tObjectCategory, u16,\n\n" );
	output.append( "\tDEFAULT,\n" );
	for( auto &category : objectCategories )
	{
		output.append( "\t" ).append( category.value ).append( ",\n" );
	}
	output.append( ");\n\n" );

	// CATEGORY_COUNT
	output.append( "namespace CoreObjects\n{\n" );
	output.append( "\tconstexpr u16 CATEGORY_COUNT = " );
	output.append( objectCategories.count() + 1 ).append( ";\n}\n\n" );

	// EOF
	output.append( COMMENT_BREAK );
}


void Objects::generate_source_system( String &output )
{
	output.append( COMMENT_BREAK "\n\n" );

	// CATEGORY_TYPE_BUCKET
	List<u16> categoryTypeCount;
	output.append( "const u16 CoreObjects::CATEGORY_TYPE_BUCKET" );
	output.append( "[CoreObjects::CATEGORY_COUNT][CoreObjects::TYPE_COUNT] =\n{\n" );
	categoryTypeCount.add( generate_source_system_category_types_mapped( output, "DEFAULT" ) );
	for( auto &category : objectCategories )
	{
		categoryTypeCount.add( generate_source_system_category_types_mapped( output, category.value ) );
	}
	output.append( "};\n\n" );

	output.append( "const u16 CoreObjects::CATEGORY_TYPES" );
	output.append( "[CoreObjects::CATEGORY_COUNT][CoreObjects::TYPE_COUNT] =\n{\n" );
	generate_source_system_category_types( output, "DEFAULT" );
	for( auto &category : objectCategories )
	{
		generate_source_system_category_types( output, category.value );
	}
	output.append( "};\n\n" );

	// CATEGORY_TYPE_COUNT
	output.append( "const u16 CoreObjects::CATEGORY_TYPE_COUNT[CoreObjects::CATEGORY_COUNT] =\n{\n\t" );
	for( usize i = 0, j = 0; i < categoryTypeCount.size(); i++, j++ )
	{
		output.append( categoryTypeCount[i] );
		output.append( ( j % 7 == 0 && i != 0 && i != categoryTypeCount.size() - 1 ) ? ",\n\t" : ", " );
	}
	output.append( "\n};\n\n" );

	// CATEGORY_NAME
	output.append( "#if COMPILE_DEBUG\n" );
	output.append( "const char *CoreObjects::CATEGORY_NAME[CoreObjects::CATEGORY_COUNT] =\n{\n" );
	output.append( "\t\"DEFAULT\",\n" );
	for( auto &category : objectCategories )
	{
		output.append( "\t\"" ).append( category.value ).append( "\",\n" );
	}
	output.append( "};\n" );
	output.append( "#endif\n\n" );

	// TYPE_SIZE
	output.append( "const u16 CoreObjects::TYPE_SIZE[CoreObjects::TYPE_COUNT] =\n{\n\t" );
	for( usize i = 0, j = 0; i < objectFilesSorted.size(); i++, j++ )
	{
		ObjectFile &object = *objectFilesSorted[i];
		output.append( "sizeof( CoreObjects::" ).append( object.type ).append( " ),\n\t" );
	}
	output.append( "\n};\n\n" );

	// TYPE_ALIGNMENT
	output.append( "const u16 CoreObjects::TYPE_ALIGNMENT[CoreObjects::TYPE_COUNT] =\n{\n\t" );
	for( usize i = 0, j = 0; i < objectFilesSorted.size(); i++, j++ )
	{
		ObjectFile &object = *objectFilesSorted[i];
		output.append( "alignof( CoreObjects::" ).append( object.type ).append( " ),\n\t" );
	}
	output.append( "\n};\n\n" );

	// TYPE_NAME
	output.append( "#if COMPILE_DEBUG\n" );
	output.append( "const char *CoreObjects::TYPE_NAME[CoreObjects::TYPE_COUNT] =\n{\n" );
	for( usize i = 0, j = 0; i < objectFilesSorted.size(); i++, j++ )
	{
		ObjectFile &object = *objectFilesSorted[i];
		output.append( "\t\"" ).append( object.name ).append( "\",\n" );
	}
	output.append( "};\n" );
	output.append( "#endif\n\n" );

	// TYPE_BUCKET_CAPACITY
	output.append( "const u16 CoreObjects::TYPE_BUCKET_CAPACITY[CoreObjects::TYPE_COUNT] =\n{\n\t" );
	for( usize i = 0, j = 0; i < objectFilesSorted.size(); i++, j++ )
	{
		ObjectFile &object = *objectFilesSorted[i];
		output.append( object.instantiable() ? object.bucketSize : 0 );
		output.append( ( j % 7 == 0 && i != 0 && i != objectFilesSorted.size() - 1 ) ? ",\n\t" : ", " );
	}
	output.append( "\n};\n\n" );

	// TYPE_MAX_COUNT
	output.append( "const u32 CoreObjects::TYPE_MAX_COUNT[CoreObjects::TYPE_COUNT] =\n{\n\t" );
	for( usize i = 0, j = 0; i < objectFilesSorted.size(); i++, j++ )
	{
		ObjectFile &object = *objectFilesSorted[i];
		output.append( object.instantiable() ? object.countMax : 0 );
		output.append( ( j % 7 == 0 && j != 0 && i != objectFilesSorted.size() - 1 ) ? ",\n\t" : ", " );
	}
	output.append( "\n};\n\n" );

	// TYPE_INHERITANCE_DEPTH
	output.append( "const u16 CoreObjects::TYPE_INHERITANCE_DEPTH[CoreObjects::TYPE_COUNT] =\n{\n\t" );
	for( usize i = 0, j = 0; i < objectFilesSorted.size(); i++, j++ )
	{
		ObjectFile &object = *objectFilesSorted[i];
		char buffer[16];
		snprintf( buffer, sizeof( buffer ), "%d", object.depth );
		output.append( buffer );
		output.append( ( j % 7 == 0 && j != 0 && i != objectFilesSorted.size() - 1 ) ? ",\n\t" : ", " );
	}
	output.append( "\n};\n\n" );

	// TYPE_HASH
	HashMap<u32, bool> hashesSeen;
	output.append( "const u32 CoreObjects::TYPE_HASH[CoreObjects::TYPE_COUNT] =\n{\n\t" );
	for( usize i = 0, j = 0; i < objectFilesSorted.size(); i++, j++ )
	{
		ObjectFile &object = *objectFilesSorted[i];

		// Check for hash collisions
		const u32 hash = object.hash.length_bytes() > 0 ? object.hash.hash() : 0;
		if( hash != 0 )
		{
			ErrorIf( hashesSeen.contains( hash ), "Object '%s' has a %s collision!",
				object.name.cstr(), g_KEYWORDS[KeywordID_HASH] );
			hashesSeen.add( hash, true );
		}
		output.append( object.hashHex );
		output.append( ( j % 7 == 0 && j != 0 && i != objectFilesSorted.size() - 1 ) ? ",\n\t" : ", " );
	}
	output.append( "\n};\n\n" );

	// TYPE_SERIALIZED
	output.append( "const bool CoreObjects::TYPE_SERIALIZED[CoreObjects::TYPE_COUNT] =\n{\n\t" );
	for( usize i = 0, j = 0; i < objectFilesSorted.size(); i++, j++ )
	{
		ObjectFile &object = *objectFilesSorted[i];
		output.append( object.hasSerialize ? "1" : "0" );
		output.append( ( j % 15 == 0 && j != 0 && i != objectFilesSorted.size() - 1 ) ? ",\n\t" : ", " );
	}
	output.append( "\n};\n\n" );

	// bool init()
	output.append( COMMENT_BREAK "\n\n" );
	output.append( "bool CoreObjects::init()\n{\n" );
	{
		output.append( "\tObjectInstance::Serialization::init();\n\n" );
		output.append( "\treturn true;\n" );
	}
	output.append( "}\n\n" );

	// bool free()
	output.append( "bool CoreObjects::free()\n{\n" );
	{
		output.append( "\tObjectInstance::Serialization::free();\n\n" );
		output.append( "\treturn true;\n" );
	}
	output.append( "}\n\n" );

	// Serialize
	output.append( COMMENT_BREAK "\n\n" );
	output.append( "void CoreObjects::serialize( Serializer &serializer, const ObjectContext &context )\n{\n" );
	output.append( "\tObjectInstance::Serialization::prepare( context );\n" );
	for( ObjectFile *object : objectFilesSorted )
	{
		if( !object->hasSerialize ) { continue; }
		output.append( "\tserializer.write( " ).append( object->hashHex );
		output.append( ", ObjectContextSerializer<Object::").append( object->name );
		output.append( ">{ context } );\n" );
	}
	output.append( "}\n\n" );

	// Deserialize
	output.append( "bool CoreObjects::deserialize( Deserializer &deserializer, ObjectContext &context )\n{\n" );
	for( ObjectFile *object : objectFilesSorted )
	{
		if( !object->hasSerialize ) { continue; }
		output.append( "\t{ ObjectContextDeserializerA<Object::" ).append( object->name );
		output.append( "> type { context }; " );
		output.append( "if( !deserializer.read( " ).append( object->hashHex );
		output.append( ", type ) ) { return false; } }\n" );
	}
	output.append( "\tObjectInstance::Serialization::prepare( context );\n" );
	for( ObjectFile *object : objectFilesSorted )
	{
		if( !object->hasSerialize ) { continue; }
		output.append( "\t{ ObjectContextDeserializerB<Object::" ).append( object->name );
		output.append( "> type { context }; " );
		output.append( "if( !deserializer.read( " ).append( object->hashHex );
		output.append( ", type ) ) { return false; } }\n" );
	}
	output.append( "\treturn true;\n}\n\n" );
}


u16 Objects::generate_source_system_category_types_mapped( String &output, const String &category )
{
	u16 count = 1; // Every category has at least DEFAULT_t
	output.append( "\t{ // " ).append( category ).append( "\n\t\t" );

	// Default Category
	if( category.equals( "DEFAULT" ) )
	{
		for( usize i = 0, j = 0; i < objectFilesSorted.size(); i++, j++ )
		{
			ObjectFile &object = *objectFilesSorted[i];
			output.append( object.instantiable() ? count++ : 0 );
			output.append( ( j % 15 == 0 && j != 0 && i != objectFilesSorted.size() - 1 ) ? ", \n\t\t" : ", " );
		}
	}
	else
	// Custom Category
	{
		for( usize i = 0, j = 0; i < objectFilesSorted.size(); i++, j++ )
		{
			ObjectFile &object = *objectFilesSorted[i];
			const u32 categoryHash = category.hash();
			output.append( object.instantiable() && object.categories.contains( categoryHash ) ? count++ : 0 );
			output.append( ( j % 15 == 0 && j != 0 && i != objectFilesSorted.size() - 1 ) ? ",\n\t\t" : ", " );
		}
	}

	output.append( "\n\t},\n" );
	return count;
}


void Objects::generate_source_system_category_types( String &output, const String &category )
{
	output.append( "\t{ // " ).append( category ).append( "\n\t\t" );

	// Default Category
	if( category.equals( "DEFAULT" ) )
	{
		for( usize i = 0, j = 0; i < objectFilesSorted.size(); i++, j++ )
		{
			ObjectFile &object = *objectFilesSorted[i];
			if( i == 0 || object.instantiable() )
			{
				output.append( i );
				output.append( ( j % 15 == 0 && j != 0 && i != objectFilesSorted.size() - 1 ) ? ", \n\t\t" : ", " );
			}
		}
	}
	else
	// Custom Category
	{
		for( usize i = 0, j = 0; i < objectFilesSorted.size(); i++, j++ )
		{
			ObjectFile &object = *objectFilesSorted[i];
			const u32 categoryHash = category.hash();
			if( i == 0 || ( object.instantiable() && object.categories.contains( categoryHash ) ) )
			{
				output.append( i );
				output.append( ( j % 15 == 0 && j != 0 && i != objectFilesSorted.size() - 1 ) ? ", \n\t\t" : ", " );
			}
		}
	}

	output.append( "\n\t},\n" );
}


void Objects::codegen_header_objects( String &output )
{
	// Log
	if( verbose_output() )
	{
		Print( PrintColor_White, TAB TAB "Generate " );
		Print( PrintColor_Cyan, "output/generated/objects.generated.hpp" );
	}
	Timer timer;

	// #pragma once
	output.append( "#pragma once\n\n" );

	// File Info
	output.append( COMMENT_BREAK "\n\n" );
	output.append( "/*\n" );
	output.append( " * File generated by build.exe\n" );
	output.append( " * Refer to: source/build/objects.cpp (Objects::generate_header_objects)\n" );
	output.append( " *\n" );
	output.append( " * This header should only be included in source/manta/objects.hpp\n" );
	output.append( " */\n\n" );
	output.append( COMMENT_BREAK "\n\n" );

	// SYSTEM INCLUDES
	output.append( "// SYSTEM INCLUDES\n" );
	output.append( "#include <vendor/new.hpp>\n" );
	output.append( "#include <manta/objects.hpp>\n" );
	output.append( "\n" );

	// HEADER_INCLUDES
	output.append( "// HEADER_INCLUDES\n" );
	output.append( Objects::headerIncludes );
	output.append( Objects::headerIncludes.length_bytes() == 0 ? "// ...\n" : "\n" );

	// Object Class Definitions
	for( ObjectFile *object : objectFilesSorted ) { object->write_header(); }

	// Object Constructor / Destructors
	output.append( COMMENT_BREAK "\n\n" );
	output.append( "__INTERNAL_OBJECT_SYSTEM_BEGIN\n" );
	{
		output.append( "constexpr void ( *TYPE_CONSTRUCT[] )( void * ) =\n{\n" );
		for( ObjectFile *object : objectFilesSorted )
		{
			output.append( "\t[]( void *object ) { new ( object ) " ).append( object->type ).append( "(); },\n" );
		}
		output.append( "};\n\n" );

		output.append( "constexpr void ( *TYPE_DESTRUCT[] )( void * ) =\n{\n" );
		for( ObjectFile *object : objectFilesSorted )
		{
			output.append( "\t[]( void *object ) { reinterpret_cast<").append( object->type );
			output.append( "*>( object )->~" ).append( object->type ).append( "(); },\n" );
		}
		output.append( "};\n" );
	}
	output.append( "__INTERNAL_OBJECT_SYSTEM_END\n\n" );

	// EOF
	output.append( COMMENT_BREAK );

	// Logging
	if( verbose_output() )
	{
		PrintLn( PrintColor_White, " (%.3f ms)", timer.elapsed_ms() );
	}
}


void Objects::codegen_source_objects( String &output )
{
	// Log
	if( verbose_output() )
	{
		Print( PrintColor_White, TAB TAB "Generate " );
		Print( PrintColor_Cyan, "output/generated/objects.generated.cpp" );
	}
	Timer timer;

	// File Info
	output.append( COMMENT_BREAK "\n\n" );
	output.append( "/*\n" );
	output.append( " * File generated by build.exe\n" );
	output.append( " * Refer to: source/build/objects.cpp (Objects::generate_source_objects)\n" );
	output.append( " *\n" );
	output.append( " * Provides implementations for manta/objects.system.hpp and output/generated/objects.generated.hpp\n" );
	output.append( " */\n\n" );
	output.append( COMMENT_BREAK "\n\n" );

	// SYSTEM INCLUDES
	output.append( "// SYSTEM INCLUDES\n" );
	output.append( "#include <vendor/new.hpp>\n" );
	output.append( "#include <core/serializer.hpp>\n" );
	output.append( "#include <manta/objects.hpp>\n" );
	output.append( "\n" );

	// SOURCE_INCLUDES
	output.append( "// SOURCE_INCLUDES\n" );
	output.append( Objects::sourceIncludes );
	output.append( Objects::sourceIncludes.length_bytes() == 0 ? "// ...\n" : "\n" );

	// Safety Defines (INHERIT)
	output.append( COMMENT_BREAK "\n\n" );
	output.append( "#define INHERIT\n\n" );

	// Object System Internals
	generate_source_system( output );

	// ObjectContext Functions
	generate_source_objects_events( output );

	// ObjectHandle<Object>.handle
	output.append( COMMENT_BREAK "\n\n" );
	for( ObjectFile *object : objectFilesSorted )
	{
		output.append( "template <> ObjectHandle<Object::" ).append( object->name );
		output.append( "> ObjectInstance::handle<Object::" ).append( object->name );
		output.append( ">( const ObjectContext &context ) const\n{\n" );
		output.append( "\treturn { context.get_object_pointer( *this ) };\n}\n\n" );
	}

	// Object Classes
	for( ObjectFile *object : objectFilesSorted ) { object->write_source(); }

	// EOF
	output.append( COMMENT_BREAK );

	// Logging
	if( verbose_output() )
	{
		PrintLn( PrintColor_White, " (%.3f ms)", timer.elapsed_ms() );
	}
};


void Objects::generate_source_objects_events( String &output )
{
	// Events
	for( u8 eventID = 0; eventID < EVENT_COUNT; eventID++ )
	{
		// Skip certain events
		if( eventID == KeywordID_EVENT_CREATE ) { continue; }
		if( strcmp( g_EVENT_FUNCTIONS[eventID][EventFunction_ReturnType], "void" ) != 0 ) { continue; }

		// Generate category-specific event functions
		output.append( COMMENT_BREAK "\n\n" );
		output.append( "__INTERNAL_OBJECT_SYSTEM_BEGIN\n" );
		List<String> categoryEventFunctions;
		categoryEventFunctions.add( generate_source_objects_events_category( output, eventID, "DEFAULT" ) );
		for( auto &category : objectCategories )
		{
			categoryEventFunctions.add( generate_source_objects_events_category( output, eventID, category.value ) );
		}

		// Generate category events function pointer array
		output.append( "static void ( *" );
		output.append( g_EVENT_FUNCTIONS[eventID][EventFunction_Name] );
		output.append( "[] )( ObjectContext &context" );
		output.append( g_EVENT_FUNCTIONS[eventID][EventFunction_Parameters][1] == ')' ? " " : "," );
		output.append( &g_EVENT_FUNCTIONS[eventID][EventFunction_Parameters][1] );
		output.append( " = \n{\n" );
		for( auto function : categoryEventFunctions ) { output.append( "\t" ).append( function ).append( ",\n" ); }
		output.append( "};\n" );
		output.append( "__INTERNAL_OBJECT_SYSTEM_END\n\n" );

		// Generate ObjectContext event function
		output.append( "void ObjectContext::" );
		output.append( g_EVENT_FUNCTIONS[eventID][EventFunction_Name] );
		output.append( g_EVENT_FUNCTIONS[eventID][EventFunction_Parameters] );
		output.append( "\n{\n" );
		output.append( "\tif( CoreObjects::").append( g_EVENT_FUNCTIONS[eventID][EventFunction_Name] );
		output.append( "[category] == nullptr ) { return; }\n" );
		output.append( "\tCoreObjects::" ).append( g_EVENT_FUNCTIONS[eventID][EventFunction_Name] );
		output.append( "[category]( *this" );
		output.append( g_EVENT_FUNCTIONS[eventID][EventFunction_Parameters][1] == ')' ? " " : "," );
		output.append( &g_EVENT_FUNCTIONS[eventID][EventFunction_ParametersCaller][1] ).append( ";\n" );
		output.append( "}\n\n" );
	}
}


String Objects::generate_source_objects_events_category( String &output, u8 eventID, const String &category )
{
	String event;
	bool generated = false;
	const bool defaultCategory = category.equals( "DEFAULT" );

	// Generate event function
	event.append( "void " );
	event.append( g_EVENT_FUNCTIONS[eventID][EventFunction_Name] ).append( "_" ).append( category );
	event.append( "( ObjectContext &context" );
	event.append( g_EVENT_FUNCTIONS[eventID][EventFunction_Parameters][1] == ')' ? " " : "," );
	event.append( &g_EVENT_FUNCTIONS[eventID][EventFunction_Parameters][1] );
	event.append( "\n{\n" );
	for( ObjectFile *object : objectFilesSorted )
	{
		if( !object->instantiable() ) { continue; }
		if( !object->events[eventID].inherits ) { continue; }
		if( object->events[eventID].disabled ) { continue; }
		if( object->events[eventID].manual ) { continue; }
		if( !defaultCategory && !object->categories.contains( category.hash() ) ) { continue; }

		event.append( "\tforeach_object( context, Object::" ).append( object->name ).append( ", h ) { " );
		event.append( "h->" ).append( g_EVENT_FUNCTIONS[eventID][EventFunction_Name] );
		event.append( g_EVENT_FUNCTIONS[eventID][EventFunction_ParametersCaller] ).append( "; }\n" );
		generated = true;
	}
	event.append( "}\n\n" );

	// Generation successful
	if( generated )
	{
		output.append( event );
		String eventFunction;
		eventFunction.append( g_EVENT_FUNCTIONS[eventID][EventFunction_Name] ).append( "_" ).append( category );
		return eventFunction;
	}

	// Generation failed (no objects have this event)
	return "nullptr";
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Objects::cache_read( const char *path )
{
	Objects::cache.dirty |= Build::cache.dirty;

	// Graphics Cache
	if( !Objects::cache.dirty )
	{
		Objects::cache.read( path );
		Objects::cache.dirty |= Objects::cache.dirty;
	}

	// Codegen Cache
	AssetFile codegen;
	if( !asset_file_register( codegen, pathIntelliSense ) ) { Objects::cache.dirty = true; return; }
	if( !asset_file_register( codegen, pathHeaderObjects ) ) { Objects::cache.dirty = true; return; }
	if( !asset_file_register( codegen, pathSourceObjects ) ) { Objects::cache.dirty = true; return; }
	if( !asset_file_register( codegen, pathHeaderObjects ) ) { Objects::cache.dirty = true; return; }
}


void Objects::cache_write( const char *path )
{
	Objects::cache.write( path );
}


void Objects::cache_validate()
{
	Objects::cache.dirty |= Build::cache.dirty;

	// Validate file count
	CacheObjects cacheObjects;
	if( !Objects::cache.fetch( 0, cacheObjects ) ) { Objects::cache.dirty |= true; }
	Objects::cache.dirty |= ( Objects::cacheFileCount != cacheObjects.fileCount );

	// Cache file count
	cacheObjects.fileCount = Objects::cacheFileCount;
	Objects::cache.store( 0, cacheObjects );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////