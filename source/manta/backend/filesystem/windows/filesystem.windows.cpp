#include <manta/filesystem.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

	// Success?
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
	if( hSrc == INVALID_HANDLE_VALUE ) { PrintLn( "fail open src" ); return false; }

	hDst = CreateFileA( destination, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr );
	if( hDst == INVALID_HANDLE_VALUE ) { PrintLn( "fail open dst" ); CloseHandle( hSrc ); return false; }

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
				PrintLn( "fail transfer" );
				return false;
			}
			totalWritten += bytesWritten;
		}
	}

	CloseHandle( hSrc );
	CloseHandle( hDst );
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool directory_create( const char *path )
{
	return CreateDirectoryA( path, nullptr );
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

	FindClose(hFind);

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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////