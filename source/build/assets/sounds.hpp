#pragma once

#include <core/types.hpp>
#include <core/list.hpp>
#include <core/buffer.hpp>
#include <core/string.hpp>

#include <build/cache.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Sound
{
public:
	bool streamed;
	bool compressed;
	u8 numChannels;
	Buffer sampleData;
	usize sampleOffsetBytes;
	usize sampleCountBytes;

	CacheKey cacheKey;
	String name;
	String path;
};

using SoundID = u32;
#define SOUNDID_MAX ( U32_MAX )
#define SOUNDID_NULL ( SOUNDID_MAX )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Sounds
{
public:
	SoundID register_new( const Sound &sound = Sound { } );
	SoundID register_new( Sound &&sound );
	SoundID register_new_from_definition( String name, const char *path );

	usize gather( const char *path, bool recurse = true );
	void build();

public:
	List<Sound> sounds;
	Sound &operator[]( u32 soundID ) { return sounds[soundID]; }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////