#pragma once

#include <core/types.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define UTF8_ACCEPT 0

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern usize utf8_length_codepoints( const char *string );
extern usize utf8_length_bytes( const char *string );
extern usize utf8_substr_length_bytes( const char *string, usize start, usize end );

extern u32 utf8_codepoint_at( const char *string, usize index );
extern int utf8_codepoint_size( u32 codepoint );

extern int utf8_encode( char *buffer, u32 codepoint );
extern u32 utf8_decode( u32 *state, u32 *codepoint, char byte );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Control characters (U+0000–U+001F) and DEL (U+007F)
#define UTF8_CODEPOINT_DELETE ( 0x0000007F )
#define UTF8_CODEPOINT_BACKSPACE ( 0x00000008 )
#define UTF8_CODEPOINT_TAB ( 0x00000009 )
#define UTF8_CODEPOINT_RETURN ( 0x0000000D )
#define UTF8_CODEPOINT_ESCAPE ( 0x0000001B )

// Non-characters (U+FDD0–U+FDEF)
#define UTF8_CODEPOINT_NONCHARACTER_START ( 0x0000FDD0 )
#define UTF8_CODEPOINT_NONCHARACTER_END ( 0x0000FDEF )

// Surrogate code points (U+D800–U+DFFF)
#define UTF8_CODEPOINT_SURROGATE_START ( 0x0000D800 )
#define UTF8_CODEPOINT_SURROGATE_END ( 0x0000DFFF )

// Private use areas
#define UTF8_CODEPOINT_PRIVATE_USE_START_1 ( 0x0000E000 )
#define UTF8_CODEPOINT_PRIVATE_USE_END_1 ( 0x0000F8FF )
#define UTF8_CODEPOINT_PRIVATE_USE_START_2 ( 0x000F0000 )
#define UTF8_CODEPOINT_PRIVATE_USE_END_2 ( 0x000FFFFD )
#define UTF8_CODEPOINT_PRIVATE_USE_START_3 ( 0x00100000 )
#define UTF8_CODEPOINT_PRIVATE_USE_END_3 ( 0x0010FFFD )

// Embedding and overrides (U+202A–U+202E)
#define UTF8_CODEPOINT_EMBEDDING_START ( 0x0000202A )
#define UTF8_CODEPOINT_EMBEDDING_END ( 0x0000202E )

// Non-characters ending in FFFE or FFFF
#define UTF8_CODEPOINT_NONCHARACTER_1 ( 0x0000FFFE )
#define UTF8_CODEPOINT_NONCHARACTER_2 ( 0x0000FFFF )
#define UTF8_CODEPOINT_NONCHARACTER_3 ( 0x0001FFFE )
#define UTF8_CODEPOINT_NONCHARACTER_4 ( 0x0001FFFF )
#define UTF8_CODEPOINT_NONCHARACTER_5 ( 0x0002FFFE )
#define UTF8_CODEPOINT_NONCHARACTER_6 ( 0x0002FFFF )
#define UTF8_CODEPOINT_NONCHARACTER_7 ( 0x0003FFFE )
#define UTF8_CODEPOINT_NONCHARACTER_8 ( 0x0003FFFF )
#define UTF8_CODEPOINT_NONCHARACTER_9 ( 0x0004FFFE )
#define UTF8_CODEPOINT_NONCHARACTER_10 ( 0x0004FFFF )
#define UTF8_CODEPOINT_NONCHARACTER_11 ( 0x0005FFFE )
#define UTF8_CODEPOINT_NONCHARACTER_12 ( 0x0005FFFF )
#define UTF8_CODEPOINT_NONCHARACTER_13 ( 0x0006FFFE )
#define UTF8_CODEPOINT_NONCHARACTER_14 ( 0x0006FFFF )
#define UTF8_CODEPOINT_NONCHARACTER_15 ( 0x0007FFFE )
#define UTF8_CODEPOINT_NONCHARACTER_16 ( 0x0007FFFF )
#define UTF8_CODEPOINT_NONCHARACTER_17 ( 0x0008FFFE )
#define UTF8_CODEPOINT_NONCHARACTER_18 ( 0x0008FFFF )
#define UTF8_CODEPOINT_NONCHARACTER_19 ( 0x0009FFFE )
#define UTF8_CODEPOINT_NONCHARACTER_20 ( 0x0009FFFF )
#define UTF8_CODEPOINT_NONCHARACTER_21 ( 0x000AFFFE )
#define UTF8_CODEPOINT_NONCHARACTER_22 ( 0x000AFFFF )
#define UTF8_CODEPOINT_NONCHARACTER_23 ( 0x000BFFFE )
#define UTF8_CODEPOINT_NONCHARACTER_24 ( 0x000BFFFF )
#define UTF8_CODEPOINT_NONCHARACTER_25 ( 0x000CFFFE )
#define UTF8_CODEPOINT_NONCHARACTER_26 ( 0x000CFFFF )
#define UTF8_CODEPOINT_NONCHARACTER_27 ( 0x000DFFFE )
#define UTF8_CODEPOINT_NONCHARACTER_28 ( 0x000DFFFF )
#define UTF8_CODEPOINT_NONCHARACTER_29 ( 0x000EFFFE )
#define UTF8_CODEPOINT_NONCHARACTER_30 ( 0x000EFFFF )
#define UTF8_CODEPOINT_NONCHARACTER_31 ( 0x000FFFFE )
#define UTF8_CODEPOINT_NONCHARACTER_32 ( 0x000FFFFF )
#define UTF8_CODEPOINT_NONCHARACTER_33 ( 0x0010FFFE )
#define UTF8_CODEPOINT_NONCHARACTER_34 ( 0x0010FFFF )

// Line and paragraph separators
#define UTF8_CODEPOINT_LINE_SEPARATOR ( 0x00002028 )
#define UTF8_CODEPOINT_PARAGRAPH_SEPARATOR ( 0x00002029 )

// Bidirectional formatting characters
#define UTF8_CODEPOINT_LTR_MARK ( 0x0000200E )
#define UTF8_CODEPOINT_RTL_MARK ( 0x0000200F )

// Zero-width characters
#define UTF8_CODEPOINT_ZERO_WIDTH_SPACE ( 0x0000200B )
#define UTF8_CODEPOINT_ZERO_WIDTH_NON_JOINER ( 0x0000200C )
#define UTF8_CODEPOINT_ZERO_WIDTH_JOINER ( 0x0000200D )

// Replacement character
#define UTF8_CODEPOINT_REPLACEMENT_CHARACTER ( 0x0000FFFD )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////