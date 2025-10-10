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

struct CacheBinaryStage { usize offset; usize size; };

enum_type( CACHE_BINARY_STAGE, CacheID )
{
	CACHE_BINARY_STAGE_OBJECTS = 0,
	CACHE_BINARY_STAGE_GFX = 1,
	CACHE_BINARY_STAGE_ASSETS = 2,
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void verbose_log_gather( const char *name, const usize count )
{
	if( !verbose_output() ) { return; }
	PrintLnColor( LOG_CYAN, TAB TAB "%u %s%s found", count, name, count == 1 ? "" : "s" );
};

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
	char pathOutputBuildCacheBuild[PATH_SIZE];
	char pathOutputBuildCacheObjects[PATH_SIZE];
	char pathOutputBuildCacheGraphics[PATH_SIZE];
	char pathOutputBuildCacheAssets[PATH_SIZE];

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

	// Binary
	String header;
	Buffer binary;

	// Compile
	List<Source> sources;
	List<String> libraries;
	List<String> includeDirectories;
#if PIPELINE_OS_WINDOWS
	List<Source> rcs;
#endif

	// Cache
	Cache cache;
	bool buildBinary = false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Build::compile_add_source( const char *pathSrc, const char *pathObj, const char *extensionObj )
{
	static char buffer[PATH_SIZE];
	path_change_extension( buffer, sizeof( buffer ), pathObj, extensionObj );
	Build::sources.add( { pathSrc, buffer } );
}


usize Build::compile_add_sources( const char *directory, const bool recurse,
	const char *extensionSrc, const char *extensionObj )
{
	List<FileInfo> sourceFiles;
	directory_iterate( sourceFiles, directory, extensionSrc, recurse ); // C++

	static char pathObj[PATH_SIZE];
	for( FileInfo &sourceFile : sourceFiles )
	{
		strjoin( pathObj, "objects" SLASH, sourceFile.path );
		Build::compile_add_source( sourceFile.path, pathObj, extensionObj );
	}

	return sourceFiles.size();
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
		strjoin( Build::pathOutputBuildCacheBuild, Build::pathOutputBuild, SLASH "core.cache" );
		strjoin( Build::pathOutputBuildCacheObjects, Build::pathOutputBuild, SLASH "objects.cache" );
		strjoin( Build::pathOutputBuildCacheGraphics, Build::pathOutputBuild, SLASH "graphics.cache" );
		strjoin( Build::pathOutputBuildCacheAssets, Build::pathOutputBuild, SLASH "assets.cache" );

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
	int run = atoi( Build::args.run );

	// Begin
	{
		Build::begin();
		Build::cache_read( Build::pathOutputBuildCacheBuild );

		Objects::begin();
		Objects::cache_read( Build::pathOutputBuildCacheObjects );

		Gfx::begin();
		Gfx::cache_read( Build::pathOutputBuildCacheGraphics );

		Assets::begin();
		Assets::cache_read( Build::pathOutputBuildCacheAssets );
	}

	// Gather
	if( codegen )
	{
		PrintLnColor( LOG_WHITE, "\nGather Assets" );
		Timer timer;

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

		build_cache_validate();
		objects_cache_validate();
		shaders_cache_validate();
		assets_cache_validate();
		binary_cache_validate();

		PrintLnColor( LOG_WHITE, TAB "Finished (%.3f ms)", timer.elapsed_ms() );
	}

	// Build
	if( codegen )
	{
		PrintLnColor( LOG_WHITE, "\nBuild Binary" );

		if( Build::buildBinary )
		{
			Timer timer;

			objects_build();
			shaders_build();
			assets_build();

			PrintLnColor( LOG_WHITE, TAB "Finished (%.3f ms)", timer.elapsed_ms() );
		}
		else
		{
			PrintColor( LOG_WHITE, TAB "Skipped... " );
			PrintLnColor( LOG_GREEN, "clean" );
		}
	}

	// Write Binary
	if( build )
	{
		PrintLnColor( LOG_WHITE, "\nWrite Binary" );

		if( Build::buildBinary )
		{
			Timer timer;

			binary_write();

			PrintLnColor( LOG_WHITE, TAB "Finished (%.3f ms)", timer.elapsed_ms() );
		}
		else
		{
			PrintColor( LOG_WHITE, TAB "Skipped... " );
			PrintLnColor( LOG_GREEN, "clean" );
		}
	}

	// End
	{
		Assets::end();
		Assets::cache_write( Build::pathOutputBuildCacheAssets );

		Gfx::end();
		Gfx::cache_write( Build::pathOutputBuildCacheGraphics );

		Objects::end();
		Objects::cache_write( Build::pathOutputBuildCacheObjects );

		Build::end();
		Build::cache_write( Build::pathOutputBuildCacheBuild );
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
	switch( run )
	{
		case 1: executable_run( argc, argv ); break; // Normal Run
		case 2: executable_run_gpu_capture( argc, argv ); break; // GPU capture
		default: break; // Build only
	}
}


void BuilderCore::parse_arguments( int argc, char **argv )
{
	// Parse Arguments
	Build::args.parse( argc, argv );
	Build::tc.detect( Build::args );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BuilderCore::build_cache_validate()
{
	Build::cache_validate();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BuilderCore::objects_gather()
{
	PrintLnColor( LOG_WHITE, TAB "Objects..." );
	static char path[PATH_SIZE];

	usize numObjects = 0LLU;
	strjoin( path, Build::pathEngine, SLASH "manta" ); // Engine
	numObjects += Objects::gather( path, true );
	strjoin( path, Build::pathProject, SLASH "runtime" ); // Project
	numObjects += Objects::gather( path, true );
	verbose_log_gather( "object", numObjects );
}


void BuilderCore::objects_cache_validate()
{
	// If Build cache is dirty, Assets must also be dirty
	Objects::cache_validate();

	PrintColor( LOG_WHITE, TAB "Objects... " );
	PrintLnColor( Objects::cache.dirty ? LOG_RED : LOG_GREEN, Objects::cache.dirty ? "dirty" : "clean" );
}


void BuilderCore::objects_build()
{
	PrintLnColor( LOG_WHITE, TAB "Objects..." );
	Timer timer;

	if( Objects::cache.dirty )
	{
		Objects::parse();
		Objects::resolve();
		Objects::validate();
		Objects::codegen();

		if( !verbose_output() )
		{
			if( Objects::objectsBuilt > 0 )
			{
				PrintLnColor( LOG_RED, TAB TAB "%llu objects built", Objects::objectsBuilt );
			}
		}

		const double elapsed = timer.elapsed_ms();
		PrintLn( TAB TAB "Finished (%.3f ms)", elapsed );
	}
	else
	{
		const double elapsed = timer.elapsed_ms();
		PrintLnColor( LOG_MAGENTA, TAB TAB "Restored from cache" );
		PrintLn( TAB TAB "Finished (%.3f ms)", elapsed );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BuilderCore::shaders_gather()
{
	PrintLnColor( LOG_WHITE, TAB "Shaders..." );

	usize numShaders = 0LLU;
	numShaders += Gfx::gather( Build::pathEngine, true );
	numShaders += Gfx::gather( Build::pathProject, true );
	verbose_log_gather( "shader", numShaders );
}


void BuilderCore::shaders_cache_validate()
{
	// If Build cache is dirty, Assets must also be dirty
	Gfx::cache_validate();

	PrintColor( LOG_WHITE, TAB "Shaders... " );
	PrintLnColor( Gfx::cache.dirty ? LOG_RED : LOG_GREEN, Gfx::cache.dirty ? "dirty" : "clean" );
}


void BuilderCore::shaders_build()
{
	CacheBinaryStage cacheStage;
	const bool hasCachedStage = Build::cache.fetch( CACHE_BINARY_STAGE_GFX, cacheStage );
	Gfx::cacheReadOffset = hasCachedStage ? cacheStage.offset : 0LLU;

	PrintLnColor( LOG_WHITE, TAB "Shaders... " );
	Timer timer;

	if( Gfx::cache.dirty || !hasCachedStage )
	{
		Gfx::build();
		Gfx::codegen();

		if( !verbose_output() )
		{
			if( Gfx::shadersBuilt > 0 )
			{
				PrintLnColor( LOG_RED, TAB TAB "%llu shaders built", Gfx::shadersBuilt );
			}
		}

		const double elapsed = timer.elapsed_ms();
		PrintLn( TAB TAB "Finished (%.3f ms)", elapsed );
	}
	else
	{
		Gfx::binary.write_from_file( Build::pathOutputRuntimeBinary,
			cacheStage.offset, cacheStage.size );

		const double elapsed = timer.elapsed_ms();
		PrintLnColor( LOG_MAGENTA, TAB TAB "Restored from cache" );
		PrintLn( TAB TAB "Finished (%.3f ms)", elapsed );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BuilderCore::assets_gather()
{
	PrintLnColor( LOG_WHITE, TAB "Assets..." );

	usize numAssets = 0LLU;
	numAssets += Assets::dataAssets.gather( Build::pathEngine );
	numAssets += Assets::dataAssets.gather( Build::pathProject );
	verbose_log_gather( "asset", numAssets );

	usize numTextures = 0LLU;
	numTextures += Assets::textures.gather( Build::pathEngine );
	numTextures += Assets::textures.gather( Build::pathProject );
	verbose_log_gather( "texture", numTextures );

	usize numSprites = 0LLU;
	numSprites += Assets::sprites.gather( Build::pathEngine );
	numSprites += Assets::sprites.gather( Build::pathProject );
	verbose_log_gather( "sprite", numSprites );

	usize numMaterials = 0LLU;
	numMaterials += Assets::materials.gather( Build::pathEngine );
	numMaterials += Assets::materials.gather( Build::pathProject );
	verbose_log_gather( "material", numMaterials );

	usize numFonts = 0LLU;
	numFonts += Assets::fonts.gather( Build::pathEngine );
	numFonts += Assets::fonts.gather( Build::pathProject );
	verbose_log_gather( "font", numFonts );

	usize numSounds = 0LLU;
	numSounds += Assets::sounds.gather( Build::pathEngine );
	numSounds += Assets::sounds.gather( Build::pathProject );
	verbose_log_gather( "sound", numSounds );

	usize numSkeletons2D = 0LLU;
	numSkeletons2D += Assets::skeleton2Ds.gather( Build::pathEngine );
	numSkeletons2D += Assets::skeleton2Ds.gather( Build::pathProject );
	verbose_log_gather( "skeleton", numSkeletons2D );
}


void BuilderCore::assets_cache_validate()
{
	// If Build cache is dirty, Assets must also be dirty
	Assets::cache_validate();

	PrintColor( LOG_WHITE, TAB "Assets... " );
	PrintLnColor( Assets::cache.dirty ? LOG_RED : LOG_GREEN, Assets::cache.dirty ? "dirty" : "clean" );
}


void BuilderCore::assets_build()
{
	CacheBinaryStage cacheStage;
	const bool hasCachedStage = Build::cache.fetch( CACHE_BINARY_STAGE_ASSETS, cacheStage );
	Assets::cacheReadOffset = hasCachedStage ? cacheStage.offset : 0LLU;

	PrintLnColor( LOG_WHITE, TAB "Assets... " );
	Timer timer;

	if( Assets::cache.dirty || !hasCachedStage )
	{
		Assets::dataAssets.build();
		Assets::textures.build();
		Assets::glyphs.build();
		Assets::sprites.build();
		Assets::materials.build();
		Assets::fonts.build();
		Assets::sounds.build();
		Assets::skeleton2Ds.build();
		Assets::codegen();

		if( !verbose_output() )
		{
			if( Assets::assetsBuilt > 0 )
			{
				PrintLnColor( LOG_RED, TAB TAB "%llu assets built", Assets::assetsBuilt );
			}

			if( Assets::assetsCached > 0 )
			{
				PrintLnColor( LOG_MAGENTA, TAB TAB "%llu assets cached", Assets::assetsCached );
			}
		}

		const double elapsed = timer.elapsed_ms();
		PrintLn( TAB TAB "Finished (%.3f ms)", elapsed );
	}
	else
	{
		Assets::binary.write_from_file( Build::pathOutputRuntimeBinary,
			cacheStage.offset, cacheStage.size );

		const double elapsed = timer.elapsed_ms();
		PrintLnColor( LOG_MAGENTA, TAB TAB "Restored from cache" );
		PrintLn( TAB TAB "Finished (%.3f ms)", elapsed );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BuilderCore::binary_cache_validate()
{
	Build::buildBinary |= Build::cache.dirty;
	Build::buildBinary |= Objects::cache.dirty;
	Build::buildBinary |= Gfx::cache.dirty;
	Build::buildBinary |= Assets::cache.dirty;

	PrintColor( LOG_WHITE, TAB "Binary.. " );
	PrintLnColor( Build::buildBinary ? LOG_RED : LOG_GREEN, Build::buildBinary ? "dirty" : "clean" );
}


void BuilderCore::binary_write()
{
	Build::header.append( "#pragma once\n\n" );
	Build::header.append( COMMENT_BREAK "\n\n" );

	// Objects
	{
		CacheBinaryStage cacheStage;
		cacheStage.offset = 0;
		cacheStage.size = 0;
		// NOTE: Objects currently have no binary data, but this may change
		Build::cache.store( CACHE_BINARY_STAGE_OBJECTS, cacheStage );

		Build::header.append( "#define BINARY_OFFSET_OBJECTS ( " );
		Build::header.append( cacheStage.offset ).append( "LLU )\n" );
		Build::header.append( "#define BINARY_SIZE_OBJECTS ( " );
		Build::header.append( cacheStage.size ).append( "LLU )\n\n" );
	}

	// Gfx
	{
		CacheBinaryStage cacheStage;
		Assert( Gfx::binary.size() > 0 );
		cacheStage.offset = Build::binary.write( Gfx::binary.data, Gfx::binary.size() );
		cacheStage.size = Gfx::binary.size();
		Build::cache.store( CACHE_BINARY_STAGE_GFX, cacheStage );

		Build::header.append( "#define BINARY_OFFSET_GFX ( " );
		Build::header.append( cacheStage.offset ).append( "LLU )\n" );
		Build::header.append( "#define BINARY_SIZE_GFX ( " );
		Build::header.append( cacheStage.size ).append( "LLU )\n\n" );
	}

	// Assets
	{
		CacheBinaryStage cacheStage;
		Assert( Assets::binary.size() > 0 );
		cacheStage.offset = Build::binary.write( Assets::binary.data, Assets::binary.size() );
		cacheStage.size = Assets::binary.size();
		Build::cache.store( CACHE_BINARY_STAGE_ASSETS, cacheStage );

		Build::header.append( "#define BINARY_OFFSET_ASSETS ( " );
		Build::header.append( cacheStage.offset ).append( "LLU )\n" );
		Build::header.append( "#define BINARY_SIZE_ASSETS ( " );
		Build::header.append( cacheStage.size ).append( "LLU )\n\n" );
	}

	// Log
	PrintColor( LOG_WHITE, TAB "Writing Binary" );
	PrintLnColor( LOG_YELLOW, " (%.2f MB)", MB( Assets::binary.size() ) );
	if( verbose_output() )
	{
		PrintColor( LOG_WHITE, TAB TAB "Write " );
		PrintColor( LOG_CYAN, "%s", Build::pathOutputRuntimeBinary );
	}
	Timer timer;

	// Binary
	ErrorIf( !Build::binary.save( Build::pathOutputRuntimeBinary ),
		"Failed to write binary (%s)", Build::pathOutputRuntimeBinary );

	// Header
	static char pathHeader[PATH_SIZE];
	strjoin( pathHeader, Build::pathOutput, SLASH "generated" SLASH "binary.generated.hpp" );
	Build::header.append( COMMENT_BREAK );
	Build::header.save( pathHeader );

	// Log
	if( verbose_output() )
	{
		PrintLnColor( LOG_WHITE, " (%.3f ms)", timer.elapsed_ms() );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BuilderCore::compile_project()
{
	Timer timer;
	usize numSources = 0LLU;

	// Include Directories
	{
		Build::compile_add_include_directory( ".." SLASH ".." SLASH "runtime" );
	}

	// C++ Sources + Library Linkage
	{
		char path[PATH_SIZE];
		PrintLnColor( LOG_WHITE, TAB "Gather Project Sources..." );
		strjoin( path, Build::pathProject, SLASH "runtime" );
		numSources += Build::compile_add_sources( path, true, ".cpp", Build::tc.linkerExtensionObj );
	}

	// Logging
	if( verbose_output() )
	{
		PrintColor( LOG_CYAN, TAB TAB "%u source%s found in project", numSources, numSources == 1 ? "" : "s" );
		PrintLnColor( LOG_WHITE, " (%.3f ms)", timer.elapsed_ms() );
	}
}


void BuilderCore::compile_engine()
{
	Timer timer;
	usize numSources = 0LLU;

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
		numSources += Build::compile_add_sources( path, false, ".cpp", Build::tc.linkerExtensionObj );

		// root/source/*.cpp
		strjoin( path, Build::pathEngine );
		numSources += Build::compile_add_sources( path, false, ".cpp", Build::tc.linkerExtensionObj );

		// -r root/source/vendor/*.cpp
		strjoin( path, Build::pathEngine, SLASH "vendor" );
		numSources += Build::compile_add_sources( path, true, ".cpp", Build::tc.linkerExtensionObj );

		// -r root/source/core/*.cpp
		strjoin( path, Build::pathEngine, SLASH "core" );
		numSources += Build::compile_add_sources( path, true, ".cpp", Build::tc.linkerExtensionObj );

		// root/source/manta/*.cpp
		strjoin( path, Build::pathEngine, SLASH "manta" );
		numSources += Build::compile_add_sources( path, false, ".cpp", Build::tc.linkerExtensionObj );

		// Backend Sources
		{
			// Audio | -r source/manta/backend/audio/*.cpp
			strjoin( path, Build::pathEngine, SLASH "manta" SLASH "backend" SLASH "audio" SLASH, BACKEND_AUDIO );
			count = Build::compile_add_sources( path, true, ".cpp", Build::tc.linkerExtensionObj );
			numSources += count;
			ErrorIf( !count, "No backend found for 'audio' (%s)", path );
			if( OS_WINDOWS ) { Build::compile_add_library( "Ole32" ); }
			if( OS_MACOS ) { Build::compile_add_library( "AudioToolbox" ); }
			if( OS_LINUX ) { Build::compile_add_library( "asound" ); }

			// Filesystem | -r source/manta/backend/filesystem/*.cpp
			strjoin( path, Build::pathEngine, SLASH "manta" SLASH "backend" SLASH "filesystem" SLASH, BACKEND_FILESYSTEM );
			count = Build::compile_add_sources( path, true, ".cpp", Build::tc.linkerExtensionObj );
			numSources += count;
			ErrorIf( !count, "No backend found for 'filesystem' (%s)", path );

			// Network | -r source/manta/backend/network/*.cpp
			strjoin( path, Build::pathEngine, SLASH "manta" SLASH "backend" SLASH "network" SLASH, BACKEND_NETWORK );
			count = Build::compile_add_sources( path, true, ".cpp", Build::tc.linkerExtensionObj );
			numSources += count;
			ErrorIf( !count, "No backend found for 'network' (%s)", path );
			if( OS_WINDOWS ) { Build::compile_add_library( "ws2_32" ); }

			// Grahpics | -r source/manta/backend/gfx/*.cpp
			strjoin( path, Build::pathEngine, SLASH "manta" SLASH "backend" SLASH "gfx" SLASH, BACKEND_GRAPHICS );
			count = Build::compile_add_sources( path, false, ".cpp", Build::tc.linkerExtensionObj ) +
				Build::compile_add_sources( path, false, ".mm", Build::tc.linkerExtensionObj );
			numSources += count;
			ErrorIf( !count, "No backend found for 'gfx' (%s)", path );

			if( GRAPHICS_OPENGL )
			{
				if( OS_WINDOWS )
				{
					// WGL
					strjoin( path, Build::pathEngine, SLASH "manta" SLASH "backend" SLASH "gfx" SLASH,
						BACKEND_GRAPHICS, SLASH "wgl" );
					count = Build::compile_add_sources( path, false, ".cpp", Build::tc.linkerExtensionObj );
					numSources += count;
					ErrorIf( !count, "No backend found for opengl 'wgl' (%s)", path );
					Build::compile_add_library( "opengl32" );
					Build::compile_add_library( "gdi32" );
				}
				else if( OS_MACOS )
				{
					// NSGL
					strjoin( path, Build::pathEngine, SLASH "manta" SLASH "backend" SLASH "gfx" SLASH,
						BACKEND_GRAPHICS, SLASH "nsgl" );
					count = Build::compile_add_sources( path, false, ".mm", Build::tc.linkerExtensionObj );
					numSources += count;
					ErrorIf( !count, "No backend found for opengl 'nsgl' (%s)", path );
					Build::compile_add_library( "OpenGL" );
				}
				else if( OS_LINUX )
				{
					// WGL
					strjoin( path, Build::pathEngine, SLASH "manta" SLASH "backend" SLASH "gfx" SLASH,
						BACKEND_GRAPHICS, SLASH "glx" );
					count = Build::compile_add_sources( path, false, ".cpp", Build::tc.linkerExtensionObj );
					numSources += count;
					ErrorIf( !count, "No backend found for opengl 'glx' (%s)", path );
					Build::compile_add_library( "GL" );
				}
				else
				{
					Error( "OpenGL not supported on this platform!" );
				}
			}
			else if( GRAPHICS_D3D11 )
			{
				if( OS_WINDOWS )
				{
					Build::compile_add_library( "d3d11" );
					Build::compile_add_library( "d3dcompiler" );
					Build::compile_add_library( "dxgi" );
				}
				else
				{
					Error( "D3D11 not supported on this platform!" );
				}
			}
			if( GRAPHICS_METAL )
			{
				if( OS_MACOS )
				{
					Build::compile_add_library( "Metal" );
					Build::compile_add_library( "QuartzCore" );
				}
				else
				{
					Error( "Metal not supported on this platform!" );
				}
			}

			// Thread | -r source/manta/backend/thread/*.cpp
			strjoin( path, Build::pathEngine, SLASH "manta" SLASH "backend" SLASH "thread" SLASH, BACKEND_THREAD );
			count = Build::compile_add_sources( path, true, ".cpp", Build::tc.linkerExtensionObj );
			numSources += count;
			ErrorIf( !count, "No backend found for 'thread' (%s)", path );

			// Time | -r source/manta/backend/time/*.cpp
			strjoin( path, Build::pathEngine, SLASH "manta" SLASH "backend" SLASH "time" SLASH, BACKEND_TIMER );
			count = Build::compile_add_sources( path, true, ".cpp", Build::tc.linkerExtensionObj );
			numSources += count;
			ErrorIf( !count, "No backend found for 'time' (%s)", path );
			if( OS_WINDOWS ) { Build::compile_add_library( "winmm" ); }

			// Window | -r source/manta/backend/window/*.cpp
			strjoin( path, Build::pathEngine, SLASH "manta" SLASH "backend" SLASH "window" SLASH, BACKEND_WINDOW );
			count = Build::compile_add_sources( path, true, OS_MACOS ? ".mm" : ".cpp", Build::tc.linkerExtensionObj );
			numSources += count;
			ErrorIf( !count, "No backend found for 'window' (%s)", path );
			if( OS_WINDOWS ) { Build::compile_add_library( "user32" ); Build::compile_add_library( "Shell32" ); }
			if( OS_MACOS ) { Build::compile_add_library( "Cocoa" ); }
			if( OS_LINUX ) { Build::compile_add_library( "X11" ); }
		}
	}

	// Logging
	if( verbose_output() )
	{
		PrintColor( LOG_CYAN, TAB TAB "%u source%s found in engine", numSources, numSources == 1 ? "" : "s" );
		PrintLnColor( LOG_WHITE, " (%.3f ms)", timer.elapsed_ms() );
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
		if( verbose_output() )
		{
			PrintColor( LOG_WHITE, TAB TAB "Write " );
			PrintLnColor( LOG_CYAN, "%s", path );
		}
	}
}


void BuilderCore::compile_run_ninja()
{
	PrintLnColor( LOG_WHITE, TAB "Run Ninja" );

	char path[PATH_SIZE];
	strjoin( path, Build::pathOutput, SLASH "runtime" );
	const char *ninja = ninja_path();
	strjoin( Build::commandNinja, ninja, " -C ", path );

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


void BuilderCore::executable_run_gpu_capture( int argc, char **argv )
{
#if PIPELINE_OS_WINDOWS
	// RenderDoc needs full exe path, not relative path
	auto get_absolute_path = [&]( const char *partial, char *buffer, usize size ) -> void
	{
		DWORD len = GetFullPathNameA( partial, size, buffer, nullptr );
		if( len == 0 || len >= size ) { buffer[0] = '\0'; }
	};
	char pathAbsolute[PATH_SIZE];
	get_absolute_path( Build::pathOutputRuntimeExecutable, pathAbsolute, PATH_SIZE );
	strjoin( Build::commandRun, "renderdoccmd capture -w -c capture \"", pathAbsolute, "\"" );

	// Launch with RenderDoc
	PrintLnColor( LOG_MAGENTA, "\nLaunching RenderDoc!\n" TAB "'%s'\n", Build::commandRun );
	int code = system( Build::commandRun );
	PrintLnColor( code ? LOG_RED : LOG_WHITE, "\n%s%s terminated with code %d\n",
		Build::args.project, Build::tc.linkerExtensionExe, code );

	// Find captures
	List<FileInfo> files;
	directory_iterate( files, Build::pathOutputRuntime, ".rdc", false );

	// Open first capture
	if( files.count() > 0 )
	{
		char commandOpenCapture[PATH_SIZE];
		strjoin( commandOpenCapture, "qrenderdoc \"", files[0].path, "\"" );
		int code = system( commandOpenCapture );
		if( code ) { PrintLnColor( LOG_RED, "Failed to open RenderDoc capture: %s", files[0].path ); }
	}

	// Delete captures
	for( FileInfo &file : files ) { file_delete( file.path ); }

#else
	// GPU captures only supported on windows, pass through to run
	PrintLn( "\n" ); Warning( "Failed to launch GPU capture (windows only)" );
	executable_run( argc, argv );
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool verbose_output()
{
	return strcmp( Build::args.verbose, "1" ) == 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Build::begin()
{
	// ...
}


void Build::end()
{
	// ...
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Build::cache_read( const char *path )
{
	Build::cache.dirty  = false;
	Build::cache.dirty |= ( strcmp( Build::args.clean, "1" ) == 0 );

	if( !Build::cache.dirty ) { Build::cache.read( path ); }

	PrintColor( LOG_WHITE, TAB "Build Cache... " );
	PrintLnColor( Build::cache.dirty ? LOG_RED : LOG_GREEN, Build::cache.dirty ? "dirty" : "clean" );
}


void Build::cache_write( const char *path )
{
	if( !Build::buildBinary ) { return; }
	Build::cache.write( path );
}


void Build::cache_validate()
{
	// ...
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////