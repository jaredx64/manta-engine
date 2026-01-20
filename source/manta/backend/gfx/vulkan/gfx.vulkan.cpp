#include <manta/gfx.hpp>

#include <config.hpp>

#include <manta/backend/gfx/vulkan/vulkan.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define GRAPHICS_API_NAME "VULKAN"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if 1
	#define DECL_ZERO(type, name) type name; memory_set( &name, 0, sizeof( type ) )
#else
	#define DECL_ZERO(type, name) type name
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static VkInstance instance;
static VkPhysicalDevice physicalDevice;
static VkDevice device;
static VkQueue graphicsQueue;
static u32 graphicsQueueFamily;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Resources

struct GfxShaderResource : public GfxResource
{
	static void release( GfxShaderResource *&resource );
};


struct GfxVertexBufferResource : public GfxResource
{
	static void release( GfxVertexBufferResource *&resource );
};


struct GfxInstanceBufferResource : public GfxResource
{
	static void release( GfxInstanceBufferResource *&resource );
};


struct GfxIndexBufferResource : public GfxResource
{
	static void release( GfxIndexBufferResource *&resource );
};


struct GfxUniformBufferResource : public GfxResource
{
	static void release( GfxUniformBufferResource *&resource );
};


struct GfxTextureResource : public GfxResource
{
	static void release( GfxTextureResource *&resource );
};


struct GfxRenderTargetResource : public GfxResource
{
	static void release( GfxRenderTargetResource *&resource );
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool vulkan_init()
{
	DECL_ZERO( VkApplicationInfo, appInfo );
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = PROJECT_NAME;
	appInfo.applicationVersion = VK_MAKE_VERSION( 1, 0, 0 );
	appInfo.pEngineName = PROJECT_NAME;
	appInfo.engineVersion = VK_MAKE_VERSION( 1, 0, 0 );
	appInfo.apiVersion = VK_API_VERSION_1_2;

	DECL_ZERO( VkInstanceCreateInfo, instanceInfo );
	instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.pApplicationInfo = &appInfo;

#if COMPILE_DEBUG
	instanceInfo.enabledLayerCount = 1;
	const char *validationLayers[] = { "VK_LAYER_KHRONOS_validation" };

	instanceInfo.ppEnabledLayerNames = validationLayers;
#else
	instanceInfo.enabledLayerCount = 0;
#endif

	const char *extensions[] =
	{
		VK_KHR_SURFACE_EXTENSION_NAME,

	#if OS_WINDOWS
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
	#elif OS_LINUX
		VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
	#endif

	#if COMPILE_DEBUG
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
	#endif
	};

	instanceInfo.enabledExtensionCount = ARRAY_LENGTH( extensions );
	instanceInfo.ppEnabledExtensionNames = extensions;

	if( vkCreateInstance( &instanceInfo, nullptr, &instance ) != VK_SUCCESS )
	{
		ErrorReturnMsg( false, "%s: Failed to create Vulkan instance", __FUNCTION__ );
	}

	u32 deviceCount = 0;
	vkEnumeratePhysicalDevices( instance, &deviceCount, nullptr );
	ErrorReturnIf( deviceCount == 0, false, "%s: No Vulkan-compatible GPU found", __FUNCTION__ );

	VkPhysicalDevice *physicalDevices = reinterpret_cast<VkPhysicalDevice *>(
		memory_alloc( deviceCount * sizeof( VkPhysicalDevice) ) );
	vkEnumeratePhysicalDevices( instance, &deviceCount, physicalDevices );
	physicalDevice = physicalDevices[0];

	u32 queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties( physicalDevice, &queueFamilyCount, nullptr );

	VkQueueFamilyProperties *queueFamilies = reinterpret_cast<VkQueueFamilyProperties *>(
		memory_alloc( queueFamilyCount * sizeof( VkQueueFamilyProperties) ) );
	vkGetPhysicalDeviceQueueFamilyProperties( physicalDevice, &queueFamilyCount, queueFamilies );

	graphicsQueueFamily = U32_MAX;
	for( u32 i = 0; i < queueFamilyCount; i++ )
	{
		if( queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT ) { graphicsQueueFamily = i; break; }
	}
	ErrorReturnIf( graphicsQueueFamily == U32_MAX, false, "%s: No graphics queue family found", __FUNCTION__ );

	DECL_ZERO( VkDeviceQueueCreateInfo, queueInfo );
	queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueInfo.queueFamilyIndex = graphicsQueueFamily;
	queueInfo.queueCount = 1;
	float queuePriority = 1.0f;
	queueInfo.pQueuePriorities = &queuePriority;

	DECL_ZERO( VkPhysicalDeviceFeatures, deviceFeatures );
	deviceFeatures.samplerAnisotropy = VK_TRUE;

	const char *deviceExtensions[] =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	};

