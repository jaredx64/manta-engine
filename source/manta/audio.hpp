#pragma once

#include <core/types.hpp>
#include <core/debug.hpp>

#include <manta/audio.tuning.hpp>

#if COMPILE_DEBUG
#include <manta/vector.hpp>
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct DiskSound;
struct DiskSong;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace SysAudio
{
	extern struct Voice voices[AUDIO_VOICE_COUNT];
	extern struct Bus buses[AUDIO_BUS_COUNT];

	extern i16 *g_AUDIO_SAMPLES; // TODO: refactor

	extern bool init();
	extern bool free();
	extern bool init_backend();
	extern bool free_backend();
	extern void audio_mixer( i16 *output, u32 frames );

#if COMPILE_DEBUG
	extern intv2 draw( const Delta delta, const float x, const float y );
	extern bool draw_bus( const Delta delta, const int voice, float &x, float &y );
	extern bool draw_voice( const Delta delta, const int voice, float &x, float &y );
	extern bool draw_effect( const Delta delta, struct Effect &effect, float &x, float &y );
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace SysAudio
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
		EffectType_Lowpass,
		EffectType_Reverb,
		EFFECTTYPE_COUNT,
	};
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace SysAudio
{
	struct EffectParameter
	{
		float valueFrom, valueTo;
		float sampleCurrent, sampleToInv;
	};


	struct EffectStateCore
	{
		// ...
	};


	struct EffectStateLowpass
	{
		float x[LPF_ORDER + 1]; // Raw values
		float y[LPF_ORDER + 1]; // Filtered values
		float a[LPF_ORDER];
		float b[LPF_ORDER + 1];
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
		EffectStateReverbComb combL[SysAudioTuning::numCombs];
		EffectStateReverbComb combR[SysAudioTuning::numCombs];

		// Allpass Filter
		EffectStateReverbAllPass allpassL[SysAudioTuning::numAllpasses];
		EffectStateReverbAllPass allpassR[SysAudioTuning::numAllpasses];

		// Buffers for the combs
		float bufcombL1[SysAudioTuning::combTuningL1];
		float bufcombR1[SysAudioTuning::combTuningR1];
		float bufcombL2[SysAudioTuning::combTuningL2];
		float bufcombR2[SysAudioTuning::combTuningR2];
		float bufcombL3[SysAudioTuning::combTuningL3];
		float bufcombR3[SysAudioTuning::combTuningR3];
		float bufcombL4[SysAudioTuning::combTuningL4];
		float bufcombR4[SysAudioTuning::combTuningR4];
		float bufcombL5[SysAudioTuning::combTuningL5];
		float bufcombR5[SysAudioTuning::combTuningR5];
		float bufcombL6[SysAudioTuning::combTuningL6];
		float bufcombR6[SysAudioTuning::combTuningR6];
		float bufcombL7[SysAudioTuning::combTuningL7];
		float bufcombR7[SysAudioTuning::combTuningR7];
		float bufcombL8[SysAudioTuning::combTuningL8];
		float bufcombR8[SysAudioTuning::combTuningR8];

		// Buffers for the allpasses
		float bufallpassL1[SysAudioTuning::allpassTuningL1];
		float bufallpassR1[SysAudioTuning::allpassTuningR1];
		float bufallpassL2[SysAudioTuning::allpassTuningL2];
		float bufallpassR2[SysAudioTuning::allpassTuningR2];
		float bufallpassL3[SysAudioTuning::allpassTuningL3];
		float bufallpassR3[SysAudioTuning::allpassTuningR3];
		float bufallpassL4[SysAudioTuning::allpassTuningL4];
		float bufallpassR4[SysAudioTuning::allpassTuningR4];
	};


	struct Effect
	{
		int type = -1;
		bool bypass = false;
		EffectParameter parameters[EFFECTPARAM_COUNT_MAX];

		union
		{
			EffectStateCore stateGain;
			EffectStateLowpass stateLowpass;
			EffectStateReverb stateReverb;
		};

		float get_parameter( const EffectParam param, const bool incrementTime = false );
		void set_parameter( const EffectParam param, const float value );
		void set_parameter( const EffectParam param, const float value, const usize timeMS );
		void set_parameter( const EffectParam param, const float valueFrom, const float valueTo, const usize timeMS );
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
		SysAudio::Effect &effect = find_effect( effectType ); \
		effect.set_parameter( effectParam, value, timeMS ); \
	} \
	void AudioEffects::set_##name( const float valueFrom, const float valueTo, const usize timeMS ) \
	{ \
		SysAudio::Effect &effect = find_effect( effectType ); \
		effect.set_parameter( effectParam, valueFrom, valueTo, timeMS ); \
	} \
	float AudioEffects::get_##name() \
	{ \
		SysAudio::Effect &effect = find_effect( effectType ); \
		return effect.get_parameter( effectParam, false ); \
	}

#define __AUDIO_EFFECT_PARAM_GET_SET_NOAUTOMATION_IMPL(effectType,name,effectParam) \
	void AudioEffects::set_##name( const float value ) \
	{ \
		SysAudio::Effect &effect = find_effect( effectType ); \
		effect.set_parameter( effectParam, value, 0.0f ); \
	} \
	float AudioEffects::get_##name() \
	{ \
		SysAudio::Effect &effect = find_effect( effectType ); \
		return effect.get_parameter( effectParam, false ); \
	}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class AudioEffects
{
_PUBLIC:
	AudioEffects() { init(); }
    SysAudio::Effect &operator[]( const usize index ) { return effects[index]; }
	const SysAudio::Effect &operator[]( const usize index ) const { return effects[index]; }

_PUBLIC:
	// EffectType_Core
	__AUDIO_EFFECT_PARAM_GET_SET_DECL( gain )
	__AUDIO_EFFECT_PARAM_GET_SET_NOAUTOMATION_DECL( pitch )

	// EffectType_Lowpass
	__AUDIO_EFFECT_PARAM_GET_SET_DECL( lowpass_cutoff )

	// EffectType_Reverb
	__AUDIO_EFFECT_PARAM_GET_SET_DECL( reverb_size )
	__AUDIO_EFFECT_PARAM_GET_SET_DECL( reverb_wet )
	__AUDIO_EFFECT_PARAM_GET_SET_DECL( reverb_dry )
	//__AUDIO_EFFECT_PARAM_GET_SET_DECL( reverb_damp )
	__AUDIO_EFFECT_PARAM_GET_SET_DECL( reverb_width )

_PRIVATE:
	void init();
	SysAudio::Effect &find_effect( SysAudio::EffectType effect );
	SysAudio::Effect effects[SysAudio::EFFECTTYPE_COUNT];
};


class SoundHandle
{
_PUBLIC:
	SoundHandle() : voice{ -1 }, id{ -1 } { };
	SoundHandle( const int voice, const int id ) : voice{ voice }, id{ id } { };
	SoundHandle( const SoundHandle &other ) : voice{ other.voice }, id{ other.id } { };

	bool is_playing() const;
	bool is_paused() const;
	bool pause( const bool pause ) const;
	bool stop() const;

	AudioEffects *operator->() const;

_PUBLIC:
	int voice = -1;
	int id = -1;
};


class AudioContext
{
_PUBLIC:
	AudioContext() : bus{ -1 } { };

	void init( const AudioEffects &effects = { }, const char *name = "" );
	void free();

	bool is_paused() const;
	bool pause( const bool pause ) const;

	AudioEffects *operator->() const;

	SoundHandle play_sound( const u32 sound, const AudioEffects &effects = { } );

_PUBLIC:
	const char *name;
_PRIVATE:
	int bus = -1;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace SysAudio
{
	struct Voice
	{
		int bus = -1;
		int id = 0;
		bool bypass = false;
		const char *name = "";

		float position = 0.0f;
		int channels = 1;
		const i16 *samples = nullptr;
		u32 samplesCount = 0;
		AudioEffects effects;
	};

	struct Bus
	{
		bool available = true;
		bool bypass = false;
		const char *name = "";

		AudioEffects effects;
	};

	extern SoundHandle play_sound( const int bus, const i16 *const samples, const u32 samplesCount, const int channels,
	                               const AudioEffects &effects, const char *name );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////






























#if 0
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// TODO: Move
#define ENGINE_SOUND_VOICE_LIMIT ( 16 )
#define ENGINE_SOUND_STREAM_LIMIT ( 8 )
#define ENGINE_SOUND_STREAM_BUFFERS ( 2 )
#define ENGINE_SOUND_STREAM_BLOCK ( 4096 )
#define ENGINE_SOUND_FREQUENCY ( 44100 )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct DiskSound;
struct DiskSong;

namespace SysAudio
{
	struct Voice
	{
		float position;
		float pitch;
		float gain;
		const DiskSound *sound;
	};

	struct AudioStream
	{
		usize position;
		usize streamPosition;
		u32 ready;
		const DiskSong *song;
	};
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace SysAudio
{
	extern Voice g_AUDIO_VOICES[];
	extern AudioStream g_AUDIO_STREAMS[];
	extern i16 g_AUDIO_BUFFERS[ENGINE_SOUND_STREAM_LIMIT][ENGINE_SOUND_STREAM_BUFFERS][ENGINE_SOUND_STREAM_BLOCK * 2];
	extern i16 *g_AUDIO_SAMPLES;

	extern bool init_backend();
	extern bool free_backend();
	extern bool init_samples();
	extern bool free_samples();

	//extern bool audio_stream_init();
	extern void audio_mixer( i16 *output, u32 frames );
	extern void draw( const Delta delta, const int x, const int y );

	extern void play_sound( const i16 *const samples, const usize samplesCount );
}


namespace Audio
{
	extern void play_sound( const u32 sound );
	extern void play_stream( const u32 stream );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif