#pragma once

#include <core/types.hpp>
#include <core/list.hpp>
#include <core/hashmap.hpp>
#include <core/buffer.hpp>
#include <core/string.hpp>

#include <build/cache.hpp>
#include <build/assets/materials.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Skin
{
public:
	u32 add_material( u32 materialKey, MaterialID materialID );
	bool contains_material( u32 materialKey ) const;
	u32 get_material_slot( u32 materialKey );

	bool load_from_mtl( const char *path );

public:
	List<MaterialID> materials;
	HashMap<u32, u32> materialKeyToSkinSlotIndex;

	String name;
	String path;
};

using SkinID = u32;
#define SKINID_MAX ( U32_MAX )
#define SKINID_NULL ( SKINID_MAX )

#define SKIN_SLOT_COUNT_MAX ( 32 )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Skins
{
public:
	SkinID register_new( const Skin &skin = Skin { } );
	SkinID register_new( Skin &&skin );
	SkinID register_new_from_file( String &name, const char *path );

	void build();

public:
	List<Skin> skins;
	Skin &operator[]( u32 skinID ) { return skins[skinID]; }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////