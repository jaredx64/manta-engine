#include <build/assets/materials.hpp>

#include <core/list.hpp>
#include <core/types.hpp>
#include <core/debug.hpp>
#include <core/json.hpp>

#include <build/build.hpp>
#include <build/assets.hpp>
#include <build/assets/textures.hpp>
#include <build/filesystem.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Materials::make_new( const Material &material )
{
	materials.add( material );
}


void Materials::gather( const char *path, const bool recurse )
{
	// Gather & Load Materials
	Timer timer;
	List<FileInfo> files;
	directory_iterate( files, path, ".material", recurse );
	for( FileInfo &fileInfo : files ) { load( fileInfo.path ); }

	// Log
	if( verbose_output() )
	{
		const u32 count = files.size();
		PrintColor( LOG_CYAN, TAB TAB "%u material%s found in: %s", count, count == 1 ? "" : "s", path );
		PrintLnColor( LOG_WHITE, " (%.3f ms)", timer.elapsed_ms() );
	}
}


void Materials::load( const char *path )
{
	// Open material file
	String materialFile;
	ErrorIf( !materialFile.load( path ), "Unable to load material file: %s", path );
	JSON materialJSON { materialFile };

	// Build Cache
	Assets::assetFileCount++;
	if( !Build::cacheDirtyAssets )
	{
		FileTime time;
		file_time( path, &time );
		Build::cacheDirtyAssets |= file_time_newer( time, Assets::timeCache );
	}

	// Read file (json)
	String name = materialJSON.get_string( "name" );
	ErrorIf( name.length_bytes() == 0, "Material '%s' has an invalid name (required)", path );

	String colorTexture = materialJSON.get_string( "colorTexture" );
	ErrorIf( colorTexture.length_bytes() == 0, "Material '%s' has an invalid color texture (required)", path );
	//String normalTexture = materialJSON.GetString( "normalTexture" );
	//ErrorIf( normalTexture.length() == 0, "Material '%s' has an invalid normal texture (required)", path );

	// Load texture (try relative path first)
	char pathRelative[PATH_SIZE];
	path_get_directory( pathRelative, sizeof( pathRelative ), path );
	strappend( pathRelative, SLASH );
	strappend( pathRelative, colorTexture.cstr() );
	Texture2DBuffer colorTextureBuffer { pathRelative };
	if( !colorTextureBuffer )
	{
		// Relative path failed -- try absolute path
		colorTextureBuffer.load( colorTexture.cstr() );
		if( !colorTextureBuffer ) { Error( "Unable to load color texture for material %s (texture: %s)", path, colorTexture.cstr() ); }
	}

	// Register Material
	Material material;
	material.name = name;
	material.textureIDColor = Assets::textures.make_new( name ); // TODO: Generate unique name
	Texture &colorTextureAsset = Assets::textures[material.textureIDColor];
	colorTextureAsset.atlasTexture = false;
	colorTextureAsset.add_glyph( static_cast<Texture2DBuffer &&>( colorTextureBuffer ) );

	// Register Material
	materials.add( material );
}


void Materials::write()
{
	Buffer &binary = Assets::binary;
	String &header = Assets::header;
	String &source = Assets::source;

	Timer timer;

	// Binary - do nothing
	{
		// ...
	}

	// Header
	{
		// Group
		assets_group( header );

		// Struct
		header.append( "struct BinMaterial;\n\n" );

		// Enums
		header.append( "enum\n{\n" );
		for( Material &material : materials )
		{
			header.append( "\t" ).append( material.name ).append( ",\n" );
		}
		header.append( "};\n\n" );

		// Table
		header.append( "namespace Assets\n{\n" );
		header.append( "\tconstexpr u32 materialsCount = " );
		header.append( static_cast<int>( materials.size() ) ).append( ";\n" );
		header.append( "\textern const BinMaterial materials[];\n" );
		header.append( "}\n\n" );
	}

	// Source
	{
		// Group
		assets_group( source );
		source.append( "namespace Assets\n{\n" );

		// Table
		char buffer[PATH_SIZE];
		source.append( "\tconst BinMaterial materials[materialsCount] =\n\t{\n" );
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

		source.append( "}\n\n" );
	}

	if( verbose_output() )
	{
		const usize count = materials.size();
		PrintColor( LOG_CYAN, "\t\tWrote %d material%s", count, count == 1 ? "" : "s" );
		PrintLnColor( LOG_WHITE, " (%.3f ms)", timer.elapsed_ms() );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////