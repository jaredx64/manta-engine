#pragma once

#include <core/debug.hpp>
#include <vendor/string.hpp>
#include <vendor/stdarg.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static char argumentOptions[256];

template <typename... Args>
void parse_argument( int argc, char **argv, const char *argument, const char *&variable,
	const bool required, const char *defaultValue, Args... options )
{
	// Search for argument
	bool found = false;
	for( int i = 1; i < argc; i++ )
	{
		const char *arg = argv[i];
		if( strncmp( arg, argument, strlen( argument ) ) == 0 )
		{
			variable = &arg[strlen( argument )];
			ErrorIf( strlen( variable ) == 0, "Build: empty commandline argument '%s'", argument );
			found = true;
			break;
		}
	}

	// Didn't find the argument
	if( !found )
	{
		ErrorIf( required, "Build: did not supply required argument '%s'", argument );
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

	Error( "Build: invalid value for argument '%s%s' (options: %s)", argument, variable, argumentOptions );
}

#define ARG_REQUIRED ( true )
#define ARG_OPTIONAL ( false )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Arguments
{
	const char *project;
	const char *config;
	const char *verbose;
	const char *clean;
	const char *codegen;
	const char *build;
	const char *package;
	const char *run;
	const char *os;
	const char *architecture;
	const char *toolchain;
	const char *gfx;
	const char *multilaunch;

	void parse( int argc, char **argv )
	{
		// Project
		parse_argument( argc, argv, "-project=", project, ARG_REQUIRED, "" );

		// Config
		parse_argument( argc, argv, "-config=", config, ARG_OPTIONAL, "" );

		// Verbose
		parse_argument( argc, argv, "-verbose=", verbose, ARG_OPTIONAL, "1", "0" );

		// Clean
		parse_argument( argc, argv, "-clean=", clean, ARG_OPTIONAL, "0", "1" );

		// Code Generation
		parse_argument( argc, argv, "-codegen=", codegen, ARG_OPTIONAL, "1", "0" );

		// Build
		parse_argument( argc, argv, "-build=", build, ARG_OPTIONAL, "1", "0" );

		// Package
		parse_argument( argc, argv, "-package=", package, ARG_OPTIONAL, "1", "0" );

		// Run
		parse_argument( argc, argv, "-run=", run, ARG_OPTIONAL, "1", "0", "2" );

		// Multi-Launch
		parse_argument( argc, argv, "-multilaunch=", multilaunch, ARG_OPTIONAL, "1",
			"1", "2", "3", "4", "5", "6", "7", "8" );

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
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////