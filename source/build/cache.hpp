#pragma once

#include <core/buffer.hpp>
#include <core/hashmap.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using CacheKey = u64;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Cache
{
public:
	struct Entry
	{
		usize size = 0;
		usize offset = 0;
		bool is_null() const { return size == 0 && offset == 0; }
		explicit operator bool() const { return !is_null(); }
	};

public:
	void read( const char *path );
	void write( const char *path );

	bool contains( CacheKey key )
	{
		if( entryTableReading.count() == 0 ) { return false; }
		return entryTableReading.contains( key );
	}

	template <typename T> bool fetch( CacheKey key, T &data )
	{
		if( entryTableReading.count() == 0 ) { return false; }
		Cache::Entry entry = entryTableReading.get( key );
		if( entry.is_null() ) { return false; }
		Assert( entry.offset + entry.size <= cacheBufferReading.size() );
		cacheBufferReading.seek_to( entry.offset );
		cacheBufferReading.read( data );
		return true;
	}

	template <typename T> void store( CacheKey key, T &data )
	{
#if 1
		ErrorIf( entryTableWriting.contains( key ), "CacheKey conflict: %u", key );
#endif
		Cache::Entry entry;
		entry.offset = cacheBufferWriting.write( &data, sizeof( T ) );
		entry.size = sizeof( T );
		entryTableWriting.set( key, entry );
	}

public:
	HashMap<CacheKey, Entry> entryTableReading { };
	HashMap<CacheKey, Entry> entryTableWriting { };
	Buffer cacheBufferReading { };
	Buffer cacheBufferWriting { };
	bool dirty = false;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////