	DECL_ZERO( VkDeviceCreateInfo, deviceInfo );
	deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceInfo.pQueueCreateInfos = &queueInfo;
	deviceInfo.queueCreateInfoCount = 1;
	deviceInfo.pEnabledFeatures = &deviceFeatures;
	deviceInfo.enabledExtensionCount = ARRAY_LENGTH( deviceExtensions );
	deviceInfo.ppEnabledExtensionNames = deviceExtensions;

#if COMPILE_DEBUG
	deviceInfo.enabledLayerCount = 1;
	deviceInfo.ppEnabledLayerNames = validationLayers;
#else
	deviceInfo.enabledLayerCount = 0;
#endif

	if( vkCreateDevice( physicalDevice, &deviceInfo, nullptr, &device ) != VK_SUCCESS )
	{
		ErrorReturnMsg( false, "%s: Failed to create logical device", __FUNCTION__ );
	}

	vkGetDeviceQueue( device, graphicsQueueFamily, 0, &graphicsQueue );

	return true;
}


static bool vulkan_free()
{
	// ...
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// GFX System

bool CoreGfx::api_init()
{
	return true;
}


bool CoreGfx::api_free()
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Frame

void CoreGfx::api_frame_begin()
{
}


void CoreGfx::api_frame_end()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Clear

void CoreGfx::api_clear_color( Color color )
{
}


void CoreGfx::api_clear_depth( float depth )
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Render State

bool CoreGfx::api_swapchain_init( u16 width, u16 height )
{
	return true;
}


bool CoreGfx::api_swapchain_free()
{
	return true;
}


bool CoreGfx::api_swapchain_set_size( u16 width, u16 height )
{
	return true;
}


bool CoreGfx::api_viewport_init( u16 width, u16 height )
{
	return true;
}


bool CoreGfx::api_viewport_free()
{
	return true;
}


bool CoreGfx::api_viewport_set_size( u16 width, u16 height )
{
	return true;
}


bool CoreGfx::api_scissor_set_state( const GfxStateScissor &state )
{
	return true;
}


bool CoreGfx::api_sampler_set_state( const GfxStateSampler &state )
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Pipeline State

bool CoreGfx::api_set_raster( const GfxPipelineDescription &description )
{
	return true;
}


bool CoreGfx::api_set_blend( const GfxPipelineDescription &description )
{
	return true;
}


bool CoreGfx::api_set_depth( const GfxPipelineDescription &description )
{
	return true;
}


bool CoreGfx::api_set_stencil( const GfxPipelineDescription &description )
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Shader

void GfxShaderResource::release( GfxShaderResource *&resource )
{
}


bool CoreGfx::api_shader_init( GfxShaderResource *&resource, u32 shaderID, const struct ShaderEntry &shaderEntry )
{
	return true;
}


bool CoreGfx::api_shader_free( GfxShaderResource *&resource )
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Buffer

bool CoreGfx::api_buffer_init( GfxBufferResource *&resource, usize capacity )
{
	return true;
}


bool CoreGfx::api_buffer_free( GfxBufferResource *&resource )
{
	return true;
}


void CoreGfx::api_buffer_write_begin( GfxBufferResource *resource )
{
	// ...
}


void CoreGfx::api_buffer_write_end( GfxBufferResource *resource )
{
	// ...
}


void CoreGfx::api_buffer_write( GfxBufferResource *resource, const void *data, usize size, usize offset )
{
	// ...
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Vertex Buffer

void GfxVertexBufferResource::release( GfxVertexBufferResource *&resource )
{
}


bool CoreGfx::api_vertex_buffer_init( GfxVertexBufferResource *&resource, u32 vertexFormatID,
	GfxWriteMode writeMode, u32 capacity, u32 stride )
{
	return true;
}


bool CoreGfx::api_vertex_buffer_free( GfxVertexBufferResource *&resource )
{
	return true;
}


void CoreGfx::api_vertex_buffer_write_begin( GfxVertexBufferResource *resource )
{
}


void CoreGfx::api_vertex_buffer_write_end( GfxVertexBufferResource *resource )
{
}


void CoreGfx::api_vertex_buffer_write( GfxVertexBufferResource *resource, const void *data, u32 size )
{
}


u32 CoreGfx::api_vertex_buffer_current( const GfxVertexBufferResource *resource )
{
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Instance Buffer

void GfxInstanceBufferResource::release( GfxInstanceBufferResource *&resource )
{
}


bool CoreGfx::api_instance_buffer_init( GfxInstanceBufferResource *&resource, u32 instanceFormatID,
	GfxWriteMode writeMode, u32 capacity, u32 stride )
{
	return true;
}


bool CoreGfx::api_instance_buffer_free( GfxInstanceBufferResource *&resource )
{
	return true;
}


void CoreGfx::api_instance_buffer_write_begin( GfxInstanceBufferResource *resource )
{
}


void CoreGfx::api_instance_buffer_write_end( GfxInstanceBufferResource *resource )
{
}


void CoreGfx::api_instance_buffer_write( GfxInstanceBufferResource *resource,
	const void *data, u32 size )
{
}


u32 CoreGfx::api_instance_buffer_current( const GfxInstanceBufferResource *resource )
{
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Index Buffer

void GfxIndexBufferResource::release( GfxIndexBufferResource *&resource )
{
}


bool CoreGfx::api_index_buffer_init( GfxIndexBufferResource *&resource,
	void *data, u32 size, double indicesToVerticesRatio,
	GfxIndexBufferFormat format, GfxWriteMode writeMode )
{
	return true;
}


bool CoreGfx::api_index_buffer_free( GfxIndexBufferResource *&resource )
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Uniform Buffer

void GfxUniformBufferResource::release( GfxUniformBufferResource *&resource )
{
}


bool CoreGfx::api_uniform_buffer_init( GfxUniformBufferResource *&resource, const char *name,
	int index, u32 size )
{
	return true;
}


bool CoreGfx::api_uniform_buffer_free( GfxUniformBufferResource *&resource )
{
	return true;
}


void CoreGfx::api_uniform_buffer_write_begin( GfxUniformBufferResource *resource )
{
}


void CoreGfx::api_uniform_buffer_write_end( GfxUniformBufferResource *resource )
{
}


void CoreGfx::api_uniform_buffer_write( GfxUniformBufferResource *resource, void *data )
{
}


bool CoreGfx::api_uniform_buffer_bind_vertex( GfxUniformBufferResource *resource, int slot )
{
	return true;
}


bool CoreGfx::api_uniform_buffer_bind_fragment( GfxUniformBufferResource *resource, int slot )
{
	return true;
}


bool CoreGfx::api_uniform_buffer_bind_compute( GfxUniformBufferResource *resource, int slot )
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// TODO: GfxConstantBuffer
// TODO: GfxMutableBuffer

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Texture

void GfxTextureResource::release( GfxTextureResource *&resource )
{
}


bool CoreGfx::api_texture_init( GfxTextureResource *&resource, void *pixels,
	u16 width, u16 height, u16 levels, const GfxColorFormat &format )
{
	return true;
}


bool CoreGfx::api_texture_free( GfxTextureResource *&resource )
{
	return true;
}


bool CoreGfx::api_texture_bind( GfxTextureResource *resource, int slot )
{
	return true;
}


bool CoreGfx::api_texture_release( GfxTextureResource *resource, int slot )
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Render Target

void GfxRenderTargetResource::release( GfxRenderTargetResource *&resource )
{
}


bool CoreGfx::api_render_target_init( GfxRenderTargetResource *&resource,
	GfxTextureResource *&resourceColor, GfxTextureResource *&resourceDepth,
	u16 width, u16 height, const GfxRenderTargetDescription &desc )
{
	return true;
}


bool CoreGfx::api_render_target_free( GfxRenderTargetResource *&resource,
	GfxTextureResource *&resourceColor,
	GfxTextureResource *&resourceDepth )
{
	return true;
}


bool CoreGfx::api_render_target_copy_color(
	GfxRenderTargetResource *&srcResource, GfxTextureResource *&srcResourceColor,
	GfxRenderTargetResource *&dstResource, GfxTextureResource *&dstResourceColor )
{
	return true;
}


bool CoreGfx::api_render_target_copy_depth(
	GfxRenderTargetResource *&srcResource, GfxTextureResource *&srcResourceDepth,
	GfxRenderTargetResource *&dstResource, GfxTextureResource *&dstResourceDepth )
{
	return true;
}


bool CoreGfx::api_render_target_copy(
	GfxRenderTargetResource *&srcResource,
	GfxTextureResource *&srcResourceColor, GfxTextureResource *&srcResourceDepth,
	GfxRenderTargetResource *&dstResource,
	GfxTextureResource *&dstResourceColor, GfxTextureResource *&dstResourceDepth )
{
	return true;
}


bool CoreGfx::api_render_target_copy_part(
	GfxRenderTargetResource *&srcResource,
	GfxTextureResource *&srcResourceColor, GfxTextureResource *&srcResourceDepth,
	GfxRenderTargetResource *&dstResource,
	GfxTextureResource *&dstResourceColor, GfxTextureResource *&dstResourceDepth,
	u16 srcX, u16 srcY, u16 dstX, u16 dstY, u16 width, u16 height )
{
	return true;
}


bool CoreGfx::api_render_target_buffer_read_color( GfxRenderTargetResource *&resource,
		GfxTextureResource *&resourceColor,
		void *buffer, u32 size )
{
	return true;
}


bool CoreGfx::api_render_target_buffer_read_color_async_request( GfxRenderTargetResource *&resource,
	GfxTextureResource *&resourceColor,
	void *buffer, u32 size )
{
	return true;
}


bool CoreGfx::api_render_target_buffer_read_color_async_poll( GfxRenderTargetResource *&resource,
	GfxTextureResource *&resourceColor,
	void *buffer, u32 size )
{
	return true;
}


bool api_render_target_buffer_read_depth( GfxRenderTargetResource *&resource,
	GfxTextureResource *&resourceDepth,
	void *buffer, u32 size )
{
	return true;
}


bool CoreGfx::api_render_target_buffer_read_depth_async_request( GfxRenderTargetResource *&resource,
	GfxTextureResource *&resourceDepth,
	void *buffer, u32 size )
{
	return true;
}


bool CoreGfx::api_render_target_buffer_read_depth_async_poll( GfxRenderTargetResource *&resource,
	GfxTextureResource *&resourceDepth,
	void *buffer, u32 size )
{
	return true;
}


bool CoreGfx::api_render_target_buffer_write_color( GfxRenderTargetResource *&resource,
	GfxTextureResource *&resourceColor,
	const void *buffer, u32 size )
{
	return true;
}


bool CoreGfx::api_render_target_buffer_write_depth( GfxRenderTargetResource *&resource,
	GfxTextureResource *&resourceDepth,
	const void *buffer, u32 size )
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Draw

bool CoreGfx::api_draw(
	const GfxVertexBufferResource *resourceVertex, const u32 vertexCount,
	const GfxInstanceBufferResource *resourceInstance, const u32 instanceCount,
	const GfxIndexBufferResource *resourceIndex,
	const GfxPrimitiveType type )
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CoreGfx::api_dispatch( u32 x, u32 y, u32 z )
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Render Command

void CoreGfx::api_render_command_execute( const GfxRenderCommand &command )
{
}


void CoreGfx::api_render_command_execute_post( const GfxRenderCommand &command )
{
}


void CoreGfx::api_render_pass_begin( const GfxRenderPass &pass )
{
}


void CoreGfx::api_render_pass_end( const GfxRenderPass &pass )
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////