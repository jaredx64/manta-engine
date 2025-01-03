#pragma once

#include <core/types.hpp>

#include <vendor/string.hpp>

namespace Hash
{
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline u32 hash( const char *key )
{
	u32 hash = 5381;
	u32 c;
	while( ( c = *key++ ) ) { hash = ( ( hash << 5 ) + hash ) + c; }
	return hash;
}

// djb2 string hash by Dan Bernstein:
// https://web.archive.org/web/20210415122049/https://www.cse.yorku.ca/~oz/hash.html
inline u32 hash( const char *key, int length )
{
	u32 hash = 5381;
	for( int i = 0; i < length; i++ ) { hash = ( ( hash << 5 ) + hash ) + key[i]; }
	return hash;
}

// 64-bit to 32-bit hash by Thomas Wang:
// https://web.archive.org/web/20071223173210/https://www.concentric.net/~Ttwang/tech/inthash.htm
inline u32 hash( u64 key )
{
	key = ~key + ( key << 18 );
	key =  key ^ ( key >> 31 );
	key =  key * ( 21 );
	key =  key ^ ( key >> 11 );
	key =  key + ( key << 6 );
	key =  key ^ ( key >> 22 );

	return static_cast<u32>( key );
}

inline u32 hash( const i64 key ) { return hash( static_cast<u64>( key ) ); }
inline u32 hash( const i32 key ) { return       static_cast<u32>( key );  }
inline u32 hash( const i16 key ) { return       static_cast<u32>( key );  }
inline u32 hash( const i8 key )  { return       static_cast<u32>( key );  }
inline u32 hash( const u32 key ) { return       static_cast<u32>( key );  }
inline u32 hash( const u16 key ) { return       static_cast<u32>( key );  }
inline u32 hash( const u8 key )  { return       static_cast<u32>( key );  }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline bool equals( const char *a, const char *b ) { return strcmp( a, b ) == 0; }

inline bool equals( const i64 a, const i64 b ) { return a == b; }
inline bool equals( const i32 a, const i32 b ) { return a == b; }
inline bool equals( const i16 a, const i16 b ) { return a == b; }
inline bool equals( const i8  a, const i8  b ) { return a == b; }
inline bool equals( const u64 a, const u64 b ) { return a == b; }
inline bool equals( const u32 a, const u32 b ) { return a == b; }
inline bool equals( const u16 a, const u16 b ) { return a == b; }
inline bool equals( const u8  a, const u8  b ) { return a == b; }

inline bool is_null( const char *a ) { return a == nullptr; }
inline void set_null( const char *&a ) { a = nullptr; }

inline bool is_null( const u32 a ) { return a == U32_MAX; }
inline void set_null( u32 &a ) { a = U32_MAX; }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
};