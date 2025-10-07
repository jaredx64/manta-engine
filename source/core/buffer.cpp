#include <core/buffer.hpp>

#if COMPILE_BUILD
#include <build/filesystem.hpp>
#elif COMPILE_ENGINE
#include <manta/filesystem.hpp>
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Buffer::init( usize reserve, const bool grow )
{
	// Set state
	capacity = reserve;
	current = 0;
	tell = 0;
	fixed = !grow;

	// Allocate memory
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

	// Free memory
	memory_free( data );
	data = nullptr;

	// Reset state
	capacity = 0;
	current = 0;
	tell = 0;
}


bool Buffer::load( const char *path, const bool grow )
{
	bool success = true;

#if MEMORY_RAII
	if( data != nullptr ) { free(); }
#else
	MemoryAssert( data == nullptr );
#endif

	// Open file for reading
	FILE *file = fopen( path, "rb" );
	if( file == nullptr ) { return false; }

	// Fetch file size
	const usize size = fsize( file );
	if( size == 0 ) { success = false; goto cleanup; }

	// Initialize buffer
	init( size, grow );

	// Read file contents to buffer
	if( fread( data, size, 1, file ) < 1 ) { success = false; goto cleanup; }

	// Set state
	current = size;
	tell = 0;

cleanup:
	// Close file
	if( fclose( file ) != 0 ) { success = false; }
	if( success == false && data != nullptr ) { free(); }
	return success;
}


bool Buffer::save( const char *path )
{
	bool success = true;
	MemoryAssert( data != nullptr );

	// Open file for writing
	FILE *file = fopen( path, "wb" );
	if( file == nullptr ) { return false; }

	// Write buffer contents to file
	if( fwrite( data, current, 1, file ) < 1 ) { success = false; goto cleanup; }

cleanup:
	// Close file
	if( fclose( file ) != 0 ) { return false; }
	return success;
}


Buffer &Buffer::copy( const Buffer &other )
{
	MemoryAssert( other.data != nullptr );
	if( this == &other ) { return *this; }
	if( data != nullptr ) { free(); }

	// Copy state
	capacity = other.capacity;
	current = other.current;
	tell = other.tell;
	fixed = other.fixed;

	// Allocate memory
	MemoryAssert( data == nullptr );
	Assert( capacity >= 1 );
	data = reinterpret_cast<byte *>( memory_alloc( capacity ) );
	memory_copy( data, other.data, capacity );

	// Return this
	return *this;
}


Buffer &Buffer::move( Buffer &&other )
{
	MemoryAssert( other.data != nullptr );
	if( this == &other ) { return *this; }
	if( data != nullptr ) { free(); }

	// Move the other buffer's resources
	data = other.data;
	capacity = other.capacity;
	current = other.current;
	tell = other.tell;
	fixed = other.fixed;

	// Reset the other buffer to null state
	other.data = nullptr;
	other.capacity = 0;
	other.current = 0;
	other.tell = 0;
	other.fixed = false;

	// Return this
	return *this;
}


void Buffer::grow()
{
	MemoryAssert( data != nullptr );
	Assert( capacity >= 1 && capacity < USIZE_MAX );

	// Reallocate memory
	capacity = capacity > USIZE_MAX / 2 ? USIZE_MAX : capacity * 2;
	data = reinterpret_cast<byte *>( memory_realloc( data, capacity ) );
	ErrorIf( data == nullptr, "Failed to reallocate memory for grow Buffer (%p: realloc %d bytes)",
	         data, capacity );
}


bool Buffer::shrink()
{
	// Shrink to current
	MemoryAssert( data != nullptr );
	if( current == capacity ) { return false; }
	capacity = current > 1 ? current : 1;
	data = reinterpret_cast<byte *>( memory_realloc( data, capacity ) );
	ErrorIf( data == nullptr, "Failed to reallocate memory for shrink Buffer (%p: alloc %d bytes)",
	         data, capacity );

	// Success
	return true;
}


void Buffer::clear()
{
	current = 0;
	tell = 0;
}


usize Buffer::write( void *bytes, const usize size )
{
	// Grow memory
	MemoryAssert( data != nullptr );
	while( !fixed )
	{
		if( tell + size <= capacity ) { break; }
		grow();
	}

	// Write element
	ErrorIf( tell + size > capacity, "Buffer: write exceeded buffer capacity" );
	const usize writeIndex = tell;
	memory_copy( &data[tell], bytes, size );
	tell += size;
	current = tell > current ? tell : current;
	return writeIndex;
}


usize Buffer::write_from_file( const char *path, const usize offset, const usize size )
{
	bool success = true;
	usize writeIndex = USIZE_MAX;

	// Grow memory
	MemoryAssert( data != nullptr );
	while( !fixed )
	{
		if( tell + size <= capacity ) { break; }
		grow();
	}

	// Open file for reading
	FILE *file = fopen( path, "rb" );
	if( file == nullptr ) { return USIZE_MAX; }

	// Seek to offset
	if( fseek( file, static_cast<size_t>( offset ), SEEK_SET ) != 0 ) { success = false; goto cleanup; }

	// Write element
	ErrorIf( tell + size > capacity, "Buffer: write exceeded buffer capacity" );
	writeIndex = tell;
	if( fread( &data[writeIndex], size, 1, file ) < 1 ) { success = false; goto cleanup; }
	tell += size;
	current = tell > current ? tell : current;

cleanup:
	// Close file
	if( fclose( file ) != 0 ) { success = false; }
	if( success == false && data != nullptr ) { free(); }
	return success ? writeIndex : USIZE_MAX;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////