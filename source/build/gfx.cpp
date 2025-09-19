#include <build/gfx.hpp>

#include <config.hpp>
#include <pipeline.hpp>

#include <build/assets.hpp>
#include <build/shaders/compiler.hpp>

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

	// Cache
	usize shaderFileCount = 0;
	FileTime timeCache;

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
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Gfx::begin()
{
	// Paths
	strjoin( pathHeaderGfx, Build::pathOutput, SLASH "generated" SLASH "gfx.generated.hpp" );
	strjoin( pathSourceGfx, Build::pathOutput, SLASH "generated" SLASH "gfx.generated.cpp" );
	strjoin( pathHeaderAPI, Build::pathOutput, SLASH "generated" SLASH "gfx.api.generated.hpp" );
	strjoin( pathSourceAPI, Build::pathOutput, SLASH "generated" SLASH "gfx.api.generated.cpp" );

	// Cache
	FileTime timeHeaderGfx;
	if( !file_time( pathHeaderGfx, &timeHeaderGfx ) ) { Build::cacheDirtyShaders = true; return; }
	FileTime timeSourceGfx;
	if( !file_time( pathSourceGfx, &timeSourceGfx ) ) { Build::cacheDirtyShaders = true; return; }
	FileTime timeHeaderAPI;
	if( !file_time( pathHeaderAPI, &timeHeaderAPI ) ) { Build::cacheDirtyShaders = true; return; }
	FileTime timeSourceAPI;
	if( !file_time( pathSourceAPI, &timeSourceAPI ) ) { Build::cacheDirtyShaders = true; return; }

	FileTime timeCacheGfx = file_time_newer( timeHeaderGfx, timeSourceGfx ) ? timeHeaderGfx : timeSourceGfx;
	FileTime timeCacheAPI = file_time_newer( timeHeaderAPI, timeSourceAPI ) ? timeHeaderAPI : timeSourceAPI;
	timeCache = file_time_newer( timeCacheGfx, timeCacheAPI ) ? timeCacheGfx : timeCacheAPI;
}


u32 Gfx::gather( const char *path, const bool recurse )
{
	// Gather Shaders
	Timer timer;
	const usize start = shaderFiles.size();
	directory_iterate( shaderFiles, path, ".shader", true );

	// Check Cache
	for( FileInfo &fileInfo : shaderFiles )
	{
		if( Build::cacheDirtyShaders ) { break; }
		FileTime timeShader;
		file_time( fileInfo.path, &timeShader );
		Build::cacheDirtyShaders |= file_time_newer( timeShader, timeCache );
	}

	// Log
	const u32 shaderCount = shaderFiles.size() - start;
	if( verbose_output() )
	{
		PrintColor( LOG_CYAN, TAB TAB "%u shader%s found in: %s", shaderCount, shaderCount == 1 ? "" : "s", path );
		PrintLnColor( LOG_WHITE, " (%.3f ms)", timer.elapsed_ms() );
	}
	return shaderCount;
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
		path_remove_extension( shaderName );

		// Compile Shader
		Shader &shader = Gfx::shaders.add( Shader { shaderName, shaderType } );
		compile_shader( shader, fileInfo.path );
	}
}


