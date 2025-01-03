#include <build/assets/sounds.hpp>

#include <core/types.hpp>
#include <core/debug.hpp>
#include <core/list.hpp>

#include <build/build.hpp>
#include <build/assets.hpp>
#include <build/assets/textures.hpp>
#include <build/filesystem.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Sounds::make_new( const Sound &sound )
{
	sounds.add( sound );
}


void Sounds::gather( const char *path, const bool recurse )
{
	// Gather & Load Sounds
	Timer timer;
	List<FileInfo> files;
	directory_iterate( files, path, ".sound.wav", recurse );
	for( FileInfo &fileInfo : files ) { load( fileInfo.path ); }

	// Log
	if( verbose_output() )
	{
		const u32 count = files.size();
		PrintColor( LOG_CYAN, TAB TAB "%u sound%s found in: %s", count, count == 1 ? "" : "s", path );
		PrintLnColor( LOG_WHITE, " (%.3f ms)", timer.elapsed_ms() );
	}
}


void Sounds::load( const char *path )
{
	// Register Sound
	Sound &sound = sounds.add( { } );
	sound.path = path;

	char filename[PATH_SIZE];
	path_get_filename( filename, sizeof( filename ), path );
	String name = filename;
	sound.name = name.substr( 0, name.find( "." ) );

	// Build Cache
	Assets::assetFileCount++;
	if( !Build::cacheDirtyAssets )
	{
		FileTime time;
		file_time( path, &time );
		Build::cacheDirtyAssets |= file_time_newer( time, Assets::timeCache );
	}

	// Read Sound File
	Buffer file;
	ErrorIf( !file.load( path ), "Failed to load sound file: %s", path );
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
	ErrorIf( fmtSampleRate != 44100, "Sound: WAV format files must be 44.1khz! (%s)", path );
	ErrorIf( fmtBitsPerSample != 16, "Sound: WAV format files must be 16-bit! (%s)", path );
	ErrorIf( fmtBlockAlign != 2 * fmtChannels, "Sound: WAV format invalid! (%s)", path );

	// Read Samples
	sound.sampleData = reinterpret_cast<byte *>( memory_alloc( dataSize ) );
	sound.sampleDataSize = dataSize;
	sound.numChannels = fmtChannels;
	void *wavData = file.read_bytes( dataSize );
	ErrorIf( wavData == nullptr, "Sound: Failed to read samples (%s)", path );
	memory_copy( sound.sampleData, wavData, dataSize );

	// Free buffer
	file.free();
}


void Sounds::write()
{
	Buffer &binary = Assets::binary;
	String &header = Assets::header;
	String &source = Assets::source;
	usize soundsSampleDataOffset;
	usize soundsSampleDataSize;
	Timer timer;

	// Binary
	{
		soundsSampleDataOffset = binary.tell;
		for( Sound &sound : sounds )
		{
			// Write Sample Data
			sound.sampleOffsetBytes = binary.tell - soundsSampleDataOffset;
			sound.sampleCountBytes = sound.sampleDataSize;
			binary.write( sound.sampleData, sound.sampleDataSize );
		}
		soundsSampleDataSize = binary.tell - soundsSampleDataOffset;
		ErrorIf( soundsSampleDataSize & 1, "Sounds: Sample data size is not even!" );
	}

	// Header
	{
		// Group
		assets_group( header );

		// Struct
		assets_struct( header,
			"DiskSound",
			"int channels;",
			"usize sampleOffset;",
			"usize sampleCount;",
			"DEBUG( const char *name );" );

		// Enums
		header.append( "enum\n{\n" );
		for( Sound &sound : sounds )
		{
			header.append( "\t" ).append( sound.name ).append( ",\n" );
		}
		header.append( "};\n\n" );

		// Table
		header.append( "namespace Assets\n{\n" );
		header.append( "\tconstexpr u32 soundCount = " );
		header.append( static_cast<int>( sounds.size() ) ).append( ";\n" );
		header.append( "\textern const DiskSound sounds[];\n" );
		header.append( "\tconstexpr usize soundSampleDataOffset = " ).append( soundsSampleDataOffset ).append( "ULL;\n" );
		header.append( "\tconstexpr usize soundSampleDataSize = " ).append( soundsSampleDataSize ).append( "ULL;\n" );
		header.append( "}\n\n" );
	}

	// Source
	{
		// Group
		assets_group( source );
		source.append( "namespace Assets\n{\n" );

		// Table
		char buffer[PATH_SIZE];
		source.append( "\tconst DiskSound sounds[soundCount] =\n\t{\n" );
		for( Sound &sound : sounds )
		{
			snprintf( buffer, PATH_SIZE,
				"\t\t{ %d, %lluULL, %lluULL, DEBUG( \"%s\" ) },\n",
				sound.numChannels,
				sound.sampleOffsetBytes / sizeof( i16 ),
				sound.sampleCountBytes / sizeof( i16 ),
				sound.name.cstr() );

			source.append( buffer );
		}
		source.append( "\t};\n" );
		source.append( "}\n\n" );
	}

	if( verbose_output() )
	{
		const usize count = sounds.size();
		PrintColor( LOG_CYAN, "\t\tWrote %d sound%s", count, count == 1 ? "" : "es" );
		PrintLnColor( LOG_WHITE, " (%.3f ms)", timer.elapsed_ms() );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////