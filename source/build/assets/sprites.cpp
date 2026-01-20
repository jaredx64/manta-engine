#include <build/assets/sprites.hpp>

#include <vendor/stb/stb_image.hpp>

#include <core/types.hpp>
#include <core/debug.hpp>
#include <core/list.hpp>
#include <core/json.hpp>

#include <build/build.hpp>
#include <build/assets.hpp>
#include <build/assets/textures.hpp>
#include <build/filesystem.hpp>

// TODO: temporary
#include <core/checksum.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct CacheSprite
{
	int imageWidth;
	int imageHeight;
	int imageChannels;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Sprites::make_new( const Sprite &sprite )
{
	sprites.add( sprite );
}


usize Sprites::gather( const char *path, bool recurse )
{
	// Gather Sprites
	List<FileInfo> files;
	directory_iterate( files, path, ".sprite", recurse );

	// Process Sprites
	for( FileInfo &fileInfo : files )
	{
		Assets::cacheFileCount++;
		process( fileInfo.path );
	}

	return files.size();
}


void Sprites::process( const char *path )
{
	// Local Directory
	static char pathDirectory[PATH_SIZE];
	path_get_directory( pathDirectory, sizeof( pathDirectory ), path );

	// Register Definition File
	AssetFile fileDefinition;
	if( !asset_file_register( fileDefinition, path ) )
	{
		Error( "Unable to locate sprite file: %s", path );
		return;
	}

	// Open Definition JSON
	String fileDefinitionContents;
	if( !fileDefinitionContents.load( path ) )
	{
		Error( "Unable to open sprite file: %s", path );
		return;
	}
	JSON fileDefinitionJSON { fileDefinitionContents };

	// Parse Definition JSON
	static char pathImage[PATH_SIZE];
	String pathImageRelative = fileDefinitionJSON.get_string( "path" );
	ErrorIf( pathImageRelative.length_bytes() == 0,
		"Sprite '%s' has an invalid path (required)", fileDefinition.name );
	snprintf( pathImage, sizeof( pathImage ),
		"%s" SLASH "%s", pathDirectory, pathImageRelative.cstr() );

	String atlasName = fileDefinitionJSON.get_string( "atlas", "default" );
	ErrorIf( atlasName.length_bytes() == 0,
		"Sprite '%s' has an invalid atlas (must not be null)", fileDefinition.name );
	atlasName.insert( 0, "atlas_" );

	int imageWidth = 0;
	int imageHeight = 0;
	int imageChannels = 0;
	const int count = fileDefinitionJSON.get_int( "count", 1 );
	ErrorIf( count < 1, "Sprite '%s' has an invalid count (must be >= 1)", fileDefinition.name );

	const int xorigin = fileDefinitionJSON.get_int( "xorigin", 0 );
	const int yorigin = fileDefinitionJSON.get_int( "yorigin", 0 );

	// Register Image File
	AssetFile fileImage;
	if( !asset_file_register( fileImage, pathImage ) )
	{
		Error( "Sprite '%s' - Unable to locate image file: '%s'", fileDefinition.name, pathImage );
		return;
	}

	// Generate Cache ID
	static char cacheIDBuffer[PATH_SIZE * 3];
	memory_set( cacheIDBuffer, 0, sizeof( cacheIDBuffer ) );
	snprintf( cacheIDBuffer, sizeof( cacheIDBuffer ), "sprite %s|%llu|%s|%llu",
		fileDefinition.path, fileDefinition.time.as_u64(),
		fileImage.path, fileImage.time.as_u64() );
	const CacheID cacheID = checksum_xcrc32( cacheIDBuffer, sizeof( cacheIDBuffer ), 0 );

	// Check Cache
	CacheSprite cacheSprite;
	if( Assets::cache.fetch( cacheID, cacheSprite ) )
	{
		imageWidth = cacheSprite.imageWidth;
		imageHeight = cacheSprite.imageHeight;
		imageChannels = cacheSprite.imageChannels;
	}
	else
	{
		Assets::cache.dirty |= true; // Dirty Cache
		stbi_info( fileImage.path, &imageWidth, &imageHeight, &imageChannels );
	}

	// Validate Image Dimensions
	ErrorIf( imageWidth <= 0 || imageHeight <= 0 || imageChannels <= 0,
		"Sprite '%s' has invalid dimensions: (w: %d, h: %d, c: %d)",
		imageWidth, imageHeight, imageChannels );
	ErrorIf( imageWidth > U16_MAX || imageHeight > U16_MAX || imageChannels > 4,
		"Sprite '%s' has invalid dimensions: (w: %d, h: %d, c: %d)",
		imageWidth, imageHeight, imageChannels );

	// Register Sprite
	Sprite &sprite = sprites.add( Sprite { } );
	sprite.name = fileDefinition.name;
	sprite.count = static_cast<u16>( count );
	sprite.width = static_cast<u16>( imageWidth / count );
	sprite.height = static_cast<u16>( imageHeight );
	sprite.xorigin = static_cast<i16>( xorigin );
	sprite.yorigin = static_cast<i16>( yorigin );
	sprite.textureID = Assets::textures.make_new( atlasName );
	sprite.glyphID = GLYPHID_MAX;

	// Split sprite into individual glyphs
	for( u16 i = 0; i < sprite.count; i++ )
	{
		Glyph glyph;
#if 0
		snprintf( cacheIDBuffer, sizeof( cacheIDBuffer ), "subimg %u|%u", cacheID, i );
		glyph.cacheID = checksum_xcrc32( cacheIDBuffer, sizeof( cacheIDBuffer ), 0 );
#else
		glyph.cacheID = cacheID + i;
#endif
		glyph.imageX1 = sprite.width * i;
		glyph.imageY1 = 0;
		glyph.imageX2 = glyph.imageX1 + sprite.width;
		glyph.imageY2 = sprite.height;
		glyph.texturePath = pathImage;

		Texture &texture = Assets::textures[sprite.textureID];
		GlyphID glyphID = texture.add_glyph( static_cast<Glyph &&>( glyph ) );
		if( sprite.glyphID == GLYPHID_MAX ) { sprite.glyphID = glyphID; } // Store the first glyph only
	}

	// Cache
	cacheSprite.imageWidth = imageWidth;
	cacheSprite.imageHeight = imageHeight;
	cacheSprite.imageChannels = imageChannels;
	Assets::cache.store( cacheID, cacheSprite );
}


void Sprites::build()
{
	Buffer &binary = Assets::binary;
	String &header = Assets::header;
	String &source = Assets::source;
	const u32 count = static_cast<u32>( sprites.size() );

	Timer timer;

	// Load
	{
		// ...
	}

	// Binary
	{
		// ...
	}

	// Header
	{
		// Group
		assets_group( header );

		// Enums
		header.append( "enum_class\n(\n\tSprite, u32,\n\n" );
		for( Sprite &sprite : sprites ) { header.append( "\t" ).append( sprite.name ).append( ",\n" ); }
		header.append( "\n\tNull = 0,\n" );
		header.append( ");\n\n" );

		// Struct
		header.append( "namespace Assets { struct SpriteEntry; }\n\n" );

		// Table
		header.append( "namespace CoreAssets\n{\n" );
		header.append( "\tconstexpr u32 spriteCount = " ).append( count ).append( ";\n" );
		header.append( count > 0 ? "\textern const Assets::SpriteEntry sprites[spriteCount];\n" :
			"\textern const Assets::SpriteEntry *sprites;\n" );
		header.append( "}\n\n" );
	}

	// Source
	{
		// Group
		assets_group( source );
		source.append( "namespace CoreAssets\n{\n" );

		// Table
		if( count > 0 )
		{
			char buffer[PATH_SIZE];
			source.append( "\tconst Assets::SpriteEntry sprites[spriteCount] =\n\t{\n" );
			for( Sprite &sprite : sprites )
			{
				snprintf( buffer, PATH_SIZE,
					"\t\t{ %d, %d, %d, %d, %d, %d, %d }, // %s\n",
					sprite.glyphID,
					sprite.textureID,
					sprite.count,
					sprite.width,
					sprite.height,
					sprite.xorigin,
					sprite.yorigin,

					sprite.name.cstr() );

				source.append( buffer );
			}
			source.append( "\t};\n" );
		}
		else
		{
			source.append( "\tconst Assets::SpriteEntry *sprites = nullptr;\n" );
		}

		source.append( "}\n\n" );
	}

	if( verbose_output() )
	{
		const usize count = sprites.size();
		Print( PrintColor_White, TAB TAB "Wrote %d sprite%s", count, count == 1 ? "" : "s" );
		PrintLn( PrintColor_White, " (%.3f ms)", timer.elapsed_ms() );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////