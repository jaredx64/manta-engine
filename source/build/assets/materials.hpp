#pragma once

#include <core/types.hpp>
#include <core/list.hpp>
#include <core/buffer.hpp>
#include <core/string.hpp>

#include <build/assets/textures.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Material
{
	CacheID cacheID;
	TextureID textureIDColor;
	TextureID textureIDNormal;
	String name;
};

using MaterialID = u32;
#define MATERIALID_MAX ( U32_MAX )
#define MATERIALID_NULL ( MATERIALID_MAX )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Materials
{
	MaterialID allocate_new( const Material &material );

	usize gather( const char *path, bool recurse = true );
	void process( const char *path );
	void build();

	Material &operator[]( u32 materialID ) { return materials[materialID]; }

	List<Material> materials;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////