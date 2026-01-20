#pragma once

#include <core/debug.hpp>
#include <core/types.hpp>

#include <manta/fonts.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// TODO:
// TextEditor - underline
// TextEditor - strikethrough
// TextEditor - highlight
// TextEditor - hyperlink

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum_type( TextErr, int )
{
	TextErr_None = 0,
	TextErr_LimitCharacter = 1,
	TextErr_IllegalCharacter = 2,
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class TextFormat
{
public:
	constexpr TextFormat() :
		font { fnt_iosevka.id }, size { 12 },
		bold { false }, italic { false }, underline { false }, highlight { false },
		alignment { 0 },
		color { c_white },
		payload { 0 } { }

public:
	// 2 bytes
	u8 font;
	u8 size;

	// 1 Byte
	u16 bold : 1;
	u16 italic : 1;
	u16 underline : 1;
	u16 strikethrough : 1;
	u16 highlight : 1;
	u16 _unused0 : 3;

	// 1 Byte
	u16 alignment : 3;
	u16 _unused1 : 5;

	// 4 bytes
	Color color;

	// 4 bytes
	u16 payload;
	u16 _unused3;
};
static_assert( sizeof( TextFormat ) == 12, "TextFormat must be 12 bytes!" );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class TextChar
{
public:
	constexpr TextChar( u32 codepoint = 0, TextFormat format = { } ) :
		codepoint { codepoint }, format { format } { }

	explicit operator bool() const { return codepoint != 0; }
	bool is_whitespace() const;
	bool is_newline() const;

	u16 get_ttf() const;
	CoreFonts::FontGlyphInfo &get_glyph() const;
	u16_v2 get_glyph_dimensions( usize index ) const;
	u16_v2 get_glyph_dimensions_raw() const;

public:
	u32 codepoint;
	TextFormat format;
};
static_assert( sizeof( TextChar ) == 16, "TextChar must be 16 bytes!" );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Text
{
public:
#if true // MEMORY_RAII
	Text( const char *string = "" ) { init( string ); }
	Text( const Text &other ) { copy( other ); }
	Text( Text &&other ) { move( static_cast<Text &&>( other ) ); }
	~Text() { free(); }
	Text &operator=( const Text &other ) { return copy( other ); }
	Text &operator=( Text &&other ) { return move( static_cast<Text &&>( other ) ); }
#else
	Text<T> &operator=( const Text &other ) { Error( "Text: assignment disabled" ); return *this; }
	Text<T> &operator=( Text &&other ) { Error( "Text: assignment disabled" ); return *this; }
#if MEMORY_ASSERTS
	~Text()
	{
		if( Debug::memoryLeakDetection && Debug::exitCode == 0 )
		{
			MemoryAssertMsg( data == nullptr, "ERROR: Memory leak in Text (%p) (size: %.2f kb)",
				this, KB( size_allocated_bytes() ) );
		}
	}
#endif
#endif

private:
	void grow();

public:
	void init( const char *string = "" );
	void free();
	Text &copy( const Text &other );
	Text &move( Text &&other );

	void clear();
	void remove( usize index, usize count );

	usize append( const TextChar &c );
	usize append( const char *string, TextFormat format );
	usize append( const char *string );
	usize append( const Text &string, TextFormat format );
	usize append( const Text &string );

	usize insert( usize index, const TextChar &c );
	usize insert( usize index, const char *string, TextFormat format );
	usize insert( usize index, const char *string );
	usize insert( usize index, const Text &string, TextFormat format );
	usize insert( usize index, const Text &string );

	usize size_allocated_bytes() const;
	usize length_bytes() const;
	usize length() const;

	TextChar &char_at( usize index );
	const TextChar &char_at( usize index ) const;
	void string( class String &string );
	String substr( usize start, usize end );
	void cstr( char *buffer, usize size );

	void draw_selection( float x, float y, usize begin, usize end, const Alpha alpha = { } );
	void draw_caret( float x, float y, usize caret, float_v4 *outCorners = nullptr, Alpha alpha = { } );
	int_v2 draw( float x, float y, Alpha alpha = { } );

	usize get_index_from_position( int x, int y, float bias = 1.0f );
	int_v3 get_position_from_index( usize index );
	int_v2 get_dimensions();

	TextChar &operator[]( usize index ) { return char_at( index ); }
	const TextChar &operator[]( usize index ) const { return char_at( index ); }

	static bool filter_ascii( Text &text, u32 codepoint );
	static bool filter_emoji( Text &text, u32 codepoint );
	static bool filter_number( Text &text, u32 codepoint );
	static bool filter_number_positive( Text &text, u32 codepoint );
	static bool filter_integer( Text &text, u32 codepoint );
	static bool filter_integer_positive( Text &text, u32 codepoint );
	static bool filter_hex( Text &text, u32 codepoint );
	static bool filter_boolean( Text &text, u32 codepoint );
	static bool filter_filename( Text &text, u32 codepoint );
	static bool filter_console( Text &text, u32 codepoint );

private:
	struct LineInfo
	{
		usize begin = 0;
		usize end = USIZE_MAX;
		usize next = USIZE_MAX;
		u16 width = 0;
		u16 height = 0;
		u16 offset = 0;
		u8 alignment = 0;
		u8 unused;
	};
	static_assert( sizeof( LineInfo ) == 32, "LineInfo must be 32 bytes!" );

	TextFormat get_format( usize index ) const;
	LineInfo get_line( usize index );

	bool limit_characters();
	bool limit_dimensions();

public:
	TextChar *data = nullptr;
	TextFormat defaultFormat;
	usize capacity = 0LLU;
	usize current = 0LLU;

public:
	usize limitCharacters = 0; // Max character count
	u16 limitWidth = 0; // Maximum width
	u16 limitHeight = 0; // Maximum height
	u16 pageWidth = 0; // Word-wrap width

	// bool filter( Text &text, u32 codepoint )
	bool ( *filter )( Text &, u32 ) = nullptr;

	// void callbackUpdate( Text &text )
	void ( *callbackOnUpdate )( Text & ) = nullptr;

	// void callbackError( TextEditor &ime, const TextErr error )
	void ( *callbackOnError )( Text &, TextErr ) = nullptr;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace CoreText
{

constexpr const char u8ToHex[][2] = {
	"\x00", "\x01", "\x02", "\x03", "\x04", "\x05", "\x06", "\x07",
	"\x08", "\x09", "\x0A", "\x0B", "\x0C", "\x0D", "\x0E", "\x0F",
	"\x10", "\x11", "\x12", "\x13", "\x14", "\x15", "\x16", "\x17",
	"\x18", "\x19", "\x1A", "\x1B", "\x1C", "\x1D", "\x1E", "\x1F",
	"\x20", "\x21", "\x22", "\x23", "\x24", "\x25", "\x26", "\x27",
	"\x28", "\x29", "\x2A", "\x2B", "\x2C", "\x2D", "\x2E", "\x2F",
	"\x30", "\x31", "\x32", "\x33", "\x34", "\x35", "\x36", "\x37",
	"\x38", "\x39", "\x3A", "\x3B", "\x3C", "\x3D", "\x3E", "\x3F",
	"\x40", "\x41", "\x42", "\x43", "\x44", "\x45", "\x46", "\x47",
	"\x48", "\x49", "\x4A", "\x4B", "\x4C", "\x4D", "\x4E", "\x4F",
	"\x50", "\x51", "\x52", "\x53", "\x54", "\x55", "\x56", "\x57",
	"\x58", "\x59", "\x5A", "\x5B", "\x5C", "\x5D", "\x5E", "\x5F",
	"\x60", "\x61", "\x62", "\x63", "\x64", "\x65", "\x66", "\x67",
	"\x68", "\x69", "\x6A", "\x6B", "\x6C", "\x6D", "\x6E", "\x6F",
	"\x70", "\x71", "\x72", "\x73", "\x74", "\x75", "\x76", "\x77",
	"\x78", "\x79", "\x7A", "\x7B", "\x7C", "\x7D", "\x7E", "\x7F",
	"\x80", "\x81", "\x82", "\x83", "\x84", "\x85", "\x86", "\x87",
	"\x88", "\x89", "\x8A", "\x8B", "\x8C", "\x8D", "\x8E", "\x8F",
	"\x90", "\x91", "\x92", "\x93", "\x94", "\x95", "\x96", "\x97",
	"\x98", "\x99", "\x9A", "\x9B", "\x9C", "\x9D", "\x9E", "\x9F",
	"\xA0", "\xA1", "\xA2", "\xA3", "\xA4", "\xA5", "\xA6", "\xA7",
	"\xA8", "\xA9", "\xAA", "\xAB", "\xAC", "\xAD", "\xAE", "\xAF",
	"\xB0", "\xB1", "\xB2", "\xB3", "\xB4", "\xB5", "\xB6", "\xB7",
	"\xB8", "\xB9", "\xBA", "\xBB", "\xBC", "\xBD", "\xBE", "\xBF",
	"\xC0", "\xC1", "\xC2", "\xC3", "\xC4", "\xC5", "\xC6", "\xC7",
	"\xC8", "\xC9", "\xCA", "\xCB", "\xCC", "\xCD", "\xCE", "\xCF",
	"\xD0", "\xD1", "\xD2", "\xD3", "\xD4", "\xD5", "\xD6", "\xD7",
	"\xD8", "\xD9", "\xDA", "\xDB", "\xDC", "\xDD", "\xDE", "\xDF",
	"\xE0", "\xE1", "\xE2", "\xE3", "\xE4", "\xE5", "\xE6", "\xE7",
	"\xE8", "\xE9", "\xEA", "\xEB", "\xEC", "\xED", "\xEE", "\xEF",
	"\xF0", "\xF1", "\xF2", "\xF3", "\xF4", "\xF5", "\xF6", "\xF7",
	"\xF8", "\xF9", "\xFA", "\xFB", "\xFC", "\xFD", "\xFE", "\xFF",
};


template <usize N> class CharBuffer
{
public:
    template <usize... Ns> constexpr CharBuffer( const char ( &...args )[Ns] )
	{
        static_assert( sizeof...( args ) > 0, "Must provide at least one string" );
		( append( args, Ns - 1 ), ... );
		data[N] = '\0';
    }

	constexpr const char *cstr() const { return data; }
	constexpr operator const char *() const { return data; }

private:
	constexpr void append( const char *buffer, usize a )
	{
		for( usize i = 0; i < a; i++ ) { data[current++] = buffer[i]; }
	}

    char data[N + 1] { };
	usize current = 0LLU;
};


template <usize... Ns> constexpr auto concatenate_strings( const char ( &...args )[Ns] )
{
    constexpr usize size = ( 0 + ... + Ns );
    return CharBuffer<size>( args... );
}

}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// TEXT_RESET() -- Resets text formatting to default state
#define TEXT_RESET() \
	"\x01"

// TEXT_FONT( u8 font ) -- Custom font ID (accepted values: 0 - 255)
#define TEXT_FONT(font) \
	"\x02", CoreText::u8ToHex[static_cast<u8>( font )]

// TEXT_SIZE( u8 size ) -- Font size (accepted values: 0 - 255)
#define TEXT_SIZE(size) \
	"\x03", CoreText::u8ToHex[static_cast<u8>( size )]

// TEXT_COLOR( Color color )
#define TEXT_COLOR(color) \
	"\x04", CoreText::u8ToHex[color.r], CoreText::u8ToHex[color.g], \
            CoreText::u8ToHex[color.b], CoreText::u8ToHex[color.a]

// TEXT_COLOR_RGBA( u8 r, u8 g, u8, b, u8 a )
#define TEXT_COLOR_RGBA(r, g, b, a) \
	"\x04", CoreText::u8ToHex[static_cast<u8>( r )], CoreText::u8ToHex[static_cast<u8>( g )], \
	        CoreText::u8ToHex[static_cast<u8>( b )], CoreText::u8ToHex[static_cast<u8>( a )]

// TEXT_COLOR_RGB( u8 r, u8 g, u8, b )
#define TEXT_COLOR_RGB(r, g, b, a) \
	"\x04", CoreText::u8ToHex[static_cast<u8>( r )], CoreText::u8ToHex[static_cast<u8>( g )], \
	        CoreText::u8ToHex[static_cast<u8>( b )], CoreText::u8ToHex[255]

// TEXT_BOLD( bool enable ) -- Enable/Disable bold (accepted values: true/false)
#define TEXT_BOLD(enable) \
	"\x05", "\x01", CoreText::u8ToHex[static_cast<u8>( enable )]

// TEXT_ITALICS( bool enable ) -- Enable/Disable italics (accepted values: true/false)
#define TEXT_ITALICS(enable) \
	"\x05", "\x02", CoreText::u8ToHex[static_cast<u8>( enable )]

// TEXT_UNDERLINE( bool enable ) -- Enable/Disable underline (accepted values: true/false)
#define TEXT_UNDERLINE(enable) \
	"\x05", "\x03", CoreText::u8ToHex[static_cast<u8>( enable )]

// TEXT_HIGHLIGHT( bool enable ) -- Enable/Disable highlight (accepted values: true/false)
#define TEXT_HIGHLIGHT(enable) \
	"\x05", "\x04", CoreText::u8ToHex[static_cast<u8>( enable )]

// TEXT_ALIGN( u8 alignment ) -- Text Alignment (accepted values: align_left, align_center, align_right)
#define TEXT_ALIGN(alignment) \
	"\x05", "\x05", CoreText::u8ToHex[static_cast<u8>( alignment )]

// TEXT_ALIGN( name, ... ) -- Begin a text buffer definition; Usage: static constexpr auto text = TEXT_DEFINE( ... )
#define TEXT_DEFINE(...) CoreText::concatenate_strings( __VA_ARGS__ )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class TextEditor
{
public:
	TextEditor()
	{
		text.callbackOnError = text_callback_error;
		text.callbackOnUpdate = text_callback_update;
	}

	~TextEditor();

public:
	static void listen();
	void activate();
	void deactivate();
	void update( u64 codepoint );
	void update( const char *buffer );
	void clear();

	void event_mouse_click();
	void event_mouse_select();
	void event_left();
	void event_right();
	void event_up();
	void event_down();
	void event_backspace();
	void event_insert();
	void event_delete();
	void event_home();
	void event_end();
	void event_pageup();
	void event_pagedown();
	void event_tab();
	void event_newline();
	void event_select_all();
	void event_cut();
	void event_copy_clipboard();
	void event_paste_clipboard();
	void event_copy_selection();
	void event_paste_selection();

	bool highlighting() const { return ( caretEnd != USIZE_MAX && caretStart < caretEnd ); }
	usize caret_get_position() const { return ( highlighting() ? caretSeek : caretStart ); }
	usize caret_get_start() const { return caretStart; }
	usize caret_get_end() const { return caretEnd; }

	void caret_alert( Color color );
	void caret_set_color( Color color );
	void caret_set_root( usize position );
	void caret_set_position( usize position );
	void caret_set_position_start();
	void caret_set_position_end();
	void caret_set_selection( usize start, usize end );
	void caret_set_selection_word( usize index );
	void caret_set_selection_paragraph( usize index );
	void caret_reset_selection();

	int_v2 draw( Delta delta, float x, float y, Alpha alpha = { } );

	// Handle
	Text *operator->() { MemoryAssert( text.data != nullptr ); return &text; }

	// Indexer
	TextChar &operator[]( usize index ) { return text.char_at( index ); }
	const TextChar &operator[]( usize index ) const { return text.char_at( index ); }

private:
	void poll();
	void text_append( const char *buffer );
	usize text_replace( usize start, usize end, const char *buffer );

	u32 codepoint_at( usize index ) const;
	void validate_selection();

private:
	static void text_callback_error( Text &text, TextErr error );
	static void text_callback_update( Text &text );

private:
	Text text;
	usize caretStart = 0; // Insertion & highlight begin
	usize caretEnd = USIZE_MAX; // Highlight end
	usize caretRoot = USIZE_MAX; // Root for shift selection seeking
	usize caretSeek = USIZE_MAX; // Current shift selection seek

	float caretTimer = 0.0;
	float caretTimerAlert = 0.0;
	Color caretColor = c_white;
	Color caretColorAlert = c_white;

	usize clickIndex = USIZE_MAX;
	usize clickStart = USIZE_MAX;
	usize clickEnd = USIZE_MAX;
	u8 clickCount = 0;

public:
	bool control = false;
	bool shifting = false;
	bool selecting = false;
	bool inserting = false;
	bool allowUTF8 = false;
	bool allowNewlines = false;
	bool allowTabs = false;
	bool allowClick = true;
	int screenX = 0;
	int screenY = 0;

public:
	// void callbackUpdate( TextEditor &ime )
	void ( *callbackOnUpdate )( TextEditor & ) = nullptr;

	// void callbackListen( TextEditor &ime )
	void ( *callbackOnListen )( TextEditor & ) = nullptr;

	// void callbackError( TextEditor &ime, TextErr error )
	void ( *callbackOnError )( TextEditor &, TextErr ) = nullptr;

	// void callbackDraw( TextEditor &ime, Delta delta detla, float x, float y )
	void ( *callbackOnDraw )( TextEditor &, Delta, float, float, Alpha ) = nullptr;
};


namespace CoreText
{
	extern TextEditor *ACTIVE_TEXT_EDITOR;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////