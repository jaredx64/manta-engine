#include <build/shaders/compiler.hpp>
#include <build/shaders/compiler.parser.hpp>
#include <build/shaders/compiler.optimizer.hpp>
#include <build/shaders/compiler.generator.hpp>
#include <build/shaders/compiler.preprocessor.hpp>

#include <build/build.hpp>
#include <build/filesystem.hpp>
#include <build/time.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void compile_shader( Shader &shader, const char *path )
{
	// Setup
	char filename[PATH_SIZE];
	char filenameOutput[PATH_SIZE];
	char pathInput[PATH_SIZE];
	char pathPreprocessed[PATH_SIZE];
	char pathOutput[PATH_SIZE];

	// Paths
	{
		// Input
		strjoin( pathInput, path );

		// Filename
		path_get_filename( filename, sizeof( filename ), pathInput );
		path_change_extension( filename, sizeof( filename ), filename, "" );

		// Preprocess
		strjoin( pathPreprocessed, Build::pathOutputGeneratedShaders, SLASH, filename, ".preprocessed" );

		// Output
		const char *shaderTypeExtensions[] =
		{
			".generated.none",  // ShaderType_NONE
			".generated.hlsl",  // ShaderType_HLSL
			".generated.glsl",  // ShaderType_GLSL
			".generated.metal", // ShaderType_METAL
		};
		static_assert( ARRAY_LENGTH( shaderTypeExtensions ) == SHADERTYPE_COUNT, "Missing ShaderType!" );

		strjoin( pathOutput, Build::pathOutputGeneratedShaders, SLASH, filename, shaderTypeExtensions[shader.type] );
	}

	// Preprocessor
#if 1
	static const char *pipelineMacros[1];
	switch( shader.type )
	{
		case ShaderType_HLSL: pipelineMacros[0] = "SHADER_HLSL"; break;
		case ShaderType_GLSL: pipelineMacros[0] = "SHADER_GLSL"; break;
		case ShaderType_METAL: pipelineMacros[0] = "SHADER_METAL"; break;
		default: pipelineMacros[0] = ""; break;
	}

	String file;
	preprocess_shader( pathInput, file, pipelineMacros, 1 );
#else
	// Invoke Official C Preprocessor
	String file;
	{
		const bool useMSVC = strcmp( Build::args.toolchain, "msvc" ) == 0;
		const bool useGNU = strcmp( Build::args.toolchain, "gnu" ) == 0;
		const bool useLLVM = strcmp( Build::args.toolchain, "llvm" ) == 0;

		char pathAPI[PATH_SIZE]; // Path to dummy "shader_api.hpp"
		strjoin_path( pathAPI, "source", "build", "shaders", "preprocess" );

		char pipelineDefines[PATH_SIZE]; pipelineDefines[0] = '\0';
		const char *pipelineDefinePrefix = useMSVC ? "/D" : "-D";

		switch( shader.type )
		{
			case ShaderType_HLSL:
			{
				strappend( pipelineDefines, pipelineDefinePrefix );
				strappend( pipelineDefines, "SHADER_HLSL " );
			}
			break;

			case ShaderType_GLSL:
			{
				strappend( pipelineDefines, pipelineDefinePrefix );
				strappend( pipelineDefines, "SHADER_GLSL " );
			}
			break;

			case ShaderType_METAL:
			{
				strappend( pipelineDefines, pipelineDefinePrefix );
				strappend( pipelineDefines, "SHADER_METAL " );
			}
			break;
		}

		char commandPreprocessor[1024];
		if( useMSVC )
		{
			// MSVC Preprocessor
			strjoin( commandPreprocessor, "cl -nologo /EP /I\"", pathAPI, "\" /TP ",
				pipelineDefines, " ", pathInput, " > ", pathPreprocessed );
		}
		else if( useGNU )
		{
			// GCC Preprocessor
			strjoin( commandPreprocessor, "gcc -E -P -I", pathAPI, " ", pipelineDefines,
				" -x c ", pathInput, " > ", pathPreprocessed );
		}
		else if( useLLVM )
		{
			// Clang Preprocessor
			strjoin( commandPreprocessor, "gcc -E -P -I", pathAPI, " ", pipelineDefines,
				" -x c ", pathInput, " > ", pathPreprocessed );
		}

		const int errorCode = system( commandPreprocessor );
		ErrorIf( errorCode != 0, "Failed to preprocess shader '%s'", pathAPI );
		ErrorIf( !file.load( pathPreprocessed ),
			"Failed to load preprocessed shader code '%s'", pathPreprocessed );
	}
#endif

	// Parse
	ShaderCompiler::Parser parser { shader, pathPreprocessed };
	{
		parser.parse( reinterpret_cast<char *>( file.data ) );
	}

	// Generate
	{
		for( ShaderStage stage = 0; stage < SHADERSTAGE_COUNT; stage++ )
		{
			// Optimize
			ShaderCompiler::Optimizer optimizer { parser };
			optimizer.optimize_stage( stage );

			// Generate
			switch( shader.type )
			{
				case ShaderType_HLSL:
				{
					ShaderCompiler::GeneratorHLSL generator { shader, stage, parser };
					generator.generate_stage( stage );
				}
				break;

				case ShaderType_GLSL:
				{
					ShaderCompiler::GeneratorGLSL generator { shader, stage, parser };
					generator.generate_stage( stage );
				}
				break;

				case ShaderType_METAL:
				{
					ShaderCompiler::GeneratorMetal generator { shader, stage, parser };
					generator.generate_stage( stage );
				}
				break;

				case ShaderType_NONE:
				{
					ShaderCompiler::GeneratorGLSL generator { shader, stage, parser };
					generator.generate_stage( stage );
				}
				break;

				default:
				{
					Error( "Unsupported shader type!" );
				}
				break;
			}
		}
	}

	// Write
	{
#if COMPILE_DEBUG
		String output = "";
		output.append( COMMENT_BREAK );
		for( ShaderStage stage = 0; stage < SHADERSTAGE_COUNT; stage++ )
		{
			// Combine Stage (Debug)
			output.append( "\n// " ).append( ShaderStageName[stage] ).append( "\n\n" );
			output.append( shader.outputs[stage] );
			output.append( COMMENT_BREAK );
		}
		output.save( pathOutput );
#endif
	}
}


namespace ShaderCompiler
{
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define NODE_BUFFER_CAPACITY_BYTES ( 1024 * 1024 ) // 1 MB

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void NodeBuffer::init()
{
	grow();
}


void NodeBuffer::free()
{
	for( byte *page : pages )
	{
		Assert( page != nullptr );
		memory_free( page );
	}

	data = nullptr;
	current = 0LLU;
	capacity = 0LLU;
}


void NodeBuffer::clear()
{
	free();
	pages.clear();
	grow();
}


void NodeBuffer::grow()
{
	// NodeBuffer allocates new memory in 'pages' to avoid calling realloc (which can to invalidate pointers)
	data = reinterpret_cast<byte *>( memory_alloc( NODE_BUFFER_CAPACITY_BYTES ) );
	capacity = NODE_BUFFER_CAPACITY_BYTES;
	current = 0LLU;

	// Ownership of the memory block is stored in the pages list
	pages.add( data );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}