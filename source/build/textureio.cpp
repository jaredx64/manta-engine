#include <build/textureio.hpp>

#include <core/memory.hpp>
#include <core/math.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <vendor/stb/stb_image.hpp>
#include <vendor/stb/stb_image_write.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Texture2DBuffer::init( const u16 width, const u16 height )
{
	// Free existing data
	if( data != nullptr ) { free(); }

	// Allocate blank texture
	data = reinterpret_cast<rgba *>( memory_alloc( width * height * sizeof( rgba ) ) );
	memory_set( data, 0, width * height * sizeof( rgba ) );
	this->width = width;
	this->height = height;
}


void Texture2DBuffer::free()
{
	if( data == nullptr ) { return; }
	memory_free( data );
	data = nullptr;
	width = 0;
	height = 0;
}


void Texture2DBuffer::copy( const Texture2DBuffer &other )
{
	Assert( other.data != nullptr );
	if( data != nullptr ) { free(); }
	init( other.width, other.height );
	ErrorIf( data == nullptr, "Failed to allocate memory for copy() Texture2DBuffer (%p)", data );
	memory_copy( data, other.data, width * height * sizeof( rgba ) );
}


void Texture2DBuffer::move( Texture2DBuffer &&other )
{
	if( data != nullptr ) { free(); }
	data = other.data;
	width = other.width;
	height = other.height;
	other.data = nullptr;
	other.width = 0;
	other.height = 0;
}


bool Texture2DBuffer::save( const char *path )
{
	// Write PNG
	Assert( data != nullptr );
	return stbi_write_png( path, width, height, sizeof( rgba ), data, width * sizeof( rgba ) ) == 0;
}


bool Texture2DBuffer::load( const char *path )
{
	// Free existing data
	if( data != nullptr ) { free(); }

	// Load PNG
	int w, h, channels;
	data = reinterpret_cast<rgba *>( stbi_load( path, &w, &h, &channels, sizeof( rgba ) ) );
	if( data == nullptr ) { width = 0; height = 0; return false; }
	AssertMsg( static_cast<u16>( w ) <= U16_MAX && static_cast<u16>( w ) <= U16_MAX,
		"Attempting to load texture larger than max supported (try: %dx%d max:%ux%u)", w, h, U16_MAX, U16_MAX );
	width = static_cast<u16>( w );
	height = static_cast<u16>( h );
	return true;
}


void Texture2DBuffer::clear( const rgba color )
{
	const int length = width * height;
	for( int i = 0; i < length; i++ ) { data[i] = color; }
}


void Texture2DBuffer::splice( Texture2DBuffer &source,
	const u16 srcX1, const u16 srcY1, const u16 srcX2, const u16 srcY2, const u16 dstX, const u16 dstY )
{
	// Error checking
	ErrorIf( !source, "Texture2DBuffer::splice - Attempting to splice null Texture2DBuffer" );
	ErrorIf( dstX >= width && dstY >= height,
		"Texture2DBuffer::splice - dstX/dstY out of bounds (dstX:%d, dstY:%d) (destination res: %dx%d)",
		dstX, dstY, width, height );
	ErrorIf( srcX1 >= source.width || srcY1 >= source.height,
		"Texture2DBuffer::splice - srcX1/srcY1 out of bounds (srcX1:%d, srcY1:%d) (source res: %dx%d)",
		srcX1, srcY1, source.width, source.height );
	ErrorIf( srcX2 > source.width || srcY2 > source.height,
		"Texture2DBuffer::splice - srcX2/srcY2 out of bounds (srcX2:%d, srcY2:%d) (source res: %dx%d)",
		srcX2, srcY2, source.width, source.height );

	const u16 w = min( srcX2 - srcX1, width - dstX );
	const u16 h = min( srcY2 - srcY1, height - dstY );

	for( int y = 0; y < h; y++ )
	{
		rgba *dst = &data[( dstY + y ) * width + dstX];
		rgba *src = &source.data[( srcY1 + y ) * source.width + srcX1];
		memory_copy( dst, src, w * sizeof( rgba ) );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct ICOHeader
{
	u16 reserved;
	u16 type;
	u16 count;
};


struct ICODirEntry
{
    u8 width;
    u8 height;
    u8 colorCount;
    u8 reserved;
    u16 planes;
    u16 bitCount;
    u32 sizeInBytes;
    u32 offset;
};


bool png_to_ico( const char *pathPNG, const char *pathICO )
{
	// Load .png File
	FILE *pngFile = fopen( pathPNG, "rb" );
	if( !pngFile ) { return false; }

	fseek( pngFile, 0, SEEK_END );
	const usize pngSizeBytes = ftell( pngFile );
	fseek( pngFile, 0, SEEK_END );

	byte *pngData = reinterpret_cast<byte *>( memory_alloc( pngSizeBytes ) );
	fseek( pngFile, 0, SEEK_SET );

	if( fread( pngData, 1, pngSizeBytes, pngFile ) != pngSizeBytes )
	{
		memory_free( pngData );
		fclose( pngFile );
		return false;
	}

	fclose( pngFile );

	// Create .ico file
	FILE *file = fopen( pathICO, "wb" );
	if( !file ) { return false; }

	ICOHeader header;
	header.reserved = 0;
	header.type = 1;
	header.count = 1;

	ICODirEntry entry;
	entry.width = 0;
	entry.height = 0;
	entry.colorCount = 0;
	entry.reserved = 0;
	entry.planes = 1;
	entry.bitCount = 32;
	entry.sizeInBytes = static_cast<u32>( pngSizeBytes );
	entry.offset = sizeof( ICOHeader ) + sizeof( ICODirEntry );

	fwrite( &header, sizeof( header ), 1, file );
	fwrite( &entry, sizeof( entry ), 1, file );
	fwrite( pngData, 1, pngSizeBytes, file );
	fclose( file );

	return true;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////