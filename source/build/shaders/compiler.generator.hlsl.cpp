#include <build/shaders/compiler.generator.hpp>

#include <vendor/stdio.hpp>
#include <core/string.hpp>

#include <build/gfx.hpp>

namespace ShaderCompiler
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static u32 SemanticCount[SEMANTICTYPE_COUNT];

static bool generatedSampler = false;

static String INSTANCE_INPUT_VARIABLE_SUBSTITUTION = "";

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void semantics_reset()
{
	for( u32 i = 0; i < SEMANTICTYPE_COUNT; i++ )
	{
		SemanticCount[i] = 0;
	}
}


static void semantics_get_name( char *buffer, const usize size, StructType structType, Variable &variable )
{
	const char *name = "";
	Assert( variable.semantic < SEMANTICTYPE_COUNT );

	switch( structType )
	{
		case StructType_InstanceInput:
		{
			switch( variable.semantic )
			{
				case SemanticType_INSTANCE: name = "INSTANCE";   break;
				default: Error( "Unexpected semantic type: %u", variable.semantic ); return;
			}
		}
		break;

		case StructType_VertexInput:
		{
			switch( variable.semantic )
			{
				case SemanticType_POSITION: snprintf( buffer, size, "POSITION" ); return;
				case SemanticType_TEXCOORD: name = "TEXCOORD"; break;
				case SemanticType_NORMAL: name = "NORMAL"; break;
				case SemanticType_DEPTH: Error( "HLSL: vertex_input: unsupported semantic DEPTH" );
				case SemanticType_COLOR: name = "COLOR"; break;
				case SemanticType_BINORMAL: name = "BINORMAL"; break;
				case SemanticType_TANGENT: name = "TANGENT";   break;
				case SemanticType_INSTANCE: name = "INSTANCE";   break;
				default: Error( "Unexpected semantic type: %u", variable.semantic ); return;
			}
		}
		break;

		case StructType_VertexOutput:
		{
			switch( variable.semantic )
			{
				case SemanticType_POSITION: snprintf( buffer, size, "SV_POSITION" ); return;
				case SemanticType_TEXCOORD: name = "TEXCOORD"; break;
				case SemanticType_NORMAL: name = "NORMAL"; break;
				case SemanticType_DEPTH: Error( "HLSL: vertex_output: unsupported semantic DEPTH" );
				case SemanticType_COLOR: name = "COLOR"; break;
				case SemanticType_BINORMAL: name = "BINORMAL"; break;
				case SemanticType_TANGENT: name = "TANGENT";   break;
				case SemanticType_INSTANCE: name = "INSTANCE";   break;
				default: Error( "Unexpected semantic type: %u", variable.semantic ); return;
			}
		}
		break;

		case StructType_FragmentInput:
		{
			switch( variable.semantic )
			{
				case SemanticType_POSITION: snprintf( buffer, size, "SV_POSITION" ); return;
				case SemanticType_TEXCOORD: name = "TEXCOORD"; break;
				case SemanticType_NORMAL: name = "NORMAL";   break;
				case SemanticType_DEPTH: Error( "HLSL: fragment_input: unsupported semantic DEPTH" );
				case SemanticType_COLOR: name = "COLOR"; break;
				case SemanticType_BINORMAL: name = "BINORMAL"; break;
				case SemanticType_TANGENT: name = "TANGENT";   break;
				case SemanticType_INSTANCE: name = "INSTANCE";   break;
				default: Error( "Unexpected semantic type: %u", variable.semantic ); return;
			}
		}
		break;

		case StructType_FragmentOutput:
		{
			switch( variable.semantic )
			{
				case SemanticType_POSITION: Error( "HLSL: fragment_output: unsupported semantic POSITION" );
				case SemanticType_TEXCOORD: Error( "HLSL: fragment_output: unsupported semantic TEXCOORD" );
				case SemanticType_NORMAL: Error( "HLSL: fragment_output: unsupported semantic NORMAL" );
				case SemanticType_DEPTH: snprintf( buffer, size, "SV_DEPTH" ); return;
				case SemanticType_COLOR: name = "SV_TARGET"; break;
				case SemanticType_BINORMAL: Error( "HLSL: fragment_output: unsupported semantic BINORMAL" );
				case SemanticType_TANGENT: Error( "HLSL: fragment_output: unsupported semantic TANGENT" );
				case SemanticType_INSTANCE: Error( "HLSL: fragment_output: unsupported semantic INSTANCE" );
				default: Error( "Unexpected semantic type: %u", variable.semantic ); return;
			}
		}
		break;

		default: AssertMsg( false, "StructType %u cannot have semantics!", structType ); return;
	}

	// Append Count
	const u32 slot = variable.slot < 0 ? SemanticCount[variable.semantic] : static_cast<u32>( variable.slot );
	snprintf( buffer, size, "%s%u", name, slot );
	SemanticCount[variable.semantic]++;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GeneratorHLSL::generate_sv_semantic_struct()
{
	output.append( "struct SV\n{\n" );
	indent_add();

	if( stage == ShaderStage_Vertex )
	{
		output.append( indent ).append( "uint vertexID;\n" );
		output.append( indent ).append( "uint instanceID;\n" );
	}
	else if( stage == ShaderStage_Fragment )
	{
		output.append( indent ).append( "uint primitiveID;\n" );
		output.append( indent ).append( "bool isFrontFace;\n" );
		//output.append( indent ).append( "uint sampleIndex : SV_SAMPLEINDEX;\n" );
	}
	else if( stage == ShaderStage_Compute )
	{
		output.append( indent ).append( "uint3 dispatchThreadID;\n" );
		output.append( indent ).append( "uint3 groupID;\n" );
		output.append( indent ).append( "uint3 groupThreadID;\n" );
		output.append( indent ).append( "uint groupIndex;\n" );
	}

	indent_sub();
	output.append( "};\n\n" );
}


void GeneratorHLSL::generate_sv_semantic_entry_parameters()
{
	if( stage == ShaderStage_Vertex )
	{
		output.append( "uint vertexID : SV_VERTEXID, " );
		output.append( "uint instanceID : SV_INSTANCEID" );
	}
	else if( stage == ShaderStage_Fragment )
	{
		output.append( "uint primitiveID : SV_PRIMITIVEID, " );
		output.append( "uint isFrontFace : SV_ISFRONTFACE" );
		//output.append( "uint sampleIndex : SV_SAMPLEINDEX, " );
	}
	else if( stage == ShaderStage_Compute )
	{
		output.append( "uint3 dispatchThreadID : SV_DISPATCHTHREADID, " );
		output.append( "uint3 groupThreadID : SV_GROUPTHREADID, " );
		output.append( "uint3 groupID : SV_GROUPID, " );
		output.append( "uint groupIndex : SV_GROUPINDEX" );
	}
}


void GeneratorHLSL::generate_sv_semantic_entry_caching()
{
	indent_add();
	output.append( indent ).append( "SV sv;\n" );

	if( stage == ShaderStage_Vertex )
	{
		output.append( indent ).append( "sv.vertexID = vertexID;\n" );
		output.append( indent ).append( "sv.instanceID = instanceID;\n" );
	}
	else if( stage == ShaderStage_Fragment )
	{
		output.append( indent ).append( "sv.primitiveID = primitiveID;\n" );
		output.append( indent ).append( "sv.isFrontFace = isFrontFace;\n" );
		//output.append( indent ).append( "sv.sampleIndex = sampleIndex;\n" );
	}
	else if( stage == ShaderStage_Compute )
	{
		output.append( indent ).append( "sv.dispatchThreadID = dispatchThreadID;\n" );
		output.append( indent ).append( "sv.groupThreadID = groupThreadID;\n" );
		output.append( indent ).append( "sv.groupID = groupID;\n" );
		output.append( indent ).append( "sv.groupIndex = groupIndex;\n" );
	}

	output.append( "\n" );
	indent_sub();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GeneratorHLSL::append_structure_member_padded( String &output, const char *indent,
	Type &type, Variable &variable, int &structureByteOffset )
{
	static char typeNameBuffer[512];
	int typeSizeBytes = 0;

	StringView typeNameCPU;
	int size, align;
	switch( variable.typeID )
	{
		case Primitive_Bool: typeNameCPU = StringView( "u32" ); size = 4; align = 4; break;
		case Primitive_Bool2: typeNameCPU = StringView( "u32_v2" ); size = 8; align = 4; break;
		case Primitive_Bool3: typeNameCPU = StringView( "u32_v3" ); size = 12; align = 4; break;
		case Primitive_Bool4: typeNameCPU = StringView( "u32_v4" ); size = 16; align = 16; break;

		case Primitive_Int: typeNameCPU = StringView( "i32" ); size = 4; align = 4; break;
		case Primitive_Int2: typeNameCPU = StringView( "int_v2" ); size = 8; align = 4; break;
		case Primitive_Int3: typeNameCPU = StringView( "int_v3" ); size = 12; align = 4; break;
		case Primitive_Int4: typeNameCPU = StringView( "int_v4" ); size = 16; align = 16; break;

		case Primitive_UInt: typeNameCPU = StringView( "u32" ); size = 4; align = 4; break;
		case Primitive_UInt2: typeNameCPU = StringView( "u32_v2" ); size = 8; align = 4; break;
		case Primitive_UInt3: typeNameCPU = StringView( "u32_v3" ); size = 12; align = 4; break;
		case Primitive_UInt4: typeNameCPU = StringView( "u32_v4" ); size = 16; align = 16; break;

		case Primitive_Float: typeNameCPU = StringView( "float" ); size = 4; align = 4; break;
		case Primitive_Float2: typeNameCPU = StringView( "float_v2" ); size = 8; align = 4; break;
		case Primitive_Float3: typeNameCPU = StringView( "float_v3" ); size = 12; align = 4; break;
		case Primitive_Float4: typeNameCPU = StringView( "float_v4" ); size = 16; align = 16; break;

		case Primitive_Float2x2: typeNameCPU = StringView( "float_m44" ); size = 64; align = 16; break;
		case Primitive_Float3x3: typeNameCPU = StringView( "float_m44" ); size = 64; align = 16; break;
		case Primitive_Float4x4: typeNameCPU = StringView( "float_m44" ); size = 64; align = 16; break;

		case Primitive_Double: typeNameCPU = StringView( "double" ); size = 8; align = 4; break;
		case Primitive_Double2: typeNameCPU = StringView( "double_v2" ); size = 16; align = 4; break;
		case Primitive_Double3: typeNameCPU = StringView( "double_v3" ); size = 24; align = 4; break;
		case Primitive_Double4: typeNameCPU = StringView( "double_v4" ); size = 32; align = 16; break;

		case Primitive_Double2x2: typeNameCPU = StringView( "double_m44" ); size = 64; align = 16; break;
		case Primitive_Double3x3: typeNameCPU = StringView( "double_m44" ); size = 64; align = 16; break;
		case Primitive_Double4x4: typeNameCPU = StringView( "double_m44" ); size = 64; align = 16; break;

		default: typeNameCPU.data = nullptr; size = 0; align = 16; break;
	};

	// Struct Size
	if( type.tokenType == TokenType_SharedStruct ) { size = type.sizeBytesPadded; align = 16; }

	// Calculate required padding & increment byte offset
	structureByteOffset += Generator::append_structure_padding( output, indent, size, align, structureByteOffset );
	auto round_up16 = []( int size ) { return ( size + 15 ) & ~15; };

	// Type
	if( variable.arrayLengthX > 0 && variable.arrayLengthY > 0 )
	{
		snprintf( typeNameBuffer, sizeof( typeNameBuffer ), "std140_array_2d_hlsl<%s%.*s, %d, %d>",
			type.tokenType == TokenType_SharedStruct ? "GfxStructPadded::" : "",
			static_cast<int>( typeNameCPU.data == nullptr ? type.name.length : typeNameCPU.length ),
			typeNameCPU.data == nullptr ? type.name.data : typeNameCPU.data,
			variable.arrayLengthX, variable.arrayLengthY );

		typeSizeBytes = round_up16( size ) *
			( variable.arrayLengthX * variable.arrayLengthY - 1 ) + size;

#if GFX_OUTPUT_STRUCTURE_SIZES
		output.append( indent ).append( "static_assert( sizeof( " ).append( typeNameBuffer );
		output.append( " ) == " ).append( typeSizeBytes ).append( ", \"size missmatch!\" );\n" );;
#endif

		// 2D Array
		output.append( indent ).append( typeNameBuffer );
	}
	else if( variable.arrayLengthX > 0 )
	{
		snprintf( typeNameBuffer, sizeof( typeNameBuffer ), "std140_array_1d_hlsl<%s%.*s, %d>",
			type.tokenType == TokenType_SharedStruct ? "GfxStructPadded::" : "",
			static_cast<int>( typeNameCPU.data == nullptr ? type.name.length : typeNameCPU.length ),
			typeNameCPU.data == nullptr ? type.name.data : typeNameCPU.data,
			variable.arrayLengthX );

		typeSizeBytes = round_up16( size ) * ( variable.arrayLengthX - 1 ) + size;

#if GFX_OUTPUT_STRUCTURE_SIZES
		output.append( indent ).append( "static_assert( sizeof( " ).append( typeNameBuffer );
		output.append( " ) == " ).append( typeSizeBytes ).append( ", \"size missmatch!\" );\n" );
#endif

		// 2D Array
		output.append( indent ).append( typeNameBuffer );
	}
	else
	{
		snprintf( typeNameBuffer, sizeof( typeNameBuffer ), "%s%.*s",
			type.tokenType == TokenType_SharedStruct ? "GfxStructPadded::" : "",
			static_cast<int>( typeNameCPU.data == nullptr ? type.name.length : typeNameCPU.length ),
			typeNameCPU.data == nullptr ? type.name.data : typeNameCPU.data );

		typeSizeBytes = size;

#if GFX_OUTPUT_STRUCTURE_SIZES
		output.append( indent ).append( "static_assert( sizeof( " ).append( typeNameBuffer );
		output.append( " ) == " ).append( typeSizeBytes ).append( ", \"size missmatch!\" );\n" );
#endif

		// 2D Array
		output.append( indent ).append( typeNameBuffer );
	}

	// Variable
	output.append( " " ).append( variable.name ).append( ";" );
#if GFX_OUTPUT_STRUCTURE_SIZES
	output.append( " // Offset: " ).append( structureByteOffset );
#endif
	output.append( "\n" );

	// Increment structureByteOffset
	structureByteOffset += typeSizeBytes;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GeneratorHLSL::generate_stage( ShaderStage stage )
{
	generatedSampler = false;

	// SV Semantics
	generate_sv_semantic_struct();

	// Super
	Generator::generate_stage( stage );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GeneratorHLSL::generate_statement_for( NodeStatementFor *node )
{
	output.append( indent ).append( "[loop] for( " );
	if( node->expr1 != nullptr ) { generate_node( node->expr1 ); }
	output.append( "; " );
	if( node->expr2 != nullptr ) { generate_node( node->expr2 ); } else { output.append( "true" ); }
	output.append( ";" );
	if( node->expr3 != nullptr ) { output.append( " " ); generate_node( node->expr3 ); }
	output.append( " )\n" );

	if( node->block != nullptr )
	{
		generate_node( node->block );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GeneratorHLSL::generate_function_declaration( NodeFunctionDeclaration *node )
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
		output.append( "( SV sv" );
		VariableID parameterCount = 0;
		for( VariableID i = first; i < last; i++ )
		{
			if( i == first ) { output.append( "," ); }
			Variable &parameter = parser.variables[i];
			Type &parameterType = parser.types[parameter.typeID];

			// Skip buffers / instance inputs  (global namespace)
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


void GeneratorHLSL::generate_function_declaration_main_pipeline( NodeFunctionDeclaration *node )
{
	Function &function = parser.functions[node->functionID];
	Type &returnType = parser.types[function.typeID];

	// In / Out
	Assert( function.parameterCount >= 2 );
	VariableID inID = function.parameterFirst;
	Type &inType = parser.types[parser.variables[inID].typeID];
	const String &inTypeName = type_name( parser.variables[inID].typeID );
	INSTANCE_INPUT_VARIABLE_SUBSTITUTION = variable_name( inID ); // instance_input -> vertex_input

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
	output.append( "out " ).append( outTypeName ).append( " " ).append( variable_name( outID ) ).append( ", " );
	generate_sv_semantic_entry_parameters();
	output.append( " )\n" );

	generate_statement_block_main( reinterpret_cast<NodeStatementBlock *>( node->block ) );
	output.append( "\n" );
}


void GeneratorHLSL::generate_function_declaration_main_compute( NodeFunctionDeclaration *node )
{
	Function &function = parser.functions[node->functionID];
	Type &returnType = parser.types[function.typeID];

	// Thread Groups
	output.append( indent ).append( "[numthreads( " );
	output.append( parser.threadGroupX ).append( ", " );
	output.append( parser.threadGroupY ).append( ", " );
	output.append( parser.threadGroupZ ).append( " )]\n" );

	// Return Type & Name
	output.append( indent ).append( "void cs_main( " );
	generate_sv_semantic_entry_parameters();
	output.append( " )\n" );

	generate_statement_block_main( reinterpret_cast<NodeStatementBlock *>( node->block ) );
	output.append( "\n" );
}


void GeneratorHLSL::generate_statement_block_main( NodeStatementBlock *node )
{
	output.append( indent ).append( "{\n" );

	// SV Semantics
	generate_sv_semantic_entry_caching();

	// Body
	generate_statement_block_no_braces( node );
	output.append( indent ).append( "}\n" );
}


void GeneratorHLSL::generate_expression_binary_dot( NodeExpressionBinary *node )
{
	Assert( node->exprType == ExpressionBinaryType_Dot );

	if( node->expr1->nodeType == NodeType_Variable )
	{
		NodeVariable *nodeVariable = reinterpret_cast<NodeVariable *>( node->expr1 );
		Variable &variable = parser.variables[nodeVariable->variableID];
		Type &type = parser.types[variable.typeID];

		if( type.tokenType == TokenType_UniformBuffer ||
			type.tokenType == TokenType_ConstantBuffer ||
			type.tokenType == TokenType_MutableBuffer )
		{
			// In HLSL, buffer variables are in the global namespace
			// Therefore, we prefix: <Buffer>_<Variable>
			output.append( type_name( variable.typeID ) ).append( "_" );
			generate_node( node->expr2 );
			return;
		}
		else if( type.tokenType == TokenType_InstanceInput )
		{
			ErrorIf( stage != ShaderStage_Vertex,
				"TokenType_InstanceInput encounted in wrong stage: %d", stage );

			// In HLSL, instance data is part of the vertex format vertex format: <Buffer>_<Variable>
			// Therefore, we rename & prefix: <Vertex>.<Instance>_<variable>
			output.append( INSTANCE_INPUT_VARIABLE_SUBSTITUTION ); // vertex_format in (vs only)
			output.append( "." );
			output.append( type_name( variable.typeID ) ).append( "_" );
			generate_node( node->expr2 );
			return;
		}
	}

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


void GeneratorHLSL::generate_function_call_parameters( NodeFunctionCall *node )
{
	output.append( "(" );

	// HLSL: All custom functions must pass SV
	const bool isIntrinsic = node->functionID < INTRINSIC_COUNT;
	if( !isIntrinsic ) { output.append( " sv" ); }

	if( node->param != nullptr )
	{
		output.append( isIntrinsic ? "" : "," );
		Node *param = node->param;
		usize parameterCount = 0;
		while( param != nullptr )
		{
			NodeExpressionList *paramNode = reinterpret_cast<NodeExpressionList *>( param );
			bool skip = false;

			// HLSL
			if( paramNode->expr->nodeType == NodeType_Variable )
			{
				NodeVariable *nodeVariable = reinterpret_cast<NodeVariable *>( paramNode->expr );
				Variable &variable = parser.variables[nodeVariable->variableID];
				Type &type = parser.types[variable.typeID];

				if( type.tokenType == TokenType_UniformBuffer ||
					type.tokenType == TokenType_ConstantBuffer ||
					type.tokenType == TokenType_MutableBuffer )
				{
					skip = true;
				}
			}

			if( !skip )
			{
				output.append( parameterCount == 0 ? " " : ", " );
				generate_node( paramNode->expr );
				parameterCount++;
			}

			param = paramNode->next;
		}
	}

	output.append( " )" );
}


void GeneratorHLSL::generate_function_call_intrinsics( NodeFunctionCall *node )
{
	Intrinsic intrinsic = node->functionID;
	Assert( intrinsic < INTRINSIC_COUNT );

	// Helper to output UV with flipped V
	#if 0
	auto generate_node_uvs_flipped = [this]( Node *node )
		{
			output.append( "float2( " );
				generate_node_parenthesis( node );
				output.append( ".x, 1.0 - " );
				generate_node_parenthesis( node );
				output.append( ".y" );
			output.append( " )" );
		};
	#endif

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

	// Type Bit Conversions
		case Intrinsic_FloatToIntBits:
		case Intrinsic_FloatToUIntBits:
		case Intrinsic_IntToFloatBits:
		case Intrinsic_UIntToFloatBits:
		{
			static const char *intrinsics[] =
			{
				"asint",    // Intrinsic_FloatToIntBits
				"asuint",   // Intrinsic_FloatToUIntBits
				"asfloat",  // Intrinsic_IntToFloatBits
				"asfloat",  // Intrinsic_UIntToFloatBits
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
			// <texture>.Sample( sampler, location );
			generate_node( get_param( node->param, 0 )->expr );
			output.append( ".Sample( " );
			output.append( "GlobalSampler, " );
			generate_node( get_param( node->param, 1 )->expr );
			output.append( " )" );
		}
		return;

		case Intrinsic_TextureSample1DLevel:
		case Intrinsic_TextureSample2DLevel:
		case Intrinsic_TextureSample3DLevel:
		case Intrinsic_TextureSampleCubeLevel:
		{
			// <texture>.SampleLevel( sampler, location, lod );
			generate_node( get_param( node->param, 0 )->expr );
			output.append( ".SampleLevel( " );
			output.append( "GlobalSampler, " );
			generate_node( get_param( node->param, 1 )->expr );
			output.append( ", " );
			generate_node( get_param( node->param, 2 )->expr );
			output.append( " )" );
		}
		return;

		case Intrinsic_TextureLoad1D:
		{
			// <texture>.Load( float2( location.x, lod ) );
			generate_node( get_param( node->param, 0 )->expr );
			output.append( ".Load( float2( " );
			generate_node( get_param( node->param, 1 )->expr ); // u
			output.append( ", " );
			generate_node( get_param( node->param, 3 )->expr ); // lod
			output.append( " ) )" );
		}
		return;

		case Intrinsic_TextureLoad2D:
		{
			// <texture>.Load( float3( location.xy, lod ) );
			generate_node( get_param( node->param, 0 )->expr );
			output.append( ".Load( float3( " );
			generate_node( get_param( node->param, 1 )->expr ); // u
			output.append( ", " );
			generate_node( get_param( node->param, 2 )->expr ); // v
			output.append( ", " );
			generate_node( get_param( node->param, 5 )->expr ); // lod
			output.append( " ) )" );
		}
		return;

		case Intrinsic_TextureLoad3D:
		{
			// <texture>.Load( float4( location.xyz, lod ) );
			generate_node( get_param( node->param, 0 )->expr );
			output.append( ".Load( float4( " );
			generate_node( get_param( node->param, 1 )->expr ); // u
			output.append( ", " );
			generate_node( get_param( node->param, 2 )->expr ); // v
			output.append( ", " );
			generate_node( get_param( node->param, 3 )->expr ); // w
			output.append( ", " );
			generate_node( get_param( node->param, 7 )->expr ); // lod
			output.append( " ) )" );
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
				generate_node_parenthesis( get_param( node->param, 0 )->expr );
				output.append( ".z / " );
				generate_node_parenthesis( get_param( node->param, 0 )->expr );
				output.append( ".w" );
			output.append( " )" );
		}
		return;

		case Intrinsic_DepthUnprojectZW:
		{
			output.append( "( " );
				generate_node_parenthesis( get_param( node->param, 0 )->expr );
				output.append( " / " );
				generate_node_parenthesis( get_param( node->param, 1 )->expr );
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


void GeneratorHLSL::generate_structure( NodeStruct *node )
{
	// Skip generating StructType_InstanceInput
	if( node->structType == StructType_InstanceInput ) { return; }

	// Append Member Variables Helper
	auto append_structure_members = [&]( NodeStruct *node, const String &prefix ) -> void
	{
		const Struct &structure = parser.structs[node->structID];
		const Type &type = parser.types[structure.typeID];
		const String &typeName = type_name( structure.typeID );
		const VariableID first = type.memberFirst;
		const VariableID last = first + type.memberCount;

		const bool expectMeta = !( node->structType == StructType_UniformBuffer ||
			node->structType == StructType_ConstantBuffer ||
			node->structType == StructType_MutableBuffer ||
			node->structType == StructType_Struct ||
			node->structType == StructType_SharedStruct );

		for( usize i = first; i < last; i++ )
		{
			Variable &memberVariable = parser.variables[i];
			const String &memberVariableName = variable_name( i );
			const String &memberTypeName = type_name( memberVariable.typeID );

			// Indent
			output.append( indent );

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
						output.append( "nointerpolation " );
					break;
				}
			}

			// <type>
			output.append( memberTypeName ).append( " " );

			// <name>
			if( node->structType == StructType_UniformBuffer ||
				node->structType == StructType_ConstantBuffer ||
				node->structType == StructType_MutableBuffer )
			{
				// buffer members are in global namespace, so we prefix them with the structure name
				output.append( typeName ).append( "_" );
			}

			if( !prefix.is_empty() ) { output.append( prefix ).append( "_" ); }
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
	};

	const Struct &structure = parser.structs[node->structID];
	const Type &type = parser.types[structure.typeID];
	const String &typeName = type_name( structure.typeID );

	static const char *structNames[] =
	{
		"struct",  // StructType_Struct
		"struct",  // StructType_SharedStruct
		"cbuffer", // StructType_UniformBuffer
		"cbuffer", // StructType_Constantuffer
		"cbuffer", // StructType_MutableBuffer
		"struct",  // StructType_InstanceInput
		"struct",  // StructType_VertexInput
		"struct",  // StructType_VertexOutput
		"struct",  // StructType_FragmentInput
		"struct",  // StructType_FragmentOutput
		"struct",  // StructType_ComputeInput
		"struct",  // StructType_ComputeOutput
	};
	static_assert( ARRAY_LENGTH( structNames ) == STRUCTTYPE_COUNT, "Missing TextureType" );

	const bool expectSlot =  ( node->structType == StructType_UniformBuffer ||
		node->structType == StructType_ConstantBuffer || node->structType == StructType_MutableBuffer );

	// <name> <type>
	output.append( structNames[node->structType] ).append( " " ).append( typeName );

	// : <register>
	if( expectSlot )
	{
		output.append( " : register( b" ).append( structure.slot ).append( " )" );
	}

	// {
	output.append( "\n{\n" );
	semantics_reset();
	indent_add();

	// Members
	append_structure_members( node, "" );

	// In HLSL, instance_input members belong to the vertex format
	if( node->structType == StructType_VertexInput && shader.instanceFormatID != U32_MAX )
	{
		const InstanceFormat &instanceFormat = Gfx::instanceFormats[shader.instanceFormatID];
		if( instanceFormat.node != nullptr )
		{
			NodeStruct *nodeInstanceInput = reinterpret_cast<NodeStruct *>( instanceFormat.node );
			const String &instanceInputName = type_name( parser.structs[nodeInstanceInput->structID].typeID );
			output.append( "\n" );
			append_structure_members( nodeInstanceInput, instanceInputName );
		}
	}

	// }
	indent_sub();
	output.append( "};\n\n" );
}


void GeneratorHLSL::generate_sv_semantic( NodeSVSemantic *node )
{
	switch( node->svSemanticType )
	{
		case SVSemanticType_DISPATCH_THREAD_ID: output.append( "sv.dispatchThreadID" ); return;
		case SVSemanticType_GROUP_ID:  output.append( "sv.groupID" ); return;
		case SVSemanticType_GROUP_THREAD_ID: output.append( "sv.groupThreadID" ); return;
		case SVSemanticType_GROUP_INDEX: output.append( "sv.groupIndex" ); return;
		case SVSemanticType_VERTEX_ID: output.append( "sv.vertexID" ); return;
		case SVSemanticType_INSTANCE_ID: output.append( "sv.instanceID" ); return;
		case SVSemanticType_PRIMITIVE_ID: output.append( "sv.primitiveID" ); return;
		case SVSemanticType_FRONT_FACING: output.append( "sv.isFrontFace" ); return;
		case SVSemanticType_SAMPLE_ID: output.append( "sv.sampleIndex" ); return;

		default: Error( "Unexpected SV Semantic: %s", SVSemantics[node->svSemanticType] );
	}
}


bool GeneratorHLSL::generate_structure_gfx_instance( NodeStruct *node )
{
	// Register this vertex format in the system
	if( !Generator::generate_structure_gfx_instance( node ) )
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

	const usize count = last - first;
	if( count > 0 )
	{
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
				case SemanticType_BINORMAL: desc.append( "BINORMAL" ); break;
				case SemanticType_TANGENT: desc.append( "TANGENT" ); break;
				case SemanticType_INSTANCE: desc.append( "INSTANCE" ); break;
				default: Error( "Unexpected semantic type: %u", memberVariable.semantic ); break;
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
			desc.append( "1" );
			desc.append( ", " );

			// AlignedByteOffset
			desc.append( byteOffset );
			byteOffset += formatSize;
			desc.append( ", " );

			// InputSlotClass
			desc.append( "D3D11_INPUT_PER_INSTANCE_DATA" );
			desc.append( ", " );

			// InstanceDataStepRate
			desc.append( "1" );
			desc.append( " },\n" );
		}
		desc.append( "\t" "};\n" );
	}
	else
	{
		desc.append( "\t" "static D3D11_INPUT_ELEMENT_DESC *inputDescription = nullptr;\n" );
	}

	// Write To gfx.api.generated.cpp
	shader.source.append( "static void d3d11_input_layout_desc_instance_" ).append( type.name );
	shader.source.append( "( D3D11InputLayoutDescription &desc )\n{\n" );
	shader.source.append( desc ).append( "\n" );
	shader.source.append( "\t" "desc.desc = inputDescription;\n" );
	shader.source.append( "\t" "desc.count = ").append( count ).append( ";\n" );
	shader.source.append( "}\n\n" );

	return true;
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

	const usize count = last - first;
	if( count > 0 )
	{
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
				case SemanticType_BINORMAL: desc.append( "BINORMAL" ); break;
				case SemanticType_TANGENT: desc.append( "TANGENT" ); break;
				case SemanticType_INSTANCE: desc.append( "INSTANCE" ); break;
				default: Error( "Unexpected semantic type: %u", memberVariable.semantic ); break;
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
	}
	else
	{
		desc.append( "\t" "static D3D11_INPUT_ELEMENT_DESC *inputDescription = nullptr;\n" );
	}

	// Write To gfx.api.generated.cpp
	shader.source.append( "static void d3d11_input_layout_desc_vertex_" ).append( type.name );
	shader.source.append( "( D3D11InputLayoutDescription &desc )\n{\n" );
	shader.source.append( desc ).append( "\n" );
	shader.source.append( "\t" "desc.desc = inputDescription;\n" );
	shader.source.append( "\t" "desc.count = ").append( count ).append( ";\n" );
	shader.source.append( "}\n\n" );

	return true;
}


void GeneratorHLSL::generate_texture( NodeTexture *node )
{
	// TODO: Multiple sampler states for each texture (like OpenGL)
	Texture &texture = parser.textures[node->textureID];
	const String &typeName = type_name( parser.variables[texture.variableID].typeID );
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
	output.append( indent ).append( textureNames[node->textureType] );
	output.append( "<" ).append( typeName ).append( "> " );
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