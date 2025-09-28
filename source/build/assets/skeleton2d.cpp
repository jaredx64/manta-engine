#include <build/assets/skeleton2d.hpp>

#include <core/types.hpp>
#include <core/debug.hpp>
#include <core/list.hpp>

#include <build/build.hpp>
#include <build/assets.hpp>
#include <build/assets/textures.hpp>
#include <build/filesystem.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Skeleton2Ds::make_new( const Skeleton &skeleton )
{
	skeletons.add( skeleton );
}


void Skeleton2Ds::gather( const char *path, const bool recurse )
{
	// Gather & Load Skeletons
	Timer timer;
	List<FileInfo> files;
	directory_iterate( files, path, ".spine", recurse );
	for( FileInfo &fileInfo : files ) { load( fileInfo.path ); }

	// Log
	if( verbose_output() )
	{
		const u32 count = files.size();
		PrintColor( LOG_CYAN, TAB TAB "%u skeleton%s found in: %s", count, count == 1 ? "" : "es", path );
		PrintLnColor( LOG_WHITE, " (%.3f ms)", timer.elapsed_ms() );
	}
}


void Skeleton2Ds::load( const char *path )
{
	// Register Skeleton
	Skeleton &skeleton = skeletons.add( { } );

	// Build Cache
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
	skeleton.name = assetName;

	// Read Skeleton File
	//ErrorIf( !skeleton.skeletonFile.load( path ), "Failed to load skeleton '%s'", path );
}


void Skeleton2Ds::write()
{
	Buffer &binary = Assets::binary;
	String &header = Assets::header;
	String &source = Assets::source;
	const u32 count = static_cast<u32>( skeletons.size() );

	Timer timer;

	// Binary - do nothing
	{
		for( Skeleton &skeleton : skeletons )
		{
			// ...
		}
	}

	// Header
	{
		// Group
		assets_group( header );

		// Enums
		header.append( "enum_class_type\n(\n\tSkeleton2D, u32,\n\n" );
		for( Skeleton &sk : skeletons ) { header.append( "\t" ).append( sk.name ).append( ",\n" ); }
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
			for( Skeleton &skeleton : skeletons )
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
		PrintColor( LOG_CYAN, "\t\tWrote %d skeleton2D%s", count, count == 1 ? "" : "s" );
		PrintLnColor( LOG_WHITE, " (%.3f ms)", timer.elapsed_ms() );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////