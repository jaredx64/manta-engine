#if 0
#include <manta/audio.hpp>

#include <core/math.hpp>

#include <manta/thread.hpp>
#include <manta/filesystem.hpp>
#include <manta/assets.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace SysAudio
{
	Voice g_AUDIO_VOICES[ENGINE_SOUND_VOICE_LIMIT];
	AudioStream g_AUDIO_STREAMS[ENGINE_SOUND_STREAM_LIMIT];
	i16 g_AUDIO_BUFFERS[ENGINE_SOUND_STREAM_LIMIT][ENGINE_SOUND_STREAM_BUFFERS][ENGINE_SOUND_STREAM_BLOCK * 2];
	i16 *g_AUDIO_SAMPLES = nullptr;
}


bool SysAudio::init_samples()
{
	ErrorReturnIf( g_AUDIO_SAMPLES != nullptr, false, "Audio: samples buffer already initialized" );
	if constexpr ( Assets::soundSampleDataSize == 0 ) { return true; }

	g_AUDIO_SAMPLES = reinterpret_cast<i16 *>( memory_alloc( Assets::soundSampleDataSize ) );
	memory_copy( g_AUDIO_SAMPLES, &Assets::binary.data[Assets::soundSampleDataOffset], Assets::soundSampleDataSize );

	return true;
}


