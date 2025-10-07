#pragma once

#include <core/types.hpp>
#include <core/list.hpp>
#include <core/buffer.hpp>
#include <core/string.hpp>

#include <build/assets/textures.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Material
{
	TextureID textureIDColor;
	TextureID textureIDNormal;
	String name;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Materials
{
	void make_new( const Material &material );

	usize gather( const char *path, const bool recurse = true );
	void process( const char *path );
	void build();

	Material &operator[]( const u32 materialID ) { return materials[materialID]; }

	List<Material> materials;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////