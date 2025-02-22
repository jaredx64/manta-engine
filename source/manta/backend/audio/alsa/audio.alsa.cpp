#include <manta/audio.hpp>

#include <vendor/alsa.hpp>
#include <core/debug.hpp>
#include <core/types.hpp>
#include <core/memory.hpp>

#include <manta/thread.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// The number of channels to output.
#define CHANNELS 2

// The number of samples per second to output.
#define SAMPLE_RATE 44100

// The minimum acceptable sound latency in milliseconds.
#define LATENCY_MS 30

// The minimum size of the shared buffer between us and ALSA.
#define BUFFER_SIZE static_cast<int>( LATENCY_MS * ( SAMPLE_RATE / 1000.0 ) )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static THREAD_FUNCTION ( audio_mixer_thread )
{
    snd_pcm_t *device;
    snd_pcm_hw_params_t *hw_params;
    snd_pcm_sw_params_t *sw_params;

	const char *errorMessage = "";
    unsigned int sample_rate = SAMPLE_RATE;
    snd_pcm_uframes_t buffer_size = BUFFER_SIZE;
    snd_pcm_uframes_t period_size;
    i16 *buffer;

    // Open Default Playback Device
    if( snd_pcm_open( &device, "default", SND_PCM_STREAM_PLAYBACK, 0 ) < 0 )
	{
		errorMessage = "ALSA: Failed to open sound playback device";
		goto error;
	}

    // Setup Hardware Parameters
    if( snd_pcm_hw_params_malloc( &hw_params ) < 0 )
	{
		errorMessage = "ALSA: Failed to malloc hw params";
		goto error;
	}

    if( snd_pcm_hw_params_any( device, hw_params ) < 0 )
	{
		errorMessage = "ALSA: Failed snd_pcm_hw_params_any";
		goto error;
	}

    if( snd_pcm_hw_params_set_access( device, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED ) < 0 )
	{
		errorMessage = "ALSA: Failed set hw params access";
		goto error;
	}

    if( snd_pcm_hw_params_set_format( device, hw_params, SND_PCM_FORMAT_S16_LE ) < 0 )
	{
		errorMessage = "ALSA: Failed set hw params format";
		goto error;
	}

    if( snd_pcm_hw_params_set_channels( device, hw_params, CHANNELS ) < 0 )
	{
		errorMessage = "ALSA: Failed set hw params channels";
		goto error;
	}

    if( snd_pcm_hw_params_set_rate_near( device, hw_params, &sample_rate, nullptr ) < 0 )
	{
		errorMessage = "ALSA: Failed set hw params sample rate";
		goto error;
	}

    if( snd_pcm_hw_params_set_buffer_size_near( device, hw_params, &buffer_size ) < 0 )
	{
		errorMessage = "ALSA: Failed set hw params buffer size";
		goto error;
	}

    // TODO: set_rate_near might change the sample rate
    if( sample_rate != 44100 )
	{
		errorMessage = "ALSA: Failed to get requested sample rate of 44100";
		goto error;
	}

    // Apply Hardware Paramters
    if( snd_pcm_hw_params( device, hw_params ) < 0 )
    {
		errorMessage = "ALSA: Failed apply hw params";
		goto error;
	}

	if( snd_pcm_hw_params_get_period_size( hw_params, &period_size, nullptr ) < 0 )
	{
		errorMessage = "ALSA: Failed to get period size";
		goto error;
	}

#if COMPILE_DEBUG && true
    PrintLn( "PCM name: %s\n", snd_pcm_name( device ) );
    PrintLn( "PCM period size: %llu\n", period_size );
    PrintLn( "PCM buffer size: %llu\n", buffer_size );
#endif

    // Allocate buffer
    buffer = reinterpret_cast<i16 *>( memory_alloc( buffer_size * CHANNELS * sizeof( i16 ) ) );

    // Setup Software Parameters
    if( snd_pcm_sw_params_malloc ( &sw_params ) < 0 )
	{
		errorMessage = "ALSA: Failed to malloc sw params";
		goto error;
	}

    if( snd_pcm_sw_params_current( device, sw_params ) < 0 )
	{
		errorMessage = "ALSA: Failed snd_pcm_sw_params_current";
		goto error;
	}

    if( snd_pcm_sw_params_set_avail_min( device, sw_params, period_size ) < 0 )
	{
		errorMessage = "ALSA: Failed to set sw params period size";
		goto error;
	}

    if( snd_pcm_sw_params_set_start_threshold( device, sw_params, buffer_size - period_size ) < 0 )
	{
		errorMessage = "ALSA: Failed to set sw params start threshold";
		goto error;
	}

    // Apply Software Parameters
    if( snd_pcm_sw_params( device, sw_params ) < 0 )
    {
			errorMessage = "ALSA: Failed to apply sw params";
			goto error;
	}

    // Free Parameters
    snd_pcm_hw_params_free( hw_params );
    snd_pcm_sw_params_free( sw_params );

    // Prepare Device
    if( snd_pcm_prepare( device ) < 0 )
    {
		errorMessage = "ALSA: Failed to prepare device";
		goto error;
	}

    // Mixer Loop
    for( ;; )
    {
        // Wait until the interface is ready for data.
        if( snd_pcm_wait( device, -1 ) < 0 ) { break; }

        // Find out how much space is available for playback data.
        snd_pcm_sframes_t frames;
        if( ( frames = snd_pcm_avail_update( device ) ) < 0 ) { break; }

        if( frames > static_cast<snd_pcm_sframes_t>( buffer_size ) )
		{
			frames = static_cast<snd_pcm_sframes_t>( buffer_size );
		}

		// Mix Audio
        SysAudio::audio_mixer( buffer, static_cast<u32>( frames ) );

        if( snd_pcm_writei( device, buffer, frames ) < 0 ) { break; }
    }

error:
    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SysAudio::init_backend()
{
#if AUDIO_ENABLED
	// Start Mixer Thread
	bool failure = Thread::create( audio_mixer_thread ) == nullptr;
	ErrorReturnIf( failure, false, "ALSA: failed to create mixer thread" );
#endif

	return true;
}


bool SysAudio::free_backend()
{
#if AUDIO_ENABLED
	// ...
#endif

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////