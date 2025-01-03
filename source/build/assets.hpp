#pragma once

#include <core/buffer.hpp>
#include <core/string.hpp>

#include <build/filesystem.hpp>

#include <build/assets/textures.hpp>
#include <build/assets/glyphs.hpp>
#include <build/assets/sprites.hpp>
#include <build/assets/materials.hpp>
#include <build/assets/fonts.hpp>
#include <build/assets/sounds.hpp>
#include <build/assets/songs.hpp>
#include <build/assets/meshes.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Assets
{
	// Output Paths
	extern char pathHeader[PATH_SIZE];
	extern char pathSource[PATH_SIZE];

	// Output contents
	extern String source;
	extern String header;
	extern Buffer binary;

	// Cache
	extern usize assetFileCount;
	extern FileTime timeCache;

	// Asset Types
	extern Textures textures;
	extern Glyphs glyphs;
	extern Sprites sprites;
	extern Materials materials;
	extern Fonts fonts;
	extern Sounds sounds;
	extern Songs songs;
	extern Meshes meshes;

	// Setup
	extern void begin();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename... Args>
void assets_struct( String &string, const char *name, Args... args )
{
	string.append( "struct " );
	string.append( name );
	string.append( "\n{\n" );
	( string.append( "\t" ).append( args ).append( "\n" ), ... );
	string.append( "};\n\n" );
}


inline void assets_group( String &string )
{
	string.append( COMMENT_BREAK "\n\n" );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////