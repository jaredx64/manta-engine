#include <manta/audio.hpp>

#include <vendor/new.hpp>

#include <core/debug.hpp>
#include <core/math.hpp>

#include <manta/thread.hpp>
#include <manta/assets.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// https://github.com/sinshu/freeverb

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define I16_TO_FLOAT(value) ( ( value ) * ( 1.0f / I16_MAX ) )
#define FLOAT_TO_I16(value) static_cast<i16>( ( value ) * I16_MAX )

#define undenormalise( sample ) \
	if( ( ( *reinterpret_cast<unsigned int *>( &sample ) ) & 0x7f800000 ) == 0 ) { sample = 0.0f; }

static AudioEffects NULL_AUDIO_EFFECTS;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace SysAudio
{
	Voice voices[AUDIO_VOICE_COUNT];
	Bus buses[AUDIO_BUS_COUNT];
	i16 *g_AUDIO_SAMPLES = nullptr; // TODO: refactor
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float SysAudio::Effect::get_parameter( const EffectParam param, const bool incrementTime )
{
	const float valueFrom = parameters[param].valueFrom;
	const float valueTo = parameters[param].valueTo;
	const float amount = parameters[param].sampleCurrent * parameters[param].sampleToInv;
	const float value = amount < 1.0f ? lerp( valueFrom, valueTo, amount ) : valueTo;
	parameters[param].sampleCurrent += static_cast<float>( amount < 1.0f && incrementTime );
	return value;
}


void SysAudio::Effect::set_parameter( const EffectParam param, const float value )
{
	parameters[param].valueFrom = value;
	parameters[param].valueTo = value;
	parameters[param].sampleCurrent = 0.0f;
	parameters[param].sampleToInv = 1.0f;
}


void SysAudio::Effect::set_parameter( const EffectParam param, const float value, const usize timeMS )
{
	parameters[param].valueFrom = get_parameter( param, false );
	parameters[param].valueTo = value;
	parameters[param].sampleCurrent = 0.0f;
	constexpr float msToHz = ( 44100.0f / 1000.0f );
	parameters[param].sampleToInv = 1.0f / ( timeMS * msToHz );
}


void SysAudio::Effect::set_parameter( const EffectParam param, const float v0, const float v1, const usize timeMS )
{
	parameters[param].valueFrom = v0;
	parameters[param].valueTo = v1;
	parameters[param].sampleCurrent = 0.0f;
	constexpr float msToHz = ( 44100.0f / 1000.0f );
	parameters[param].sampleToInv = 1.0f / ( timeMS * msToHz );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// EffectType_Core

static void core_init( SysAudio::Effect &effect )
{
	effect.set_parameter( SysAudio::EffectParam_Core_Gain, 1.0f );
	effect.set_parameter( SysAudio::EffectParam_Core_Pitch, 1.0f );
}


static void core_apply( SysAudio::Effect &effect, float *samples, u32 sampleCount )
{
	SysAudio::EffectStateCore &core = effect.stateGain;

	for( u32 i = 0; i < sampleCount; i++ )
	{
		const float volume = effect.get_parameter( SysAudio::EffectParam_Core_Gain, true );
		samples[i * 2 + 0] *= volume;
		samples[i * 2 + 1] *= volume;
	}
}

__AUDIO_EFFECT_PARAM_GET_SET_IMPL( SysAudio::EffectType_Core, gain, SysAudio::EffectParam_Core_Gain )
__AUDIO_EFFECT_PARAM_GET_SET_NOAUTOMATION_IMPL( SysAudio::EffectType_Core, pitch, SysAudio::EffectParam_Core_Pitch )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// EffectType_Lowpass

static void lowpass_update( SysAudio::Effect &effect )
{
	SysAudio::EffectStateLowpass &lowpass = effect.stateLowpass;

	// Parameters
	const float paramCutoff = effect.get_parameter( SysAudio::EffectParam_Lowpass_Cutoff, true );
	lowpass.omega0 = 6.28318530718f * paramCutoff;

	// Internal State
	float alpha = lowpass.omega0 * lowpass.dt;
#if LPF_ORDER == 1
	lowpass.a[0] = -( alpha - 2.0f ) / ( alpha + 2.0f );
	lowpass.b[0] = alpha / (alpha + 2.0f);
	lowpass.b[1] = alpha / (alpha + 2.0f);
#elif LPF_ORDER == 2
	float alphaSq = alpha * alpha;
	float beta[] = { 1.0f, 1.41421356237f /* sqrtf(2.0f) */, 1.0f };
	float D = alphaSq * beta[0] + 2.0f * alpha * beta[1] + 4.0f * beta[2];

	lowpass.b[0] = alphaSq / D;
	lowpass.b[1] = 2.0f * lowpass.b[0];
	lowpass.b[2] = lowpass.b[0];
	lowpass.a[0] = -( 2.0f * alphaSq * beta[0] - 8.0f * beta[2] ) / D;
	lowpass.a[1] = -( beta[0] * alphaSq - 2.0f * beta[1] * alpha + 4.0f * beta[2] ) / D;
#endif
}


static void lowpass_init( SysAudio::Effect &effect )
{
	SysAudio::EffectStateLowpass &lowpass = effect.stateLowpass;
	effect.set_parameter( SysAudio::EffectParam_Lowpass_Cutoff, 20000.0f );
	static float sampleRate = 44100.0f;
	lowpass.dt = 1.0f / sampleRate;
	lowpass_update( effect );
}


static void lowpass_apply( SysAudio::Effect &effect, float *samples, u32 sampleCount )
{
	SysAudio::EffectStateLowpass &lowpass = effect.stateLowpass;

	for( u32 i = 0; i < sampleCount; i++ )
	{
		// Update coefficients
		lowpass_update( effect );

		lowpass.y[0] = 0;
		lowpass.x[0] = samples[i * 2 + 0];

		// Compute the filtered values
		for( int j = 0; j < LPF_ORDER; j++ )
		{
			lowpass.y[0] += lowpass.a[j] * lowpass.y[j + 1] + lowpass.b[j] * lowpass.x[j];
		}

		lowpass.y[0] += lowpass.b[LPF_ORDER] * lowpass.x[LPF_ORDER];

		// Save the historical values
		for( int j = LPF_ORDER; j > 0; j--)
		{
			lowpass.y[j] = lowpass.y[j - 1];
			lowpass.x[j] = lowpass.x[j - 1];
		}

		// return lpf.x[0];
		samples[i * 2 + 0] = lowpass.y[0];
		samples[i * 2 + 1] = lowpass.y[0];
	}
}


__AUDIO_EFFECT_PARAM_GET_SET_IMPL( SysAudio::EffectType_Lowpass, lowpass_cutoff, SysAudio::EffectParam_Lowpass_Cutoff )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// EffectType_Reverb

static void reverb_comb_mute( SysAudio::EffectStateReverbComb &comb )
{
	for( int i = 0; i < comb.bufsize; i++ ) { comb.buffer[i] = 0.0f; }
}


static void reverb_comb_set_buffer( SysAudio::EffectStateReverbComb &comb, float *buffer, int size )
{
	comb.buffer = buffer;
	comb.bufsize = size;
}


static void reverb_comb_set_damp( SysAudio::EffectStateReverbComb &comb, float val )
{
	comb.damp1 = val;
	comb.damp2 = 1.0f - val;
}


static float reverb_comb_process( SysAudio::EffectStateReverbComb &comb, float input )
{
	float output;

	output = comb.buffer[comb.bufidx];
	undenormalise( output );

	comb.filterstore = (output * comb.damp2) + ( comb.filterstore * comb.damp1 );
	undenormalise( comb.filterstore );

	comb.buffer[comb.bufidx] = input + ( comb.filterstore * comb.feedback );

	if( ++comb.bufidx >= comb.bufsize ) { comb.bufidx = 0; }

	return output;
}


static void reverb_allpass_set_buffer( SysAudio::EffectStateReverbAllPass &allpass, float *buffer, int size )
{
	allpass.buffer = buffer;
	allpass.bufsize = size;
}


static void reverb_allpass_mute( SysAudio::EffectStateReverbAllPass &allpass)
{
	for( int i = 0; i < allpass.bufsize; i++ ) { allpass.buffer[i] = 0.0f; }
}


static inline float reverb_allpass_process( SysAudio::EffectStateReverbAllPass &allpass, float input )
{
	float output;
	float bufout;

	bufout = allpass.buffer[allpass.bufidx];
	undenormalise( bufout );

	output = -input + bufout;
	allpass.buffer[allpass.bufidx] = input + ( bufout * allpass.feedback );

	if( ++allpass.bufidx >= allpass.bufsize ) { allpass.bufidx = 0; }

	return output;
}


static void reverb_mute( SysAudio::Effect &effect )
{
	SysAudio::EffectStateReverb &reverb = effect.stateReverb;

	// Mute Combs
	for( int i = 0; i < SysAudioTuning::numCombs; i++ )
	{
		reverb_comb_mute( reverb.combL[i] );
		reverb_comb_mute( reverb.combR[i] );
	}

	// Mute All-passes
	for( int i = 0; i < SysAudioTuning::numAllpasses; i++ )
	{
		reverb_allpass_mute( reverb.allpassL[i] );
		reverb_allpass_mute( reverb.allpassR[i] );
	}
}


static void reverb_update( SysAudio::Effect &effect )
{
	SysAudio::EffectStateReverb &reverb = effect.stateReverb;

	// Parameters
	const float paramRoomsize = effect.get_parameter( SysAudio::EffectParam_Reverb_RoomSize, true );
	const float paramWet = effect.get_parameter( SysAudio::EffectParam_Reverb_Wet, true );
	const float paramDry = effect.get_parameter( SysAudio::EffectParam_Reverb_Dry, true );
	const float paramDamp = effect.get_parameter( SysAudio::EffectParam_Reverb_Damp, true );
	const float paramWidth = effect.get_parameter( SysAudio::EffectParam_Reverb_Width, true );

	// Recalculate internal values after parameter change (TODO: Refactor?)
	reverb.roomsize = paramRoomsize * SysAudioTuning::scaleRoom + SysAudioTuning::offsetRoom; // TODO
	reverb.wet = paramWet * SysAudioTuning::scaleWet; // TODO
	reverb.wet1 = reverb.wet * ( reverb.width * 0.5f + 0.5f );
	reverb.wet2 = reverb.wet * ( ( 1.0f - reverb.width ) * 0.5f );
	reverb.dry = paramDry * SysAudioTuning::scaleDry; // TODO
	reverb.damp = paramDry * SysAudioTuning::scaleDamp; // TODO
	reverb.width = paramWidth; // TODO
	reverb.dry = paramDry; // TODO

	reverb.roomsize1 = reverb.roomsize;
	reverb.damp1 = reverb.damp;
	reverb.gain = SysAudioTuning::fixedGain;

	for( int i = 0; i < SysAudioTuning::numCombs; i++ )
	{
		reverb.combL[i].feedback = reverb.roomsize1;
		reverb_comb_set_damp( reverb.combL[i], reverb.damp1 );
		reverb.combR[i].feedback = reverb.roomsize1;
		reverb_comb_set_damp( reverb.combR[i], reverb.damp1 );
	}
}


static void reverb_init( SysAudio::Effect &effect )
{
	SysAudio::EffectStateReverb &reverb = effect.stateReverb;
	{
		// Tie the components to their buffers
		reverb_comb_set_buffer( reverb.combL[0], reverb.bufcombL1, SysAudioTuning::combTuningL1 );
		reverb_comb_set_buffer( reverb.combR[0], reverb.bufcombR1, SysAudioTuning::combTuningR1 );
		reverb_comb_set_buffer( reverb.combL[1], reverb.bufcombL2, SysAudioTuning::combTuningL2 );
		reverb_comb_set_buffer( reverb.combR[1], reverb.bufcombR2, SysAudioTuning::combTuningR2 );
		reverb_comb_set_buffer( reverb.combL[2], reverb.bufcombL3, SysAudioTuning::combTuningL3 );
		reverb_comb_set_buffer( reverb.combR[2], reverb.bufcombR3, SysAudioTuning::combTuningR3 );
		reverb_comb_set_buffer( reverb.combL[3], reverb.bufcombL4, SysAudioTuning::combTuningL4 );
		reverb_comb_set_buffer( reverb.combR[3], reverb.bufcombR4, SysAudioTuning::combTuningR4 );
		reverb_comb_set_buffer( reverb.combL[4], reverb.bufcombL5, SysAudioTuning::combTuningL5 );
		reverb_comb_set_buffer( reverb.combR[4], reverb.bufcombR5, SysAudioTuning::combTuningR5 );
		reverb_comb_set_buffer( reverb.combL[5], reverb.bufcombL6, SysAudioTuning::combTuningL6 );
		reverb_comb_set_buffer( reverb.combR[5], reverb.bufcombR6, SysAudioTuning::combTuningR6 );
		reverb_comb_set_buffer( reverb.combL[6], reverb.bufcombL7, SysAudioTuning::combTuningL7 );
		reverb_comb_set_buffer( reverb.combR[6], reverb.bufcombR7, SysAudioTuning::combTuningR7 );
		reverb_comb_set_buffer( reverb.combL[7], reverb.bufcombL8, SysAudioTuning::combTuningL8 );
		reverb_comb_set_buffer( reverb.combR[7], reverb.bufcombR8, SysAudioTuning::combTuningR8 );

		reverb_allpass_set_buffer( reverb.allpassL[0], reverb.bufallpassL1, SysAudioTuning::allpassTuningL1 );
		reverb_allpass_set_buffer( reverb.allpassR[0], reverb.bufallpassR1, SysAudioTuning::allpassTuningR1 );
		reverb_allpass_set_buffer( reverb.allpassL[1], reverb.bufallpassL2, SysAudioTuning::allpassTuningL2 );
		reverb_allpass_set_buffer( reverb.allpassR[1], reverb.bufallpassR2, SysAudioTuning::allpassTuningR2 );
		reverb_allpass_set_buffer( reverb.allpassL[2], reverb.bufallpassL3, SysAudioTuning::allpassTuningL3 );
		reverb_allpass_set_buffer( reverb.allpassR[2], reverb.bufallpassR3, SysAudioTuning::allpassTuningR3 );
		reverb_allpass_set_buffer( reverb.allpassL[3], reverb.bufallpassL4, SysAudioTuning::allpassTuningL4 );
		reverb_allpass_set_buffer( reverb.allpassR[3], reverb.bufallpassR4, SysAudioTuning::allpassTuningR4 );

		// Set default values
		reverb.allpassL[0].feedback = 0.5f;
		reverb.allpassR[0].feedback = 0.5f;
		reverb.allpassL[1].feedback = 0.5f;
		reverb.allpassR[1].feedback = 0.5f;
		reverb.allpassL[2].feedback = 0.5f;
		reverb.allpassR[2].feedback = 0.5f;
		reverb.allpassL[3].feedback = 0.5f;
		reverb.allpassR[3].feedback = 0.5f;

		effect.set_parameter( SysAudio::EffectParam_Reverb_Wet, SysAudioTuning::initialWet, 0 );
		effect.set_parameter( SysAudio::EffectParam_Reverb_RoomSize, SysAudioTuning::initialRoom, 0 );
		effect.set_parameter( SysAudio::EffectParam_Reverb_Dry, SysAudioTuning::initialDry, 0 );
		effect.set_parameter( SysAudio::EffectParam_Reverb_Damp, SysAudioTuning::initialDamp, 0 );
		effect.set_parameter( SysAudio::EffectParam_Reverb_Width, SysAudioTuning::initialWidth, 0 );

		// Buffer will be full of rubbish - so we MUST mute them
		reverb_mute( effect );
	}
}


static void reverb_apply( SysAudio::Effect &effect, float *samples, u32 sampleCount )
{
	SysAudio::EffectStateReverb &reverb = effect.stateReverb;

	for( u32 i = 0; i < sampleCount; i++ )
	{
		reverb_update( effect );

		float outL = 0.0f;
		float outR = 0.0f;
		float input = ( samples[i * 2 + 0] + samples[i * 2 + 1] ) * reverb.gain;

		// Accumulate comb filters in parallel
		for( int j = 0; j < SysAudioTuning::numCombs; j++ )
		{
			outL += reverb_comb_process( reverb.combL[j], input );
			outR += reverb_comb_process( reverb.combR[j], input );
		}

		// Feed through allpasses in series
		for( int j = 0; j < SysAudioTuning::numAllpasses; j++ )
		{
			outL = reverb_allpass_process( reverb.allpassL[j], outL );
			outR = reverb_allpass_process( reverb.allpassR[j], outR );
		}

		samples[i * 2 + 0] = outL * reverb.wet1 + outR * reverb.wet2 + samples[i * 2 + 0] * reverb.dry;
		samples[i * 2 + 1] = outR * reverb.wet1 + outL * reverb.wet2 + samples[i * 2 + 1] * reverb.dry;
	}
}


__AUDIO_EFFECT_PARAM_GET_SET_IMPL( SysAudio::EffectType_Reverb, reverb_size, SysAudio::EffectParam_Reverb_RoomSize )
__AUDIO_EFFECT_PARAM_GET_SET_IMPL( SysAudio::EffectType_Reverb, reverb_wet, SysAudio::EffectParam_Reverb_Wet )
__AUDIO_EFFECT_PARAM_GET_SET_IMPL( SysAudio::EffectType_Reverb, reverb_dry, SysAudio::EffectParam_Reverb_Dry )
//__AUDIO_EFFECT_PARAM_GET_SET_IMPL( SysAudio::EffectType_Reverb, reverb_damp, SysAudio::EffectParam_Reverb_Damp )
__AUDIO_EFFECT_PARAM_GET_SET_IMPL( SysAudio::EffectType_Reverb, reverb_width, SysAudio::EffectParam_Reverb_Width )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct EffectFunctions
{
	void ( *init )( struct SysAudio::Effect &effect );
	void ( *apply )( struct SysAudio::Effect &effect, float *output, u32 frames );
};

static EffectFunctions effectFunctions[] =
{
	{ core_init, core_apply },       // EffectType_Core
 	{ lowpass_init, lowpass_apply }, // EffectType_Lowpass
	{ reverb_init, reverb_apply },   // EffectType_Reverb
};

static_assert( ARRAY_LENGTH( effectFunctions ) == SysAudio::EFFECTTYPE_COUNT, "Missing audio effect type" );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SysAudio::init()
{
#if AUDIO_ENABLED
	// Initialize Audio Buses & Voices
	for( int i = 0; i < AUDIO_BUS_COUNT; i++ ) { new ( &buses[i] ) SysAudio::Bus { }; }
	for( int i = 0; i < AUDIO_VOICE_COUNT; i++ ) { new ( &voices[i] ) SysAudio::Voice { }; }

	// Initialize Samples Buffer
	ErrorReturnIf( g_AUDIO_SAMPLES != nullptr, false, "Audio: samples buffer already initialized" );
	if constexpr ( Assets::soundSampleDataSize == 0 ) { return true; }

	g_AUDIO_SAMPLES = reinterpret_cast<i16 *>( memory_alloc( Assets::soundSampleDataSize ) );
	memory_copy( g_AUDIO_SAMPLES, &Assets::binary.data[Assets::soundSampleDataOffset], Assets::soundSampleDataSize );

	// Initialize Backend
	bool failure = !init_backend();
	ErrorReturnIf( failure, false, "Audio: failed to initialize audio backend" );
#endif

	return true;
}


bool SysAudio::free()
{
#if AUDIO_ENABLED
	// Free Backend
	bool failure = !free_backend();
	ErrorReturnIf( failure, false, "Audio: failed to free audio backend" );

	// Free Samples
	if( g_AUDIO_SAMPLES != nullptr )
	{
		memory_free( g_AUDIO_SAMPLES );
		g_AUDIO_SAMPLES = nullptr;
	}

	// Free Voices
	for( int i = 0; i < AUDIO_VOICE_COUNT; i++ )
	{
		new ( &voices[i] ) Voice { };
	}
#endif

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static THREAD_FUNCTION( audio_stream )
{
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int find_voice()
{
	// Find first available voice
	for( int i = 0; i < AUDIO_VOICE_COUNT; i++ )
	if( SysAudio::voices[i].bus < 0 ) { return i; }

	// Failure
	return -1;
}


static int find_bus()
{
	// Find first available bus
	for( int i = 0; i < AUDIO_BUS_COUNT; i++ )
	if( SysAudio::buses[i].available ) { return i; }

	// Failure
	return -1;
}

void SysAudio::audio_mixer( i16 *output, u32 frames )
{
#if AUDIO_ENABLED
	const int CHANNELS = 2; // TODO
	const usize framesBufferI16Size = frames * CHANNELS * sizeof( i16 );
	const usize framesBufferFloatSize = frames * CHANNELS * sizeof( float );

	// Allocate float buffer
	byte *buffer = reinterpret_cast<byte *>( memory_alloc( 3 * framesBufferFloatSize ) ); // TODO: Remove malloc
	float *bufferBus = reinterpret_cast<float *>( &buffer[0 * framesBufferFloatSize] );
	float *bufferVoice = reinterpret_cast<float *>( &buffer[1 * framesBufferFloatSize] );
	float *bufferMaster = reinterpret_cast<float *>( &buffer[2 * framesBufferFloatSize] );
	memory_set( bufferMaster, 0, framesBufferFloatSize );

	// Mix buses
	for( int i = 0; i < AUDIO_BUS_COUNT; i++ )
	{
		Bus &bus = buses[i];
		memory_set( bufferBus, 0, framesBufferFloatSize );

		Assert( bus.effects[0].type == SysAudio::EffectType_Core );
		const float pitchBus = bus.effects[0].get_parameter( SysAudio::EffectParam_Core_Pitch, false );

		// Mix voices
		for( int j = 0; j < AUDIO_VOICE_COUNT; j++ )
		{
			Voice &voice = voices[j];
			if( voice.bus != i || voice.bypass ) { continue; }
			memory_set( bufferVoice, 0, framesBufferFloatSize );

			Assert( voice.effects[0].type == SysAudio::EffectType_Core );
			const float pitchVoice = voice.effects[0].get_parameter( SysAudio::EffectParam_Core_Pitch, false ) * pitchBus;

			const u32 frameCurrent = static_cast<u32>( voice.position );
			const u32 framesRemaining = ( voice.samplesCount - frameCurrent ) / voice.channels;
			const u32 framesToMix = min( frames, framesRemaining );

			// Read voice samples
			switch( voice.channels )
			{
				// Mono
				case 1:
				{
					for( u32 k = 0; k < framesToMix && voice.position < voice.samplesCount; k++ )
					{
						const u32 sampleIndex = static_cast<u32>( voice.position );

						const float sample1 = I16_TO_FLOAT( voice.samples[sampleIndex + 0] );
						const float sample2 = ( sampleIndex + 1 < voice.samplesCount ) ?
											  I16_TO_FLOAT( voice.samples[sampleIndex + 1] ) : sample1;

						const float sampleFrac = voice.position - sampleIndex;
						const float sampleInterpolated = sample1 * ( 1.0f - sampleFrac ) + sample2 * sampleFrac;

						bufferVoice[k * 2 + 0] = sampleInterpolated;
						bufferVoice[k * 2 + 1] = sampleInterpolated;
						voice.position += pitchVoice;
					}
				}
				break;

				// Stereo
				case 2:
				{
					for( u32 k = 0; k < framesToMix && voice.position < voice.samplesCount; k++ )
					{
						const u32 sampleIndex = static_cast<u32>( voice.position );

						const float left1 = I16_TO_FLOAT( voice.samples[sampleIndex + 0] );
						const float left2 = ( sampleIndex + 2 < voice.samplesCount ) ?
						                    I16_TO_FLOAT( voice.samples[sampleIndex + 2] ) : left1;
						const float right1 = I16_TO_FLOAT( voice.samples[sampleIndex + 1] );
						const float right2 = ( sampleIndex + 3 < voice.samplesCount ) ?
						                     I16_TO_FLOAT( voice.samples[sampleIndex + 3] ) : right1;

						const float sampleFrac = voice.position - static_cast<u32>( voice.position );
						const float leftInterpolated = left1 * ( 1.0f - sampleFrac ) + left2 * sampleFrac;
						const float rightInterpolated = right1 * ( 1.0f - sampleFrac ) + right2 * sampleFrac;

						bufferVoice[k * 2 + 0] = leftInterpolated;
						bufferVoice[k * 2 + 1] = rightInterpolated;
						voice.position += pitchVoice * 2;
					}
				}
				break;
			}

			// Process per-voice effects
			for( u32 k = 0; k < SysAudio::EFFECTTYPE_COUNT; k++ )
			{
				Effect &effect = voice.effects[k];
				if( effect.type < 0 ) { break; }
				effectFunctions[effect.type].apply( effect, bufferVoice, frames );
			}

			// Write voice to bus
			for( u32 k = 0; k < framesToMix; k++ )
			{
				bufferBus[k * 2 + 0] += bufferVoice[k * 2 + 0];
				bufferBus[k * 2 + 1] += bufferVoice[k * 2 + 1];
			}

			// Voice complete?
			if( static_cast<u32>( voice.position ) >= voice.samplesCount ) { voice.bus = -1; }
		}

		// Process per-bus effects
		for( u32 j = 0; j < SysAudio::EFFECTTYPE_COUNT; j++ )
		{
			Effect &effect = bus.effects[j];
			if( effect.type < 0 ) { continue; }
			effectFunctions[effect.type].apply( effect, bufferBus, frames );
		}

		// Write bus to master
		for( u32 j = 0; j < frames; j++ )
		{
			bufferMaster[j * 2 + 0] += bufferBus[j * 2 + 0];
			bufferMaster[j * 2 + 1] += bufferBus[j * 2 + 1];
		}
	}

	// Write to master output as i16
	for( u32 i = 0; i < frames; i++ )
	{
		const float lClamped = clamp( bufferMaster[i * 2 + 0], -1.0f, 1.0f );
		output[i * 2 + 0] = FLOAT_TO_I16( lClamped );
		const float rClamped = clamp( bufferMaster[i * 2 + 1], -1.0f, 1.0f );
		output[i * 2 + 1] = FLOAT_TO_I16( rClamped );
	}

	// Free float buffer
	memory_free( buffer );
#endif
}


SoundHandle SysAudio::play_sound( const int bus, const i16 *const samples, const u32 samplesCount, const int channels,
                                  const AudioEffects &effects, const char *name )
{
	const int voice = find_voice();
	if( voice < 0 ) { return SoundHandle { -1, -1 }; }
	SysAudio::Voice &audioVoice = SysAudio::voices[voice];

	// Copy Effects
	memory_copy( &audioVoice.effects, &effects, sizeof( AudioEffects ) );

	// Setup State
	audioVoice.position = 0.0f;
	audioVoice.channels = channels;
	audioVoice.samples = samples;
	audioVoice.samplesCount = samplesCount;
#if COMPILE_DEBUG
	audioVoice.name = name;
#endif

	// TODO: Thread/Compile Barrier here
	audioVoice.id++;
	audioVoice.bus = bus;


	// Return handle
	return SoundHandle { voice, audioVoice.id };
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AudioEffects::init()
{
	// Default-initialize Effects
	for( int i = 0; i < SysAudio::EFFECTTYPE_COUNT; i++ )
	{
		SysAudio::Effect &effect = effects[i];
		new ( &effect ) SysAudio::Effect { };

		// First effect is always AudioEffect_Core
		effects[0].type = SysAudio::EffectType_Core;
		effects[0].set_parameter( SysAudio::EffectParam_Core_Gain, 1.0f, 0.0f );
		effects[0].set_parameter( SysAudio::EffectParam_Core_Pitch, 1.0f, 0.0f );
	}
}


SysAudio::Effect &AudioEffects::find_effect( const SysAudio::EffectType effectType )
{
	Assert( effectType < SysAudio::EFFECTTYPE_COUNT );

	for( int i = 0; i < SysAudio::EFFECTTYPE_COUNT; i++ )
	{
		SysAudio::Effect &effect = effects[i];

		// Found an already-enabled slot with our type, early exit
		if( effect.type == effectType )
		{
			return effect;
		}

		// Found empty effect slot, so let's enable it as our type
		if( effect.type < 0 )
		{
			effect.type = effectType;
			if( effectFunctions[effectType].init != nullptr ) { effectFunctions[effectType].init( effect ); }
			return effect;
		}
	}

	AssertMsg( true, "Failed to find effect id!" );
	return effects[0]; // Unreachable
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SoundHandle::is_playing() const
{
	if( voice < 0 || voice >= AUDIO_VOICE_COUNT ) { return false; }
	if( SysAudio::voices[voice].id != id ) { return false; }
	return !( SysAudio::voices[voice].bus < 0 );
}


bool SoundHandle::is_paused() const
{
	if( voice < 0 || voice >= AUDIO_VOICE_COUNT ) { return false; }
	if( SysAudio::voices[voice].bus < 0 || SysAudio::voices[voice].id != id ) { return false; }
	return SysAudio::voices[voice].bypass;
}


bool SoundHandle::pause( const bool pause ) const
{
	if( voice < 0 || voice >= AUDIO_VOICE_COUNT ) { return false; }
	if( SysAudio::voices[voice].bus < 0 || SysAudio::voices[voice].id != id ) { return false; }
	SysAudio::voices[voice].bypass = pause;
	return true;
}


bool SoundHandle::stop() const
{
	if( voice < 0 || voice >= AUDIO_VOICE_COUNT ) { return false; }
	if( SysAudio::voices[voice].bus < 0 || SysAudio::voices[voice].id != id ) { return false; }
	SysAudio::voices[voice].bus = -1;
	return true;
}


AudioEffects *SoundHandle::operator->() const
{
	if( voice < 0 || voice >= AUDIO_VOICE_COUNT ) { return &NULL_AUDIO_EFFECTS; }
	if( SysAudio::voices[voice].bus < 0 || SysAudio::voices[voice].id != id ) { return &NULL_AUDIO_EFFECTS; }
	return &SysAudio::voices[voice].effects;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AudioContext::init( const AudioEffects &effects, const char *name )
{
	// Reserve an audio bus
	bus = find_bus();
	ErrorIf( bus < 0, "AudioLayer init: exceeded bus capacity!" );
	SysAudio::Bus &audioBus = SysAudio::buses[bus];
	audioBus.available = false;
	this->name = name;
	audioBus.name = name;

	// Copy Effects
	memory_copy( &audioBus.effects, &effects, sizeof( AudioEffects ) );
};


void AudioContext::free()
{
	if( bus < 0 || bus >= AUDIO_BUS_COUNT ) { return; }

	// Stop playing voices
	for( int i = 0; i < AUDIO_VOICE_COUNT; i++ )
	{
		SysAudio::Voice &voice = SysAudio::voices[i];
		if( voice.bus == bus ) { voice.bus = -1; }
	}

	// Mark our audio bus as available
	SysAudio::Bus &audioBus = SysAudio::buses[bus];
	audioBus.available = true;
};


AudioEffects *AudioContext::operator->() const
{
	Assert( bus > 0 || bus < AUDIO_BUS_COUNT );
	return &SysAudio::buses[bus].effects;
}


bool AudioContext::is_paused() const
{
	if( bus < 0 || bus >= AUDIO_BUS_COUNT ) { return false; }
	return SysAudio::buses[bus].bypass;
}


bool AudioContext::pause( const bool pause ) const
{
	if( bus < 0 || bus >= AUDIO_BUS_COUNT ) { return false; }
	SysAudio::buses[bus].bypass = pause;
	return true;
}


SoundHandle AudioContext::play_sound( const u32 sound, const AudioEffects &effects )
{
	Assert( sound < Assets::soundCount );
	Assert( bus >= 0 || bus < AUDIO_BUS_COUNT );
	const i16 *const samples = &SysAudio::g_AUDIO_SAMPLES[Assets::sounds[sound].sampleOffset];
	const u32 samplesCount = Assets::sounds[sound].sampleCount;
	const int channels = Assets::sounds[sound].channels;
#if COMPILE_DEBUG
	const char *name = Assets::sounds[sound].name;
#else
	const char *name = "";
#endif
	return SysAudio::play_sound( bus, samples, samplesCount, channels, effects, name );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if COMPILE_DEBUG
#include <manta/draw.hpp>

static const Color colorLabel = c_ltgray;
static const Color colorValue = Color { 255, 215, 135 };
static const Color colorSeparator = Color { 190, 190, 190 };

static const Font font = fnt_iosevka;
static const u16 fontSizeTitle = 14;
static const u16 fontSizeLabel = 12;


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
	const usize secondsTotal = sampleCount / 44100 / sizeof( i16 ) / channels;
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


static void audio_draw_label_value( const float x, const float y, const int width,
                                    const Color colorLabel, const Color colorSeparator, const Color colorValue,
                                    const char *label, const char *value )
{
	const intv2 labelDimensions = text_dimensions( font, fontSizeLabel, label );
	const intv2 valueDimensions = text_dimensions( font, fontSizeLabel, value );

	// Label
	draw_text( font, fontSizeLabel, x, y, colorLabel, label );

	// Separator
	char buffer[128];
	for( int i = 0; i < 128; i++ ) { buffer[i] = '.'; }
	const int r = clamp( ( width - valueDimensions.x - labelDimensions.x ) /
	                     ( text_dimensions( font, fontSizeLabel, "." ).x ), 0, 128 );
	buffer[r] = '\0';
	draw_text( font, fontSizeLabel, x + labelDimensions.x, y, colorSeparator, buffer );

	// Value
	draw_text( font, fontSizeLabel, x + width - valueDimensions.x, y, colorValue, value );
}


intv2 SysAudio::draw( const Delta delta, const float x, const float y )
{
	intv2 dimensions = { 0, 0 };

	for( int i = 0; i < AUDIO_BUS_COUNT; i++ )
	{
		float dX = x + i * 224;
		float dY = y;
		if( !draw_bus( delta, i, dX, dY ) ) { continue; }

		const int width = ( i + 1 ) * 224;
		dimensions.x = ( width > dimensions.x ? width : dimensions.x );
		dimensions.y = ( dY - y > dimensions.y ? dY - y : dimensions.y );
	}

	return dimensions;
}


bool SysAudio::draw_bus( const Delta delta, const int bus, float &x, float &y )
{
	Bus &audioBus = buses[bus];
	if( audioBus.available ) { return false; }
	const float xIn = x;
	const float yIn = y;

	// Bus Label
	const char *labelFormat = audioBus.name[0] == '\0' ? "Bus %d" : "Bus %d: ";
	const intv2 labelDimensions = text_dimensions_f( font, fontSizeTitle, labelFormat, bus );
	draw_text_f( font, fontSizeTitle, x, y, c_white, labelFormat, bus );
	draw_text( font, fontSizeTitle, x + labelDimensions.x, y, c_yellow, audioBus.name );
	y += 20.0f;

	// Effects
	x += 16.0f;
	for( int i = 0; i < SysAudio::EFFECTTYPE_COUNT; i++ ) { draw_effect( delta, audioBus.effects[i], x, y ); }
	x -= 16.0f;
	y += 4.0f;

	// Voices
	x += 16.0f;
	{
		bool hasVoices = false;

		for( int i = 0; i < AUDIO_VOICE_COUNT; i++ )
		{
			const Voice &audioVoice = voices[i];
			if( audioVoice.bus != bus ) { continue; }
			hasVoices |= draw_voice( delta, i, x, y );
			y += 4.0f;
		}

		if( !hasVoices )
		{
			draw_text( font, fontSizeLabel, x, y, c_gray, "No Active Voices" );
			y += 20.0f;
		}
	}
	x = xIn;

	return true;
}


bool SysAudio::draw_voice( const Delta delta, const int voice, float &x, float &y )
{
	Voice &audioVoice = voices[voice];
	if( audioVoice.bus < 0 ) { return false; }

	// Widget State
	const int widgetWidth = 192;

	// Voice Label
	const char *labelFormat = audioVoice.name[0] == '\0' ? "v%d" : "v%d: ";
	const intv2 labelDimensions = text_dimensions_f( font, fontSizeLabel, labelFormat, voice );
	draw_text_f( font, fontSizeLabel, x, y, c_white, labelFormat, voice );
	draw_text( font, fontSizeLabel, x + labelDimensions.x, y, c_yellow, audioVoice.name );
	y += 16.0f;

	// Progress Bar
	const float progress = audioVoice.position / audioVoice.samplesCount;
	draw_progress_bar( x, y, widgetWidth, 10.0f, progress, c_dkgray, c_white );
	y += 16.0f;

	// Effects
	for( int i = 0; i < SysAudio::EFFECTTYPE_COUNT; i++ ) { draw_effect( delta, audioVoice.effects[i], x, y ); }
	y += 4.0f;

	return true;
}


bool SysAudio::draw_effect( const Delta delta, Effect &effect, float &x, float &y )
{
	if( effect.type < 0 ) { return false; }
	const int labelIndent = 64;
	const int labelWidth = 128;

	// Helper Lambda
	char value[16];
	auto draw_value_label = [ &value, &effect, &x, &y ]( const char *label, const SysAudio::EffectParam param )
	{
		snprintf( value, 16, "%.2f", effect.get_parameter( param ) );
		audio_draw_label_value( x, y, labelWidth, colorLabel, colorSeparator, colorValue, label, value );
		y += 16.0f;
	};

	// Draw Effect Labels
	switch( effect.type )
	{
		case SysAudio::EffectType_Core:
		{
			draw_text( font, fontSizeLabel, x, y, c_white, "Core" );
			x += labelIndent;
			draw_value_label( "Gain", SysAudio::EffectParam_Core_Gain );
			draw_value_label( "Pitch", SysAudio::EffectParam_Core_Pitch );
			x -= labelIndent;
		}
		return true;

		case SysAudio::EffectType_Lowpass:
		{
			draw_text( font, fontSizeLabel, x, y, c_white, "Lowpass" );
			x += labelIndent;
			draw_value_label( "Cutoff", SysAudio::EffectParam_Lowpass_Cutoff );
			x -= labelIndent;
		}
		return true;

		case SysAudio::EffectType_Reverb:
		{
			draw_text( font, fontSizeLabel, x, y, c_white, "Reverb" );
			x += labelIndent;
			draw_value_label( "Room Size", SysAudio::EffectParam_Reverb_RoomSize );
			draw_value_label( "Wet", SysAudio::EffectParam_Reverb_Wet );
			draw_value_label( "Dry", SysAudio::EffectParam_Reverb_Dry );
			draw_value_label( "Width", SysAudio::EffectParam_Reverb_Width );
			x -= labelIndent;
		}
		return true;

		default: AssertMsg( true, "Unsupported audio effect debug label!" ); return false;
	}
}

#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////