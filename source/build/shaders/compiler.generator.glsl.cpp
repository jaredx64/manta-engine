#include <build/shaders/compiler.generator.hpp>

#include <core/string.hpp>

#include <vendor/stdio.hpp>

namespace ShaderCompiler
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct SemanticSubsituation
{
	SemanticSubsituation( const char *string, const char *substitute ) :
		string{ string }, substitute{ substitute } { }

	String string;
	String substitute;
};


static List<SemanticSubsituation> SemanticSubstitutions;


static void substitute_semantics( String &output )
{
	for( SemanticSubsituation &sub : SemanticSubstitutions )
	{
		output.replace( sub.string.cstr(), sub.substitute.cstr() );
	}

	SemanticSubstitutions.clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
	"double",           // Primitive_Double
	"dvec2",            // Primitive_Double2
	"dvec3",            // Primitive_Double3
	"dvec4",            // Primitive_Double4
	"dmat2",            // Primitive_Double2x2
	"dmat3",            // Primitive_Double3x3
	"dmat4",            // Primitive_Double4x4
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

	// Super
	Generator::generate_stage( stage );

	// Replace Semantics
	substitute_semantics( output );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GeneratorGLSL::generate_function_declaration( NodeFunctionDeclaration *node )
{
	switch( node->functionType )
	{
		case FunctionType_MainVertex:
		case FunctionType_MainFragment:
		case FunctionType_MainCompute:
			generate_function_declaration_main( node );
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


void GeneratorGLSL::generate_function_declaration_main( NodeFunctionDeclaration *node )
{
	Function &function = parser.functions[node->functionID];
	Type &returnType = parser.types[function.typeID];

	// Return Type & Name
	output.append( indent ).append( "void main" ).append( "()\n" );
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

			output.append( "dFdyFine" );
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
		case SVSemanticType_DISPATCH_THREAD_ID: output.append( "gl_GlobalInvocationID" ); return;
		case SVSemanticType_GROUP_ID:  output.append( "gl_WorkGroupID" ); return;
		case SVSemanticType_GROUP_THREAD_ID: output.append( "gl_LocalInvocationID" ); return;
		case SVSemanticType_GROUP_INDEX: output.append( "gl_LocalInvocationIndex" ); return;
		case SVSemanticType_VERTEX_ID: output.append( "gl_VertexID" ); return;
		case SVSemanticType_INSTANCE_ID: output.append( "gl_InstanceID" ); return;
		case SVSemanticType_PRIMITIVE_ID: output.append( "gl_PrimitiveID" ); return;
		case SVSemanticType_FRONT_FACING: output.append( "gl_FrontFacing" ); return;
		case SVSemanticType_SAMPLE_ID: output.append( "gl_SampleID" ); return;

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
					if( type.pipelineIntermediate )
					{
						output.append( "pipelineIntermediate_" );
					}
					else
					{
						String &typeName = type_name( variable.typeID );
						output.append( typeName ).append( "_" );
					}

					generate_node( node->expr2 );
					break;
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
	Struct &structure = parser.structs[node->structID];
	Type &type = parser.types[structure.typeID];
	const String &typeName = type_name( structure.typeID );
	const VariableID first = type.memberFirst;
	const VariableID last = first + type.memberCount;

	static const char *structNames[] =
	{
		"struct",                   // StructType_Struct
		"struct",                   // StructType_SharedStruct
		"layout( std140 ) uniform", // StructType_UniformBuffer
		"layout( std140 ) uniform", // StructType_ConstantBuffer
		"layout( std140 ) uniform", // StructType_MutuableBuffer
		"",                         // StructType_VertexInput
		"",                         // StructType_VertexOutput
		"",                         // StructType_FragmentInput
		"",                         // StructType_FragmentOutput
		"",                         // StructType_ComputeInput
		"",                         // StructType_ComputeOutput
	};
	static_assert( ARRAY_LENGTH( structNames ) == STRUCTTYPE_COUNT,
		"Missing TextureType" );

	const bool expectSlot =  ( node->structType == StructType_UniformBuffer ||
		node->structType == StructType_ConstantBuffer || node->structType == StructType_MutableBuffer );

	const bool expectMeta = !( node->structType == StructType_UniformBuffer ||
		node->structType == StructType_ConstantBuffer || node->structType == StructType_MutableBuffer ||
		node->structType == StructType_Struct || node->structType == StructType_SharedStruct );

	bool hasBody, hasIn, hasOut, hasLayout;
	switch( node->structType )
	{
		default:
		case StructType_Struct:         { hasBody = true;  hasIn = false; hasOut = false; hasLayout = false; } break;
		case StructType_SharedStruct:   { hasBody = true;  hasIn = false; hasOut = false; hasLayout = false; } break;
		case StructType_UniformBuffer:  { hasBody = true;  hasIn = false; hasOut = false; hasLayout = false; } break;
		case StructType_ConstantBuffer: { hasBody = true;  hasIn = false; hasOut = false; hasLayout = false; } break;
		case StructType_MutableBuffer:  { hasBody = true;  hasIn = false; hasOut = false; hasLayout = false; } break;
		case StructType_VertexInput:    { hasBody = false; hasIn = true;  hasOut = false; hasLayout = true;  } break;
		case StructType_VertexOutput:   { hasBody = false; hasIn = false; hasOut = true;  hasLayout = true;  } break;
		case StructType_FragmentInput:  { hasBody = false; hasIn = true;  hasOut = false; hasLayout = true;  } break;
		case StructType_FragmentOutput: { hasBody = false; hasIn = false; hasOut = true;  hasLayout = true;  } break;
		case StructType_ComputeInput:   { hasBody = false; hasIn = true;  hasOut = false; hasLayout = false; } break;
		case StructType_ComputeOutput:  { hasBody = false; hasIn = false; hasOut = true;  hasLayout = false; } break;
	}

	// { <body> }
	if( hasBody )
	{
		// <name> <type>
		output.append( structNames[node->structType] ).append( " " ).append( typeName );
		output.append( "\n{\n" );
		indent_add();
	}

	for( usize i = first, location = 0; i < last; i++, location++ )
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
				switch( memberVariable.semantic )
				{
					case SemanticType_POSITION:
					{
						output.append( "// " );
						//String replace = String( typeName ).append( "_" ).append( memberVariableName );
						String replace = String( "pipelineIntermediate_" ).append( memberVariableName );
						SemanticSubstitutions.add( SemanticSubsituation( replace.cstr(), "gl_Position" ) );
					}
					break;
				}
			}
			break;

			// fragment_input
			case StructType_FragmentInput:
			{
				switch( memberVariable.semantic )
				{
					case SemanticType_POSITION:
					{
						output.append( "// " );
						//String replace = String( typeName ).append( "_" ).append( memberVariableName );
						String replace = String( "pipelineIntermediate_" ).append( memberVariableName );
						SemanticSubstitutions.add( SemanticSubsituation( replace.cstr(), "gl_FragCoord" ) );
					}
					break;
				}
			}
			break;

			// fragment_output
			case StructType_FragmentOutput:
			{
				switch( memberVariable.semantic )
				{
					case SemanticType_DEPTH:
					{
						output.append( "// " );
						String replace = String( typeName ).append( "_" ).append( memberVariableName );
						SemanticSubstitutions.add( SemanticSubsituation( replace.cstr(), "gl_FragDepth" ) );
					}
					break;
				}
			}
			break;
		}

		// <layout>
		if( hasLayout )
		{
			int slot = memberVariable.slot != -1 ? memberVariable.slot : static_cast<int>( location );
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
		if( hasIn ) { output.append( "in " ); }
		if( hasOut ) { output.append( "out " ); }

		// <type>
		output.append( memberTypeName ).append( " " );

		// <name>
		if( node->structType == StructType_UniformBuffer ||
			node->structType == StructType_ConstantBuffer ||
			node->structType == StructType_MutableBuffer || !hasBody )
		{
			if( node->structType == StructType_VertexOutput || node->structType == StructType_FragmentInput )
			{
				// Vertex output and fragment input names must match
				// Note: This shouldn't be the case with OpenGL 4.1 and explicit layouts,
				//       but there is a bug with MacOS OpenGL compiler.
				output.append( "pipelineIntermediate_" );
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
	}

	if( hasBody )
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

	// Generate OpenGL/GLSL Vertex Format interfaces
	const Struct &structure = parser.structs[node->structID];
	const Type &type = parser.types[structure.typeID];
	const VariableID first = type.memberFirst;
	const VariableID last = first + type.memberCount;
	const String &structureName = type_name( structure.typeID );

	int byteOffset = 0;

	String link;
	String bind;

	for( usize i = first; i < last; i++ )
	{
		Variable &memberVariable = parser.variables[i];
		const String &memberVariableName = variable_name( i );

		// opengl_vertex_input_layout_init
		{
			link.append( "\tnglBindAttribLocation( program, " );
			link.append( static_cast<int>( i - first ) ).append( ", \"" );
			link.append( structureName ).append( "_" ).append( memberVariableName ).append( "\" );\n" );
		}

		// opengl_verteX_input_layout_bind
		{
			// Get glVertexAttribPointer function
			const char *glVertexAttribFunc = "";
			bool hasNormFlag = false;
			switch( memberVariable.typeID )
			{
				case Primitive_Int:
				case Primitive_UInt:
				case Primitive_Int2:
				case Primitive_UInt2:
				case Primitive_Int3:
				case Primitive_UInt3:
				case Primitive_Int4:
				case Primitive_UInt4:
					glVertexAttribFunc = "nglVertexAttribIPointer";
				break;

				case Primitive_Double:
				case Primitive_Double2:
				case Primitive_Double3:
				case Primitive_Double4:
					glVertexAttribFunc = "nglVertexAttribLPointer";
				break;

				default:
					glVertexAttribFunc = "nglVertexAttribPointer";
					hasNormFlag = true;
				break;
			}

			// Get type dimensions (vector length)
			int dimensions = 0;
			switch( memberVariable.typeID )
			{
				case Primitive_Bool:
				case Primitive_Int:
				case Primitive_UInt:
				case Primitive_Float:
				case Primitive_Double:
					dimensions = 1;
				break;

				case Primitive_Bool2:
				case Primitive_Int2:
				case Primitive_UInt2:
				case Primitive_Float2:
				case Primitive_Double2:
					dimensions = 2;
				break;

				case Primitive_Bool3:
				case Primitive_Int3:
				case Primitive_UInt3:
				case Primitive_Float3:
				case Primitive_Double3:
					dimensions = 3;
				break;

				case Primitive_Bool4:
				case Primitive_Int4:
				case Primitive_UInt4:
				case Primitive_Float4:
				case Primitive_Double4:
					dimensions = 4;
				break;

				default:
					Error( "Unexpected vertex input type: %llu (%s)",
						memberVariable.typeID, memberVariableName.cstr() );
				break;
			}

			// Get type format
			struct FormatInfo
			{
				FormatInfo() = default;
				FormatInfo( const char *type, bool normalized, int size, int stride = 0 ) :
					type{ type }, normalized{ normalized }, size{ size }, stride{ stride } { }

				const char *type = "";
				bool normalized = false;
				int size = 1;
				int stride = 0;
			};

			FormatInfo format;
			switch( memberVariable.format )
			{
				// 1 Byte
				case InputFormat_UNORM8:
					//format = FormatInfo( "GL_UNSIGNED_BYTE", true, 1, 0 );
					format = FormatInfo( "GL_UNSIGNED_BYTE", true, 1, 0 );
				break;

				case InputFormat_SNORM8:
					format = FormatInfo( "GL_BYTE", true, 1, 0 );
				break;

				case InputFormat_UINT8:
					format = FormatInfo( "GL_UNSIGNED_BYTE", false, 1, 0 );
				break;

				case InputFormat_SINT8:
					format = FormatInfo( "GL_BYTE", false, 1, 0 );
				break;

				// 2 Bytes
				case InputFormat_UNORM16:
					format = FormatInfo( "GL_UNSIGNED_SHORT", true, 2, 0 );
				break;

				case InputFormat_SNORM16:
					format = FormatInfo( "GL_SHORT", true, 2, 0 );
				break;

				case InputFormat_UINT16:
					format = FormatInfo( "GL_UNSIGNED_SHORT", false, 2, 0 );
				break;

				case InputFormat_SINT16:
					format = FormatInfo( "GL_SHORT", false, 2, 0 );
				break;

				case InputFormat_FLOAT16:
					format = FormatInfo( "GL_HALF_FLOAT", false, 2, 0 );
				break;

				// 4 Bytes
				case InputFormat_UNORM32:
					format = FormatInfo( "GL_UNSIGNED_INT", true, 4, 0 );
				break;

				case InputFormat_SNORM32:
					format = FormatInfo( "GL_INT", true, 4, 0 );
				break;

				case InputFormat_UINT32:
					format = FormatInfo( "GL_UNSIGNED_INT", false, 4, 0 );
				break;

				case InputFormat_SINT32:
					format = FormatInfo( "GL_INT", false, 4, 0 );
				break;

				case InputFormat_FLOAT32:
					format = FormatInfo( "GL_FLOAT", false, 4, 0 );
				break;
			}

			bind.append( "\t" ).append( glVertexAttribFunc ).append( "( " );
			bind.append( static_cast<int>( i - first ) ).append( ", " );
			bind.append( dimensions ).append( ", " ).append( format.type ).append( ", " );
			if( hasNormFlag ) { bind.append( format.normalized ? "true" : "false" ).append( ", " ); }
			bind.append( "sizeof( GfxVertex::" ).append( type.name ).append( " ), " );
			bind.append( "reinterpret_cast<void *>( " ).append( byteOffset ).append( " ) );\n" );
			byteOffset += format.size * dimensions;

			bind.append( "\tnglEnableVertexAttribArray( " );
			bind.append( static_cast<int>( i - first ) ).append( " );\n" );
		}
	}

	// Write To gfx.api.generated.cpp
	shader.source.append( "static void opengl_vertex_input_layout_init_" );
	shader.source.append( type.name ).append( "( GLuint program )\n" );
	shader.source.append( "{\n" );
	shader.source.append( link );
	shader.source.append( "}\n\n" );

	shader.source.append( "static void opengl_vertex_input_layout_bind_" );
	shader.source.append( type.name ).append( "()\n" );
	shader.source.append( "{\n" );
	shader.source.append( bind );
	shader.source.append( "}\n\n" );

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