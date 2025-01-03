#include <build/objloader.hpp>

#include <config.hpp>

#include <vendor/stdlib.hpp>

#include <core/memory.hpp>
#include <core/list.hpp>
#include <core/debug.hpp>
#include <core/string.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool is_number( const char c )
{
	return ( c >= '0' && c <= '9' ) || c == '+' || c == '-' || c == '.';
}


static usize next_number( const String &file, usize &tell )
{
	// Early out
	if( file[tell] == '\n' || file[tell] == '\0' )
	{
		return USIZE_MAX;
	}

	// Seek to beginning of a number
	usize start = tell;
	while( !is_number( file[start] ) )
	{
		if( file[start] == '\n' || file[start] == '\0' )
		{
			tell = start;
			return USIZE_MAX;
		}

		start++;
	}

	// Seek to end of number
	usize end = start;
	while( is_number( file[end] ) ) { end++; }
	tell = end;

	// Return number position
	return start == end ? USIZE_MAX : start;
}


static bool next_float( const String &file, usize &tell, float &out )
{
	// Seek to beginning of a number
	usize index = next_number( file, tell );
	if( index == USIZE_MAX) { out = 0.0f; return false; }

	// Read float
	out = static_cast<float>( atof( file.get_pointer( index ) ) );
	return true;
}


static bool next_u32( const String &file, usize &tell, u32 &out )
{
	// Seek to beginning of a number
	usize index = next_number( file, tell );
	if( index == USIZE_MAX) { out = 0; return false; }

	// Read float
	out = static_cast<u32>( atoi( file.get_pointer( index ) ) );
	return true;
}


static usize next_key( const String &file, usize &tell, const char *key )
{
	tell = file.find( key, tell );
	if( tell == USIZE_MAX ) { return tell; }
	tell += strlen( key );
	return tell;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct VertexPosition
{
	float x, y, z;
};

static bool next_vertex_position( const String &file, usize &tell, List<VertexPosition> &positions )
{
	// Seek to position line
	tell = next_key( file, tell, "v " );
	if( tell == USIZE_MAX ) { return false; }

	// Register Position
	VertexPosition &vertex = positions.add( VertexPosition { } );
	next_float( file, tell, vertex.x );
	next_float( file, tell, vertex.y );
	next_float( file, tell, vertex.z );
	return true;
};

static void parse_vertex_positions( const String &file, List<VertexPosition> &positions )
{
	usize tell = 0;
	while( next_vertex_position( file, tell, positions ) ) { }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct VertexNormal
{
	float x, y, z;
};


static bool next_vertex_normal( const String &file, usize &tell, List<VertexNormal> &normals )
{
	// Seek to normal line
	tell = next_key( file, tell, "vn " );
	if( tell == USIZE_MAX ) { return false; }

	// Register Normal
	VertexNormal &normal = normals.add( VertexNormal { } );
	next_float( file, tell, normal.x );
	next_float( file, tell, normal.y );
	next_float( file, tell, normal.z );
	return true;
};


static void parse_vertex_normals( const String &file, List<VertexNormal> &normals )
{
	usize tell = 0;
	while( next_vertex_normal( file, tell, normals ) ) { }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct VertexUV
{
	float u, v, w;
};


static bool next_vertex_uv( const String &file, usize &tell, List<VertexUV> &uvs )
{
	// Seek to uv line
	tell = next_key( file, tell, "vt " );
	if( tell == USIZE_MAX ) { return false; }

	// Register UV
	VertexUV &uv = uvs.add( VertexUV { } );
	next_float( file, tell, uv.u );
	next_float( file, tell, uv.v );
	// next_float( file, tell, uv.w ); TODO: Is this ever even used?
	return true;
};


static void parse_vertex_uvs( const String &file, List<VertexUV> &uvs )
{
	usize tell = 0;
	while( next_vertex_uv( file, tell, uvs ) ) { }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Vertex
{
	u32 position;
	u32 normal;
	u32 uv;
};


static bool next_face( const String &file, usize &tell, List<Vertex> &vertices )
{
	// Seek to face line
	tell = next_key( file, tell, "f " );
	if( tell == USIZE_MAX ) { return false; }

	// Register Vertex1
	Vertex &v1 = vertices.add( Vertex { } );
	next_u32( file, tell, v1.position );
	next_u32( file, tell, v1.uv );
	next_u32( file, tell, v1.normal );

	// Register Vertex2
	Vertex &v2 = vertices.add( Vertex { } );
	next_u32( file, tell, v2.position );
	next_u32( file, tell, v2.uv );
	next_u32( file, tell, v2.normal );

	// Register Vertex2
	Vertex &v3 = vertices.add( Vertex { } );
	next_u32( file, tell, v3.position );
	next_u32( file, tell, v3.uv );
	next_u32( file, tell, v3.normal );

	// Success
	return true;
};


static void parse_faces( const String &file, List<Vertex> &vertices )
{
	usize tell = 0;
	while( next_face( file, tell, vertices ) ) { }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool MeshObj::load( const char *path )
{
	// Load Model File
	String file;
	if( !file.load( path ) ) { return false; }

	// Vertex Members
	List<VertexPosition> positions;
	List<VertexNormal> normals;
	List<VertexUV> uvs;
	List<Vertex> vertices;

	// Parse Members
	parse_vertex_positions( file, positions );
	//parse_vertex_normals( file, normals );
	parse_vertex_uvs( file, uvs );
	parse_faces( file, vertices );

	// Build Vertex Buffer
	Assert( vertexBufferData == nullptr );
	vertexCount = vertices.size();
	vertexBufferSize = vertexCount * sizeof( GfxBuiltInVertex );
	vertexBufferData = reinterpret_cast<GfxBuiltInVertex *>( memory_alloc( vertexBufferSize ) );
	for( usize i = 0; i < vertexCount; i++ )
	{
		Vertex &vertexRaw = vertices[i];
		GfxBuiltInVertex &vertex = vertexBufferData[i];

		// Position
		VertexPosition &position = positions[vertexRaw.position - 1];
		vertex.x = position.x;
		vertex.y = position.y;
		vertex.z = position.z;

		// UV
		VertexUV &uv = uvs[vertexRaw.uv - 1];
		vertex.u = static_cast<u16>( uv.u * 0xFFFF );
		vertex.v = static_cast<u16>( ( 1.0f - uv.v ) * 0xFFFF );

		// Color
		vertex.r = 255;
		vertex.g = 255;
		vertex.b = 255;
		vertex.a = 255;
	}

	// Success!
	return true;
}

bool MeshObj::free()
{
	if( vertexBufferData != nullptr ) { memory_free( vertexBufferData ); }
	vertexBufferData = nullptr;
	vertexCount = 0;
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////