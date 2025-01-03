#pragma once

#include <config.hpp>

#include <core/types.hpp>
#include <core/debug.hpp>

#include <boot/string.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define FILESYSTEM_WINDOWS ( PIPELINE_OS_WINDOWS )
#define FILESYSTEM_POSIX   ( PIPELINE_OS_LINUX || PIPELINE_OS_MACOS )

#if PIPELINE_OS_WINDOWS
	// Windows
	#include <vendor/windows.hpp>

	#ifdef MAX_PATH
	#define PATH_SIZE MAX_PATH
	#else
	#define PATH_SIZE 256
	#endif
#elif PIPELINE_OS_LINUX || PIPELINE_OS_MACOS
	// POSIX
	#include <vendor/posix.hpp>

	#ifdef PATH_MAX
	#define PATH_SIZE PATH_MAX
	#else
	#define PATH_SIZE 256
	#endif
#else
	static_assert( false, "Unsupported OS" );
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool char_is_slash( const char c )
{
	return ( c == '\\' || c == '/' );
}


inline void path_change_extension( char *buffer, const usize size, const char *path, const char *extension )
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
	strncpy( buffer, path, length );
	buffer[length] = '\0';

	// Append the new extension
	strncat( buffer, extension, size - strlen( buffer ) - 1 );
}


inline void swrite( const char *string, FILE *file )
{
	fwrite( string, strlen( string ), 1, file );
}


