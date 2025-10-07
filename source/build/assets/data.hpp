#pragma once

#include <core/types.hpp>
#include <core/list.hpp>
#include <core/buffer.hpp>
#include <core/string.hpp>

#include <build/cache.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct DataAsset
{
	CacheID cacheID;
	String path;
	String name;
	Buffer data;
	usize offset;
	usize size;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct DataAssets
{
	void make_new( const DataAsset &asset );

	usize gather( const char *path, const bool recurse = true );
	void process( const char *path );
	void build();

	DataAsset &operator[]( const u32 dataAssetID ) { return dataAssets[dataAssetID]; }

	List<DataAsset> dataAssets;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////