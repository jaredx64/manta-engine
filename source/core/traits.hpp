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

inline void hash32_bytes( u32 &hash, const void *data, usize size )
{
	static constexpr u32 FNV_PRIME = 16777619U;
	const byte *bytes = reinterpret_cast<const byte *>( data );
	for( usize i = 0; i < size; i++ )
	{
		hash ^= bytes[i];
		hash *= FNV_PRIME;
	}
}


inline void hash32_cstr( u32 &hash, const char *str )
{
	static constexpr u32 FNV_PRIME = 16777619U;
	while( *str )
	{
		hash ^= static_cast<u8>( *str++ );
		hash *= FNV_PRIME;
	}
}


template <typename T> inline void hash32_value( u32 &hash, const T &value )
{
	hash32_bytes( hash, &value, sizeof( T ) );
}


template <> inline void hash32_value<const char *>( u32 &hash, const char *const &value )
{
	if( value != nullptr )
	{
		hash32_cstr( hash, value );
	}
	else
	{
		u32 zero = 0;
		hash32_bytes( hash, &zero, sizeof( zero ) );
	}
}


inline void hash32_combine( u32 & ) { }


template <typename T, typename... Ts> inline void hash32_combine( u32 &hash, const T &value, const Ts &...rest )
{
	static constexpr u32 FNV_PRIME = 16777619U;
	hash32_value( hash, value );
	hash ^= 0x9e3779b9u;
	hash *= FNV_PRIME;
	hash32_combine( hash, rest... );
}


template <typename... Args> inline u32 hash32_from( const Args &...args )
{
	static constexpr u32 FNV_OFFSET_BASIS = 2166136261U;
	u32 hash = FNV_OFFSET_BASIS;
	hash32_combine( hash, args... );
	return hash;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline void hash64_bytes( u64 &hash, const void *data, usize size )
{
	static constexpr u64 FNV_PRIME = 1099511628211ULL;
	const byte *bytes = reinterpret_cast<const byte *>( data );

	for( usize i = 0; i < size; i++ )
	{
		hash ^= bytes[i];
		hash *= FNV_PRIME;
	}
}


inline void hash64_cstr( u64 &hash, const char *str )
{
	static constexpr u64 FNV_PRIME = 1099511628211ULL;

	while( *str )
	{
		hash ^= static_cast<u8>( *str++ );
		hash *= FNV_PRIME;
	}
}


template <typename T> inline void hash64_value( u64 &hash, const T &value )
{
	hash64_bytes( hash, &value, sizeof( T ) );
}


template <> inline void hash64_value<const char *>( u64 &hash, const char *const &value )
{
	if( value )
	{
		hash64_cstr( hash, value );
	}
	else
	{
		u64 zero = 0;
		hash64_bytes( hash, &zero, sizeof( zero ) );
	}
}


inline void hash64_combine( u64 & ) { }


template <typename T, typename... Ts> inline void hash64_combine( u64 &hash, const T &value, const Ts &...rest )
{
	static constexpr u64 FNV_PRIME = 1099511628211ULL;
	hash64_value( hash, value );
	hash ^= 0x9e3779b97f4a7c15ULL;
	hash *= FNV_PRIME;
	hash64_combine( hash, rest... );
}


template <typename... Args> inline u64 hash64_from( const Args &...args )
{
	static constexpr u64 FNV_OFFSET_BASIS = 14695981039346656037ULL;
	u64 hash = FNV_OFFSET_BASIS;
	hash64_combine( hash, args... );
	return hash;
}

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