#include <build/assets/sounds.hpp>

#include <core/types.hpp>
#include <core/debug.hpp>
#include <core/list.hpp>
#include <core/checksum.hpp>

#include <build/build.hpp>
#include <build/assets.hpp>
#include <build/filesystem.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct CacheSound
{
	usize offset;
	usize size;
	u8 channels;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SoundID Sounds::allocate_new( const Sound &sound )
{
	sounds.add( sound );
	return sounds.size() - 1;
}


usize Sounds::gather( const char *path, bool recurse )
{
	// Gather Sounds
	List<FileInfo> files;
	directory_iterate( files, path, ".wav", recurse );
	directory_iterate( files, path, ".ogg", recurse );

	// Process Sounds
	for( FileInfo &fileInfo : files )
	{
		Assets::cacheFileCount++;
		process( fileInfo.path );
	}

	return files.size();
}


void Sounds::process( const char *path )
{
	// Local Directory
	static char pathDirectory[PATH_SIZE];
	path_get_directory( pathDirectory, sizeof( pathDirectory ), path );

	// Register Definition File
	AssetFile fileDefinition;
	if( !asset_file_register( fileDefinition, path ) )
	{
		Error( "Unable to locate sound file: %s", path );
		return;
	}
	String filepath = fileDefinition.path;

	const bool streamed = filepath.contains( ".stream" );
	const bool compressed = filepath.contains( ".ogg" );

	// Generate Cache ID
	static char cacheIDBuffer[PATH_SIZE * 2];
	memory_set( cacheIDBuffer, 0, sizeof( cacheIDBuffer ) );
	snprintf( cacheIDBuffer, sizeof( cacheIDBuffer ), "sound %s|%llu",
		fileDefinition.path, fileDefinition.time.as_u64() );
	const CacheID cacheID = checksum_xcrc32( cacheIDBuffer, sizeof( cacheIDBuffer ), 0 );

	// Check Cache
	CacheSound cacheSound;
	if( !Assets::cache.fetch( cacheID, cacheSound ) )
	{
		Assets::cache.dirty |= true; // Dirty Cache
	}

	// Register Sound
	Sound &sound = sounds.add( Sound { } );
	sound.cacheID = cacheID;
	sound.path = fileDefinition.path;
	sound.name = fileDefinition.name;
	sound.streamed = streamed;
	sound.compressed = compressed;
}


void Sounds::build()
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

	// Load
	{
		for( Sound &sound : sounds )
		{
			CacheSound cacheSound;
			if( Assets::cache.fetch( sound.cacheID, cacheSound ) )
			{
				// Load from cached binary
				sound.sampleData.write_from_file( Build::pathOutputRuntimeBinary,
					Assets::cacheReadOffset + cacheSound.offset, cacheSound.size );
				sound.numChannels = cacheSound.channels;
				Assets::log_asset_cache( "Sound", sound.name.cstr() );
			}
			else
			{
				// Load from sound file
				Buffer file;
				ErrorIf( !file.load( sound.path ),
					"Failed to load sound file: %s", sound.path.cstr() );
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
				ErrorIf( fmtSampleRate != 44100,
					"Sound: WAV format files must be 44.1khz! (%s)", sound.path.cstr() );
				ErrorIf( fmtBitsPerSample != 16,
					"Sound: WAV format files must be 16-bit! (%s)", sound.path.cstr() );
				ErrorIf( fmtBlockAlign != 2 * fmtChannels,
					"Sound: WAV format invalid! (%s)", sound.path.cstr() );

				// Read Samples
				sound.sampleData.write( file.read_bytes( dataSize ), dataSize );
				sound.numChannels = fmtChannels;
				Assets::log_asset_build( "Sound", sound.name.cstr() );
			}
		}
	}

	// Binary
	{
		voiceSampleDataOffset = binary.size();
		for( Sound &sound : sounds )
		{
			if( sound.streamed ) { continue; }

			// Write Sound Sample Data
			const usize binaryOffset = binary.write( sound.sampleData.data, sound.sampleData.size() );
			sound.sampleOffsetBytes = binaryOffset - voiceSampleDataOffset;
			sound.sampleCountBytes = sound.sampleData.size();

			// Cache
			CacheSound cacheSound;
			cacheSound.offset = binaryOffset;
			cacheSound.size = sound.sampleData.size();
			cacheSound.channels = sound.numChannels;
			Assets::cache.store( sound.cacheID, cacheSound );
		}
		voiceSampleDataSize = binary.size() - voiceSampleDataOffset;
		ErrorIf( voiceSampleDataSize & 1, "Sounds: Sample data size is not even!" );

		streamSampleDataOffset = binary.size();
		for( Sound &sound : sounds )
		{
			if( !sound.streamed ) { continue; }

			// Write Stream Sample Data
			const usize binaryOffset = binary.write( sound.sampleData.data, sound.sampleData.size() );
			sound.sampleOffsetBytes = binaryOffset - streamSampleDataOffset;
			sound.sampleCountBytes = sound.sampleData.size();

			// Cache
			CacheSound cacheSound;
			cacheSound.offset = binaryOffset;
			cacheSound.size = sound.sampleData.size();
			cacheSound.channels = sound.numChannels;
			Assets::cache.store( sound.cacheID, cacheSound );
		}
		streamSampleDataSize = binary.size() - streamSampleDataOffset;
		ErrorIf( streamSampleDataSize & 1, "Streams: Sample data size is not even!" );
	}

	// Header
	{
		// Group
		assets_group( header );

		// Enums
		header.append( "enum_class\n(\n\tSound, u32,\n\n" );
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
		header.append( "\tconstexpr usize voiceSampleDataOffset = BINARY_OFFSET_ASSETS + " );
		header.append( voiceSampleDataOffset ).append( "LLU;\n" );
		header.append( "\tconstexpr usize voiceSampleDataSize = " );
		header.append( voiceSampleDataSize ).append( "LLU;\n" );
		header.append( "\tconstexpr usize streamSampleDataOffset = BINARY_OFFSET_ASSETS + " );
		header.append( streamSampleDataOffset ).append( "LLU;\n" );
		header.append( "\tconstexpr usize streamSampleDataSize = " );
		header.append( streamSampleDataSize ).append( "LLU;\n" );
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
					"\t\t{ %d, %d, %d, %lluLLU, %lluLLU, DEBUG( \"%s\" ) },\n",
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
		Print( PrintColor_White, TAB TAB "Wrote %d sound%s", count, count == 1 ? "" : "s" );
		PrintLn( PrintColor_White, " (%.3f ms)", timer.elapsed_ms() );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////