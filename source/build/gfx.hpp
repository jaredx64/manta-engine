#pragma once

#include <config.hpp>

#include <core/types.hpp>
#include <core/debug.hpp>
#include <core/hashmap.hpp>

#include <build/build.hpp>
#include <build/filesystem.hpp>
#include <build/shaders/compiler.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum_type( ShaderType, u8 )
{
	ShaderType_NONE,
	ShaderType_HLSL,
	ShaderType_GLSL,
	ShaderType_METAL,
	SHADERTYPE_COUNT,
};


enum_type( ShaderStage, u32 )
{
	ShaderStage_Vertex,
	ShaderStage_Fragment,
	ShaderStage_Compute,
	SHADERSTAGE_COUNT,
};


struct Shader
{
	Shader() = default;
	Shader( const char *name, const ShaderType type ) : name{ name }, type{ type } { }

	String name;
	ShaderType type;

	// C++ Code
	List<u32> uniformBufferIDs[SHADERSTAGE_COUNT];
	List<int> uniformBufferSlots[SHADERSTAGE_COUNT];
	u32 vertexFormatID;
	String header; // gfx.api.generated.hpp
	String source; // gfx.api.generated.cpp

	// Shader Code
	String outputs[SHADERSTAGE_COUNT];
	u32 offset[SHADERSTAGE_COUNT];
	u32 size[SHADERSTAGE_COUNT];
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct SharedStruct
{
	String name;
	u32 id = 0;
	u32 checksum = 0;
	String headerTight;
	String headerAlign;
};

struct UniformBuffer
{
	String name;
	u32 id = 0;
	u32 checksum = 0;
	String header;
	String source;
};

struct VertexFormat
{
	String name;
	u32 id = 0;
	u32 checksum = 0;
	String header;
	String source;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Gfx
{
	// Output Paths
	extern char pathHeaderGfx[PATH_SIZE];
	extern char pathSourceGfx[PATH_SIZE];
	extern char pathHeaderAPI[PATH_SIZE];
	extern char pathSourceAPI[PATH_SIZE];

	// Output Strings
	extern String sourceGfx;
	extern String sourceAPI;
	extern String headerGfx;
	extern String headerAPI;

	// Cache
	extern usize shaderFileCount;
	extern FileTime timeCache;

	// Shaders
	extern List<FileInfo> shaderFiles;
	extern List<Shader> shaders;

	// Shader Structs
	extern List<SharedStruct> sharedStructs;
	extern HashMap<u32, u32> sharedStructCache;

	// Uniform Buffers
	extern List<UniformBuffer> uniformBuffers;
	extern HashMap<u32, u32> uniformBufferCache;

	// Vertex Formats
	extern List<VertexFormat> vertexFormats;
	extern HashMap<u32, u32> vertexFormatCache;

	// Stages
	extern void begin();
	extern u32 gather( const char *path, const bool recurse );
	extern void build();
	extern void write();

	// Backends
	void write_header_api_opengl( String &header );
	void write_source_api_opengl( String &source );
	void write_header_api_d3d11( String &header );
	void write_source_api_d3d11( String &source );
	void write_header_api_d3d12( String &header );
	void write_source_api_d3d12( String &source );
	void write_header_api_metal( String &header );
	void write_source_api_metal( String &source );
	void write_header_api_vulkan( String &header );
	void write_source_api_vulkan( String &source );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////