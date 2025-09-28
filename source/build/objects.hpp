// ObjectFile system based on manta/build/api.hpp
#pragma once

#include <core/types.hpp>
#include <core/list.hpp>
#include <core/string.hpp>
#include <core/hashmap.hpp>

#include <build/filesystem.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum_type( KeywordID, u8 )
{
// EVENTS
	KeywordID_EVENT_CREATE,
	KeywordID_EVENT_DESTROY,

	KeywordID_EVENT_INITIALIZE,

	KeywordID_EVENT_FRAME_START,
	KeywordID_EVENT_FRAME_END,

	KeywordID_EVENT_UPDATE_CUSTOM,
	KeywordID_EVENT_UPDATE_GUI,
	KeywordID_EVENT_UPDATE,

	KeywordID_EVENT_RENDER_CUSTOM,
	KeywordID_EVENT_RENDER_GUI,
	KeywordID_EVENT_RENDER,

	KeywordID_EVENT_CUSTOM,
	KeywordID_EVENT_PREPARE,
	KeywordID_EVENT_TEST,

	KeywordID_EVENT_SLEEP,
	KeywordID_EVENT_WAKE,

	KeywordID_EVENT_FLAG,
	KeywordID_EVENT_PARTITION,

	KeywordID_EVENT_NETWORK_SEND,
	KeywordID_EVENT_NETWORK_RECEIVE,
	EVENT_COUNT,

// KEYWORDS
	KeywordID_INCLUDES = EVENT_COUNT,
	KeywordID_HEADER_INCLUDES,
	KeywordID_SOURCE_INCLUDES,
	KeywordID_OBJECT,
	KeywordID_PARENT,
	KeywordID_COUNT,
	KeywordID_BUCKET_SIZE,
	KeywordID_HASH,
	KeywordID_CATEGORY,
	KeywordID_VERSIONS,
	KeywordID_ABSTRACT,
	KeywordID_NETWORKED,
	KeywordID_CONSTRUCTOR,
	KeywordID_WRITE,
	KeywordID_READ,
	KeywordID_SERIALIZE,
	KeywordID_DESERIALIZE,
	KeywordID_PRIVATE,
	KeywordID_PROTECTED,
	KeywordID_PUBLIC,
	KeywordID_GLOBAL,
	KeywordID_FRIEND,

	KEYWORD_COUNT,
};

struct Keyword
{
	Keyword( KeywordID id = 0, usize start = 0, usize end = 0 ) : id{ id }, start{ start }, end{ end } { }
	KeywordID id = 0;
	usize start = 0;
	usize end = 0;
};

struct KeywordRequirements
{
	KeywordRequirements( bool required, int count ) : required{ required }, count{ count } { }
	bool required;
	int count;
};

struct Event
{
	bool implements = false;
	bool inherits = false;
	bool manual = false;
	bool disabled = false;
	bool noinherit = false;
	String header;
	String source;
	String null;
};

enum_type( EventFunction, u8 )
{
	EventFunction_Name,
	EventFunction_ReturnType,
	EventFunction_ReturnValue,
	EventFunction_Parameters,
	EventFunction_ParametersCaller,

	EVENT_FUNCTION_DEFINITION_COUNT,
};

extern const char *g_KEYWORDS[KEYWORD_COUNT];
extern int g_KEYWORD_COUNT[KEYWORD_COUNT];
extern KeywordRequirements g_KEYWORD_REQUIREMENTS[KEYWORD_COUNT];
extern const char *g_EVENT_FUNCTIONS[][EVENT_FUNCTION_DEFINITION_COUNT];

