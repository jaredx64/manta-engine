#include <manta/audio.hpp>

#include <AudioToolbox/AudioToolbox.h>

#include <core/types.hpp>
#include <core/debug.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define BUFFERS 2
#define CHANNELS 2
#define SAMPLE_RATE 44100
#define FRAME_SIZE 4
#define LATENCY_MS 30
#define BUFFER_SIZE ( ( static_cast<int>( LATENCY_MS * ( SAMPLE_RATE / 1000.0 ) ) * FRAME_SIZE ) / BUFFERS )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void output_callback( void *data, AudioQueueRef queue, AudioQueueBufferRef buffer )
{
	UInt32 bytes = buffer->mAudioDataBytesCapacity;
	UInt32 frames = bytes / FRAME_SIZE;

	// Let Core Audio know we're gonna be using the entire buffer
    buffer->mAudioDataByteSize = bytes;

	// Mix the audio
	CoreAudio::audio_mixer( reinterpret_cast<i16 *>( buffer->mAudioData ), static_cast<u32>( frames ) );

	// Re-enqueue this buffer
	AudioQueueEnqueueBuffer( queue, buffer, 0, nullptr );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CoreAudio::init_backend()
{
#if AUDIO_ENABLED
	AudioStreamBasicDescription desc;
	AudioQueueRef queue;
    AudioQueueBufferRef buffers[BUFFERS];

	// Setup Stream Description
	desc.mSampleRate = static_cast<Float64>( SAMPLE_RATE );
	desc.mFormatID = kAudioFormatLinearPCM;
	desc.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger | kLinearPCMFormatFlagIsPacked;
	desc.mBytesPerPacket = FRAME_SIZE;
	desc.mFramesPerPacket = 1;
	desc.mBytesPerFrame = FRAME_SIZE;
	desc.mChannelsPerFrame = 2;
	desc.mBitsPerChannel = 16;
	desc.mReserved = 0;

	// Setup Audio Queue
	bool failure = AudioQueueNewOutput( &desc, output_callback, nullptr, nullptr, nullptr, 0, &queue ) != 0;
	ErrorReturnIf( failure, false, "CoreAudio: Failed to setup audio queue" );

	// Setup Audio Buffers
	for( int i = 0; i < BUFFERS; i++ )
	{
		// Allocate Buffer
		failure = AudioQueueAllocateBuffer( queue, BUFFER_SIZE, &buffers[i] ) != 0;
		ErrorReturnIf( failure, false, "CoreAudio: Failed to setup audio buffer %d", i );

	#if COMPILE_DEBUG
		PrintLn( "core audio buffer size [%d]: %d frames\n", i, buffers[i]->mAudioDataBytesCapacity / 4 );
	#endif

		// This will prime the buffer to make sure we have audio data before
		// trying to play anything, and it will also enqueue the buffer to be
		// played for the first time
		output_callback( nullptr, queue, buffers[i] );
	}

	// Start Queue
	failure = AudioQueueStart( queue, nullptr ) != 0;
	ErrorReturnIf( failure, false, "CoreAudio: Failed to start audio queue" );
#endif

	return true;
}


bool CoreAudio::free_backend()
{
#if AUDIO_ENABLED
	// ...
#endif

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////