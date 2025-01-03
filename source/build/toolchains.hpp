#pragma once

#include <core/debug.hpp>
#include <vendor/string.hpp>
#include <build/arguments.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Toolchain
{
	void detect( Arguments &args )
	{
		// MSVC
		if( strcmp( args.toolchain, "msvc" ) == 0 )
		{
			compilerName = "cl";
			compilerFlags = "-c -showIncludes -nologo -std:c++20 -EHsc -DUNICODE -DCOMPILE_ENGINE";
			compilerFlagsIncludes = "-I\"%s\"";
			compilerFlagsWarnings = "-wd4100 -wd4101 -wd4189 -wd4201 -wd4244 -wd4324 -wd4456 -wd4458 -wd4459 -wd4505 -wd4702 -wd4804 -wd4996";
			if( strcmp( args.architecture,   "x64" ) == 0 ) { compilerFlagsArchitecture = ""; } else
			if( strcmp( args.architecture, "arm64" ) == 0 ) { compilerFlagsArchitecture = "-arch:ARM64"; } else
															{ Error( "Unsupported target architecture '%s' for compiler '%s'", args.architecture, args.toolchain ); }
			compilerOutput = "-Fo:";

			linkerName = "link";
			linkerFlags = "-nologo";
			linkerOutput = "-out:";

			linkerExtensionExe = PIPELINE_OS_WINDOWS ? ".exe" : "";
			linkerExtensionObj = ".obj";
			linkerExtensionLibrary = ".lib";
			linkerPrefixLibrary = "";
		}
		// LLVM
		else if( strcmp( args.toolchain, "llvm" ) == 0 )
		{
			compilerName = "clang";
			compilerFlags = "-c -MD -MF $out.d -std=c++20 -fno-exceptions -DUNICODE -DCOMPILE_ENGINE";
			compilerFlagsIncludes = "-I%s";
			compilerFlagsWarnings = "-Wno-unused-variable -Wno-unused-function -Wno-unused-private-field -Wno-delete-non-abstract-non-virtual-dtor -Wno-unused-but-set-variable";
			if( strcmp( args.architecture,   "x64" ) == 0 ) { compilerFlagsArchitecture = "-m64"; } else
			if( strcmp( args.architecture, "arm64" ) == 0 ) { compilerFlagsArchitecture = "-target aarch64-linux-gnu"; } else
															{ Error( "Unsupported target architecture '%s' for compiler '%s'", args.architecture, args.toolchain ); }
			compilerOutput = "-o ";

			linkerName = "clang++";
			linkerFlags = "-fuse-ld=lld";
			linkerOutput = "-o ";

			linkerExtensionExe = PIPELINE_OS_WINDOWS ? ".exe" : "";
			linkerExtensionObj = ".o";
			linkerExtensionLibrary = "";
			linkerPrefixLibrary = "-l";
		}
		// GNU
		else if( strcmp( args.toolchain, "gnu" ) == 0 )
		{
			compilerName = "gcc";
			compilerFlags = "-c -MD -MF $out.d -std=c++20 -fno-exceptions -DUNICODE -DCOMPILE_ENGINE";
			compilerFlagsIncludes = "-I%s";
			compilerFlagsWarnings = "-Wno-unused-variable -Wno-unused-function -Wno-unused-private-field -Wno-int-in-bool-context -Wno-unused-but-set-variable -Wno-delete-non-abstract-non-virtual-dtor";
			if( strcmp( args.architecture,   "x64" ) == 0 ) { compilerFlagsArchitecture = "-m64"; } else
			if( strcmp( args.architecture, "arm64" ) == 0 ) { compilerFlagsArchitecture = "-march=armv8-a"; } else
															{ Error( "Unsupported target architecture '%s' for compiler '%s'", args.architecture, args.toolchain ); }
			compilerOutput = "-o ";

			linkerName = "g++";
			linkerFlags = "";
			linkerOutput = "-o ";

			linkerExtensionExe = PIPELINE_OS_WINDOWS ? ".exe" : "";
			linkerExtensionObj = ".o";
			linkerExtensionLibrary = "";
			linkerPrefixLibrary = "-l";
		}
		else
		{
			Error( "Unsupported toolchain! '%s'", args.toolchain );
		}
	}

	const char *compilerName;
	const char *compilerFlags;
	const char *compilerFlagsIncludes;
	const char *compilerFlagsWarnings;
	const char *compilerFlagsArchitecture;
	const char *compilerOutput;

	const char *linkerName;
	const char *linkerFlags;
	const char *linkerOutput;

	const char *linkerExtensionExe;
	const char *linkerExtensionObj;
	const char *linkerExtensionLibrary;
	const char *linkerPrefixLibrary;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////