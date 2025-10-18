#pragma once

#include <core/types.hpp>

#include <vendor/stdio.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if defined( COMPILE_BUILD )
	#define MEMORY_RAII ( true )
#elif defined( COMPILE_ENGINE )
	#define MEMORY_RAII ( false )
#else
	#define MEMORY_RAII ( false )
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern void *memory_alloc( const usize size );

extern void *memory_realloc( void *block, const usize size );

extern void memory_copy( void *dst, const void *src, const usize size );

extern void memory_move( void *dst, const void *src, const usize size );

extern void memory_set( void *block, int c, const usize length );

extern int memory_compare( const void *a, const void *b, const usize size );

extern void memory_free( void *block );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern usize align_pow2( usize n );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////