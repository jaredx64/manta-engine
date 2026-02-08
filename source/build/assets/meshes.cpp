#include <build/assets/meshes.hpp>

#include <core/types.hpp>
#include <core/debug.hpp>
#include <core/list.hpp>
#include <core/json.hpp>
#include <core/checksum.hpp>

#include <build/build.hpp>
#include <build/assets.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct CacheMesh
{
	MeshFormatTypeVertex formatVertex;
	usize offsetVertex;
	usize sizeVertex;
	MeshFormatTypeIndex formatIndex;
	usize offsetIndex;
	usize sizeIndex;
	float x1, y1, z1;
	float x2, y2, z2;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MeshID Meshes::allocate_new( const String &name, CacheID cacheID,
	MeshFormatTypeVertex formatVertex, void *dataVertex, usize sizeVertex,
	MeshFormatTypeIndex formatIndex, void *dataIndex, usize sizeIndex,
	float x1, float y1, float z1, float x2, float y2, float z2 )
{
	Assert( dataVertex != nullptr );
	Assert( sizeVertex != 0LLU );
	AssertMsg( meshes.size() < MESHID_MAX, "Exceeded max number of Meshes" );

	Mesh &mesh = meshes.add( Mesh { } );
	mesh.cacheID = cacheID;

	mesh.formatVertex = formatVertex;
	mesh.dataVertex.clear();
	mesh.dataVertex.write( dataVertex, sizeVertex );

	mesh.formatIndex = formatIndex;
	mesh.dataIndex.clear();
	if( dataIndex != nullptr ) { mesh.dataIndex.write( dataIndex, sizeIndex ); }

	mesh.x1 = x1;
	mesh.y1 = y1;
	mesh.z1 = z1;
	mesh.x2 = x2;
	mesh.y2 = y2;
	mesh.z2 = z2;

	if( name.is_empty() )
	{
		char buffer[64];
		const u32 checksum = checksum_xcrc32( reinterpret_cast<char *>( dataVertex ), sizeVertex, 12345 );
		snprintf( buffer, sizeof( buffer ), "mesh_%u", checksum );
		mesh.name = buffer;
	}
	else
	{
		mesh.name = name;
	}

	return static_cast<MeshID>( meshes.count() - 1 );
}


MeshID Meshes::retrieve_from_cache( const String &name, CacheID cacheID )
{
	CacheMesh cache;
	if( Assets::cache.fetch( cacheID, cache ) )
	{
		// Vertex Data
		static Buffer dataVertexCache;
		Assert( cache.formatVertex < MESHFORMATTYPEVERTEX_COUNT );
		dataVertexCache.clear();
		Assert( cache.sizeVertex != 0LLU );
		dataVertexCache.write_from_file( Build::pathOutputRuntimeBinary,
			Assets::cacheReadOffset + cache.offsetVertex, cache.sizeVertex );

		// Index Data
		static Buffer dataIndexCache;
		Assert( cache.formatIndex < MESHFORMATTYPEINDEX_COUNT );
		dataIndexCache.clear();
		if( cache.sizeIndex != 0LLU )
		{
			dataIndexCache.write_from_file( Build::pathOutputRuntimeBinary,
				Assets::cacheReadOffset + cache.offsetIndex, cache.sizeIndex );
		}

		return allocate_new( name, cacheID,
			cache.formatVertex, dataVertexCache.data, dataVertexCache.size(),
			cache.formatIndex, dataIndexCache.data, dataIndexCache.size(),
			cache.x1, cache.y1, cache.z1,
			cache.x2, cache.y2, cache.x2 );
	}

	return MESHID_NULL;
}


void Meshes::build()
{
	Buffer &binary = Assets::binary;
	String &header = Assets::header;
	String &source = Assets::source;
	const u32 count = static_cast<u32>( meshes.size() );

	Timer timer;

	// Binary
	{
		for( Mesh &mesh : meshes )
		{
			// Binary
			const bool hasVertex = mesh.dataVertex.is_initialized() && mesh.dataVertex.size() > 0;
			AssertMsg( hasVertex, "Mesh asset does not have vertex data!" );
			mesh.offsetVertex = hasVertex ? binary.write( mesh.dataVertex.data, mesh.dataVertex.size() ) : USIZE_MAX;
			mesh.sizeVertex = hasVertex ? mesh.dataVertex.size() : 0LLU;
			const bool hasIndex = mesh.dataIndex.is_initialized() && mesh.dataIndex.size() > 0;
			mesh.offsetIndex = hasIndex ? binary.write( mesh.dataIndex.data, mesh.dataIndex.size() ) : USIZE_MAX;
			mesh.sizeIndex = hasIndex ? mesh.dataIndex.size() : 0LLU;

			// Cache
			CacheMesh cache;
			cache.formatVertex = mesh.formatVertex;
			cache.offsetVertex = mesh.offsetVertex;
			cache.sizeVertex = mesh.sizeVertex;
			cache.formatIndex = mesh.formatIndex;
			cache.offsetIndex = mesh.offsetIndex;
			cache.sizeIndex = mesh.sizeIndex;
			cache.x1 = mesh.x1;
			cache.y1 = mesh.y1;
			cache.z1 = mesh.z1;
			cache.x2 = mesh.x2;
			cache.y2 = mesh.y2;
			cache.z2 = mesh.z2;
			Assets::cache.store( mesh.cacheID, cache );
		}
	}

	// Header
	{
		// Group
		assets_group( header );

		// Enums
		header.append( "enum_class\n(\n\tMesh, u32,\n\n" );
		for( Mesh &mesh : meshes ) { header.append( "\t" ).append( mesh.name ).append( ",\n" ); }
		header.append( "\n\tNull = 0,\n" );
		header.append( ");\n\n" );

		// Struct
		header.append( "namespace Assets { struct MeshEntry; }\n\n" );

		// Entries
		header.append( "namespace CoreAssets\n{\n" );
		header.append( "\tconstexpr u32 meshCount = " ).append( count ).append( ";\n" );
		header.append( count > 0 ? "\textern const Assets::MeshEntry meshes[meshCount];\n" :
			"\textern const Assets::MeshEntry *meshes;\n" );
		header.append( "}\n\n" );
	}

	// Source
	{
		// Group
		assets_group( source );
		source.append( "namespace CoreAssets\n{\n" );
		if( count > 0 )
		{
			// Assets::MeshEntry Table
			char buffer[PATH_SIZE];
			source.append( "\tconst Assets::MeshEntry meshes[meshCount] =\n\t{\n" );
			for( Mesh &mesh : meshes )
			{
				source.append( "\t\t{ " );

				snprintf( buffer, sizeof( buffer ), "%d, BINARY_OFFSET_ASSETS + %lluLLU, %lluLLU, ",
					mesh.formatVertex,
					mesh.offsetVertex,
					mesh.sizeVertex );
				source.append( buffer );

				snprintf( buffer, sizeof( buffer ), "%d, BINARY_OFFSET_ASSETS + %lluLLU, %lluLLU, ",
					mesh.formatIndex,
					mesh.offsetIndex,
					mesh.sizeIndex );
				source.append( buffer );

				snprintf( buffer, sizeof( buffer ), "%ff, %ff, %ff, %ff, %ff, %ff ",
					mesh.x1,
					mesh.y1,
					mesh.z1,
					mesh.x2,
					mesh.y2,
					mesh.z2 );
				source.append( buffer );

				source.append( " }, // " ).append( mesh.name ).append( "\n" );
			}
			source.append( "\t};\n" );
		}
		else
		{
			source.append( "\tconst Assets::MeshEntry *meshes = nullptr;\n" );
		}
		source.append( "}\n\n" );
	}

	if( verbose_output() )
	{
		const usize count = meshes.size();
		Print( PrintColor_White, TAB TAB "Wrote %d mesh%s", count, count == 1 ? "" : "es" );
		PrintLn( PrintColor_White, " (%.3f ms)", timer.elapsed_ms() );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////