#pragma once

#include <config.hpp>
#include <core/debug.hpp>
#include <boot/string.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Toolchain
{
	Toolchain( const char *toolchain )
	{
		// MSVC
		if( strcmp( toolchain, "msvc" ) == 0 )
		{
#if 0
			compilerName = "cl";
			compilerFlags = "-c -O2 -showIncludes -nologo -std:c++20 -EHsc -DUNICODE -DCOMPILE_BUILD -DCOMPILE_ASSERTS";
			compilerFlagsIncludes = "-I\"%s\" -I\"%s\" -I\"%s\"";
			compilerFlagsWarnings = "-W4 -wd4100 -wd4101 -wd4189 -wd4201 -wd4244 -wd4456 -wd4458 -wd4459 -wd4505 -wd4702 -wd4996";
			compilerOutput = "-Fo:";

			linkerName = "link";
			linkerFlags = "-nologo -DEBUG ";
			linkerOutput = "-out:";
#else
			// ASAN
			compilerName = "cl";
			compilerFlags = "-c -fsanitize=address -Od -Z7 -showIncludes -nologo -std:c++20 -EHsc -DUNICODE -DCOMPILE_BUILD -DCOMPILE_ASSERTS";
			compilerFlagsIncludes = "-I\"%s\" -I\"%s\" -I\"%s\"";
			compilerFlagsWarnings = "-W4 -wd4100 -wd4101 -wd4189 -wd4201 -wd4244 -wd4456 -wd4458 -wd4459 -wd4505 -wd4702 -wd4996";
			compilerOutput = "-Fo:";
			linkerName = "link";
			linkerFlags = "-nologo -DEBUG ";
			linkerOutput = "-out:";
#endif
			linkerExtensionExe = PIPELINE_OS_WINDOWS ? ".exe" : "";
			linkerExtensionObj = ".obj";
			linkerExtensionLibrary = ".lib";
			linkerPrefixLibrary = "";
		}
		else if( strcmp( toolchain, "gnu" ) == 0 )
		// GNU
		{
			compilerName = "gcc";
			compilerFlags = "-c -O2 -MD -MF $out.d -std=c++20 -fno-exceptions -DUNICODE -DCOMPILE_BUILD -I%s -I%s -I%s";
			compilerFlagsIncludes = "-I%s -I%s -I%s";
			compilerFlagsWarnings = "-Wall -Wno-unused-variable -Wno-unused-function -Wno-format-truncation";
			compilerOutput = "-o ";

			linkerName = "g++";
			linkerFlags = "";
			linkerOutput = "-o ";

			linkerExtensionExe = PIPELINE_OS_WINDOWS ? ".exe" : "";
			linkerExtensionObj = ".o";
			linkerExtensionLibrary = "";
			linkerPrefixLibrary = "-l";
		}
		else if( strcmp( toolchain, "llvm" ) == 0 )
		// LLVM
		{
			compilerName = "clang";
			compilerFlags = "-c -O2 -MD -MF $out.d -std=c++20 -fno-exceptions -DUNICODE -DCOMPILE_BUILD -I%s -I%s -I%s";
			compilerFlagsIncludes = "-I%s -I%s -I%s";
			compilerFlagsWarnings = "-Wall -Wno-unused-variable -Wno-unused-function";
			compilerOutput = "-o ";

			linkerName = "clang++";
			linkerFlags = "";
			linkerOutput = "-o ";

			linkerExtensionExe = PIPELINE_OS_WINDOWS ? ".exe" : "";
			linkerExtensionObj = ".o";
			linkerExtensionLibrary = "";
			linkerPrefixLibrary = "-l";
		}
		else
		{
			Error( "Unsupported toolchain: %s", toolchain );
		}
	}

	const char *compilerName;
	const char *compilerFlags;
	const char *compilerFlagsIncludes;
	const char *compilerFlagsWarnings;
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

static void linkerflags_add_library( char *buffer, const usize size, const Toolchain &tc, const char *library )
{
	CoreText::macro_strappend( size, buffer, tc.linkerPrefixLibrary );
	CoreText::macro_strappend( size, buffer, library );
	CoreText::macro_strappend( size, buffer, tc.linkerExtensionLibrary );
	CoreText::macro_strappend( size, buffer, " " );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static const char *ninja_path()
{
	auto ninja_warning = [&]()
	{
		PrintLnColor( LOG_YELLOW, "\nWARNING:\n" );
		PrintLnColor( LOG_YELLOW, "Ninja: unrecognized operating system or architecture!" );
		PrintLnColor( LOG_YELLOW, "Attempting system installed ninja..." );
		PrintLnColor( LOG_YELLOW, "If ninja fails, download: https://github.com/ninja-build/ninja/releases\n" );
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
