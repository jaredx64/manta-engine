#include <manta/audio.hpp>

#include <vendor/new.hpp>

#include <core/debug.hpp>
#include <core/math.hpp>

#include <manta/thread.hpp>
#include <manta/assets.hpp>
#include <manta/vector.hpp>
#include <manta/random.hpp>
#include <manta/time.hpp>

#include <manta/console.hpp>
#include <manta/window.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define I16_TO_FLOAT(value) ( ( value ) * ( 1.0f / I16_MAX ) )
#define FLOAT_TO_I16(value) static_cast<i16>( ( value ) * I16_MAX )

#define undenormalise( sample ) \
	if( ( ( *reinterpret_cast<unsigned int *>( &sample ) ) & 0x7f800000 ) == 0 ) { sample = 0.0f; }

static AudioEffects NULL_AUDIO_EFFECTS;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace CoreAudio
{
	Voice voices[AUDIO_VOICE_COUNT];
	Stream sounds[AUDIO_STREAM_COUNT];
	Bus buses[AUDIO_BUS_COUNT];

	static i16 *samples = nullptr;
	static i16 buffers[AUDIO_STREAM_COUNT][AUDIO_STREAM_BUFFERS][AUDIO_STREAM_BLOCK];

	static Random random;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool DEBUG_ENABLED = false;

static CommandHandle CMD_DEBUG_ENABLED;
static CONSOLE_COMMAND_FUNCTION( cmd_debug_enabled )
{
	DEBUG_ENABLED = Console::get_parameter_toggle( 0, DEBUG_ENABLED );
	Console::Log( c_white, "debug audio %d", DEBUG_ENABLED );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float CoreAudio::Effect::get_parameter( const EffectParam param, const bool incrementTime )
{
	//Assert( active );
	const float valueFrom = parameters[param].valueFrom;
	const float valueTo = parameters[param].valueTo;
	const float amount = parameters[param].sampleCurrent * parameters[param].sampleToInv;
	const float value = amount < 1.0f ? lerp( valueFrom, valueTo, amount ) : valueTo;
	parameters[param].sampleCurrent += static_cast<float>( amount < 1.0f && incrementTime );
	return value;
}


void CoreAudio::Effect::set_parameter_default( const EffectParam param, const float value )
{
	parameters[param].valueFrom = value;
	parameters[param].valueTo = value;
	parameters[param].sampleCurrent = 0.0f;
	parameters[param].sampleToInv = 1.0f;
}


void CoreAudio::Effect::set_parameter( const EffectParam param, const float value )
{
	active = true;
	parameters[param].valueFrom = value;
	parameters[param].valueTo = value;
	parameters[param].sampleCurrent = 0.0f;
	parameters[param].sampleToInv = 1.0f;
}


void CoreAudio::Effect::set_parameter( const EffectParam param, const float value, const usize timeMS )
{
	active = true;
	parameters[param].valueFrom = get_parameter( param, false );
	parameters[param].valueTo = value;
	parameters[param].sampleCurrent = 0.0f;
	constexpr float msToHz = ( 44100.0f / 1000.0f );
	parameters[param].sampleToInv = 1.0f / ( timeMS * msToHz );
}


void CoreAudio::Effect::set_parameter( const EffectParam param, const float v0, const float v1, const usize timeMS )
{
	active = true;
	parameters[param].valueFrom = v0;
	parameters[param].valueTo = v1;
	parameters[param].sampleCurrent = 0.0f;
	constexpr float msToHz = ( 44100.0f / 1000.0f );
	parameters[param].sampleToInv = 1.0f / ( timeMS * msToHz );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// EffectType_Core

static void core_init( CoreAudio::Effect &effect, CoreAudio::EffectState &state )
{
}


static void core_reset( CoreAudio::Effect &effect, CoreAudio::EffectState &state )
{
}


static void core_apply( CoreAudio::Bus &bus, CoreAudio::Effect &effect, CoreAudio::EffectState &state,
	float *samples, u32 sampleCount )
{
	CoreAudio::EffectStateCore &core = state.stateGain;

	for( u32 i = 0; i < sampleCount; i++ )
	{
		const float volume = effect.get_parameter( CoreAudio::EffectParam_Core_Gain, true );
		samples[i * 2 + 0] *= volume;
		samples[i * 2 + 1] *= volume;
	}
}

__AUDIO_EFFECT_PARAM_GET_SET_IMPL( CoreAudio::EffectType_Core, gain, CoreAudio::EffectParam_Core_Gain )
__AUDIO_EFFECT_PARAM_GET_SET_NOAUTOMATION_IMPL( CoreAudio::EffectType_Core, pitch, CoreAudio::EffectParam_Core_Pitch )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// EffectType_Gain

static void spatial_update( CoreAudio::Bus &bus, CoreAudio::Effect &effect, CoreAudio::EffectState &state )
{
	CoreAudio::EffectStateSpatial &spatial = state.stateSpatial;

	// Get spatial parameters
	const float params[] =
	{
		effect.get_parameter( CoreAudio::EffectParam_Spatial_X, true ),
		effect.get_parameter( CoreAudio::EffectParam_Spatial_Y, true ),
		effect.get_parameter( CoreAudio::EffectParam_Spatial_Z, true ),
		effect.get_parameter( CoreAudio::EffectParam_Spatial_Attenuation, true ),
	};

	// Calculate distance
	const float distanceSqr = params[0] * params[0] + params[1] * params[1] + params[2] * params[2] + 1e-8f;
	const float distanceAttenuationInv = 1.0f / ( 1.0f + distanceSqr * params[3] );
	const float distanceNormInv = 1.0f / sqrtf( distanceSqr );

	// Get listener orientation vectors
	const float_v3 forward = float_v3 { bus.lookX, bus.lookY, bus.lookZ };
	const float_v3 up = float_v3 { bus.upX, bus.upY, bus.upZ };
	float_v3 right = float_v3_cross( forward, up );
	const float rightLengthInv = 1.0f / sqrtf( right.x * right.x + right.y * right.y + right.z * right.z );
	right = float_v3 { right.x * rightLengthInv, right.y * rightLengthInv, right.z * rightLengthInv };

	// Calculate panning
	const float pan = ( params[0] * right.x + params[1] * right.y + params[2] * right.z ) * distanceNormInv;
	const float panClamped = clamp( pan, -1.0f, 1.0f );

	// Equal-power panning
	const float panPosition = ( panClamped + 1.0f ) * 0.5f;
#if 0
	spatial.gainLeft = sqrtf( 1.0f - panPosition ) * distanceAttenuationInv;
	spatial.gainRight = sqrtf( panPosition ) * distanceAttenuationInv;
#else
	const float panLeft = 1.0f - panPosition;
	spatial.gainLeft = ( 0.93f * panLeft + 0.07f * panLeft * panLeft ) * distanceAttenuationInv;
	const float panRight = panPosition;
	spatial.gainRight = ( 0.93f * panRight + 0.07f * panRight * panRight ) * distanceAttenuationInv;
#endif
}


static void spatial_init( CoreAudio::Effect &effect, CoreAudio::EffectState &state )
{
	CoreAudio::EffectStateSpatial &spatial = state.stateSpatial;
}


static void spatial_reset( CoreAudio::Effect &effect, CoreAudio::EffectState &state )
{
	CoreAudio::EffectStateSpatial &spatial = state.stateSpatial;
}


static void spatial_apply( CoreAudio::Bus &bus, CoreAudio::Effect &effect, CoreAudio::EffectState &state,
	float *samples, u32 sampleCount )
{
	CoreAudio::EffectStateSpatial &spatial = state.stateSpatial;

	for( u32 i = 0; i < sampleCount; i++ )
	{
		spatial_update( bus, effect, state );

		const float sample = samples[i * 2];
		samples[i * 2] = sample * spatial.gainLeft;
		samples[i * 2 + 1] = sample * spatial.gainRight;
	}
}


__AUDIO_EFFECT_PARAM_GET_SET_IMPL( CoreAudio::EffectType_Spatial, spatial_x, CoreAudio::EffectParam_Spatial_X )
__AUDIO_EFFECT_PARAM_GET_SET_IMPL( CoreAudio::EffectType_Spatial, spatial_y, CoreAudio::EffectParam_Spatial_Y )
__AUDIO_EFFECT_PARAM_GET_SET_IMPL( CoreAudio::EffectType_Spatial, spatial_z, CoreAudio::EffectParam_Spatial_Z )
__AUDIO_EFFECT_PARAM_GET_SET_IMPL( CoreAudio::EffectType_Spatial, spatial_attenuation,
	CoreAudio::EffectParam_Spatial_Attenuation )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// EffectType_Lowpass (https://github.com/sinshu/freeverb)

static void lowpass_update( CoreAudio::Effect &effect, CoreAudio::EffectState &state )
{
	CoreAudio::EffectStateLowpass &lowpass = state.stateLowpass;

	// Parameters
	const float paramCutoff = effect.get_parameter( CoreAudio::EffectParam_Lowpass_Cutoff, true );
	lowpass.omega0 = 6.28318530718f * paramCutoff;

	// Internal State
	const float alpha = lowpass.omega0 * lowpass.dt;
#if LPF_ORDER == 1
	lowpass.a[0][0] = -( alpha - 2.0f ) / ( alpha + 2.0f );
	lowpass.a[1][0] = lowpass.a[0][0];

	lowpass.b[0][0] = alpha / ( alpha + 2.0f );
	lowpass.b[1][0] = lowpass.b[0][0];

	lowpass.b[0][1] = alpha / ( alpha + 2.0f );
	lowpass.b[1][1] = lowpass.b[0][1];
#elif LPF_ORDER == 2
	const float alphaSq = alpha * alpha;
	const float beta[] = { 1.0f, 1.41421356237f /* sqrtf(2.0f) */, 1.0f };
	const float invD = 1.0f / ( alphaSq * beta[0] + 2.0f * alpha * beta[1] + 4.0f * beta[2] );

	lowpass.b[0][0] = alphaSq * invD;
	lowpass.b[1][0] = lowpass.b[0][0];

	lowpass.b[0][1] = 2.0f * lowpass.b[0][0];
	lowpass.b[1][1] = lowpass.b[0][1];

	lowpass.b[0][2] = lowpass.b[0][0];
	lowpass.b[1][2] = lowpass.b[0][2];

	lowpass.a[0][0] = -( 2.0f * alphaSq * beta[0] - 8.0f * beta[2] ) * invD;
	lowpass.a[1][0] = lowpass.a[0][0];

	lowpass.a[0][1] = -( beta[0] * alphaSq - 2.0f * beta[1] * alpha + 4.0f * beta[2] ) * invD;
	lowpass.a[1][1] = lowpass.a[0][1] ;
#endif
}


static void lowpass_init( CoreAudio::Effect &effect, CoreAudio::EffectState &state )
{
	CoreAudio::EffectStateLowpass &lowpass = state.stateLowpass;
	static float sampleRate = 44100.0f;
	lowpass.dt = 1.0f / sampleRate;
	lowpass_update( effect, state );
}


static void lowpass_reset( CoreAudio::Effect &effect, CoreAudio::EffectState &state )
{
	effect.set_parameter_default( CoreAudio::EffectParam_Lowpass_Cutoff, 20000.0f );
}


static void lowpass_apply( CoreAudio::Bus &bus, CoreAudio::Effect &effect, CoreAudio::EffectState &state,
	float *samples, u32 sampleCount )
{
	CoreAudio::EffectStateLowpass &lowpass = state.stateLowpass;

	for( u32 i = 0; i < sampleCount; i++ )
	{
		// Update coefficients
		lowpass_update( effect, state );

		for( int ch = 0; ch < 2; ch++ )
		{
			lowpass.y[ch][0] = 0;
			lowpass.x[ch][0] = samples[i * 2 + ch];

			// Compute the filtered values
			for( int j = 0; j < LPF_ORDER; j++ )
			{
				lowpass.y[ch][0] += lowpass.a[ch][j] * lowpass.y[ch][j + 1] + lowpass.b[ch][j] * lowpass.x[ch][j];
			}

			lowpass.y[ch][0] += lowpass.b[ch][LPF_ORDER] * lowpass.x[ch][LPF_ORDER];

			// Save the historical values
			for( int j = LPF_ORDER; j > 0; j--)
			{
				lowpass.y[ch][j] = lowpass.y[ch][j - 1];
				lowpass.x[ch][j] = lowpass.x[ch][j - 1];
			}

			// return lpf.x[0];
			samples[i * 2 + ch] = lowpass.y[ch][0];
		}
	}
}


__AUDIO_EFFECT_PARAM_GET_SET_IMPL( CoreAudio::EffectType_Lowpass, lowpass_cutoff, CoreAudio::EffectParam_Lowpass_Cutoff )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// EffectType_Reverb (https://github.com/sinshu/freeverb)

static void reverb_comb_mute( CoreAudio::EffectStateReverbComb &comb )
{
	for( int i = 0; i < comb.bufsize; i++ ) { comb.buffer[i] = 0.0f; }
}


static void reverb_comb_set_buffer( CoreAudio::EffectStateReverbComb &comb, float *buffer, int size )
{
	comb.buffer = buffer;
	comb.bufsize = size;
}


static void reverb_comb_set_damp( CoreAudio::EffectStateReverbComb &comb, float val )
{
	comb.damp1 = val;
	comb.damp2 = 1.0f - val;
}


static float reverb_comb_process( CoreAudio::EffectStateReverbComb &comb, float input )
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


static void reverb_allpass_set_buffer( CoreAudio::EffectStateReverbAllPass &allpass, float *buffer, int size )
{
	allpass.buffer = buffer;
	allpass.bufsize = size;
}


static void reverb_allpass_mute( CoreAudio::EffectStateReverbAllPass &allpass)
{
	for( int i = 0; i < allpass.bufsize; i++ ) { allpass.buffer[i] = 0.0f; }
}


static inline float reverb_allpass_process( CoreAudio::EffectStateReverbAllPass &allpass, float input )
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


static void reverb_mute( CoreAudio::Effect &effect, CoreAudio::EffectState &state )
{
	CoreAudio::EffectStateReverb &reverb = state.stateReverb;

	// Mute Combs
	for( int i = 0; i < CoreAudioTuning::numCombs; i++ )
	{
		reverb_comb_mute( reverb.combL[i] );
		reverb_comb_mute( reverb.combR[i] );
	}

	// Mute All-passes
	for( int i = 0; i < CoreAudioTuning::numAllpasses; i++ )
	{
		reverb_allpass_mute( reverb.allpassL[i] );
		reverb_allpass_mute( reverb.allpassR[i] );
	}
}


static void reverb_update( CoreAudio::Effect &effect, CoreAudio::EffectState &state )
{
	CoreAudio::EffectStateReverb &reverb = state.stateReverb;

	// Parameters
	const float paramRoomsize = effect.get_parameter( CoreAudio::EffectParam_Reverb_RoomSize, true );
	const float paramWet = effect.get_parameter( CoreAudio::EffectParam_Reverb_Wet, true );
	const float paramDry = effect.get_parameter( CoreAudio::EffectParam_Reverb_Dry, true );
	const float paramDamp = effect.get_parameter( CoreAudio::EffectParam_Reverb_Damp, true );
	const float paramWidth = effect.get_parameter( CoreAudio::EffectParam_Reverb_Width, true );

	// Recalculate internal values after parameter change (TODO: refactor?)
	reverb.roomsize = paramRoomsize * CoreAudioTuning::scaleRoom + CoreAudioTuning::offsetRoom;
	reverb.wet = paramWet * CoreAudioTuning::scaleWet;
	reverb.wet1 = reverb.wet * ( reverb.width * 0.5f + 0.5f );
	reverb.wet2 = reverb.wet * ( ( 1.0f - reverb.width ) * 0.5f );
	reverb.dry = paramDry * CoreAudioTuning::scaleDry;
	reverb.damp = paramDry * CoreAudioTuning::scaleDamp;
	reverb.width = paramWidth;
	reverb.dry = paramDry;

	reverb.roomsize1 = reverb.roomsize;
	reverb.damp1 = reverb.damp;
	reverb.gain = CoreAudioTuning::fixedGain;

	for( int i = 0; i < CoreAudioTuning::numCombs; i++ )
	{
		reverb.combL[i].feedback = reverb.roomsize1;
		reverb_comb_set_damp( reverb.combL[i], reverb.damp1 );
		reverb.combR[i].feedback = reverb.roomsize1;
		reverb_comb_set_damp( reverb.combR[i], reverb.damp1 );
	}
}


static void reverb_init( CoreAudio::Effect &effect, CoreAudio::EffectState &state )
{
	CoreAudio::EffectStateReverb &reverb = state.stateReverb;
	{
		// Tie the components to their buffers
		reverb_comb_set_buffer( reverb.combL[0], reverb.bufcombL1, CoreAudioTuning::combTuningL1 );
		reverb_comb_set_buffer( reverb.combR[0], reverb.bufcombR1, CoreAudioTuning::combTuningR1 );
		reverb_comb_set_buffer( reverb.combL[1], reverb.bufcombL2, CoreAudioTuning::combTuningL2 );
		reverb_comb_set_buffer( reverb.combR[1], reverb.bufcombR2, CoreAudioTuning::combTuningR2 );
		reverb_comb_set_buffer( reverb.combL[2], reverb.bufcombL3, CoreAudioTuning::combTuningL3 );
		reverb_comb_set_buffer( reverb.combR[2], reverb.bufcombR3, CoreAudioTuning::combTuningR3 );
		reverb_comb_set_buffer( reverb.combL[3], reverb.bufcombL4, CoreAudioTuning::combTuningL4 );
		reverb_comb_set_buffer( reverb.combR[3], reverb.bufcombR4, CoreAudioTuning::combTuningR4 );
		reverb_comb_set_buffer( reverb.combL[4], reverb.bufcombL5, CoreAudioTuning::combTuningL5 );
		reverb_comb_set_buffer( reverb.combR[4], reverb.bufcombR5, CoreAudioTuning::combTuningR5 );
		reverb_comb_set_buffer( reverb.combL[5], reverb.bufcombL6, CoreAudioTuning::combTuningL6 );
		reverb_comb_set_buffer( reverb.combR[5], reverb.bufcombR6, CoreAudioTuning::combTuningR6 );
		reverb_comb_set_buffer( reverb.combL[6], reverb.bufcombL7, CoreAudioTuning::combTuningL7 );
		reverb_comb_set_buffer( reverb.combR[6], reverb.bufcombR7, CoreAudioTuning::combTuningR7 );
		reverb_comb_set_buffer( reverb.combL[7], reverb.bufcombL8, CoreAudioTuning::combTuningL8 );
		reverb_comb_set_buffer( reverb.combR[7], reverb.bufcombR8, CoreAudioTuning::combTuningR8 );

		reverb_allpass_set_buffer( reverb.allpassL[0], reverb.bufallpassL1, CoreAudioTuning::allpassTuningL1 );
		reverb_allpass_set_buffer( reverb.allpassR[0], reverb.bufallpassR1, CoreAudioTuning::allpassTuningR1 );
		reverb_allpass_set_buffer( reverb.allpassL[1], reverb.bufallpassL2, CoreAudioTuning::allpassTuningL2 );
		reverb_allpass_set_buffer( reverb.allpassR[1], reverb.bufallpassR2, CoreAudioTuning::allpassTuningR2 );
		reverb_allpass_set_buffer( reverb.allpassL[2], reverb.bufallpassL3, CoreAudioTuning::allpassTuningL3 );
		reverb_allpass_set_buffer( reverb.allpassR[2], reverb.bufallpassR3, CoreAudioTuning::allpassTuningR3 );
		reverb_allpass_set_buffer( reverb.allpassL[3], reverb.bufallpassL4, CoreAudioTuning::allpassTuningL4 );
		reverb_allpass_set_buffer( reverb.allpassR[3], reverb.bufallpassR4, CoreAudioTuning::allpassTuningR4 );

		// Set default values
		reverb.allpassL[0].feedback = 0.5f;
		reverb.allpassR[0].feedback = 0.5f;
		reverb.allpassL[1].feedback = 0.5f;
		reverb.allpassR[1].feedback = 0.5f;
		reverb.allpassL[2].feedback = 0.5f;
		reverb.allpassR[2].feedback = 0.5f;
		reverb.allpassL[3].feedback = 0.5f;
		reverb.allpassR[3].feedback = 0.5f;

		effect.set_parameter_default( CoreAudio::EffectParam_Reverb_Wet, CoreAudioTuning::initialWet );
		effect.set_parameter_default( CoreAudio::EffectParam_Reverb_RoomSize, CoreAudioTuning::initialRoom );
		effect.set_parameter_default( CoreAudio::EffectParam_Reverb_Dry, CoreAudioTuning::initialDry );
		effect.set_parameter_default( CoreAudio::EffectParam_Reverb_Damp, CoreAudioTuning::initialDamp );
		effect.set_parameter_default( CoreAudio::EffectParam_Reverb_Width, CoreAudioTuning::initialWidth );

		// Buffer will be full of rubbish - so we MUST mute them
		reverb_mute( effect, state );
	}
}


static void reverb_reset( CoreAudio::Effect &effect, CoreAudio::EffectState &state )
{
	effect.set_parameter_default( CoreAudio::EffectParam_Reverb_Wet, CoreAudioTuning::initialWet );
	effect.set_parameter_default( CoreAudio::EffectParam_Reverb_RoomSize, CoreAudioTuning::initialRoom );
	effect.set_parameter_default( CoreAudio::EffectParam_Reverb_Dry, CoreAudioTuning::initialDry );
	effect.set_parameter_default( CoreAudio::EffectParam_Reverb_Damp, CoreAudioTuning::initialDamp );
	effect.set_parameter_default( CoreAudio::EffectParam_Reverb_Width, CoreAudioTuning::initialWidth );
}


static void reverb_apply( CoreAudio::Bus &bus, CoreAudio::Effect &effect, CoreAudio::EffectState &state,
	float *samples, u32 sampleCount )
{
	CoreAudio::EffectStateReverb &reverb = state.stateReverb;

	for( u32 i = 0; i < sampleCount; i++ )
	{
		reverb_update( effect, state );

		float outL = 0.0f;
		float outR = 0.0f;
		float input = ( samples[i * 2 + 0] + samples[i * 2 + 1] ) * reverb.gain;

		// Accumulate comb filters in parallel
		for( int j = 0; j < CoreAudioTuning::numCombs; j++ )
		{
			outL += reverb_comb_process( reverb.combL[j], input );
			outR += reverb_comb_process( reverb.combR[j], input );
		}

		// Feed through allpasses in series
		for( int j = 0; j < CoreAudioTuning::numAllpasses; j++ )
		{
			outL = reverb_allpass_process( reverb.allpassL[j], outL );
			outR = reverb_allpass_process( reverb.allpassR[j], outR );
		}

		samples[i * 2 + 0] = outL * reverb.wet1 + outR * reverb.wet2 + samples[i * 2 + 0] * reverb.dry;
		samples[i * 2 + 1] = outR * reverb.wet1 + outL * reverb.wet2 + samples[i * 2 + 1] * reverb.dry;
	}
}


__AUDIO_EFFECT_PARAM_GET_SET_IMPL( CoreAudio::EffectType_Reverb, reverb_size, CoreAudio::EffectParam_Reverb_RoomSize )
__AUDIO_EFFECT_PARAM_GET_SET_IMPL( CoreAudio::EffectType_Reverb, reverb_wet, CoreAudio::EffectParam_Reverb_Wet )
__AUDIO_EFFECT_PARAM_GET_SET_IMPL( CoreAudio::EffectType_Reverb, reverb_dry, CoreAudio::EffectParam_Reverb_Dry )
__AUDIO_EFFECT_PARAM_GET_SET_IMPL( CoreAudio::EffectType_Reverb, reverb_width, CoreAudio::EffectParam_Reverb_Width )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct EffectFunctions
{
	void ( *init )( class CoreAudio::Effect &effect, CoreAudio::EffectState &state );
	void ( *reset )( class CoreAudio::Effect &effect, CoreAudio::EffectState &state );
	void ( *apply )( class CoreAudio::Bus &bus, class CoreAudio::Effect &effect, CoreAudio::EffectState &state,
		float *output, u32 frames );
};

static EffectFunctions effectFunctions[] =
{
	{ core_init, core_reset, core_apply },          // EffectType_Core
	{ spatial_init, spatial_reset, spatial_apply }, // EffectType_Spatial
 	{ lowpass_init, lowpass_reset, lowpass_apply }, // EffectType_Lowpass
	{ reverb_init, reverb_reset, reverb_apply },    // EffectType_Reverb
};

static_assert( ARRAY_LENGTH( effectFunctions ) == CoreAudio::EFFECTTYPE_COUNT, "Missing audio effect type" );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int find_bus()
{
	// Find first available bus
	for( int i = 0; i < AUDIO_BUS_COUNT; i++ )
	{
		if( CoreAudio::buses[i].available ) { return i; }
	}

	// Failure
	return -1;
}


static u16 find_voice()
{
	// Find first available voice
	for( u16 i = 0; i < AUDIO_VOICE_COUNT; i++ )
	{
		if( CoreAudio::voices[i].idBus < 0 ) { return i; }
	}

	// Failure
	return U16_MAX;
}


static u16 find_stream()
{
	// Find first available voice
	for( u16 i = 0; i < AUDIO_STREAM_COUNT; i++ )
	{
		if( CoreAudio::sounds[i].idBus < 0 ) { return i; }
	}

	// Failure
	return U16_MAX;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static THREAD_FUNCTION( audio_stream )
{
	for( ;; )
	{
		// For each stream
		for( int i = 0; i < AUDIO_STREAM_COUNT; i++ )
		{
			CoreAudio::Stream &stream = CoreAudio::sounds[i];
			if( stream.idBus < 0 ) { continue; }

			Assert( stream.assetID < CoreAssets::soundCount );
			Assets::SoundEntry const *binSound = &Assets::sound( stream.assetID );
			Assert( binSound->streamed );
			Assert( stream.channels == binSound->channels );

			// For each buffer
			for( int j = 0; j < AUDIO_STREAM_BUFFERS; j++ )
			{
				// This buffer is already filled and queued for mixing
				if( stream.ready & ( 1 << j ) ) { continue; }

				// Read the samples into the buffer
				const usize frames = min( static_cast<usize>( AUDIO_STREAM_BLOCK ),
					binSound->sampleCount - stream.streamPosition );

				// todo: Stream from file
				void *dst = reinterpret_cast<void *>( &CoreAudio::buffers[i][j] );
				void *src = reinterpret_cast<void *>( Assets::binary.data + CoreAssets::streamSampleDataOffset +
					( binSound->sampleOffset + stream.streamPosition ) * sizeof( i16 ) );
				memory_copy( dst, src, frames * sizeof( i16 ) );

				// If we're at the end of the stream, copy from the beginning to allow looping
				const usize extra = AUDIO_STREAM_BLOCK - frames;
				if( extra != 0 )
				{
					void *dst = reinterpret_cast<void *>( &CoreAudio::buffers[i][j][frames] );
					void *src = reinterpret_cast<void *>( Assets::binary.data + CoreAssets::streamSampleDataOffset +
						binSound->sampleOffset * sizeof( i16 ) );
					memory_copy( dst, src, extra * sizeof( i16 ) );

					stream.streamPosition = 0;
				}
				else
				{
					stream.streamPosition += AUDIO_STREAM_BLOCK;
				}

				// todo: Do we need to put up a memory fence here?
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
		// buffer underrun
		Thread::sleep( 4 );
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CoreAudio::init()
{
#if AUDIO_ENABLED
	// Initialize Audio Buses, Voices, & sounds
	for( int i = 0; i < AUDIO_BUS_COUNT; i++ ) { buses[i].init(); }
	for( int i = 0; i < AUDIO_VOICE_COUNT; i++ ) { voices[i].init(); }
	for( int i = 0; i < AUDIO_STREAM_COUNT; i++ ) { sounds[i].init(); }

	// Initialize Samples Buffer
	ErrorReturnIf( samples != nullptr, false, "Audio: samples buffer already initialized" );
	if constexpr ( CoreAssets::voiceSampleDataSize == 0 ) { return true; }

	samples = reinterpret_cast<i16 *>( memory_alloc( CoreAssets::voiceSampleDataSize ) );
	memory_copy( samples, &Assets::binary.data[CoreAssets::voiceSampleDataOffset], CoreAssets::voiceSampleDataSize );

	// Initialize Backend
	bool failure = !init_backend();
	ErrorReturnIf( failure, false, "Audio: failed to initialize audio backend" );

	// Initialize Streamer
	failure = ( Thread::create( audio_stream ) == nullptr );
	ErrorReturnIf( failure, false, "Audio: failed to create audio stream thread" );

	// Console
	CMD_DEBUG_ENABLED = Console::command_init( "debug audio <enable>", "Set audio debug mode", cmd_debug_enabled );
#endif

	return true;
}


bool CoreAudio::free()
{
#if AUDIO_ENABLED
	// Console
	Console::command_free( CMD_DEBUG_ENABLED );

	// Free Backend
	bool failure = !free_backend();
	ErrorReturnIf( failure, false, "Audio: failed to free audio backend" );

	// Free Samples
	if( samples != nullptr )
	{
		memory_free( samples );
		samples = nullptr;
	}

	// Free Voices
	for( int i = 0; i < AUDIO_VOICE_COUNT; i++ )
	{
		new ( &voices[i] ) Voice { };
	}

	// Free sounds
	for( int i = 0; i < AUDIO_STREAM_COUNT; i++ )
	{
		new ( &sounds[i] ) Stream { };
	}
#endif

	return true;
}


void CoreAudio::audio_mixer( i16 *output, u32 frames )
{
#if AUDIO_ENABLED
	constexpr int CHANNELS = 2;
	const usize framesBufferI16Size = frames * CHANNELS * sizeof( i16 );
	const usize framesBufferFloatSize = frames * CHANNELS * sizeof( float );

	// Allocate float buffer
	byte *buffer = reinterpret_cast<byte *>( memory_alloc( 3 * framesBufferFloatSize ) );
	float *bufferBus = reinterpret_cast<float *>( &buffer[0 * framesBufferFloatSize] );
	float *bufferLayer = reinterpret_cast<float *>( &buffer[1 * framesBufferFloatSize] );
	float *bufferMaster = reinterpret_cast<float *>( &buffer[2 * framesBufferFloatSize] );
	memory_set( bufferMaster, 0, framesBufferFloatSize );

	// Mix buses
	for( int i = 0; i < AUDIO_BUS_COUNT; i++ )
	{
		Bus &bus = buses[i];
		if( bus.available ) { continue; }

		memory_set( bufferBus, 0, framesBufferFloatSize );

		Assert( bus.effects[CoreAudio::EffectType_Core].active );
		const float pitchBus = bus.effects[CoreAudio::EffectType_Core].get_parameter(
			CoreAudio::EffectParam_Core_Pitch, false );

		// Mix voices
		for( int j = 0; j < AUDIO_VOICE_COUNT; j++ )
		{
			Voice &voice = voices[j];
			if( voice.idBus != i || voice.bypass ) { continue; }

			memory_set( bufferLayer, 0, framesBufferFloatSize );

			const u32 sampleCurrent = static_cast<u32>( voice.samplePosition );
			const u32 framesRemaining = ( voice.samplesCount - sampleCurrent ) / voice.channels;
			const u32 framesToMix = voice.description.loop ? frames : min( frames, framesRemaining );
			const u32 framesCount = voice.samplesCount / voice.channels;

			Assert( voice.effects[CoreAudio::EffectType_Core].active );
			const float pitch = voice.effects[CoreAudio::EffectType_Core].get_parameter(
				CoreAudio::EffectParam_Core_Pitch, false ) * pitchBus;

			// Read voice samples
			switch( voice.channels )
			{
				// Mono
				case 1:
				{
					for( u32 k = 0; k < framesToMix; k++ )
					{
						if( !voice.description.loop && voice.samplePosition >= framesCount ) { break; }

						const u32 sample = static_cast<u32>( voice.samplePosition );

						const u32 sampleThis = ( sample ); // 1 sample per frame
						const u32 sampleNext = ( voice.description.loop ?
							( ( sample + 1 ) % framesCount ) :
							( ( sample + 1 ) < framesCount ? ( sample + 1 ) : sample ) );

						const float valueThis = I16_TO_FLOAT( voice.samples[sampleThis] );
						const float valueNext = I16_TO_FLOAT( voice.samples[sampleNext] );

						const float lerp = voice.samplePosition - sample;
						const float valueMono = valueThis * ( 1.0f - lerp ) + valueNext * lerp;

						bufferLayer[k * 2 + 0] = valueMono;
						bufferLayer[k * 2 + 1] = valueMono;

						voice.samplePosition += pitch;
					}
				}
				break;

				// Stereo
				case 2:
				{
					for( u32 k = 0; k < framesToMix; k++ )
					{
						if( !voice.description.loop && voice.samplePosition >= framesCount ) { break; }
						const u32 sample = static_cast<u32>( voice.samplePosition );

						const u32 sampleThisLeft = ( sample ) * 2;
						u32 sampleNextLeft = ( voice.description.loop ?
							( ( sample + 1 ) % framesCount ) :
							( ( sample + 1 ) < framesCount ? ( sample + 1 ) : sample ) ) * 2;
						const u32 sampleThisRight = sampleThisLeft + 1;
						u32 sampleNextRight = sampleNextLeft + 1;

						if( !voice.description.loop )
						{
							if( UNLIKELY( sampleNextLeft >= voice.samplesCount ) )
							{
								sampleNextLeft = sampleThisLeft;
							}

							if( UNLIKELY( sampleNextRight >= voice.samplesCount ) )
							{
								sampleNextRight = sampleThisRight;
							}
						}

						const float valueThisLeft = I16_TO_FLOAT( voice.samples[sampleThisLeft] );
						const float valueNextLeft = I16_TO_FLOAT( voice.samples[sampleNextLeft] );
						const float valueThisRight = I16_TO_FLOAT( voice.samples[sampleThisRight] );
						const float valueNextRight = I16_TO_FLOAT( voice.samples[sampleNextRight] );

						const float lerp = voice.samplePosition - sample;
						const float valueLeft = valueThisLeft * ( 1.0f - lerp ) + valueNextLeft * lerp;
						const float valueRight = valueThisRight * ( 1.0f - lerp ) + valueNextRight * lerp;

						bufferLayer[k * 2 + 0] = valueLeft;
						bufferLayer[k * 2 + 1] = valueRight;

						voice.samplePosition += pitch;
					}
				}
				break;
			}

			// Process per-voice effects
			for( u32 k = 0; k < CoreAudio::EFFECTTYPE_COUNT; k++ )
			{
				Effect &effect = voice.effects[k];
				EffectState &state = voice.states[k];
				if( !effect.active ) { continue; }
				effectFunctions[k].apply( bus, effect, state, bufferLayer, frames );
			}

			// Write to bus
			for( u32 k = 0; k < framesToMix; k++ )
			{
				bufferBus[k * 2 + 0] += bufferLayer[k * 2 + 0];
				bufferBus[k * 2 + 1] += bufferLayer[k * 2 + 1];
			}

			// Loop
			if( voice.description.loop )
			{
				while( static_cast<u32>( voice.samplePosition ) >= framesCount - 1 )
				{
					voice.samplePosition -= framesCount;
				}
			}
			// End
			else if( static_cast<u32>( voice.samplePosition ) >= framesCount - 1 )
			{
				voice.idBus = -1;
			}
		}

		// Mix streams
		for( int j = 0; j < AUDIO_STREAM_COUNT; j++ )
		{
			Stream &stream = sounds[j];
			if( stream.idBus != i || stream.bypass ) { continue; }

			memory_set( bufferLayer, 0, framesBufferFloatSize );

			const u32 sampleCurrent = static_cast<u32>( stream.samplePosition );
			const u32 framesRemaining = ( stream.samplesCount - sampleCurrent ) / stream.channels;
			const u32 framesToMix = stream.description.loop ? frames : min( frames, framesRemaining );
			const u32 framesCount = stream.samplesCount / stream.channels;

			Assert( stream.effects[CoreAudio::EffectType_Core].active );
			const float pitch = stream.effects[CoreAudio::EffectType_Core].get_parameter(
				CoreAudio::EffectParam_Core_Pitch, false ) * pitchBus;

			// Read stream samples
			switch( stream.channels )
			{
				// Mono
				case 1:
				{
					// x / AUDIO_STREAM_BLOCK to x >> AUDIO_STREAM_BLOCK_DIV_EXPN;
					static constexpr u32 AUDIO_STREAM_BLOCK_DIV_EXPN = AUDIO_STREAM_BLOCK_EXPN;

					// x % AUDIO_STREAM_BLOCK to x & AUDIO_STREAM_BLOCK_MOD_MASK;
					static constexpr u32 AUDIO_STREAM_BLOCK_MOD_MASK = ( 1 << AUDIO_STREAM_BLOCK_DIV_EXPN ) - 1;

					const u32 buffer = 1 << ( ( sampleCurrent >> AUDIO_STREAM_BLOCK_DIV_EXPN ) % AUDIO_STREAM_BUFFERS );
					if( !( stream.ready & buffer ) ) { continue; }

					for( u32 k = 0; k < framesToMix; k++ )
					{
						if( !stream.description.loop && stream.samplePosition >= framesCount ) { break; }

						const u32 sample = static_cast<u32>( stream.samplePosition );
						const u32 frame = sample; // 1 sample per frame

						const u32 frameThis = frame;
						const u32 bufferThis = ( frameThis >> AUDIO_STREAM_BLOCK_DIV_EXPN ) % AUDIO_STREAM_BUFFERS;
						const u32 sampleThis = ( frameThis & AUDIO_STREAM_BLOCK_MOD_MASK );

						const u32 frameNext = frame + 1;
						const u32 bufferNext = ( frameNext >> AUDIO_STREAM_BLOCK_DIV_EXPN ) % AUDIO_STREAM_BUFFERS;
						const u32 sampleNext = ( frameNext & AUDIO_STREAM_BLOCK_MOD_MASK );

						const float valueThis = I16_TO_FLOAT( CoreAudio::buffers[j][bufferThis][sampleThis] );
						const float valueNext = I16_TO_FLOAT( CoreAudio::buffers[j][bufferNext][sampleNext] );

						const float lerp = stream.samplePosition - sample;
						const float valueMono = valueThis * ( 1.0f - lerp ) + valueNext * lerp;

						bufferLayer[k * 2 + 0] = valueMono;
						bufferLayer[k * 2 + 1] = valueMono;

						stream.samplePosition += pitch;

						const u32 buffer0 = bufferThis;
						const u32 buffer1 = ( static_cast<u32>( stream.samplePosition ) >> AUDIO_STREAM_BLOCK_DIV_EXPN ) %
							AUDIO_STREAM_BUFFERS;
						if( buffer0 != buffer1 ) { stream.ready &= ~( 1 << buffer0 ); }
					}
				}
				break;

				// Stereo
				case 2:
				{
					// x / ( AUDIO_STREAM_BLOCK / 2 ) to x >> AUDIO_STREAM_BLOCK_DIV_EXPN;
					static constexpr u32 AUDIO_STREAM_BLOCK_DIV_EXPN = AUDIO_STREAM_BLOCK_EXPN - 1;

					// x % ( AUDIO_STREAM_BLOCK / 2 ) to x & AUDIO_STREAM_BLOCK_MOD_MASK;
					static constexpr u32 AUDIO_STREAM_BLOCK_MOD_MASK = ( 1 << AUDIO_STREAM_BLOCK_DIV_EXPN ) - 1;

					const u32 buffer = 1 << ( ( sampleCurrent >> AUDIO_STREAM_BLOCK_DIV_EXPN ) % AUDIO_STREAM_BUFFERS );
					if( !( stream.ready & buffer ) ) { continue; }

					for( u32 k = 0; k < framesToMix; k++ )
					{
						if( !stream.description.loop && stream.samplePosition >= framesCount ) { break; }

						const u32 sample = static_cast<u32>( stream.samplePosition );

						const u32 sampleThis = sample;
						const u32 sampleThisLeft = ( ( sampleThis & AUDIO_STREAM_BLOCK_MOD_MASK ) << 1 ) + 0;
						const u32 sampleThisRight = ( ( sampleThis & AUDIO_STREAM_BLOCK_MOD_MASK ) << 1 ) + 1;
						const u32 bufferThis = ( ( sampleThis >> AUDIO_STREAM_BLOCK_DIV_EXPN ) % AUDIO_STREAM_BUFFERS );

						const u32 sampleNext = sample + 1;
						const u32 sampleNextLeft = ( ( sampleNext & AUDIO_STREAM_BLOCK_MOD_MASK ) << 1 ) + 0;
						const u32 sampleNextRight = ( ( sampleNext & AUDIO_STREAM_BLOCK_MOD_MASK ) << 1 ) + 1;
						const u32 bufferNext = ( ( sampleNext >> AUDIO_STREAM_BLOCK_DIV_EXPN ) % AUDIO_STREAM_BUFFERS );

						const float valueThisLeft = I16_TO_FLOAT( CoreAudio::buffers[j][bufferThis][sampleThisLeft] );
						const float valueThisRight = I16_TO_FLOAT( CoreAudio::buffers[j][bufferThis][sampleThisRight] );
						const float valueNextLeft = I16_TO_FLOAT( CoreAudio::buffers[j][bufferNext][sampleNextLeft] );
						const float valueNextRight = I16_TO_FLOAT( CoreAudio::buffers[j][bufferNext][sampleNextRight] );

						const float lerp = stream.samplePosition - sample;
						const float valueLeft = valueThisLeft * ( 1.0f - lerp ) + valueNextLeft * lerp;
						const float valueRight = valueThisRight * ( 1.0f - lerp ) + valueNextRight * lerp;

						bufferLayer[k * 2 + 0] = valueLeft;
						bufferLayer[k * 2 + 1] = valueRight;

						stream.samplePosition += pitch;

						const u32 buffer0 = bufferThis;
						const u32 buffer1 = ( ( static_cast<u32>( stream.samplePosition ) >>
							AUDIO_STREAM_BLOCK_DIV_EXPN ) % AUDIO_STREAM_BUFFERS );
						if( buffer0 != buffer1 ) { stream.ready &= ~( 1 << buffer0 ); }
					}
				}
				break;
			}

			// Process per-stream effects
			for( u32 k = 0; k < CoreAudio::EFFECTTYPE_COUNT; k++ )
			{
				Effect &effect = stream.effects[k];
				EffectState &state = stream.states[k];
				if( !effect.active ) { continue; }
				effectFunctions[k].apply( bus, effect, state, bufferLayer, frames );
			}

			// Write to bus
			for( u32 k = 0; k < framesToMix; k++ )
			{
				bufferBus[k * 2 + 0] += bufferLayer[k * 2 + 0];
				bufferBus[k * 2 + 1] += bufferLayer[k * 2 + 1];
			}

			// End Condition
			if( stream.samplePosition >= framesCount - 1 )
			{
				// Loop
				if( stream.description.loop )
				{
					while( stream.samplePosition >= framesCount - 1 ) { stream.samplePosition -= framesCount; }
				}
				// End
				else
				{
					stream.idBus = -1;
				}
			}
		}

		// Process per-bus effects
		for( u32 j = 0; j < CoreAudio::EFFECTTYPE_COUNT; j++ )
		{
			Effect &effect = bus.effects[j];
			EffectState &state = bus.states[j];
			if( !effect.active ) { continue; };
			effectFunctions[j].apply( bus, effect, state, bufferBus, frames );
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CoreAudio::Bus::init()
{
	available = true;
	bypass = false;
	name = "";

	lookX = 0.0f;
	lookY = 0.0f;
	lookZ = 1.0f;
	upX = 0.0f;
	upY = -1.0f;
	upZ = 0.0f;

	new ( &effects ) AudioEffects { };
	for( int i = 0; i < CoreAudio::EFFECTTYPE_COUNT; i++ )
	{
		memory_set( &states[i], 0, sizeof( states[i] ) );
		if( effectFunctions[i].init == nullptr ) { continue; }
		effectFunctions[i].init( effects[i], states[i] );
	}
}


void CoreAudio::Voice::init()
{
	idBus = -1;
	generation = 0;
	bypass = false;
	name = "";

	channels = 1;
	samplePosition = 0.0f;
	samples = nullptr;
	samplesCount = 0;

	new ( &description ) AudioDescription { };
	new ( &effects ) AudioEffects { };
	for( int i = 0; i < CoreAudio::EFFECTTYPE_COUNT; i++ )
	{
		memory_set( &states[i], 0, sizeof( states[i] ) );
		if( effectFunctions[i].init == nullptr ) { continue; }
		effectFunctions[i].init( effects[i], states[i] );
	}
};


void CoreAudio::Stream::init()
{
	idBus = -1;
	generation = 0;
	bypass = false;
	name = "";

	streamPosition = 0;
	ready = 0;

	channels = 1;
	samplePosition = 0.0f;
	samples = nullptr;
	samplesCount = 0;

	new ( &description ) AudioDescription { };
	new ( &effects ) AudioEffects { };
	for( int i = 0; i < CoreAudio::EFFECTTYPE_COUNT; i++ )
	{
		memory_set( &states[i], 0, sizeof( states[i] ) );
		if( effectFunctions[i].init == nullptr ) { continue; }
		effectFunctions[i].init( effects[i], states[i] );
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SoundHandle CoreAudio::play_voice( const int idBus, const i16 *const samples, const u32 samplesCount,
	const int channels, const AudioEffects &effects, const AudioDescription &description, const char *name )
{
	const u16 v = find_voice();
	if( v == U16_MAX ) { return SoundHandle { }; }
	CoreAudio::Voice &voice = CoreAudio::voices[v];

	// Effects
	memory_copy( &voice.effects, &effects, sizeof( voice.effects ) );
	for( int i = 0; i < CoreAudio::EFFECTTYPE_COUNT; i++ )
	{
		memory_set( &voice.states[i], 0, sizeof( voice.states[i] ) );
		if( effectFunctions[i].init == nullptr ) { continue; }
		effectFunctions[i].init( voice.effects[i], voice.states[i] );
		if( effectFunctions[i].reset == nullptr ) { continue; }
		effectFunctions[i].reset( voice.effects[i], voice.states[i] );
	}

	// Description
	memory_copy( &voice.description, &description, sizeof( voice.description ) );

	// State
	voice.samplePosition = description.startTimeRandomize ?
		CoreAudio::random.next_float( samplesCount ) : description.startTimeMS;
	voice.channels = channels;
	voice.samples = samples;
	voice.samplesCount = samplesCount;
#if COMPILE_DEBUG
	voice.name = name;
#endif

	// TODO: Thread/Compile Barrier here
	voice.generation++;
	voice.idBus = idBus;

	// Return handle
	return SoundHandle { static_cast<u16>( v ), U16_MAX, voice.generation };
}


SoundHandle CoreAudio::play_stream( const int idBus, const u32 assetID,
	const AudioEffects &effects, const AudioDescription &description, const char *name )
{
	const u16 s = find_stream();
	if( s == U16_MAX ) { return SoundHandle { }; }
	CoreAudio::Stream &stream = CoreAudio::sounds[s];

	Assert( assetID < CoreAssets::soundCount );
	const Assets::SoundEntry &binSound = Assets::sound( assetID );
	Assert( binSound.streamed );

	// Effects
	memory_copy( &stream.effects, &effects, sizeof( stream.effects ) );
	for( int i = 0; i < CoreAudio::EFFECTTYPE_COUNT; i++ )
	{
		memory_set( &stream.states[i], 0, sizeof( stream.states[i] ) );
		if( effectFunctions[i].init == nullptr ) { continue; }
		effectFunctions[i].init( stream.effects[i], stream.states[i] );
		if( effectFunctions[i].reset == nullptr ) { continue; }
		effectFunctions[i].reset( stream.effects[i], stream.states[i] );
	}

	// Description
	memory_copy( &stream.description, &description, sizeof( stream.description ) );

	// State
	stream.channels = binSound.channels;
	stream.samplePosition = description.startTimeRandomize ?
		CoreAudio::random.next_float( binSound.sampleCount ) : description.startTimeMS;
	stream.samplesCount = binSound.sampleCount;

	stream.assetID = assetID;
	stream.streamPosition = ( static_cast<usize>( stream.samplePosition ) / AUDIO_STREAM_BLOCK ) *
		AUDIO_STREAM_BLOCK;
	stream.ready = 0;

#if COMPILE_DEBUG
	stream.name = name;
#endif

	// TODO: thread barrier here?
	stream.generation++;
	stream.idBus = idBus;

	// Return handle
	return SoundHandle { U16_MAX, static_cast<u16>( s ), stream.generation };
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AudioEffects::init()
{
	// Zero-initialize Effects
	for( int i = 0; i < CoreAudio::EFFECTTYPE_COUNT; i++ )
	{
		memory_set( &effects[i], 0, sizeof( effects[i] ) );
	}

	// First effect is always AudioEffect_Core
	effects[CoreAudio::EffectType_Core].active = true;
	effects[CoreAudio::EffectType_Core].set_parameter( CoreAudio::EffectParam_Core_Gain, 1.0f, 0.0f );
	effects[CoreAudio::EffectType_Core].set_parameter( CoreAudio::EffectParam_Core_Pitch, 1.0f, 0.0f );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SoundHandle::is_playing() const
{
	if( idStream == U16_MAX )
	{
		if( idVoice >= AUDIO_VOICE_COUNT ) { return false; }
		if( CoreAudio::voices[idVoice].generation != generation ) { return false; }
		return !( CoreAudio::voices[idVoice].idBus < 0 );
	}
	else if( idVoice == U16_MAX )
	{
		if( idStream >= AUDIO_STREAM_COUNT ) { return false; }
		if( CoreAudio::sounds[idStream].generation != generation ) { return false; }
		return !( CoreAudio::sounds[idStream].idBus < 0 );
	}

	return false;
}


bool SoundHandle::is_paused() const
{
	if( !is_playing() ) { return false; }
	return idStream == U16_MAX ? CoreAudio::voices[idVoice].bypass : CoreAudio::sounds[idStream].bypass;
}


bool SoundHandle::pause( const bool pause ) const
{
	if( !is_playing() ) { return false; }

	if( idVoice != U16_MAX ) { CoreAudio::voices[idVoice].bypass = pause; }
	if( idStream != U16_MAX ) { CoreAudio::sounds[idStream].bypass = pause; }

	return true;
}


bool SoundHandle::stop() const
{
	if( !is_playing() ) { return true; }

	if( idVoice != U16_MAX ) { CoreAudio::voices[idVoice].idBus = -1; }
	if( idStream != U16_MAX ) { CoreAudio::sounds[idStream].idBus = -1; }

	return true;
}


AudioEffects *SoundHandle::operator->() const
{
	if( !is_playing() ) { return &NULL_AUDIO_EFFECTS; }
	return idStream == U16_MAX ? &CoreAudio::voices[idVoice].effects : &CoreAudio::sounds[idStream].effects;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AudioContext::init( const AudioEffects &effects, const char *name )
{
	// Reserve an audio bus
	idBus = find_bus();
	ErrorIf( idBus < 0, "AudioContext: exceeded bus capacity!" );
	CoreAudio::Bus &bus = CoreAudio::buses[idBus];
	bus.available = false;
	this->name = name;
	bus.name = name;

	// Copy Effects
	memory_copy( &bus.effects, &effects, sizeof( AudioEffects ) );
	for( int i = 0; i < CoreAudio::EFFECTTYPE_COUNT; i++ )
	{
		memory_set( &bus.states[i], 0, sizeof( bus.states[i] ) );
		if( effectFunctions[i].init == nullptr ) { continue; }
		effectFunctions[i].init( bus.effects[i], bus.states[i] );
	}
};


void AudioContext::free()
{
	if( idBus < 0 || idBus >= AUDIO_BUS_COUNT ) { return; }

	// Stop playing voices
	for( int i = 0; i < AUDIO_VOICE_COUNT; i++ )
	{
		CoreAudio::Voice &voice = CoreAudio::voices[i];
		if( voice.idBus == idBus ) { voice.idBus = -1; }
	}

	// Mark our audio bus as available
	CoreAudio::Bus &bus = CoreAudio::buses[idBus];
	bus.available = true;
};


AudioEffects *AudioContext::operator->() const
{
	Assert( idBus > 0 || idBus < AUDIO_BUS_COUNT );
	return &CoreAudio::buses[idBus].effects;
}


bool AudioContext::is_paused() const
{
	if( idBus < 0 || idBus >= AUDIO_BUS_COUNT ) { return false; }
	return CoreAudio::buses[idBus].bypass;
}


bool AudioContext::pause( const bool pause ) const
{
	if( idBus < 0 || idBus >= AUDIO_BUS_COUNT ) { return false; }
	CoreAudio::buses[idBus].bypass = pause;
	return true;
}


void AudioContext::set_listener( const float lookX, const float lookY, const float lookZ,
	const float upX, const float upY, const float upZ )
{
	if( idBus < 0 || idBus >= AUDIO_BUS_COUNT ) { return; }
	CoreAudio::buses[idBus].lookX = lookX;
	CoreAudio::buses[idBus].lookY = lookY;
	CoreAudio::buses[idBus].lookZ = lookZ;
	CoreAudio::buses[idBus].upX = upX;
	CoreAudio::buses[idBus].upY = upY;
	CoreAudio::buses[idBus].upZ = upZ;
}


SoundHandle AudioContext::play_sound( const u32 sound,
	const AudioEffects &effects, const AudioDescription &description )
{
	Assert( idBus >= 0 || idBus < AUDIO_BUS_COUNT );

	Assert( sound < CoreAssets::soundCount );
	const Assets::SoundEntry &binSound = Assets::sound( sound );

#if COMPILE_DEBUG
	const char *name = binSound.name;
#else
	const char *name = "";
#endif

	if( binSound.streamed )
	{
		return CoreAudio::play_stream( idBus, sound, effects, description, name );
	}
	else
	{
		const i16 *const samples = &CoreAudio::samples[binSound.sampleOffset];
		const u32 samplesCount = binSound.sampleCount;
		const int channels = binSound.channels;
		return CoreAudio::play_voice( idBus, samples, samplesCount, channels, effects, description, name );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
	const int_v2 labelDimensions = text_dimensions( font, fontSizeLabel, label );
	const int_v2 valueDimensions = text_dimensions( font, fontSizeLabel, value );

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


int_v2 CoreAudio::draw_debug( const Delta delta, const float x, const float y )
{
	int_v2 dimensions = int_v2 { 0, 0 };
	if( !DEBUG_ENABLED ) { return dimensions; }

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


bool CoreAudio::draw_bus( const Delta delta, const int id, float &x, float &y )
{
	Bus &bus = buses[id];
	if( bus.available ) { return false; }
	const float xIn = x;
	const float yIn = y;

	// Bus Label
	const char *labelFormat = bus.name[0] == '\0' ? "Bus %d" : "Bus %d: ";
	const int_v2 labelDimensions = text_dimensions_f( font, fontSizeTitle, labelFormat, id );
	draw_text_f( font, fontSizeTitle, x, y, c_white, labelFormat, id );
	draw_text( font, fontSizeTitle, x + labelDimensions.x, y, c_yellow, bus.name );
	y += 20.0f;

	// Effects
	x += 16.0f;
	for( int i = 0; i < CoreAudio::EFFECTTYPE_COUNT; i++ )
	{
		draw_effect( delta, bus.effects[i], i, x, y );
	}
	x -= 16.0f;
	y += 4.0f;

	// Voices
	x += 16.0f;
	{
		bool active = false;

		for( int i = 0; i < AUDIO_STREAM_COUNT; i++ )
		{
			const Stream &stream = sounds[i];
			if( stream.idBus != id ) { continue; }
			active |= draw_stream( delta, i, x, y );
			y += 4.0f;
		}

		for( int i = 0; i < AUDIO_VOICE_COUNT; i++ )
		{
			const Voice &voice = voices[i];
			if( voice.idBus != id ) { continue; }
			active |= draw_voice( delta, i, x, y );
			y += 4.0f;
		}

		if( !active )
		{
			draw_text( font, fontSizeLabel, x, y, c_gray, "No Active Voices/sounds" );
			y += 20.0f;
		}
	}
	x = xIn;

	return true;
}


bool CoreAudio::draw_voice( const Delta delta, const int id, float &x, float &y )
{
	Voice &voice = voices[id];
	if( voice.idBus < 0 ) { return false; }

	// Widget State
	const int widgetWidth = 192;

	// Voice Label
	const char *labelFormat = voice.name[0] == '\0' ? "v%d" : "v%d: ";
	const int_v2 labelDimensions = text_dimensions_f( font, fontSizeLabel, labelFormat, id );
	draw_text_f( font, fontSizeLabel, x, y, c_white, labelFormat, id );
	draw_text( font, fontSizeLabel, x + labelDimensions.x, y, c_yellow, voice.name );
	y += 16.0f;

	// Progress Bar
	const float progress = ( voice.samplePosition / voice.samplesCount ) * voice.channels;
	draw_progress_bar( x, y, widgetWidth, 10.0f, progress, c_dkgray, c_white );
	y += 16.0f;

	// Effects
	for( int i = 0; i < CoreAudio::EFFECTTYPE_COUNT; i++ )
	{
		draw_effect( delta, voice.effects[i], i, x, y );
	}
	y += 4.0f;

	return true;
}


bool CoreAudio::draw_stream( const Delta delta, const int id, float &x, float &y )
{
	Stream &stream = sounds[id];
	if( stream.idBus < 0 ) { return false; }

	// Widget State
	const int widgetWidth = 192;

	// Voice Label
	const char *labelFormat = stream.name[0] == '\0' ? "s%d" : "s%d: ";
	const int_v2 labelDimensions = text_dimensions_f( font, fontSizeLabel, labelFormat, id );
	draw_text_f( font, fontSizeLabel, x, y, c_white, labelFormat, id );
	draw_text( font, fontSizeLabel, x + labelDimensions.x, y, c_yellow, stream.name );
	y += 16.0f;

	// Progress Bar
	const float progress = ( stream.samplePosition / stream.samplesCount ) * stream.channels;
	draw_progress_bar( x, y, widgetWidth, 10.0f, progress, c_dkgray, c_white );
	y += 10.0f;

	// Buffers
	for( int i = 0; i < AUDIO_STREAM_BUFFERS; i++ )
	{
		const int bufferWidth = widgetWidth / AUDIO_STREAM_BUFFERS;

		const u32 frame = static_cast<u32>( stream.samplePosition ) * stream.channels;
		const int buffer = frame / AUDIO_STREAM_BLOCK % AUDIO_STREAM_BUFFERS;
		const int sample = frame % AUDIO_STREAM_BLOCK;

		const float precent = buffer == i ? ( static_cast<float>( sample ) / AUDIO_STREAM_BLOCK ) : 0.0f;
		const Color color = ( stream.ready & ( 1 << i ) ? c_green : c_red ) * ( buffer == i ? c_white : c_gray );

		draw_progress_bar( x + bufferWidth * i, y, bufferWidth, 10, precent, c_gray * color, color );
	}
	y += 16.0f;

	// Effects
	for( int i = 0; i < CoreAudio::EFFECTTYPE_COUNT; i++ )
	{
		draw_effect( delta, stream.effects[i], i, x, y );
	}
	y += 4.0f;

	return true;
}


bool CoreAudio::draw_effect( const Delta delta, Effect &effect, const int type, float &x, float &y )
{
	if( !effect.active ) { return false; }

	const int labelIndent = 64;
	const int labelWidth = 128;

	// Helper Lambda
	char value[16];
	auto draw_value_label = [ &value, &effect, &x, &y ]( const char *label, const CoreAudio::EffectParam param )
	{
		snprintf( value, 16, "%.2f", effect.get_parameter( param ) );
		audio_draw_label_value( x, y, labelWidth, colorLabel, colorSeparator, colorValue, label, value );
		y += 16.0f;
	};

	// Draw Effect Labels
	switch( type )
	{
		case CoreAudio::EffectType_Core:
		{
			draw_text( font, fontSizeLabel, x, y, c_white, "Core" );
			x += labelIndent;
			draw_value_label( "Gain", CoreAudio::EffectParam_Core_Gain );
			draw_value_label( "Pitch", CoreAudio::EffectParam_Core_Pitch );
			x -= labelIndent;
		}
		return true;

		case CoreAudio::EffectType_Lowpass:
		{
			draw_text( font, fontSizeLabel, x, y, c_white, "Lowpass" );
			x += labelIndent;
			draw_value_label( "Cutoff", CoreAudio::EffectParam_Lowpass_Cutoff );
			x -= labelIndent;
		}
		return true;

		case CoreAudio::EffectType_Reverb:
		{
			draw_text( font, fontSizeLabel, x, y, c_white, "Reverb" );
			x += labelIndent;
			draw_value_label( "Room Size", CoreAudio::EffectParam_Reverb_RoomSize );
			draw_value_label( "Wet", CoreAudio::EffectParam_Reverb_Wet );
			draw_value_label( "Dry", CoreAudio::EffectParam_Reverb_Dry );
			draw_value_label( "Width", CoreAudio::EffectParam_Reverb_Width );
			x -= labelIndent;
		}
		return true;

		default: AssertMsg( true, "Unsupported audio effect debug label!" ); return false;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////