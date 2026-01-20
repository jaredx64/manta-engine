#pragma once

#include <vendor/config.hpp>
#include <core/types.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern u32 checksum_xcrc32( const char *buffer, usize size, u32 seed );
inline u32 checksum_xcrc32( const unsigned char *buffer, usize size, u32 seed )
{
	return checksum_xcrc32( reinterpret_cast<const char *>( buffer ), size, seed );
}


extern u32 checksum_xcrc32( const char *buffer, usize offset, int size, u32 seed );
inline u32 checksum_xcrc32( const unsigned char *buffer, usize offset, int size, u32 seed )
{
	return checksum_xcrc32( reinterpret_cast<const char *>( buffer ), offset, size, seed );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if 0
extern u64 checksum_fnv1a64( const char *buffer, usize size, u32 seed );
extern u64 checksum_fnv1a64( const char *buffer, usize offset, int size, u32 seed );
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////