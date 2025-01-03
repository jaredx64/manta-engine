#pragma once

#include <core/types.hpp>
#include <core/list.hpp>
#include <core/buffer.hpp>
#include <core/string.hpp>

#include <build/assets/textures.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct TTF
{
	String path;
	Buffer buffer;
	usize offset;
	usize size;
};


struct Font
{
	String name;
	String path;
	String licensePath;
	Buffer licenseData;

	u16 ttfDefault;
	u16 ttfItalic;
	u16 ttfBold;
	u16 ttfBoldItalic;
};


struct Fonts
{
	void gather( const char *path, const bool recurse = true );
	void load( const char *path );
	void write();

	u16 load_ttf( const char *pathFont, const char *pathTTF );

	Font &operator[]( const u32 fontID ) { return fonts[fontID]; }

	List<TTF> ttfs;
	List<Font> fonts;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////