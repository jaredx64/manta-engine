#include <manta/text.hpp>

#include <vendor/string.hpp>
#include <vendor/stdarg.hpp>
#include <vendor/stdio.hpp>
#include <vendor/vendor.hpp>

#include <core/debug.hpp>
#include <core/types.hpp>
#include <core/memory.hpp>
#include <core/utf8.hpp>
#include <core/string.hpp>

#include <manta/draw.hpp>
#include <manta/assets.hpp>
#include <manta/input.hpp>
#include <manta/time.hpp>
#include <manta/window.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static constexpr u32 CARET_CODEPOINT = 'A';
static constexpr float CARET_PADDING = 1.20f;
static constexpr float LINE_SPACING = 1.33f;

static Timer CLICK_TIMER;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool process_formatting_codes( TextFormat &format, const char *&string, char &c )
{
	// Formatting codes
	switch( c )
	{
		// RESET
		case '\x01':
			format = TextFormat { };
		return true;

		// FONT
		case '\x02':
			format.font = static_cast<u8>( ( c = *string++ ) );
		return true;

		// SIZE
		case '\x03':
			format.size = static_cast<u8>( ( c = *string++ ) );
		return true;

		// COLOR
		case '\x04':
			format.color.r = static_cast<u8>( ( c = *string++ ) );
			format.color.g = static_cast<u8>( ( c = *string++ ) );
			format.color.b = static_cast<u8>( ( c = *string++ ) );
			format.color.a = static_cast<u8>( ( c = *string++ ) );
		return true;

		// STYLE FLAGS
		case '\x05':
			switch( ( c = *string++ ) )
			{
				// BOLD
				case '\x01':
					format.bold = static_cast<bool>( ( c = *string++ ) );
					Assert( c <= '\1' );
				return true;

				// ITALICS
				case '\x02':
					format.italic = static_cast<bool>( ( c = *string++ ) );
					Assert( c <= '\1' );
				return true;

				// UNDERLINE
				case '\x03':
					format.underline = static_cast<bool>( ( c = *string++ ) );
					Assert( c <= '\1' );
				return true;

				// HIGHLIGHT
				case '\x04':
					format.highlight = static_cast<bool>( ( c = *string++ ) );
					Assert( c <= '\1' );
				return true;

				// ALIGNMENT
				case '\x05':
					format.alignment = static_cast<u8>( ( c = *string++ ) );
					Assert( c <= '\3' );
				return true;

				default: AssertMsg( false, "Corrupt format codes!" ); return false;
			}
		return false;

		// PASS THROUGH
		default: return false;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool TextChar::is_whitespace() const
{
	return codepoint == ' ' || codepoint == '\t' || codepoint == '\n';
}


bool TextChar::is_newline() const
{
	return codepoint == '\n';
}


u16 TextChar::get_ttf() const
{
	return Assets::font( format.font ).ttfs[( format.bold << 1 ) | ( format.italic << 0 )];
}


CoreFonts::FontGlyphInfo &TextChar::get_glyph() const
{
	const u16 ttf = get_ttf();
	return CoreFonts::get( CoreFonts::FontGlyphKey { codepoint, ttf, format.size } );
}


u16_v2 TextChar::get_glyph_dimensions( const usize index ) const
{
	static usize indexInLine = 0;
	if( UNLIKELY( index == 0 ) ) { indexInLine = 0; }

	// Tabs
	if( UNLIKELY( codepoint == '\t' ) )
	{
		const u16 ttf = get_ttf();
		const CoreFonts::FontGlyphInfo &glyph = CoreFonts::get( CoreFonts::FontGlyphKey { ' ', ttf, format.size } );
		const u16 tabSize = 4 - ( indexInLine % 4 );
		indexInLine += tabSize;
		return u16_v2 { static_cast<u16>( tabSize * glyph.advance ) , static_cast<u16>( glyph.height ) };
	}
	// Newlines
	else if( UNLIKELY( codepoint == '\n' ) )
	{
		indexInLine++;
		const u16 ttf = get_ttf();
		const CoreFonts::FontGlyphInfo &glyph = CoreFonts::get( CoreFonts::FontGlyphKey { ' ', ttf, format.size } );
		return u16_v2 { static_cast<u16>( format.size / 2 ), static_cast<u16>( glyph.height ) };
	}
	// Normal character
	else
	{
		indexInLine++;
		const CoreFonts::FontGlyphInfo &glyph = get_glyph();
		return u16_v2 { static_cast<u16>( glyph.advance ), static_cast<u16>( glyph.height ) };
	}
}


u16_v2 TextChar::get_glyph_dimensions_raw() const
{
	const CoreFonts::FontGlyphInfo &glyph = get_glyph();
	return u16_v2 { static_cast<u16>( glyph.advance ), static_cast<u16>( glyph.height ) };
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Text::grow()
{
	MemoryAssert( data != nullptr );
	if( capacity == 0 ) { capacity = 1; } else { capacity = capacity > USIZE_MAX / 2 ? USIZE_MAX : capacity * 2; }

	// Reallocate memory
	data = reinterpret_cast<TextChar *>( memory_realloc( data, capacity * sizeof( TextChar ) ) );
	ErrorIf( data == nullptr, "Failed to allocate memory for grow Text (%p: alloc %d bytes)",
		data, capacity * sizeof( TextChar ) );
}


void Text::init( const char *string )
{
	// Set state
	capacity = 1LLU;
	current = 0LLU;

	// Allocate memory
	MemoryAssert( data == nullptr );
	data = reinterpret_cast<TextChar *>( memory_alloc( capacity * sizeof( TextChar ) ) );

	// Default Format
	defaultFormat = TextFormat { };

	// Append string
	if( string[0] != '\0' ) { append( string ); };
}


void Text::free()
{
	if( data == nullptr )
	{
	#if true // MEMORY_RAII
		return;
	#else
		MemoryWarning( "Text: attempting to free() string that is already freed!" ); return;
	#endif
	}

	// Free memory
	memory_free( data );
	data = nullptr;

	// Reset state
	capacity = 0LLU;
	current = 0LLU;
}


Text &Text::copy( const Text &other )
{
	MemoryAssert( other.data != nullptr );
	if( this == &other ) { return *this; }
	if( data != nullptr ) { free(); }

	// Copy state
	capacity = other.capacity;
	current = other.current;

	// Allocate memory
	MemoryAssert( data == nullptr );
	data = reinterpret_cast<TextChar *>( memory_alloc( capacity * sizeof( TextChar ) ) );
	memory_copy( data, other.data, current * sizeof( TextChar ) );

	// Return this
	return *this;
}


Text &Text::move( Text &&other )
{
	MemoryAssert( other.data != nullptr );
	if( this == &other ) { return *this; }
	if( data != nullptr ) { free(); }

	// Move the other Text's resources
	data = other.data;
	capacity = other.capacity;
	current = other.current;

	// Reset other Text to null state
	other.data = nullptr;
	other.capacity = 0LLU;
	other.current = 0LLU;

	// Return this
	return *this;
}


void Text::clear()
{
	MemoryAssert( data != nullptr );

	current = 0LLU;
	if( callbackOnUpdate != nullptr ) { callbackOnUpdate( *this ); }
}


void Text::remove( const usize index, const usize count )
{
	MemoryAssert( data != nullptr );
	Assert( index < current && index + count <= current );
	if( count == 0 ) { return; }

	// Shift the right side of the string over
	const usize shift = current - ( index + count );
	memory_move( &data[index], &data[index + count], shift * sizeof( TextChar ) );
	current -= count;

	if( callbackOnUpdate != nullptr ) { callbackOnUpdate( *this ); }
}


usize Text::append( const TextChar &c )
{
	MemoryAssert( data != nullptr );

	if( !c ) { return 0; }
	if( limit_characters() ) { return 0; }
	if( filter != nullptr && !filter( *this, c.codepoint ) ) { return 0; }

	if( current == capacity ) { grow(); }
	memory_copy( &data[current], &c, sizeof( TextChar ) );
	current++;

	if( limit_dimensions() ) { remove( current - 1, 1 ); return 0; }

	if( callbackOnUpdate != nullptr ) { callbackOnUpdate( *this ); }
	return 1;
}


usize Text::append( const char *string, TextFormat format )
{
	MemoryAssert( data != nullptr );
	usize count = 0;
	u32 state = UTF8_ACCEPT;
	u32 codepoint;
	char utf8[5];
	char c;

	while( ( c = *string++ ) != '\0' )
	{
		// Process Formatting Codes
		if( process_formatting_codes( format, string, c ) ) { continue; }

		// Decode UTF8
		if( utf8_decode( &state, &codepoint, c ) != UTF8_ACCEPT ) { continue; }

		// Append Character
		count += append( TextChar { codepoint, format } );
	}

	return count;
}


usize Text::append( const char *string )
{
	MemoryAssert( data != nullptr );
	return append( string, get_format( current ) );
}



usize Text::append( const Text &string )
{
	MemoryAssert( data != nullptr );
	MemoryAssert( string.data != nullptr );
	usize count = 0;
	for( usize i = 0; i < string.length(); i++ ) { count += append( string.data[i] ); }
	return count;
}


usize Text::insert( const usize index, const TextChar &c )
{
	MemoryAssert( data != nullptr );

	if( !c ) { return 0; }
	if( limit_characters() ) { return 0; }
	if( filter != nullptr && !filter( *this, c.codepoint ) ) { return 0; }

	// Grow data (if necessary)
	Assert( index <= current );
	for( ; current + 1 > capacity; grow() ) { }

	// Move characters to the right & insert character
	const usize shift = current - index;
	memory_move( &data[index + 1], &data[index], shift * sizeof( TextChar ) );
	memory_copy( &data[index], &c, sizeof( TextChar ) );
	current++;

	if( limit_dimensions() ) { remove( index, 1 ); return 0; }

	if( callbackOnUpdate != nullptr ) { callbackOnUpdate( *this ); }
	return 1;
}


usize Text::insert( const usize index, const char *string, TextFormat format )
{
	MemoryAssert( data != nullptr );
	usize count = 0;
	u32 state = UTF8_ACCEPT;
	u32 codepoint;
	char utf8[5];
	char c;

	while( ( c = *string++ ) != '\0' )
	{
		// Process Formatting Codes
		if( process_formatting_codes( format, string, c ) ) { continue; }

		// Decode UTF8
		if( utf8_decode( &state, &codepoint, c ) != UTF8_ACCEPT ) { continue; }

		// Append Character
		count += insert( index + count, { codepoint, format } );
	}

	return count;
}


usize Text::insert( const usize index, const char *string )
{
	MemoryAssert( data != nullptr );
	return insert( index, string, get_format( index ) );
}


usize Text::insert( const usize index, const Text &string )
{
	MemoryAssert( data != nullptr );
	MemoryAssert( string.data != nullptr );
	usize count = 0;
	for( usize i = 0; i < string.length(); i++ ) { count += insert( index + count, string.data[i] ); }
	return count;
}


usize Text::size_allocated_bytes() const
{
	return capacity * sizeof( TextChar );
}


usize Text::length_bytes() const
{
	usize size = 1; // Null terminator
	for( usize i = 0; i < current; i++ ) { size += utf8_codepoint_size( data[i].codepoint ); }
	return size;
}


usize Text::length() const
{
	return current;
}


TextChar &Text::char_at( const usize index )
{
	 MemoryAssert( data != nullptr );
	 Assert( index < current );
	 return data[index];
}


const TextChar &Text::char_at( const usize index ) const
{
	 MemoryAssert( data != nullptr );
	 Assert( index < current );
	 return data[index];
}


void Text::string( String &string )
{
	MemoryAssert( data != nullptr );
	string.clear();

	// Encode TextChars to output sting
	char buffer[5];
	for( usize i = 0; i < current; i++ )
	{
		buffer[utf8_encode( buffer, data[i].codepoint )] = '\0';
		string.append( buffer );
	}
};


String Text::substr( const usize start, const usize end )
{
	MemoryAssert( data != nullptr );
	Assert( start < end );
	Assert( end <= current );
	String string;

	// Encode TextChars to output sting
	char buffer[5];
	for( usize i = start; i < end; i++ )
	{
		buffer[utf8_encode( buffer, data[i].codepoint )] = '\0';
		string.append( buffer );
	}

	return string;
};


void Text::cstr( char *buffer, const usize size )
{
	// Size check
	MemoryAssert( data != nullptr );
	const usize bytes = length_bytes();
	if( size < bytes ) { Warning( "Text::cstr() buffer not large enough: %llu, needs %llu", size, bytes ); }

	// Encode TextChars to output buffer
	usize offset = 0;
	for( usize i = 0; i < current; i++ )
	{
		if( offset + utf8_codepoint_size( data[i].codepoint ) >= size ) { break; }
		offset += utf8_encode( &buffer[offset], data[i].codepoint );
	}
	buffer[offset] = '\0';
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void draw_glyph( const float x, const float y, const float xOffset, const float yOffset,
	const CoreFonts::FontGlyphInfo &glyph, const Color color )
{
	const float glyphX1 = x + xOffset + glyph.xshift;
	const float glyphY1 = y + yOffset + glyph.yshift;
	const float glyphX2 = x + xOffset + glyph.xshift + glyph.width;
	const float glyphY2 = y + yOffset + glyph.yshift + glyph.height;

	constexpr u16 uvScale = ( 1 << 16 ) / CoreFonts::FONTS_TEXTURE_SIZE;
	const u16 u1 = ( glyph.u ) * uvScale;
	const u16 v1 = ( glyph.v ) * uvScale;
	const u16 u2 = ( glyph.u + glyph.width ) * uvScale;
	const u16 v2 = ( glyph.v + glyph.height ) * uvScale;

	// TODO: Implement this properly...
	if( UNLIKELY( Gfx::quad_batch_can_break() ) ||
		UNLIKELY( CoreGfx::state.boundTexture[0] != CoreFonts::glyphAtlasTexture.resource ) )
	{
		CoreFonts::update();
	}

	Gfx::quad_batch_write( glyphX1, glyphY1, glyphX2, glyphY2, u1, v1, u2, v2, color, &CoreFonts::glyphAtlasTexture, 0.0f );
}


TextFormat Text::get_format( const usize index ) const
{
	if( index > 0 ) { return data[index - 1].format; }
	if( index == 0 && current > 0 ) { return data[index].format; }
	return defaultFormat;
}


Text::LineInfo Text::get_line( const usize index )
{
	Assert( index <= current );
	LineInfo line { };
	line.begin = index;

	if( current > 0 )
	{
		// Tracking variables for line width breaking
		u16 wordBreakWidth = 0;
		u16 wordBreakHeight = 0;
		usize wordBreakIndex = 0;

		// Calculate line height and find line end
		TextChar charPrevious;
		for( usize i = line.begin; i < current; i++ )
		{
			const TextChar c = data[i];

			// Alignment
			if( c.format.alignment != 0 ) { line.alignment = c.format.alignment; }

			// Newline
			if( c.codepoint == '\n' )
			{
				line.end = i;
				line.next = i + 1;
				break;
			}

			// Line Break (exceeded width)
			if( i > line.begin && !charPrevious.is_whitespace() && c.is_whitespace() )
			{
				wordBreakWidth = line.width;
				wordBreakHeight = line.height;
				wordBreakIndex = i;
			}

			// Line Width
			const u16_v2 glyphDimensions = c.get_glyph_dimensions( i - line.begin );
			if( pageWidth > 0 && line.width + glyphDimensions.x >= pageWidth )
			{
				if( wordBreakIndex != 0 )
				{
					// Split line after last word if possible
					line.width = wordBreakWidth;
					line.height = wordBreakHeight;
					line.end = wordBreakIndex;
					line.next = line.end + 1;
				}
				else
				{
					// Otherwise, split character-wise
					line.end = i;
					line.next = line.end + ( line.end == line.begin ? 1 : 0 );
				}

				break;
			}
			line.width += glyphDimensions.x;

			// Line Height
			line.height = c.format.size > line.height ? c.format.size : line.height;

			// Update charPrevious
			charPrevious = c;
		}
	}

	// Validate End Index
	if( line.end == USIZE_MAX ) { line.end = current; }

	// Empty Line Size
	if( line.height == 0 )
	{
		const TextFormat format = get_format( index );
		line.height = format.size;
		line.alignment = format.alignment;
	}

	// Line Spacing
	if( line.begin > 0 ) { line.height = static_cast<int>( line.height * LINE_SPACING ); }

	// Horizontal Alignment
	line.offset = 0;
	const u16 width = min( pageWidth > 0 ? pageWidth : limitWidth, limitWidth > 0 ? limitWidth : pageWidth );
	if( width > 0 )
	{
		if( line.alignment == Align_Center ) { line.offset = ( width / 2 ) - ( line.width / 2 ); } else
		if( line.alignment == Align_Right )  { line.offset = width - line.width; }
	}

	// Return
	return line;
}


usize Text::get_index_from_position( const int x, const int y, const float bias )
{
	// Early out for empty Text
	if( current == 0 ) { return 0; }

	// Find line containing Y
	int yOffset = 0;
	for( LineInfo line = get_line( 0 ); true; line = get_line( line.next ) )
	{
		// Y is within or above line, or we're the last line
		if( y < yOffset + line.height || line.end == current )
		{
			int xOffset = line.offset;
			for( usize i = line.begin; i < line.end; i++ )
			{
				// X is within or before glyph, return this index
				const u16_v2 glyphDimensions = data[i].get_glyph_dimensions( i - line.begin );
				if( x < xOffset + glyphDimensions.x * bias ) { return i; }
				xOffset += glyphDimensions.x;
			}

			// If we've reached the here, X must be beyond the line/textbox; return end index
			return line.end;
		}

		// Increment yOffset
		yOffset += line.height;
		Assert( line.next != USIZE_MAX );
	}

	// If we've reached here, Y must be below the textbox; return final index
	return current;
}


int_v3 Text::get_position_from_index( const usize index )
{
	// Early out for empty Text

	AssertMsg( index <= current, "%llu, %llu", index, current );

	// Find Caret
	int yOffset = 0;
	for( LineInfo line = get_line( 0 ); true; line = get_line( line.next ) )
	{
		// Caret is within or above the line, we must determine the character
		if( index < line.next )
		{
			// Calculate xOffset within the line
			int xOffset = line.offset;
			for( usize i = line.begin; i < index; i++ )
			{
				const u16_v2 glyphDimensions = data[i].get_glyph_dimensions( i - line.begin );
				xOffset += glyphDimensions.x;
			}

			return int_v3 { xOffset, yOffset, yOffset + line.height };
		}

		// Incremenet yOffset
		yOffset += line.height;
		Assert( line.next != USIZE_MAX );
	}
}


int_v2 Text::get_dimensions()
{
	int_v2 dimensions { 0, 0 };

	// Calculate dimensions
	for( LineInfo line = get_line( 0 ); true; line = get_line( line.next ) )
	{
		dimensions.x = max( dimensions.x, static_cast<int>( line.width ) );
		dimensions.y += line.height;
		if( line.next == USIZE_MAX ) { break; }
	}

	return dimensions;
};


bool Text::limit_characters()
{
	// No character limit
	if( limitCharacters == 0 ) { return false; }

	// Test character limit
	const bool limitExceeded = current >= limitCharacters;
	if( limitExceeded && callbackOnError ) { callbackOnError( *this, TextErr_LimitCharacter ); }
	return limitExceeded;
}


bool Text::limit_dimensions()
{
	if( limitWidth == 0 && limitHeight == 0 ) { return false; }

	const int_v2 dimensions = get_dimensions();

	const bool limitExceededWidth = pageWidth == 0 && limitWidth > 0 && dimensions.x > limitWidth;
	const bool limitExceededHeight = limitHeight > 0 && dimensions.y > limitHeight;
	const bool limitExceeded = limitExceededWidth || limitExceededHeight;

	if( limitExceeded && callbackOnError ) { callbackOnError( *this, TextErr_LimitCharacter ); }
	return limitExceeded;
}


void Text::draw_selection( const float x, const float y, const usize begin, const usize end, const Alpha alpha )
{
	const Color colorSelection = Color { 0, 40, 100, 255 };

	Assert( begin <= end );
	Assert( begin <= current );
	Assert( end <= current );
	if( begin == end ) { return; }

	// Draw Selection Quads
	int yOffset = 0;
	for( LineInfo line = get_line( 0 ); true; line = get_line( line.next ) )
	{
		// Loop over all characters in the line
		int xOffset = line.offset;

		// Variables for batching
		int batchX1 = -1;
		int batchX2 = -1;
		int batchY1 = -1;
		int batchY2 = -1;

		for( usize i = line.begin; i <= line.end; i++ )
		{
			// Break Conditions
			if( begin > line.end ) { break; } // Seletion is beyond this line, continue
			if( end < line.begin ) { return; } // Selection before this line, we're done

			if( i >= end ) // We've reached the end of the selection, we're done
			{
				if( batchX1 != -1 )
				{
					draw_rectangle( x + batchX1, y + batchY1, x + batchX2, y + batchY2, colorSelection * alpha );
				}

				return;
			}

			// Increment xOffset
			TextChar c = data[i];
			if( i == line.end && !c.is_newline() ) { break; }
			const u16_v2 glyphDimensions = c.get_glyph_dimensions( i - line.begin );
			xOffset += glyphDimensions.x;

			// Calculate Quad Bounds
			if( i >= begin )
			{
				const CoreFonts::FontGlyphKey caretKey { CARET_CODEPOINT, c.get_ttf(), c.format.size };
				const CoreFonts::FontGlyphInfo &caretGlyph = CoreFonts::get( caretKey );

				const int caretHeight = caretGlyph.height >= c.format.size ? caretGlyph.height : c.format.size;
				const int caretHeightPadding = static_cast<int>( caretHeight * CARET_PADDING ) - caretHeight;
				const int lineHeightOffset = line.height - caretHeight;

				const int x1 = xOffset - glyphDimensions.x;
				const int x2 = xOffset;
				const int y1 = yOffset + caretGlyph.yshift + lineHeightOffset - caretHeightPadding;
				const int y2 = yOffset + caretGlyph.yshift + lineHeightOffset + caretHeight + caretHeightPadding;

				// Check for batching
				if( batchY1 != -1 && ( batchY1 != y1 || batchY2 != y2 ) )
				{
					// Draw the previous batch
					draw_rectangle( x + batchX1, y + batchY1, x + batchX2, y + batchY2, colorSelection * alpha );
					batchX1 = -1;
					batchX2 = -1;
				}

				// Update batch
				if( batchX1 == -1 )
				{
					batchX1 = x1;
					batchY1 = y1;
					batchY2 = y2;
				}
				batchX2 = x2;
			}
		}

		// Draw any remaining batch for the line
		if( batchX1 != -1 )
		{
			draw_rectangle( x + batchX1, y + batchY1, x + batchX2, y + batchY2, colorSelection * alpha );
		}

		// Increment yOffset
		yOffset += line.height;
		AssertMsg( line.next != USIZE_MAX,
			"Unreachable state. Are 'begin' (%llu) and 'end' (%llu) valid?", begin, end );
	}
}


void Text::draw_caret( const float x, const float y, const usize index, float_v4 *outCorners, const Alpha alpha )
{
	Assert( index <= current );

	// Find Caret
	int yOffset = 0;
	for( LineInfo line = get_line( 0 ); true; line = get_line( line.next ) )
	{
		// Caret is within or above the line, we must determine the character
		if( index < line.next )
		{
			// Calculate xOffset within the line
			int xOffset = line.offset;
			for( usize i = line.begin; i < index; i++ )
			{
				const u16_v2 glyphDimensions = data[i].get_glyph_dimensions( i - line.begin );
				xOffset += glyphDimensions.x;
			}

			// Draw Caret
			const TextChar c { CARET_CODEPOINT, get_format( index ) };
			const CoreFonts::FontGlyphKey caretKey { CARET_CODEPOINT, c.get_ttf(), c.format.size };
			const CoreFonts::FontGlyphInfo &caretGlyph = CoreFonts::get( caretKey );

			const int caretHeight = caretGlyph.height >= c.format.size ? caretGlyph.height : c.format.size;
			const int caretHeightPadding = static_cast<int>( caretHeight * CARET_PADDING ) - caretHeight;
			const int lineHeightOffset = line.height - caretHeight;

			const int x1 = xOffset;
			const int x2 = xOffset + 2;
			const int y1 = yOffset + caretGlyph.yshift + lineHeightOffset - caretHeightPadding;
			const int y2 = yOffset + caretGlyph.yshift + lineHeightOffset + caretHeight + caretHeightPadding;
			const float_v4 corners { x + x1, y + y1, x + x2, y + y2 };

			// Draw
			if( outCorners == nullptr )
			{
				draw_rectangle( corners.x, corners.y, corners.z, corners.w, c_white * alpha );
			}
			else
			{
				*outCorners = corners;
			}

			// Return
			return;
		}

		// Caret is beyond the line, continue loop if possible
		yOffset += line.height;
		AssertMsg( line.end != current, "Unreachable state. Is 'caret' (%llu) valid?", index );
	}
}


int_v2 Text::draw( const float x, const float y, const Alpha alpha )
{
	int_v2 dimensions { 0, 0 };

	// Loop over lines
	int yOffset = 0;
	for( LineInfo line = get_line( 0 ); true; line = get_line( line.next ) )
	{
		// Loop over characters
		int xOffset = line.offset;
		for( usize i = line.begin; i < line.end; i++ )
		{
			// Retrieve glyph
			const TextChar &c = data[i];
			const CoreFonts::FontGlyphInfo &glyph = c.get_glyph();
			const u16_v2 glyphDimensions = c.get_glyph_dimensions( i - line.begin );

			// Skipped characters
			const bool skipped = c.codepoint == '\t' || c.codepoint == '\n';

			// Draw glyph
			if( !skipped && glyph.width != 0 && glyph.height != 0 )
			{
				const CoreFonts::FontGlyphKey glyphKeySizeChar { CARET_CODEPOINT, c.get_ttf(), c.format.size };
				const float lineHeightOffset = line.height - CoreFonts::get( glyphKeySizeChar ).height;
				draw_glyph( x + xOffset, y + yOffset, 0, lineHeightOffset, glyph, c.format.color * alpha );
			}

			// Increment xOffset
			xOffset += glyphDimensions.x;
			dimensions.x = max( dimensions.x, static_cast<int>( xOffset ) );
		}

		// Increment yOffset
		yOffset += line.height;
		dimensions.y = yOffset;

		// Break loop
		if( line.next == USIZE_MAX ) { break; }
	}

	return dimensions;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Text::filter_ascii( Text &text, u32 codepoint )
{
	// Allow ASCII ranges
	if( codepoint >= 32 && codepoint < 127 ) { return true; }
	if( codepoint == '\t' ) { return true; }
	if( codepoint == '\n' ) { return true; }

	// Failure
	return true;
}


bool Text::filter_emoji( Text &text, u32 codepoint )
{
	// TODO ...

	// Failure
	if( text.callbackOnError != nullptr ) { text.callbackOnError( text, TextErr_IllegalCharacter ); }
	return false;
}


bool Text::filter_number( Text &text, u32 codepoint )
{
	// Allow digits
	if( codepoint >= '0' && codepoint <= '9' ) { return true; }

	// Allow decimal if there isn't one
	bool hasDecimal = false;
	for( usize i = 0; i < text.length(); i++ ) { hasDecimal |= text.data[i].codepoint == '.'; }
	if( codepoint == '.' && !hasDecimal ) { return true; }

	// Allow '-' if on the first character
	if( codepoint == '-' && text.length() == 0 ) { return true; }

	// Failure
	if( text.callbackOnError != nullptr ) { text.callbackOnError( text, TextErr_IllegalCharacter ); }
	return false;
}


bool Text::filter_number_positive( Text &text, u32 codepoint )
{
	// Allow digits
	if( codepoint >= '0' && codepoint <= '9' ) { return true; }

	// Allow decimal if there isn't one
	bool hasDecimal = false;
	for( usize i = 0; i < text.length(); i++ ) { hasDecimal |= text.data[i].codepoint == '.'; }
	if( codepoint == '.' && !hasDecimal ) { return true; }

	// Failure
	if( text.callbackOnError != nullptr ) { text.callbackOnError( text, TextErr_IllegalCharacter ); }
	return false;
}


bool Text::filter_integer( Text &text, u32 codepoint )
{
	// Allow digits
	if( codepoint >= '0' && codepoint <= '9' ) { return true; }

	// Allow '-' if on the first character
	if( codepoint == '-' && text.length() == 0 ) { return true; }

	// Failure
	if( text.callbackOnError != nullptr ) { text.callbackOnError( text, TextErr_IllegalCharacter ); }
	return false;
}


bool Text::filter_integer_positive( Text &text, u32 codepoint )
{
	// Allow digits
	if( codepoint >= '0' && codepoint <= '9' ) { return true; }

	// Failure
	if( text.callbackOnError != nullptr ) { text.callbackOnError( text, TextErr_IllegalCharacter ); }
	return false;
}


bool Text::filter_hex( Text &text, u32 codepoint )
{
	// Allow digits
	if( codepoint >= '0' && codepoint <= '9' ) { return true; }

	// Allow A - F
	if( codepoint >= 'a' && codepoint <= 'f' ) { return true; }
	if( codepoint >= 'A' && codepoint <= 'F' ) { return true; }

	// Failure
	if( text.callbackOnError != nullptr ) { text.callbackOnError( text, TextErr_IllegalCharacter ); }
	return false;
}


bool Text::filter_boolean( Text &text, u32 codepoint )
{
	// Only allow a '0' or '1'
	if( text.length() == 0 && ( codepoint == '0' || codepoint == '1' ) ) { return true; }

	// Failure
	if( text.callbackOnError != nullptr ) { text.callbackOnError( text, TextErr_IllegalCharacter ); }
	return false;
}


bool Text::filter_filename( Text &text, u32 codepoint )
{
	// Illegal characters
	constexpr const char *illegalChars = "\t\n/\\<>:\"|?*.";
	for( usize i = 0; illegalChars[i] != '\0'; i++ )
	{
		if( codepoint == static_cast<u32>( illegalChars[i] ) ) { goto error; }
	}

	// Only allow ASCII (TODO: Fix this)
	if( !filter_ascii( text, codepoint ) ) { goto error; }

	// Cannot start with a space
	if( codepoint == ' ' && text.length() == 0 ) { goto error; }

	// Success
	return true;

error:
	// Failure
	if( text.callbackOnError != nullptr ) { text.callbackOnError( text, TextErr_IllegalCharacter ); }
	return false;
}


bool Text::filter_console( Text &text, u32 codepoint )
{
	// Illegal characters
	constexpr const char *illegalChars = "\t\n";
	for( usize i = 0; illegalChars[i] != '\0'; i++ )
	{
		if( codepoint == static_cast<u32>( illegalChars[i] ) ) { goto error; }
	}

	// Only allow ASCII (TODO: Fix this)
	if( !filter_ascii( text, codepoint ) ) { goto error; }

	// Cannot start with a space
	if( codepoint == ' ' && text.length() == 0 ) { goto error; }

	// Success
	return true;

error:
	// Failure
	if( text.callbackOnError != nullptr ) { text.callbackOnError( text, TextErr_IllegalCharacter ); }
	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace CoreText
{
	TextEditor *ACTIVE_TEXT_EDITOR = nullptr;
}

TextEditor::~TextEditor()
{
	if( !Debug::memoryLeakDetection ) { return; }
	AssertMsg( CoreText::ACTIVE_TEXT_EDITOR != this, "Actively bound TextEditor is being destructed!" );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool codepoint_is_whitespace( const u32 codepoint )
{
	return codepoint == static_cast<u32>( ' ' ) ||
	       codepoint == static_cast<u32>( '\t' ) ||
		   codepoint == static_cast<u32>( '\n' );
}


static bool codepoint_is_newline( const u32 codepoint )
{
	return codepoint == static_cast<u32>( '\n' );
}


static bool codepoint_is_tab( const u32 codepoint )
{
	return codepoint == static_cast<u32>( '\t' );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TextEditor::listen()
{
	if( CoreText::ACTIVE_TEXT_EDITOR == nullptr ) { return; }
	CoreText::ACTIVE_TEXT_EDITOR->poll();
}


void TextEditor::activate()
{
	if( CoreText::ACTIVE_TEXT_EDITOR == this ) { return; }
	CLICK_TIMER.stop();
	CoreText::ACTIVE_TEXT_EDITOR = this;
}


void TextEditor::deactivate()
{
	if( CoreText::ACTIVE_TEXT_EDITOR != this ) { return; }
	CLICK_TIMER.stop();
	CoreText::ACTIVE_TEXT_EDITOR = nullptr;
}


void TextEditor::update( const u64 codepoint )
{
	// Append codepoint
	char buffer[6];
	utf8_encode( buffer, codepoint );
	update( buffer );
}


void TextEditor::update( const char *buffer )
{
	// Append buffer
	u32 state = UTF8_ACCEPT;
	u32 codepoint;
	char utf8[5];
	char c;

	while( ( c = *buffer++ ) != '\0' )
	{
		if( utf8_decode( &state, &codepoint, c ) != UTF8_ACCEPT ) { continue; }

		// Exclude UTF8?
		if( codepoint >= 0x0000007E && !allowUTF8 ) { continue; }

		// Control characters (U+0000–U+001F) and DEL (U+007F)
		if( codepoint >= 0x00000000 && codepoint <= 0x0000001F ) { continue; }

		// Non-characters (U+FDD0–U+FDEF)
		if( codepoint >= 0x0000FDD0 && codepoint <= 0x0000FDEF ) { continue; }

		// Surrogate code points (U+D800–U+DFFF)
		if( codepoint >= 0x0000D800 && codepoint <= 0x0000DFFF ) { continue; }

		// Private use areas
		if( codepoint >= 0x0000E000 && codepoint <= 0x0000F8FF ) { continue; }
		if( codepoint >= 0x000F0000 && codepoint <= 0x000FFFFD ) { continue; }
		if( codepoint >= 0x00100000 && codepoint <= 0x0010FFFD ) { continue; }

		// Embedding and overrides
		if( codepoint >= 0x0000202A && codepoint <= 0x0000202E ) { continue; }

		switch( codepoint )
		{
			// Non-characters ending in FFFE or FFFF
			case 0x0000FFFE: case 0x0000FFFF:
			case 0x0001FFFE: case 0x0001FFFF:
			case 0x0002FFFE: case 0x0002FFFF:
			case 0x0003FFFE: case 0x0003FFFF:
			case 0x0004FFFE: case 0x0004FFFF:
			case 0x0005FFFE: case 0x0005FFFF:
			case 0x0006FFFE: case 0x0006FFFF:
			case 0x0007FFFE: case 0x0007FFFF:
			case 0x0008FFFE: case 0x0008FFFF:
			case 0x0009FFFE: case 0x0009FFFF:
			case 0x000AFFFE: case 0x000AFFFF:
			case 0x000BFFFE: case 0x000BFFFF:
			case 0x000CFFFE: case 0x000CFFFF:
			case 0x000DFFFE: case 0x000DFFFF:
			case 0x000EFFFE: case 0x000EFFFF:
			case 0x000FFFFE: case 0x000FFFFF:
			case 0x0010FFFE: case 0x0010FFFF:
			break;

			// Line and paragraph separators
			case 0x00002028: // line separator
			case 0x00002029: // paragraph separator
			break;

			// Bidirectional formatting characters
			case 0x0000200E: // left-to-right mark
			case 0x0000200F: // right-to-left mark
			break;

			// Zero-width characters
			case 0x0000200B: // zero Width Space
			case 0x0000200C: // zero Width Non-Joiner
			case 0x0000200D: // zero Width Joiner
			break;

			// Replacement character
			case 0x0000FFFD:
			break;

			// Allowed Codepoints
			default:
			{
				utf8[utf8_encode( utf8, codepoint )] = '\0';
				text_append( utf8 );
			}
			break;
		}
	}
}


void TextEditor::clear()
{
	text.clear();
	caret_reset_selection();
	caret_set_position( 0 );
}


void TextEditor::event_mouse_click()
{
	const usize index = text.get_index_from_position( mouse_x - screenX, mouse_y - screenY, 0.5f );

	// Increment Click
	if( CLICK_TIMER.running() && CLICK_TIMER.elapsed_ms() < 500.0 &&
		clickIndex == index && clickCount < 2 )
	{
		// Consecutive Click
		clickCount++;
		CLICK_TIMER.start();
	}
	else
	{
		// Start Click Counter
		clickCount = 0;
		clickIndex = index;
		clickStart = USIZE_MAX;
		clickEnd = USIZE_MAX;
		CLICK_TIMER.start();
	}

	// Validate Caret Selection
	validate_selection();

	// Click Types
	switch( clickCount )
	{
		case 0:
			caret_set_position( index );
		break;

		case 1:
			caret_set_selection_word( index );
			clickStart = caretStart;
			clickEnd = caretEnd;
		break;

		case 2:
			caret_set_selection_paragraph( index );
			clickStart = caretStart;
			clickEnd = caretEnd;
		break;
	}
}


void TextEditor::event_mouse_select()
{
	const usize index = text.get_index_from_position( mouse_x - screenX, mouse_y - screenY, 0.5f );

	if( clickStart != USIZE_MAX && clickEnd != USIZE_MAX )
	{
		if( index > clickEnd )
		{
			caret_set_root( clickStart );
			caret_set_position( index );
		}
		else if( index < clickStart )
		{
			caret_set_root( clickEnd );
			caret_set_position( index );
		}
		else
		{
			caret_set_selection( clickStart, clickEnd );
		}
	}
	else
	{
		caret_set_position( index );
	}
}


void TextEditor::event_left()
{
	validate_selection();
	const usize current = caret_get_position();
	const usize length = text.length();
	if( length == 0 || current == 0 ) { return; }

	if( control )
	{
		// Move left to next word
		bool sawChar = false;
		usize index = current - 1;
		while( index > 0 )
		{
			const u32 codepoint = codepoint_at( index - 1 );
			const bool whitespace = codepoint_is_whitespace( codepoint );
			sawChar |= !whitespace;
			if( whitespace && sawChar ) { break; }
			index--;
		}
		caret_set_position( index );
	}
	else
	{
		// Move left 1 codepoint
		caret_set_position( current - 1 );
	}
}


void TextEditor::event_right()
{
	validate_selection();
	const usize current = caret_get_position();
	const usize length = text.length();
	if( length == 0 || current == length ) { return; }

	if( control )
	{
		// Move right to next word
		bool sawChar = false;
		usize index = current;
		while( index < length )
		{
			const u32 codepoint = codepoint_at( index );
			const bool whitespace = codepoint_is_whitespace( codepoint );
			sawChar |= !whitespace;
			if( whitespace && sawChar ) { break; }
			index++;
		}
		caret_set_position( index );
	}
	else
	{
		// Move right 1 codepoint
		caret_set_position( current + 1 );
	}
}


void TextEditor::event_up()
{
	validate_selection();
	const usize current = caret_get_position();
	const usize length = text.length();
	if( length == 0 || current == 0 ) { return; }

	const int_v3 position = text.get_position_from_index( caret_get_position() );
	caret_set_position( text.get_index_from_position( position.x, position.y - 1, 0.5f ) );
}


void TextEditor::event_down()
{
	validate_selection();
	const usize current = caret_get_position();
	const usize length = text.length();
	if( length == 0 || current == length ) { return; }

	const int_v3 position = text.get_position_from_index( caret_get_position() );
	caret_set_position( text.get_index_from_position( position.x, position.z + 1, 0.5f ) );
}


void TextEditor::event_backspace()
{
	const usize caret = caretStart;

	// Selection highlighted, remove it
	if( highlighting() )
	{
		text_replace( caretStart, caretEnd, "" );
		caret_reset_selection();
		caretStart = caret;
	}
	// No highlight, remove preceeding character
	else if( caretStart > 0 )
	{
		text_replace( caretStart - 1, caretStart, "" );
		caret_reset_selection();
		caretStart = caret - 1;
	}
}


void TextEditor::event_insert()
{
	inserting = !inserting;
}


void TextEditor::event_delete()
{
	const usize caret = caretStart;

	// Selection highlighted, remove it
	if( highlighting() )
	{
		text_replace( caretStart, caretEnd, "" );
	}
	// No highlight, remove proceeding character
	else if( caretStart < text.length() )
	{
		text_replace( caretStart, caretStart + 1, "" );
	}

	caret_reset_selection();
	caretStart = caret;
}


void TextEditor::event_home()
{
	validate_selection();
	const usize current = caret_get_position();
	const usize length = text.length();
	if( length == 0 || current == 0 ) { return; }

	if( control )
	{
		// Move to start of string
		caret_set_position( 0 );
	}
	else
	{
		// Move right to next newline
		usize index = current - 1;
		while( index > 0 )
		{
			if( codepoint_is_newline( codepoint_at( index ) ) ) { index++; break; }
			index--;
		}
		caret_set_position( index );
	}
}


void TextEditor::event_end()
{
	validate_selection();
	const usize current = caret_get_position();
	const usize length = text.length();
	if( length == 0 || current == length ) { return; }

	if( control )
	{
		// Move to end of string
		caret_set_position( length );
	}
	else
	{
		// Move right to next newline
		usize index = current;
		while( index < length )
		{
			if( codepoint_is_newline( codepoint_at( index ) ) ) { break; }
			index++;
		}
		caret_set_position( index );
	}
}


void TextEditor::event_pageup()
{
	event_home();
}


void TextEditor::event_pagedown()
{
	event_end();
}


void TextEditor::event_tab()
{
	if( !allowTabs ) { return; }
	text_append( "\t" );
}


void TextEditor::event_newline()
{
	if( !allowNewlines ) { return; }
	text_append( "\n" );
}


void TextEditor::event_select_all()
{
	caret_set_selection( 0, text.length() );
}


void TextEditor::event_cut()
{
	if( !highlighting() ) { return; }

	event_copy_clipboard();
	event_delete();
}


void TextEditor::event_copy_clipboard()
{
	if( !highlighting() ) { return; }

	// Copy selection to clipboard
	String selection = text.substr( caret_get_start(), caret_get_end() );
	Window::set_clipboard( selection );
}


void TextEditor::event_paste_clipboard()
{
	char buffer[4096];
	Window::get_clipboard( buffer, sizeof( buffer ) );
	if( buffer[0] == '\0' ) { return; }
	text_append( buffer );
}


void TextEditor::event_copy_selection()
{
	if( !highlighting() ) { return; }

	static usize startPrevious = USIZE_MAX;
	static usize endPrevious = USIZE_MAX;
	const usize start = caret_get_start();
	const usize end = caret_get_end();

	if( start != startPrevious || end != endPrevious )
	{
		// Copy selection to clipboard
		String selection = text.substr( caret_get_start(), caret_get_end() );
		Window::set_selection( selection );
		startPrevious = start;
		endPrevious = end;
	}
}


void TextEditor::event_paste_selection()
{
	char buffer[4096];
	Window::get_selection( buffer, sizeof( buffer ) );
	if( buffer[0] == '\0' ) { return; }
	text_append( buffer );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TextEditor::caret_alert( const Color color )
{
	caretTimer = 0.0f;
	caretTimerAlert = 1.0f;
	caretColorAlert = color;
}


void TextEditor::caret_set_color( const Color color )
{
	caretColor = color;
}


void TextEditor::caret_set_root( const usize position )
{
	Assert( position <= text.length() );
	caretRoot = position;
	caretSeek = position;
}


void TextEditor::caret_set_position( const usize position )
{
	Assert( position <= text.length() );

	// Update Position
	if( caretRoot != USIZE_MAX )
	{
		caretSeek = position;
		caretStart = caretSeek < caretRoot ? caretSeek : caretRoot;
		caretEnd = caretSeek < caretRoot ? caretRoot : caretSeek;
	}
	else
	{
		// Set caret position
		caret_reset_selection();
		caretStart = position;
	}

#if OS_LINUX
	// Selection Clipboard (Linux)
	event_copy_selection();
#endif

	// Reset Timer
	caretTimer = 0.0f;
}


void TextEditor::caret_set_position_start()
{
	caret_set_position( 0 );
}


void TextEditor::caret_set_position_end()
{
	caret_set_position( text.current );
}


void TextEditor::caret_set_selection( const usize start, const usize end )
{
	Assert( start <= end );
	caret_set_root( start );
	caret_set_position( end );
}


void TextEditor::caret_set_selection_word( const usize index )
{
	validate_selection();
	const usize current = caret_get_position();
	const usize length = text.length();
	if( length == 0 ) { return; }

	usize start, end, seek;

	// Seach for word start
	bool sawChar = false;
	seek = current;
	while( seek > 0 )
	{
		if( seek == length ) { seek--; continue; }
		const u32 codepoint = codepoint_at( seek );
		const bool newline = codepoint_is_newline( codepoint );
		const bool whitespace = codepoint_is_whitespace( codepoint );
		sawChar |= !whitespace;
		if( newline && seek != current ) { break; }
		if( whitespace && sawChar ) { seek++; break; }
		seek--;
	}
	start = seek;

	// Search for word end
	bool sawWhitespace = false;
	seek = current;
	while( seek < length )
	{
		const u32 codepoint = codepoint_at( seek );
		const bool newline = codepoint_is_newline( codepoint );
		const bool whitespace = codepoint_is_whitespace( codepoint );
		sawWhitespace |= whitespace;
		if( newline || ( !whitespace && sawWhitespace ) ) { break; }
		seek++;
	}
	end = seek;

	// Update Selection
	caret_set_selection( start, end );
}


void TextEditor::caret_set_selection_paragraph( const usize index )
{
	validate_selection();
	const usize current = caret_get_position();
	const usize length = text.length();
	if( length == 0 ) { return; }

	usize start, end, seek;

	// Seach for paragraph start
	seek = current;
	while( seek > 0 )
	{
		if( seek == length ) { seek--; continue; }
		const bool newline = codepoint_is_newline( codepoint_at( seek ) );
		if( newline && seek != current ) { seek++; break; }
		seek--;
	}
	start = seek;

	// Search for word end
	seek = current;
	while( seek < length )
	{
		const bool newline = codepoint_is_newline( codepoint_at( seek ) );
		if( newline ) { break; }
		seek++;
	}
	end = seek;

	// Update Selection
	caret_set_selection( start, end );
}


void TextEditor::caret_reset_selection()
{
	caretStart = caret_get_position();

	// Reset selection range
	caretEnd = USIZE_MAX;
	caretRoot = USIZE_MAX;
	caretSeek = USIZE_MAX;

	// Reset click selection range
	clickStart = USIZE_MAX;
	clickEnd = USIZE_MAX;

	// Reset Timer
	caretTimer = 0.0f;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int_v2 TextEditor::draw( const Delta delta, const float x, const float y, const Alpha alpha )
{
	// Highlight
	if( highlighting() ) { text.draw_selection( x, y, caretStart, caretEnd, alpha ); }

	// Active Caret
	if( CoreText::ACTIVE_TEXT_EDITOR == this )
	{
		// Color & Alert Timer
		Color color = caretColor;
		if( caretTimerAlert > 0.0f )
		{
			caretTimerAlert -= delta * 3;
			color = color_mix( caretColor, caretColorAlert, caretTimerAlert );
		}
		else
		{
			caretTimerAlert = 0.0f;
		}

		// Caret Visibility
		if( caretTimer < 1.0f && caretTimerAlert == 0.0f ) { caretTimer += delta; } else { caretTimer = 0.0f; }
		if( caretTimer < 0.5f )
		{
			float_v4 caret;
			text.draw_caret( x, y, caret_get_position(), &caret, alpha );
			draw_rectangle( caret.x, caret.y, caret.z, caret.w, color * alpha );
		}
	}

	// Text
	const int_v2 dimensions = text.draw( x, y, alpha );

	// Callback
	if( callbackOnDraw != nullptr ) { callbackOnDraw( *this, delta, x, y, alpha ); }

	// Return
	return dimensions;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TextEditor::poll()
{
	// Shift
	{
		// Begin Shifting
		if( Keyboard::check_pressed( vk_shift ) )
		{
			shifting = true;
			if( caretRoot == USIZE_MAX ) { caret_set_root( caret_get_position() ); }
		}

		// End Shifting
		if( Keyboard::check_released( vk_shift ) )
		{
			shifting = false;
		}

		// Legal Control Characters
		if( shifting && Keyboard::check_pressed_repeat_any() )
		{
			const bool legalShiftingModifiers =
				Keyboard::check_pressed_repeat( vk_shift ) ||
				Keyboard::check_pressed_repeat( vk_control ) ||
				Keyboard::check_pressed_repeat( vk_command ) ||
				Keyboard::check_pressed_repeat( vk_left ) ||
				Keyboard::check_pressed_repeat( vk_right ) ||
				Keyboard::check_pressed_repeat( vk_up ) ||
				Keyboard::check_pressed_repeat( vk_down ) ||
				Keyboard::check_pressed_repeat( vk_backspace ) ||
				Keyboard::check_pressed_repeat( vk_delete ) ||
				Keyboard::check_pressed_repeat( vk_home ) ||
				Keyboard::check_pressed_repeat( vk_end ) ||
				Keyboard::check_pressed_repeat( vk_pageup ) ||
				Keyboard::check_pressed_repeat( vk_pagedown ) ||
				Keyboard::check_pressed_repeat( vk_tab );

			if( !legalShiftingModifiers )
			{
				shifting = false;
				validate_selection();
			}
		}
	}

	// Mouse
	{
		// Begin Selecting
		if( Mouse::check_pressed( mb_left ) )
		{
			if( allowClick )
			{
				event_mouse_click();
				selecting = true;
				if( caretRoot == USIZE_MAX ) { caret_set_root( caret_get_position() ); }
			}
		}
		// Active Selection
		else if( selecting && ( Mouse::check( mb_left ) ) )
		{
			event_mouse_select();
		}

		// End Selecting
		if( Mouse::check_released( mb_left ) )
		{
			selecting = false;
		}

		// Legal Control Characters
		if( selecting && Keyboard::check_pressed_repeat_any() )
		{
			const bool legalSelectingModifiers =
				Keyboard::check_pressed_repeat( vk_shift ) ||
				Keyboard::check_pressed_repeat( vk_control ) ||
				Keyboard::check_pressed_repeat( vk_command );

			if( !legalSelectingModifiers )
			{
				selecting = false;
				validate_selection();
			}
		}
	}

	// Control
	#if OS_MACOS
		if( Keyboard::check_pressed( vk_command ) ) { control = true; }
		if( Keyboard::check_released( vk_command ) ) { control = false; }
	#else
		if( Keyboard::check_pressed( vk_control ) ) { control = true; }
		if( Keyboard::check_released( vk_control ) ) { control = false; }
	#endif

	// Events
	if( Keyboard::check_pressed_repeat( vk_left ) ) { event_left(); }
	if( Keyboard::check_pressed_repeat( vk_right ) ) { event_right(); }
	if( Keyboard::check_pressed_repeat( vk_up ) ) { event_up(); }
	if( Keyboard::check_pressed_repeat( vk_down ) ) { event_down(); }
	if( Keyboard::check_pressed_repeat( vk_backspace ) ) { event_backspace(); }
	//if( Keyboard::check_pressed_repeat( vk_insert ) ) { event_insert(); }
	if( Keyboard::check_pressed_repeat( vk_delete ) ) { event_delete(); }
	if( Keyboard::check_pressed_repeat( vk_home ) ) { event_home(); }
	if( Keyboard::check_pressed_repeat( vk_end ) ) { event_end(); }
	if( Keyboard::check_pressed_repeat( vk_pageup ) ) { event_pageup(); }
	if( Keyboard::check_pressed_repeat( vk_pagedown ) ) { event_pagedown(); }
	if( Keyboard::check_pressed_repeat( vk_tab ) ) { event_tab(); }
	if( Keyboard::check_pressed_repeat( vk_enter ) ) { event_newline(); }

	// Hotkey Events
	if( control && Keyboard::check_pressed( vk_a ) ) { event_select_all(); }
	if( control && Keyboard::check_pressed( vk_x ) ) { event_cut(); }
	if( control && Keyboard::check_pressed( vk_c ) ) { event_copy_clipboard(); }
	if( control && Keyboard::check_pressed_repeat( vk_v ) ) { event_paste_clipboard(); }

	// Selection Paste
#if OS_LINUX
	if( Mouse::check_pressed( mb_middle ) ) { event_paste_selection(); }
#endif

	// Input
	if( Keyboard::has_input() ) { update( Keyboard::input_buffer() ); }

	// Callback
	if( callbackOnListen != nullptr ) { callbackOnListen( *this ); }
}


void TextEditor::text_append( const char *buffer )
{
	if( caretStart == caretEnd ) { caretEnd = USIZE_MAX; }

	const usize caret = caretStart;
	const usize offset = text_replace( caretStart, highlighting() ? caretEnd : caretStart, buffer );
	caret_reset_selection();
	caretStart = caret + offset;
}


usize TextEditor::text_replace( const usize start, const usize end, const char *buffer )
{
	Assert( start <= end );
	Assert( end <= text.length() );
	usize offset = 0;

	// Remove section
	if( start != end )
	{
		text.remove( start, end - start );
	}

	// Insert string
	if( buffer[0] != '\0' )
	{
		offset = text.insert( start, buffer );
	}

	// Callback
	if( callbackOnUpdate != nullptr ){ callbackOnUpdate( *this ); }
	return offset;
}


u32 TextEditor::codepoint_at( const usize index ) const
{
	Assert( index < text.length() );
	return text.data[index].codepoint;
}


void TextEditor::validate_selection()
{
	// Has selection, but no longer shifting
	if( !shifting && !selecting ) { caret_reset_selection(); }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TextEditor::text_callback_error( Text &text, const TextErr error )
{
	if( CoreText::ACTIVE_TEXT_EDITOR == nullptr ) { return; }
	TextEditor &textEditor = *CoreText::ACTIVE_TEXT_EDITOR;

	// Update Cursor Alert
	textEditor.caret_alert( c_red );

	// Callback
	if( textEditor.callbackOnError != nullptr ) { textEditor.callbackOnError( textEditor, error ); }
}

void TextEditor::text_callback_update( Text &text )
{
	if( CoreText::ACTIVE_TEXT_EDITOR == nullptr ) { return; }
	TextEditor &textEditor = *CoreText::ACTIVE_TEXT_EDITOR;

	// Validate Caret Ranges
	const usize length = text.length();

	textEditor.caretStart = min( textEditor.caretStart, length );

	textEditor.caretEnd = textEditor.caretEnd < USIZE_MAX ? min( textEditor.caretEnd, length ) : USIZE_MAX;
	textEditor.caretRoot = textEditor.caretRoot < USIZE_MAX ? min( textEditor.caretRoot, length ) : USIZE_MAX;
	textEditor.caretSeek = textEditor.caretSeek < USIZE_MAX ? min( textEditor.caretSeek, length ) : USIZE_MAX;

	textEditor.clickIndex = textEditor.clickIndex < USIZE_MAX ? min( textEditor.clickIndex, length ) : USIZE_MAX;
	textEditor.clickStart = textEditor.clickStart < USIZE_MAX ? min( textEditor.clickStart, length ) : USIZE_MAX;
	textEditor.clickEnd = textEditor.clickEnd < USIZE_MAX ? min( textEditor.clickEnd, length ) : USIZE_MAX;

	// Callback
	if( textEditor.callbackOnUpdate != nullptr ) { textEditor.callbackOnUpdate( textEditor ); }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////