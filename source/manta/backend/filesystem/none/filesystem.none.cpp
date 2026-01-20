#include <manta/filesystem.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool file_time( const char *path, FileTime *result )
{
	return true;
}


bool file_time_newer( const FileTime &a, const FileTime &b )
{
	return true;
}


bool file_delete( const char *path )
{
	return true;
}


bool file_exists( const char *path )
{
	return true;
}


bool file_rename( const char *path, const char *name )
{
	return true;
}


bool file_copy( const char *source, const char *destination )
{
	return true;
}


void file_time_string( u64 time, char *buffer, usize size )
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool directory_create( const char *path )
{
	return true;
}


bool directory_delete( const char *path )
{
	return true;
}


bool directory_exists( const char *path )
{
	return true;
}


bool directory_rename( const char *path, const char *name )
{
	return true;
}


bool directory_copy( const char *source, const char *destination )
{
	return true;
}


bool directory_iterate( List<FileInfo> &list, const char *path, const char *extension, bool recurse )
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool path_get_directory_executable( const char *executable )
{
	return true;
}


bool path_get_directory_application( const char *application )
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////