#pragma once

#include <core/types.hpp>
#include <core/list.hpp>
#include <core/buffer.hpp>
#include <core/string.hpp>

#include <build/cache.hpp>
#include <build/gfx.hpp>

#include <build/assets/materials.hpp>

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

class Mesh
{
public:
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

	u32 skinSlotIndex;

	CacheKey cacheKey;
};

using MeshID = u32;
#define MESHID_MAX ( U32_MAX )
#define MESHID_NULL ( MESHID_MAX )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Meshes
{
public:
	MeshID register_new( CacheKey cacheKey,
		MeshFormatTypeVertex formatVertex, void *dataVertex, usize sizeVertex,
		MeshFormatTypeIndex formatIndex, void *dataIndex, usize sizeIndex,
		float x1, float y1, float z1, float x2, float y2, float z2,
		u32 skinSlotIndex );

	MeshID register_new_from_cache( CacheKey cacheKey );

	void build();

public:
	List<Mesh> meshes;
	Mesh &operator[]( u32 meshID ) { return meshes[meshID]; }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////