#include <build/filesystem.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool char_is_slash( const char c )
{
	return ( c == '\\' || c == '/' );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if OS_WINDOWS

	bool file_time( const char *path, FileTime *result )
	{
		// Open File
		HANDLE file;
		if( ( file = CreateFileA( path, GENERIC_READ, FILE_SHARE_READ, nullptr,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr ) ) == INVALID_HANDLE_VALUE )
		{
			return false;
		}

		// Get File Time
		bool success = true;
		if( !GetFileTime( file, nullptr, nullptr, &result->time ) ) { success = false; }

		// Close File
		CloseHandle( file );
		return success;
	}


	bool file_time_newer( const FileTime &a, const FileTime &b )
	{
		ULARGE_INTEGER integerA;
		integerA.LowPart  = a.time.dwLowDateTime;
		integerA.HighPart = a.time.dwHighDateTime;

		ULARGE_INTEGER integerB;
		integerB.LowPart  = b.time.dwLowDateTime;
		integerB.HighPart = b.time.dwHighDateTime;

		return integerA.QuadPart > integerB.QuadPart;
	}


	bool file_delete( const char *path )
	{
		return DeleteFileA( path );
	}


	bool file_rename( const char *path, const char *name )
	{
		char pathNew[MAX_PATH];

		const char *lastSlash = strrchr( path, '\\' );
		if( !lastSlash ) { return false; }

		usize dirLength = lastSlash - path + 1;
		if( dirLength >= MAX_PATH ) { return false; }
		strncpy( pathNew, path, dirLength );
		pathNew[dirLength] = '\0';

		if( strlen( pathNew ) + dirLength >= MAX_PATH ) { return false; }
		strcat( pathNew, name );

		return MoveFileA( path, pathNew );
	}


	bool file_copy( const char *source, const char *destination )
	{
		HANDLE hSrc, hDst;
		DWORD bytesRead, bytesWritten, totalWritten;
		char buffer[4096];

		hSrc = CreateFileA( source, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr );
		if( hSrc == INVALID_HANDLE_VALUE ) { return false; }

		hDst = CreateFileA( destination, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr );
		if( hDst == INVALID_HANDLE_VALUE ) { CloseHandle( hSrc ); return false; }

		while( ReadFile( hSrc, buffer, sizeof( buffer ), &bytesRead, nullptr ) && bytesRead > 0 )
		{
			totalWritten = 0;
			while( totalWritten < bytesRead )
			{
				if( !WriteFile( hDst, buffer + totalWritten, bytesRead - totalWritten, &bytesWritten, nullptr ) )
				{
					CloseHandle( hSrc );
					CloseHandle( hDst );
					DeleteFileA( destination );
					return false;
				}
				totalWritten += bytesWritten;
			}
		}

		CloseHandle( hSrc );
		CloseHandle( hDst );
		return true;
	}


	bool directory_create( const char *path )
	{
		return CreateDirectoryA( path, nullptr );
	}


	bool directory_iterate( List<FileInfo> &list, const char *path, const char *extension, bool recurse )
	{
		WIN32_FIND_DATAA findData;
		HANDLE hFind;

		char buffer[PATH_SIZE];
		strjoin( buffer, path, SLASH, "*" );
		if( ( hFind = FindFirstFileA( buffer, &findData ) ) == INVALID_HANDLE_VALUE ) { return false; }

		do
		{
			if( strcmp( findData.cFileName, "." ) == 0 || strcmp( findData.cFileName, ".." ) == 0 )
			{
				continue;
			}

			if( ( findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) && recurse )
			{
				char subdir[PATH_SIZE];
				strjoin( subdir, path, SLASH, findData.cFileName );
				directory_iterate( list, subdir, extension, recurse );
			}
			else
			{
				// Filter extension
				const int length = static_cast<int>( strlen( findData.cFileName ) );
				const int extensionLength = static_cast<int>( strlen( extension ) );
				if( length <= extensionLength ||
					strcmp( findData.cFileName + length - extensionLength, extension ) )
				{
					continue;
				}

				// Add FileInfo
				FileInfo info;
				strjoin( info.path, path, SLASH, findData.cFileName );
				strncpy( info.name, findData.cFileName, sizeof( info.name ) - 1 );
				info.name[sizeof(info.name) - 1] = '\0';
				info.time.time = findData.ftLastWriteTime;
				list.add( info );
			}
		}
		while( FindNextFileA( hFind, &findData ) );

		FindClose( hFind );
		return true;
	}


	bool directory_delete( const char *path )
	{
		WIN32_FIND_DATAA findData;
		char searchPath[MAX_PATH];
		snprintf( searchPath, sizeof( searchPath ), "%s\\*", path );

		HANDLE hFind = FindFirstFileA( searchPath, &findData );
		if (hFind == INVALID_HANDLE_VALUE) { return false; }

		do
		{
			if( strcmp( findData.cFileName, "." ) == 0 || strcmp( findData.cFileName, ".." ) == 0 )
			{
				continue;
			}

			char fullPath[MAX_PATH];
			snprintf(fullPath, sizeof(fullPath), "%s\\%s", path, findData.cFileName);

			if( findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			{
				if( !directory_delete( fullPath ) ) { FindClose( hFind ); return false; }
			}
			else
			{
				if( !file_delete( fullPath ) ) { FindClose( hFind ); return false; }
			}
		}
		while( FindNextFileA( hFind, &findData ) );

		FindClose( hFind );

		if( !RemoveDirectoryA( path ) ) { return false; }
		return true;
	}


	bool directory_rename( const char *path, const char *name )
	{
		return file_rename( path, name );
	}


	bool directory_copy( const char *source, const char *destination )
	{
		WIN32_FIND_DATAA findFileData;
		HANDLE hFind;
		char pathSource[MAX_PATH];
		char pathDestination[MAX_PATH];
		snprintf( pathSource, MAX_PATH, "%s\\*", source );

		hFind = FindFirstFileA( pathSource, &findFileData );
		if( hFind == INVALID_HANDLE_VALUE ) { return false; }

		if( !CreateDirectoryA( destination, nullptr ) ) { FindClose( hFind ); return false; }

		do
		{
			if( strcmp( findFileData.cFileName, "." ) == 0 || strcmp( findFileData.cFileName, ".." ) == 0 )
			{
				continue;
			}

			snprintf( pathSource, MAX_PATH, "%s\\%s", source, findFileData.cFileName );
			snprintf( pathDestination, MAX_PATH, "%s\\%s", destination, findFileData.cFileName );

			if( findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			{
				if( !directory_copy( pathSource, pathDestination ) ) { FindClose( hFind ); return false; }
			}
			else
			{
				if( !file_copy( pathSource, pathDestination ) ) { FindClose( hFind ); return false; }
			}
		}
		while( FindNextFileA(hFind, &findFileData ) );

		FindClose( hFind );
		return true;
	}

#else

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


	bool directory_create( const char *path )
	{
		char dir[PATH_SIZE];
		strjoin( dir, "." SLASH, path );
		return mkdir( dir, 0777 ) == 0;
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

#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool path_is_directory( const char *path )
{
	// Empty path
	if( path == nullptr || path[0] == '\0' ) { return false; }

	// Find the last slash
	const char *lastSlashFw = strrchr( path, '/' );
	const char *lastSlashBk = strrchr( path, '\\' );
	const char *lastSlash = lastSlashFw > lastSlashBk ? lastSlashFw : lastSlashBk;

	// Find last dot
	const char *lastDot = strrchr( path, '.' );

	// Is it a directory?
	return !lastDot || ( lastSlash && lastDot < lastSlashFw );
}


bool path_is_file( const char *path )
{
	return !path_is_directory( path );
}


void path_get_directory( char *buffer, usize size, const char *path )
{
	// Ensure valid strings
	if( buffer == nullptr || path == nullptr ) { return; }

	// Find the last slash
	const char *lastSlash = nullptr;
	for( const char *p = path + strlen( path ) - 1; p >= path; p-- )
	{
		if( char_is_slash( *p ) ) { lastSlash = p; break; }
	}

	// No slash found
	if( lastSlash == nullptr )
	{
		buffer[0] = '\0';
		return;
	}

	// Copy the portion of the path up to the last separator
	usize length = static_cast<usize>( lastSlash - path );
	if( length >= size ) { length = size - 1; }
	strncpy( buffer, path, length );
	buffer[length] = '\0';
}


void path_get_filename( char *buffer, usize size, const char *path )
{
	// Ensure valid strings
	if( buffer == nullptr || path == nullptr || size == 0 ) { return; }
	if( path[0] == '\0' ) { buffer[0] = '\0'; return; }

	// Find the last slash
	const char *lastSlash = nullptr;
	for( const char *p = path + strlen( path ) - 1; p >= path; p-- )
	{
		if( char_is_slash( *p ) ) { lastSlash = p; break; }
	}

	// No slash found, entire path is a filename
	if( lastSlash == nullptr )
	{
		strncpy( buffer, path, size - 1 );
		buffer[size - 1] = '\0';
		return;
	}

	// Copy the portion after the last separator (filename) to the buffer
	const char *filename = lastSlash + 1;
	strncpy( buffer, filename, size - 1 );
	buffer[size - 1] = '\0';
}


void path_get_extension( char *buffer, usize size, const char *path )
{
	// Ensure valid strings
	if( buffer == nullptr || path == nullptr ) { return; }

	// Find the last dot
	const char *lastDot = strrchr( path, '.' );

	// Find the last slash
	const char *lastSlash = nullptr;
	for( const char *p = path + strlen( path ) - 1; p >= path; p-- )
	{
		if( char_is_slash( *p ) ) { lastSlash = p; break; }
	}

    // If there's no dot or the dot appears before the last separator, no extension
    if( lastDot == nullptr || ( lastSlash != nullptr && lastDot < lastSlash ) )
	{
        buffer[0] = '\0';
        return;
    }

    // Copy the extension (including the dot) to the buffer
    strncpy( buffer, lastDot, size - 1 );
    buffer[size - 1] = '\0';
}


void path_change_extension( char *buffer, usize size, const char *path, const char *extension )
{
	// Ensure valid strings
	if( buffer == nullptr || path == nullptr || extension == nullptr ) { return; }

	// Find the last dot
	const char *lastDot = strrchr( path, '.' );

	// Find the last slash
	const char *lastSlash = nullptr;
	for( const char *p = path + strlen( path ) - 1; p >= path; p-- )
	{
		if( char_is_slash( *p ) ) { lastSlash = p; break; }
	}

	// If there's no extension (no period), just copy the path and append the new extension
	if( lastDot == nullptr || ( lastSlash != nullptr && lastDot < lastSlash ) )
	{
		snprintf( buffer, size, "%s%s", path, extension );
		return;
	}

	// Copy the portion of the path before the last dot
	usize length = lastDot - path;
	if( length >= size ) { length = size - 1; }
	memmove( buffer, path, length );
	buffer[length] = '\0';

	// Append the new extension
	strncat( buffer, extension, size - strlen( buffer ) - 1 );
}


void path_remove_extension( char *path, usize size )
{
	if( path == nullptr || size == 0 ) { return; }

	char *extension = nullptr;

	for( usize i = 0; i < size && path[i] != '\0'; i++ )
	{
		if( path[i] == '.' ) { extension = &path[i]; };
	}

	if( extension != nullptr ) { *extension = '\0'; }
}


void path_remove_extensions( char *path, usize size )
{
	if( path == nullptr || size == 0 ) { return; }

	char *extension = nullptr;

	for( usize i = 0; i < size && path[i] != '\0'; i++ )
	{
		if( path[i] == '.' ) { path[i] = '\0'; return; };
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void swrite( const char *string, FILE *file )
{
	fwrite( string, strlen( string ), 1, file );
}


usize fsize( FILE *file )
{
	fseek( file, 0, SEEK_END );
	usize size = ftell( file );
	fseek( file, 0, SEEK_SET );
	return size;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool File::open( const char *path )
{
	// Close current file if one is open
	if( data != nullptr ) { close(); }
	Assert( file == nullptr );

	// Try to open file for reading
	filepath = path;
	file = fopen( filepath, "rb" );
	if( file == nullptr ) { return false; }

	// Get file size
	size = fsize( file );
	if( size == 0 ) { goto cleanup; }

	// Allocate memory memory & read file contents into 'data'
	data = reinterpret_cast<byte *>( memory_alloc( size + 1 ) );
	data[size] = '\0';
	if( fread( data, size, 1, file ) < 1 ) { goto cleanup; }

	return true;
cleanup:
	fclose( file );
	if( data != nullptr ) { memory_free( data ); }
	file = nullptr;
	data = nullptr;
	size = 0;
	return false;
}


bool File::save( const char *path )
{
	// Skip if there is no file open
	if( data == nullptr ) { return false; }
	if( file == nullptr ) { return false; }

	// Open file for writing
	FILE *wfile = fopen( path, "wb" );
	if( wfile == nullptr ) { return false; }

	// Write file
	if( fwrite( data, size, 1, wfile ) < 1 ) { goto cleanup; }

	return true;
cleanup:
	fclose( wfile );
	return false;
}


bool File::close()
{
	// Free memory
	if( data != nullptr ) { memory_free( data ); }
	data = nullptr;
	size = 0;

	// Close file
	if( file == nullptr ) { return true; }
	if( fclose( file ) != 0 ) { return false; }
	file = nullptr;
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////