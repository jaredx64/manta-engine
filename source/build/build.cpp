#include <build/build.hpp>

#include <core/list.hpp>
#include <core/hashmap.hpp>
#include <core/buffer.hpp>
#include <core/string.hpp>
#include <core/json.hpp>

#include <build/toolchains.hpp>
#include <build/assets.hpp>
#include <build/gfx.hpp>
#include <build/filesystem.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Build
{
	// Paths
	char pathEngine[PATH_SIZE];
	char pathProject[PATH_SIZE];
	char pathPackages[PATH_SIZE];
	char pathOutput[PATH_SIZE];
	char pathOutputBoot[PATH_SIZE];
	char pathOutputBuild[PATH_SIZE];
	char pathOutputGenerated[PATH_SIZE];
	char pathOutputGeneratedShaders[PATH_SIZE];
	char pathOutputRuntime[PATH_SIZE];
	char pathOutputRuntimeLicenses[PATH_SIZE];
	char pathOutputRuntimeDistributables[PATH_SIZE];
	char pathOutputRuntimeExecutable[PATH_SIZE];
	char pathOutputRuntimeBinary[PATH_SIZE];
	char pathOutputBuildCache[PATH_SIZE];

	// Package
	String packageName;
	String packageCompany;
	String packageVersionA;
	String packageVersionB;
	String packageVersionC;
	String packageVersionD;
	String packageIcon;
	Source packageRC { "", "" };

	// Commands
	char commandNinja[1024];
	char commandRun[1024];

	// Pipeline
	Arguments args;
	Toolchain tc;

	// Timer
	Timer timer;

	// Cache
	bool cacheDirty = false;
	bool cacheDirtyObjects = false;
	bool cacheDirtyAssets = false;
	bool cacheDirtyBinary = false;
	bool cacheDirtyShaders = false;
	Buffer cacheBufferPrevious;
	Buffer cacheBufferCurrent;

	// Compile
	List<Source> sources;
	List<String> libraries;
	List<String> includeDirectories;
#if PIPELINE_OS_WINDOWS
	List<Source> rcs;
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Build::compile_add_source( const char *pathSrc, const char *pathObj, const char *extensionObj )
{
	static char buffer[PATH_SIZE];
	path_change_extension( buffer, sizeof( buffer ), pathObj, extensionObj );
	Build::sources.add( { pathSrc, buffer } );
}


u32 Build::compile_add_sources( const char *directory, const bool recurse,
	const char *extensionSrc, const char *extensionObj )
{
	// Gather sources files
	Timer timer;
	List<FileInfo> sourceFiles;
	directory_iterate( sourceFiles, directory, extensionSrc, recurse ); // C++

	// Add sources
	static char pathObj[PATH_SIZE];
	for( FileInfo &sourceFile : sourceFiles )
	{
		strjoin( pathObj, "objects" SLASH, sourceFile.path );
		Build::compile_add_source( sourceFile.path, pathObj, extensionObj );
	}

	// Logging
	const u32 sourcesCount = sourceFiles.size();
	if( verbose_output() )
	{
		PrintColor( LOG_CYAN, TAB TAB "%u source%s found in: %s",
			sourcesCount, sourcesCount == 1 ? "" : "s", directory );
		PrintLnColor( LOG_WHITE, " (%.3f ms)", timer.elapsed_ms() );
	}
	return sourcesCount;
}


void Build::compile_add_library( const char *library )
{
	Build::libraries.add( library );
}


void Build::compile_add_include_directory( const char *includePath )
{
	Build::includeDirectories.add( includePath );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BuilderCore::build( int argc, char **argv )
{
	// Setup
	{
		// Timer
		Time::init();
		Build::timer.start();

		// Parse arguments
		parse_arguments( argc, argv );

		// Log
		PrintColor( LOG_YELLOW, "\n>" );
		const u32 exeArgsCount = verbose_output() ? argc : 1;
		for( u32 i = 0; i < exeArgsCount; i++ ) { PrintColor( LOG_YELLOW, " %s", argv[i] ); }
		Print( "\n" );

		// Paths
		strjoin( Build::pathEngine, "source" );
		strjoin( Build::pathProject, "projects" SLASH, Build::args.project );
		strjoin( Build::pathPackages, Build::pathProject, SLASH "packages" );
		strjoin( Build::pathOutput, Build::pathProject, SLASH "output" );
		strjoin( Build::pathOutputBoot, Build::pathOutput, SLASH, "boot" );
		strjoin( Build::pathOutputBuild, Build::pathOutput, SLASH, "build" );
		strjoin( Build::pathOutputGenerated, Build::pathOutput, SLASH, "generated" );
		strjoin( Build::pathOutputGeneratedShaders, Build::pathOutputGenerated, SLASH, "shaders" );
		strjoin( Build::pathOutputRuntime, Build::pathOutput, SLASH, "runtime" );
		strjoin( Build::pathOutputRuntimeLicenses, Build::pathOutputRuntime, SLASH, "licenses" );
		strjoin( Build::pathOutputRuntimeDistributables, Build::pathOutputRuntime, SLASH, "distributables" );
		strjoin( Build::pathOutputRuntimeExecutable, Build::pathOutputRuntime, SLASH,
			Build::args.project, Build::tc.linkerExtensionExe );
		strjoin( Build::pathOutputRuntimeBinary, Build::pathOutputRuntime, SLASH,
			Build::args.project, ".bin" );
		strjoin( Build::pathOutputBuildCache, Build::pathOutputBuild, SLASH "build.cache" );

		// Load Package
		package_load();

		// Output Directories
		directory_create( Build::pathPackages );
		directory_create( Build::pathOutputRuntime );
		directory_create( Build::pathOutputRuntimeLicenses );
		directory_create( Build::pathOutputRuntimeDistributables );
		directory_create( Build::pathOutputGenerated );
		directory_create( Build::pathOutputGeneratedShaders );
	}

	// Build Conditions
	bool codegen = ( strcmp( Build::args.codegen, "1" ) == 0 );
	bool build = ( strcmp( Build::args.build, "1" ) == 0 );
	bool package = ( strcmp( Build::args.package, "1" ) == 0 );
	bool run = ( strcmp( Build::args.run, "1" ) == 0 );

	// Gather
	if( codegen )
	{
		PrintLnColor( LOG_WHITE, "\nGather Assets" );
		Timer timer;

		Objects::begin();
		Gfx::begin();
		Assets::begin();

		objects_gather();
		shaders_gather();
		assets_gather();

		PrintLnColor( LOG_WHITE, TAB "Finished (%.3f ms)", timer.elapsed_ms() );
	}

	// Cache
	if( codegen )
	{
		PrintLnColor( LOG_WHITE, "\nCheck Cache" );
		Timer timer;

		// TODO: A dirty cache currently forces everything to be rebuilt

		build_cache();
		objects_cache();
		shaders_cache();
		assets_cache();
		binary_cache();

		PrintLnColor( LOG_WHITE, TAB "Finished (%.3f ms)", timer.elapsed_ms() );
	}

	// Build
	if( codegen )
	{
		PrintLnColor( LOG_WHITE, "\nBuild Assets" );
		Timer timer;

		objects_parse();
		objects_write();

		shaders_build();
		shaders_write();

		assets_build();
		assets_write();

		PrintLnColor( LOG_WHITE, TAB "Finished (%.3f ms)", timer.elapsed_ms() );
	}

	// Write Binary
	if( build )
	{
		PrintLnColor( LOG_WHITE, "\nWrite Binary" );
		Timer timer;

		binary_write();

		PrintLnColor( LOG_WHITE, TAB "Finished (%.3f ms)", timer.elapsed_ms() );
	}

	// Compile Executable
	if( build )
	{
		PrintLnColor( LOG_WHITE, "\nCompile Code" );
		Timer timer;

		compile_project();
		compile_engine();
		compile_write_ninja();
		compile_run_ninja();

		PrintLnColor( LOG_WHITE, "\n" TAB "Finished: %.3f s (%.3f ms)", timer.elapsed_s(), timer.elapsed_ms() );
	}

	// Finish
	{
		PrintColor( LOG_GREEN, "\nBuild Finished!" );
		PrintLnColor( LOG_WHITE, " (%.3f s)", Build::timer.elapsed_s() );
		Build::cacheBufferCurrent.save( Build::pathOutputBuildCache );
	}

	// Package
	if( build && package )
	{
		PrintLnColor( LOG_WHITE, "\nCreate Package" );
		Timer timer;

		if( strcmp( Build::args.os, "windows" ) == 0 )
		{
			ErrorIf( !package_windows(), "Create Package: package_windows failed" );
		}
		else if( strcmp( Build::args.os, "linux" ) == 0 )
		{
			ErrorIf( !package_linux(), "Create Package: package_linux failed" );
		}
		else if( strcmp( Build::args.os, "macOS" ) == 0 )
		{
			ErrorIf( !package_macos(), "Create Package: package_macos failed" );
		}
		else
		{
			Error( "Create Package: Unsupported OS: %s", Build::args.os );
		}

		PrintLnColor( LOG_WHITE, TAB "Finished: %.3f s (%.3f ms)", timer.elapsed_s(), timer.elapsed_ms() );
	}

	// Run Executable
	if( run )
	{
		executable_run( argc, argv );
	}
}


void BuilderCore::parse_arguments( int argc, char **argv )
{
	// Parse Arguments
	Build::args.parse( argc, argv );
	Build::tc.detect( Build::args );
}


void BuilderCore::build_cache()
{
	Build::cacheDirty  = false;
	Build::cacheDirty |= ( strcmp( Build::args.clean, "1" ) == 0 );
	Build::cacheDirty |= ( !Build::cacheBufferPrevious.load( Build::pathOutputBuildCache, true ) );

	PrintColor( LOG_WHITE, TAB "Build Cache... " );
	PrintLnColor( Build::cacheDirty ? LOG_RED : LOG_GREEN, Build::cacheDirty ? "dirty" : "clean" );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BuilderCore::objects_gather()
{
	PrintLnColor( LOG_WHITE, TAB "Gather Objects..." );
	static char path[PATH_SIZE];

	// Engine
	strjoin( path, Build::pathEngine, SLASH "manta" );
	Objects::gather( path, true );

	// Project
	strjoin( path, Build::pathProject, SLASH "runtime" );
	Objects::gather( path, true );
}


void BuilderCore::objects_cache()
{
	Build::cacheDirtyObjects |= Build::cacheDirty ||
		( Objects::objectFilesCount != Build::cacheBufferPrevious.read<usize>() );
	Build::cacheDirty |= Build::cacheDirtyObjects;
	Build::cacheBufferCurrent.write( Objects::objectFilesCount );

	PrintColor( LOG_WHITE, TAB "Objects Cache... " );
	PrintLnColor( Build::cacheDirtyObjects ? LOG_RED : LOG_GREEN, Build::cacheDirtyObjects ? "dirty" : "clean" );
}


void BuilderCore::objects_parse()
{
	if( !Build::cacheDirty ) { return; }
	PrintLnColor( LOG_WHITE, TAB "Parse Objects..." );

	// Parse
	Objects::parse();
}


void BuilderCore::objects_write()
{
	if( !Build::cacheDirty ) { return; }
	PrintLnColor( LOG_WHITE, TAB "Write Objects..." );

	// Resolve inheritance tree
	Objects::resolve();

	// Validate Objects
	Objects::validate();

	// Generate C++ files
	Objects::generate();

	// Write C++ files to disk
	Objects::write();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BuilderCore::shaders_gather()
{
	PrintLnColor( LOG_WHITE, TAB "Gather Shaders..." );

	// Gather Shaders
	Gfx::gather( Build::pathEngine, true );
	Gfx::gather( Build::pathProject, true );
}


void BuilderCore::shaders_cache()
{
	Build::cacheDirtyShaders |= Build::cacheDirty ||
		( Gfx::shaderFileCount != Build::cacheBufferPrevious.read<usize>() );
	Build::cacheDirty |= Build::cacheDirtyShaders;
	Build::cacheBufferCurrent.write( Gfx::shaderFileCount );

	PrintColor( LOG_WHITE, TAB "Shaders Cache... " );
	PrintLnColor( Build::cacheDirtyShaders ? LOG_RED : LOG_GREEN, Build::cacheDirtyShaders ? "dirty" : "clean" );
}


void BuilderCore::shaders_build()
{
	if( !Build::cacheDirty ) { return; }
	PrintLnColor( LOG_WHITE, TAB "Build Shaders..." );

	Gfx::build();
}


void BuilderCore::shaders_write()
{
	if( !Build::cacheDirty ) { return; }
	PrintLnColor( LOG_WHITE, TAB "Write Shaders..." );

	Gfx::write();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BuilderCore::assets_gather()
{
	PrintLnColor( LOG_WHITE, TAB "Gather Assets..." );

	// Gather Sprites
	Assets::sprites.gather( Build::pathEngine );
	Assets::sprites.gather( Build::pathProject );

	// Gather Materials
	Assets::materials.gather( Build::pathEngine );
	Assets::materials.gather( Build::pathProject );

	// Gather Fonts
	Assets::fonts.gather( Build::pathEngine );
	Assets::fonts.gather( Build::pathProject );

	// Gather Sounds
	Assets::sounds.gather( Build::pathEngine );
	Assets::sounds.gather( Build::pathProject );

	// Gather Songs
	Assets::songs.gather( Build::pathEngine );
	Assets::songs.gather( Build::pathProject );

	// Gather Meshes
	Assets::meshes.gather( Build::pathEngine );
	Assets::meshes.gather( Build::pathProject );

	// Gather Skeleton2Ds
	Assets::skeleton2Ds.gather( Build::pathEngine );
	Assets::skeleton2Ds.gather( Build::pathProject );
}


void BuilderCore::assets_cache()
{
	Build::cacheDirtyAssets |= Build::cacheDirty ||
		( Assets::assetFileCount != Build::cacheBufferPrevious.read<usize>() );
	Build::cacheDirty |= Build::cacheDirtyAssets;
	Build::cacheBufferCurrent.write( Assets::assetFileCount );

	PrintColor( LOG_WHITE, TAB "Assets Cache... " );
	PrintLnColor( Build::cacheDirtyAssets ? LOG_RED : LOG_GREEN, Build::cacheDirtyAssets ? "dirty" : "clean" );
}


void BuilderCore::assets_build()
{
	if( !Build::cacheDirty ) { return; }
	PrintLnColor( LOG_WHITE, TAB "Build Assets..." );

	// Write Textures
	Assets::textures.write();

	// Write Glyphs
	Assets::glyphs.write();

	// Write Sprites
	Assets::sprites.write();

	// Write Materials
	Assets::materials.write();

	// Write Fonts
	Assets::fonts.write();

	// Write Sounds
	Assets::sounds.write();

	// Write Songs
	Assets::songs.write();

	// Write Meshes
	Assets::meshes.write();

	// Write Skeleton2Ds
	Assets::skeleton2Ds.write();
}


void BuilderCore::assets_write()
{
	if( !Build::cacheDirty ) { return; }
	PrintLnColor( LOG_WHITE, TAB "Write Assets..." );

	// assets.generated.hpp
	{
		if( verbose_output() ) { PrintColor( LOG_CYAN, TAB TAB "Write %s", Assets::pathHeader ); }
		Timer timer;

		// Begin header
		String header;
		header.append( "/*\n" );
		header.append( " * File generated by build.exe--do not edit!\n" );
		header.append( " * Refer to: source/build/build.cpp (BuilderCore::assets_write)\n" );
		header.append( " */\n" );
		header.append( "#pragma once\n\n" );
		header.append( "#include <core/types.hpp>\n" );
		header.append( "#include <core/debug.hpp>\n\n\n" );

		// Append header
		header.append( Assets::header );

		// Save header
		ErrorIf( !header.save( Assets::pathHeader ), "Failed to write '%s'", Assets::pathHeader );

		if( verbose_output() ) { PrintLnColor( LOG_WHITE, " (%.3f ms)", timer.elapsed_ms() ); }
	}

	// assets.generated.cpp
	{
		if( verbose_output() ) { PrintColor( LOG_CYAN, TAB TAB "Write %s", Assets::pathSource ); }
		Timer timer;

		// Begin source
		String source;
		source.append( "/*\n" );
		source.append( " * File generated by build.exe--do not edit!\n" );
		source.append( " * Refer to: source/build/build.cpp (BuilderCore::assets_write)\n" );
		source.append( " */\n" );
		source.append( "#include <assets.generated.hpp>\n" );
		source.append( "#include <manta/fonts.hpp>\n\n\n" );

		// Append source
		source.append( Assets::source );

		// Save source
		ErrorIf( !source.save( Assets::pathSource ), "Failed to write '%s'", Assets::pathSource );

		if( verbose_output() ) { PrintLnColor( LOG_WHITE, " (%.3f ms)", timer.elapsed_ms() ); }
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BuilderCore::binary_cache()
{
	Build::cacheDirtyBinary |= Build::cacheDirty;
	Build::cacheDirtyBinary |= ( Assets::binary.size() > 0 );

	PrintColor( LOG_WHITE, TAB "Binary Cache... " );
	PrintLnColor( Build::cacheDirtyBinary ? LOG_RED : LOG_GREEN, Build::cacheDirtyBinary ? "dirty" : "clean" );
}


void BuilderCore::binary_write()
{
	if( !Build::cacheDirtyBinary ) { return; }

	// Log
	PrintColor( LOG_WHITE, TAB "Writing Binary" );
	PrintLnColor( LOG_CYAN, " (%.2f MB)", MB( Assets::binary.size() ) );
	if( verbose_output() ) { PrintColor( LOG_CYAN, TAB TAB "Write %s", Build::pathOutputRuntimeBinary ); }
	Timer timer;

	// Write
	ErrorIf( !Assets::binary.save( Build::pathOutputRuntimeBinary ),
		"Failed to write binary (%s)", Build::pathOutputRuntimeBinary );

	// Log
	if( verbose_output() ) { PrintLnColor( LOG_WHITE, " (%.3f ms)", timer.elapsed_ms() ); }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BuilderCore::compile_project()
{
	// Include Directories
	{
		Build::compile_add_include_directory( ".." SLASH ".." SLASH "runtime" ); // Project
	}

	// C++ Sources + Library Linkage
	{
		char path[PATH_SIZE];
		PrintLnColor( LOG_WHITE, TAB "Gather Project Sources..." );
		strjoin( path, Build::pathProject, SLASH "runtime" );
		Build::compile_add_sources( path, true, ".cpp", Build::tc.linkerExtensionObj ); // Project
	}
}


void BuilderCore::compile_engine()
{
	// Include Directories
	{
		Build::compile_add_include_directory( ".." SLASH "generated" ); // Generated C++ files
		Build::compile_add_include_directory( ".." SLASH ".." SLASH ".." SLASH ".." SLASH "source" ); // Engine
	}

	// C++ Sources + Library Linkage
	PrintLnColor( LOG_WHITE, TAB "Gather Engine Sources..." );
	{
		usize count = 0;
		char path[PATH_SIZE];

		// root/projects/<project>/output/generated/*.cpp
		strjoin( path, Build::pathOutput, SLASH "generated" );
		Build::compile_add_sources( path, false, ".cpp", Build::tc.linkerExtensionObj );

		// root/source/*.cpp
		strjoin( path, Build::pathEngine );
		Build::compile_add_sources( path, false, ".cpp", Build::tc.linkerExtensionObj );

		// -r root/source/vendor/*.cpp
		strjoin( path, Build::pathEngine, SLASH "vendor" );
		Build::compile_add_sources( path, true, ".cpp", Build::tc.linkerExtensionObj );

		// -r root/source/core/*.cpp
		strjoin( path, Build::pathEngine, SLASH "core" );
		Build::compile_add_sources( path, true, ".cpp", Build::tc.linkerExtensionObj );

		// root/source/manta/*.cpp
		strjoin( path, Build::pathEngine, SLASH "manta" );
		Build::compile_add_sources( path, false, ".cpp", Build::tc.linkerExtensionObj );

		// Backend Sources
		{
			// Audio | -r source/manta/backend/audio/*.cpp
			strjoin( path, Build::pathEngine, SLASH "manta" SLASH "backend" SLASH "audio" SLASH, BACKEND_AUDIO );
			count = Build::compile_add_sources( path, true, ".cpp", Build::tc.linkerExtensionObj );
			ErrorIf( !count, "No backend found for 'audio' (%s)", path );
			if( OS_WINDOWS ) { Build::compile_add_library( "Ole32" ); }
			if( OS_MACOS ) { Build::compile_add_library( "AudioToolbox" ); }
			if( OS_LINUX ) { Build::compile_add_library( "asound" ); }

			// Filesystem | -r source/manta/backend/filesystem/*.cpp
			strjoin( path, Build::pathEngine, SLASH "manta" SLASH "backend" SLASH "filesystem" SLASH, BACKEND_FILESYSTEM );
			count = Build::compile_add_sources( path, true, ".cpp", Build::tc.linkerExtensionObj );
			ErrorIf( !count, "No backend found for 'filesystem' (%s)", path );

			// Network | -r source/manta/backend/network/*.cpp
			strjoin( path, Build::pathEngine, SLASH "manta" SLASH "backend" SLASH "network" SLASH, BACKEND_NETWORK );
			count = Build::compile_add_sources( path, true, ".cpp", Build::tc.linkerExtensionObj );
			ErrorIf( !count, "No backend found for 'network' (%s)", path );
			if( OS_WINDOWS ) { Build::compile_add_library( "ws2_32" ); }

			// Grahpics | -r source/manta/backend/gfx/*.cpp
			strjoin( path, Build::pathEngine, SLASH "manta" SLASH "backend" SLASH "gfx" SLASH, BACKEND_GRAPHICS );
			count = Build::compile_add_sources( path, false, ".cpp", Build::tc.linkerExtensionObj );
			ErrorIf( !count, "No backend found for 'gfx' (%s)", path );

			if( GRAPHICS_OPENGL )
			{
				if( OS_WINDOWS )
				{
					// WGL
					strjoin( path, Build::pathEngine, SLASH "manta" SLASH "backend" SLASH "gfx" SLASH,
						BACKEND_GRAPHICS, SLASH "wgl" );
					count = Build::compile_add_sources( path, false, ".cpp", Build::tc.linkerExtensionObj );
					ErrorIf( !count, "No backend found for opengl 'wgl' (%s)", path );
					Build::compile_add_library( "opengl32" );
					Build::compile_add_library( "gdi32" );
				}
				if( OS_MACOS )
				{
					// NSGL
					strjoin( path, Build::pathEngine, SLASH "manta" SLASH "backend" SLASH "gfx" SLASH,
						BACKEND_GRAPHICS, SLASH "nsgl" );
					count = Build::compile_add_sources( path, false, ".mm", Build::tc.linkerExtensionObj );
					ErrorIf( !count, "No backend found for opengl 'nsgl' (%s)", path );
					Build::compile_add_library( "OpenGL" );
				}
				if( OS_LINUX )
				{
					// WGL
					strjoin( path, Build::pathEngine, SLASH "manta" SLASH "backend" SLASH "gfx" SLASH,
						BACKEND_GRAPHICS, SLASH "glx" );
					count = Build::compile_add_sources( path, false, ".cpp", Build::tc.linkerExtensionObj );
					ErrorIf( !count, "No backend found for opengl 'glx' (%s)", path );
					Build::compile_add_library( "GL" );
				}
			}
			else
			if( GRAPHICS_D3D11 )
			{
				if( OS_WINDOWS )
				{
					Build::compile_add_library( "d3d11" );
					Build::compile_add_library( "d3dcompiler" );
					Build::compile_add_library( "dxgi" );
				}
			}

			// Thread | -r source/manta/backend/thread/*.cpp
			strjoin( path, Build::pathEngine, SLASH "manta" SLASH "backend" SLASH "thread" SLASH, BACKEND_THREAD );
			count = Build::compile_add_sources( path, true, ".cpp", Build::tc.linkerExtensionObj );
			ErrorIf( !count, "No backend found for 'thread' (%s)", path );

			// Time | -r source/manta/backend/time/*.cpp
			strjoin( path, Build::pathEngine, SLASH "manta" SLASH "backend" SLASH "time" SLASH, BACKEND_TIMER );
			count = Build::compile_add_sources( path, true, ".cpp", Build::tc.linkerExtensionObj );
			ErrorIf( !count, "No backend found for 'time' (%s)", path );
			if( OS_WINDOWS ) { Build::compile_add_library( "winmm" ); }

			// Window | -r source/manta/backend/window/*.cpp
			strjoin( path, Build::pathEngine, SLASH "manta" SLASH "backend" SLASH "window" SLASH, BACKEND_WINDOW );
			count = Build::compile_add_sources( path, true, OS_MACOS ? ".mm" : ".cpp", Build::tc.linkerExtensionObj );
			ErrorIf( !count, "No backend found for 'window' (%s)", path );
			if( OS_WINDOWS ) { Build::compile_add_library( "user32" ); Build::compile_add_library( "Shell32" ); }
			if( OS_MACOS ) { Build::compile_add_library( "Cocoa" ); }
			if( OS_LINUX ) { Build::compile_add_library( "X11" ); }
		}
	}
}


void BuilderCore::compile_write_ninja()
{
	String output;
	PrintLnColor( LOG_WHITE, TAB "Write Ninja" );
	{
		// Load <project>/config.json
		String configJSONContents;
		char pathConfig[PATH_SIZE];
		strjoin_path( pathConfig, "projects", Build::args.project, "configs.json" );
		ErrorIf( !configJSONContents.load( pathConfig ), "Failed to load configs file: %s\n", pathConfig );


		// Read config.json
		JSON configsJSON = JSON( configJSONContents ).object( Build::args.config )
			.object( "compile" ).object( Build::args.toolchain );
		String configCompilerFlags = configsJSON.get_string( "compilerFlags" );
		String configCompilerFlagsWarnings = configsJSON.get_string( "compilerFlagsWarnings" );
		String configLinkerFlags = configsJSON.get_string( "linkerFlags" );

		// Rule compile
		output.append( "rule compile\n" );
		output.append( PIPELINE_COMPILER_MSVC ? "  deps = msvc\n" : "  deps = gcc\n  depfile = $out.d\n" );
		output.append( "  command = " );
		output.append( Build::tc.compilerName );
		output.append( " $in " );
		output.append( Build::tc.compilerOutput );
		output.append( "$out " );

		// Core compiler flags (build/toolchains.hpp)
		output.append( Build::tc.compilerFlags );

		// Compiler architecture (x64/arm/etc.)
		output.append( " " ).append( Build::tc.compilerFlagsArchitecture );

		// Project flags (configs.json)
		if( configCompilerFlags.length_bytes() > 0 ) { output.append( " " ).append( configCompilerFlags ); }

		// Project warning flags (configs.json)
		if( configCompilerFlagsWarnings.length_bytes() > 0 ) { output.append( " " ).append( configCompilerFlagsWarnings ); }

		// Core compiler warnings (build/toolchains.hpp)
		output.append( " " ).append( Build::tc.compilerFlagsWarnings );
		static char includeFlag[1024];
		for( String &includeDirectory : Build::includeDirectories )
		{
			// #include <...> directories
			snprintf( includeFlag, sizeof( includeFlag ), Build::tc.compilerFlagsIncludes, includeDirectory.cstr() );
			output.append( " " ).append( includeFlag );
		}
		output.append( "\n\n" );


#if PIPELINE_OS_WINDOWS
		// Rule RC
		output.append( "rule rc\n" );
		output.append( "  command = windres --input $in --output $out --output-format=coff\n\n" );
#endif

		// Rule Link
		output.append( "rule link\n  command = " );
		output.append( Build::tc.linkerName );
		output.append( " $in " );
		output.append( Build::tc.linkerOutput );
		output.append( "$out " );
		output.append( Build::tc.linkerFlags );
		if( configLinkerFlags.length_bytes() > 0 ) { output.append( " " ); output.append( configLinkerFlags ); }
		for( String &library : Build::libraries )
		{
		#if OS_MACOS
			output.append( " -framework " );
			output.append( library );
		#else
			output.append( " " );
			output.append( Build::tc.linkerPrefixLibrary );
			output.append( library );
			output.append( Build::tc.linkerExtensionLibrary );
		#endif
		}
		output.append( "\n\n" );

		// Build Sources
		for( Source &source : Build::sources )
		{
			output.append( "build " );
			output.append( source.objPath );
			output.append( ": compile .." SLASH ".." SLASH ".." SLASH ".." SLASH );
			output.append( source.srcPath );
			output.append( "\n" );
		}
		output.append( "\n" );

#if PIPELINE_OS_WINDOWS
		// Package RC
		output.append( "build " );
		output.append( Build::packageRC.objPath );
		output.append( ": rc .." SLASH ".." SLASH ".." SLASH ".." SLASH );
		output.append( Build::packageRC.srcPath );
		output.append( "\n\n" );
#endif

		// Build Exe
		output.append( "build " );
		output.append( Build::args.project );
		output.append( Build::tc.linkerExtensionExe );
		output.append( ": link" );
		for( Source &source : Build::sources ) { output.append( " " ).append( source.objPath ); }
#if PIPELINE_OS_WINDOWS
		output.append( " " ).append( Build::packageRC.objPath );
#endif
		output.append( "\n" );

		// Write build.ninja
		char path[PATH_SIZE];
		strjoin( path, Build::pathOutput, SLASH "runtime" SLASH "build.ninja" );
		ErrorIf( !output.save( path ), "Failed to write %s", path );
		if( verbose_output() ) { PrintLnColor( LOG_CYAN, TAB TAB "Wrote ninja to: %s", path ); }
	}
}


void BuilderCore::compile_run_ninja()
{
	PrintLnColor( LOG_WHITE, TAB "Run Ninja" );

	char path[PATH_SIZE];
	strjoin( path, Build::pathOutput, SLASH "runtime" );
	strjoin( Build::commandNinja, "ninja -C ", path );

	// Run Ninja
	if( verbose_output() ) { PrintLnColor( LOG_MAGENTA, TAB TAB "> %s", Build::commandNinja ); }
	Print( "\n ");

	ErrorIf( system( Build::commandNinja ) != 0, "Compile failed" );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BuilderCore::package_load()
{
	// Load <project>/package.json
	String jsonFile;
	char pathPackage[PATH_SIZE];
	strjoin_path( pathPackage, "projects", Build::args.project, "package.json" );
	ErrorIf( !jsonFile.load( pathPackage ), "Failed to load package.json file: %s\n", pathPackage );

	// Read package.json
	JSON json = JSON( jsonFile );
	Build::packageName = json.get_string( "name", Build::args.project );
	Build::packageCompany = json.get_string( "company", Build::args.project );
	Build::packageVersionA = json.get_string( "versionA", "1" );
	Build::packageVersionB = json.get_string( "versionB", "0" );
	Build::packageVersionC = json.get_string( "versionC", "0" );
	Build::packageVersionD = json.get_string( "versionD", "0" );

	Build::packageIcon = json.get_string( "icon", "" );
	if( Build::packageIcon.length_bytes() > 0 )
	{
		char iconPath[PATH_SIZE];
		strjoin( iconPath, Build::pathProject, SLASH, Build::packageIcon.cstr() );
		Build::packageIcon = iconPath;
	}
	else
	{
		Build::packageIcon = ".manta" SLASH "icon.png";
	}

	// package.rc
#if PIPELINE_OS_WINDOWS
	package_generate_rc();
#endif
}


void BuilderCore::package_generate_rc()
{
	// package.rc
	String rc = "";
	rc.append( "1 ICON \"" ).append( Build::args.project ).append( ".ico\"\n" );
	rc.append( "1 VERSIONINFO\n" );
	rc.append( "\tFILEVERSION " );
	rc.append( Build::packageVersionA ).append( "," );
	rc.append( Build::packageVersionB ).append( "," );
	rc.append( Build::packageVersionC ).append( "," );
	rc.append( Build::packageVersionD ).append( "\n" );
	rc.append( "\tPRODUCTVERSION " );
	rc.append( Build::packageVersionA ).append( "," );
	rc.append( Build::packageVersionB ).append( "," );
	rc.append( Build::packageVersionC ).append( "," );
	rc.append( Build::packageVersionD ).append( "\n" );
	rc.append( "\tFILEFLAGSMASK 0x3F\n" );
	rc.append( "\tFILEOS 0x04\n" );
	rc.append( "\tFILETYPE 0x01\n" );
	rc.append( "BEGIN\n" );
	rc.append( "\tBLOCK \"StringFileInfo\"\n" );
	rc.append( "\tBEGIN\n" );
	rc.append( "\t\tBLOCK \"040904b0\"\n" );
	rc.append( "\t\tBEGIN\n" );
	rc.append( "\t\t\tVALUE \"CompanyName\", \"" ).append( Build::packageCompany ).append( "\"\n" );
	rc.append( "\t\t\tVALUE \"FileDescription\", \"" ).append( Build::packageName ).append( "\"\n" );
	rc.append( "\t\t\tVALUE \"FileVersion\", \"" );
	rc.append( Build::packageVersionA ).append( "." );
	rc.append( Build::packageVersionB ).append( "." );
	rc.append( Build::packageVersionC ).append( "." );
	rc.append( Build::packageVersionD ).append( "\"\n" );
	rc.append( "\t\t\tVALUE \"LegalCopyright\", \"\"\n" );
	rc.append( "\t\t\tVALUE \"ProductName\", \"" ).append( Build::packageName ).append( "\"\n" );
	rc.append( "\t\t\tVALUE \"ProductVersion\", \"" );
	rc.append( Build::packageVersionA ).append( "." );
	rc.append( Build::packageVersionB ).append( "." );
	rc.append( Build::packageVersionC ).append( "." );
	rc.append( Build::packageVersionD ).append( "\"\n" );
	rc.append( "\t\tEND\n" );
	rc.append( "\tEND\n\n" );
	rc.append( "\tBLOCK \"VarFileInfo\"\n" );
	rc.append( "\tBEGIN\n" );
	rc.append( "\t\tVALUE \"Translation\", 0x409, 0x4B0\n" );
	rc.append( "\tEND\n" );
	rc.append( "END\n" );

	char pathSrc[PATH_SIZE];
	char pathObj[PATH_SIZE];

	strjoin( pathSrc, Build::pathOutputRuntime, SLASH, Build::args.project, ".rc" );
	strjoin( pathObj, "objects" SLASH, Build::args.project, ".res" );

	PrintLn( "%s", pathSrc );
	ErrorIf( !rc.save( pathSrc ), "Failed to save %s", pathSrc );

	Build::packageRC.srcPath = pathSrc;
	Build::packageRC.objPath = pathObj;

	// <project>.ico
	char pathIco[PATH_SIZE];
	strjoin( pathIco, Build::pathOutputRuntime, SLASH, Build::args.project, ".ico" );
	ErrorIf( !png_to_ico( Build::packageIcon, pathIco ), "Failed to save %s.ico",
		Build::args.project );
}


bool BuilderCore::package_windows()
{
	// Create Package Directory
	char pathPackage[PATH_SIZE];
	snprintf( pathPackage, PATH_SIZE, "%s" SLASH "%s-%s-%s-%s-%s", Build::pathPackages,
		Build::args.project, Build::args.config, Build::args.architecture, Build::args.toolchain, Build::args.gfx );

	directory_delete( pathPackage );
	directory_create( pathPackage );

	// Copy Package Contents
	char pathSrc[PATH_SIZE];
	char pathDst[PATH_SIZE];
	{
		// Copy exe
		strjoin( pathSrc, Build::pathOutputRuntimeExecutable );
		strjoin( pathDst, pathPackage, SLASH, Build::args.project, Build::tc.linkerExtensionExe );
		if( !file_copy( pathSrc, pathDst ) ) { return false; }

		// Copy .bin
		strjoin( pathSrc, Build::pathOutputRuntimeBinary );
		strjoin( pathDst, pathPackage, SLASH, Build::args.project, ".bin" );
		if( !file_copy( pathSrc, pathDst ) ) { return false; }

		// Copy Licenses
		strjoin( pathSrc, Build::pathOutputRuntimeLicenses );
		strjoin( pathDst, pathPackage, SLASH, "licenses" );
		if( !directory_copy( pathSrc, pathDst ) ) { return false; }
	}

	PrintLn( TAB "> %s", pathPackage );
	return true;
}


bool BuilderCore::package_linux()
{
	// Linux is basically the same as Windows in this regard
	return package_windows();
}


bool BuilderCore::package_macos()
{
	// Create Package
	char pathPackage[PATH_SIZE];
	snprintf( pathPackage, PATH_SIZE, "%s" SLASH "%s-%s-%s-%s-%s.app", Build::pathPackages,
		Build::args.project, Build::args.config, Build::args.architecture, Build::args.toolchain, Build::args.gfx );
	directory_delete( pathPackage );
	directory_create( pathPackage );

	// Package Contents
	char pathSrc[PATH_SIZE];
	char pathDst[PATH_SIZE];
	{
		// Create .iconset directory
		char pathIconSet[PATH_SIZE];
		strjoin( pathIconSet, Build::pathOutputRuntime, SLASH, Build::args.project, ".iconset" );
		directory_delete( pathIconSet );
		if( !directory_create( pathIconSet ) ) { return false; }

		// Fill <project>.iconset directory
		char pathIconPng[PATH_SIZE];
		strjoin( pathIconPng, pathIconSet, SLASH, "icon_16x16.png" );
		if( !file_copy( Build::packageIcon.cstr(), pathIconPng ) ) { return false; }
		strjoin( pathIconPng, pathIconSet, SLASH, "icon_32x32.png" );
		if( !file_copy( Build::packageIcon.cstr(), pathIconPng ) ) { return false; }
		strjoin( pathIconPng, pathIconSet, SLASH, "icon_64x64.png" );
		if( !file_copy( Build::packageIcon.cstr(), pathIconPng ) ) { return false; }
		strjoin( pathIconPng, pathIconSet, SLASH, "icon_128x128.png" );
		if( !file_copy( Build::packageIcon.cstr(), pathIconPng ) ) { return false; }
		strjoin( pathIconPng, pathIconSet, SLASH, "icon_256x256.png" );
		if( !file_copy( Build::packageIcon.cstr(), pathIconPng ) ) { return false; }
		strjoin( pathIconPng, pathIconSet, SLASH, "icon_512x512.png" );
		if( !file_copy( Build::packageIcon.cstr(), pathIconPng ) ) { return false; }
		strjoin( pathIconPng, pathIconSet, SLASH, "icon_512x512@2.png" );
		if( !file_copy( Build::packageIcon.cstr(), pathIconPng ) ) { return false; }

		// Generate icons (system: iconutil)
		char iconCommand[PATH_SIZE];
		snprintf( iconCommand, sizeof( iconCommand ), "iconutil -c icns %s", pathIconSet );
		if( system( iconCommand ) != 0 ) { return false; }

		// Copy icons
		char pathIconIcs[PATH_SIZE];
		strjoin( pathIconIcs, Build::pathOutputRuntime, SLASH, Build::args.project, ".icns" );
		char pathIconPackage[PATH_SIZE];
		strjoin( pathIconPackage, pathPackage, SLASH, Build::args.project, ".icns" );
		if( !file_copy( pathIconIcs, pathIconPackage ) ) { return false; }

		// Copy exe
		strjoin( pathSrc, Build::pathOutputRuntimeExecutable );
		strjoin( pathDst, pathPackage, SLASH, Build::args.project, Build::tc.linkerExtensionExe );
		if( !file_copy( pathSrc, pathDst ) ) { return false; }

		// Copy .bin
		strjoin( pathSrc, Build::pathOutputRuntimeBinary );
		strjoin( pathDst, pathPackage, SLASH, Build::args.project, ".bin" );
		if( !file_copy( pathSrc, pathDst ) ) { return false; }

		// Copy Licenses
		strjoin( pathSrc, Build::pathOutputRuntimeLicenses );
		strjoin( pathDst, pathPackage, SLASH, "licenses" );
		if( !directory_copy( pathSrc, pathDst ) ) { return false; }

		// Info.plist
		String info = "";
		info.append( "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" );
		info.append( "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" " );
		info.append( "\"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n" );
		info.append( "<plist version=\"1.0\">\n" );
		info.append( "<dict>\n\n" );
		{
			info.append( "\t<key>CFBundleExecutable</key>\n" );
			info.append( "\t<string>" ).append( Build::args.project ).append( "</string>\n\n" );

			info.append( "\t<key>CFBundleIconFile</key>\n" );
			info.append( "\t<string>" ).append( Build::args.project ).append( "</string>\n\n" );

			info.append( "\t<key>CFBundleIdentifier</key>\n" );
			info.append( "\t<string>" );
			info.append( "com.manta." );
			info.append( Build::args.project ).append( "_" );
			info.append( Build::args.config ).append( "_" );
			info.append( Build::args.architecture ).append( "_" );
			info.append( Build::args.gfx );
			info.append( "</string>\n\n" );

			info.append( "\t<key>CFBundleVersion</key>\n" );
			info.append( "\t<string>1.0</string>\n\n" );

			info.append( "\t<key>CFBundleShortVersionString</key>\n" );
			info.append( "\t<string>1.0</string>\n\n" );

			info.append( "\t<key>CFBundlePackageType</key>\n" );
			info.append( "\t<string>APPL</string>\n\n" );
		}
		info.append( "</dict>\n" );
		info.append( "</plist>\n" );

		strjoin( pathDst, pathPackage, SLASH, "Info.plist" );
		if( !info.save( pathDst ) ) { return false; }
	}

	PrintLn( TAB "> %s", pathPackage );
	return true;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BuilderCore::executable_run( int argc, char **argv )
{
	strjoin( Build::commandRun, Build::pathOutputRuntimeExecutable );

	// Run Executable
	int code = system( Build::commandRun );
	PrintLnColor( code ? LOG_RED : LOG_WHITE, "\n%s%s terminated with code %d\n",
		Build::args.project, Build::tc.linkerExtensionExe, code );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool verbose_output()
{
	return strcmp( Build::args.verbose, "1" ) == 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////