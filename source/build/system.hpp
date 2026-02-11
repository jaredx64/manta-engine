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
	#include <vendor/windows.hpp>

	#define SLASH "\\"
	#define SLASH_CHAR '\\'

	#ifdef MAX_PATH
		#define PATH_SIZE ( MAX_PATH )
	#else
		#define PATH_SIZE ( 256 )
	#endif
#else
	#include <vendor/posix.hpp>

	#define SLASH "/"
	#define SLASH_CHAR '/'

	#ifdef PATH_MAX
		#define PATH_SIZE ( PATH_MAX )
	#else
		#define PATH_SIZE ( 256 )
	#endif
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct FileTime
{
#if PIPELINE_OS_WINDOWS
	FILETIME time = FILETIME { 0, 0 };
	u64 as_u64() const
	{
		return ( static_cast<u64>( time.dwHighDateTime ) << 32 ) | static_cast<u64>( time.dwLowDateTime );
	}
#else
	u64 time = 0;
	u64 as_u64() const { return time; }
#endif
};


struct FileInfo
{
	char path[PATH_SIZE];
	char name[PATH_SIZE];
	FileTime time;
};


class File
{
public:
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

public:
	FILE *file = nullptr;
	byte *data = nullptr;
	const char *filepath = "";
	usize size = 0;
	FileTime time;
};


extern bool file_time( const char *path, FileTime *result );
extern bool file_time_newer( const FileTime &a, const FileTime &b );
extern bool file_delete( const char *path );
extern bool file_rename( const char *path, const char *name );
extern bool file_copy( const char *source, const char *destination );

extern bool directory_create( const char *path );
extern bool directory_iterate( List<FileInfo> &list, const char *path, const char *extension, bool recurse );
extern bool directory_delete( const char *path );
extern bool directory_rename( const char *path, const char *name );
extern bool directory_copy( const char *source, const char *destination );


extern void swrite( const char *string, FILE *file );
extern usize fsize( FILE *file );


extern bool path_is_directory( const char *path );
extern bool path_is_file( const char *path );

extern void path_get_directory( char *buffer, usize size, const char *path );
extern void path_get_filename( char *buffer, usize size, const char *path );
extern void path_get_extension( char *buffer, usize size, const char *path );

extern void path_change_extension( char *buffer, usize size, const char *path, const char *extension );

extern void path_remove_extension( char *path, usize size );
extern void path_remove_extension( char *buffer, usize size, const char *path );

extern void path_remove_extensions( char *path, usize size );
extern void path_remove_extensions( char *buffer, usize size, const char *path );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Time
{
	extern bool init();
	extern double value();
	extern u64 seed();
}


class Timer
{
public:
	Timer();

	void start();
	void stop();

	double elapsed_s();
	double elapsed_ms();
	double elapsed_us();

public:
	double timeStart = 0.0;
	double timeEnd = 0.0;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////