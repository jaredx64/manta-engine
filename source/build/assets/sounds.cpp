#include <build/assets/sounds.hpp>

#include <core/types.hpp>
#include <core/debug.hpp>
#include <core/list.hpp>

#include <build/build.hpp>
#include <build/assets.hpp>
#include <build/filesystem.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SoundID Sounds::make_new( const Sound &sound )
{
	sounds.add( sound );
	return sounds.size() - 1;
}


void Sounds::gather( const char *path, const bool recurse )
{
	// Gather & Load Sounds
	Timer timer;
	List<FileInfo> files;
	directory_iterate( files, path, ".wav", recurse );
	directory_iterate( files, path, ".ogg", recurse );
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

	char buffer[PATH_SIZE];
	path_get_filename( buffer, sizeof( buffer ), path );

	String filepath = buffer;
	sound.streamed = filepath.contains( ".stream" );
	sound.compressed = filepath.contains( ".ogg" );
	sound.name = filepath.substr( 0, filepath.find( "." ) );

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
	const u32 count = static_cast<u32>( sounds.size() );

	usize voiceSampleDataOffset;
	usize voiceSampleDataSize;
	usize streamSampleDataOffset;
	usize streamSampleDataSize;
	Timer timer;

	// Binary
	{
		voiceSampleDataOffset = binary.tell;
		for( Sound &sound : sounds )
		{
			if( sound.streamed ) { continue; }

			// Write Sound Sample Data
			sound.sampleOffsetBytes = binary.tell - voiceSampleDataOffset;
			sound.sampleCountBytes = sound.sampleDataSize;
			binary.write( sound.sampleData, sound.sampleDataSize );
		}
		voiceSampleDataSize = binary.tell - voiceSampleDataOffset;
		ErrorIf( voiceSampleDataSize & 1, "Sounds: Sample data size is not even!" );

		streamSampleDataOffset = binary.tell;
		for( Sound &sound : sounds )
		{
			if( !sound.streamed ) { continue; }

			// Write Stream Sample Data
			sound.sampleOffsetBytes = binary.tell - streamSampleDataOffset;
			sound.sampleCountBytes = sound.sampleDataSize;
			binary.write( sound.sampleData, sound.sampleDataSize );
		}
		streamSampleDataSize = binary.tell - streamSampleDataOffset;
		ErrorIf( streamSampleDataSize & 1, "Streams: Sample data size is not even!" );
	}

	// Header
	{
		// Group
		assets_group( header );

		// Enums
		header.append( "enum_class_type\n(\n\tSound, u32,\n\n" );
		for( Sound &sound : sounds ) { header.append( "\t" ).append( sound.name ).append( ",\n" ); }
		header.append( "\n\tNull = 0,\n" );
		header.append( ");\n\n" );

		// Struct
		header.append( "namespace Assets { struct SoundEntry; }\n\n" );

		// Table
		header.append( "namespace CoreAssets\n{\n" );
		header.append( "\tconstexpr u32 soundCount = " ).append( count ).append( ";\n" );
		header.append( count > 0 ? "\textern const Assets::SoundEntry sounds[soundCount];\n" :
			"\textern const Assets::SoundEntry *sounds;\n" );

		header.append( "\n" );
		header.append( "\tconstexpr usize voiceSampleDataOffset = " ).append( voiceSampleDataOffset ).append( "ULL;\n" );
		header.append( "\tconstexpr usize voiceSampleDataSize = " ).append( voiceSampleDataSize ).append( "ULL;\n" );
		header.append( "\tconstexpr usize streamSampleDataOffset = " ).append( streamSampleDataOffset ).append( "ULL;\n" );
		header.append( "\tconstexpr usize streamSampleDataSize = " ).append( streamSampleDataSize ).append( "ULL;\n" );
		header.append( "}\n\n" );
	}

	// Source
	{
		// Group
		assets_group( source );
		source.append( "namespace CoreAssets\n{\n" );

		// Table
		if( count > 0 )
		{
			char buffer[PATH_SIZE];
			source.append( "\tconst Assets::SoundEntry sounds[soundCount] =\n\t{\n" );
			for( Sound &sound : sounds )
			{
				snprintf( buffer, PATH_SIZE,
					"\t\t{ %d, %d, %d, %lluULL, %lluULL, DEBUG( \"%s\" ) },\n",
					sound.streamed,
					sound.compressed,
					sound.numChannels,
					sound.sampleOffsetBytes / sizeof( i16 ),
					sound.sampleCountBytes / sizeof( i16 ),
					sound.name.cstr() );

				source.append( buffer );
			}
			source.append( "\t};\n" );
		}
		else
		{
			source.append( "\tconst Assets::SoundEntry *sounds = nullptr;\n" );
		}

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