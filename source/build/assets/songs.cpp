#include <build/assets/songs.hpp>

#include <core/types.hpp>
#include <core/debug.hpp>
#include <core/list.hpp>

#include <build/build.hpp>
#include <build/assets.hpp>
#include <build/assets/textures.hpp>
#include <build/filesystem.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Songs::make_new( const Song &song )
{
	songs.add( song );
}


void Songs::gather( const char *path, const bool recurse )
{
	// Gather & Load Songs
	Timer timer;
	List<FileInfo> files;
	directory_iterate( files, path, ".track.wav", recurse );
	for( FileInfo &fileInfo : files ) { load( fileInfo.path ); }

	// Log
	if( verbose_output() )
	{
		const u32 count = files.size();
		PrintColor( LOG_CYAN, TAB TAB "%u song%s found in: %s", count, count == 1 ? "" : "s", path );
		PrintLnColor( LOG_WHITE, " (%.3f ms)", timer.elapsed_ms() );
	}
}


void Songs::load( const char *path )
{
	// Register Song
	Song &song = songs.add( { } );
	song.path = path;

	char filename[PATH_SIZE];
	path_get_filename( filename, sizeof( filename ), path );
	String name = filename;
	song.name = name.substr( 0, name.find( "." ) );

	// Build Cache
	Assets::assetFileCount++;
	if( !Build::cacheDirtyAssets )
	{
		FileTime time;
		file_time( path, &time );
		Build::cacheDirtyAssets |= file_time_newer( time, Assets::timeCache );
	}

	// Read Song File
	Buffer file;
	ErrorIf( !file.load( path ), "Failed to load song file: %s", path );
	const u32 riff = file.read<u32>();
	const u32 riffSize = file.read<u32>();
	const u32 riffType = file.read<u32>();
	const u32 fmt = file.read<u32>();
	const u32 fmtSize = file.read<u32>();
	const u16 fmtType = file.read<u16>();
	const u16 fmtChannels = file.read<u16>();
	const u32 fmtSampleRate = file.read<u32>();
	const u32 fmtAvgBytesPerSec = file.read<u32>();
	const u16 fmtBlockAlign = file.read<u16>();
	const u16 fmtBitsPerSample = file.read<u16>();
	const u32 data = file.read<u32>();
	const u32 dataSize = file.read<u32>();
	ErrorIf( fmtSampleRate != 44100, "Song: WAV format files must be 44.1khz! (%s)", path );
	ErrorIf( fmtBitsPerSample != 16, "Song: WAV format files must be 16-bit! (%s)", path );
	ErrorIf( fmtBlockAlign != 2 * fmtChannels, "Sound: WAV format invalid! (%s)", path );

	// Read Samples
	song.sampleData = reinterpret_cast<byte *>( memory_alloc( dataSize ) );
	song.sampleDataSize = dataSize;
	song.numChannels = fmtChannels;
	void *wavData = file.read_bytes( dataSize );
	ErrorIf( wavData == nullptr, "Song: Failed to read samples (%s)", path );
	memory_copy( song.sampleData, wavData, dataSize );

	// Free buffer
	file.free();
}


void Songs::write()
{
	Buffer &binary = Assets::binary;
	String &header = Assets::header;
	String &source = Assets::source;
	usize songsSampleDataOffset;
	usize songsSampleDataSize;
	Timer timer;

	// Binary
	{
		songsSampleDataOffset = binary.tell;
		for( Song &song : songs )
		{
			// Write Sample Data
			song.sampleDataOffsetBytes = binary.tell;
			binary.write( song.sampleData, song.sampleDataSize );
		}
		songsSampleDataSize = binary.tell - songsSampleDataOffset;
		ErrorIf( songsSampleDataSize & 1, "Songs: Sample data size is not even!" );
	}

	// Header
	{
		// Group
		assets_group( header );

		// Struct
		assets_struct( header,
			"DiskSong",
			"int channels;",
			"usize offset;",
			"usize length;",
			"DEBUG( const char *name );" );

		// Enums
		header.append( "enum\n{\n" );
		for( Song &song : songs )
		{
			header.append( "\t" ).append( song.name ).append( ",\n" );
		}
		header.append( "};\n\n" );


		// Table
		header.append( "namespace Assets\n{\n" );
		header.append( "\tconstexpr u32 songCount = " );
		header.append( static_cast<int>( songs.size() ) ).append( ";\n" );
		header.append( "\textern const DiskSong songs[];\n" );
		header.append( "\tconstexpr usize songSampleDataOffset = " ).append( songsSampleDataOffset ).append( "ULL;\n" );
		header.append( "\tconstexpr usize songSampleDataSize = " ).append( songsSampleDataSize ).append( "ULL;\n" );
		header.append( "}\n\n" );
	}

	// Source
	{
		// Group
		assets_group( source );
		source.append( "namespace Assets\n{\n" );

		// Table
		char buffer[PATH_SIZE];
		source.append( "\tconst DiskSong songs[songCount] =\n\t{\n" );
		for( Song &song : songs )
		{
			snprintf( buffer, PATH_SIZE,
				"\t\t{ %d, %lluULL, %lluULL, DEBUG( \"%s\" ) },\n",
				song.numChannels,
				song.sampleDataOffsetBytes,
				song.sampleDataSize,
				song.name.cstr() );

			source.append( buffer );
		}
		source.append( "\t};\n" );
		source.append( "}\n\n" );
	}

	if( verbose_output() )
	{
		const usize count = songs.size();
		PrintColor( LOG_CYAN, "\t\tWrote %d song%s", count, count == 1 ? "" : "es" );
		PrintLnColor( LOG_WHITE, " (%.3f ms)", timer.elapsed_ms() );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////