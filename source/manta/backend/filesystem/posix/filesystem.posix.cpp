#include <manta/filesystem.hpp>

#include <core/string.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool file_time( const char *path, FileTime *result )
{
	struct stat file_stat;
	int file = open( path, O_RDONLY);
	if( file == -1 ) { return false; }
	if( fstat( file, &file_stat ) == -1 ) { return false; }

	#if PIPELINE_OS_MACOS
	result->time = static_cast<u64>( file_stat.st_mtime );
	#else
	result->time = static_cast<u64>( file_stat.st_mtim.tv_sec ) * 1000000 +
	               static_cast<u64>( file_stat.st_mtim.tv_nsec ) / 1000;
	#endif

	return true;
}


bool file_time_newer( const FileTime &a, const FileTime &b )
{
	return a.time > b.time;
}


void directory_create( const char *path )
{
	char dir[PATH_SIZE];
	strjoin( dir, "." SLASH, path );
	mkdir( dir, 0777 );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////