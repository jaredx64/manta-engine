#include <manta/filesystem.hpp>

#include <core/string.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool file_time( const char *path, FileTime *result )
{
	struct stat file_stat;
	int file = open( path, O_RDONLY );
	if( file == -1 ) { return false; }
	if( fstat( file, &file_stat ) == -1 ) { close( file ); return false; }
	close( file );

#if PIPELINE_OS_MACOS
	// Use st_mtime (seconds). st_birthtime is not reliably portable.
	result->time = static_cast<u64>( file_stat.st_mtime );
#else
	// Use st_mtim (nanoseconds precision)
	result->time = static_cast<u64>( file_stat.st_mtim.tv_sec ) * 1000000 +
		static_cast<u64>( file_stat.st_mtim.tv_nsec ) / 1000;
#endif

	return true;
}


bool file_time_newer( const FileTime &a, const FileTime &b )
{
	return a.time > b.time;
}


bool file_delete( const char *path )
{
	return ( unlink( path ) == 0 );
}


bool file_rename( const char *path, const char *name )
{
	char pathSrc[PATH_SIZE];
	snprintf( pathSrc, sizeof( pathSrc ), "%s", path );
	char pathNew[PATH_SIZE];

	char *dir = dirname( pathSrc );
	if( snprintf( pathNew, PATH_SIZE, "%s/%s", dir, name ) >= PATH_SIZE ) { return false; }

	int result = rename( path, pathNew );
	return ( result == 0 );
}


bool file_copy( const char *source, const char *destination )
{
	int fdSrc, fdDst;
	i64 bytesRead, bytesWritten, totalWritten;
	char buffer[4096];

	fdSrc = open( source, O_RDONLY );
	if( fdSrc < 0 ) { return false; }

	struct stat srcStat;
	if( fstat( fdSrc, &srcStat ) < 0 )
	{
		close( fdSrc );
		return false;
	}

	fdDst = open( destination, O_WRONLY | O_CREAT | O_TRUNC, srcStat.st_mode );
	if( fdDst < 0 )
	{
		close( fdSrc );
		return false;
	}

	while( ( bytesRead = read( fdSrc, buffer, sizeof( buffer ) ) ) > 0 )
	{
		totalWritten = 0;
		while( totalWritten < bytesRead )
		{
			bytesWritten = write( fdDst, buffer + totalWritten, bytesRead - totalWritten );
			if( bytesWritten < 0 )
			{
				close( fdSrc );
				close( fdDst );
				unlink( destination );
				return false;
			}
			totalWritten += bytesWritten;
		}
	}

	close( fdSrc );
	close( fdDst );
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool directory_create( const char *path )
{
	char dir[PATH_SIZE];
	strjoin( dir, "." SLASH, path );
	return mkdir( dir, 0777 ) == 0;
}


bool directory_delete( const char *path )
{
	struct dirent *entry;
	DIR *dir = opendir( path );
	if( !dir ) { return false; }

	while( ( entry = readdir( dir ) ) != NULL )
	{
		if( strcmp( entry->d_name, "." ) == 0 || strcmp( entry->d_name, ".." ) == 0 )
		{
			continue;
		}

		char fullPath[PATH_SIZE];
		snprintf( fullPath, sizeof( fullPath ), "%s/%s", path, entry->d_name );

		struct stat statBuf;
		if( lstat( fullPath, &statBuf ) != 0 )
		{
			closedir( dir );
			return false;
		}

		if( S_ISDIR( statBuf.st_mode ) )
		{
			if( !directory_delete( fullPath ) ) { closedir(dir); return false; }
		}
		else
		{
			if( !file_delete( fullPath ) ) { closedir(dir); return false; }
		}
	}

	closedir(dir);

	if( rmdir( path ) != 0 ) { return false; }
	return true;
}


bool directory_rename( const char *path, const char *name )
{
	return file_rename( path, name );
}


bool directory_copy( const char *source, const char *destination )
{
	struct stat statBuf;
	DIR *dir;
	struct dirent *entry;

	// Get source directory information
	if( lstat( source, &statBuf ) != 0 ) { return false; }

	// Create the destination directory
	if( mkdir( destination, statBuf.st_mode ) != 0 )
	{
		//if( errno != EEXIST ) { return false; }
		return false;
	}

	// Open the source directory
	dir = opendir( source );
	if( !dir ) { return false; }

	// Traverse the directory
	while( ( entry = readdir( dir ) ) != NULL )
	{
		// Skip "." and ".."
		if( strcmp( entry->d_name, "." ) == 0 || strcmp( entry->d_name, ".." ) == 0 )
		{
			continue;
		}

		char pathSource[PATH_SIZE];
		char pathDestination[PATH_SIZE];

		snprintf( pathSource, PATH_SIZE, "%s/%s", source, entry->d_name );
		snprintf( pathDestination, PATH_SIZE, "%s/%s", destination, entry->d_name );

		if( lstat( pathSource, &statBuf ) == 0 )
		{
			if( S_ISDIR( statBuf.st_mode ) )
			{
				if( !directory_copy( pathSource, pathDestination ) ) { closedir( dir ); return false; }
			}
			else if( S_ISREG( statBuf.st_mode ) )
			{
				if( !file_copy( pathSource, pathDestination ) ) { closedir( dir ); return false; }
			}
		}
		else
		{
			closedir( dir );
			return false;
		}
	}

	closedir( dir );
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////