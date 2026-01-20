#include <build/shaders/compiler.generator.hpp>

#include <core/string.hpp>

#include <vendor/stdio.hpp>

namespace ShaderCompiler
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct OpenGLInputFormatInfo { const char *format; int size; bool normalized; };

static const OpenGLInputFormatInfo OPENGL_INPUT_FORMATS[] =
{
	{ "GL_UNSIGNED_BYTE", 1, true },   // InputFormat_UNORM8
	{ "GL_UNSIGNED_SHORT", 2, true },  // InputFormat_UNORM16
	{ "GL_UNSIGNED_INT", 4, true },    // InputFormat_UNORM32
	{ "GL_BYTE", 1, true },            // InputFormat_SNORM8
	{ "GL_SHORT", 2, true },           // InputFormat_SNORM16
	{ "GL_INT", 4, true },             // InputFormat_SNORM32
	{ "GL_UNSIGNED_BYTE", 1, false },  // InputFormat_UINT8
	{ "GL_UNSIGNED_SHORT", 2, false }, // InputFormat_UINT16
	{ "GL_UNSIGNED_INT", 4, false },   // InputFormat_UINT32
	{ "GL_BYTE", 1, false },           // InputFormat_SINT8
	{ "GL_SHORT", 2, false },          // InputFormat_SINT16
	{ "GL_INT", 4, false },            // InputFormat_SINT32
	{ "GL_HALF_FLOAT", 2, false },     // InputFormat_FLOAT16
	{ "GL_FLOAT", 4, false },          // InputFormat_FLOAT32
};


static int input_format_components( const Primitive primitive )
{
	switch( primitive )
	{
		case Primitive_Bool:
		case Primitive_Int:
		case Primitive_UInt:
		case Primitive_Float:
			return 1;

		case Primitive_Bool2:
		case Primitive_Int2:
		case Primitive_UInt2:
		case Primitive_Float2:
			return 2;

		case Primitive_Bool3:
		case Primitive_Int3:
		case Primitive_UInt3:
		case Primitive_Float3:
			return 3;

		case Primitive_Bool4:
		case Primitive_Int4:
		case Primitive_UInt4:
		case Primitive_Float4:
			return 4;

		default:
			return 0;
	}
}


static const char *input_attibute_type( const Primitive primitive )
{
	switch( primitive )
	{
		case Primitive_Int:
		case Primitive_UInt:
		case Primitive_Int2:
		case Primitive_UInt2:
		case Primitive_Int3:
		case Primitive_UInt3:
		case Primitive_Int4:
		case Primitive_UInt4:
			return "OpenGLInputAttributeType_INTEGER";
		break;

		default:
			return "OpenGLInputAttributeType_FLOAT";
	}
}


