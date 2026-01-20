#include <manta/filesystem.hpp>

#include <core/string.hpp>
#include <core/list.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static const GUID _GUID_FOLDERID_LocalAppData
	{ 0xF1B32785, 0x6FBA, 0x4FCF, { 0x9D, 0x55, 0x7B, 0x8E, 0x7F, 0x15, 0x70, 0x91 } };

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool file_time( const char *path, FileTime *result )
{
	HANDLE file;
	if( ( file = CreateFileA( path, GENERIC_READ, FILE_SHARE_READ, nullptr,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr ) ) == INVALID_HANDLE_VALUE )
	{
		return false;
	}

	bool success = true;
	if( !GetFileTime( file, nullptr, nullptr, &result->time ) ) { success = false; }

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


bool file_exists( const char *path )
{
	DWORD attrs = GetFileAttributesA( path );
	if( attrs == INVALID_FILE_ATTRIBUTES )
	{
		return false;
	}
	return ( ( attrs & FILE_ATTRIBUTE_DIRECTORY ) == 0 );
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


void file_time_string( u64 time, char *buffer, usize size )
{
	// Convert uint64 time to FILETIME
	FILETIME timeFile;
	timeFile.dwLowDateTime = static_cast<DWORD>( time );
	timeFile.dwHighDateTime = static_cast<DWORD>( time >> 32 );

	// Convert to local time (timezones)
	FILETIME timeLocal;
	FileTimeToLocalFileTime( &timeFile, &timeLocal );

	// Convert to system time
	SYSTEMTIME sysTime;
	FileTimeToSystemTime( &timeLocal, &sysTime );

	// Create String
	const bool AM = sysTime.wHour < 12;
	const int month = sysTime.wMonth;
	const int day = sysTime.wDay;
	const int year = sysTime.wYear;
	int hour = sysTime.wHour % 12;
	if( hour == 0 ) { hour = 12; }
	const int minute = sysTime.wMinute;
	const int second = sysTime.wSecond;
	snprintf( buffer, size, "%02d/%02d/%04d %02d:%02d:%02d %s",
		month, day, year, hour, minute, second, AM ? "AM" : "PM" );
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


bool directory_exists( const char *path )
{
	DWORD attrs = GetFileAttributesA( path );
	if( attrs == INVALID_FILE_ATTRIBUTES )
	{
		return false;
	}
	return ( ( attrs & FILE_ATTRIBUTE_DIRECTORY ) != 0 );
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool path_get_directory_executable( const char *executable )
{
	path_get_directory( EXECUTABLE_DIRECTORY, sizeof( EXECUTABLE_DIRECTORY ), executable );
	return true;
}


bool path_get_directory_application( const char *application )
{
	APPLICATION_DIRECTORY[0] = '\0';

	WCHAR *wide = NULL;
	if( FAILED( SHGetKnownFolderPath( _GUID_FOLDERID_LocalAppData, 0, NULL, &wide ) ) ) { return false; }

	char appdata[PATH_SIZE];
	WideCharToMultiByte( CP_UTF8, 0, wide, -1, appdata, sizeof( appdata ), NULL, NULL );
	CoTaskMemFree( wide );

	char applicationLower[PATH_SIZE];
	snprintf( applicationLower, sizeof( applicationLower ), "%s", application );
	strlower( applicationLower );

	snprintf( APPLICATION_DIRECTORY, sizeof( APPLICATION_DIRECTORY ), "%s" SLASH "%s", appdata, applicationLower );
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////