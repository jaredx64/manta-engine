#include <build/shaders/compiler.generator.hpp>

#include <vendor/stdio.hpp>
#include <core/string.hpp>

#include <build/gfx.hpp>

namespace ShaderCompiler
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static u32 SemanticTypeCount[SEMANTICTYPE_COUNT];

static bool generatedSampler = false;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void semantics_reset()
{
	for( u32 i = 0; i < SEMANTICTYPE_COUNT; i++ )
	{
		SemanticTypeCount[i] = 0;
	}
}


static void semantics_get_name( char *buffer, const usize size, StructType structType, Variable &variable )
{
	const char *name = "";
	Assert( variable.semantic < SEMANTICTYPE_COUNT );

	switch( structType )
	{
		case StructType_VertexInput:
		{
			switch( variable.semantic )
			{
				case SemanticType_POSITION: snprintf( buffer, size, "POSITION" ); return;
				case SemanticType_TEXCOORD: name = "TEXCOORD"; break;
				case SemanticType_NORMAL:   name = "NORMAL";   break;
				case SemanticType_DEPTH:    Error( "HLSL: vertex_input: unsupported semantic DEPTH" );
				case SemanticType_COLOR:    name = "COLOR";    break;
			}
		}
		break;

		case StructType_VertexOutput:
		{
			switch( variable.semantic )
			{
				case SemanticType_POSITION: snprintf( buffer, size, "SV_POSITION" ); return;
				case SemanticType_TEXCOORD: name = "TEXCOORD"; break;
				case SemanticType_NORMAL:   name = "NORMAL";   break;
				case SemanticType_DEPTH:    Error( "HLSL: vertex_output: unsupported semantic DEPTH" );
				case SemanticType_COLOR:    name = "COLOR";    break;
			}
		}
		break;

		case StructType_FragmentInput:
		{
			switch( variable.semantic )
			{
				case SemanticType_POSITION: snprintf( buffer, size, "SV_POSITION" ); return;
				case SemanticType_TEXCOORD: name = "TEXCOORD"; break;
				case SemanticType_NORMAL:   name = "NORMAL";   break;
				case SemanticType_DEPTH:    Error( "HLSL: fragment_input: unsupported semantic DEPTH" );
				case SemanticType_COLOR:    name = "COLOR";    break;
			}
		}
		break;

		case StructType_FragmentOutput:
		{
			switch( variable.semantic )
			{
				case SemanticType_POSITION: Error( "HLSL: fragment_output: unsupported semantic POSITION" );
				case SemanticType_TEXCOORD: Error( "HLSL: fragment_output: unsupported semantic TEXCOORD" );
				case SemanticType_NORMAL:   Error( "HLSL: fragment_output: unsupported semantic NORMAL" );
				case SemanticType_DEPTH:    snprintf( buffer, size, "SV_DEPTH" ); return;
				case SemanticType_COLOR:    name = "SV_TARGET"; break;
			}
		}
		break;

		default: AssertMsg( false, "StructType %u cannot have semantics!", structType ); return;
	}

	// Append Count
	const u32 slot = variable.slot < 0 ? SemanticTypeCount[variable.semantic] : static_cast<u32>( variable.slot );
	snprintf( buffer, size, "%s%u", name, slot );
	SemanticTypeCount[variable.semantic]++;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GeneratorHLSL::generate_stage( ShaderStage stage )
{
	generatedSampler = false;

	// Super
	Generator::generate_stage( stage );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GeneratorHLSL::generate_function_declaration( NodeFunctionDeclaration *node )
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

			// Skip CBuffers (global namespace)
			if( parameterType.tokenType == TokenType_CBuffer ) { continue; }

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


void GeneratorHLSL::generate_function_declaration_main( NodeFunctionDeclaration *node )
{
	Function &function = parser.functions[node->functionID];
	Type &returnType = parser.types[function.typeID];

	// In / Out
	Assert( function.parameterCount >= 2 );
	VariableID inID = function.parameterFirst;
	Type &inType = parser.types[parser.variables[inID].typeID];
	const String &inTypeName = type_name( parser.variables[inID].typeID );

	VariableID outID = function.parameterFirst + 1;
	Type &outType = parser.types[parser.variables[outID].typeID];
	const String &outTypeName = type_name( parser.variables[outID].typeID );

	const char *mainName = "";
	switch( node->functionType )
	{
		case FunctionType_MainVertex: mainName = "vs_main"; break;
		case FunctionType_MainFragment: mainName = "ps_main"; break;
		case FunctionType_MainCompute: mainName = "cs_main"; break;
	}

	// Return Type & Name
	output.append( indent ).append( "void " ).append( mainName ).append( "( " );
	output.append( "in " ).append( inTypeName ).append( " " ).append( variable_name( inID ) ).append( ", " );
	output.append( "out " ).append( outTypeName ).append( " " ).append( variable_name( outID ) ).append( " )\n" );

	generate_statement_block( reinterpret_cast<NodeStatementBlock *>( node->block ) );
	output.append( "\n" );
}


void GeneratorHLSL::generate_expression_binary_dot( NodeExpressionBinary *node )
{
	Assert( node->exprType == ExpressionBinaryType_Dot );

	// CBuffer members are in the global namespace, so we prefix them by structure name
	if( node->expr1->nodeType == NodeType_Variable )
	{
		NodeVariable *nodeVariable = reinterpret_cast<NodeVariable *>( node->expr1 );
		Variable &variable = parser.variables[nodeVariable->variableID];
		Type &type = parser.types[variable.typeID];

		if( type.tokenType == TokenType_CBuffer )
		{
			String &typeName = type_name( variable.typeID );
			output.append( typeName ).append( "_" );
			generate_node( node->expr2 );
			return;
		}
	}

	// Only reachable if LHS not cbuffer type
	generate_node( node->expr1 );
	output.append( "." );
	generate_node( node->expr2 );
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


void GeneratorHLSL::generate_function_call_intrinsics( NodeFunctionCall *node )
{
	Intrinsic intrinsic = node->functionID;
	Assert( intrinsic < INTRINSIC_COUNT );

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
		case Intrinsic_ATan2:
		{
			generate_function_call( node );
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
		case Intrinsic_RSqrt:
		case Intrinsic_Clamp:
		case Intrinsic_Max:
		case Intrinsic_Min:
		case Intrinsic_Frac:
		case Intrinsic_Ldexp:
		case Intrinsic_Fma:
		case Intrinsic_Sign:
		case Intrinsic_Saturate:
		case Intrinsic_DDx:
		case Intrinsic_DDy:
		case Intrinsic_DDyCoarse:
		case Intrinsic_DDyFine:
		case Intrinsic_DDxCoarse:
		case Intrinsic_DDxFine:
		{
			generate_function_call( node );
		}
		return;

		case Intrinsic_Mod:
		{
			output.append( "fmod" );
			generate_function_call_parameters( node );
		}
		return;

	// Vector and Matrix Functions
		case Intrinsic_Mul:
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
		case Intrinsic_Lerp:
		case Intrinsic_Step:
		case Intrinsic_Smoothstep:
		{
			generate_function_call( node );
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
				"countbits",                   // Intrinsic_BitCount
				"firstbithigh",                // Intrinsic_BitFirstHigh
				"firstbitlow",                 // Intrinsic_BitFirstLow
				"reversebits",                 // Intrinsic_BitReverse
				"InterlockedAdd",              // Intrinsic_AtomicAdd
				"InterlockedCompareExchange",  // Intrinsic_AtomicCompareExchange
				"InterlockedExchange",         // Intrinsic_AtomicExchange
				"InterlockedMax",              // Intrinsic_AtomicMax
				"InterlockedMin",              // Intrinsic_AtomicMin
				"InterlockedAnd",              // Intrinsic_AtomicAnd
				"InterlockedOr",               // Intrinsic_AtomicOr
				"InterlockedXor",              // Intrinsic_AtomicXor
			};
			static_assert( ARRAY_LENGTH( bitwiseOperations ) == ( Intrinsic_AtomicXor - Intrinsic_BitCount + 1 ),
				"Missing Intrinsic!" );

			output.append( bitwiseOperations[intrinsic - Intrinsic_BitCount] );
			generate_function_call_parameters( node );
		}
		return;

	// Texture Sampling Functions
		case Intrinsic_SampleTexture1D:
		case Intrinsic_SampleTexture1DArray:
		case Intrinsic_SampleTexture2D:
		case Intrinsic_SampleTexture2DArray:
		case Intrinsic_SampleTexture3D:
		case Intrinsic_SampleTextureCube:
		case Intrinsic_SampleTextureCubeArray:
		{
			// <texture>.Sample( sampler, <uv> );
			generate_node( get_param( node->param, 0 )->expr );
			output.append( ".Sample( " );
			output.append( "GlobalSampler, " );
			generate_node( get_param( node->param, 1 )->expr );
			output.append( " )" );
		}
		return;

		case Intrinsic_SampleTexture2DLevel:
		{
			// <texture>.SampleLevel( sampler, <uv> );
			generate_node( get_param( node->param, 0 )->expr );
			output.append( ".SampleLevel( " );
			output.append( "GlobalSampler, " );
			generate_node( get_param( node->param, 1 )->expr );
			output.append( ", 0 )" );
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


void GeneratorHLSL::generate_structure( NodeStruct *node )
{
	const Struct &structure = parser.structs[node->structID];
	const Type &type = parser.types[structure.typeID];
	const String &typeName = type_name( structure.typeID );
	const VariableID first = type.memberFirst;
	const VariableID last = first + type.memberCount;

	static const char *structNames[] =
	{
		"struct",  // StructType_Struct
		"cbuffer", // StructType_CBuffer
		"struct",  // StructType_VertexInput
		"struct",  // StructType_VertexOutput
		"struct",  // StructType_FragmentInput
		"struct",  // StructType_FragmentOutput
		"struct",  // StructType_ComputeInput
		"struct",  // StructType_ComputeOutput
	};
	static_assert( ARRAY_LENGTH( structNames ) == STRUCTTYPE_COUNT, "Missing TextureType" );

	const bool expectSlot =  ( node->structType == StructType_CBuffer );
	const bool expectMeta = !( node->structType == StructType_CBuffer || node->structType == StructType_Struct );

	// <name> <type>
	output.append( structNames[node->structType] ).append( " " ).append( typeName );

	// : <register>
	if( expectSlot )
	{
		output.append( " : register( b" ).append( structure.slot ).append( " )" );
	}

	// { <body> }
	output.append( "\n{\n" );
	semantics_reset();
	indent_add();
	for( usize i = first; i < last; i++ )
	{
		Variable &memberVariable = parser.variables[i];
		const String &memberVariableName = variable_name( i );
		const String &memberTypeName = type_name( memberVariable.typeID );

		// <type>
		output.append( indent );
		output.append( memberTypeName ).append( " " );

		// <name>
		if( node->structType == StructType_CBuffer )
		{
			// cbuffer members are in global namespace, so we prefix them with the structure name
			output.append( typeName ).append( "_" );
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

		// : <semantic>
		if( expectMeta )
		{
			char buffer[128];
			semantics_get_name( buffer, sizeof( buffer ), node->structType, memberVariable );
			output.append( " : " ).append( buffer );
		}

		output.append( ";\n" );
	}
	indent_sub();
	output.append( "};\n\n" );
}


bool GeneratorHLSL::generate_structure_gfx_vertex( NodeStruct *node )
{
	// Register this vertex format in the system
	if( !Generator::generate_structure_gfx_vertex( node ) )
	{
		return false; // Already registered -- no need to continue
	}

	// Generate D3D11/HLSL Vertex Format interfaces
	const Struct &structure = parser.structs[node->structID];
	const Type &type = parser.types[structure.typeID];
	const VariableID first = type.memberFirst;
	const VariableID last = first + type.memberCount;

	int byteOffset = 0;
	int semanticIndex[SEMANTICTYPE_COUNT];
	for( u32 i = 0; i < SEMANTICTYPE_COUNT; i++ ) { semanticIndex[i] = 0; }

	String desc;

	desc.append( "\t" "static D3D11_INPUT_ELEMENT_DESC inputDescription[] = \n" );
	desc.append( "\t" "{\n" );
	for( usize i = first; i < last; i++ )
	{
		Variable &memberVariable = parser.variables[i];
		const String &memberVariableName = variable_name( i );
		const String &memberTypeName = type_name( memberVariable.typeID );

		// SemanticName
		desc.append( "\t" "\t" "{ \"" );
		switch( memberVariable.semantic )
		{
			case SemanticType_POSITION: desc.append( "POSITION" ); break;
			case SemanticType_TEXCOORD: desc.append( "TEXCOORD" ); break;
			case SemanticType_NORMAL: desc.append( "NORMAL" ); break;
			case SemanticType_DEPTH: desc.append( "DEPTH" ); break;
			case SemanticType_COLOR: desc.append( "COLOR" ); break;
		}
		desc.append( "\", " );

		// SemanticIndex
		desc.append( semanticIndex[memberVariable.semantic] );
		semanticIndex[memberVariable.semantic]++;
		desc.append( ", " );

		const char *format = "";
		int formatSize = 0;

		// Format
		switch( memberVariable.typeID )
		{
			case Primitive_Bool:
			case Primitive_Int:
			case Primitive_UInt:
			case Primitive_Float:
			case Primitive_Double:
			{
				switch( memberVariable.format )
				{
					// 1 Byte
					case InputFormat_UNORM8:
						format = "DXGI_FORMAT_R8_UNORM";
						formatSize = 1;
					break;

					case InputFormat_SNORM8:
						format = "DXGI_FORMAT_R8_SNORM";
						formatSize = 1;
					break;

					case InputFormat_UINT8:
						format = "DXGI_FORMAT_R8_UINT";
						formatSize = 1;
					break;

					case InputFormat_SINT8:
						format = "DXGI_FORMAT_R8_SINT";
						formatSize = 1;
					break;

					// 2 Bytes
					case InputFormat_UNORM16:
						format = "DXGI_FORMAT_R16_UNORM";
						formatSize = 2;
					break;

					case InputFormat_SNORM16:
						format = "DXGI_FORMAT_R16_SNORM";
						formatSize = 2;
					break;

					case InputFormat_UINT16:
						format = "DXGI_FORMAT_R16_UINT";
						formatSize = 2;
					break;

					case InputFormat_SINT16:
						format = "DXGI_FORMAT_R16_SINT";
						formatSize = 2;
					break;

					case InputFormat_FLOAT16:
						format = "DXGI_FORMAT_R16_FLOAT";
						formatSize = 2;
					break;

					// 4 Bytes
					case InputFormat_UNORM32:
						format = "DXGI_FORMAT_R32_UNORM";
						formatSize = 4;
					break;

					case InputFormat_SNORM32:
						format = "DXGI_FORMAT_R32_SNORM";
						formatSize = 4;
					break;

					case InputFormat_UINT32:
						format = "DXGI_FORMAT_R32_UINT";
						formatSize = 4;
					break;

					case InputFormat_SINT32:
						format = "DXGI_FORMAT_R32_SINT";
						formatSize = 4;
					break;

					case InputFormat_FLOAT32:
						format = "DXGI_FORMAT_R32_FLOAT";
						formatSize = 4;
					break;
				}
			}
			break;

			case Primitive_Bool2:
			case Primitive_Int2:
			case Primitive_UInt2:
			case Primitive_Float2:
			case Primitive_Double2:
			{
				switch( memberVariable.format )
				{
					// 1 Byte
					case InputFormat_UNORM8:
						format = "DXGI_FORMAT_R8G8_UNORM";
						formatSize = 2;
					break;

					case InputFormat_SNORM8:
						format = "DXGI_FORMAT_R8G8_SNORM";
						formatSize = 2;
					break;

					case InputFormat_UINT8:
						format = "DXGI_FORMAT_R8G8_UINT";
						formatSize = 2;
					break;

					case InputFormat_SINT8:
						format = "DXGI_FORMAT_R8G8_SINT";
						formatSize = 2;
					break;

					// 2 Bytes
					case InputFormat_UNORM16:
						format = "DXGI_FORMAT_R16G16_UNORM";
						formatSize = 4;
					break;

					case InputFormat_SNORM16:
						format = "DXGI_FORMAT_R16G16_SNORM";
						formatSize = 4;
					break;

					case InputFormat_UINT16:
						format = "DXGI_FORMAT_R16G16_UINT";
						formatSize = 4;
					break;

					case InputFormat_SINT16:
						format = "DXGI_FORMAT_R16G16_SINT";
						formatSize = 4;
					break;

					case InputFormat_FLOAT16:
						format = "DXGI_FORMAT_R16G16_FLOAT";
						formatSize = 4;
					break;

					// 4 Bytes
					case InputFormat_UNORM32:
						format = "DXGI_FORMAT_R32G32_UNORM";
						formatSize = 8;
					break;

					case InputFormat_SNORM32:
						format = "DXGI_FORMAT_R32G32_SNORM";
						formatSize = 8;
					break;

					case InputFormat_UINT32:
						format = "DXGI_FORMAT_R32G32_UINT";
						formatSize = 8;
					break;

					case InputFormat_SINT32:
						format = "DXGI_FORMAT_R32G32_SINT";
						formatSize = 8;
					break;

					case InputFormat_FLOAT32:
						format = "DXGI_FORMAT_R32G32_FLOAT";
						formatSize = 8;
					break;
				}
			}
			break;

			case Primitive_Bool3:
			case Primitive_Int3:
			case Primitive_UInt3:
			case Primitive_Float3:
			case Primitive_Double3:
			{
				switch( memberVariable.format )
				{
					// 1 Byte
					case InputFormat_UNORM8:
						format = "DXGI_FORMAT_R8G8B8A8_UNORM";
						formatSize = 4;
					break;

					case InputFormat_SNORM8:
						format = "DXGI_FORMAT_R8G8B8A8_SNORM";
						formatSize = 4;
					break;

					case InputFormat_UINT8:
						format = "DXGI_FORMAT_R8G8B8A8_UINT";
						formatSize = 4;
					break;

					case InputFormat_SINT8:
						format = "DXGI_FORMAT_R8G8B8A8_SINT";
						formatSize = 4;
					break;

					// 2 Bytes
					case InputFormat_UNORM16:
						format = "DXGI_FORMAT_R16G16B16A16_UNORM";
						formatSize = 8;
					break;

					case InputFormat_SNORM16:
						format = "DXGI_FORMAT_R16G16B16A16_SNORM";
						formatSize = 8;
					break;

					case InputFormat_UINT16:
						format = "DXGI_FORMAT_R16G16B16A16_UINT";
						formatSize = 8;
					break;

					case InputFormat_SINT16:
						format = "DXGI_FORMAT_R16G16B16A16_SINT";
						formatSize = 8;
					break;

					case InputFormat_FLOAT16:
						format = "DXGI_FORMAT_R16G16B16A16_FLOAT";
						formatSize = 8;
					break;

					// 4 Bytes
					case InputFormat_UNORM32:
						format = "DXGI_FORMAT_R32G32B32_TYPELESS";
						formatSize = 12;
					break;

					case InputFormat_SNORM32:
						format = "DXGI_FORMAT_R32G32B32_TYPELESS";
						formatSize = 12;
					break;

					case InputFormat_UINT32:
						format = "DXGI_FORMAT_R32G32B32_UINT";
						formatSize = 12;
					break;

					case InputFormat_SINT32:
						format = "DXGI_FORMAT_R32G32B32_SINT";
						formatSize = 12;
					break;

					case InputFormat_FLOAT32:
						format = "DXGI_FORMAT_R32G32B32_FLOAT";
						formatSize = 12;
					break;
				}
			}
			break;

			case Primitive_Bool4:
			case Primitive_Int4:
			case Primitive_UInt4:
			case Primitive_Float4:
			case Primitive_Double4:
			{
				switch( memberVariable.format )
				{
					// 1 Byte
					case InputFormat_UNORM8:
						format = "DXGI_FORMAT_R8G8B8A8_UNORM";
						formatSize = 4;
					break;

					case InputFormat_SNORM8:
						format = "DXGI_FORMAT_R8G8B8A8_SNORM";
						formatSize = 4;
					break;

					case InputFormat_UINT8:
						format = "DXGI_FORMAT_R8G8B8A8_UINT";
						formatSize = 4;
					break;

					case InputFormat_SINT8:
						format = "DXGI_FORMAT_R8G8B8A8_SINT";
						formatSize = 4;
					break;

					// 2 Bytes
					case InputFormat_UNORM16:
						format = "DXGI_FORMAT_R16G16B16A16_UNORM";
						formatSize = 8;
					break;

					case InputFormat_SNORM16:
						format = "DXGI_FORMAT_R16G16B16A16_SNORM";
						formatSize = 8;
					break;

					case InputFormat_UINT16:
						format = "DXGI_FORMAT_R16G16B16A16_UINT";
						formatSize = 8;
					break;

					case InputFormat_SINT16:
						format = "DXGI_FORMAT_R16G16B16A16_SINT";
						formatSize = 8;
					break;

					case InputFormat_FLOAT16:
						format = "DXGI_FORMAT_R16G16B16A16_FLOAT";
						formatSize = 8;
					break;

					// 4 Bytes
					case InputFormat_UNORM32:
						format = "DXGI_FORMAT_R32G32B32A32_TYPELESS";
						formatSize = 16;
					break;

					case InputFormat_SNORM32:
						format = "DXGI_FORMAT_R32G32B32A32_TYPELESS";
						formatSize = 16;
					break;

					case InputFormat_UINT32:
						format = "DXGI_FORMAT_R32G32B32A32_UINT";
						formatSize = 16;
					break;

					case InputFormat_SINT32:
						format = "DXGI_FORMAT_R32G32B32A32_SINT";
						formatSize = 16;
					break;

					case InputFormat_FLOAT32:
						format = "DXGI_FORMAT_R32G32B32A32_FLOAT";
						formatSize = 16;
					break;
				}
			}
			break;
		}
		desc.append( format );
		desc.append( ", " );

		// InputSlot
		desc.append( "0" );
		desc.append( ", " );

		// AlignedByteOffset
		desc.append( byteOffset );
		byteOffset += formatSize;
		desc.append( ", " );

		// InputSlotClass
		desc.append( "D3D11_INPUT_PER_VERTEX_DATA" );
		desc.append( ", " );

		// InstanceDataStepRate
		desc.append( "0" );
		desc.append( " },\n" );
	}
	desc.append( "\t" "};\n" );

	// Write To gfx.api.generated.cpp
	shader.source.append( "static void d3d11_vertex_input_layout_desc_" ).append( type.name );
	shader.source.append( "( D3D11VertexInputLayoutDescription &desc )\n{\n" );
	shader.source.append( desc ).append( "\n" );
	shader.source.append( "\t" "desc.desc = inputDescription;\n" );
	shader.source.append( "\t" "desc.count = ").append( static_cast<int>( last - first ) ).append( ";\n" );
	shader.source.append( "}\n\n" );

	return true;
}


void GeneratorHLSL::generate_texture( NodeTexture *node )
{
	// TODO: Multiple sampler states for each texture (like OpenGL)
	Texture &texture = parser.textures[node->textureID];
	const String &variableName = variable_name( texture.variableID );

	static const char *textureNames[] =
	{
		"Texture1D",        // TextureType_Texture1D
		"Texture1DArray",   // TextureType_Texture1DArray
		"Texture2D",        // TextureType_Texture2D
		"Texture2DArray",   // TextureType_Texture2DArray
		"Texture3D",        // TextureType_Texture3D
		"TextureCube",      // TextureType_TextureCube
		"TextureCubeArray", // TextureType_TextureCubeArray
	};
	static_assert( ARRAY_LENGTH( textureNames ) == TEXTURETYPE_COUNT, "Missing TextureType" );

	// Texture
	output.append( indent ).append( textureNames[node->textureType] ).append( " " );
	output.append( variableName );
	output.append( " : register( t" ).append( texture.slot ).append( " );\n" );

	// Sampler
	if( !generatedSampler )
	{
		output.append( indent ).append( "SamplerState" ).append( " " );
		output.append( "GlobalSampler" );
		output.append( " : register( s" ).append( 0 /*texture.slot*/ ).append( " );\n\n" );
		generatedSampler = true;
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}