#pragma once

#include <core/types.hpp>
#include <core/list.hpp>
#include <core/buffer.hpp>
#include <core/string.hpp>

#include <build/cache.hpp>
#include <build/gfx.hpp>

#include <build/assets/meshes.hpp>
#include <build/assets/skins.hpp>

#include <build/assets/textures.hpp>
#include <build/assets/materials.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Model
{
public:
	bool load_from_obj();
	bool load_from_cache();

public:
	List<MeshID> meshes;
	float x1, y1, z1;
	float x2, y2, z2;

	CacheKey cacheKey;
	SkinID skinID;
	String name;
	String path;
};

using ModelID = u32;
#define MODELID_MAX ( U32_MAX )
#define MODELID_NULL ( MODELID_MAX )

#define MODEL_MESH_COUNT_MAX ( 32 )
#define MODEL_MATERIAL_COUNT_MAX ( 32 )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Models
{
public:
	ModelID register_new( const Model &model = Model { } );
	ModelID register_new( Model &&model );
	ModelID register_new_from_definition( String name, const char *path );

	usize gather( const char *path, bool recurse = true );
	void build();

public:
	List<Model> models;
	Model &operator[]( u32 meshID ) { return models[meshID]; }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////