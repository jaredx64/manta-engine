#include <build/assets.hpp>

#include <build/build.hpp>

#include <core/string.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Assets
{
	// Output Paths
	char pathHeader[PATH_SIZE];
	char pathSource[PATH_SIZE];

	// Output contents
	String source;
	String header;
	Buffer binary;

	// Cache
	usize assetFileCount = 0;
	FileTime timeCache;

	// Asset Types
	Textures textures;
	Glyphs glyphs;
	Sprites sprites;
	Materials materials;
	Fonts fonts;
	Sounds sounds;
	Songs songs;
	Meshes meshes;
}


void Assets::begin()
{
	// Paths
	strjoin( pathHeader, Build::pathOutput, SLASH "generated" SLASH "assets.generated.hpp" );
	strjoin( pathSource, Build::pathOutput, SLASH "generated" SLASH "assets.generated.cpp" );

	// Cache
	FileTime timeHeader;
	if( !file_time( pathHeader, &timeHeader ) ) { Build::cacheDirtyAssets = true; return; }
	FileTime timeSource;
	if( !file_time( pathSource, &timeSource ) ) { Build::cacheDirtyAssets = true; return; }

	timeCache = file_time_newer( timeHeader, timeSource ) ? timeHeader : timeSource;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////