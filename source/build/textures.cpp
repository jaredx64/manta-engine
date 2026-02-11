#include <build/textures.hpp>

#include <core/memory.hpp>
#include <core/math.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <vendor/stb/stb_image.hpp>
#include <vendor/stb/stb_image_write.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Texture2DBuffer::init( u16 width, u16 height )
{
	if( data != nullptr ) { free(); }

	data = reinterpret_cast<Color *>( memory_alloc( width * height * sizeof( Color ) ) );
	memory_set( data, 0, width * height * sizeof( Color ) );
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
	memory_copy( data, other.data, width * height * sizeof( Color ) );
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
	Assert( data != nullptr );
	return stbi_write_png( path, width, height, sizeof( Color ), data, width * sizeof( Color ) ) == 0;
}


bool Texture2DBuffer::load( const char *path )
{
	if( data != nullptr ) { free(); }

	int w, h, channels;
	data = reinterpret_cast<Color *>( stbi_load( path, &w, &h, &channels, sizeof( Color ) ) );
	if( data == nullptr ) { width = 0; height = 0; return false; }
	AssertMsg( static_cast<u16>( w ) <= U16_MAX && static_cast<u16>( w ) <= U16_MAX,
		"Attempting to load texture larger than max supported (try: %dx%d max:%ux%u)", w, h, U16_MAX, U16_MAX );
	width = static_cast<u16>( w );
	height = static_cast<u16>( h );
	return true;
}


void Texture2DBuffer::clear( const Color color )
{
	const int length = width * height;
	for( int i = 0; i < length; i++ ) { data[i] = color; }
}


void Texture2DBuffer::splice( Texture2DBuffer &source, u16 srcX1, u16 srcY1, u16 srcX2, u16 srcY2,
	u16 dstX, u16 dstY )
{
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
		Color *dst = &data[( dstY + y ) * width + dstX];
		Color *src = &source.data[( srcY1 + y ) * source.width + srcX1];
		memory_copy( dst, src, w * sizeof( Color ) );
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