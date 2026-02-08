#pragma once

#include <core/types.hpp>
#include <core/list.hpp>
#include <core/buffer.hpp>
#include <core/string.hpp>

#include <build/cache.hpp>
#include <build/gfx.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// NOTE: Must maintain parity with manta/assets.hpp
enum_type( MeshFormatTypeVertex, int )
{
	MeshFormatTypeVertex_Default = 0,
	// ...
	MESHFORMATTYPEVERTEX_COUNT
};

// NOTE: Must maintain parity with manta/assets.hpp
enum_type( MeshFormatTypeIndex, int )
{
	MeshFormatTypeIndex_U32 = 0,
	MeshFormatTypeIndex_U16,
	// ...
	MESHFORMATTYPEINDEX_COUNT
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Mesh
{
	CacheID cacheID;

	String name;

	MeshFormatTypeVertex formatVertex;
	Buffer dataVertex;
	usize offsetVertex;
	usize sizeVertex;

	MeshFormatTypeIndex formatIndex;
	Buffer dataIndex;
	usize offsetIndex;
	usize sizeIndex;

	float x1, y1, z1;
	float x2, y2, z2;
};

using MeshID = u32;
#define MESHID_MAX ( U32_MAX )
#define MESHID_NULL ( MESHID_MAX )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Meshes
{
	MeshID allocate_new( const String &name, CacheID cacheID,
		MeshFormatTypeVertex formatVertex, void *dataVertex, usize sizeVertex,
		MeshFormatTypeIndex formatIndex, void *dataIndex, usize sizeIndex,
		float x1, float y1, float z1, float x2, float y2, float z2 );

	MeshID retrieve_from_cache( const String &name, CacheID cacheID );

	void build();

	Mesh &operator[]( u32 meshID ) { return meshes[meshID]; }

	List<Mesh> meshes;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////