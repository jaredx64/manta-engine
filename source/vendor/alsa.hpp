#pragma once
#include <vendor/config.hpp>

#if USE_OFFICIAL_HEADERS
	#include <vendor/conflicts.hpp>
		#include <alsa/asoundlib.h>
	#include <vendor/conflicts.hpp>
#else
	enum snd_pcm_stream_t
	{
		SND_PCM_STREAM_PLAYBACK,
		SND_PCM_STREAM_CAPTURE,
	};

	enum snd_pcm_access_t
	{
		SND_PCM_ACCESS_MMAP_INTERLEAVED,
		SND_PCM_ACCESS_MMAP_NONINTERLEAVED,
		SND_PCM_ACCESS_MMAP_COMPLEX,
		SND_PCM_ACCESS_RW_INTERLEAVED,
		SND_PCM_ACCESS_RW_NONINTERLEAVED,
	};

	enum snd_pcm_format_t
	{
		SND_PCM_FORMAT_S16_LE = 2,
	};

	using snd_pcm_t = void *;
	using snd_pcm_hw_params_t = void *;
	using snd_pcm_sw_params_t = void *;
	using snd_pcm_uframes_t = unsigned long;
	using snd_pcm_sframes_t = long;

	extern "C" int snd_pcm_open(snd_pcm_t **, const char *, snd_pcm_stream_t, int);
	extern "C" int snd_pcm_hw_params_malloc(snd_pcm_hw_params_t **);
	extern "C" int snd_pcm_hw_params_any(snd_pcm_t *, snd_pcm_hw_params_t *);

	extern "C" int snd_pcm_hw_params_set_access(snd_pcm_t *, snd_pcm_hw_params_t *, snd_pcm_access_t);
	extern "C" int snd_pcm_hw_params_set_format(snd_pcm_t *, snd_pcm_hw_params_t *, snd_pcm_format_t);
	extern "C" int snd_pcm_hw_params_set_channels(snd_pcm_t *, snd_pcm_hw_params_t *, unsigned int);
	extern "C" int snd_pcm_hw_params_set_rate_near(snd_pcm_t *, snd_pcm_hw_params_t *, unsigned int *, int *);
	extern "C" int snd_pcm_hw_params_set_buffer_size_near(snd_pcm_t *, snd_pcm_hw_params_t *, snd_pcm_uframes_t *);
	extern "C" int snd_pcm_hw_params(snd_pcm_t *, snd_pcm_hw_params_t *);
	extern "C" int snd_pcm_hw_params_get_period_size(const snd_pcm_hw_params_t *, snd_pcm_uframes_t *, int *);
	extern "C" const char *snd_pcm_name(snd_pcm_t *);
	extern "C" int snd_pcm_sw_params_malloc(snd_pcm_sw_params_t **);
	extern "C" int snd_pcm_sw_params_current(snd_pcm_t *, snd_pcm_sw_params_t *);
	extern "C" int snd_pcm_sw_params_set_avail_min(snd_pcm_t *, snd_pcm_sw_params_t *, snd_pcm_uframes_t);
	extern "C" int snd_pcm_sw_params_set_start_threshold(snd_pcm_t *, snd_pcm_sw_params_t *, snd_pcm_uframes_t);
	extern "C" int snd_pcm_sw_params(snd_pcm_t *, snd_pcm_sw_params_t *);
	extern "C" void snd_pcm_hw_params_free(snd_pcm_hw_params_t *);
	extern "C" void snd_pcm_sw_params_free(snd_pcm_sw_params_t *);
	extern "C" int snd_pcm_prepare(snd_pcm_t *);
	extern "C" int snd_pcm_wait(snd_pcm_t *, int);
	extern "C" snd_pcm_sframes_t snd_pcm_avail_update(snd_pcm_t *);
	extern "C" snd_pcm_sframes_t snd_pcm_writei(snd_pcm_t *, const void *, snd_pcm_uframes_t);
#endif