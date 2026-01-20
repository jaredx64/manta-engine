#include <manta/filesystem.hpp>

#include <core/string.hpp>
#include <core/list.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool file_time( const char *path, FileTime *result )
{
	struct stat file_stat;
	int file = open( path, O_RDONLY );
	if( file == -1 ) { return false; }
	if( fstat( file, &file_stat ) == -1 ) { close( file ); return false; }
	close( file );

#if PIPELINE_OS_MACOS
	result->time =
		static_cast<u64>( file_stat.st_mtime ) * 1000000000ULL +
		static_cast<u64>( file_stat.st_mtimespec.tv_nsec );
#else
	result->time =
		static_cast<u64>( file_stat.st_mtim.tv_sec ) * 1000000000ULL +
		static_cast<u64>( file_stat.st_mtim.tv_nsec );
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


bool file_exists( const char *path )
{
	struct stat statBuf;
	if( lstat( path, &statBuf ) != 0 )
	{
		return false;
	}
	return S_ISREG( statBuf.st_mode );
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


void file_time_string( u64 time, char *buffer, usize size )
{
	// Convert to struct timespec
	struct timespec ts;
	ts.tv_sec = time / 1000000000;
	ts.tv_nsec = time % 1000000000;

	// Convert to struct tm (local time)
	struct tm localTime;
	localtime_r( &ts.tv_sec, &localTime );

	// Create String
	const bool AM = localTime.tm_hour < 12;
	const int month = localTime.tm_mon + 1;
	const int day = localTime.tm_mday;
	const int year = localTime.tm_year + 1900;
	int hour = localTime.tm_hour % 12;
	if( hour == 0 ) { hour = 12; }
	const int minute = localTime.tm_min;
	const int second = localTime.tm_sec;
	snprintf( buffer, size, "%02d/%02d/%04d %02d:%02d:%02d %s",
		month, day, year, hour, minute, second, AM ? "AM" : "PM" );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool directory_create( const char *path )
{
#if 0
	char dir[PATH_SIZE];
	strjoin( dir, "." SLASH, path );
	return mkdir( dir, 0777 ) == 0;
#else
	if( !path || path[0] == '\0' ) { return false; }
	return mkdir( path, 0777 ) == 0 || errno == EEXIST;
#endif
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


bool directory_exists( const char *path )
{
	struct stat statBuf;
	if( lstat( path, &statBuf ) != 0 )
	{
		return false;
	}
	return S_ISDIR( statBuf.st_mode );
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


bool directory_iterate( List<FileInfo> &list, const char *path, const char *extension, bool recurse )
{
	struct dirent *entry;
	DIR *dir = opendir( path );
	if( !dir ) { return false; }

	// Find first file
	if( ( entry = readdir( dir ) ) == nullptr ) { closedir( dir ); return false; }

	do
	{
		// Ignore hidden directories
		if( entry->d_name[0] == '.' ) { continue; }

		// Recurse Into Directories
		if( entry->d_type == DT_DIR && recurse )
		{
			char subdir[PATH_SIZE];
			strjoin( subdir, path, SLASH, entry->d_name );
			directory_iterate( list, subdir, extension, recurse );
		}
		// Add File
		else
		{
			// Filter extension
			const int length = strlen( entry->d_name );
			const int extensionLength = strlen( extension );
			if( length <= extensionLength ||
				strcmp( entry->d_name + length - extensionLength, extension ) != 0 )
			{
				continue;
			}

			// Add FileInfo
			FileInfo info;
			strjoin( info.path, path, SLASH, entry->d_name );
			strncpy( info.name, entry->d_name, sizeof( info.name ) - 1 );
			info.name[sizeof( info.name ) - 1] = '\0';
			file_time( info.path, &info.time );
			list.add( info );
		}
	} while ( ( entry = readdir( dir ) ) != nullptr );

	closedir( dir );
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool path_get_directory_executable( const char *executable )
{
	path_get_directory( EXECUTABLE_DIRECTORY, sizeof( EXECUTABLE_DIRECTORY ), executable );
	return true;
}


bool path_get_directory_application( const char *application )
{
	APPLICATION_DIRECTORY[0] = '\0';

#if OS_MACOS

	const char *home = getenv( "HOME" );
    if( !home ) { return false; }

	snprintf( APPLICATION_DIRECTORY, sizeof( APPLICATION_DIRECTORY ),
		"%s" SLASH "Library" SLASH "Application Support" SLASH "%s", home, application );

	return true;

#elif OS_LINUX

	const char *home = getenv( "HOME" );
    if( !home ) { return false; }

	char applicationLower[PATH_SIZE];
	snprintf( applicationLower, sizeof( applicationLower ), "%s", application );
	strlower( applicationLower );

	snprintf( APPLICATION_DIRECTORY, sizeof( APPLICATION_DIRECTORY ),
		"%s" SLASH ".local" SLASH "share" SLASH "%s", home, applicationLower );

	return true;

#else

	static_assert( false, "Unsupported operating system!" );
	return false;

#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////