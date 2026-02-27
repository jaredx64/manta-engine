#pragma once

#include <core/types.hpp>
#include <core/assets.hpp>
#include <core/list.hpp>
#include <core/buffer.hpp>
#include <core/string.hpp>

#include <build/assets/textures.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

constexpr const char *MaterialTextureSlotNames[Assets::MATERIALTEXTURESLOT_COUNT] =
{
	"Color", // MaterialTextureSlot_Color
	"Normal", // MaterialTextureSlot_Normal
	"Roughness", // MaterialTextureSlot_Roughness
	"Metallic", // MaterialTextureSlot_Metallic
	"Emissive", // MaterialTextureSlot_Emissive
	"Specular", // MaterialTextureSlot_Specular
	"AO", // MaterialTextureSlot_AmbientOcclusion
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Material
{
public:
	void allocate_texture_from_file( Assets::MaterialTextureSlot slot,
		const char *textureName, const char *texturePath, bool textureGenerateMips );

public:
	CacheKey cacheKey;
	TextureID textures[Assets::MATERIALTEXTURESLOT_COUNT] = { };
	String name;
};

using MaterialID = u32;
#define MATERIALID_MAX ( U32_MAX )
#define MATERIALID_NULL ( MATERIALID_MAX )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Materials
{
public:
	MaterialID register_new( const Material &material = Material { } );
	MaterialID register_new( Material &&material );
	MaterialID register_new_from_definition( String name, const char *path );

	usize gather( const char *path, bool recurse = true );
	void build();

public:
	List<Material> materials;
	Material &operator[]( u32 materialID ) { return materials[materialID]; }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////