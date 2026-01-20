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
			compilerFlags = "-c -showIncludes -nologo -std:c++20 -EHsc -MD -DUNICODE -DCOMPILE_ENGINE";
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
			linkerPrefixLibraryPath = "/LIBPATH:";
		}
		// LLVM
		else if( strcmp( args.toolchain, "llvm" ) == 0 )
		{
			compilerName = "clang";
			compilerFlags = "-c -MD -MF $out.d -std=c++20 -fno-exceptions -DUNICODE -DCOMPILE_ENGINE";
			compilerFlagsIncludes = "-I%s";
			compilerFlagsWarnings = "-Wno-unused-variable -Wno-unused-function -Wno-unused-private-field -Wno-delete-non-abstract-non-virtual-dtor -Wno-unused-but-set-variable";
			if( strcmp( args.architecture,   "x64" ) == 0 ) { compilerFlagsArchitecture = "-m64"; } else
			if( strcmp( args.architecture, "arm64" ) == 0 ) { /*compilerFlagsArchitecture = "-target aarch64-linux-gnu";*/ } else
															{ Error( "Unsupported target architecture '%s' for compiler '%s'", args.architecture, args.toolchain ); }
			compilerOutput = "-o ";

			linkerName = "clang++";
			#if PIPELINE_OS_MACOS
				// ...
			#else
				linkerFlags = "-fuse-ld=lld";
			#endif
			linkerOutput = "-o ";

			linkerExtensionExe = PIPELINE_OS_WINDOWS ? ".exe" : "";
			linkerExtensionObj = ".o";
			linkerExtensionLibrary = "";
			linkerPrefixLibrary = "-l";
			linkerPrefixLibraryPath = "-L";
		}
		// GNU
		else if( strcmp( args.toolchain, "gnu" ) == 0 )
		{
			compilerName = "gcc";
			compilerFlags = "-c -MD -MF $out.d -std=c++20 -fno-exceptions -DUNICODE -DCOMPILE_ENGINE";
			compilerFlagsIncludes = "-I%s";
			compilerFlagsWarnings = "-Wno-unused-variable -Wno-unused-function -Wno-unused-private-field -Wno-int-in-bool-context -Wno-unused-but-set-variable -Wno-delete-non-abstract-non-virtual-dtor -Wno-format-truncation";
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
			linkerPrefixLibraryPath = "-L";
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
	const char *linkerPrefixLibraryPath;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static const char *ninja_path()
{
	auto ninja_warning = [&]()
	{
		PrintLn( PrintColor_Yellow, "\nWARNING:\n" );
		PrintLn( PrintColor_Yellow, "Ninja: unrecognized operating system or architecture!" );
		PrintLn( PrintColor_Yellow, "Attempting system installed ninja..." );
		PrintLn( PrintColor_Yellow, "If ninja fails, download: https://github.com/ninja-build/ninja/releases\n" );
	};

	// Windows
	#if defined( _WIN32 ) || defined( _WIN64 )
		#if defined( _M_X64 ) || defined( __x86_64__ ) || defined( __amd64__ )
			// X64
			return ".manta" SLASH "ninja-build" SLASH "windows_x64" SLASH "ninja.exe";
		#elif defined(__aarch64__) || defined(_M_ARM64)
			// ARM64
			return ".manta" SLASH "ninja-build" SLASH "windows_arm64" SLASH "ninja.exe";
		#else
			// Unrecognized architecture: fallback ot system-installed ninja
			ninja_warning();
			return "ninja.exe";
		#endif
	#endif

	// MacOS
	#if defined( __APPLE__ ) && defined( __MACH__ )
		#if defined( _M_X64 ) || defined( __x86_64__ ) || defined( __amd64__ )
			// X64
			return ".manta" SLASH "ninja-build" SLASH "macos" SLASH "ninja";
		#elif defined( __aarch64__ ) || defined( _M_ARM64 )
			// ARM64
			return ".manta" SLASH "ninja-build" SLASH "macos" SLASH "ninja";
		#else
			// Unrecognized architecture: fallback ot system-installed ninja
			ninja_warning();
			return "ninja";
		#endif
	#endif

	// Linux
	#if defined( __linux__ )
		#if defined( _M_X64 ) || defined( __x86_64__ ) || defined( __amd64__ )
			// X64
			return ".manta" SLASH "ninja-build" SLASH "linux_x64" SLASH "ninja";
		#elif defined( __aarch64__ ) || defined( _M_ARM64 )
			// ARM64
			return ".manta" SLASH "ninja-build" SLASH "linux_arm64" SLASH "ninja";
		#else
			// Unrecognized architecture: fallback ot system-installed ninja
			ninja_warning();
			return "ninja";
		#endif
	#endif

	// If reached, unknown OS: fallback to system-installed 'ninja'
	ninja_warning();
	return "ninja";
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////