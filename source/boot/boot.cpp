#define _CRT_SECURE_NO_WARNINGS

#define HEADER_ONLY_DEBUG ( 1 )
#include <core/debug.hpp>

#include <boot/toolchains.hpp>
#include <boot/filesystem.hpp>
#include <boot/arguments.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static char commandNinja[1024];
static char commandRun[1024];

static char pathOutput[PATH_SIZE];
static char pathOutputBoot[PATH_SIZE];
static char pathOutputBootCache[PATH_SIZE];

static char pathOutputBuild[PATH_SIZE];
static char pathOutputBuildNinja[PATH_SIZE];
static char pathOutputBuildExe[PATH_SIZE];
static char pathOutputBuildCache[PATH_SIZE];

static char pathOutputGenerated[PATH_SIZE];
static char pathOutputGeneratedConfig[PATH_SIZE];

static char pathOutputRuntime[PATH_SIZE];
static char pathOutputRuntimeLicenses[PATH_SIZE];
static char pathOutputRuntimeDistributables[PATH_SIZE];
static char pathPackage[PATH_SIZE];

static char pathSourceManta[PATH_SIZE];
static char pathSourceMantaBuild[PATH_SIZE];
static char pathSourceMantaVendor[PATH_SIZE];
static char pathSourceMantaCore[PATH_SIZE];

