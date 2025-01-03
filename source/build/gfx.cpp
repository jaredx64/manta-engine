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

	// Vertex Structs
	List<VertexFormat> vertexFormats;
	HashMap<u32, u32> vertexFormatCache;

	// Constant Buffers
	List<ConstantBuffer> constantBuffers;
	HashMap<u32, u32> constantBufferCache;
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
	#if GRAPHICS_OPENGL
		const ShaderType shaderType = ShaderType_GLSL;
	#elif GRAPHICS_D3D11 || GRAPHICS_D3D12
		const ShaderType shaderType = ShaderType_HLSL;
	#elif GRAPHICS_VULKAN
		const ShaderType shaderType = ShaderType_GLSL;
	#elif GRAPHICS_METAL
		const ShaderType shaderType = ShaderType_METAL;
	#else
		const ShaderType shaderType = ShaderType_DEFAULT;
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
		header.append( "#include <manta/math.hpp>\n\n#include <manta/vector.hpp>\n\n" );

		header.append( COMMENT_BREAK "\n\n" );
		{
			assets_struct( header,
				"DiskShader",
				"u32 offsetVertex;",
				"u32 sizeVertex;",
				"u32 offsetFragment;",
				"u32 sizeFragment;",
				"u32 offsetCompute;",
				"u32 sizeCompute;",
				"u32 vertexFormat;" );

			header.append( "enum\n{\n" );
			for( Shader &shader : shaders )
			{
				header.append( "\t" ).append( shader.name ).append( ",\n" );
			}
			header.append( "};\n\n" );

			header.append( "namespace Gfx\n{\n" );
			header.append( "\tconstexpr u32 shadersCount = " );
			header.append( static_cast<int>( Gfx::shaders.size() ) ).append( ";\n" );
			header.append( "\textern const DiskShader diskShaders[];\n" );
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
			header.append( "namespace GfxCoreCBuffer\n{\n" );
			for( ConstantBuffer &cbuffer : Gfx::constantBuffers )
			{
				header.append( cbuffer.header );
			}
			header.append( "}\n\n" );
		}

		header.append( COMMENT_BREAK "\n\n" );
		{
			header.append( "namespace GfxCBuffer\n{\n" );
			for( ConstantBuffer &cbuffer : Gfx::constantBuffers )
			{
				header.append( "\textern GfxCoreCBuffer::" ).append( cbuffer.name ).append("_t " );
				header.append( cbuffer.name ).append( ";\n" );
			}
			//header.append( Gfx::constantBuffersSource );
			header.append( "}\n\n" );
		}

		header.append( COMMENT_BREAK "\n\n" );
		{
			header.append( "namespace GfxCore\n{\n" );

			header.append( "\tconstexpr u32 constantBufferCount = " );
			header.append( static_cast<int>( Gfx::constantBuffers.size() ) ).append( ";\n\n" );

			header.append( "}\n\n" );
		}

		// Save
		header.save( Gfx::pathHeaderGfx );
	}

	// Source (Gfx)
	{
		String &source = Gfx::sourceGfx;
		source.append( "#include <gfx.generated.hpp>\n\n" );
		source.append( "#include <core/memory.hpp>\n\n" );
		source.append( "#include <manta/gfx.hpp>\n" );

		source.append( COMMENT_BREAK "\n\n" );
		{
			source.append( "namespace Gfx\n{\n" );

			char buffer[PATH_SIZE];
			source.append( "\tconst DiskShader diskShaders[shadersCount] =\n\t{\n" );
			for( Shader &shader : shaders )
			{
				snprintf( buffer, PATH_SIZE, "\t\t{ %u, %u, %u, %u, %u, %u, %u },",
					shader.offset[ShaderStage_Vertex],
					shader.size[ShaderStage_Vertex],
					shader.offset[ShaderStage_Fragment],
					shader.size[ShaderStage_Fragment],
					shader.offset[ShaderStage_Compute],
					shader.size[ShaderStage_Compute],
					shader.vertexFormatID );

				source.append( buffer );
				source.append( " // " ).append( shader.name ).append( "\n" );
			}
			source.append( "\t};\n" );
			source.append( "}\n\n" );
		}

		source.append( COMMENT_BREAK "\n\n" );
		{
			source.append( "namespace GfxCBuffer\n{\n" );
			for( ConstantBuffer &cbuffer : Gfx::constantBuffers )
			{
				source.append( "\tGfxCoreCBuffer::" ).append( cbuffer.name ).append("_t " );
				source.append( cbuffer.name ).append( ";\n" );
			}
			//source.append( Gfx::constantBuffersSource );
			source.append( "}\n\n" );
		}

		source.append( COMMENT_BREAK "\n\n" );
		{
			source.append( "namespace GfxCore\n{\n" );

			source.append( "\tGfxConstantBufferResource *gfxCBufferResources[GfxCore::constantBufferCount];\n\n" );

			source.append( "\tbool rb_init_cbuffers()\n\t{\n" );
			for( ConstantBuffer &cbuffer : Gfx::constantBuffers )
			{
				source.append( "\t\tgfxCBufferResources[" ).append( static_cast<int>( cbuffer.id ) ).append( "] = nullptr;\n" );
				source.append( "\t\tGfxCBuffer::" ).append( cbuffer.name ).append( ".zero();\n" );
				source.append( "\t\tif( !GfxCore::rb_constant_buffer_init( " );
				source.append( "gfxCBufferResources[" ).append( static_cast<int>( cbuffer.id ) ).append( "], \"t_" ).append( cbuffer.name );
				source.append( "\", " ).append( static_cast<int>( cbuffer.id ) ).append( ", sizeof( GfxCoreCBuffer::" );
				source.append( cbuffer.name ).append( "_t ) ) ) { return false; }\n\n" );
			}
			source.append( "\t\t// Success!\n" );
			source.append( "\t\treturn true;\n" );
			source.append( "\t}\n\n" );

			source.append( "\tbool rb_free_cbuffers()\n\t{\n" );
			source.append( "\t\tfor( u32 i = 0; i < constantBufferCount; i++ )\n" );
			source.append( "\t\t{\n" );
			source.append( "\t\t\tif( !GfxCore::rb_constant_buffer_free( gfxCBufferResources[i] ) ) { return false; }\n" );
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

			// shader_bind_constant_buffers_*
			for( Shader &shader : shaders )
			{
				for( ShaderStage stage = 0; stage < SHADERSTAGE_COUNT; stage++ )
				{
					source.append( "\tstatic bool rb_shader_bind_constant_buffers_" ).append( shaderStages[stage] ).append( "_" );
					source.append( shader.name ).append( "()\n\t{\n" );
					for( usize i = 0; i < shader.constantBufferIDs[stage].size(); i++ )
					{
						int cbufferID = shader.constantBufferIDs[stage][i];
						int cbufferSlot = shader.constantBufferSlots[stage][i];
						ConstantBuffer &cbuffer = Gfx::constantBuffers[cbufferID];
						source.append( "\t\tif( !GfxCore::rb_constant_buffer_bind_" ).append( shaderStages[stage] );
						source.append( "( GfxCore::gfxCBufferResources[" );
						source.append( cbufferID ).append( "], " ).append( cbufferSlot ).append ( " ) ) { return false; }" );
						source.append( " // ").append( cbuffer.name ).append( "\n" );
					}
					source.append( "\t\treturn true;\n" );
					source.append( "\t}\n\n" );
				}
			}

			// rb_shader_bind_constant_buffers_vs/fs/cs[] function pointer table
			for( ShaderStage stage = 0; stage < SHADERSTAGE_COUNT; stage++ )
			{
				if( stage != 0 ) { source.append( "\n" ); }
				source.append( "\tFUNCTION_POINTER_ARRAY( bool, rb_shader_bind_constant_buffers_" );
				source.append( shaderStages[stage] ).append( " ) =\n\t{\n" );
				for( Shader &shader : shaders )
				{
					source.append( "\t\trb_shader_bind_constant_buffers_" ).append( shaderStages[stage] ).append( "_" );
					source.append( shader.name ).append( ",\n" );
				}
				source.append( "\t};\n" );
			}

			source.append( "}\n\n" );
		}

		source.append( COMMENT_BREAK "\n\n" );
		{
			source.append( "namespace GfxCoreCBuffer\n{\n" );
			for( ConstantBuffer &cbuffer : Gfx::constantBuffers )
			{
				source.append( cbuffer.source );
			}
			source.append( "}\n\n" );
		}

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
			// ...
		#elif GRAPHICS_VULKAN
			// ...
		#elif GRAPHICS_METAL
			// ...
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
			// ...
		#elif GRAPHICS_VULKAN
			// ...
		#elif GRAPHICS_METAL
			// ...
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

		header.append( "extern FUNCTION_POINTER_ARRAY( void, d3d11_vertex_input_layout_desc, D3D11VertexInputLayoutDescription & );\n\n" );
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

		source.append( "FUNCTION_POINTER_ARRAY( void, d3d11_vertex_input_layout_desc, D3D11VertexInputLayoutDescription & ) =\n{\n" );
		for( VertexFormat &vertexFormat : vertexFormats )
		{
			source.append( TAB ).append( "d3d11_vertex_input_layout_desc_" ).append( vertexFormat.name ).append( ",\n" );
		}
		source.append( "};\n\n" );
	}
	source.append( COMMENT_BREAK "\n" );
	source.append( "}" );
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
			source.append( TAB ).append( "opengl_vertex_input_layout_init_" ).append( vertexFormat.name ).append( ",\n" );
		}
		source.append( "};\n\n" );

		source.append( "FUNCTION_POINTER_ARRAY( void, opengl_vertex_input_layout_bind ) =\n{\n" );
		for( VertexFormat &vertexFormat : vertexFormats )
		{
			source.append( TAB ).append( "opengl_vertex_input_layout_bind_" ).append( vertexFormat.name ).append( ",\n" );
		}
		source.append( "};\n\n" );
	}
	source.append( COMMENT_BREAK "\n" );
	source.append( "}" );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////