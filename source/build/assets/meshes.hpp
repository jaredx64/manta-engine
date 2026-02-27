#pragma once

#include <core/types.hpp>
#include <core/assets.hpp>
#include <core/list.hpp>
#include <core/buffer.hpp>
#include <core/string.hpp>

#include <build/cache.hpp>
#include <build/gfx.hpp>

#include <build/assets/materials.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Mesh
{
public:
	Assets::MeshFormatTypeVertex formatVertex;
	Buffer dataVertex;
	usize offsetVertex;
	usize sizeVertex;

	Assets::MeshFormatTypeIndex formatIndex;
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
		Assets::MeshFormatTypeVertex formatVertex, void *dataVertex, usize sizeVertex,
		Assets::MeshFormatTypeIndex formatIndex, void *dataIndex, usize sizeIndex,
		float x1, float y1, float z1, float x2, float y2, float z2,
		u32 skinSlotIndex );

	MeshID register_new_from_cache( CacheKey cacheKey );

	void build();

public:
	List<Mesh> meshes;
	Mesh &operator[]( u32 meshID ) { return meshes[meshID]; }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////