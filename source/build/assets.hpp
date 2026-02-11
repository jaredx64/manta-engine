#pragma once

#include <core/buffer.hpp>
#include <core/string.hpp>
#include <core/hashmap.hpp>

#include <build/cache.hpp>
#include <build/system.hpp>

#include <build/assets/data.hpp>
#include <build/assets/textures.hpp>
#include <build/assets/glyphs.hpp>
#include <build/assets/sprites.hpp>
#include <build/assets/materials.hpp>
#include <build/assets/models.hpp>
#include <build/assets/meshes.hpp>
#include <build/assets/skins.hpp>
#include <build/assets/fonts.hpp>
#include <build/assets/sounds.hpp>
#include <build/assets/skeleton2d.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct CacheAssetsBinary
{
	usize offset;
	usize size;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Assets
{
	// Stages
	extern void begin();
	extern void end();
	extern void codegen();

	// Output Paths
	extern char pathHeader[PATH_SIZE];
	extern char pathSource[PATH_SIZE];

	// Output contents
	extern String source;
	extern String header;
	extern Buffer binary;

	// Asset Types
	extern DataAssets dataAssets;
	extern Textures textures;
	extern Glyphs glyphs;
	extern Sprites sprites;
	extern Materials materials;
	extern Models models;
	extern Meshes meshes;
	extern Skins skins;
	extern Fonts fonts;
	extern Sounds sounds;
	extern Skeleton2Ds skeleton2Ds;

	// Cache
	extern Cache cache;
	extern usize cacheReadOffset;
	extern usize cacheFileCount;
	extern void cache_read( const char *path );
	extern void cache_write( const char *path );
	extern void cache_validate();

	// Logging
	extern usize assetsBuilt;
	extern usize assetsCached;
	extern void log_asset_cache( const char *type, const char *name );
	extern void log_asset_build( const char *type, const char *name );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class AssetFile
{
public:
	explicit operator bool() const { return exists; }
	CacheKey cache_id() const;

public:
	FileTime time = FileTime { };
	char path[PATH_SIZE] = { 0 };
	char name[PATH_SIZE] = { 0 };
	bool exists = false;
};

extern bool asset_file_register( AssetFile &asset, const char *path );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename... Args> void assets_struct( String &string, const char *name, Args... args )
{
	string.append( "struct " );
	string.append( name );
	string.append( "\n{\n" );
	( string.append( "\t" ).append( args ).append( "\n" ), ... );
	string.append( "};\n\n" );
}


inline void assets_group( String &string )
{
	string.append( COMMENT_BREAK "\n\n" );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////