#pragma once

#include <boot/config.hpp>
#include <boot/string.hpp>

#include <vendor/string.hpp>
#include <vendor/stdarg.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static char argumentOptions[256];

template <typename... Args>
void parse_argument( int argc, char **argv, const char *argument, const char *&variable,
	bool required, const char *defaultValue, Args... options )
{
	// Search for argument
	bool found = false;
	for( int i = 1; i < argc; i++ )
	{
		const char *arg = argv[i];
		if( strncmp( arg, argument, strlen( argument ) ) == 0 )
		{
			variable = &arg[strlen( argument )];
			ErrorIf( strlen( variable ) == 0, "BOOT: Empty commandline argument '%s'", argument );
			found = true;
			break;
		}
	}

	// Didn't find the argument
	if( !found )
	{
		ErrorIf( required, "BOOT: Did not supply required argument '%s'", argument );
		variable = defaultValue;
	}

	// Verify that the argument is one of the allowed options
	const char *allowed[] = { defaultValue, options..., nullptr };
	if( strlen( allowed[0] ) == 0 ) { return; }

	const char *const *current = allowed;
	while( *current != nullptr )
	{
		if( strcmp( variable, *current ) == 0 ) { return; } // Success!
		strappend( argumentOptions, *current );
		++current;
		if( *current != nullptr ) { strappend( argumentOptions, ", " ); }
	}

	Error( "BOOT: Invalid value for argument '%s%s' (options: %s)", argument, variable, argumentOptions );
}

#define ARG_REQUIRED ( true )
#define ARG_OPTIONAL ( false )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Arguments
{
	const char *verbose;
	const char *project;
	const char *config;
	const char *os;
	const char *architecture;
	const char *toolchain;
	const char *gfx;

	void parse( int argc, char **argv )
	{
		// Verbose
		parse_argument( argc, argv, "-verbose=", verbose, ARG_OPTIONAL, "1", "0" );

		// Project
		parse_argument( argc, argv, "-project=", project, ARG_REQUIRED, "" );

		// Config
		parse_argument( argc, argv, "-config=", config, ARG_OPTIONAL, "" );
		if( config[0] == '\0' ) { config = "release"; }

		// Operating System
		#if PIPELINE_OS_WINDOWS
			parse_argument( argc, argv, "-os=", os, ARG_OPTIONAL, "windows" );
		#elif PIPELINE_OS_LINUX
			parse_argument( argc, argv, "-os=", os, ARG_OPTIONAL, "linux" );
		#elif PIPELINE_OS_MACOS
			parse_argument( argc, argv, "-os=", os, ARG_OPTIONAL, "macOS" );
		#endif

		// Architecture
		#if PIPELINE_OS_WINDOWS
			parse_argument( argc, argv, "-architecture=", architecture, ARG_OPTIONAL, "x64" );
		#elif PIPELINE_OS_LINUX
			parse_argument( argc, argv, "-architecture=", architecture, ARG_OPTIONAL, "x64" );
		#elif PIPELINE_OS_MACOS
			parse_argument( argc, argv, "-architecture=", architecture, ARG_OPTIONAL, "arm64", "x64" );
		#endif

		// Toolchain
		#if PIPELINE_OS_WINDOWS
			parse_argument( argc, argv, "-toolchain=", toolchain, ARG_OPTIONAL, "msvc", "llvm", "gnu" );
		#elif PIPELINE_OS_LINUX
			parse_argument( argc, argv, "-toolchain=", toolchain, ARG_OPTIONAL, "llvm", "gnu" );
		#elif PIPELINE_OS_MACOS
			parse_argument( argc, argv, "-toolchain=", toolchain, ARG_OPTIONAL, "llvm" );
		#endif

		// Graphics
		#if PIPELINE_OS_WINDOWS
			parse_argument( argc, argv, "-gfx=", gfx, ARG_OPTIONAL, "d3d11", "d3d12", "opengl", "vulkan", "none" );
		#elif PIPELINE_OS_LINUX
			parse_argument( argc, argv, "-gfx=", gfx, ARG_OPTIONAL, "opengl", "vulkan", "none" );
		#elif PIPELINE_OS_MACOS
			parse_argument( argc, argv, "-gfx=", gfx, ARG_OPTIONAL, "opengl", "metal", "none" );
		#endif
	}

	bool verbose_output()
	{
		return strcmp( verbose, "1" ) == 0;
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct PipelineCache
{
	char project[128];
	char os[128];
	char architecture[128];
	char toolchain[128];
	char config[128];
	char gfx[128];

	static bool load( Arguments &args, const char *path )
	{
		bool dirty = false;
		File cacheFile { path };

		if( cacheFile && cacheFile.size == sizeof( PipelineCache ) )
		{
			PipelineCache &cache = *reinterpret_cast<PipelineCache *>( cacheFile.data );
			dirty |= ( strcmp( args.os, cache.os ) != 0 );
			dirty |= ( strcmp( args.toolchain, cache.toolchain ) != 0 );
			dirty |= ( strcmp( args.gfx, cache.gfx ) != 0 );
			dirty |= ( strcmp( args.architecture, cache.architecture ) != 0 );
			dirty |= ( strcmp( args.config, cache.config ) != 0 );
			dirty |= ( strcmp( args.project, cache.project ) != 0 );
		}
		else
		{
			dirty = true;  // No cache file
		}

		cacheFile.close();
		return dirty;
	}

	static void save( Arguments &args, const char *path )
	{
		PipelineCache cache;
		strjoin( cache.project, args.project );
		strjoin( cache.os, args.os );
		strjoin( cache.architecture, args.architecture );
		strjoin( cache.toolchain, args.toolchain );
		strjoin( cache.config, args.config );
		strjoin( cache.gfx, args.gfx );

		FILE *file = fopen( path, "wb" );
		ErrorIf( file == nullptr, "Failed to open file '%s' for writing", path );
		fwrite( &cache, 1, sizeof( cache ), file );
		ErrorIf( fclose( file ) != 0, "Failed to close file '%s'", path );
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////