static char pathProjectBuild[PATH_SIZE];

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main( int argc, char **argv )
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Parse Arguments & Toolchain

	Arguments args;
	args.parse( argc, argv );
	Toolchain tc { args.toolchain };

	// Print Args
	Debug::console_enable_colors();
	Print( PrintColor_Yellow, "\n>" );
	const u32 exeArgsCount = args.verbose_output() ? argc : 1;
	for( int i = 0; i < exeArgsCount; i++ ) { Print( PrintColor_Yellow, " %s", argv[i] ); }
	Print( "\n" );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Setup Paths

	// output
	{
		// projects/<project>/output
		strjoin_path( pathOutput, "projects", args.project, "output" );

		// projects/<project>/output/boot
		strjoin_path( pathOutputBoot, pathOutput, "boot" );

		// projects/<project>/output/boot/boot.cache
		strjoin_path( pathOutputBootCache, pathOutputBoot, "boot.cache" );

		// projects/<project>/output/build
		strjoin_path( pathOutputBuild, pathOutput, "build" );

		// projects/<project>/output/build/build.ninja
		strjoin_path( pathOutputBuildNinja, pathOutputBuild, "build.ninja" );

		// projects/<project>/output/build/build.exe
		strjoin_path( pathOutputBuildExe, pathOutputBuild, "build" );
		strappend( pathOutputBuildExe, tc.linkerExtensionExe );

		// projects/<project>/output/build/build.cache
		strjoin_path( pathOutputBuildCache, pathOutputBuild, "build.cache" );

		// projects/<project>/output/generated
		strjoin_path( pathOutputGenerated, pathOutput, "generated" );

		// projects/<project>/output/generated/pipeline.generated.hpp
		strjoin_path( pathOutputGeneratedConfig, pathOutputGenerated, "pipeline.generated.hpp" );

		// projects/<project>/output/runtime
		strjoin_path( pathOutputRuntime, pathOutput, "runtime" );

		// projects/<project>/output/runtime/licenses
		strjoin_path( pathOutputRuntimeLicenses, pathOutputRuntime, "licenses" );

		// projects/<project>/output/runtime/distributables
		strjoin_path( pathOutputRuntimeDistributables, pathOutputRuntime, "distributables" );

		// projects/<project>/output/package
		strjoin_path( pathPackage, pathOutput, "packages" );
	}

	// C++ Source Paths
	{
		// source/manta
		strjoin_path( pathSourceManta, "source" );

		// source/build
		strjoin_path( pathSourceMantaBuild, "source", "build" );

		// source/vendor
		strjoin_path( pathSourceMantaVendor, "source", "vendor" );

		// source/core
		strjoin_path( pathSourceMantaCore, "source", "core" );
	}

	// Project Paths
	{
		// projects/<project>/build
		strjoin_path( pathProjectBuild, "projects", args.project, "build" );
	}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Load & Check Cache

	const bool cacheDirty = PipelineCache::load( args, pathOutputBootCache );

	if( !args.verbose_output() )
	{
		Print( PrintColor_White, "Project: " );
		Print( PrintColor_Magenta, "%s ", args.project );
		PrintLn( PrintColor_Blue, "(%s, %s)", args.config, args.toolchain );
	}

	Print( PrintColor_White, "Boot Cache... " );
	PrintLn( cacheDirty ? PrintColor_Red : PrintColor_Green, cacheDirty ? "dirty" : "clean" );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Output Directories

	if( cacheDirty )
	{
		// If cache is dirty, clear /output directory
		directory_delete( pathOutputBuild, true );
		directory_delete( pathOutputGenerated, true );
		directory_delete( pathOutputRuntime, true );
		directory_delete( pathPackage, true );
	}

	directory_create( pathOutputBuild );
	directory_create( pathOutputGenerated );
	directory_create( pathOutputRuntime );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Write Core Defines (output/generated/pipeline.generated.hpp)

	if( cacheDirty )
	{
		// Oper Header File
		FILE *file = fopen( pathOutputGeneratedConfig, "wb" );
		ErrorIf( file == nullptr, "Failed to open file '%s' for writing", pathOutputGeneratedConfig );
		char buffer[512];

		// Header Guard
		swrite( "#pragma once\n\n", file );
		swrite( "/*\n * File generated by boot.exe\n * Refer to: source/boot/boot.cpp\n */\n\n", file );

		// Operating System
		#define MACRO_OPERATING_SYSTEM( macro, value ) \
			strjoin( buffer, "#define " macro " ( ", \
				strcmp( args.os, value ) == 0 ? "1" : "0", " )\n" ); \
				swrite( buffer, file );

		swrite( "// Operating System\n", file );
		MACRO_OPERATING_SYSTEM( "OS_WINDOWS", "windows" );
		MACRO_OPERATING_SYSTEM( "OS_LINUX", "linux" );
		MACRO_OPERATING_SYSTEM( "OS_ANDROID", "android" );
		MACRO_OPERATING_SYSTEM( "OS_MACOS", "macOS" );
		MACRO_OPERATING_SYSTEM( "OS_IOS", "iOS" );
		MACRO_OPERATING_SYSTEM( "OS_IPADOS", "ipadOS" );
		MACRO_OPERATING_SYSTEM( "OS_WEBASM", "Wasm" );

		// Toolchain
		#define MACRO_TOOLCHAIN( macro, value ) \
			strjoin( buffer, "#define " macro " ( ", \
				strcmp( args.toolchain, value ) == 0 ? "1" : "0", " )\n" ); \
				swrite( buffer, file );

		swrite( "\n// Toolchain\n", file );
		MACRO_TOOLCHAIN( "TOOLCHAIN_MSVC", "msvc" );
		MACRO_TOOLCHAIN( "TOOLCHAIN_LLVM", "llvm" );
		MACRO_TOOLCHAIN( "TOOLCHAIN_GNU", "gnu" );

		// Graphics API
		#define MACRO_GRAPHICS_API( macro, value ) \
			strjoin( buffer, "#define " macro " ( ", \
				strcmp( args.gfx, value ) == 0 ? "1" : "0", " )\n" ); \
				swrite( buffer, file );

		swrite( "\n// Graphics Backend\n", file );
		MACRO_GRAPHICS_API( "GRAPHICS_NONE", "none" );
		MACRO_GRAPHICS_API( "GRAPHICS_D3D12", "d3d12" );
		MACRO_GRAPHICS_API( "GRAPHICS_D3D11", "d3d11" );
		MACRO_GRAPHICS_API( "GRAPHICS_OPENGL", "opengl" );
		MACRO_GRAPHICS_API( "GRAPHICS_VULKAN", "vulkan" );
		MACRO_GRAPHICS_API( "GRAPHICS_METAL", "metal" );

		// Architecture
		#define MACRO_ARCHITECTURE( macro, value ) \
			strjoin( buffer, "#define " macro " ( ", \
				strcmp( args.architecture, value ) == 0 ? "1" : "0", " )\n" ); \
				swrite( buffer, file );

		swrite( "\n// Architecture\n", file );
		MACRO_ARCHITECTURE( "ARCHITECTURE_X64", "x64" );
		MACRO_ARCHITECTURE( "ARCHITECTURE_ARM64", "arm64" );

		// Build Info
		#define MACRO_BUILD_INFO( macro, value ) \
			strjoin( buffer, "#define " macro " \"", value, "\"\n" ); swrite( buffer, file );

		swrite( "\n// Build Info\n", file );
		MACRO_BUILD_INFO( "BUILD_PROJECT", args.project );
		MACRO_BUILD_INFO( "BUILD_OS", args.os );
		MACRO_BUILD_INFO( "BUILD_ARCHITECTURE", args.architecture );
		MACRO_BUILD_INFO( "BUILD_TOOLCHAIN", args.toolchain );
		MACRO_BUILD_INFO( "BUILD_CONFIG", args.config );
		MACRO_BUILD_INFO( "BUILD_GRAPHICS", args.gfx );

		// Close Header File
		swrite( "\n", file );
		ErrorIf( fclose( file ) != 0, "Failed to close file '%s'", pathOutputGeneratedConfig );
	}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Gather Source Files

	// projects/<project>/build/*.cpp
	FileList sources_ProjectBuild { 16 };
	directory_iterate( sources_ProjectBuild, pathProjectBuild, ".cpp", true );

	// manta/*.cpp
	FileList sources_Manta { 16 };
	directory_iterate( sources_Manta, pathSourceManta, ".cpp", false );

	// manta/build/*.cpp
	FileList sources_MantaBuild { 16 };
	directory_iterate( sources_MantaBuild, pathSourceMantaBuild, ".cpp", true );

	// manta/vendor/*.cpp
	FileList sources_MantaVendor { 16 };
	directory_iterate( sources_MantaVendor, pathSourceMantaVendor, ".cpp", true );

	// manta/core/*.cpp
	FileList sources_MantaCore { 16 };
	directory_iterate( sources_MantaCore, pathSourceMantaCore, ".cpp", true );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Write Ninja File

	// Open File
	FILE *file = fopen( pathOutputBuildNinja, "wb" );
	ErrorIf( file == nullptr, "Failed to open file '%s' for writing", pathOutputBuildNinja );

	// Write Compile Rule
	{
		const bool toolchainMSVC = ( strcmp( args.toolchain, "msvc" ) == 0 );
		const bool toolchainLLVM = ( strcmp( args.toolchain, "llvm" ) == 0 );
		const bool toolchainGNU = ( strcmp( args.toolchain, "gnu" ) == 0 );

		swrite( "rule compile\n", file );
		swrite( toolchainMSVC ? "  deps = msvc\n" : "  deps = gcc\n  depfile = $out.d\n", file );
		swrite( "  command = ", file );
		swrite( tc.compilerName, file );
		swrite( " $in ", file );
		swrite( tc.compilerOutput, file );
		swrite( "$out ", file );

		// Include Paths
		char compilerFlagsIncludes[1024];

		// root/projects/<project>/output/build -> root/source
		const char *includePath_Manta = ".." SLASH ".." SLASH ".." SLASH ".." SLASH "source";

		// root/projects/<project>/output/build -> root/projects/<project>/output/generated
		const char *includePath_Generated = ".." SLASH "generated";

		// root/projects/<project>/output/build -> root/projects/<project>/build
		const char *includePath_ProjectBuild = ".." SLASH ".." SLASH "build";

		snprintf( compilerFlagsIncludes, sizeof( compilerFlagsIncludes ),
		          tc.compilerFlagsIncludes, includePath_Manta, includePath_Generated, includePath_ProjectBuild );

		// Compiler Flags
		char compilerFlags[1024];
		strjoin( compilerFlags, tc.compilerFlags, " ", tc.compilerFlagsWarnings, " ", compilerFlagsIncludes );
		swrite( compilerFlags, file );
	}

	// Write Link Rule
	{
		swrite( "\n\nrule link\n  command = ", file );
		swrite( tc.linkerName, file );
		swrite( " $in ", file );
		swrite( tc.linkerOutput, file );
		swrite( "$out ", file );

		char linkerFlags[1024];
		strjoin( linkerFlags, tc.linkerFlags );

	#if PIPELINE_OS_WINDOWS
		linkerflags_add_library( linkerFlags, sizeof( linkerFlags ), tc, "winmm" ); // Windows timer
	#endif

		swrite( linkerFlags, file );
		swrite( "\n", file );
	}

	// Build Objects
	{
		// root/projects/<project>/output/build/objects/
		const char *ninjaBuild = "\nbuild objects" SLASH;

		// root/projects/<project>/output/build -> // root/
		const char *ninjaCompile = ": compile " ".." SLASH ".." SLASH ".." SLASH ".." SLASH;

		// projects/<project>/build/*.cpp's
		for( int i = 0; i < sources_ProjectBuild.count; i++ )
		{
			char objFilePath[512];
			path_change_extension( objFilePath, sizeof( objFilePath ),
			                       sources_ProjectBuild.data[i].path, tc.linkerExtensionObj );

			// Write Command
			swrite( ninjaBuild, file );
			swrite( objFilePath, file );
			swrite( ninjaCompile, file );
			swrite( sources_ProjectBuild.data[i].path, file );
		}

		// manta/*.cpp's
		swrite( "\n", file );
		for( int i = 0; i < sources_Manta.count; i++ )
		{
			char objFilePath[512];
			path_change_extension( objFilePath, sizeof( objFilePath ),
			                       sources_Manta.data[i].path, tc.linkerExtensionObj );

			// Write Command
			swrite( ninjaBuild, file );
			swrite( objFilePath, file );
			swrite( ninjaCompile, file );
			swrite( sources_Manta.data[i].path, file );
		}

		// manta/build/*.cpp's
		for( int i = 0; i < sources_MantaBuild.count; i++ )
		{
			char objFilePath[512];
			path_change_extension( objFilePath, sizeof( objFilePath ),
			                       sources_MantaBuild.data[i].path, tc.linkerExtensionObj );

			// Write Command
			swrite( ninjaBuild, file );
			swrite( objFilePath, file );
			swrite( ninjaCompile, file );
			swrite( sources_MantaBuild.data[i].path, file );
		}

		// manta/vendor/*.cpp's
		for( int i = 0; i < sources_MantaVendor.count; i++ )
		{
			char objFilePath[512];
			path_change_extension( objFilePath, sizeof( objFilePath ),
			                       sources_MantaVendor.data[i].path, tc.linkerExtensionObj );

			// Write Command
			swrite( ninjaBuild, file );
			swrite( objFilePath, file );
			swrite( ninjaCompile, file );
			swrite( sources_MantaVendor.data[i].path, file );
		}

		// manta/core/*.cpp's
		for( int i = 0; i < sources_MantaCore.count; i++ )
		{
			char objFilePath[512];
			path_change_extension( objFilePath, sizeof( objFilePath ),
			                       sources_MantaCore.data[i].path, tc.linkerExtensionObj );

			// Write Command
			swrite( ninjaBuild, file );
			swrite( objFilePath, file );
			swrite( ninjaCompile, file );
			swrite( sources_MantaCore.data[i].path, file );
		}
	}

	// Build Executable
	{
		// Write Command
		swrite( "\n\nbuild build", file );
		swrite( tc.linkerExtensionExe, file );
		swrite( ": link ", file );

		// Write Input (projects/<project>)
		for( int i = 0; i < sources_ProjectBuild.count; i++ )
		{
			char objFilePath[512];
			path_change_extension( objFilePath, sizeof( objFilePath ),
			                       sources_ProjectBuild.data[i].path, tc.linkerExtensionObj );

			swrite( "objects" SLASH, file );
			swrite( objFilePath, file );
			swrite( " ", file );
		}

		// Write Input (manta/)
		for( int i = 0; i < sources_Manta.count; i++ )
		{
			char objFilePath[512];
			path_change_extension( objFilePath, sizeof( objFilePath ),
			                       sources_Manta.data[i].path, tc.linkerExtensionObj );

			swrite( "objects" SLASH, file );
			swrite( objFilePath, file );
			swrite( " ", file );
		}

		// Write Input (manta/build)
		for( int i = 0; i < sources_MantaBuild.count; i++ )
		{
			char objFilePath[512];
			path_change_extension( objFilePath, sizeof( objFilePath ),
			                       sources_MantaBuild.data[i].path, tc.linkerExtensionObj );

			swrite( "objects" SLASH, file );
			swrite( objFilePath, file );
			swrite( " ", file );
		}

		// Write Input (manta/vendor)
		for( int i = 0; i < sources_MantaVendor.count; i++ )
		{
			char objFilePath[512];
			path_change_extension( objFilePath, sizeof( objFilePath ),
			                       sources_MantaVendor.data[i].path, tc.linkerExtensionObj );

			swrite( "objects" SLASH, file );
			swrite( objFilePath, file );
			swrite( " ", file );
		}

		// Write Input (manta/core)
		for( int i = 0; i < sources_MantaCore.count; i++ )
		{
			char objFilePath[512];
			path_change_extension( objFilePath, sizeof( objFilePath ),
			                       sources_MantaCore.data[i].path, tc.linkerExtensionObj );

			swrite( "objects" SLASH, file );
			swrite( objFilePath, file );
			swrite( " ", file );
		}
	}

	// Close File
	swrite( "\n", file );
	ErrorIf( fclose( file ) != 0, "Failed to close file '%s'", pathOutputBuildNinja );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Start Ninja

	Print( "\n" );
	const char *ninja = ninja_path();

	// Linux/MacOS: chmod +x <ninja>
#if defined( __linux__ ) || ( defined( __APPLE__ ) && defined( __MACH__ ) )
	char chmod[PATH_SIZE]; snprintf( chmod, sizeof( chmod ), "chmod +x %s", ninja );
	system( chmod );
#endif

	// Run Ninja
	strjoin( commandNinja, ninja, " -C ", pathOutputBuild );
	const int code = system( commandNinja );
	if( code != 0 ) { Error( "ninja failed! Code: %d (%s)", code, commandNinja ); }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Check dirty build.exe

	FileTime bootCacheTime, buildExeTime;
	file_time( pathOutputBootCache, &bootCacheTime );
	file_time( pathOutputBuildExe, &buildExeTime );
	if( file_time_newer( buildExeTime, bootCacheTime ) )
	{
		// If build.exe is new, we need to force clear the build cache
		file_delete( pathOutputBuildCache );
	}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Save Cache

	PipelineCache::save( args, pathOutputBootCache );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Start build.exe

	strappend( commandRun, pathOutputBuildExe );
	for( int i = 1; i < argc; i++ )
	{
		strappend( commandRun, " " );
		strappend( commandRun, argv[i] );
	}
	if( system( commandRun ) != 0 ) { return 1; }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Success

	return 0;
}