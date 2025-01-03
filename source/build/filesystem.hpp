#pragma once

#include <config.hpp>

#include <vendor/stdio.hpp>
#include <vendor/stdlib.hpp>
#include <vendor/stdarg.hpp>
#include <vendor/string.hpp>

#include <core/debug.hpp>
#include <core/memory.hpp>
#include <core/list.hpp>
#include <core/string.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if PIPELINE_OS_WINDOWS
	// Windows
	#include <vendor/windows.hpp>

	#define SLASH "\\"
	#define SLASH_CHAR '\\'

	#ifdef MAX_PATH
	#define PATH_SIZE MAX_PATH
	#else
	#define PATH_SIZE 256
	#endif
#else
	// POSIX
	#include <vendor/posix.hpp>

	#define SLASH "/"
	#define SLASH_CHAR '/'

	#ifdef PATH_MAX
	#define PATH_SIZE PATH_MAX
	#else
	#define PATH_SIZE 256
	#endif
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct FileTime
{
#if PIPELINE_OS_WINDOWS
	// Windows
	FILETIME time;
#else
	// POSIX
	u64 time;
#endif
};


struct FileInfo
{
	char path[PATH_SIZE];
	char name[PATH_SIZE];
	FileTime time;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern bool file_time( const char *path, FileTime *result );
extern bool file_time_newer( const FileTime &a, const FileTime &b );

extern void directory_create( const char *path );
extern bool directory_iterate( List<FileInfo> &list, const char *path, const char *extension, const bool recurse );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern bool path_is_directory( const char *path );
extern bool path_is_file( const char *path );

extern void path_get_directory( char *buffer, const usize size, const char *path );
extern void path_get_filename( char *buffer, const usize size, const char *path );
extern void path_get_extension( char *buffer, const usize size, const char *path );

extern void path_change_extension( char *buffer, const usize size, const char *path, const char *extension );
extern void path_remove_extension( char *path );
extern void path_remove_extensions( char *path );

extern void swrite( const char *string, FILE *file );
extern usize fsize( FILE *file );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class File
{
_PUBLIC:
	File() = default;
	File( const char *path ) { open( path ); }

	bool open( const char *path );
	bool save( const char *path );
	bool close();

	explicit operator bool() const { return file != nullptr && data != nullptr; }

	// Memory Leak Detection
	#if COMPILE_DEBUG
	~File()
	{
		if( Debug::memoryLeakDetection && Debug::exitCode == 0 )
		{
			AssertMsg( data == nullptr,
				"ERROR: Memory leak in File (%p) (size: %.2f kb) (%s)", this, KB( size ), filepath );
			AssertMsg( file == nullptr,
				"ERROR: Opened File but did not close! (%p) (size: %.2f kb) (%s)", this, KB( size ), filepath );
		}
	}
	#endif

_PUBLIC:
	FILE *file = nullptr;
	byte *data = nullptr;
	const char *filepath = "";
	usize size = 0;
	FileTime time;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern bool file_copy( const char *srcPath, const char *dstPath );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////