static const char *GLSLPrimitives[] =
{
	"void",             // Primitive_Void
	"bool",             // Primitive_Bool
	"bvec2",            // Primitive_Bool2
	"bvec3",            // Primitive_Bool3
	"bvec4",            // Primitive_Bool4
	"int",              // Primitive_Int
	"ivec2",            // Primitive_Int2
	"ivec3",            // Primitive_Int3
	"ivec4",            // Primitive_Int4
	"uint",             // Primitive_UInt
	"uvec2",            // Primitive_UInt2
	"uvec3",            // Primitive_UInt3
	"uvec4",            // Primitive_UInt4
	"float",            // Primitive_Float
	"vec2",             // Primitive_Float2
	"vec3",             // Primitive_Float3
	"vec4",             // Primitive_Float4
	"mat2",             // Primitive_Float2x2
	"mat3",             // Primitive_Float3x3
	"mat4",             // Primitive_Float4x4
	"sampler1D",        // Primitive_Texture1D
	"sampler1DArray",   // Primitive_Texture1DArray
	"sampler2D",        // Primitive_Texture2D
	"sampler2DArray",   // Primitive_Texture2DArray
	"sampler3D",        // Primitive_Texture3D
	"samplerCube",      // Primitive_TextureCube
	"samplerCubeArray", // Primitive_TextureCubeArray
};
static_assert( ARRAY_LENGTH( GLSLPrimitives ) == PRIMITIVE_COUNT, "Missing Primitive!" );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GeneratorGLSL::process_names()
{
	// Types
	{
		for( TypeID typeID = 0; typeID < parser.types.size(); typeID++ )
		{
			String &name = typeNames.add( "" );
			Type &type = parser.types[typeID];

			if( typeID < PRIMITIVE_COUNT )
			{
				// Built-in Types
				name.append( GLSLPrimitives[typeID] );
			}
			else
			{
				// Custom Types
				#if SHADER_OUTPUT_PREFIX_IDENTIFIERS
					name.append( "t_" ).append( type.name );
				#else
					name.append( type.name );
				#endif
			}
		}
	}

	// Functions
	{
		for( FunctionID functionID = 0; functionID < parser.functions.size(); functionID++ )
		{
			String &name = functionNames.add( "" );
			Function &function = parser.functions[functionID];

			if( functionID < INTRINSIC_COUNT )
			{
				// Built-in Functions
				name.append( function.name );
			}
			else
			{
				// Custom Types
				#if SHADER_OUTPUT_PREFIX_IDENTIFIERS
					name.append( "f_" ).append( function.name );
				#else
					name.append( function.name );
				#endif
			}
		}
	}

	// Variables
	{
		for( VariableID variableID = 0; variableID < parser.variables.size(); variableID++ )
		{
			String &name = variableNames.add( "" );
			Variable &variable = parser.variables[variableID];

			switch( variable.texture )
			{
				case Primitive_Texture1D:
				case Primitive_Texture1DArray:
				case Primitive_Texture2D:
				case Primitive_Texture2DArray:
				case Primitive_Texture3D:
				case Primitive_TextureCube:
				case Primitive_TextureCubeArray:
				{
					name.append( "u_texture" ).append( variable.slot );
				}
				break;

				default:
				{
				#if SHADER_OUTPUT_PREFIX_IDENTIFIERS
					name.append( "v_" ).append( variable.name );
				#else
					name.append( variable.name );
				#endif
				}
				break;
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GeneratorGLSL::generate_stage( ShaderStage stage )
{
	// GLSL Version
	output.append( "#version 410 core\n\n" );
	//output.append( "#extension GL_ARB_separate_shader_objects : enable\n\n" );

	// Compute Extensions
	if( stage == ShaderStage_Compute )
	{
		output.append( "#extension GL_ARB_compute_shader : enable\n" );
		output.append( "#extension GL_ARB_shading_language_420pack : enable\n\n" );
	}

	// Super
	Generator::generate_stage( stage );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GeneratorGLSL::generate_function_declaration( NodeFunctionDeclaration *node )
{
	switch( node->functionType )
	{
		case FunctionType_MainVertex:
		case FunctionType_MainFragment:
			generate_function_declaration_main_pipeline( node );
		return;

		case FunctionType_MainCompute:
			generate_function_declaration_main_compute( node );
		return;
	}

	Function &function = parser.functions[node->functionID];
	const String &returnTypeName = type_name( function.typeID );
	const String &functionName = function_name( node->functionID );

	// Return Type & Name
	output.append( indent ).append( returnTypeName ).append( " " );
	output.append( functionName );

	// Parameter List
	const VariableID first = function.parameterFirst;
	const VariableID last = first + function.parameterCount;

	if( first != last )
	{
		output.append( "(" );
		VariableID parameterCount = 0;
		for( VariableID i = first; i < last; i++ )
		{
			Variable &parameter = parser.variables[i];
			Type &parameterType = parser.types[parameter.typeID];

			// Skip buffers (global namespace)
			if( parameterType.tokenType == TokenType_UniformBuffer ||
				parameterType.tokenType == TokenType_ConstantBuffer ||
				parameterType.tokenType == TokenType_MutableBuffer )
			{
				continue;
			}

			output.append( parameterCount == 0 ? " " : ", " );
			const String &parameterTypeName = type_name( parameter.typeID );
			const String &parameterVariableName = variable_name( i );

			// <in> / <out> / <inout>
			if(  parameter.in && !parameter.out ) { output.append( "in " ); } else
			if( !parameter.in &&  parameter.out ) { output.append( "out " ); } else
			if(  parameter.in &&  parameter.out ) { output.append( "inout " ); }

			// <const>
			if( parameter.constant ) { output.append( "const " ); }

			// <type> <variable>
			output.append( parameterTypeName ).append( " " ).append( parameterVariableName );
			parameterCount++;
		}
		output.append( parameterCount > 0 ? " )\n" : ")\n" );
	}
	else
	{
		output.append( "()\n" );
	}

	// Body
	generate_statement_block( reinterpret_cast<NodeStatementBlock *>( node->block ) );
	output.append( "\n" );
}


void GeneratorGLSL::generate_function_declaration_main_pipeline( NodeFunctionDeclaration *node )
{
	Function &function = parser.functions[node->functionID];
	Type &returnType = parser.types[function.typeID];

	// Return Type & Name
	output.append( indent ).append( "void main()\n" );
	generate_statement_block( reinterpret_cast<NodeStatementBlock *>( node->block ) );
	output.append( "\n" );
}


void GeneratorGLSL::generate_function_declaration_main_compute( NodeFunctionDeclaration *node )
{
	Function &function = parser.functions[node->functionID];
	Type &returnType = parser.types[function.typeID];

	// thread Groups
	output.append( indent ).append( "layout( " );
	output.append( "local_size_x = " ).append( parser.threadGroupX ).append( ", " );
	output.append( "local_size_y = " ).append( parser.threadGroupY ).append( ", " );
	output.append( "local_size_z = " ).append( parser.threadGroupZ ).append( " ) in;\n\n" );

	// Return Type & Name
	output.append( indent ).append( "void main()\n" );
	generate_statement_block( reinterpret_cast<NodeStatementBlock *>( node->block ) );
	output.append( "\n" );
}


static NodeExpressionList *get_param( Node *paramFirst, u32 paramIndex )
{
	u32 paramCount = 0;
	Node *param = paramFirst;
	while( param != nullptr )
	{
		Assert( param->nodeType == NodeType_ExpressionListNode );
		NodeExpressionList *paramNode = reinterpret_cast<NodeExpressionList *>( param );
		if( paramCount == paramIndex ) { return paramNode; }
		param = paramNode->next;
		paramCount++;
	}

	Error( "unable to get parameter %u", paramIndex );
	return nullptr;
}


void GeneratorGLSL::generate_function_call_intrinsics( NodeFunctionCall *node )
{
	Intrinsic intrinsic = node->functionID;
	Assert( intrinsic < INTRINSIC_COUNT );

	// Helper to output UV with flipped V
	auto generate_node_uv_flipped = [this]( Node *node )
		{
			output.append( "vec2( " );
				generate_node_parenthesis( node );
				output.append( ".x, 1.0 - " );
				generate_node_parenthesis( node );
				output.append( ".y" );
			output.append( " )" );
		};

	// Helper to output UVW with flipped V
	auto generate_node_uvw_flipped = [this]( Node *node )
		{
			output.append( "vec3( " );
				generate_node_parenthesis( node );
				output.append( ".x, 1.0 - " );
				generate_node_parenthesis( node );
				output.append( ".y, " );
				generate_node_parenthesis( node );
				output.append( ".z" );
			output.append( " )" );
		};

	switch( intrinsic )
	{
	// Trigonometric Functions
		case Intrinsic_Cos:
		case Intrinsic_Sin:
		case Intrinsic_Tan:
		case Intrinsic_Sinh:
		case Intrinsic_Cosh:
		case Intrinsic_Tanh:
		case Intrinsic_ASin:
		case Intrinsic_ACos:
		case Intrinsic_ATan:
		{
			generate_function_call( node );
		}
		return;

		case Intrinsic_ATan2:
		{
			output.append( "atan" );
			generate_function_call_parameters( node );
		}
		return;

	// Exponential and Logarithmic Functions
		case Intrinsic_Exp:
		case Intrinsic_Exp2:
		case Intrinsic_Log:
		case Intrinsic_Log2:
		{
			generate_function_call( node );
		}
		return;

	// Conversion Functions
		case Intrinsic_Degrees:
		case Intrinsic_Radians:
		case Intrinsic_Round:
		case Intrinsic_Trunc:
		case Intrinsic_Ceil:
		case Intrinsic_Floor:
		{
			generate_function_call( node );
		}
		return;

	// Arithmetic Functions
		case Intrinsic_Abs:
		case Intrinsic_Pow:
		case Intrinsic_Sqrt:
		case Intrinsic_Clamp:
		case Intrinsic_Max:
		case Intrinsic_Min:
		case Intrinsic_Mod:
		case Intrinsic_Ldexp:
		case Intrinsic_Fma:
		case Intrinsic_Sign:
		{
			generate_function_call( node );
		}
		return;

		case Intrinsic_RSqrt:
		{
			output.append( "inversesqrt" );
			generate_function_call_parameters( node );
		}
		return;

		case Intrinsic_Frac:
		{
			output.append( "fract" );
			generate_function_call_parameters( node );
		}
		return;

		case Intrinsic_Saturate:
		{
			output.append( "clamp( " );
			generate_node( get_param( node->param, 0 )->expr );
			output.append( ", 0.0, 1.0 )" );
		}
		return;

		case Intrinsic_DDx:
		case Intrinsic_DDy:
		case Intrinsic_DDxCoarse:
		case Intrinsic_DDxFine:
		case Intrinsic_DDyCoarse:
		case Intrinsic_DDyFine:
		{
			static const char *ddOperations[] =
			{
				"dFdx",       // Intrinsic_DDx
				"dFdy",       // Intrinsic_DDy
				"dFdxCoarse", // Intrinsic_DDxCoarse
				"dFdxFine",   // Intrinsic_DDxFine
				"dFdxCoarse", // Intrinsic_DDyCoarse
				"dFdxFine",   // Intrinsic_DDyFine
			};
			static_assert( ARRAY_LENGTH( ddOperations ) == ( Intrinsic_DDyFine - Intrinsic_DDx + 1 ),
				"Missing Intrinsic!" );

			output.append( ddOperations[intrinsic - Intrinsic_DDx] );
			generate_function_call_parameters( node );
		}
		return;

	// Vector and Matrix Functions
		case Intrinsic_Length:
		case Intrinsic_Distance:
		case Intrinsic_Dot:
		case Intrinsic_Cross:
		case Intrinsic_Normalize:
		case Intrinsic_Reflect:
		case Intrinsic_Refract:
		case Intrinsic_Faceforward:
		case Intrinsic_Transpose:
		case Intrinsic_Determinant:
		case Intrinsic_Step:
		case Intrinsic_Smoothstep:
		{
			generate_function_call( node );
		}
		return;

		case Intrinsic_Mul:
		{
			output.append( "( ( " );
			generate_node( get_param( node->param, 0 )->expr );
			output.append( " ) * ( " );
			generate_node( get_param( node->param, 1 )->expr );
			output.append( " ) )" );
		}
		return;

		case Intrinsic_Lerp:
		{
			output.append( "mix" );
			generate_function_call_parameters( node );
		}
		return;

	// Bitwise Operations
		case Intrinsic_BitCount:
		case Intrinsic_BitFirstHigh:
		case Intrinsic_BitFirstLow:
		case Intrinsic_BitReverse:
		case Intrinsic_AtomicAdd:
		case Intrinsic_AtomicCompareExchange:
		case Intrinsic_AtomicExchange:
		case Intrinsic_AtomicMax:
		case Intrinsic_AtomicMin:
		case Intrinsic_AtomicAnd:
		case Intrinsic_AtomicOr:
		case Intrinsic_AtomicXor:
		{
			static const char *bitwiseOperations[] =
			{
				"bitCount",         // Intrinsic_BitCount
				"findMSB",          // Intrinsic_BitFirstHigh
				"findLSB",          // Intrinsic_BitFirstLow
				"bitfieldReverse",  // Intrinsic_BitReverse
				"atomicAdd",        // Intrinsic_AtomicAdd
				"atomicCompSwap",   // Intrinsic_AtomicCompareExchange
				"atomicExchange",   // Intrinsic_AtomicExchange
				"atomicMax",        // Intrinsic_AtomicMax
				"atomicMin",        // Intrinsic_AtomicMin
				"atomicAnd",        // Intrinsic_AtomicAnd
				"atomicOr",         // Intrinsic_AtomicOr
				"atomicXor",        // Intrinsic_AtomicXor
			};
			static_assert( ARRAY_LENGTH( bitwiseOperations ) == ( Intrinsic_AtomicXor - Intrinsic_BitCount + 1 ),
				"Missing Intrinsic!" );

			output.append( bitwiseOperations[intrinsic - Intrinsic_BitCount] );
			generate_function_call_parameters( node );
		}
		return;

	// Type Bit Conversions
		case Intrinsic_FloatToIntBits:
		case Intrinsic_FloatToUIntBits:
		case Intrinsic_IntToFloatBits:
		case Intrinsic_UIntToFloatBits:
		{
			static const char *intrinsics[] =
			{
				"floatBitsToInt",   // Intrinsic_FloatToIntBits
				"floatBitsToUint",  // Intrinsic_FloatToUIntBits
				"intBitsToFloat",   // Intrinsic_IntToFloatBits
				"uintBitsToFloat",  // Intrinsic_UIntToFloatBits
			};
			static_assert( ARRAY_LENGTH( intrinsics ) == ( Intrinsic_UIntToFloatBits - Intrinsic_FloatToIntBits + 1 ),
				"Missing Intrinsic!" );

			output.append( intrinsics[intrinsic - Intrinsic_FloatToIntBits] );
			generate_function_call_parameters( node );
		}
		return;

	// Texture Sampling Functions
		case Intrinsic_TextureSample1D:
		case Intrinsic_TextureSample1DArray:
		case Intrinsic_TextureSample2D:
		case Intrinsic_TextureSample2DArray:
		case Intrinsic_TextureSample3D:
		case Intrinsic_TextureSample3DArray:
		case Intrinsic_TextureSampleCube:
		case Intrinsic_TextureSampleCubeArray:
		{
			// texture( texture, location );
			output.append( "texture" );
			output.append( "( " );
			generate_node( get_param( node->param, 0 )->expr );
			output.append( ", " );
			generate_node_uv_flipped( get_param( node->param, 1 )->expr );
			output.append( " )" );
		}
		return;

		case Intrinsic_TextureSample1DLevel:
		case Intrinsic_TextureSampleCubeLevel:
		{
			// textureLod( texture, location, lod );
			output.append( "textureLod" );
			output.append( "( " );
			generate_node( get_param( node->param, 0 )->expr );
			output.append( ", " );
			generate_node( get_param( node->param, 1 )->expr );
			output.append( ", " );
			generate_node( get_param( node->param, 2 )->expr );
			output.append( " )" );
		}
		return;

		case Intrinsic_TextureSample2DLevel:
		{
			// textureLod( texture, location, lod );
			output.append( "textureLod" );
			output.append( "( " );
			generate_node( get_param( node->param, 0 )->expr );
			output.append( ", " );
			generate_node_uv_flipped( get_param( node->param, 1 )->expr ); // OpenGL: flip uv.v
			output.append( ", " );
			generate_node( get_param( node->param, 2 )->expr );
			output.append( " )" );
		}
		return;

		case Intrinsic_TextureSample3DLevel:
		{
			// textureLod( texture, location, lod );
			output.append( "textureLod" );
			output.append( "( " );
			generate_node( get_param( node->param, 0 )->expr );
			output.append( ", " );
			generate_node_uvw_flipped( get_param( node->param, 1 )->expr ); // OpenGL: flip uvw.v
			output.append( ", " );
			generate_node( get_param( node->param, 2 )->expr );
			output.append( " )" );
		}
		return;

		case Intrinsic_TextureLoad1D:
		{
			// texelFetch( sampler1D texture, int location, int lod );
			output.append( "texelFetch" );
			output.append( "( " );
			generate_node( get_param( node->param, 0 )->expr ); // texture
			output.append( ", " );
			generate_node( get_param( node->param, 1 )->expr ); // u
			output.append( ", " );
			generate_node( get_param( node->param, 3 )->expr ); // lod
			output.append( ")" );
		}
		return;

		case Intrinsic_TextureLoad2D:
		{
			// texelFetch( sampler2D texture, int_v2 location, int lod );
			output.append( "texelFetch" );
			output.append( "( " );
			generate_node( get_param( node->param, 0 )->expr ); // texture
			output.append( ", ivec2( " );
			generate_node( get_param( node->param, 1 )->expr ); // u
			output.append( ", int( " );
			generate_node( get_param( node->param, 4 )->expr ); // height
			output.append( " - 1 ) - ( " );
			generate_node( get_param( node->param, 2 )->expr ); // v
			output.append( " ) ), " );
			generate_node( get_param( node->param, 5 )->expr ); // lod
			output.append( ")" );
		}
		return;

		case Intrinsic_TextureLoad3D:
		{
			// texelFetch( sampler3D texture, int_v3 location, int lod );
			output.append( "texelFetch" );
			output.append( "( " );
			generate_node( get_param( node->param, 0 )->expr ); // texture
			output.append( ", ivec3( " );
			generate_node( get_param( node->param, 1 )->expr ); // u
			output.append( ", int( " );
			generate_node( get_param( node->param, 5 )->expr ); // height
			output.append( " - 1 ) - ( " );
			generate_node( get_param( node->param, 2 )->expr ); // v
			output.append( " ), " );
			generate_node( get_param( node->param, 3 )->expr ); // w
			output.append( " ), " );
			generate_node( get_param( node->param, 7 )->expr ); // lod
			output.append( ")" );
		}
		return;

	// Depth
		case Intrinsic_DepthNormalize:
		{
			// ( depth - near ) / ( far - near )
			output.append( "( " );
				output.append( "( " );
					generate_node_parenthesis( get_param( node->param, 0 )->expr ); // depth
					output.append( " - " );
					generate_node_parenthesis( get_param( node->param, 1 )->expr ); // near
				output.append( " )" );
				output.append( " / " );
				output.append( "( " );
					generate_node_parenthesis( get_param( node->param, 2 )->expr ); // far
					output.append( " - " );
					generate_node_parenthesis( get_param( node->param, 1 )->expr ); // near
				output.append( " )" );
			output.append( " )" );
		}
		return;

		case Intrinsic_DepthLinearize:
		{
			// ( ( ( near * far ) / ( far - depth * ( far - near ) ) ) - near ) / ( far - near )
			output.append( "( " );
				output.append( "( " );
					output.append( "( " );
						output.append( "( " );
							generate_node_parenthesis( get_param( node->param, 1 )->expr ); // near
							output.append( " * " );
							generate_node_parenthesis( get_param( node->param, 2 )->expr ); // far
						output.append( " )" );
						output.append( " / " );
						output.append( "( " );
							generate_node_parenthesis( get_param( node->param, 2 )->expr ); // far
							output.append( " - " );
							generate_node_parenthesis( get_param( node->param, 0 )->expr ); // depth
							output.append( " * " );
							output.append( "( " );
								generate_node_parenthesis( get_param( node->param, 2 )->expr ); // far
								output.append( " - " );
								generate_node_parenthesis( get_param( node->param, 1 )->expr ); // near
							output.append( " )" );
						output.append( " )" );
					output.append( " )" );
					output.append( " - " );
					generate_node_parenthesis( get_param( node->param, 1 )->expr ); // near
				output.append( " )" );
				output.append( " / " );
				output.append( "( " );
					generate_node_parenthesis( get_param( node->param, 2 )->expr ); // far
					output.append( " - " );
					generate_node_parenthesis( get_param( node->param, 1 )->expr ); // near
				output.append( " )" );
			output.append( " )" );
		}
		return;

		case Intrinsic_DepthUnproject:
		{
			output.append( "( " );
				output.append( "( " );
					generate_node_parenthesis( get_param( node->param, 0 )->expr );
					output.append( ".z / " );
					generate_node_parenthesis( get_param( node->param, 0 )->expr );
					output.append( ".w" );
				output.append( " )" );
				output.append( " * 0.5 + 0.5 " );
			output.append( " )" );
		}
		return;

		case Intrinsic_DepthUnprojectZW:
		{
			output.append( "( " );
				output.append( "( " );
					generate_node_parenthesis( get_param( node->param, 0 )->expr );
					output.append( " / " );
					generate_node_parenthesis( get_param( node->param, 1 )->expr );
				output.append( " )" );
				output.append( " * 0.5 + 0.5 " );
			output.append( " )" );
		}
		return;

	// Other
		default:
		{
			generate_function_call( node );
		}
		return;
	}
}


void GeneratorGLSL::generate_sv_semantic( NodeSVSemantic *node )
{
	switch( node->svSemanticType )
	{
		case SVSemanticType_VERTEX_ID: output.append( "gl_VertexID" ); return;
		case SVSemanticType_INSTANCE_ID: output.append( "gl_InstanceID" ); return;
		case SVSemanticType_PRIMITIVE_ID: output.append( "gl_PrimitiveID" ); return;
		case SVSemanticType_SAMPLE_ID: output.append( "gl_SampleID" ); return;
		case SVSemanticType_FRONT_FACING: output.append( "gl_FrontFacing" ); return;
		case SVSemanticType_DISPATCH_THREAD_ID: output.append( "gl_GlobalInvocationID" ); return;
		case SVSemanticType_GROUP_THREAD_ID: output.append( "gl_LocalInvocationID" ); return;
		case SVSemanticType_GROUP_ID:  output.append( "gl_WorkGroupID" ); return;
		case SVSemanticType_GROUP_INDEX: output.append( "gl_LocalInvocationIndex" ); return;
		default: Error( "Unexpected SV Semantic: %s", SVSemantics[node->svSemanticType] );
	}
}


void GeneratorGLSL::generate_expression_binary( NodeExpressionBinary *node )
{
	static const char *binaryOperators[] =
	{
		".",     // ExpressionBinaryType_Dot
		"",      // ExpressionBinaryType_Subscript
		" = ",   // ExpressionBinaryType_Assign
		" + ",   // ExpressionBinaryType_Add
		" += ",  // ExpressionBinaryType_AddAssign
		" - ",   // ExpressionBinaryType_Sub
		" -= ",  // ExpressionBinaryType_SubAssign
		" * ",   // ExpressionBinaryType_Mul
		" *= ",  // ExpressionBinaryType_MulAssign
		" / ",   // ExpressionBinaryType_Div
		" /= ",  // ExpressionBinaryType_DivAssign
		" % ",   // ExpressionBinaryType_Mod
		" %= ",  // ExpressionBinaryType_ModAssign
		" & ",   // ExpressionBinaryType_BitAnd
		" &= ",  // ExpressionBinaryType_BitAndAssign
		" | ",   // ExpressionBinaryType_BitOr
		" |= ",  // ExpressionBinaryType_BitOrAssign
		" ^ ",   // ExpressionBinaryType_BitXor
		" ^= ",  // ExpressionBinaryType_BitXorAssign
		" << ",  // ExpressionBinaryType_BitShiftLeft
		" <<= ", // ExpressionBinaryType_BitShiftLeftAssign
		" >> ",  // ExpressionBinaryType_BitShiftRight
		" >>= ", // ExpressionBinaryType_BitShiftRightAssign
		" == ",  // ExpressionBinaryType_Equals
		" != ",  // ExpressionBinaryType_NotEquals
		" && ",  // ExpressionBinaryType_And
		" || ",  // ExpressionBinaryType_Or
		" > ",   // ExpressionBinaryType_Greater
		" >= ",  // ExpressionBinaryType_GreaterEquals
		" < ",   // ExpressionBinaryType_Less
		" <= ",  // ExpressionBinaryType_LessEquals
	};
	static_assert( ARRAY_LENGTH( binaryOperators ) == EXPRESSIONBINARYTYPE_COUNT,
		"Missing ExpressionBinaryType!" );

	switch( node->exprType )
	{
		case ExpressionBinaryType_Subscript:
		{
			generate_node( node->expr1 );
			output.append( "[" );
			generate_node( node->expr2 );
			output.append( "]" );
		}
		break;

		case ExpressionBinaryType_Dot:
		{
			// buffer members are in the global namespace, so we prefix them by structure name
			if( node->expr1->nodeType == NodeType_Variable )
			{
				NodeVariable *nodeVariable = reinterpret_cast<NodeVariable *>( node->expr1 );
				Variable &variable = parser.variables[nodeVariable->variableID];
				Type &type = parser.types[variable.typeID];

				if( type.global )
				{
					if( type.pipelineVarying )
					{
						output.append( "varying_" );
						generate_node( node->expr2 );
						break;
					}
					else
					{
						String &typeName = type_name( variable.typeID );
						output.append( typeName ).append( "_" );
						generate_node( node->expr2 );
						break;
					}
				}
			}

			// Only reachable if LHS not uniform_buffer type
			generate_node( node->expr1 );
			output.append( "." );
			generate_node( node->expr2 );
		}
		break;

		default:
		{
			generate_node( node->expr1 );
			output.append( binaryOperators[node->exprType] );
			generate_node( node->expr2 );
		}
		break;
	}
}


void GeneratorGLSL::generate_structure( NodeStruct *node )
{
	generate_structure_gfx( node );

	// Skip generating StructType_InstanceInput
	if( node->structType == StructType_InstanceInput ) { return; }

	// layout( location = ... )
	int location = 0;

	struct StructInfo { bool hasBody, hasIn, hasOut, hasLayout; };
	auto get_struct_info = [&]( StructType structType ) -> StructInfo
	{
		StructInfo out;
		switch( node->structType )
		{
			default:
			case StructType_Struct: { out.hasBody = true;  out.hasIn = false; out.hasOut = false; out.hasLayout = false; } break;
			case StructType_SharedStruct: { out.hasBody = true;  out.hasIn = false; out.hasOut = false; out.hasLayout = false; } break;
			case StructType_UniformBuffer: { out.hasBody = true;  out.hasIn = false; out.hasOut = false; out.hasLayout = false; } break;
			case StructType_ConstantBuffer: { out.hasBody = true;  out.hasIn = false; out.hasOut = false; out.hasLayout = false; } break;
			case StructType_MutableBuffer: { out.hasBody = true;  out.hasIn = false; out.hasOut = false; out.hasLayout = false; } break;
			case StructType_InstanceInput: { out.hasBody = false; out.hasIn = true;  out.hasOut = false; out.hasLayout = true; } break;
			case StructType_VertexInput: { out.hasBody = false; out.hasIn = true;  out.hasOut = false; out.hasLayout = true; } break;
			case StructType_VertexOutput: { out.hasBody = false; out.hasIn = false; out.hasOut = true;  out.hasLayout = true; } break;
			case StructType_FragmentInput: { out.hasBody = false; out.hasIn = true;  out.hasOut = false; out.hasLayout = true; } break;
			case StructType_FragmentOutput: { out.hasBody = false; out.hasIn = false; out.hasOut = true;  out.hasLayout = true; } break;
	}
		return out;
	};

	auto append_structure_members = [&]( NodeStruct *node ) -> void
	{
		const Struct &structure = parser.structs[node->structID];
		const Type &type = parser.types[structure.typeID];
		const String &typeName = type_name( structure.typeID );
		const VariableID first = type.memberFirst;
		const VariableID last = first + type.memberCount;
		const StructInfo info = get_struct_info( node->structType );;

		const bool expectMeta = !( node->structType == StructType_UniformBuffer ||
			node->structType == StructType_ConstantBuffer || node->structType == StructType_MutableBuffer ||
			node->structType == StructType_Struct || node->structType == StructType_SharedStruct );

		for( usize i = first; i < last; i++ )
		{
			Variable &memberVariable = parser.variables[i];
			Type &memberVariableType = parser.types[memberVariable.typeID];
			const String &memberVariableName = variable_name( i );
			const String &memberTypeName = type_name( memberVariable.typeID );
			output.append( indent );

			// GLSL Restrictions
			switch( node->structType )
			{
				// vertex_output
				case StructType_VertexOutput:
				{
					if( memberVariable.semantic == SemanticType_POSITION )
					{
						String varying = String( "varying_" ).append( memberVariableName );
						output.append( "#define " ).append( varying ).append( " gl_Position\n" );
						location++;
						continue;
					}
				}
				break;

				// fragment_input
				case StructType_FragmentInput:
				{
					if( memberVariable.semantic == SemanticType_POSITION )
					{
						String replace = String( "varying_" ).append( memberVariableName );
						output.append( "#define " ).append( replace ).append( " gl_FragCoord\n" );
						location++;
						continue;
					}
				}
				break;

				// fragment_output
				case StructType_FragmentOutput:
				{
					if( memberVariable.semantic == SemanticType_DEPTH )
					{
						String varying = String( typeName ).append( "_" ).append( memberVariableName );
						output.append( "#define " ).append( varying ).append( " gl_FragDepth\n" );
						continue;
					}
				}
				break;
			}

			// <layout>
			if( info.hasLayout )
			{
				int slot = memberVariable.slot != -1 ? memberVariable.slot : location;
				output.append( "layout( location = " ).append( slot ).append( " ) " );
			}

			// Flat
			if( node->structType == StructType_VertexOutput ||
				node->structType == StructType_FragmentInput )
			{
				switch( memberVariable.typeID )
				{
					case Primitive_Bool:
					case Primitive_Bool2:
					case Primitive_Bool3:
					case Primitive_Bool4:
					case Primitive_Int:
					case Primitive_Int2:
					case Primitive_Int3:
					case Primitive_Int4:
					case Primitive_UInt:
					case Primitive_UInt2:
					case Primitive_UInt3:
					case Primitive_UInt4:
						output.append( "flat " );
					break;
				}
			}

			// <in> / <out>
			if( info.hasIn ) { output.append( "in " ); }
			if( info.hasOut ) { output.append( "out " ); }

			// <type>
			output.append( memberTypeName ).append( " " );

			// <name>
			if( node->structType == StructType_UniformBuffer ||
				node->structType == StructType_ConstantBuffer ||
				node->structType == StructType_MutableBuffer || !info.hasBody )
			{
				if( node->structType == StructType_VertexOutput || node->structType == StructType_FragmentInput )
				{
					// Vertex output and fragment input names must match
					// Note: This shouldn't be the case with OpenGL 4.1 and explicit layouts,
					//       but there is a bug with MacOS OpenGL compiler.
					output.append( "varying_" );
				}
				else
				{
					// uniform_buffer, constant_buffer, mutable_buffer, vertex_input, and fragment_output
					// are in global namespace, so we prefix them with the structure name
					output.append( typeName ).append( "_" );
				}
			}

			output.append( memberVariableName );

			// [arrayX]
			if( memberVariable.arrayLengthX > 0 )
			{
				output.append( "[" ).append( memberVariable.arrayLengthX ).append( "]");
			}

			// [arrayY]
			if( memberVariable.arrayLengthY > 0 )
			{
				output.append( "[" ).append( memberVariable.arrayLengthY ).append( "]");
			}

			output.append( ";\n" );

			location++;
		}
	};

	Struct &structure = parser.structs[node->structID];
	const StructInfo info = get_struct_info( node->structType );;

	static const char *structNames[] =
	{
		"struct",                   // StructType_Struct
		"struct",                   // StructType_SharedStruct
		"layout( std140 ) uniform", // StructType_UniformBuffer
		"layout( std140 ) uniform", // StructType_ConstantBuffer
		"layout( std140 ) uniform", // StructType_MutuableBuffer
		"",                         // StructType_InstanceInput
		"",                         // StructType_VertexInput
		"",                         // StructType_VertexOutput
		"",                         // StructType_FragmentInput
		"",                         // StructType_FragmentOutput
	};
	static_assert( ARRAY_LENGTH( structNames ) == STRUCTTYPE_COUNT,
		"Missing TextureType" );

	// {
	if( info.hasBody )
	{
		// <name> <type>
		output.append( structNames[node->structType] ).append( " " );
		output.append( type_name( structure.typeID ) );
		output.append( "\n{\n" );
		indent_add();
	}

	// Members
	append_structure_members( node );

	// In GLSL, instance_input members belong to the vertex format
	if( node->structType == StructType_VertexInput && shader.instanceFormatID != U32_MAX )
	{
		const InstanceFormat &instanceFormat = Gfx::instanceFormats[shader.instanceFormatID];
		if( instanceFormat.node != nullptr )
		{
			NodeStruct *nodeInstanceInput = reinterpret_cast<NodeStruct *>( instanceFormat.node );
			append_structure_members( nodeInstanceInput );
		}
	}

	// }
	if( info.hasBody )
	{
		indent_sub();
		output.append( "};\n" );
	}

	output.append( "\n" );
}


bool GeneratorGLSL::generate_structure_gfx_vertex( NodeStruct *node )
{
	// Register this vertex format in the system
	if( !Generator::generate_structure_gfx_vertex( node ) )
	{
		return false; // Already registered -- no need to continue
	}

	// Generate OpenGL Vertex Format interfaces
	const Struct &structure = parser.structs[node->structID];
	const Type &type = parser.types[structure.typeID];
	const VariableID first = type.memberFirst;
	const VariableID last = first + type.memberCount;
	const String &structureName = type_name( structure.typeID );

	int byteOffset = 0;
	String attributes;

	const usize count = last - first;
	if( count > 0 )
	{
		attributes.append( "constexpr OpenGLInputLayoutAttributes openglInputAttributesVertex_" );
		attributes.append( type.name ).append( "[" ).append( count ).append("]  =\n{\n" );

		for( usize i = first; i < last; i++ )
		{
			const Variable &memberVariable = parser.variables[i];
			Assert( memberVariable.format < INPUTFORMAT_COUNT );
			const OpenGLInputFormatInfo &inputFormat = OPENGL_INPUT_FORMATS[memberVariable.format];
			const int components = input_format_components( memberVariable.typeID );
			Assert( components != 0 );

			Assert( memberVariable.semantic < SEMANTICTYPE_COUNT );
			attributes.append( "\t" "{ \"" );
			attributes.append( structureName ).append( "_" ).append( memberVariable.name ).append( "\", " );
			attributes.append( input_attibute_type( memberVariable.typeID ) ).append( ", " );
			attributes.append( components ).append( ", " );
			attributes.append( inputFormat.format ).append( ", " );
			attributes.append( inputFormat.normalized ? "true" : "false" ).append( ", " );
			attributes.append( byteOffset ).append( " },\n" );
			byteOffset += inputFormat.size * components;
		}
		attributes.append( "};\n\n" );
	}
	else
	{
		attributes.append( "constexpr OpenGLInputLayoutAttributes *openglInputAttributesVertex_" );
		attributes.append( type.name ).append( " = nullptr;\n\n" );
	}

	attributes.append( "constexpr OpenGLInputLayoutFormats inputLayoutFormatVertex_" );
	attributes.append( type.name ).append( " =\n{\n\topenglInputAttributesVertex_" );
	attributes.append( type.name ).append( ", " ).append( count ).append( ", " ).append( byteOffset );
	attributes.append( "\n};\n\n" );

	// Write to gfx.api.generated.hpp
	shader.header.append( attributes );
	return true;
}


bool GeneratorGLSL::generate_structure_gfx_instance( NodeStruct *node )
{
	// Register this instance format in the system
	if( !Generator::generate_structure_gfx_instance( node ) )
	{
		return false; // Already registered -- no need to continue
	}

	// Generate OpenGL Instance Format interfaces
	const Struct &structure = parser.structs[node->structID];
	const Type &type = parser.types[structure.typeID];
	const VariableID first = type.memberFirst;
	const VariableID last = first + type.memberCount;
	const String &structureName = type_name( structure.typeID );

	int byteOffset = 0;
	String attributes;

	const usize count = last - first;
	if( count > 0 )
	{
		attributes.append( "constexpr OpenGLInputLayoutAttributes openglInputAttributesInstance_" );
		attributes.append( type.name ).append( "[" ).append( count ).append("]  =\n{\n" );

		for( usize i = first; i < last; i++ )
		{
			const Variable &memberVariable = parser.variables[i];
			Assert( memberVariable.format < INPUTFORMAT_COUNT );
			const OpenGLInputFormatInfo &inputFormat = OPENGL_INPUT_FORMATS[memberVariable.format];
			const int components = input_format_components( memberVariable.typeID );
			Assert( components != 0 );

			Assert( memberVariable.semantic < SEMANTICTYPE_COUNT );
			attributes.append( "\t" "{ \"" );
			attributes.append( structureName ).append( "_" ).append( memberVariable.name ).append( "\", " );
			attributes.append( input_attibute_type( memberVariable.typeID ) ).append( ", " );
			attributes.append( components ).append( ", " );
			attributes.append( inputFormat.format ).append( ", " );
			attributes.append( inputFormat.normalized ? "true" : "false" ).append( ", " );
			attributes.append( byteOffset ).append( " },\n" );
			byteOffset += inputFormat.size * components;
		}
		attributes.append( "};\n\n" );
	}
	else
	{
		attributes.append( "constexpr OpenGLInputLayoutAttributes *openglInputAttributesInstance_" );
		attributes.append( type.name ).append( " = nullptr;\n\n" );
	}

	attributes.append( "constexpr OpenGLInputLayoutFormats inputLayoutFormatInstance_" );
	attributes.append( type.name ).append( " =\n{\n\topenglInputAttributesInstance_" );
	attributes.append( type.name ).append( ", " ).append( count ).append( ", " ).append( byteOffset );
	attributes.append( "\n};\n\n" );

	// Write to gfx.api.generated.hpp
	shader.header.append( attributes );
	return true;
}


void GeneratorGLSL::generate_texture( NodeTexture *node )
{
	// TODO: Multiple sampler states for each texture (like OpenGL)
	static bool generatedSampler = false;
	Texture &texture = parser.textures[node->textureID];
	const String &samplerName = type_name( parser.variables[texture.variableID].texture );
	const String &variableName = variable_name( texture.variableID );

	const char *samplerPrefix;

	switch( parser.variables[texture.variableID].typeID )
	{
		case Primitive_Int:
		case Primitive_Int2:
		case Primitive_Int3:
		case Primitive_Int4:
			samplerPrefix = "i";
		break;

		case Primitive_UInt:
		case Primitive_UInt2:
		case Primitive_UInt3:
		case Primitive_UInt4:
			samplerPrefix = "u";
		break;

		default:
			samplerPrefix = "";
		break;
	}

	// Texture
	output.append( indent ).append( "uniform " ).append( samplerPrefix );
	output.append( samplerName ).append( " " ).append( variableName ).append( ";\n\n" );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}