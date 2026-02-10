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
	CacheKey cacheKey;
	String path;
	Buffer data;
	usize offset;
	usize size;
};

using TTFID = u32;
#define TTFID_MAX ( U32_MAX )
#define TTFID_NULL ( TTFID_MAX )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Font
{
public:
	u16 ttfDefault;
	u16 ttfItalic;
	u16 ttfBold;
	u16 ttfBoldItalic;
	String pathLicense;
	Buffer dataLicense;

	String name;
};

using FontID = u32;
#define FONTID_MAX ( U32_MAX )
#define FONTID_NULL ( FONTID_MAX )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Fonts
{
public:
	usize gather( const char *path, bool recurse = true );
	void process( const char *path );
	void build();

public:
	List<TTF> ttfs;
	List<Font> fonts;
	Font &operator[]( u32 fontID ) { return fonts[fontID]; }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////