bool SysAudio::free_samples()
{
	if( g_AUDIO_SAMPLES == nullptr ) { return true; }
	memory_free( g_AUDIO_SAMPLES );
	g_AUDIO_SAMPLES = nullptr;
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//static File file; // TODO: Stream from file

static THREAD_FUNCTION( audio_stream )
{
	PrintLn( "Audio Streaming Started" );

	for( ;; )
	{
		// For each stream
		for( int i = 0; i < ENGINE_SOUND_STREAM_LIMIT; i++ )
		{
			SysAudio::AudioStream &stream = SysAudio::g_AUDIO_STREAMS[i];
			DiskSong const *song = stream.song;

			// This stream isn't playing anything
			if( song == nullptr ) { continue; }

			// For each buffer
			for( int j = 0; j < ENGINE_SOUND_STREAM_BUFFERS; j++ )
			{
				// This buffer is already filled and queued for mixing
				if( stream.ready & ( 1 << j ) ) { continue; }

				// Read the samples into the buffer
				usize frames = min( static_cast<usize>( ENGINE_SOUND_STREAM_BLOCK ),
				                    song->length / 4 - stream.streamPosition );
				usize difference = ENGINE_SOUND_STREAM_BLOCK - frames;

			// TODO: Stream from file
			#if 1
				memory_copy( SysAudio::g_AUDIO_BUFFERS[i][j],
				             &Assets::binary.data[song->offset + stream.streamPosition * 4],
				             frames * 4 );
			#else
				file_seek( file, song->offset + stream.streamPosition * 4 );
				file_read( file, g_AUDIO_BUFFERS[i][j], frames * 4 );
			#endif

				if( difference != 0 )
				{
			#if 1
					memory_copy( SysAudio::g_AUDIO_BUFFERS[i][j],
								&Assets::binary.data[song->offset],
								difference * 4 );
			#else
					file_seek( file, song->offset );
					file_read( file, g_AUDIO_BUFFERS[i][j], difference * 4 );
			#endif
					stream.streamPosition = difference;
				}
				else
				{
					stream.streamPosition += ENGINE_SOUND_STREAM_BLOCK;
				}

				// TODO: Do we need to put up a memory fence here?
				stream.ready |= ( 1 << j );
			}
		}

		// This thread does not have hard latency requirements at all: on a 5400rpm
		// hard drive, it fills up a 16k streaming buffer in about a hundredth of a
		// millisecond. A 16k streaming buffer itself contains about 92 milliseconds
		// worth of sound, so sleeping here could theoretically end up taking about
		// 9x longer than we specified (totally unrealistic) and it *still* wouldn't
		// matter. If the OS is underscheduling us that bad, there's probably a good
		// reason for it, and the user would notice that before they notice an audio
		// buffer underrun. - @mollycoddled member of sega
		Thread::sleep( 10 );
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SysAudio::audio_stream_init()
{
	// TODO: Stream from file
	// ...

	// Start Thread
	bool failure = Thread::create( audio_stream ) == nullptr;
	ErrorReturnIf( failure, false, "AUDIO: Failed to start audio stream thread" );

	// Success
	return true;
}


void SysAudio::audio_mixer( i16 *output, u32 frames )
{
	// Clear Output
	memory_set( output, 0, frames * 2 * sizeof( i16 ) );

	// Mix Streams
	for( int i = 0; i < ENGINE_SOUND_STREAM_LIMIT; i++ )
	{
		AudioStream &stream = g_AUDIO_STREAMS[i];
		DiskSong const *song = stream.song;

		// This stream isn't playing anything
		if( stream.song == nullptr ) { continue; }

		// This stream's buffer isn't ready yet
		if( ( stream.ready & ( 1ULL << ( stream.position / ENGINE_SOUND_STREAM_BLOCK % ENGINE_SOUND_STREAM_BUFFERS ) ) ) == 0 )
		{
			continue;
		}

		// Stereo Mix
		for( u32 j = 0; j < frames; j++ )
		{
			usize absolute = stream.position + j;
			usize position = absolute % ENGINE_SOUND_STREAM_BLOCK;
			usize buffer = absolute / ENGINE_SOUND_STREAM_BLOCK % ENGINE_SOUND_STREAM_BUFFERS;

			// Mix
			output[j * 2] += g_AUDIO_BUFFERS[i][buffer][position * 2 ];
			output[j * 2 + 1] += g_AUDIO_BUFFERS[i][buffer][position * 2 + 1];

			// TODO: mfence?
			if( position == ENGINE_SOUND_STREAM_BLOCK - 1 ) { stream.ready &= ~( 1 << buffer ); }
		}

		// Advance Stream
		stream.position += frames;
	}

	// Mix Voices
	for( int i = 0; i < ENGINE_SOUND_VOICE_LIMIT; i++ )
	{
		Voice &voice = g_AUDIO_VOICES[i];
		DiskSound const *sound = voice.sound;

		// This voice isn't playing anything
		if( voice.sound == nullptr ) { continue; }

		i16 const *samples = &g_AUDIO_SAMPLES[sound->sampleOffset];
		const usize length = sound->sampleCount;
		const usize position = static_cast<usize>( voice.position );
		const usize remaining = length - position;
		const usize to_mix = min( static_cast<usize>( frames ), remaining );

		for( usize j = 0; j < to_mix; j++ )
		{
			// Stereo Mix
			output[j * 2] += static_cast<i16>( samples[position + j] * voice.gain );
			output[j * 2 + 1] += static_cast<i16>( samples[position + j] * voice.gain );
		}

		voice.position += frames;

		// Stop playing sound
		if( voice.position >= length ) { voice.sound = nullptr; }
	}
}


static void audio_play_sound( const DiskSound &sound, float pitch, float gain )
{
	for( int i = 0; i < ENGINE_SOUND_VOICE_LIMIT; i++ )
	{
		SysAudio::Voice &voice = SysAudio::g_AUDIO_VOICES[i];

		if( voice.sound == nullptr )
		{
			voice.position = 0.0f;
			voice.pitch = pitch;
			voice.gain = gain;

			// NOTE: Make sure other threads see this last by putting up a memory fence
			voice.sound = &sound;
			// voice.sound.store( &sound, std::memory_order_release );
			return;
		}
	}
}


void SysAudio::audio_play_sound( const u32 sound, float pitch, float gain )
{
	AssertMsg( sound < Assets::soundCount, "%u, %u", sound, Assets::soundCount );
	audio_play_sound( Assets::sounds[sound], pitch, gain );
}


static void audio_play_song( const DiskSong &song )
{
	for( int i = 0; i < ENGINE_SOUND_STREAM_LIMIT; i++ )
	{
		SysAudio::AudioStream &stream = SysAudio::g_AUDIO_STREAMS[i];

		if( stream.song == nullptr )
		{
			stream.position = 0;
			stream.streamPosition = 0;
			stream.ready = 0;

			// NOTE: Make sure other threads see this last by putting up a memory fence
			stream.song = &song;
			// stream.song.store( &song, std::memory_order_release );
			return;
		}
	}
}


void SysAudio::audio_play_song( const u32 song )
{
	Assert( song < Assets::songCount );
	audio_play_song( Assets::songs[song] );
}


#if COMPILE_DEBUG
#include <manta/draw.hpp>

static void draw_progress_bar( const int x, const int y, const int width, const int height,
                               const float percent, Color colorBackground, Color colorBar )
{
	draw_rectangle( x, y, x + width, y + height, colorBackground );
	draw_rectangle( x + 1, y + 1, x + 1 + static_cast<int>( ( width - 2 ) * percent ), y + height - 1, colorBar );
}


static void audio_time_string( char *buffer, const usize size,
	                           const float percent, const usize sampleCount, const int channels )
{
	// Total Time
	char timeTotal[32];
	const usize secondsTotal = sampleCount / ENGINE_SOUND_FREQUENCY / sizeof( i16 ) / channels;
	{
		const u32 minutes = static_cast<u32>( secondsTotal / 60 );
		const u32 seconds = static_cast<u32>( secondsTotal % 60 );
		snprintf( timeTotal, sizeof( timeTotal ), seconds < 10 ? "%u:0%u" : "%u:%u", minutes, seconds );
	}

	// Current Time
	char timeCurrent[32];
	const usize secondsCurrent = static_cast<usize>( secondsTotal * percent );
	{
		const u32 minutes = static_cast<u32>( secondsCurrent / 60 );
		const u32 seconds = static_cast<u32>( secondsCurrent % 60 );
		snprintf( timeCurrent, sizeof( timeCurrent ), seconds < 10 ? "%u:0%u" : "%u:%u", minutes, seconds );
	}

	// Output buffer
	snprintf( buffer, size, "%s/%s", timeCurrent, timeTotal );
}


void SysAudio::draw( const Delta delta, const int x, const int y )
{
	char buffer[256];
	int dx = x;
	int dy = y;

	// Songs
	{
		for( int i = 0; i < ENGINE_SOUND_STREAM_LIMIT; i++ )
		{
			AudioStream &stream = g_AUDIO_STREAMS[i];
			DiskSong const *song = stream.song;
			const bool playing = !( stream.song == nullptr );
			const int widgetWidth = 96;
			const int widgetHeight = 16;

			// Song Label
			draw_text_f( fnt_iosevka, 10, dx, dy, playing ? c_lime : c_white, "Song %d:", i );
			dx += 64;
			// Voice Widget
			if( playing )
			{
				// Progress Bar
				const float percent = ( stream.position % ( song->length / 4 ) /
					static_cast<float>( song->length / 4 ) );
				draw_progress_bar( dx, dy - 3, widgetWidth, widgetHeight, percent, c_dkgray, c_white );

				// Duration Label
				char time[64];
				audio_time_string( time, sizeof( time ), percent, song->length, 2 );
				draw_text( fnt_iosevka, 10, dx + 5, dy, c_black, time );
				draw_text( fnt_iosevka, 10, dx + 4, dy, c_red, time );

				// Streaming Buffers
				const int border = 2;
				const int widgetWidthBuffer = widgetWidth / ENGINE_SOUND_STREAM_BUFFERS;
				const Color colors[] = { Color { 185, 95, 95 }, Color { 145, 245, 240 } };
				for( int j = 0; j < ENGINE_SOUND_STREAM_BUFFERS; j++ )
				{
					const bool ready  = stream.ready & ( 1 << j );
					const int offset = border + widgetWidth + j * widgetWidthBuffer;
					const float percentBuffer = ( stream.position % ENGINE_SOUND_STREAM_BLOCK ) /
						static_cast<float>( ENGINE_SOUND_STREAM_BLOCK );
					draw_progress_bar( dx + offset, dy - 3, widgetWidthBuffer, widgetHeight, 1.0, c_dkgray, colors[ready] );
					draw_line( dx + offset + 1 + static_cast<int>( ( widgetWidthBuffer - 2 ) * percentBuffer ),
							dy - 3,
							dx + offset + 1 + static_cast<int>( ( widgetWidthBuffer - 2 ) * percentBuffer ),
							dy - 3 + widgetHeight, c_red );
				}

				// Info Label
				draw_text_f( fnt_iosevka, 10, dx + border + widgetWidth * 2 + 4, dy, c_white, "%s", song->name );
			}
			else
			// Off Label
			{
				draw_text( fnt_iosevka, 10, dx, dy, c_dkgray, "<off>" );
			}

			dx -= 64;
			dy += 24;
		}
	}

	// Voices
	{
		dy += 16;
		for( int i = 0; i < ENGINE_SOUND_VOICE_LIMIT; i++ )
		{
			const Voice *voice = &g_AUDIO_VOICES[i];
			const DiskSound *sound = voice->sound;
			const bool playing = !( voice->sound == nullptr );
			const int widgetWidth = 96;
			const int widgetHeight = 16;

			// Voice Label
			draw_text_f( fnt_iosevka, 10, dx, dy, playing ? c_lime : c_white, "Voice %d:", i );
			dx += 72;

			// Sound Widget
			if( playing )
			{
				// Progress Bar
				const float percent = ( voice->position / sound->sampleCount );
				draw_progress_bar( dx, dy - 3, widgetWidth, widgetHeight, percent, c_dkgray, c_white );

				// Duration Label
				char time[64];
				audio_time_string( time, sizeof( time ), percent, sound->sampleCount * sizeof( i16 ), 1 );
				draw_text( fnt_iosevka, 10, dx + 5, dy, c_black, time );
				draw_text( fnt_iosevka, 10, dx + 4, dy, c_red, time );

				// Info Label
				draw_text_f( fnt_iosevka, 10, dx + widgetWidth + 4, dy, c_white,
					"[G: %.2f, P: %.2f] %s", voice->gain, voice->pitch, sound->name );
			}
			else
			// Off Label
			{
				draw_text( fnt_iosevka, 10, dx, dy, c_dkgray, "<off>" );
			}

			dx -= 72;
			dy += 24;
		}
	}
}
#endif

#endif