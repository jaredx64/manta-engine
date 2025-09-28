#include <build/assets/data.hpp>

#include <core/types.hpp>
#include <core/debug.hpp>
#include <core/list.hpp>
#include <core/json.hpp>

#include <build/build.hpp>
#include <build/assets.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void load_asset_file( Buffer &data, const char *pathJSON, const char *pathAsset )
{
	// Load License (try relative path first)
	char pathRelative[PATH_SIZE];
	path_get_directory( pathRelative, sizeof( pathRelative ), pathJSON );
	strappend( pathRelative, SLASH ); strappend( pathRelative, pathAsset );
	if( !data.load( pathRelative ) )
	{
		// Relative path failed -- try absolute path
		ErrorIf( !data.load( pathAsset ), "Failed to load asset file: %s", pathAsset );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void DataAssets::make_new( const DataAsset &asset )
{
	dataAssets.add( asset );
}


void DataAssets::gather( const char *path, const bool recurse )
{
	// Gather & Load DataAssets
	Timer timer;
	List<FileInfo> files;
	directory_iterate( files, path, ".asset", recurse );
	for( FileInfo &fileInfo : files ) { load( fileInfo.path ); }

	// Log
	if( verbose_output() )
	{
		const u32 count = files.size();
		PrintColor( LOG_CYAN, TAB TAB "%u asset%s found in: %s", count, count == 1 ? "" : "es", path );
		PrintLnColor( LOG_WHITE, " (%.3f ms)", timer.elapsed_ms() );
	}
}


void DataAssets::load( const char *path )
{
	// Open JSON (.json)
	String file;
	ErrorIf( !file.load( path ), "Unable to load asset file: %s", path );
	JSON assetJSON { file };

	// Cache
	Assets::assetFileCount++;
	if( !Build::cacheDirtyAssets )
	{
		FileTime time;
		file_time( path, &time );
		Build::cacheDirtyAssets |= file_time_newer( time, Assets::timeCache );
	}

	// Asset Name (extracted from <name>.asset)
	char assetName[PATH_SIZE];
	path_get_filename( assetName, sizeof( assetName ), path );
	path_remove_extension( assetName );

	// Register DataAsset
	DataAsset &asset = dataAssets.add( { assetName } );

	// Read file (json)
	asset.filePath = assetJSON.get_string( "asset" );
	ErrorIf( asset.filePath.length_bytes() == 0, "Asset '%s' has no specified 'path'", path );

	// Load Asset File
	load_asset_file( asset.fileData, path, asset.filePath );
}


void DataAssets::write()
{
	Buffer &binary = Assets::binary;
	String &header = Assets::header;
	String &source = Assets::source;
	const u32 count = static_cast<u32>( dataAssets.size() );

	Timer timer;

	// Binary
	{
		for( DataAsset &asset : dataAssets )
		{
			asset.size = asset.fileData.size();
			ErrorIf( asset.size <= 0, "Invalid asset file %s", asset.filePath.cstr() );
			asset.offset = binary.tell;
			binary.write( asset.fileData.data, asset.size );
		}
	}

	// Header
	{
		// Group
		assets_group( header );

		// Enums
		header.append( "enum_class_type\n(\n\tDataAsset, u32,\n\n" );
		for( DataAsset &asset : dataAssets ) { header.append( "\t" ).append( asset.name ).append( ",\n" ); }
		header.append( "\n\tNull = 0,\n" );
		header.append( ");\n\n" );

		// Struct
		header.append( "namespace Assets { struct DataAssetEntry; }\n\n" );

		// Entries
		header.append( "namespace CoreAssets\n{\n" );
		header.append( "\tconstexpr u32 dataAssetCount = " ).append( count ).append( ";\n" );
		header.append( count > 0 ? "\textern const Assets::DataAssetEntry dataAssets[dataAssetCount];\n" :
			"\textern const Assets::DataAssetEntry *dataAssets;\n" );
		header.append( "}\n\n" );
	}

	// Source
	{
		// Group
		assets_group( source );
		source.append( "namespace CoreAssets\n{\n" );
		if( count > 0 )
		{
			// Assets::DataAssetEntry Table
			char buffer[PATH_SIZE];
			source.append( "\tconst Assets::DataAssetEntry dataAssets[dataAssetCount] =\n\t{\n" );
			for( DataAsset &asset : dataAssets )
			{
				snprintf( buffer, PATH_SIZE,
					"\t\t{ %lluULL, %lluULL }, // %s\n",
					asset.offset,
					asset.size,
					asset.name.cstr() );

				source.append( buffer );
			}
			source.append( "\t};\n" );
		}
		else
		{
			source.append( "\tconst Assets::DataAssetEntry *dataAssets = nullptr;\n" );
		}
		source.append( "}\n\n" );
	}

	if( verbose_output() )
	{
		const usize count = dataAssets.size();
		PrintColor( LOG_CYAN, "\t\tWrote %d assets%s", count, count == 1 ? "" : "s" );
		PrintLnColor( LOG_WHITE, " (%.3f ms)", timer.elapsed_ms() );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////