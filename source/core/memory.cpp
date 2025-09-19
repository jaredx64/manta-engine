#include <core/memory.hpp>

#include <vendor/stdlib.hpp>

#include <vendor/string.hpp>

#include <core/debug.hpp>

#include <pipeline.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void *memory_alloc( const usize size )
{
	void *ptr = malloc( size );
	ErrorIf( ptr == nullptr, "Memory allocation failure! Out of memory?" );
	return ptr;
}


void *memory_realloc( void *block, const usize size )
{
	Assert( block != nullptr );
	void *ptr = realloc( block, size );
	ErrorIf( ptr == nullptr, "Memory reallocation failure! Out of memory?" );
	return ptr;
}


void memory_copy( void *dst, const void *src, const usize size )
{
	Assert( dst != nullptr );
	Assert( src != nullptr );
	memcpy( dst, src, size );
}


void memory_move( void *dst, const void *src, const usize size )
{
	Assert( dst != nullptr );
	Assert( src != nullptr );
	memmove( dst, src, size );
}


void memory_set( void *block, int c, const usize length )
{
	Assert( block != nullptr );
	memset( block, c, length );
}


int memory_compare( const void *a, const void *b, const usize size )
{
	return memcmp( a, b, size );
}


void memory_free( void *block )
{
	Assert( block != nullptr );
	free( block );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

usize align_pow2( usize n )
{
	if( n == 0 ) { return 1; }
	n -= 1;
	n |= n >> 1;
	n |= n >> 2;
	n |= n >> 4;
	n |= n >> 8;
	n |= n >> 16;
	n |= n >> 32;
	return n + 1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////