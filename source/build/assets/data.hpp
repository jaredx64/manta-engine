#pragma once

#include <core/types.hpp>
#include <core/list.hpp>
#include <core/buffer.hpp>
#include <core/string.hpp>

#include <build/cache.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class DataAsset
{
public:
	Buffer data;
	usize offset;
	usize size;

	CacheKey cacheKey;
	String name;
	String path;
};

using DataAssetID = u32;
#define DATAASSETID_MAX ( U32_MAX )
#define DATAASSETID_NULL ( DATAASSETID_MAX )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class DataAssets
{
public:
	DataAssetID register_new( const DataAsset &asset = DataAsset { } );
	DataAssetID register_new( DataAsset &&asset );
	DataAssetID register_new_from_definition( String name, const char *path );

	usize gather( const char *path, bool recurse = true );
	void build();

public:
	List<DataAsset> dataAssets;
	DataAsset &operator[]( u32 dataAssetID ) { return dataAssets[dataAssetID]; }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////