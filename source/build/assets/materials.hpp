#pragma once

#include <core/types.hpp>
#include <core/list.hpp>
#include <core/buffer.hpp>
#include <core/string.hpp>

#include <build/assets/textures.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// NOTE: Must maintain parity with manta/assets.hpp
enum_type( MaterialTextureSlot, int )
{
	MaterialTextureSlot_Diffuse = 0,
	MaterialTextureSlot_Normal,
	MaterialTextureSlot_Specular,
	// ...
	MATERIALTEXTURESLOT_COUNT
};


constexpr const char *MaterialTextureSlotNames[MATERIALTEXTURESLOT_COUNT] =
{
	"Diffuse", // MaterialTextureSlot_Diffuse
	"Normal", // MaterialTextureSlot_Normal
	"Specular", // MaterialTextureSlot_Specular
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Material
{
public:
	void allocate_texture_from_file( MaterialTextureSlot slot, const char *textureName,
		const char *texturePath, bool textureGenerateMips );

public:
	CacheKey cacheKey;
	TextureID textures[MATERIALTEXTURESLOT_COUNT] = { };
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