#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <pipeline.hpp>
#include <pipeline.generated.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef PROJECT_NAME
	#define PROJECT_NAME "Project"
#endif

#ifndef PROJECT_VERSION
	#define PROJECT_VERSION "0.0.0"
#endif

#ifndef PROJECT_CAPTION
	#define PROJECT_CAPTION                                                          \
		PROJECT_NAME " - " PROJECT_VERSION " (" BUILD_CONFIG ") | "                  \
		BUILD_OS " - " BUILD_ARCHITECTURE " - " BUILD_TOOLCHAIN " - " BUILD_GRAPHICS
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef FPS_MARGIN
	#define FPS_MARGIN ( 5 )
#endif

#ifndef FPS_LIMIT
	#define FPS_LIMIT ( 60 )
#endif

#ifndef DELTA_TIME_FRAMERATE
	#define DELTA_TIME_FRAMERATE ( 60.0f )
#endif

#ifndef SWAPCHAIN_DEPTH_ENABLED
	#define SWAPCHAIN_DEPTH_ENABLED ( false )
#endif

#ifndef SWAPCHAIN_DEPTH_FORMAT
	#define SWAPCHAIN_DEPTH_FORMAT ( GfxDepthFormat_R32_FLOAT )
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef SHOW_ERROR_MESSAGES
	#define SHOW_ERROR_MESSAGES ( true )
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef WINDOW_WIDTH_DEFAULT
	#define WINDOW_WIDTH_DEFAULT ( 1280 )
#endif

#ifndef WINDOW_HEIGHT_DEFAULT
	#define WINDOW_HEIGHT_DEFAULT ( 720 )
#endif

#ifndef WINDOW_WIDTH_MIN
	#define WINDOW_WIDTH_MIN ( 480 )
#endif

#ifndef WINDOW_HEIGHT_MIN
	#define WINDOW_HEIGHT_MIN ( 480 )
#endif

#ifndef WINDOW_WIDTH_MAX
	#define WINDOW_WIDTH_MAX ( 1920 )
#endif

#ifndef WINDOW_HEIGHT_MAX
	#define WINDOW_HEIGHT_MAX ( 720 )
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GFX_QUAD_BATCH_SIZE
	#define GFX_QUAD_BATCH_SIZE ( 4096 )
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef AUDIO_BUS_COUNT
	#define AUDIO_BUS_COUNT ( 8 )
#endif

#ifndef AUDIO_VOICE_COUNT
	#define AUDIO_VOICE_COUNT ( 8 )
#endif

#ifndef AUDIO_STREAM_COUNT
	#define AUDIO_STREAM_COUNT ( 8 )
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define AUDIO_ALSA ( OS_LINUX | OS_ANDROID )
#define AUDIO_COREAUDIO ( OS_MACOS | OS_IOS | OS_IPADOS )
#define AUDIO_WASAPI ( OS_WINDOWS )
#define AUDIO_NONE ( !( AUDIO_ALSA || AUDIO_COREAUDIO || AUDIO_WASAPI ))

#if AUDIO_ALSA
	#define BACKEND_AUDIO "alsa"
#elif AUDIO_COREAUDIO
	#define BACKEND_AUDIO "coreaudio"
#elif AUDIO_WASAPI
	#define BACKEND_AUDIO "wasapi"
#else
	#define BACKEND_AUDIO "none"
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define FILESYSTEM_POSIX ( OS_LINUX | OS_ANDROID | OS_MACOS | OS_IOS | OS_IPADOS )
#define FILESYSTEM_WINDOWS ( OS_WINDOWS )
#define FILESYSTEM_NONE ( !( FILESYSTEM_POSIX || FILESYSTEM_WINDOWS ) )

#if FILESYSTEM_POSIX
	#define BACKEND_FILESYSTEM "posix"
#elif FILESYSTEM_WINDOWS
	#define BACKEND_FILESYSTEM "windows"
#else
	#define BACKEND_FILESYSTEM "none"
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define NETWORK_POSIX ( OS_LINUX | OS_ANDROID | OS_MACOS | OS_IOS | OS_IPADOS )
#define NETWORK_WINDOWS ( OS_WINDOWS )
#define NETWORK_NONE ( !( NETWORK_POSIX || NETWORK_WINDOWS ) )

#if NETWORK_POSIX
	#define BACKEND_NETWORK "posix"
#elif NETWORK_WINDOWS
	#define BACKEND_NETWORK "windows"
#else
	#define BACKEND_NETWORK "none"
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define BACKEND_GRAPHICS BUILD_GRAPHICS

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define THREAD_POSIX ( OS_LINUX | OS_ANDROID | OS_MACOS | OS_IOS | OS_IPADOS )
#define THREAD_WINDOWS ( OS_WINDOWS )

#if THREAD_POSIX
	#define BACKEND_THREAD "posix"
#elif THREAD_WINDOWS
	#define BACKEND_THREAD "windows"
#else
	#define BACKEND_THREAD "none"
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define TIMER_MACH ( OS_MACOS | OS_IOS | OS_IPADOS )
#define TIMER_POSIX ( OS_LINUX | OS_ANDROID )
#define TIMER_WINDOWS ( OS_WINDOWS )
#define TIMER_NONE ( !( TIMER_MACH || TIMER_POSIX || TIMER_WINDOWS ) )

#if TIMER_MACH
	#define BACKEND_TIMER "mach"
#elif TIMER_POSIX
	#define BACKEND_TIMER "posix"
#elif TIMER_WINDOWS
	#define BACKEND_TIMER "windows"
#else
	#define BACKEND_TIMER "none"
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define WINDOW_COCOA ( OS_MACOS | OS_IOS | OS_IPADOS )
#define WINDOW_WINDOWS ( OS_WINDOWS )
#define WINDOW_X11 ( OS_LINUX )
#define WINDOW_NONE ( !( WINDOW_COCOA || WINDOW_WINDOWS || WINDOW_X11 ) )

#if WINDOW_COCOA
	#define BACKEND_WINDOW "cocoa"
#elif WINDOW_WINDOWS
	#define BACKEND_WINDOW "windows"
#elif WINDOW_X11
	#define BACKEND_WINDOW "x11"
#else
	#define BACKEND_WINDOW "none"
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef USING_IMAGES
	#define USING_IMAGES ( 1 )
#endif

#ifndef USING_SOUNDS
	#define USING_SOUNDS ( 1 )
#endif

#ifndef USING_SPRITES
	#define USING_SPRITES ( 1 )
#endif

#ifndef USING_FONTS
	#define USING_FONTS ( 1 )
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////