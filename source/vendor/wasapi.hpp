#pragma once
#include <vendor/config.hpp>

#if USE_OFFICIAL_HEADERS
	#include <vendor/conflicts.hpp>
		#include <audioclient.h>
		#include <mmdeviceapi.h>
	#include <vendor/conflicts.hpp>
#else
	#include <vendor/vendor.hpp>
	#include <vendor/com.hpp>

	#define WAVE_FORMAT_PCM 1

	#define AUDCLNT_STREAMFLAGS_EVENTCALLBACK 0x00040000
	#define AUDCLNT_STREAMFLAGS_RATEADJUST 0x00100000
	#define AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY 0x08000000
	#define AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM 0x80000000

	using REFERENCE_TIME = LONGLONG;

	enum EDataFlow
	{
		eRender,
		eCapture,
	};

	enum ERole
	{
		eConsole,
		eMultimedia,
		eCommunications,
	};

	enum AUDCLNT_SHAREMODE
	{
		AUDCLNT_SHAREMODE_SHARED,
		AUDCLNT_SHAREMODE_EXCLUSIVE,
	};

	struct WAVEFORMATEX
	{
		WORD wFormatTag;
		WORD nChannels;
		DWORD nSamplesPerSec;
		DWORD nAvgBytesPerSec;
		WORD nBlockAlign;
		WORD wBitsPerSample;
		WORD cbSize;
	};

	MIDL_INTERFACE IMMDevice : IUnknown
	{
		virtual HRESULT STD_CALL Activate(const GUID &, DWORD, struct PROPVARIANT *, void **) = 0;
		virtual HRESULT STD_CALL OpenPropertyStore(DWORD, struct IPropertyStore **) = 0;
		virtual HRESULT STD_CALL GetId(LPWSTR *) = 0;
		virtual HRESULT STD_CALL GetState(DWORD *) = 0;
	};

	MIDL_INTERFACE IMMDeviceEnumerator : IUnknown
	{
		virtual HRESULT STD_CALL EnumAudioEndpoints(EDataFlow, DWORD, struct IMMDeviceCollection **) = 0;
		virtual HRESULT STD_CALL GetDefaultAudioEndpoint(EDataFlow, ERole, IMMDevice **) = 0;
		virtual HRESULT STD_CALL GetDevice(LPCWSTR, IMMDevice **) = 0;
		virtual HRESULT STD_CALL RegisterEndpointNotificationCallback(struct IMMNotificationClient *) = 0;
		virtual HRESULT STD_CALL UnregisterEndpointNotificationCallback(struct IMMNotificationClient *) = 0;
	};

	MIDL_INTERFACE MMDeviceEnumerator;

	MIDL_INTERFACE IAudioClient : IUnknown
	{
		virtual HRESULT STD_CALL Initialize(AUDCLNT_SHAREMODE, DWORD, REFERENCE_TIME, REFERENCE_TIME, const WAVEFORMATEX *, const GUID *) = 0;
		virtual HRESULT STD_CALL GetBufferSize(UINT32 *) = 0;
		virtual HRESULT STD_CALL GetStreamLatency(REFERENCE_TIME *) = 0;
		virtual HRESULT STD_CALL GetCurrentPadding(UINT32 *) = 0;
		virtual HRESULT STD_CALL IsFormatSupported(AUDCLNT_SHAREMODE, const WAVEFORMATEX *, WAVEFORMATEX **) = 0;
		virtual HRESULT STD_CALL GetMixFormat(WAVEFORMATEX **) = 0;
		virtual HRESULT STD_CALL GetDevicePeriod(REFERENCE_TIME *, REFERENCE_TIME *) = 0;
		virtual HRESULT STD_CALL Start() = 0;
		virtual HRESULT STD_CALL Stop() = 0;
		virtual HRESULT STD_CALL Reset() = 0;
		virtual HRESULT STD_CALL SetEventHandle(HANDLE) = 0;
		virtual HRESULT STD_CALL GetService(const GUID &, void **) = 0;
	};

	MIDL_INTERFACE IAudioRenderClient : IUnknown
	{
		virtual HRESULT STD_CALL GetBuffer(UINT32, BYTE **) = 0;
		virtual HRESULT STD_CALL ReleaseBuffer(UINT32, DWORD) = 0;
	};
#endif