inline usize fsize( FILE *file )
{
	fseek( file, 0, SEEK_END );
	usize size = ftell( file );
	fseek( file, 0, SEEK_SET );
	return size;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct FileTime
{
#if FILESYSTEM_WINDOWS
	FILETIME time;
#elif FILESYSTEM_POSIX
	usize time;
#endif
};


struct FileInfo
{
	char path[512];
	FileTime time;
};


struct FileList
{
	FileList( const usize reserve = 1 )
	{
		data = reinterpret_cast<FileInfo *>( malloc( reserve * sizeof( FileInfo ) ) );
		AssertMsg( data != nullptr, "Failed to malloc FileList buffer" );
		capacity = reserve;
		count = 0;
	}

	~FileList()
	{
		Assert( data != nullptr );
		free( data );
	}

	inline FileInfo &push()
	{
		if( count == capacity )
		{
			capacity *= 2;
			data = reinterpret_cast<FileInfo *>( realloc( data, capacity * sizeof( FileInfo ) ) );
			AssertMsg( data != nullptr, "Failed realloc FileInfo buffer on push()" );
		}
		return data[count++];
	}

	// Data & State
	FileInfo *data;
	usize capacity;
	usize count;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if FILESYSTEM_WINDOWS


inline bool file_time( const char *path, FileTime *result )
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


inline bool file_time_newer( const FileTime &a, const FileTime &b )
{
	ULARGE_INTEGER integerA;
	integerA.LowPart  = a.time.dwLowDateTime;
	integerA.HighPart = a.time.dwHighDateTime;

	ULARGE_INTEGER integerB;
	integerB.LowPart  = b.time.dwLowDateTime;
	integerB.HighPart = b.time.dwHighDateTime;

	return integerA.QuadPart > integerB.QuadPart;
}


static bool file_delete( const char *path )
{
	char filePath[PATH_SIZE];
	strjoin( filePath, "." SLASH, path );
	return DeleteFileA( filePath ); // TODO: Unicode Support
}


inline void directory_create( const char *path )
{
	CreateDirectoryA( path, nullptr );
}


static bool directory_delete( const char *path, const bool recurse )
{
	if( recurse )
	{
		char searchPath[PATH_SIZE];
		strjoin( searchPath, "." SLASH, path, SLASH "*" );

		HANDLE hFind;
		WIN32_FIND_DATAA findData;
		hFind = FindFirstFileA( searchPath, &findData );

		if( hFind != INVALID_HANDLE_VALUE )
		{
			do
			{
				if( strcmp( findData.cFileName, "." ) != 0 && strcmp( findData.cFileName, ".." ) != 0 )
				{
					char filePath[PATH_SIZE];
					strjoin( filePath, path, SLASH, findData.cFileName );

					if( findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
					{
						// Delete directory
						if( !directory_delete( filePath, recurse ) ) { return false; }
					}
					else
					{
						// Delete file
						if( !file_delete( filePath ) ) { return false; }
					}
				}
			}
			while( FindNextFileA(hFind, &findData) != 0 );

			FindClose( hFind );
		}
	}

	// Delete directory
    if( !RemoveDirectoryA( path ) ) { return false; }

	// Success
	return true;
}


inline bool directory_iterate( FileList &list, const char *path, const char *extension, const bool recurse )
{
	WIN32_FIND_DATAA findData;
	HANDLE findFile;

	// Find First File
	char buffer[512];
	strjoin( buffer, path, SLASH, "*" );
	if( ( findFile = FindFirstFileA( buffer, &findData ) ) == INVALID_HANDLE_VALUE ) { return false; }

	do
	{
		// Ignore Hidden Directories
		if( findData.cFileName[0] == '.' ) { continue; }

		// Recurse Into Directories
		if( findData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY && recurse )
		{
			strjoin( buffer, path, SLASH, findData.cFileName );
			directory_iterate( list, buffer, extension, recurse );
		}
		// Add File
		else
		{
			// Filter extension
			const int length = static_cast<int>( strlen( findData.cFileName ) );
			const int extensionLength = strlen( extension );

			if( length <= extensionLength || strcmp( findData.cFileName + length - extensionLength, extension ) )
			{
				continue;
			}

			// Add FileInfo
			FileInfo &info = list.push();
			strjoin( info.path, path, SLASH, findData.cFileName );
			info.time.time = findData.ftLastWriteTime;
		}
	} while ( FindNextFileA( findFile, &findData ) );

	// Success
	return true;
}


#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if	FILESYSTEM_POSIX

#include <vendor/posix.hpp>

inline bool file_time( const char *path, FileTime *result )
{
	struct stat file_stat;
	int file = open( path, O_RDONLY);
	if( file == -1 ) { return false; }
	if( fstat( file, &file_stat ) == -1 ) { return false; }

	#if PIPELINE_OS_MACOS
	result->time = static_cast<u64>( file_stat.st_mtime );
	#else
	result->time = static_cast<u64>( file_stat.st_mtim.tv_sec ) * 1000000 + static_cast<u64>( file_stat.st_mtim.tv_nsec ) / 1000;
	#endif

	return true;
}


inline bool file_time_newer( const FileTime &a, const FileTime &b )
{
	return a.time > b.time;
}


static bool file_delete( const char *path )
{
	char filePath[PATH_SIZE];
	strjoin( filePath, "." SLASH, path );
	return ( unlink( filePath ) == 0 );
}


inline void directory_create( const char *path )
{
	char dir[PATH_SIZE];
	strjoin( dir, "." SLASH, path );
	mkdir( dir, 0777 );
}


static bool directory_delete( const char *path, const bool recurse )
{
	if( recurse )
	{
		char searchPath[PATH_SIZE];
		strjoin( searchPath, "." SLASH, path, SLASH "*" );

		DIR *dir;
		struct dirent *entry;
		if( ( dir = opendir( path ) ) != nullptr )
		{
			while( ( entry = readdir( dir ) ) != nullptr )
			{
				if( strcmp( entry->d_name, "." ) != 0 && strcmp( entry->d_name, ".." ) != 0 )
				{
					char filePath[PATH_SIZE];
					strjoin( filePath, path, SLASH, entry->d_name );

					// Determine if the path is a file or directory
					struct stat st;
					if( lstat( filePath, &st ) != 0 ) { return false; }

					if( S_ISDIR( st.st_mode ) )
					{
						// Delete directory
						if( !directory_delete( filePath, recurse ) ) { return false; }
					}
					else
					{
						// Delete file
						if( !file_delete( filePath ) ) { return false; }
					}
				}
			}

			closedir( dir );
		}
	}

	// Delete directory
	if( rmdir( path ) != 0 ) { return false; }

	// Success
	return true;
}


inline bool directory_iterate( FileList &list, const char *path, const char *extension, const bool recurse )
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
			char buffer[512];
			strjoin( buffer, path, SLASH, entry->d_name );
			directory_iterate( list, buffer, extension, recurse );
		}
		// Add File
		else
		{
			// Filter extension
			const int length = strlen( entry->d_name );
			const int extensionLength = strlen( extension );

			if( length <= extensionLength || strcmp( entry->d_name + length - extensionLength, extension ) != 0 )
			{
				continue;
			}

			// Add FileInfo
			FileInfo &info = list.push();
			strjoin( info.path, path, SLASH, entry->d_name );
			file_time( path, &info.time );
		}
	} while ( ( entry = readdir( dir ) ) != nullptr );

	// Success
	closedir( dir );
	return true;
}


#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
	data = reinterpret_cast<byte *>( malloc( size + 1 ) );
	data[size] = '\0';
	if( fread( data, size, 1, file ) < 1 ) { goto cleanup; }

	// Success
	return true;

cleanup:
	fclose( file );
	if( data != nullptr ) { free( data ); }
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

	// Success
	return true;

cleanup:
	fclose( wfile );
	return false;
}


bool File::close()
{
	// Free memory
	if( data != nullptr ) { free( data ); }
	data = nullptr;
	size = 0;

	// Close file
	if( file == nullptr ) { return true; }
	if( fclose( file ) != 0 ) { return false; }
	file = nullptr;
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////