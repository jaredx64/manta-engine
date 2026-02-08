#include <build/assets/skeleton2d.hpp>

#include <core/types.hpp>
#include <core/debug.hpp>
#include <core/list.hpp>
#include <core/json.hpp>
#include <core/checksum.hpp>

#include <build/build.hpp>
#include <build/assets.hpp>
#include <build/assets/textures.hpp>
#include <build/filesystem.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct CacheSkeleton2D
{
	usize offset;
	usize size;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Skeleton2Ds::allocate_new( const Skeleton2D &skeleton )
{
	skeletons.add( skeleton );
}


usize Skeleton2Ds::gather( const char *path, bool recurse )
{
	// Gather Skeletons
	List<FileInfo> files;
	directory_iterate( files, path, ".skeleton2d", recurse );

	// Process Skeletons
	for( FileInfo &fileInfo : files )
	{
		Assets::cacheFileCount++;
		process( fileInfo.path );
	}

	return files.size();
}


void Skeleton2Ds::process( const char *path )
{
	// Local Directory
	static char pathDirectory[PATH_SIZE];
	path_get_directory( pathDirectory, sizeof( pathDirectory ), path );

	// Register Definition File
	AssetFile fileDefinition;
	if( !asset_file_register( fileDefinition, path ) )
	{
		Error( "Unable to locate skeleton2D file: %s", path );
		return;
	}

	// Open Definition JSON
	String fileDefinitionContents;
	if( !fileDefinitionContents.load( path ) )
	{
		Error( "Unable to open skeleton2d file: %s", path );
		return;
	}
	JSON fileDefinitionJSON { fileDefinitionContents };

	// Parse Definition JSON
	static char pathSkeleton[PATH_SIZE];
	String pathSkeletonRelative = fileDefinitionJSON.get_string( "path" );
	ErrorIf( pathSkeletonRelative.length_bytes() == 0,
		"Skeleton2D '%s' has an invalid path (required)", fileDefinition.name );
	snprintf( pathSkeleton, sizeof( pathSkeleton ),
		"%s" SLASH "%s", pathDirectory, pathSkeletonRelative.cstr() );

	// Register Image File
	AssetFile fileAsset;
	if( !asset_file_register( fileAsset, pathSkeleton ) )
	{
		Error( "Skeleton2D '%s' - Unable to locate skeleton file: '%s'", fileDefinition.name, pathSkeleton );
		return;
	}

	// Generate Cache ID
	static char cacheIDBuffer[PATH_SIZE * 3];
	memory_set( cacheIDBuffer, 0, sizeof( cacheIDBuffer ) );
	snprintf( cacheIDBuffer, sizeof( cacheIDBuffer ), "skeleton2D %s|%llu|%s|%llu",
		fileDefinition.path, fileDefinition.time.as_u64(),
		fileAsset.path, fileAsset.time.as_u64() );
	const CacheID cacheID = checksum_xcrc32( cacheIDBuffer, sizeof( cacheIDBuffer ), 0 );

	// Check Cache
	CacheSkeleton2D cacheSkeleton;
	if( !Assets::cache.fetch( cacheID, cacheSkeleton ) )
	{
		Assets::cache.dirty |= true; // Dirty Cache
	}

	// Register Asset
	Skeleton2D &asset = skeletons.add( Skeleton2D { } );
	asset.cacheID = cacheID;
	asset.name = fileDefinition.name;
	asset.path = fileAsset.path;
}


void Skeleton2Ds::build()
{
	Buffer &binary = Assets::binary;
	String &header = Assets::header;
	String &source = Assets::source;
	const u32 count = static_cast<u32>( skeletons.size() );

	Timer timer;

	// Load
	{
		for( Skeleton2D &skeleton : skeletons )
		{
			CacheSkeleton2D cacheSkeleton;
			if( Assets::cache.fetch( skeleton.cacheID, cacheSkeleton ) )
			{
				// Load from cached binary
				skeleton.data.write_from_file( Build::pathOutputRuntimeBinary,
					Assets::cacheReadOffset + cacheSkeleton.offset, cacheSkeleton.size );
			}
			else
			{
				// Load from data file
				ErrorIf( !skeleton.data.load( skeleton.path ),
					"Failed to load asset file: %s", skeleton.path.cstr() );
			}
		}
	}

	// Binary
	{
		// TODO: ...
	}

	// Header
	{
		// Group
		assets_group( header );

		// Enums
		header.append( "enum_class\n(\n\tSkeleton, u32,\n\n" );
		for( Skeleton2D &sk : skeletons ) { header.append( "\t" ).append( sk.name ).append( ",\n" ); }
		header.append( "\n\tNull = 0,\n" );
		header.append( ");\n\n" );

		// Struct
		header.append( "namespace Assets { struct Skeleton2DEntry; }\n\n" );

		// Table
		header.append( "namespace CoreAssets\n{\n" );
		header.append( "\tconstexpr u32 skeleton2DCount = " ).append( count ).append( ";\n" );
		header.append( count > 0 ? "\textern const Assets::Skeleton2DEntry skeleton2Ds[skeleton2DCount];\n" :
			"\textern const Assets::Skeleton2DEntry *skeleton2Ds;\n" );
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
			char name[PATH_SIZE];
			source.append( "\tconst Assets::Skeleton2DEntry skeleton2Ds[skeleton2DCount] =\n\t{\n" );
			for( Skeleton2D &skeleton : skeletons )
			{
				path_get_filename( name, sizeof( name ), skeleton.name.cstr() );

				snprintf( buffer, PATH_SIZE,
					"\t\t{ \"%s\" },\n",
					name );

				source.append( buffer );
			}
			source.append( "\t};\n" );
		}
		else
		{
			source.append( "\tconst Assets::Skeleton2DEntry *skeleton2Ds = nullptr;\n" );
		}

		source.append( "}\n\n" );
	}

	if( verbose_output() )
	{
		const usize count = skeletons.size();
		Print( PrintColor_White, TAB TAB "Wrote %d skeleton2D%s", count, count == 1 ? "" : "s" );
		PrintLn( PrintColor_White, " (%.3f ms)", timer.elapsed_ms() );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////