#pragma once

#include <core/types.hpp>

#include <vendor/string.hpp>

namespace Hash
{
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

constexpr u32 hash( const char *key )
{
	u32 hash = 5381;
	u32 c;
	while( ( c = *key++ ) ) { hash = ( ( hash << 5 ) + hash ) + c; }
	return hash;
}

// djb2 string hash by Dan Bernstein:
// https://web.archive.org/web/20210415122049/https://www.cse.yorku.ca/~oz/hash.html
constexpr u32 hash( const char *key, int length )
{
	u32 hash = 5381;
	for( int i = 0; i < length; i++ ) { hash = ( ( hash << 5 ) + hash ) + key[i]; }
	return hash;
}

// 64-bit to 32-bit hash by Thomas Wang:
// https://web.archive.org/web/20071223173210/https://www.concentric.net/~Ttwang/tech/inthash.htm
constexpr u32 hash( u64 key )
{
	key = ~key + ( key << 18 );
	key =  key ^ ( key >> 31 );
	key =  key * ( 21 );
	key =  key ^ ( key >> 11 );
	key =  key + ( key << 6 );
	key =  key ^ ( key >> 22 );
	return static_cast<u32>( key );
}

constexpr u32 hash( i64 key ) { return hash( static_cast<u64>( key ) ); }
constexpr u32 hash( i32 key ) { return static_cast<u32>( key );  }
constexpr u32 hash( i16 key ) { return static_cast<u32>( key );  }
constexpr u32 hash( i8 key ) { return static_cast<u32>( key );  }
constexpr u32 hash( u32 key ) { return static_cast<u32>( key );  }
constexpr u32 hash( u16 key ) { return static_cast<u32>( key );  }
constexpr u32 hash( u8 key ) { return static_cast<u32>( key );  }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline bool equals( const char *a, const char *b ) { return strcmp( a, b ) == 0; }

inline bool equals( i64 a, i64 b ) { return a == b; }
inline bool equals( i32 a, i32 b ) { return a == b; }
inline bool equals( i16 a, i16 b ) { return a == b; }
inline bool equals( i8 a, i8 b ) { return a == b; }
inline bool equals( u64 a, u64 b ) { return a == b; }
inline bool equals( u32 a, u32 b ) { return a == b; }
inline bool equals( u16 a, u16 b ) { return a == b; }
inline bool equals( u8 a, u8 b ) { return a == b; }

inline bool is_null( const char *a ) { return a == nullptr; }
inline void set_null( const char *&a ) { a = nullptr; }

inline bool is_null( u32 a ) { return a == U32_MAX; }
inline void set_null( u32 &a ) { a = U32_MAX; }

inline bool is_null( u64 a ) { return a == U64_MAX; }
inline void set_null( u64 &a ) { a = U64_MAX; }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
};