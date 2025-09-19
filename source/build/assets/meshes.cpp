#include <build/assets/meshes.hpp>

#include <core/types.hpp>
#include <core/debug.hpp>
#include <core/list.hpp>

#include <build/build.hpp>
#include <build/assets.hpp>
#include <build/assets/textures.hpp>
#include <build/filesystem.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Meshes::make_new( const Mesh &mesh )
{
	meshes.add( mesh );
}


void Meshes::gather( const char *path, const bool recurse )
{
	// Gather & Load Meshes
	Timer timer;
	List<FileInfo> files;
	directory_iterate( files, path, ".mesh", recurse );
	for( FileInfo &fileInfo : files ) { load( fileInfo.path ); }

	// Log
	if( verbose_output() )
	{
		const u32 count = files.size();
		PrintColor( LOG_CYAN, TAB TAB "%u mesh%s found in: %s", count, count == 1 ? "" : "es", path );
		PrintLnColor( LOG_WHITE, " (%.3f ms)", timer.elapsed_ms() );
	}
}


void Meshes::load( const char *path )
{
	// Register Mesh
	Mesh &mesh = meshes.add( { } );
	mesh.filepath = path;

	// Build Cache
	Assets::assetFileCount++;
	if( !Build::cacheDirtyAssets )
	{
		FileTime time;
		file_time( path, &time );
		Build::cacheDirtyAssets |= file_time_newer( time, Assets::timeCache );
	}

	// Read Mesh File
	ErrorIf( !mesh.meshFile.load( path ), "Failed to load mesh '%s'", path );
}


void Meshes::write()
{
	Buffer &binary = Assets::binary;
	String &header = Assets::header;
	String &source = Assets::source;

	Timer timer;

	// Binary - do nothing
	{
		for( Mesh &mesh : meshes )
		{
			// Write Vertex Buffer Data
			mesh.vertexBufferOffset = binary.tell;
			binary.write( mesh.meshFile.vertexBufferData, mesh.meshFile.vertexBufferSize );

			// Write Index Buffer Data
			mesh.indexBufferOffset = USIZE_MAX;
		}
	}

	// Header
	{
		// Group
		assets_group( header );

		// Struct
		header.append( "struct BinMesh;\n\n" );

		// Table
		header.append( "namespace Assets\n{\n" );
		header.append( "\tconstexpr u32 meshCount = " );
		header.append( static_cast<int>( meshes.size() ) ).append( ";\n" );
		header.append( "\textern const BinMesh meshes[];\n" );
		header.append( "}\n\n" );
	}

	// Source
	{
		// Group
		assets_group( source );
		source.append( "namespace Assets\n{\n" );

		// Table
		char buffer[PATH_SIZE];
		source.append( "\tconst BinMesh meshes[meshCount] =\n\t{\n" );
		for( Mesh &mesh : meshes )
		{
			snprintf( buffer, PATH_SIZE,
				"\t\t{ %lluULL, %lluULL, %lluULL, %lluULL, %lluULL, %lluULL, %f, %f, %f, %f, %f, %f },\n",
				mesh.vertexBufferOffset,
				mesh.meshFile.vertexBufferSize,
				mesh.meshFile.vertexCount,
				mesh.indexBufferOffset,
				mesh.meshFile.indexBufferSize,
				mesh.meshFile.indexCount,
				mesh.minX,
				mesh.minY,
				mesh.minZ,
				mesh.maxX,
				mesh.maxY,
				mesh.maxZ );

			source.append( buffer );
		}
		source.append( "\t};\n" );
		source.append( "}\n\n" );
	}

	if( verbose_output() )
	{
		const usize count = meshes.size();
		PrintColor( LOG_CYAN, "\t\tWrote %d mesh%s", count, count == 1 ? "" : "es" );
		PrintLnColor( LOG_WHITE, " (%.3f ms)", timer.elapsed_ms() );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////