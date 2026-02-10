#include <build/assets/data.hpp>

#include <core/types.hpp>
#include <core/debug.hpp>
#include <core/list.hpp>
#include <core/json.hpp>
#include <core/checksum.hpp>

#include <build/build.hpp>
#include <build/assets.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct CacheDataAsset
{
	usize offset;
	usize size;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DataAssetID DataAssets::register_new( const DataAsset &asset )
{
	AssertMsg( dataAssets.count() < MATERIALID_MAX, "Exceeded max number of DataAssets" );
	dataAssets.add( asset );
	return static_cast<DataAssetID>( dataAssets.count() - 1 );
}


DataAssetID DataAssets::register_new( DataAsset &&asset )
{
	AssertMsg( dataAssets.count() < MATERIALID_MAX, "Exceeded max number of DataAssets" );
	dataAssets.add( static_cast<DataAsset &&>( asset ) );
	return static_cast<DataAssetID>( dataAssets.count() - 1 );
}


DataAssetID DataAssets::register_new_from_definition( String name, const char *path )
{
	static char pathDirectory[PATH_SIZE];
	path_get_directory( pathDirectory, sizeof( pathDirectory ), path );

	// Register Definition File
	AssetFile fileDefinition;
	if( !asset_file_register( fileDefinition, path ) )
	{
		Error( "Unable to locate asset file: %s", path );
		return DATAASSETID_NULL;
	}

	// Open Definition JSON
	String fileDefinitionContents;
	if( !fileDefinitionContents.load( path ) )
	{
		Error( "Unable to open asset file: %s", path );
		return DATAASSETID_NULL;
	}
	JSON fileDefinitionJSON { fileDefinitionContents };

	// Parse Definition JSON
	static char pathAsset[PATH_SIZE];
	String pathAssetRelative = fileDefinitionJSON.get_string( "path" );
	ErrorIf( pathAssetRelative.length_bytes() == 0,
		"DataAsset '%s' has an invalid path (required)", fileDefinition.name );
	snprintf( pathAsset, sizeof( pathAsset ),
		"%s" SLASH "%s", pathDirectory, pathAssetRelative.cstr() );

	// Register Image File
	AssetFile fileAsset;
	if( !asset_file_register( fileAsset, pathAsset ) )
	{
		Error( "DataAsset '%s' - Unable to locate asset file: '%s'", fileDefinition.name, pathAsset );
		return DATAASSETID_NULL;
	}

	// Check Cache
	CacheDataAsset cacheAsset;
	const CacheKey cacheKey = Hash::hash64_from( fileDefinition.cache_id(), fileAsset.cache_id() );
	if( !Assets::cache.fetch( cacheKey, cacheAsset ) )
	{
		Assets::cache.dirty |= true; // Dirty Cache
	}

	// Register Asset
	const DataAssetID dataAssetID = register_new();
	DataAsset &asset = dataAssets[dataAssetID];
	asset.cacheKey = cacheKey;
	asset.name = name;
	asset.path = fileAsset.path;

	return dataAssetID;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

usize DataAssets::gather( const char *path, bool recurse )
{
	List<FileInfo> files;
	directory_iterate( files, path, ".asset", recurse );

	static char name[PATH_SIZE];
	for( FileInfo &fileInfo : files )
	{
		Assets::cacheFileCount++;
		path_remove_extension( name, sizeof( name ), fileInfo.name );
		register_new_from_definition( name, fileInfo.path );
	}

	return files.size();
}


void DataAssets::build()
{
	Buffer &binary = Assets::binary;
	String &header = Assets::header;
	String &source = Assets::source;
	const u32 count = static_cast<u32>( dataAssets.size() );

	Timer timer;

	// Load
	{
		for( DataAsset &asset : dataAssets )
		{
			CacheDataAsset cacheAsset;
			if( Assets::cache.fetch( asset.cacheKey, cacheAsset ) )
			{
				// Load from cached binary
				asset.data.write_from_file( Build::pathOutputRuntimeBinary,
					Assets::cacheReadOffset + cacheAsset.offset, cacheAsset.size );
				Assets::log_asset_cache( "Asset", asset.name.cstr() );
			}
			else
			{
				// Load from data file
				ErrorIf( !asset.data.load( asset.path ), "Failed to load asset file: %s", asset.path.cstr() );
				Assets::log_asset_build( "Asset", asset.name.cstr() );
			}
		}
	}

	// Binary
	{
		for( DataAsset &asset : dataAssets )
		{
			// Binary
			ErrorIf( asset.data.size() <= 0, "Invalid asset file %s", asset.path.cstr() );
			asset.offset = binary.write( asset.data.data, asset.data.size() );
			asset.size = asset.data.size();

			// Cache
			CacheDataAsset cacheAsset;
			cacheAsset.offset = asset.offset;
			cacheAsset.size = asset.size;
			Assets::cache.store( asset.cacheKey, cacheAsset );
		}
	}

	// Header
	{
		// Group
		assets_group( header );

		// Enums
		header.append( "enum_class\n(\n\tDataAsset, u32,\n\n" );
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
					"\t\t{ BINARY_OFFSET_ASSETS + %lluLLU, %lluLLU }, // %s\n",
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
		Print( PrintColor_White, TAB TAB "Wrote %d data asset%s", count, count == 1 ? "" : "s" );
		PrintLn( PrintColor_White, " (%.3f ms)", timer.elapsed_ms() );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////