#pragma once

#include <core/types.hpp>
#include <core/list.hpp>
#include <core/buffer.hpp>
#include <core/string.hpp>

#include <build/cache.hpp>
#include <build/gfx.hpp>

#include <build/assets/meshes.hpp>
#include <build/assets/textures.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define MODEL_MESH_COUNT_MAX ( 32 )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum_type( ModelFileType, int )
{
	ModelFileType_BINARY = 0,
	ModelFileType_OBJ = 1,
	// ...
	MODELFILETYPE_COUNT,
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Model
{
	String path;
	String name;

	List<MeshID> meshes;
	List<TextureID> textures;
	float x1, y1, z1;
	float x2, y2, z2;
};

using ModelID = u32;
#define MODELID_MAX ( U32_MAX )
#define MODELID_NULL ( MODELID_MAX )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Models
{
	ModelID allocate_new( const Model &model );
	ModelID allocate_new( Model &&model );

	usize gather( const char *path, bool recurse = true );
	void process( const char *path, ModelFileType type );

	void build();

	bool load_from_cache( CacheID cacheID );
	bool load_binary( const class AssetFile &file, CacheID cacheID );
	bool load_obj( const class AssetFile &file, CacheID cacheID );

	Model &operator[]( u32 meshID ) { return models[meshID]; }

	List<Model> models;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////