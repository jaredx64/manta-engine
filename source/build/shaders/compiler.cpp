#include <build/shaders/compiler.hpp>
#include <build/shaders/compiler.parser.hpp>
#include <build/shaders/compiler.optimizer.hpp>
#include <build/shaders/compiler.generator.hpp>

#include <build/build.hpp>
#include <build/filesystem.hpp>
#include <build/time.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void compile_shader( Shader &shader, const char *path )
{
	// Setup
	Timer profiler;
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
			".generated.shader", // ShaderType_NONE
			".generated.hlsl",   // ShaderType_HLSL
			".generated.glsl",   // ShaderType_GLSL
			".generated.metal",  // ShaderType_METAL
		};
		static_assert( ARRAY_LENGTH( shaderTypeExtensions ) == SHADERTYPE_COUNT, "Missing ShaderType!" );

		strjoin( pathOutput, Build::pathOutputGeneratedShaders, SLASH, filename, shaderTypeExtensions[shader.type] );
	}

	// Preprocessor
	String file;
	{
		char pathAPI[PATH_SIZE]; // Path to dummy "shader_api.hpp"
		strjoin_path( pathAPI, "source", "build", "shaders", "preprocess" );

		char commandPreprocessor[1024];
		if( strcmp( Build::args.toolchain, "msvc" ) == 0 )
		{
			// MSVC Preprocessor
			strjoin( commandPreprocessor, "cl -nologo /EP /I\"", pathAPI ,"\" /TP ", pathInput, " > ", pathPreprocessed );
		}
		else if( strcmp( Build::args.toolchain, "gnu" ) == 0 )
		{
			// GCC Preprocessor
			strjoin( commandPreprocessor, "gcc -E -P -I", pathAPI ," -x c ", pathInput, " > ", pathPreprocessed );
		}
		else if( strcmp( Build::args.toolchain, "llvm" ) == 0 )
		{
			// Clang Preprocessor
			strjoin( commandPreprocessor, "gcc -E -P -I", pathAPI ," -x c ", pathInput, " > ", pathPreprocessed );
		}

		const int errorCode = system( commandPreprocessor );
		ErrorIf( errorCode != 0, "Failed to preprocess shader '%s'", pathAPI );
		ErrorIf( !file.load( pathPreprocessed ), "Failed to load preprocessed shader code '%s'", pathPreprocessed );
	}

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
		for( ShaderStage stage = 0; stage < SHADERSTAGE_COUNT; stage++ )
		{
			// Combine Stage (Debug)
			output.append( COMMENT_BREAK "\n\n" );
			output.append( shader.outputs[stage] );
		}
		output.save( pathOutput );
	#endif
	}

	// Finished
	{
		//PrintLnColor( LOG_GREEN, "Finished: %.3f ms", profiler.elapsed_ms() );
	}
}


namespace ShaderCompiler
{
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define NODE_BUFFER_CAPACITY_BYTES ( 1024 * 256 ) // 0.25 KB

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void NodeBuffer::init()
{
	// Init Page
	grow();
}


void NodeBuffer::free()
{
	// Free Pages
	for( byte *page : pages )
	{
		Assert( page != nullptr );
		memory_free( page );
	}
}


void NodeBuffer::grow()
{
	// Allocate New Page
	data = reinterpret_cast<byte *>( memory_alloc( NODE_BUFFER_CAPACITY_BYTES ) );
	pages.add( data );
	current = 0;
	capacity = NODE_BUFFER_CAPACITY_BYTES;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}