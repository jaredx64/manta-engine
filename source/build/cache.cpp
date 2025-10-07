#include <build/cache.hpp>

#include <build/filesystem.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Cache::read( const char *path )
{
	Buffer cacheFile;
	if( cacheFile.load( path ) )
	{
		usize entryCount = cacheFile.read<usize>();
		for( usize i = 0; i < entryCount; i++ )
		{
			const CacheID id = cacheFile.read<CacheID>();
			const Cache::Entry entry = cacheFile.read<Cache::Entry>();
			entryTableReading.set( id, entry );
		}
		usize cacheBufferSize = cacheFile.read<usize>();
		cacheBufferReading.write( cacheFile.read_bytes( cacheBufferSize ), cacheBufferSize );
		return;
	}

	dirty = true;
}


void Cache::write( const char *path )
{
	file_delete( path );

	Buffer assetCacheBuffer;
	assetCacheBuffer.write<usize>( entryTableWriting.count() );
	for( auto &entry : entryTableWriting )
	{
		assetCacheBuffer.write<CacheID>( entry.key );
		assetCacheBuffer.write<Cache::Entry>( entry.value );
	}
	assetCacheBuffer.write<usize>( cacheBufferWriting.size() );
	assetCacheBuffer.write( cacheBufferWriting.data, cacheBufferWriting.size() );
	assetCacheBuffer.save( path );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////