void Gfx::write()
{
	Buffer &binary = Assets::binary;
	Timer timer;

	// Binary
	{
		for( Shader &shader : shaders )
		{
			// Write Vertex Shader
			String &codeVertex = shader.outputs[ShaderStage_Vertex];
 			if( codeVertex.length_bytes() > 0 )
			{
				ErrorIf( binary.tell > U32_MAX, "Binary is too large for vertex shader!" );
				shader.offset[ShaderStage_Vertex] = binary.tell;
				ErrorIf( codeVertex.length_bytes() > U32_MAX, "Vertex shader is too large!" );
				shader.size[ShaderStage_Vertex] = static_cast<u32>( codeVertex.length_bytes() );
				binary.write( codeVertex.data, shader.size[ShaderStage_Vertex] );
			}

			// Write Fragment Shader
			String &codeFragment = shader.outputs[ShaderStage_Fragment];
 			if( codeFragment.length_bytes() > 0 )
			{
				ErrorIf( binary.tell >= U32_MAX, "Binary is too large for fragment shader!" );
				shader.offset[ShaderStage_Fragment] = binary.tell;
				ErrorIf( codeFragment.length_bytes() >= U32_MAX, "Fragment shader is too large!" );
				shader.size[ShaderStage_Fragment] = static_cast<u32>( codeFragment.length_bytes() );
				binary.write( codeFragment.data, shader.size[ShaderStage_Fragment] );
			}

			// Write Compute Shader
			String &codeCompute = shader.outputs[ShaderStage_Compute];
 			if( codeCompute.length_bytes() > 0 )
			{
				ErrorIf( binary.tell > U32_MAX, "Binary is too large for compute shader!" );
				shader.offset[ShaderStage_Compute] = binary.tell;
				ErrorIf( codeCompute.length_bytes() > U32_MAX, "Compute shader is too large!" );
				shader.size[ShaderStage_Compute] = static_cast<u32>( codeCompute.length_bytes() );
				binary.write( codeCompute.data, shader.size[ShaderStage_Compute] );
			}
		}
	}

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
				"BinShader",
				"u32 offsetVertex;",
				"u32 sizeVertex;",
				"u32 offsetFragment;",
				"u32 sizeFragment;",
				"u32 offsetCompute;",
				"u32 sizeCompute;",
				"u32 vertexFormat;",
				"const char *name;" );

			header.append( "enum\n{\n" );
			for( Shader &shader : shaders )
			{
				header.append( "\t" ).append( shader.name ).append( ",\n" );
			}
			header.append( "};\n\n" );

			header.append( "namespace Gfx\n{\n" );
			header.append( "\tconstexpr u32 shadersCount = " );
			header.append( static_cast<int>( Gfx::shaders.size() ) ).append( ";\n" );
			header.append( "\textern const BinShader binShaders[];\n" );
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
			header.append( "namespace GfxCoreVertex\n{\n" );
			header.append( "\ttemplate <typename T> consteval u32 vertex_format_id() { return 0; }\n" );
			for( VertexFormat &vertexFormat : Gfx::vertexFormats )
			{
				header.append( "\ttemplate <> consteval u32 vertex_format_id<GfxVertex::" ).append( vertexFormat.name );
				header.append( ">() { return " ).append( static_cast<int>( vertexFormat.id ) ).append( "; }\n" );
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
			header.append( "namespace GfxCoreUniformBuffer\n{\n" );
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
				header.append( "\textern GfxCoreUniformBuffer::" ).append( uniformBuffer.name ).append("_t " );
				header.append( uniformBuffer.name ).append( ";\n" );
			}
			header.append( "}\n\n" );
		}

		header.append( COMMENT_BREAK "\n\n" );
		{
			header.append( "namespace GfxCore\n{\n" );

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
		source.append( "#include <core/memory.hpp>\n\n" );
		source.append( "#include <manta/gfx.hpp>\n\n" );

		source.append( COMMENT_BREAK "\n\n" );
		{
			source.append( "namespace Gfx\n{\n" );

			char buffer[PATH_SIZE];
			source.append( "\tconst BinShader binShaders[shadersCount] =\n\t{\n" );
			for( Shader &shader : shaders )
			{
				snprintf( buffer, PATH_SIZE, "\t\t{ %u, %u, %u, %u, %u, %u, %u, \"%s\" },",
					shader.offset[ShaderStage_Vertex],
					shader.size[ShaderStage_Vertex],
					shader.offset[ShaderStage_Fragment],
					shader.size[ShaderStage_Fragment],
					shader.offset[ShaderStage_Compute],
					shader.size[ShaderStage_Compute],
					shader.vertexFormatID,
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
				source.append( "\tGfxCoreUniformBuffer::" ).append( uniformBuffer.name ).append("_t " );
				source.append( uniformBuffer.name ).append( ";\n" );
			}
			source.append( "}\n\n" );
		}

		source.append( COMMENT_BREAK "\n\n" );
		{
			source.append( "namespace GfxCore\n{\n" );

			source.append( "\tGfxUniformBufferResource *gfxUniformBufferResources[GfxCore::uniformBufferCount];\n\n" );

			source.append( "\tbool rb_init_uniform_buffers()\n\t{\n" );
			for( UniformBuffer &uniformBuffer : Gfx::uniformBuffers )
			{
				source.append( "\t\tgfxUniformBufferResources[" ).append( static_cast<int>( uniformBuffer.id ) );
				source.append( "] = nullptr;\n" );
				source.append( "\t\tGfxUniformBuffer::" ).append( uniformBuffer.name ).append( ".zero();\n" );
				source.append( "\t\tif( !GfxCore::rb_uniform_buffer_init( " );
				source.append( "gfxUniformBufferResources[" ).append( static_cast<int>( uniformBuffer.id ) );
				source.append( "], \"t_" ).append( uniformBuffer.name );
				source.append( "\", " ).append( static_cast<int>( uniformBuffer.id ) );
				source.append( ", sizeof( GfxCoreUniformBuffer::" );
				source.append( uniformBuffer.name ).append( "_t ) ) ) { return false; }\n\n" );
			}
			source.append( "\t\t// Success!\n" );
			source.append( "\t\treturn true;\n" );
			source.append( "\t}\n\n" );

			source.append( "\tbool rb_free_uniform_buffers()\n\t{\n" );
			source.append( "\t\tfor( u32 i = 0; i < uniformBufferCount; i++ )\n" );
			source.append( "\t\t{\n" );
			source.append( "\t\t\tif( !GfxCore::rb_uniform_buffer_free( gfxUniformBufferResources[i] ) ) { return false; }\n" );
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
					source.append( "\tstatic bool rb_shader_bind_uniform_buffers_" );
					source.append( shaderStages[stage] ).append( "_" );
					source.append( shader.name ).append( "()\n\t{\n" );
					for( usize i = 0; i < shader.uniformBufferIDs[stage].size(); i++ )
					{
						int uniformBufferID = shader.uniformBufferIDs[stage][i];
						int uniformBufferSlot = shader.uniformBufferSlots[stage][i];
						UniformBuffer &uniformBuffer = Gfx::uniformBuffers[uniformBufferID];
						source.append( "\t\tif( !GfxCore::rb_uniform_buffer_bind_" ).append( shaderStages[stage] );
						source.append( "( GfxCore::gfxUniformBufferResources[" );
						source.append( uniformBufferID ).append( "], " ).append( uniformBufferSlot );
						source.append( " ) ) { return false; }" );
						source.append( " // ").append( uniformBuffer.name ).append( "\n" );
					}
					source.append( "\t\treturn true;\n" );
					source.append( "\t}\n\n" );
				}
			}

			// rb_shader_bind_uniform_buffers_vs/fs/cs[] function pointer table
			for( ShaderStage stage = 0; stage < SHADERSTAGE_COUNT; stage++ )
			{
				if( stage != 0 ) { source.append( "\n" ); }
				source.append( "\tFUNCTION_POINTER_ARRAY( bool, rb_shader_bind_uniform_buffers_" );
				source.append( shaderStages[stage] ).append( " ) =\n\t{\n" );
				for( Shader &shader : shaders )
				{
					source.append( "\t\trb_shader_bind_uniform_buffers_" ).append( shaderStages[stage] ).append( "_" );
					source.append( shader.name ).append( ",\n" );
				}
				source.append( "\t};\n" );
			}

			source.append( "}\n\n" );
		}

		source.append( COMMENT_BREAK "\n\n" );
		{
			source.append( "namespace GfxCoreUniformBuffer\n{\n" );
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
		PrintColor( LOG_CYAN, TAB TAB "Wrote %d shaders%s", count, count == 1 ? "s" : "" );
		PrintLnColor( LOG_WHITE, " (%.3f ms)", timer.elapsed_ms() );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Gfx::write_header_api_opengl( String &header )
{
	// OpenGL Header
	header.append( "#include <manta/backend/gfx/opengl/opengl.hpp>\n\n" );

	// GfxCore
	header.append( "namespace GfxCore\n{\n" );
	header.append( COMMENT_BREAK "\n\n" );
	{
		for( Shader &shader : shaders )
		{
			header.append( shader.header );
		}

		header.append( "extern FUNCTION_POINTER_ARRAY( void, opengl_vertex_input_layout_init, GLuint );\n\n" );
		header.append( "extern FUNCTION_POINTER_ARRAY( void, opengl_vertex_input_layout_bind );\n\n" );
	}
	header.append( COMMENT_BREAK "\n" );
	header.append( "}" );
}


void Gfx::write_source_api_opengl( String &source )
{
	// GfxCore
	source.append( "namespace GfxCore\n{\n" );
	source.append( COMMENT_BREAK "\n\n" );
	{
		for( Shader &shader : shaders )
		{
			source.append( shader.source );
		}

		source.append( "FUNCTION_POINTER_ARRAY( void, opengl_vertex_input_layout_init, GLuint ) =\n{\n" );
		for( VertexFormat &vertexFormat : vertexFormats )
		{
			source.append( TAB ).append( "opengl_vertex_input_layout_init_" );
			source.append( vertexFormat.name ).append( ",\n" );
		}
		source.append( "};\n\n" );

		source.append( "FUNCTION_POINTER_ARRAY( void, opengl_vertex_input_layout_bind ) =\n{\n" );
		for( VertexFormat &vertexFormat : vertexFormats )
		{
			source.append( TAB ).append( "opengl_vertex_input_layout_bind_" );
			source.append( vertexFormat.name ).append( ",\n" );
		}
		source.append( "};\n\n" );
	}
	source.append( COMMENT_BREAK "\n" );
	source.append( "}" );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Gfx::write_header_api_d3d11( String &header )
{
	// D3D11 Header
	header.append( "#include <vendor/d3d11.hpp>\n\n" );

	// GfxCore
	header.append( "namespace GfxCore\n{\n" );
	header.append( COMMENT_BREAK "\n\n" );
	{
		for( Shader &shader : shaders )
		{
			header.append( shader.header );
		}

		header.append( "struct D3D11VertexInputLayoutDescription\n{\n" );
		header.append( "\tD3D11_INPUT_ELEMENT_DESC *desc;\n" );
		header.append( "\tint count;\n" );
		header.append( "};\n\n" );

		header.append( "extern FUNCTION_POINTER_ARRAY( void, d3d11_vertex_input_layout_desc, " );
		header.append( "D3D11VertexInputLayoutDescription & );\n\n" );
	}
	header.append( COMMENT_BREAK "\n" );
	header.append( "}" );
}


void Gfx::write_source_api_d3d11( String &source )
{
	// GfxCore
	source.append( "namespace GfxCore\n{\n" );
	source.append( COMMENT_BREAK "\n\n" );
	{
		for( Shader &shader : shaders )
		{
			source.append( shader.source );
		}

		source.append( "FUNCTION_POINTER_ARRAY( void, d3d11_vertex_input_layout_desc, " );
		source.append( "D3D11VertexInputLayoutDescription & ) =\n{\n" );
		for( VertexFormat &vertexFormat : vertexFormats )
		{
			source.append( TAB ).append( "d3d11_vertex_input_layout_desc_" );
			source.append( vertexFormat.name ).append( ",\n" );
		}
		source.append( "};\n\n" );
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

	// GfxCore
	header.append( "namespace GfxCore\n{\n" );
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
	// GfxCore
	source.append( "namespace GfxCore\n{\n" );
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