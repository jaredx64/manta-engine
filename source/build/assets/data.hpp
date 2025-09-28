#pragma once

#include <core/types.hpp>
#include <core/list.hpp>
#include <core/buffer.hpp>
#include <core/string.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct DataAsset
{
	DataAsset( String name ) : name( name ) { }

	String name;
	usize offset;
	usize size;

	String filePath;
	Buffer fileData;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct DataAssets
{
	void make_new( const DataAsset &asset );
	void gather( const char *path, const bool recurse = true );
	void load( const char *path );
	void write();

	DataAsset &operator[]( const u32 dataAssetID ) { return dataAssets[dataAssetID]; }

	List<DataAsset> dataAssets;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////