#pragma once

#include <pipeline.generated.hpp>
#include <core/types.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if OS_WINDOWS
	#define VK_USE_PLATFORM_WIN32_KHR
#elif OS_LINUX
    #define VK_USE_PLATFORM_XLIB_KHR
#elif OS_ANDROID
	#define VK_USE_PLATFORM_ANDROID_KHR
#elif OS_MACOS || OS_IOS || OS_IPADOS
    #define VK_USE_PLATFORM_METAL_EXT
#elif OS_WEBASM
	static_assert( false, "Vulkan not supported on this platform" );
#endif

#define uint32_t u32 // TODO: Temp
#include <vulkan/vulkan.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////