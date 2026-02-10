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

struct CacheMaterialTexture
{
	int width;
	int height;
	int channels;
};


void Material::allocate_texture_from_file( MaterialTextureSlot textureSlot, const char *textureName,
	const char *texturePath, bool textureGenerateMips )
{
	static char buffer[PATH_SIZE];
	snprintf( buffer, sizeof( buffer ), "Material_%s_%s_%s",
		this->name.cstr(), MaterialTextureSlotNames[textureSlot], textureName );
	String nameTexture = buffer;

	// Register Image File
	AssetFile file;
	if( !asset_file_register( file, texturePath ) )
	{
		Error( "Material '%s' - Unable to locate file: '%s'", this->name.cstr(), texturePath );
	}
	const CacheKey fileCacheKey = file.cache_id();

	// Check Cache
	int width = 0;
	int height = 0;
	int channels = 0;
	CacheMaterialTexture cacheMaterialTexture;
	if( Assets::cache.fetch( fileCacheKey, cacheMaterialTexture ) )
	{
		width = cacheMaterialTexture.width;
		height = cacheMaterialTexture.height;
		channels = cacheMaterialTexture.channels;
	}
	else
	{
		Assets::cache.dirty |= true; // Dirty Cache
		stbi_info( file.path, &width, &height, &channels );
	}

	// Validate Image Dimensions
	ErrorIf( width <= 0 || height <= 0 || channels <= 0,
		"Material '%s' has invalid dimensions: (w: %d, h: %d, c: %d)",
		width, height, channels );
	ErrorIf( width > U16_MAX || height > U16_MAX || channels > 4,
		"Material '%s' has invalid dimensions: (w: %d, h: %d, c: %d)",
		width, height, channels );

	// Register Texture
	this->textures[textureSlot] = Assets::textures.register_new( nameTexture );
	Texture &texture = Assets::textures[this->textures[textureSlot]];
	texture.atlasTexture = false;
	texture.generateMips = textureGenerateMips;

	Glyph glyphColor;
	glyphColor.cacheKey = fileCacheKey;
	glyphColor.texturePath = texturePath;
	glyphColor.imageX1 = static_cast<u16>( 0 );
	glyphColor.imageY1 = static_cast<u16>( 0 );
	glyphColor.imageX2 = static_cast<u16>( width );
	glyphColor.imageY2 = static_cast<u16>( height );
	texture.add_glyph( static_cast<Glyph &&>( glyphColor ) );

	// Cache
	cacheMaterialTexture.width = width;
	cacheMaterialTexture.height = height;
	cacheMaterialTexture.channels = channels;
	Assets::cache.store( fileCacheKey, cacheMaterialTexture );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MaterialID Materials::register_new( const Material &material )
{
	AssertMsg( materials.count() < MATERIALID_MAX, "Exceeded max number of Materials" );
	materials.add( material );
	return static_cast<MaterialID>( materials.count() - 1 );
}


MaterialID Materials::register_new( Material &&material )
{
	AssertMsg( materials.count() < MATERIALID_MAX, "Exceeded max number of Materials" );
	materials.add( static_cast<Material &&>( material ) );
	return static_cast<MaterialID>( materials.count() - 1 );
}


MaterialID Materials::register_new_from_definition( String name, const char *path )
{
	static char pathDirectory[PATH_SIZE];
	path_get_directory( pathDirectory, sizeof( pathDirectory ), path );

	auto try_parse_texture = []( Material &material, JSON &json,
		MaterialTextureSlot slot, const char *key )
	{
		String pathTextureRelative = json.get_string( key );
		if( pathTextureRelative.is_empty() ) { material.textures[slot] = 0; return; }

		static char pathTexture[PATH_SIZE];
		snprintf( pathTexture, sizeof( pathTexture ), "%s" SLASH "%s",
			pathDirectory, pathTextureRelative.cstr() );

		static char nameTexture[PATH_SIZE];
		path_get_filename( nameTexture, sizeof( nameTexture ), pathTextureRelative.cstr() );
		path_remove_extension( nameTexture, sizeof( nameTexture ) );

		material.allocate_texture_from_file( slot, nameTexture, pathTexture, true );
	};

	// Open JSON
	String definitionContents;
	if( !definitionContents.load( path ) )
	{
		Error( "Unable to open material file: %s", path );
		return MATERIALID_NULL;
	}
	JSON definitionJSON { definitionContents };

	// Allocate Material
	const MaterialID materialID = register_new();
	Material &material = materials[materialID];
	material.name = name;
	try_parse_texture( material, definitionJSON, MaterialTextureSlot_Diffuse, "diffuse" );
	try_parse_texture( material, definitionJSON, MaterialTextureSlot_Normal, "normal" );
	try_parse_texture( material, definitionJSON, MaterialTextureSlot_Specular, "specular" );

	return materialID;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

usize Materials::gather( const char *path, bool recurse )
{
	List<FileInfo> files;
	directory_iterate( files, path, ".material", recurse );

	static char name[PATH_SIZE];
	for( FileInfo &fileInfo : files )
	{
		Assets::cacheFileCount++;
		path_remove_extension( name, sizeof( name ), fileInfo.name );
		register_new_from_definition( name, fileInfo.path );
	}

	return files.count();
}


void Materials::build()
{
	Buffer &binary = Assets::binary;
	String &header = Assets::header;
	String &source = Assets::source;
	const u32 count = static_cast<u32>( materials.count() );

	Timer timer;

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
					"\t\t{ %u, %u, %u }, // %s\n",
					material.textures[MaterialTextureSlot_Diffuse],
					material.textures[MaterialTextureSlot_Normal],
					material.textures[MaterialTextureSlot_Specular],

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
		const usize count = materials.count();
		Print( PrintColor_White, TAB TAB "Wrote %d material%s", count, count == 1 ? "" : "s" );
		PrintLn( PrintColor_White, " (%.3f ms)", timer.elapsed_ms() );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////