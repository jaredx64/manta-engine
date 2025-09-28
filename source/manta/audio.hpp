#pragma once

#include <core/types.hpp>
#include <core/debug.hpp>

#include <manta/audio.tuning.hpp>

#include <manta/vector.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if !defined( HEADLESS )
	#define AUDIO_ENABLED ( true )
#else
	#define AUDIO_ENABLED ( false )
#endif

#define AUDIO_STREAM_BUFFERS ( 2 )

#define AUDIO_STREAM_BLOCK ( 8192 )
#define AUDIO_STREAM_BLOCK_EXPN ( 13 )
static_assert( ( 1 << AUDIO_STREAM_BLOCK_EXPN ) == AUDIO_STREAM_BLOCK,
	"Mismatched AUDIO_STREAM_BLOCK_EXPN!" );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace CoreAudio
{
	extern class Voice voices[AUDIO_VOICE_COUNT];
	extern class Stream streams[AUDIO_STREAM_COUNT];
	extern class Bus buses[AUDIO_BUS_COUNT];

	extern bool init();
	extern bool free();
	extern bool init_backend();
	extern bool free_backend();
	extern void audio_mixer( i16 *output, u32 frames );

	extern int_v2 draw_debug( const Delta delta, const float x, const float y );
	extern bool draw_bus( const Delta delta, const int id, float &x, float &y );
	extern bool draw_voice( const Delta delta, const int id, float &x, float &y );
	extern bool draw_stream( const Delta delta, const int id, float &x, float &y );
	extern bool draw_effect( const Delta delta, class Effect &effect, const int type,
		float &x, float &y );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace CoreAudio
{
	constexpr usize EFFECTPARAM_COUNT_MAX = 8;

	// Gain
	enum_type( EffectParam, int )
	{
		EffectParam_Core_Gain,
		EffectParam_Core_Pitch,
		EFFECTPARAM_CORE_COUNT,
	};
	static_assert( EFFECTPARAM_CORE_COUNT <= EFFECTPARAM_COUNT_MAX );

	// Spatial
	enum_type( EffectParam, int )
	{
		EffectParam_Spatial_X,
		EffectParam_Spatial_Y,
		EffectParam_Spatial_Z,
		EffectParam_Spatial_Attenuation,
		EFFECTPARAM_SPATIAL_COUNT,
	};
	static_assert( EFFECTPARAM_SPATIAL_COUNT <= EFFECTPARAM_COUNT_MAX );

	// Lowpass
	enum_type( EffectParam, int )
	{
		EffectParam_Lowpass_Cutoff,
		EFFECTPARAM_LOWPASS_COUNT,
	};
	static_assert( EFFECTPARAM_LOWPASS_COUNT <= EFFECTPARAM_COUNT_MAX );

	// Reverb
	enum_type( EffectParam, int )
	{
		EffectParam_Reverb_RoomSize,
		EffectParam_Reverb_Wet,
		EffectParam_Reverb_Dry,
		EffectParam_Reverb_Damp,
		EffectParam_Reverb_Width,
		EFFECTPARAM_REVERB_COUNT,
	};
	static_assert( EFFECTPARAM_REVERB_COUNT <= EFFECTPARAM_COUNT_MAX );

	// Effects
	enum_type( EffectType, int )
	{
		EffectType_Core,
		EffectType_Spatial,
		EffectType_Lowpass,
		EffectType_Reverb,
		EFFECTTYPE_COUNT,
	};
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace CoreAudio
{
	class Effect
	{
	public:
		float get_parameter( const EffectParam param, const bool incrementTime = false );
		void set_parameter_default( const EffectParam param, const float value );
		void set_parameter( const EffectParam param, const float value );
		void set_parameter( const EffectParam param, const float value, const usize timeMS );
		void set_parameter( const EffectParam param, const float valueFrom, const float valueTo, const usize timeMS );

	public:
		struct Parameter
		{
			float valueFrom, valueTo;
			float sampleCurrent, sampleToInv;
		};

		bool active;
		bool bypass;
		Parameter parameters[EFFECTPARAM_COUNT_MAX];
	};


	struct EffectStateCore
	{
		// ...
	};


	struct EffectStateSpatial
	{
		float x, y, z;
		float attenuation;
		float gainLeft;
		float gainRight;
	};


	struct EffectStateLowpass
	{
		float x[2][LPF_ORDER + 1]; // Raw values
		float y[2][LPF_ORDER + 1]; // Filtered values
		float a[2][LPF_ORDER];
		float b[2][LPF_ORDER + 1];
		float omega0;
		float dt;
	};


	struct EffectStateReverbComb
	{
		float feedback;
		float filterstore;
		float damp1;
		float damp2;
		float *buffer;
		int bufsize;
		int bufidx;
	};


	struct EffectStateReverbAllPass
	{
		float feedback;
		float *buffer;
		int bufsize;
		int bufidx;
	};


	struct EffectStateReverb
	{
		float gain;
		float roomsize, roomsize1;
		float damp, damp1;
		float wet, wet1, wet2;
		float dry;
		float width;
		float mode;

		// EffectStateReverbComb Filter
		EffectStateReverbComb combL[CoreAudioTuning::numCombs];
		EffectStateReverbComb combR[CoreAudioTuning::numCombs];

		// Allpass Filter
		EffectStateReverbAllPass allpassL[CoreAudioTuning::numAllpasses];
		EffectStateReverbAllPass allpassR[CoreAudioTuning::numAllpasses];

		// Buffers for the combs
		float bufcombL1[CoreAudioTuning::combTuningL1];
		float bufcombR1[CoreAudioTuning::combTuningR1];
		float bufcombL2[CoreAudioTuning::combTuningL2];
		float bufcombR2[CoreAudioTuning::combTuningR2];
		float bufcombL3[CoreAudioTuning::combTuningL3];
		float bufcombR3[CoreAudioTuning::combTuningR3];
		float bufcombL4[CoreAudioTuning::combTuningL4];
		float bufcombR4[CoreAudioTuning::combTuningR4];
		float bufcombL5[CoreAudioTuning::combTuningL5];
		float bufcombR5[CoreAudioTuning::combTuningR5];
		float bufcombL6[CoreAudioTuning::combTuningL6];
		float bufcombR6[CoreAudioTuning::combTuningR6];
		float bufcombL7[CoreAudioTuning::combTuningL7];
		float bufcombR7[CoreAudioTuning::combTuningR7];
		float bufcombL8[CoreAudioTuning::combTuningL8];
		float bufcombR8[CoreAudioTuning::combTuningR8];

		// Buffers for the allpasses
		float bufallpassL1[CoreAudioTuning::allpassTuningL1];
		float bufallpassR1[CoreAudioTuning::allpassTuningR1];
		float bufallpassL2[CoreAudioTuning::allpassTuningL2];
		float bufallpassR2[CoreAudioTuning::allpassTuningR2];
		float bufallpassL3[CoreAudioTuning::allpassTuningL3];
		float bufallpassR3[CoreAudioTuning::allpassTuningR3];
		float bufallpassL4[CoreAudioTuning::allpassTuningL4];
		float bufallpassR4[CoreAudioTuning::allpassTuningR4];
	};

	struct EffectState
	{
		union
		{
			EffectStateCore stateGain;
			EffectStateSpatial stateSpatial;
			EffectStateLowpass stateLowpass;
			EffectStateReverb stateReverb;
		};
	};
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define __AUDIO_EFFECT_PARAM_GET_SET_DECL(name) \
	void set_##name( const float value, const usize timeMS = 0 ); \
	void set_##name( const float valueFrom, const float valueTo, const usize timeMS ); \
	float get_##name();

#define __AUDIO_EFFECT_PARAM_GET_SET_NOAUTOMATION_DECL(name) \
	void set_##name( const float value ); \
	float get_##name();

#define __AUDIO_EFFECT_PARAM_GET_SET_IMPL(effectType,name,effectParam) \
	void AudioEffects::set_##name( const float value, const usize timeMS ) \
	{ \
		effects[effectType].set_parameter( effectParam, value, timeMS ); \
	} \
	void AudioEffects::set_##name( const float valueFrom, const float valueTo, const usize timeMS ) \
	{ \
		effects[effectType].set_parameter( effectParam, valueFrom, valueTo, timeMS ); \
	} \
	float AudioEffects::get_##name() \
	{ \
		return effects[effectType].get_parameter( effectParam, false ); \
	}

#define __AUDIO_EFFECT_PARAM_GET_SET_NOAUTOMATION_IMPL(effectType,name,effectParam) \
	void AudioEffects::set_##name( const float value ) \
	{ \
		effects[effectType].set_parameter( effectParam, value, 0.0f ); \
	} \
	float AudioEffects::get_##name() \
	{ \
		return effects[effectType].get_parameter( effectParam, false ); \
	}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class AudioEffects
{
public:
	AudioEffects() { init(); }
    CoreAudio::Effect &operator[]( const usize index ) { return effects[index]; }
	const CoreAudio::Effect &operator[]( const usize index ) const { return effects[index]; }

public:
	// EffectType_Core
	__AUDIO_EFFECT_PARAM_GET_SET_DECL( gain )
	__AUDIO_EFFECT_PARAM_GET_SET_NOAUTOMATION_DECL( pitch )

	// EffectType_Spatial
	__AUDIO_EFFECT_PARAM_GET_SET_DECL( spatial_x )
	__AUDIO_EFFECT_PARAM_GET_SET_DECL( spatial_y )
	__AUDIO_EFFECT_PARAM_GET_SET_DECL( spatial_z )
	__AUDIO_EFFECT_PARAM_GET_SET_DECL( spatial_attenuation )

	// EffectType_Lowpass
	__AUDIO_EFFECT_PARAM_GET_SET_DECL( lowpass_cutoff )

	// EffectType_Reverb
	__AUDIO_EFFECT_PARAM_GET_SET_DECL( reverb_size )
	__AUDIO_EFFECT_PARAM_GET_SET_DECL( reverb_wet )
	__AUDIO_EFFECT_PARAM_GET_SET_DECL( reverb_dry )
	//__AUDIO_EFFECT_PARAM_GET_SET_DECL( reverb_damp )
	__AUDIO_EFFECT_PARAM_GET_SET_DECL( reverb_width )

private:
	void init();
	CoreAudio::Effect effects[CoreAudio::EFFECTTYPE_COUNT];
};


struct AudioDescription
{
	bool loop = false;
	bool startTimeRandomize = false;
	usize startTimeMS = 0;
};


class SoundHandle
{
public:
	SoundHandle() : idVoice{ U16_MAX }, idStream{ U16_MAX }, generation { U32_MAX } { };
	SoundHandle( const u16 idVoice, const u16 idStream, const u32 generation ) :
		idVoice{ idVoice }, idStream{ idStream }, generation{ generation } { };
	SoundHandle( const SoundHandle &other ) :
		idVoice{ other.idVoice }, idStream { other.idStream }, generation { other.generation } { };

	bool is_playing() const;
	bool is_paused() const;
	bool pause( const bool pause ) const;
	bool stop() const;

	AudioEffects *operator->() const;

public:
	u16 idVoice = U16_MAX;
	u16 idStream = U16_MAX;
	u32 generation = U32_MAX;
};


class AudioContext
{
public:
	AudioContext() : idBus{ -1 } { };

	void init( const AudioEffects &effects = { }, const char *name = "" );
	void free();

	bool is_paused() const;
	bool pause( const bool pause ) const;

	void set_listener( const float lookX, const float lookY, const float lookZ,
		const float upX, const float upY, const float upZ );

	AudioEffects *operator->() const;

	SoundHandle play_sound( const u32 sound, const AudioEffects &effects = { },
		const AudioDescription &description = { } );
public:
	const char *name;
private:
	int idBus = -1; // Bus
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace CoreAudio
{
	class Voice
	{
	public:
		void init();

	public:
		int idBus;
		u32 generation;
		bool bypass;
		const char *name;

		int channels;
		float samplePosition;
		const i16 *samples;
		u32 samplesCount;

		AudioDescription description;
		AudioEffects effects;
		EffectState states[CoreAudio::EFFECTTYPE_COUNT];
	};


	class Stream
	{
	public:
		void init();

	public:
		int idBus;
		u32 generation;
		bool bypass;
		const char *name;

		u32 assetID;
		usize streamPosition;
		float bufferPosition;
		u32 ready;

		int channels;
		float samplePosition;
		u32 samplesCount;

		AudioDescription description;
		AudioEffects effects;
		EffectState states[CoreAudio::EFFECTTYPE_COUNT];
	};


	class Bus
	{
	public:
		void init();

	public:
		bool available;
		bool bypass;
		const char *name;

		float lookX, lookY, lookZ;
		float upX, upY, upZ;

		AudioEffects effects;
		EffectState states[CoreAudio::EFFECTTYPE_COUNT];
	};


	extern SoundHandle play_voice( const int idBus, const i16 *const samples, const u32 samplesCount,
		const int channels, const AudioEffects &effects, const AudioDescription &description, const char *name );

	extern SoundHandle play_stream( const int idBus, const u32 assetID,
		const AudioEffects &effects, const AudioDescription &description, const char *name );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////