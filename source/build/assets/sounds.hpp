#pragma once

#include <core/types.hpp>
#include <core/list.hpp>
#include <core/buffer.hpp>
#include <core/string.hpp>

#include <build/objloader.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Sound
{
	String path;
	String name;

	byte *sampleData;
	usize sampleDataSize;
	usize sampleOffsetBytes;
	usize sampleCountBytes;
	u8 numChannels;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Sounds
{
	void make_new( const Sound &sound );
	void gather( const char *path, const bool recurse = true );
	void load( const char *path );
	void write();

	Sound &operator[]( const u32 soundID ) { return sounds[soundID]; }

	List<Sound> sounds;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////