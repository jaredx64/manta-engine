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
	ShaderType_Default,
	ShaderType_HLSL,
	ShaderType_GLSL,
	ShaderType_Metal,
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
	List<u32> constantBufferIDs[SHADERSTAGE_COUNT];
	List<int> constantBufferSlots[SHADERSTAGE_COUNT];
	u32 vertexFormatID;
	String header; // gfx.api.generated.hpp
	String source; // gfx.api.generated.cpp

	// Shader Code
	String outputs[SHADERSTAGE_COUNT];
	u32 offset[SHADERSTAGE_COUNT];
	u32 size[SHADERSTAGE_COUNT];
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct VertexFormat
{
	String name;
	u32 id = 0;
	u32 checksum = 0;
	String header;
	String source;
};


struct ConstantBuffer
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

	// Vertex Formats
	extern List<VertexFormat> vertexFormats;
	extern HashMap<u32, u32> vertexFormatCache;

	// Constant Buffers
	extern List<ConstantBuffer> constantBuffers;
	extern HashMap<u32, u32> constantBufferCache;

	// Stages
	extern void begin();
	extern u32 gather( const char *path, const bool recurse );
	extern void build();
	extern void write();

	// Backends
	void write_header_api_d3d11( String &header );
	void write_source_api_d3d11( String &source );
	void write_header_api_opengl( String &header );
	void write_source_api_opengl( String &source );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////