#define KEYWORD_IS_EVENT( keywordID ) ( keywordID < EVENT_COUNT )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct ObjectFile
{
	ObjectFile( const char *path = "" ) : path( path ) { }
	String path;

	void parse();
	void validate();
	void write_header();
	void write_source();
	void write_intellisense();

	void parse_keywords( const String &buffer );
	void parse_keywords_values( const String &buffer );
	void parse_keywords_code( const String &buffer );

	String keyword_PARENTHESES_string( const String &buffer, Keyword &keyword, const bool requireParentheses = true );
	int keyword_PARENTHESES_int( const String &buffer, Keyword &keyword, const bool requireParentheses = true );
	i64 keyword_PARENTHESES_i64( const String &buffer, Keyword &keyword, const bool requireParentheses = true );
	double keyword_PARENTHESES_double( const String &buffer, Keyword &keyword, const bool requireParentheses = true );
	float keyword_PARENTHESES_float( const String &buffer, Keyword &keyword, const bool requireParentheses = true );
	bool keyword_PARENTHESES_bool( const String &buffer, Keyword &keyword, const bool requireParentheses = true );

	void keyword_INCLUDES( const String &buffer, Keyword &keyword );
	void keyword_CONSTRUCTOR( const String &buffer, Keyword &keyword );
	void keyword_WRITE( const String &buffer, Keyword &keyword );
	void keyword_READ( const String &buffer, Keyword &keyword );
	void keyword_SERIALIZE( const String &buffer, Keyword &keyword );
	void keyword_DESERIALIZE( const String &buffer, Keyword &keyword );
	void keyword_EVENT( const String &buffer, Keyword &keyword );
	void keyword_EVENT_NULL( const String &buffer, Keyword &keyword );
	void keyword_PRIVATE_PUBLIC_GLOBAL( const String &buffer, Keyword &keyword );
	void keyword_FRIEND( const String &buffer, Keyword &keyword );
	void keyword_CATEGORY( const String &buffer, Keyword &keyword );
	void keyword_VERSIONS( const String &buffer, Keyword &keyword );

	bool instantiable()
	{
		if( abstract ) { return false; }
		if( countMax == 0 ) { return false; }
		if( bucketSize == 0 ) { return false; }
		return true;
	}

	// ObjectFile Info
	String name;
	String type;
	String hash;
	String hashHex;
	String nameParent = "DEFAULT";
	String typeParent = "DEFAULT_t";
	String typeParentFull = "CoreObjects::DEFAULT_t";
	usize countMax = U32_MAX;
	usize bucketSize = 1024;
	bool abstract = false;
	bool noinherit = false;
	bool networked = false;
	bool hasSerialize = false;
	bool hasWriteRead = false;

	// Topological Sort
	bool visited = false;
	bool hasParent = false;
	u16 depth = 0;
	struct ObjectFile *parent = nullptr;
	List<struct ObjectFile *> children;

	// Code Generation
	List<Keyword> keywords;
	Event events[EVENT_COUNT];
	HashMap<u32, bool> categories;

	bool constructorHasDefault = false;
	List<String> constructorHeader;
	List<String> constructorSource;

	List<String> friendsHeader;

	List<String> privateVariableHeader;
	List<String> protectedVariableHeader;
	List<String> publicVariableHeader;

	List<String> privateFunctionHeader;
	List<String> privateFunctionSource;
	List<String> protectedFunctionHeader;
	List<String> protectedFunctionSource;
	List<String> publicFunctionHeader;
	List<String> publicFunctionSource;

	List<String> globalVariableHeader;
	List<String> globalVariableSource;
	List<String> globalFunctionHeader;
	List<String> globalFunctionSource;

	List<String> inheritedVariables;
	List<String> inheritedFunctions;
	List<String> inheritedEvents;

	String versionsHeader;
	String writeHeader;
	String writeSource;
	String readHeader;
	String readSource;
	String serializeHeader;
	String serializeSource;
	String deserializeHeader;
	String deserializeSource;

	// Override Error Macros
	void ERROR_HANDLER_FUNCTION_DECL;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Objects
{
	// Output Paths
	extern char pathSource[PATH_SIZE];
	extern char pathHeader[PATH_SIZE];
	extern char pathIntelliSense[PATH_SIZE];
	extern FileTime timeCache;

	// Output Contents
	extern String intellisense;
	extern String headerIncludes;
	extern String sourceIncludes;
	extern String header;
	extern String source;

	// Object Files
	extern List<ObjectFile> objectFiles;
	extern List<ObjectFile *> objectFilesSorted;
	extern usize objectFilesCount;
	extern HashMap<u32, String> objectTypes;
	extern HashMap<u32, String> objectCategories;

	extern void begin();
	extern void gather( const char *directory, const bool recurse );
	extern void parse();

	extern void resolve();
	extern void validate();
	extern void generate();
	extern void write();

	extern void sort_objects( ObjectFile *object, u16 depth, List<ObjectFile *> &outList );

	extern void generate_intellisense( String &output );

	extern void generate_header_system( String &output );
	extern void generate_source_system( String &output );
	extern void generate_source_system_category_types( String &output, const String &category );
	extern u16 generate_source_system_category_types_mapped( String &output, const String &category );

	extern void generate_header_objects( String &output );
	extern void generate_source_objects( String &output );
	extern void generate_source_objects_events( String &output );
	extern String generate_source_objects_events_category( String &output, const u8 eventID, const String &category );

	// Override Error Macros
	void ERROR_HANDLER_FUNCTION_DECL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////