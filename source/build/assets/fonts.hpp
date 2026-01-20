#pragma once

#include <core/types.hpp>
#include <core/list.hpp>
#include <core/buffer.hpp>
#include <core/string.hpp>

#include <build/cache.hpp>
#include <build/assets/textures.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct TTF
{
	CacheID cacheID;
	String path;
	Buffer data;
	usize offset;
	usize size;
};


struct Font
{
	String name;
	u16 ttfDefault;
	u16 ttfItalic;
	u16 ttfBold;
	u16 ttfBoldItalic;
	String pathLicense;
	Buffer dataLicense;
};


struct Fonts
{
	usize gather( const char *path, bool recurse = true );
	void process( const char *path );
	void build();

	Font &operator[]( u32 fontID ) { return fonts[fontID]; }

	List<TTF> ttfs;
	List<Font> fonts;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////