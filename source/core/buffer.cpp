#include <core/buffer.hpp>

#include <vendor/compression/lzav.hpp>

#if COMPILE_BUILD
#include <build/filesystem.hpp>
#elif COMPILE_ENGINE
#include <manta/filesystem.hpp>
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Buffer::init( usize reserve, bool grow )
{
	capacity = reserve;
	current = 0LLU;
	tell = 0LLU;
	fixed = !grow;

	MemoryAssert( data == nullptr );
	Assert( capacity >= 1 );
	data = reinterpret_cast<byte *>( memory_alloc( capacity ) );
}


void Buffer::free()
{
	if( data == nullptr )
	{
	#if MEMORY_RAII
		return;
	#else
		MemoryWarning( "Buffer: attempting to free() buffer that is already freed!" ); return;
	#endif
	}

	memory_free( data );
	data = nullptr;

	capacity = 0LLU;
	current = 0LLU;
	tell = 0LLU;
}


bool Buffer::load( const char *path, bool grow )
{
	bool success = true;

#if MEMORY_RAII
	if( data != nullptr ) { free(); }
#else
	MemoryAssert( data == nullptr );
#endif

	FILE *file = fopen( path, "rb" );
	if( file == nullptr ) { return false; }

	const usize size = fsize( file );
	if( size == 0LLU ) { success = false; goto cleanup; }

	init( size, grow );
	if( fread( data, size, 1, file ) < 1 ) { success = false; goto cleanup; }

	current = size;
	tell = 0LLU;

cleanup:
	if( fclose( file ) != 0 ) { success = false; }
	if( success == false && data != nullptr ) { free(); }
	return success;
}


bool Buffer::save( const char *path )
{
	bool success = true;
	MemoryAssert( data != nullptr );

	FILE *file = fopen( path, "wb" );
	if( file == nullptr ) { return false; }

	if( fwrite( data, current, 1, file ) < 1 ) { success = false; goto cleanup; }

cleanup:
	if( fclose( file ) != 0 ) { return false; }
	return success;
}


Buffer &Buffer::copy( const Buffer &other )
{
	MemoryAssert( other.data != nullptr );
	if( this == &other ) { return *this; }
	if( data != nullptr ) { free(); }

	capacity = other.capacity;
	current = other.current;
	tell = other.tell;
	fixed = other.fixed;

	MemoryAssert( data == nullptr );
	Assert( capacity >= 1 );
	data = reinterpret_cast<byte *>( memory_alloc( capacity ) );
	memory_copy( data, other.data, capacity );

	return *this;
}


Buffer &Buffer::move( Buffer &&other )
{
	MemoryAssert( other.data != nullptr );
	if( this == &other ) { return *this; }
	if( data != nullptr ) { free(); }

	data = other.data;
	capacity = other.capacity;
	current = other.current;
	tell = other.tell;
	fixed = other.fixed;

	other.data = nullptr;
	other.capacity = 0LLU;
	other.current = 0LLU;
	other.tell = 0LLU;
	other.fixed = false;

	return *this;
}


void Buffer::grow()
{
	MemoryAssert( data != nullptr );
	Assert( capacity >= 1 && capacity < USIZE_MAX );

	capacity = capacity > USIZE_MAX / 2 ? USIZE_MAX : capacity * 2;
	data = reinterpret_cast<byte *>( memory_realloc( data, capacity ) );
	ErrorIf( data == nullptr, "Failed to reallocate memory for grow Buffer (%p: realloc %d bytes)",
		data, capacity );
}


bool Buffer::shrink()
{
	MemoryAssert( data != nullptr );
	if( current == capacity ) { return false; }
	capacity = current > 1 ? current : 1;
	data = reinterpret_cast<byte *>( memory_realloc( data, capacity ) );
	ErrorIf( data == nullptr, "Failed to reallocate memory for shrink Buffer (%p: alloc %d bytes)",
		data, capacity );

	return true;
}


void Buffer::clear()
{
	current = 0LLU;
	tell = 0LLU;
}


usize Buffer::compress()
{
	MemoryAssert( data != nullptr );

	const usize sizeCompressedBound = static_cast<usize>( lzav_compress_bound( size() ) );
	void *compressed = memory_alloc( sizeCompressedBound );
	MemoryAssert( compressed );

	const usize sizeCompressed =
		static_cast<usize>( lzav_compress_default( data, compressed, size(), sizeCompressedBound ) );

	clear();
	ErrorIf( fixed && capacity < sizeCompressed, "Buffer: fixed buffer too small for compression" );
	for( ; !fixed && tell + sizeCompressed > capacity; grow() ) { }

	memory_copy( data, compressed, sizeCompressed );
	tell = 0LLU;
	current = sizeCompressed;

	memory_free( compressed );

	return sizeCompressed;
}


