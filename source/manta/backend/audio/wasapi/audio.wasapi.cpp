#include <manta/audio.hpp>

#include <vendor/wasapi.hpp>
#include <core/debug.hpp>
#include <core/types.hpp>

#include <manta/assets.hpp>
#include <manta/thread.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// The GUIDs used for this API.
static const GUID _IID_MMDeviceEnumerator
	{ 0xBCDE0395, 0xE52F, 0x467C, { 0x8E, 0x3D, 0xC4, 0x57, 0x92, 0x91, 0x69, 0x2E } };
static const GUID _IID_IMMDeviceEnumerator
	{ 0xA95664D2, 0x9614, 0x4F35, { 0xA7, 0x46, 0xDE, 0x8D, 0xB6, 0x36, 0x17, 0xE6 } };
static const GUID _IID_IAudioClient
	{ 0x1CB9AD4C, 0xDBFA, 0x4C32, { 0xB1, 0x78, 0xC2, 0xF5, 0x68, 0xA7, 0x03, 0xB2 } };
static const GUID _IID_IAudioRenderClient
	{ 0xF294ACFC, 0x3146, 0x4483, { 0xA7, 0xBF, 0xAD, 0xDC, 0xA7, 0xC2, 0x60, 0xE2 } };

// The number of channels to output.
#define CHANNELS 2

// The number of samples per second to output.
#define SAMPLE_RATE 44100

// The minimum acceptable sound latency in milliseconds.
#define LATENCY_MS 30

// The number of REFERENCE_TIMEs per millisecond.
#define REFTIMES_MS 10000

// The minimum size of the shared buffer between us and WASAPI.
#define BUFFER_SIZE LATENCY_MS * REFTIMES_MS

// The format of the data stream we want to send to WASAPI. Of course, this
// is Microsoft we're talking about here, so this is specified to the API in
// WAV format.
static const WAVEFORMATEX format
{
    .wFormatTag = WAVE_FORMAT_PCM,
    .nChannels = CHANNELS,
    .nSamplesPerSec = SAMPLE_RATE,
    .nAvgBytesPerSec = CHANNELS * sizeof( i16 ) * SAMPLE_RATE,
    .nBlockAlign = CHANNELS * sizeof( i16 ),
    .wBitsPerSample = 16,
    .cbSize = 0,
};

static IAudioClient *client;
static IAudioRenderClient *renderClient;
static HANDLE event;
static UINT32 bufferSize;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static THREAD_FUNCTION( audio_mixer_thread )
{
	// Mixer Loop
	const char *errorMessage = "";
	for( ;; )
	{
		UINT32 padding;
		int frames;
		i16 *output;

		// Wait until WASAPI signals the event to tell us that the buffer is
		// ready to be processed by us
		if( WaitForSingleObject( event, INFINITE ) != 0 )
		{
			errorMessage = "WASAPI: Failed to wait for single object";
			goto error;
		}

		// The "padding" of the buffer is the number of audio frames in the
        // buffer that are currently queued to play by WASAPI. So subtract that
        // from the actual size of the buffer to get the number of audio frames
        // that are not queued (i.e. ones that we can write to).
        if( FAILED( client->GetCurrentPadding( &padding ) ) )
        {
			errorMessage = "WASAPI: Failed get current padding";
			goto error;
		}

		frames = bufferSize - padding;

		// Map Buffer
		if( FAILED( renderClient->GetBuffer( frames, reinterpret_cast<BYTE **>( &output ) ) ) )
		{
			errorMessage = "WASAPI: Failed to map buffer";
			goto error;
		}

		// Mix Audio
		SysAudio::audio_mixer( output, static_cast<u32>( frames ) );

		// Unmap Buffer
		if( FAILED( renderClient->ReleaseBuffer( frames, 0 ) ) )
		{
			errorMessage = "WASAPI: Failed to unmap buffer";
			goto error;
		}
	}

error:
	// TODO: Find new device
	ErrorReturnMsg( 0, "%s", errorMessage );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SysAudio::init_backend()
{
#if AUDIO_ENABLED
	IMMDeviceEnumerator *enumerator;
	IMMDevice *device;

	// Initialize COM
	bool failure = FAILED( CoInitializeEx( nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE ) );
	ErrorReturnIf( failure, false, "WASAPI: Failed to initialize COM" );

	// Create Device Enumerator
	failure = FAILED( CoCreateInstance( _IID_MMDeviceEnumerator, nullptr, CLSCTX_ALL, _IID_IMMDeviceEnumerator,
		                               reinterpret_cast<void **>( &enumerator ) ) );
	ErrorReturnIf( failure, false, "WASAPI: Failed to create device enumerator" );

	// Get Default Endpoint Device
	failure = FAILED( enumerator->GetDefaultAudioEndpoint( eRender, eConsole, &device ) );
	ErrorReturnIf( failure, false, "WASAPI: Failed to get default endpoint device" );

	// Create Audio Client
	failure = FAILED( device->Activate( _IID_IAudioClient, CLSCTX_ALL, nullptr,
		reinterpret_cast<void **>( &client ) ) );
	ErrorReturnIf( failure, false, "WASAPI: Failed to create audio client" );

	// Initialize Audio Client
	failure = FAILED( client->Initialize(
        // Shared mode, so our application plays nice with others.
        AUDCLNT_SHAREMODE_SHARED,

        // Have the Windows audio engine handle a lot of stuff for us,
        // like sample-rate and bit-depth conversion.
        AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM | AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY |

        // Request event-driven processing of the audio buffer.
        AUDCLNT_STREAMFLAGS_EVENTCALLBACK,

        // The minimum size of the shared buffer between us and WASAPI.
        BUFFER_SIZE,

        // This can only be non-zero in exclusive mode.
        0,

        // The format of the audio data we're going to specify in the audio buffer.
        &format,

        // TODO
        nullptr ) );
    ErrorReturnIf( failure, false, "WASAPI: Failed to initialize audio client" );

	// Get Render Client
	failure = FAILED( client->GetService( _IID_IAudioRenderClient,
		reinterpret_cast<void **>( &renderClient ) ) );
	ErrorReturnIf( failure, false, "WASAPI: Failed to get render client" );

	// Get Buffer Size
	failure = FAILED( client->GetBufferSize( &bufferSize ) );
	ErrorReturnIf( failure, false, "WASAPI: Failed to get buffer size" );

	// Create Event
	failure = ( event = CreateEventW( nullptr, false, false, nullptr ) ) == nullptr;
	ErrorReturnIf( failure, false, "WASAPI: Failed to create event" );

	// Set Event Handle
	failure = FAILED( client->SetEventHandle( event ) );
	ErrorReturnIf( failure, false, "WASAPI: Failed to set event handle" );

	// Start Mixer Thread
	failure = Thread::create( audio_mixer_thread ) == nullptr;
	ErrorReturnIf( failure, false, "WASAPI: Failed to create mixer thread" );

	// Start Audio Client
	failure = FAILED( client->Start() );
	ErrorReturnIf( failure, false, "WASAPI: Failed to start audio client" );

	// TODO: print in debug mode only
#if COMPILE_DEBUG && false
	PrintLn( "wasapi buffer size: %d frames (%d bytes, %f milliseconds)\n",
		bufferSize,
		static_cast<int>( bufferSize * sizeof(i16) * CHANNELS ),
		bufferSize * 1000.0 / SAMPLE_RATE );
#endif
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