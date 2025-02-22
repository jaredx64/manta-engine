#pragma once

#include <pipeline.hpp>

#include <core/types.hpp>
#include <core/debug.hpp>

#include <vendor/stdarg.hpp>

#include <build/assets/textures.hpp>
#include <build/assets/sounds.hpp>
#include <build/assets/fonts.hpp>
#include <build/assets/sprites.hpp>

#include <build/arguments.hpp>
#include <build/objects.hpp>
#include <build/toolchains.hpp>
#include <build/filesystem.hpp>
#include <build/time.hpp>

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
	extern char pathOutput[PATH_SIZE];
	extern char pathOutputBoot[PATH_SIZE];
	extern char pathOutputBuild[PATH_SIZE];
	extern char pathOutputGenerated[PATH_SIZE];
	extern char pathOutputGeneratedShaders[PATH_SIZE];
	extern char pathOutputRuntime[PATH_SIZE];
	extern char pathOutputRuntimeLicenses[PATH_SIZE];
	extern char pathOutputRuntimeDistributables[PATH_SIZE];
	extern char pathPackage[PATH_SIZE];
	extern char pathOutputBuildCache[PATH_SIZE];

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

	// Cache
	extern bool cacheDirty;
	extern bool cacheDirtyObjects;
	extern bool cacheDirtyAssets;
	extern bool cacheDirtyShaders;
	extern bool cacheDirtyBinary;
	extern Buffer cacheBufferPrevious;
	extern Buffer cacheBufferCurrent;

	// Compile
	extern List<Source> rcs;
	extern List<Source> sources;
	extern List<String> libraries;
	extern List<String> includeDirectories;

	extern void compile_add_source( const char *pathSrc, const char *pathObj, const char *extensionObj );
	extern u32 compile_add_sources( const char *directory, const bool recurse,
		const char *extensionSrc, const char *extensionObj );

	extern void compile_add_library( const char *library );
	extern void compile_add_include_directory( const char *includePath );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class BuilderCore
{
public:
	// Build
	virtual void build( int argc, char **argv );
	virtual void build_cache();

	// Arguments & Toolchain
	virtual void parse_arguments( int argc, char **argv );

	// Objects
	virtual void objects_gather();
	virtual void objects_cache();
	virtual void objects_parse();
	virtual void objects_write();

	// Assets
	virtual void assets_gather();
	virtual void assets_cache();
	virtual void assets_build();
	virtual void assets_write();

	// Graphics
	virtual void shaders_gather();
	virtual void shaders_cache();
	virtual void shaders_build();
	virtual void shaders_write();

	// Binary
	virtual void binary_cache();
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
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern bool verbose_output();

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////