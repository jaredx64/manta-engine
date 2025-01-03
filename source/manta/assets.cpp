#include <manta/assets.hpp>

#include <core/string.hpp>

#include <manta/filesystem.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Assets
{
	char binaryPath[PATH_SIZE];
	File binary;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SysAssets::init()
{
	// Binary Path
	strjoin( Assets::binaryPath, WORKING_DIRECTORY, SLASH BUILD_PROJECT, ".bin" );

	// Open Binary
	Assets::binary.open( Assets::binaryPath );
	ErrorReturnIf( !Assets::binary, false, "Assets: Failed to open binary file: %s", Assets::binaryPath );

	// Success
	return true;
}


bool SysAssets::free()
{
	// Close Binary File
	Assets::binary.close();

	// Success
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////