usize Buffer::compress( void *tempBuffer, usize tempBufferSize )
{
	MemoryAssert( data != nullptr );
	MemoryAssert( tempBuffer != nullptr );

	const usize sizeCompressedBound = static_cast<usize>( lzav_compress_bound( size() ) );
	ErrorIf( sizeCompressedBound > tempBufferSize, "Buffer: supplied temp buffer too small for compression" );

	const usize sizeCompressed =
		static_cast<usize>( lzav_compress_default( data, tempBuffer, size(), sizeCompressedBound ) );

	clear();
	ErrorIf( fixed && capacity < sizeCompressed, "Buffer: fixed buffer too small for compression" );
	for( ; !fixed && sizeCompressed > capacity; grow() ) { }

	memory_copy( data, tempBuffer, sizeCompressed );
	tell = 0LLU;
	current = sizeCompressed;

	return sizeCompressed;
}


usize Buffer::compress_into( void *buffer, usize size )
{
	MemoryAssert( data != nullptr );
	MemoryAssert( buffer != nullptr );

	const usize sizeCompressedBound = static_cast<usize>( lzav_compress_bound( this->size() ) );
	ErrorIf( sizeCompressedBound > size, "Buffer: supplied buffer too small for compression" );

	const usize sizeCompressed =
		static_cast<usize>( lzav_compress_default( data, buffer, this->size(), sizeCompressedBound ) );

	return sizeCompressed;
}


bool Buffer::decompress( const void *dataCompressed, usize sizeCompressed, usize sizeOriginal )
{
	MemoryAssert( data != nullptr );
	MemoryAssert( dataCompressed != nullptr );

	clear();
	ErrorIf( fixed && capacity < sizeOriginal, "Buffer: fixed buffer too small for decompression" );
	for( ; !fixed && sizeOriginal > capacity; grow() ) { }

	const int sizeDecompressed = lzav_decompress( dataCompressed, data, sizeCompressed, sizeOriginal );
	if( sizeDecompressed < 0 ) { clear(); return false; }

	tell = 0LLU;
	current = static_cast<usize>( sizeDecompressed );

	return true;
}


usize Buffer::write( const void *bytes, usize size )
{
	MemoryAssert( data != nullptr );
	for( ; !fixed && tell + size > capacity; grow() ) { }

	ErrorIf( tell + size > capacity, "Buffer: write exceeded buffer capacity" );
	const usize writeIndex = tell;
	memory_copy( &data[tell], bytes, size );
	tell += size;
	current = tell > current ? tell : current;
	return writeIndex;
}


usize Buffer::write_from_file( const char *path, usize offset, usize size )
{
	bool success = true;
	usize writeIndex = USIZE_MAX;

	MemoryAssert( data != nullptr );
	for( ; !fixed && tell + size > capacity; grow() ) { }

	FILE *file = fopen( path, "rb" );
	if( file == nullptr ) { return USIZE_MAX; }

	if( fseek( file, static_cast<long>( offset ), SEEK_SET ) != 0 ) { success = false; goto cleanup; }

	ErrorIf( tell + size > capacity, "Buffer: write exceeded buffer capacity" );
	writeIndex = tell;
	if( fread( &data[writeIndex], size, 1, file ) < 1 ) { success = false; goto cleanup; }
	tell += size;
	current = tell > current ? tell : current;

cleanup:
	if( fclose( file ) != 0 ) { success = false; }
	if( success == false && data != nullptr ) { free(); }
	return success ? writeIndex : USIZE_MAX;
}


void Buffer::shift( int amount )
{
	MemoryAssert( data != nullptr );
	if( amount == 0 ) { return; }

	// Left Shift
	if( amount < 0 )
	{
		const usize shift = static_cast<usize>( -amount );
		const usize discard = shift > current ? current : shift;
		const usize remaining = current - discard;
		if( remaining > 0 ) { memory_move( data, data + discard, remaining ); }
		current = remaining;
		if( tell < discard ) { tell = 0; } else { tell -= discard; }
		return;
	}

	// Right shift
	const usize shift = static_cast<usize>( amount );
	for( ; !fixed && current + shift > capacity; grow() ) { }
	ErrorIf( current + shift > capacity, "Buffer: shift exceeded fixed buffer capacity" );
	if( current > 0 ) { memory_move( data + shift, data, current ); }
	current += shift;
	tell += shift;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////