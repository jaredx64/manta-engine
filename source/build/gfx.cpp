#include <build/gfx.hpp>

#include <config.hpp>
#include <pipeline.hpp>

#include <core/checksum.hpp>

#include <build/assets.hpp>
#include <build/shaders/compiler.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct CacheShader { usize dummy = 0LLU; };
struct CacheGfx { usize fileCount = 0LLU; };

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Gfx
{
	// Output Paths
	char pathSourceGfx[PATH_SIZE];
	char pathHeaderGfx[PATH_SIZE];
	char pathSourceAPI[PATH_SIZE];
	char pathHeaderAPI[PATH_SIZE];

	// Output Strings
	String headerGfx;
	String sourceGfx;
	String headerAPI;
	String sourceAPI;

	// Shaders
	List<FileInfo> shaderFiles;
	List<Shader> shaders;

	// Shared Structs
	List<SharedStruct> sharedStructs;
	HashMap<u32, u32> sharedStructCache;

	// Uniform Buffers
	List<UniformBuffer> uniformBuffers;
	HashMap<u32, u32> uniformBufferCache;

	// Vertex Structs
	List<VertexFormat> vertexFormats;
	HashMap<u32, u32> vertexFormatCache;

	// Instance Structs
	List<InstanceFormat> instanceFormats;
	HashMap<u32, u32> instanceFormatCache;

	// Cache
	Cache cache;
	usize cacheReadOffset = 0LLU;
	usize cacheFileCount = 0LLU;

	// Logging
	usize shadersBuilt = 0LLU;

	// Binary
	Buffer binary;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Gfx::begin()
{
	// Paths
	strjoin( pathHeaderGfx, Build::pathOutput, SLASH "generated" SLASH "gfx.generated.hpp" );
	strjoin( pathSourceGfx, Build::pathOutput, SLASH "generated" SLASH "gfx.generated.cpp" );
	strjoin( pathHeaderAPI, Build::pathOutput, SLASH "generated" SLASH "gfx.api.generated.hpp" );
	strjoin( pathSourceAPI, Build::pathOutput, SLASH "generated" SLASH "gfx.api.generated.cpp" );
}


void Gfx::end()
{
	// ...
}


u32 Gfx::gather( const char *path, const bool recurse )
{
	// Gather Shaders
	const usize start = shaderFiles.size();
	directory_iterate( shaderFiles, path, ".shader", true );

	// Check Cache
	for( usize i = start; i < shaderFiles.size(); i++ )
	{
		FileInfo &file = shaderFiles[i];

		static char cacheIDBuffer[PATH_SIZE * 2];
		memory_set( cacheIDBuffer, 0, sizeof( cacheIDBuffer ) );
		snprintf( cacheIDBuffer, sizeof( cacheIDBuffer ), "shader %s|%llu",
			file.path, file.time.as_u64() );

		const CacheID cacheID = checksum_xcrc32( cacheIDBuffer, sizeof( cacheIDBuffer ), 0 );

		CacheShader cacheShader;
		if( !Gfx::cache.dirty && !Gfx::cache.fetch( cacheID, cacheShader ) )
		{
			Gfx::cache.dirty |= true;
		}
		Gfx::cache.store( cacheID, cacheShader );
		Gfx::cacheFileCount++;
	}

	return shaderFiles.size() - start;
}


void Gfx::build()
{
	// Shader Type
	#if GRAPHICS_OPENGL || GRAPHICS_VULKAN
		const ShaderType shaderType = ShaderType_GLSL;
	#elif GRAPHICS_D3D11 || GRAPHICS_D3D12
		const ShaderType shaderType = ShaderType_HLSL;
	#elif GRAPHICS_METAL
		const ShaderType shaderType = ShaderType_METAL;
	#else
		const ShaderType shaderType = ShaderType_NONE;
	#endif

	// Build Shaders
	for( FileInfo &fileInfo : shaderFiles )
	{
		// Register
		char shaderName[PATH_SIZE];
		path_get_filename( shaderName, sizeof( shaderName ), fileInfo.path );
		path_remove_extension( shaderName, sizeof( shaderName ) );

		// Compile Shader
		Shader &shader = Gfx::shaders.add( Shader { shaderName, shaderType } );

		Timer timer;
		if( verbose_output() )
		{
			PrintColor( LOG_WHITE, TAB TAB "Compile " );
			PrintColor( LOG_CYAN, "%s", fileInfo.name );
		}

		compile_shader( shader, fileInfo.path );
		Gfx::shadersBuilt++;

		if( verbose_output() )
		{
			PrintLnColor( LOG_WHITE, " (%.2f ms)", timer.elapsed_ms() );
		}
	}

	// Binary
	for( Shader &shader : shaders )
	{
		// Write Vertex Shader
		String &codeVertex = shader.outputs[ShaderStage_Vertex];
		if( codeVertex.length_bytes() > 0 )
		{
			shader.size[ShaderStage_Vertex] = codeVertex.length_bytes();
			const usize binaryOffset = Gfx::binary.write( codeVertex.data, shader.size[ShaderStage_Vertex] );
			shader.offset[ShaderStage_Vertex] = binaryOffset;
		}

		// Write Fragment Shader
		String &codeFragment = shader.outputs[ShaderStage_Fragment];
		if( codeFragment.length_bytes() > 0 )
		{
			shader.size[ShaderStage_Fragment] = codeFragment.length_bytes();
			const usize binaryOffset = Gfx::binary.write( codeFragment.data, shader.size[ShaderStage_Fragment] );
			shader.offset[ShaderStage_Fragment] = binaryOffset;
		}

		// Write Compute Shader
		String &codeCompute = shader.outputs[ShaderStage_Compute];
		if( codeCompute.length_bytes() > 0 )
		{
			shader.size[ShaderStage_Compute] = codeCompute.length_bytes();
			const usize binaryOffset = Gfx::binary.write( codeCompute.data, shader.size[ShaderStage_Compute] );
			shader.offset[ShaderStage_Compute] = binaryOffset;
		}
	}
}


void Gfx::codegen()
{
	Buffer &binary = Assets::binary;
	Timer timer;

	// Header (Gfx)
	{
		// Header Guard
		String &header = Gfx::headerGfx;
		header.append( "#pragma once\n\n" );
		header.append( "#include <core/types.hpp>\n#include <core/memory.hpp>\n" );
		header.append( "#include <core/math.hpp>\n\n#include <manta/vector.hpp>\n\n" );

		header.append( COMMENT_BREAK "\n\n" );
		{
			assets_struct( header,
				"ShaderEntry",
				"usize offsetVertex;",
				"usize sizeVertex;",
				"usize offsetFragment;",
				"usize sizeFragment;",
				"usize offsetCompute;",
				"usize sizeCompute;",
				"u32 vertexFormat;",
				"u32 instanceFormat;",
				"const char *name;" );

			header.append( "enum_class_type\n(\n\tShader, u32,\n\n" );
			for( Shader &shader : shaders )
			{
				header.append( "\t" ).append( shader.name ).append( ",\n" );
			}
			header.append( "\n\tNull = 0,\n" );
			header.append( ");\n\n" );

			header.append( "namespace CoreGfx\n{\n" );
			header.append( "\tconstexpr u32 shaderCount = " );
			header.append( static_cast<int>( Gfx::shaders.size() ) ).append( ";\n" );
			header.append( "\textern const ShaderEntry shaderEntries[];\n" );
			header.append( "}\n\n" );
		}

		header.append( COMMENT_BREAK "\n\n" );
		{
			header.append( "namespace GfxVertex\n{\n" );
			for( VertexFormat &vertexFormat : Gfx::vertexFormats )
			{
				header.append( vertexFormat.header );
			}
			header.append( "}\n\n" );
		}

		header.append( COMMENT_BREAK "\n\n" );
		{
			header.append( "namespace GfxInstance\n{\n" );
			for( InstanceFormat &instanceFormat : Gfx::instanceFormats )
			{
				header.append( instanceFormat.header );
			}
			header.append( "}\n\n" );
		}

		header.append( COMMENT_BREAK "\n\n" );
		{
			header.append( "namespace CoreGfxVertex\n{\n" );
			header.append( "\ttemplate <typename T> consteval u32 vertex_format_id() { return 0; }\n" );
			for( VertexFormat &vertexFormat : Gfx::vertexFormats )
			{
				header.append( "\ttemplate <> consteval u32 vertex_format_id<GfxVertex::" );
				header.append( vertexFormat.name );
				header.append( ">() { return " ).append( static_cast<int>( vertexFormat.id ) ).append( "; }\n" );
			}
			header.append( "}\n\n" );
		}

		header.append( COMMENT_BREAK "\n\n" );
		{
			header.append( "namespace CoreGfxInstance\n{\n" );
			header.append( "\ttemplate <typename T> consteval u32 instance_format_id() { return 0; }\n" );
			for( InstanceFormat &instanceFormat : Gfx::instanceFormats )
			{
				header.append( "\ttemplate <> consteval u32 instance_format_id<GfxInstance::" );
				header.append( instanceFormat.name );
				header.append( ">() { return " ).append( static_cast<int>( instanceFormat.id ) );
				header.append( "; }\n" );
			}
			header.append( "}\n\n" );
		}

		header.append( COMMENT_BREAK "\n\n" );
		{
			header.append( "namespace GfxStructPacked\n{\n" );
			for( SharedStruct &sharedStruct : Gfx::sharedStructs )
			{
				header.append( sharedStruct.headerTight );
			}
			header.append( "}\n\n" );

			header.append( "namespace GfxStructPadded\n{\n" );
			for( SharedStruct &sharedStruct : Gfx::sharedStructs )
			{
				header.append( sharedStruct.headerAlign );
			}
			header.append( "}\n\n" );
		}

		header.append( COMMENT_BREAK "\n\n" );
		{
			header.append( "namespace CoreGfxUniformBuffer\n{\n" );
			for( UniformBuffer &uniformBuffer : Gfx::uniformBuffers )
			{
				header.append( uniformBuffer.header );
			}
			header.append( "}\n\n" );
		}

		header.append( COMMENT_BREAK "\n\n" );
		{
			header.append( "namespace GfxUniformBuffer\n{\n" );
			for( UniformBuffer &uniformBuffer : Gfx::uniformBuffers )
			{
				header.append( "\textern CoreGfxUniformBuffer::" ).append( uniformBuffer.name ).append("_t " );
				header.append( uniformBuffer.name ).append( ";\n" );
			}
			header.append( "}\n\n" );
		}

		header.append( COMMENT_BREAK "\n\n" );
		{
			header.append( "namespace CoreGfx\n{\n" );

			header.append( "\tconstexpr u32 uniformBufferCount = " );
			header.append( static_cast<int>( Gfx::uniformBuffers.size() ) ).append( ";\n" );
			header.append( "}\n\n" );
		}

		header.append( COMMENT_BREAK "\n" );

		// Save
		header.save( Gfx::pathHeaderGfx );
	}

	// Source (Gfx)
	{
		String &source = Gfx::sourceGfx;
		source.append( "#include <gfx.generated.hpp>\n\n" );
		source.append( "#include <binary.generated.hpp>\n\n" );
		source.append( "#include <core/memory.hpp>\n\n" );
		source.append( "#include <manta/gfx.hpp>\n\n" );

		source.append( COMMENT_BREAK "\n\n" );
		{
			source.append( "namespace CoreGfx\n{\n" );

			char buffer[PATH_SIZE];
			source.append( "#define OFFSET ( BINARY_OFFSET_GFX )\n" );
			source.append( "\tconst ShaderEntry shaderEntries[shaderCount] =\n\t{\n" );
			for( Shader &shader : shaders )
			{
				snprintf( buffer, PATH_SIZE,
					"\t\t{ OFFSET + %llu, %llu, OFFSET + %llu, %llu, OFFSET + %llu, %llu, %u, %u, \"%s\" },",
					shader.offset[ShaderStage_Vertex],
					shader.size[ShaderStage_Vertex],
					shader.offset[ShaderStage_Fragment],
					shader.size[ShaderStage_Fragment],
					shader.offset[ShaderStage_Compute],
					shader.size[ShaderStage_Compute],
					shader.vertexFormatID,
					shader.instanceFormatID,
					shader.name.cstr() );

				source.append( buffer );
				source.append( " // " ).append( shader.name ).append( "\n" );
			}
			source.append( "\t};\n" );
			source.append( "}\n\n" );
		}

		source.append( COMMENT_BREAK "\n\n" );
		{
			source.append( "namespace GfxUniformBuffer\n{\n" );
			for( UniformBuffer &uniformBuffer : Gfx::uniformBuffers )
			{
				source.append( "\tCoreGfxUniformBuffer::" ).append( uniformBuffer.name ).append("_t " );
				source.append( uniformBuffer.name ).append( ";\n" );
			}
			source.append( "}\n\n" );
		}

		source.append( COMMENT_BREAK "\n\n" );
		{
			source.append( "namespace CoreGfx\n{\n" );

			source.append( "\tGfxUniformBufferResource *gfxUniformBufferResources[CoreGfx::uniformBufferCount];\n\n" );

			source.append( "\tbool api_init_uniform_buffers()\n\t{\n" );
			for( UniformBuffer &uniformBuffer : Gfx::uniformBuffers )
			{
				source.append( "\t\tgfxUniformBufferResources[" ).append( static_cast<int>( uniformBuffer.id ) );
				source.append( "] = nullptr;\n" );
				source.append( "\t\tGfxUniformBuffer::" ).append( uniformBuffer.name ).append( ".zero();\n" );
				source.append( "\t\tif( !CoreGfx::api_uniform_buffer_init( " );
				source.append( "gfxUniformBufferResources[" ).append( static_cast<int>( uniformBuffer.id ) );
				source.append( "], \"t_" ).append( uniformBuffer.name );
				source.append( "\", " ).append( static_cast<int>( uniformBuffer.id ) );
				source.append( ", sizeof( CoreGfxUniformBuffer::" );
				source.append( uniformBuffer.name ).append( "_t ) ) ) { return false; }\n\n" );
			}
			source.append( "\t\t// Success!\n" );
			source.append( "\t\treturn true;\n" );
			source.append( "\t}\n\n" );

			source.append( "\tbool api_free_uniform_buffers()\n\t{\n" );
			source.append( "\t\tfor( u32 i = 0; i < uniformBufferCount; i++ )\n" );
			source.append( "\t\t{\n" );
			source.append( "\t\t\tif( !CoreGfx::api_uniform_buffer_free( gfxUniformBufferResources[i] ) ) { return false; }\n" );
			source.append( "\t\t}\n" );
			source.append( "\n\t\t// Success!\n" );
			source.append( "\t\treturn true;\n" );
			source.append( "\t}\n\n" );

			static const char *shaderStages[] =
			{
				"vertex",   // ShaderStage_Vertex
				"fragment", // ShaderStage_Fragment
				"compute",  // ShaderStage_Compute
			};
			static_assert( ARRAY_LENGTH( shaderStages ) == SHADERSTAGE_COUNT, "Missing ShaderStage!" );

			// shader_bind_uniform_buffers_*
			for( Shader &shader : shaders )
			{
				for( ShaderStage stage = 0; stage < SHADERSTAGE_COUNT; stage++ )
				{
					source.append( "\tstatic bool api_shader_bind_uniform_buffers_" );
					source.append( shaderStages[stage] ).append( "_" );
					source.append( shader.name ).append( "()\n\t{\n" );
					for( usize i = 0; i < shader.uniformBufferIDs[stage].size(); i++ )
					{
						int uniformBufferID = shader.uniformBufferIDs[stage][i];
						int uniformBufferSlot = shader.uniformBufferSlots[stage][i];
						UniformBuffer &uniformBuffer = Gfx::uniformBuffers[uniformBufferID];
						source.append( "\t\tif( !CoreGfx::api_uniform_buffer_bind_" ).append( shaderStages[stage] );
						source.append( "( CoreGfx::gfxUniformBufferResources[" );
						source.append( uniformBufferID ).append( "], " ).append( uniformBufferSlot );
						source.append( " ) ) { return false; }" );
						source.append( " // ").append( uniformBuffer.name ).append( "\n" );
					}
					source.append( "\t\treturn true;\n" );
					source.append( "\t}\n\n" );
				}
			}

			// api_shader_bind_uniform_buffers_vs/fs/cs[] function pointer table
			for( ShaderStage stage = 0; stage < SHADERSTAGE_COUNT; stage++ )
			{
				if( stage != 0 ) { source.append( "\n" ); }
				source.append( "\tFUNCTION_POINTER_ARRAY( bool, api_shader_bind_uniform_buffers_" );
				source.append( shaderStages[stage] ).append( " ) =\n\t{\n" );
				for( Shader &shader : shaders )
				{
					source.append( "\t\tapi_shader_bind_uniform_buffers_" ).append( shaderStages[stage] ).append( "_" );
					source.append( shader.name ).append( ",\n" );
				}
				source.append( "\t};\n" );
			}

			source.append( "}\n\n" );
		}

		source.append( COMMENT_BREAK "\n\n" );
		{
			source.append( "namespace CoreGfxUniformBuffer\n{\n" );
			for( UniformBuffer &uniformBuffer : Gfx::uniformBuffers )
			{
				source.append( uniformBuffer.source );
			}
			source.append( "}\n\n" );
		}

		source.append( COMMENT_BREAK "\n" );

		// Save
		source.save( Gfx::pathSourceGfx );
	}

	// Header (API)
	{
		// Header Guard
		String &header = Gfx::headerAPI;
		header.append( "#pragma once\n\n" );
		header.append( "#include <core/types.hpp>\n\n" );

		#if GRAPHICS_OPENGL
			write_header_api_opengl( header );
		#elif GRAPHICS_D3D11
			write_header_api_d3d11( header );
		#elif GRAPHICS_D3D12
			write_header_api_d3d12( header );
		#elif GRAPHICS_METAL
			write_header_api_metal( header );
		#elif GRAPHICS_VULKAN
			write_header_api_vulkan( header );
		#elif GRAPHICS_NONE
			// Do nothing
		#else
			Error( "Unsupported graphics API! (%s)", BUILD_GRAPHICS );
		#endif

		// Save
		header.save( Gfx::pathHeaderAPI );
	}

	// Source (API)
	{
		String &source = Gfx::sourceAPI;
		source.append( "#include <gfx.api.generated.hpp>\n" );
		source.append( "#include <gfx.generated.hpp>\n\n" );

		#if GRAPHICS_OPENGL
			write_source_api_opengl( source );
		#elif GRAPHICS_D3D11
			write_source_api_d3d11( source );
		#elif GRAPHICS_D3D12
			write_source_api_d3d12( source );
		#elif GRAPHICS_METAL
			write_source_api_metal( source );
		#elif GRAPHICS_VULKAN
			write_source_api_vulkan( source );
		#elif GRAPHICS_NONE
			// Do nothing
		#else
			Error( "Unsupported graphics API! (%s)", BUILD_GRAPHICS );
		#endif

		// Save
		source.save( Gfx::pathSourceAPI );
	}

	if( verbose_output() )
	{
		const usize count = shaders.size();
		PrintColor( LOG_WHITE, TAB TAB "Wrote %d shaders%s", count, count == 1 ? "s" : "" );
		PrintLnColor( LOG_WHITE, " (%.3f ms)", timer.elapsed_ms() );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Gfx::write_header_api_opengl( String &header )
{
	// OpenGL Header
	header.append( "#include <manta/backend/gfx/opengl/opengl.hpp>\n\n" );

	// CoreGfx
	header.append( "namespace CoreGfx\n{\n" );
	header.append( COMMENT_BREAK "\n\n" );
	{
		for( Shader &shader : shaders )
		{
			header.append( shader.header );
		}

		// Vertex Input Layout
		header.append( "extern FUNCTION_POINTER_ARRAY( GLuint, opengl_input_layout_vertex_init, GLuint, GLuint );\n\n" );
		header.append( "extern FUNCTION_POINTER_ARRAY( GLuint, opengl_input_layout_vertex_bind, GLuint );\n\n" );

		// Instance Input Layout
		header.append( "extern FUNCTION_POINTER_ARRAY( GLuint, opengl_input_layout_instance_init, GLuint, GLuint );\n\n" );
		header.append( "extern FUNCTION_POINTER_ARRAY( GLuint, opengl_input_layout_instance_bind, GLuint );\n\n" );
	}
	header.append( COMMENT_BREAK "\n" );
	header.append( "}" );
}


void Gfx::write_source_api_opengl( String &source )
{
	// CoreGfx
	source.append( "namespace CoreGfx\n{\n" );
	source.append( COMMENT_BREAK "\n\n" );
	{
		for( Shader &shader : shaders )
		{
			source.append( shader.source );
		}

		// Vertex Input Layout
		if( vertexFormats.count() > 0 )
		{
			source.append(
				"FUNCTION_POINTER_ARRAY( GLuint, opengl_input_layout_vertex_init, GLuint, GLuint ) =\n{\n" );
			for( VertexFormat &vertexFormat : vertexFormats )
			{
				source.append( TAB ).append( "opengl_input_layout_vertex_init_" );
				source.append( vertexFormat.name ).append( ",\n" );
			}
			source.append( "};\n\n" );

			source.append(
				"FUNCTION_POINTER_ARRAY( GLuint, opengl_input_layout_vertex_bind, GLuint ) =\n{\n" );
			for( VertexFormat &vertexFormat : vertexFormats )
			{
				source.append( TAB ).append( "opengl_input_layout_vertex_bind_" );
				source.append( vertexFormat.name ).append( ",\n" );
			}
			source.append( "};\n\n" );
		}
		else
		{
			source.append( "GLuint ( *opengl_input_layout_vertex_init[] )( GLuint, GLuint ) = { nullptr };\n" );
			source.append( "GLuint ( *opengl_input_layout_vertex_bind[] )( GLuint ) = { nullptr };\n\n" );
		}

		// Instance Input Layout
		if( instanceFormats.count() > 0 )
		{
			source.append(
				"FUNCTION_POINTER_ARRAY( GLuint, opengl_input_layout_instance_init, GLuint, GLuint ) =\n{\n" );
			for( InstanceFormat &instanceFormat : instanceFormats )
			{
				source.append( TAB ).append( "opengl_input_layout_instance_init_" );
				source.append( instanceFormat.name ).append( ",\n" );
			}
			source.append( "};\n\n" );

			source.append(
				"FUNCTION_POINTER_ARRAY( GLuint, opengl_input_layout_instance_bind, GLuint ) =\n{\n" );
			for( InstanceFormat &instanceFormat : instanceFormats )
			{
				source.append( TAB ).append( "opengl_input_layout_instance_bind_" );
				source.append( instanceFormat.name ).append( ",\n" );
			}
			source.append( "};\n\n" );
		}
		else
		{
			source.append( "GLuint ( *opengl_input_layout_instance_init[] )( GLuint, GLuint ) = { nullptr };\n" );
			source.append( "GLuint ( *opengl_input_layout_instance_bind[] )( GLuint ) = { nullptr };\n\n" );
		}
	}
	source.append( COMMENT_BREAK "\n" );
	source.append( "}" );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Gfx::write_header_api_d3d11( String &header )
{
	// D3D11 Header
	header.append( "#include <vendor/d3d11.hpp>\n\n" );

	// CoreGfx
	header.append( "namespace CoreGfx\n{\n" );
	header.append( COMMENT_BREAK "\n\n" );
	{
		for( Shader &shader : shaders )
		{
			header.append( shader.header );
		}


		header.append( "struct D3D11InputLayoutDescription\n{\n" );
		header.append( "\tD3D11_INPUT_ELEMENT_DESC *desc;\n" );
		header.append( "\tint count;\n" );
		header.append( "};\n\n" );

		// Per-Vertex Input Layout
		header.append( "extern FUNCTION_POINTER_ARRAY( void, d3d11_input_layout_desc_vertex, " );
		header.append( "D3D11InputLayoutDescription & );\n\n" );

		// Per-Instance Input Layout
		header.append( "extern FUNCTION_POINTER_ARRAY( void, d3d11_input_layout_desc_instance, " );
		header.append( "D3D11InputLayoutDescription & );\n\n" );
	}
	header.append( COMMENT_BREAK "\n" );
	header.append( "}" );
}


void Gfx::write_source_api_d3d11( String &source )
{
	// CoreGfx
	source.append( "namespace CoreGfx\n{\n" );
	source.append( COMMENT_BREAK "\n\n" );
	{
		for( Shader &shader : shaders )
		{
			source.append( shader.source );
		}

		// Per-Vertex Input Layout
		if( vertexFormats.count() > 0 )
		{
			source.append( "FUNCTION_POINTER_ARRAY( void, d3d11_input_layout_desc_vertex, " );
			source.append( "D3D11InputLayoutDescription & ) =\n{\n" );
			for( VertexFormat &vertexFormat : vertexFormats )
			{
				source.append( TAB ).append( "d3d11_input_layout_desc_vertex_" );
				source.append( vertexFormat.name ).append( ",\n" );
			}
			source.append( "};\n\n" );
		}
		else
		{
			source.append( "void ( *d3d11_input_layout_desc_vertex[] )" );
			source.append( "( D3D11InputLayoutDescription & ) = { nullptr };\n\n" );
		}

		// Per-Instance Input Layout
		if( instanceFormats.count() > 0 )
		{
			source.append( "FUNCTION_POINTER_ARRAY( void, d3d11_input_layout_desc_instance, " );
			source.append( "D3D11InputLayoutDescription & ) =\n{\n" );
			for( InstanceFormat &instanceFormat : instanceFormats )
			{
				source.append( TAB ).append( "d3d11_input_layout_desc_instance_" );
				source.append( instanceFormat.name ).append( ",\n" );
			}
			source.append( "};\n\n" );
		}
		else
		{
			source.append( "void ( *d3d11_input_layout_desc_instance[] )" );
			source.append( "( D3D11InputLayoutDescription & ) = { nullptr };\n\n" );
		}
	}
	source.append( COMMENT_BREAK "\n" );
	source.append( "}" );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Gfx::write_header_api_d3d12( String &header )
{
	// TODO
	Error( "D3D12 unsupported!" );
}


void Gfx::write_source_api_d3d12( String &source )
{
	// TODO
	Error( "D3D12 unsupported!" );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Gfx::write_header_api_metal( String &header )
{
	// Metal Header
	header.append( "#include <manta/backend/gfx/metal/metal.hpp>\n\n" );

	// CoreGfx
	header.append( "namespace CoreGfx\n{\n" );
	header.append( COMMENT_BREAK "\n\n" );
	{
		// TODO
		// ...
	}
	header.append( COMMENT_BREAK "\n" );
	header.append( "}" );
}


void Gfx::write_source_api_metal( String &source )
{
	// CoreGfx
	source.append( "namespace CoreGfx\n{\n" );
	source.append( COMMENT_BREAK "\n\n" );
	{
		// TODO
		// ...
	}
	source.append( COMMENT_BREAK "\n" );
	source.append( "}" );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Gfx::write_header_api_vulkan( String &header )
{
	// TODO
	Error( "Vulkan unsupported!" );
}


void Gfx::write_source_api_vulkan( String &source )
{
	// TODO
	Error( "Vulkan unsupported!" );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Gfx::cache_read( const char *path )
{
	Gfx::cache.dirty |= Build::cache.dirty;

	// Graphics Cache
	if( !Gfx::cache.dirty )
	{
		Gfx::cache.read( path );
		Gfx::cache.dirty |= Gfx::cache.dirty;
	}

	// Codegen Cache
	AssetFile codegen;
	if( !asset_file_register( codegen, pathHeaderGfx ) ) { Gfx::cache.dirty = true; return; }
	if( !asset_file_register( codegen, pathSourceGfx ) ) { Gfx::cache.dirty = true; return; }
	if( !asset_file_register( codegen, pathHeaderAPI ) ) { Gfx::cache.dirty = true; return; }
	if( !asset_file_register( codegen, pathSourceAPI ) ) { Gfx::cache.dirty = true; return; }
}


void Gfx::cache_write( const char *path )
{
	if( !Gfx::cache.dirty ) { return; }
	Gfx::cache.write( path );
}


void Gfx::cache_validate()
{
	Gfx::cache.dirty |= Build::cache.dirty;

	// Validate file count
	CacheGfx cacheGfx;
	if( !Gfx::cache.fetch( 0, cacheGfx ) ) { Gfx::cache.dirty |= true; }
	Gfx::cache.dirty |= ( Gfx::cacheFileCount != cacheGfx.fileCount );

	// Cache file count
	cacheGfx.fileCount = Gfx::cacheFileCount;
	Gfx::cache.store( 0, cacheGfx );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////