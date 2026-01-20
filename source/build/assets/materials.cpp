#include <build/assets/materials.hpp>

#include <vendor/stb/stb_image.hpp>

#include <core/list.hpp>
#include <core/types.hpp>
#include <core/debug.hpp>
#include <core/json.hpp>
#include <core/checksum.hpp>

#include <build/build.hpp>
#include <build/assets.hpp>
#include <build/assets/textures.hpp>
#include <build/filesystem.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct CacheMaterial
{
	int imageColorWidth;
	int imageColorHeight;
	int imageColorChannels;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Materials::make_new( const Material &material )
{
	materials.add( material );
}


usize Materials::gather( const char *path, bool recurse )
{
	// Gather Materials
	List<FileInfo> files;
	directory_iterate( files, path, ".material", recurse );

	// Process Materials
	for( FileInfo &fileInfo : files )
	{
		Assets::cacheFileCount++;
		process( fileInfo.path );
	}

	return files.size();
}


void Materials::process( const char *path )
{
	// Local Directory
	static char pathDirectory[PATH_SIZE];
	path_get_directory( pathDirectory, sizeof( pathDirectory ), path );

	// Register Definition File
	AssetFile fileDefinition;
	if( !asset_file_register( fileDefinition, path ) )
	{
		Error( "Unable to locate material file: %s", path );
		return;
	}

	// Open Definition JSON
	String fileDefinitionContents;
	if( !fileDefinitionContents.load( path ) )
	{
		Error( "Unable to open material file: %s", path );
		return;
	}
	JSON fileDefinitionJSON { fileDefinitionContents };

	// Parse Definition JSON
	static char pathImageColor[PATH_SIZE];
	String pathImageColorRelative = fileDefinitionJSON.get_string( "colorTexture" );
	ErrorIf( pathImageColorRelative.length_bytes() == 0,
		"Material '%s' has an invalid colorTexture path (required)", fileDefinition.name );
	snprintf( pathImageColor, sizeof( pathImageColor ),
		"%s" SLASH "%s", pathDirectory, pathImageColorRelative.cstr() );

	// Register Color Image File
	AssetFile fileImageColor;
	if( !asset_file_register( fileImageColor, pathImageColor ) )
	{
		Error( "Material '%s' - Unable to locate colorTexture file: '%s'", fileDefinition.name, pathImageColor );
		return;
	}

	// Generate Cache ID
	static char cacheIDBuffer[PATH_SIZE * 3];
	memory_set( cacheIDBuffer, 0, sizeof( cacheIDBuffer ) );
	snprintf( cacheIDBuffer, sizeof( cacheIDBuffer ), "material %s|%llu|%s|%llu",
		fileDefinition.path, fileDefinition.time.as_u64(),
		fileImageColor.path, fileImageColor.time.as_u64() );
	const CacheID cacheID = checksum_xcrc32( cacheIDBuffer, sizeof( cacheIDBuffer ), 0 );

	// Check Cache
	int imageColorWidth = 0;
	int imageColorHeight = 0;
	int imageColorChannels = 0;
	CacheMaterial cacheMaterial;
	if( Assets::cache.fetch( cacheID, cacheMaterial ) )
	{
		imageColorWidth = cacheMaterial.imageColorWidth;
		imageColorHeight = cacheMaterial.imageColorHeight;
		imageColorChannels = cacheMaterial.imageColorChannels;
	}
	else
	{
		Assets::cache.dirty |= true; // Dirty Cache
		stbi_info( fileImageColor.path, &imageColorWidth, &imageColorHeight, &imageColorChannels );
	}

	// Validate Image Dimensions
	ErrorIf( imageColorWidth <= 0 || imageColorHeight <= 0 || imageColorChannels <= 0,
		"Material '%s' has invalid dimensions: (w: %d, h: %d, c: %d)",
		imageColorWidth, imageColorHeight, imageColorChannels );
	ErrorIf( imageColorWidth > U16_MAX || imageColorHeight > U16_MAX || imageColorChannels > 4,
		"Material '%s' has invalid dimensions: (w: %d, h: %d, c: %d)",
		imageColorWidth, imageColorHeight, imageColorChannels );

	// Register Material
	Material &material = materials.add( Material { } );
	material.name = fileDefinition.name;
	material.textureIDColor = Assets::textures.make_new( material.name );
	Texture &colorTextureAsset = Assets::textures[material.textureIDColor];
	colorTextureAsset.atlasTexture = false;
	colorTextureAsset.generateMips = true;

	Glyph glyphColor;
	glyphColor.cacheID = cacheID;
	glyphColor.texturePath = pathImageColor;
	glyphColor.imageX1 = static_cast<u16>( 0 );
	glyphColor.imageY1 = static_cast<u16>( 0 );
	glyphColor.imageX2 = static_cast<u16>( imageColorWidth );
	glyphColor.imageY2 = static_cast<u16>( imageColorHeight );
	colorTextureAsset.add_glyph( static_cast<Glyph &&>( glyphColor ) );

	// Cache
	cacheMaterial.imageColorWidth = imageColorWidth;
	cacheMaterial.imageColorHeight = imageColorHeight;
	cacheMaterial.imageColorChannels = imageColorChannels;
	Assets::cache.store( cacheID, cacheMaterial );
}


void Materials::build()
{
	Buffer &binary = Assets::binary;
	String &header = Assets::header;
	String &source = Assets::source;
	const u32 count = static_cast<u32>( materials.size() );

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
		header.append( "enum_class\n(\n\tMaterial, u32,\n\n" );
		for( Material &material : materials ) { header.append( "\t" ).append( material.name ).append( ",\n" ); }
		header.append( "\n\tNull = 0,\n" );
		header.append( ");\n\n" );

		// Struct
		header.append( "namespace Assets { struct MaterialEntry; }\n\n" );

		// Table
		header.append( "namespace CoreAssets\n{\n" );
		header.append( "\tconstexpr u32 materialCount = " ).append( count ).append( ";\n" );
		header.append( count > 0 ? "\textern const Assets::MaterialEntry materials[materialCount];\n" :
			"\textern const Assets::MaterialEntry *materials;\n" );
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
			source.append( "\tconst Assets::MaterialEntry materials[materialCount] =\n\t{\n" );
			for( Material &material : materials )
			{
				snprintf( buffer, PATH_SIZE,
					"\t\t{ %d, %d }, // %s\n",
					material.textureIDColor,
					material.textureIDNormal,

					material.name.cstr() );

				source.append( buffer );
			}
			source.append( "\t};\n" );
		}
		else
		{
			source.append( "\tconst Assets::MaterialEntry *materials = nullptr;\n" );
		}

		source.append( "}\n\n" );
	}

	if( verbose_output() )
	{
		const usize count = materials.size();
		Print( PrintColor_White, TAB TAB "Wrote %d material%s", count, count == 1 ? "" : "s" );
		PrintLn( PrintColor_White, " (%.3f ms)", timer.elapsed_ms() );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////