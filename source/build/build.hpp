#pragma once

#include <pipeline.hpp>

#include <core/types.hpp>
#include <core/debug.hpp>

#include <vendor/stdarg.hpp>

#include <build/assets/textures.hpp>
#include <build/assets/fonts.hpp>
#include <build/assets/sprites.hpp>

#include <build/arguments.hpp>
#include <build/objects.hpp>
#include <build/toolchains.hpp>
#include <build/filesystem.hpp>
#include <build/time.hpp>
#include <build/cache.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Source
{
	Source( const char *srcPath, const char *objPath ) : srcPath( srcPath ), objPath( objPath ) { }
	String srcPath;
	String objPath;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Build
{
	// Paths
	extern char pathEngine[PATH_SIZE];
	extern char pathProject[PATH_SIZE];
	extern char pathPackages[PATH_SIZE];
	extern char pathOutput[PATH_SIZE];
	extern char pathOutputBoot[PATH_SIZE];
	extern char pathOutputBuild[PATH_SIZE];
	extern char pathOutputGenerated[PATH_SIZE];
	extern char pathOutputGeneratedShaders[PATH_SIZE];
	extern char pathOutputRuntime[PATH_SIZE];
	extern char pathOutputRuntimeLicenses[PATH_SIZE];
	extern char pathOutputRuntimeDistributables[PATH_SIZE];
	extern char pathOutputRuntimeExecutable[PATH_SIZE];
	extern char pathOutputRuntimeBinary[PATH_SIZE];
	extern char pathOutputBuildCache[PATH_SIZE];
	extern char pathOutputBuildCacheBuild[PATH_SIZE];
	extern char pathOutputBuildCacheObjects[PATH_SIZE];
	extern char pathOutputBuildCacheGraphics[PATH_SIZE];
	extern char pathOutputBuildCacheAssets[PATH_SIZE];

	// Package
	extern String packageName;
	extern String packageCompany;
	extern String packageVersionA;
	extern String packageVersionB;
	extern String packageVersionC;
	extern String packageVersionD;
	extern String packageIcon;
	extern Texture2DBuffer packageIconBuffer;
	extern Source packageRC;

	// Commands
	extern char commandNinja[1024];
	extern char commandRun[1024];

	// Pipeline
	extern Arguments args;
	extern Toolchain tc;

	// Timing
	extern Timer timer;

	// Binary
	extern String header;
	extern Buffer binary;

	// Compile
	extern List<Source> rcs;
	extern List<Source> sources;
	extern List<String> libraries;
	extern List<String> includeDirectories;

	extern void compile_add_source( const char *pathSrc, const char *pathObj, const char *extensionObj );
	extern usize compile_add_sources( const char *directory, const bool recurse,
		const char *extensionSrc, const char *extensionObj );

	extern void compile_add_library( const char *library );
	extern void compile_add_include_directory( const char *includePath );

	// Stages
	extern void begin();
	extern void end();

	// Cache
	extern Cache cache;
	extern bool buildBinary;

	extern void cache_read( const char *path );
	extern void cache_write( const char *path );
	extern void cache_validate();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class BuilderCore
{
public:
	// Build
	virtual void build( int argc, char **argv );
	virtual void build_cache_validate();

	// Arguments & Toolchain
	virtual void parse_arguments( int argc, char **argv );

	// Objects
	virtual void objects_gather();
	virtual void objects_cache_validate();
	virtual void objects_build();

	// Assets
	virtual void assets_gather();
	virtual void assets_cache_validate();
	virtual void assets_build();

	// Graphics
	virtual void shaders_gather();
	virtual void shaders_cache_validate();
	virtual void shaders_build();

	// Binary
	virtual void binary_cache_validate();
	virtual void binary_write();

	// Compile
	virtual void compile_project();
	virtual void compile_engine();
	virtual void compile_write_ninja();
	virtual void compile_run_ninja();

	// Package
	virtual void package_load();
	virtual void package_generate_rc();

	virtual bool package_windows();
	virtual bool package_linux();
	virtual bool package_macos();

	// Run
	virtual void executable_run( int argc, char **argv );
	virtual void executable_run_gpu_capture( int argc, char **argv );
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern bool verbose_output();

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////