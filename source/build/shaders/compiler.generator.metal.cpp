#include <build/shaders/compiler.generator.hpp>

#include <vendor/stdio.hpp>
#include <core/string.hpp>
#include <core/checksum.hpp>

namespace ShaderCompiler
{
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static String entryParameters;
static String globalMembers;
static String globalConstructor;
static bool globalHasSampler;

static String INSTANCE_INPUT_VARIABLE_SUBSTITUTION = "";

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct MetalInputFormatInfo { const char *format; int size; };

static const MetalInputFormatInfo METAL_INPUT_FORMATS_UNORM8[] =
{
	{ "MTLVertexFormatUCharNormalized", 1 },
	{ "MTLVertexFormatUChar2Normalized", 2 },
	{ "MTLVertexFormatUChar4Normalized", 4 },
	{ "MTLVertexFormatUChar4Normalized", 4 },
};


static const MetalInputFormatInfo METAL_INPUT_FORMATS_UNORM16[] =
{
	{ "MTLVertexFormatUShortNormalized", 2 },
	{ "MTLVertexFormatUShort2Normalized", 4 },
	{ "MTLVertexFormatUShort4Normalized", 8 },
	{ "MTLVertexFormatUShort4Normalized", 8 },
};


static const MetalInputFormatInfo METAL_INPUT_FORMATS_SNORM8[] =
{
	{ "MTLVertexFormatCharNormalized", 1 },
	{ "MTLVertexFormatChar2Normalized", 2 },
	{ "MTLVertexFormatChar4Normalized", 4 },
	{ "MTLVertexFormatChar4Normalized", 4 },
};


static const MetalInputFormatInfo METAL_INPUT_FORMATS_SNORM16[] =
{
	{ "MTLVertexFormatShortNormalized", 2 },
	{ "MTLVertexFormatShort2Normalized", 4 },
	{ "MTLVertexFormatShort4Normalized", 8 },
	{ "MTLVertexFormatShort4Normalized", 8 },
};


static const MetalInputFormatInfo METAL_INPUT_FORMATS_UINT8[] =
{
	{ "MTLVertexFormatUChar", 1 },
	{ "MTLVertexFormatUChar2", 2 },
	{ "MTLVertexFormatUChar4", 4 },
	{ "MTLVertexFormatUChar4", 4 },
};


static const MetalInputFormatInfo METAL_INPUT_FORMATS_UINT16[] =
{
	{ "MTLVertexFormatUShort", 2 },
	{ "MTLVertexFormatUShort2", 4 },
	{ "MTLVertexFormatUShort4", 8 },
	{ "MTLVertexFormatUShort4", 8 },
};


static const MetalInputFormatInfo METAL_INPUT_FORMATS_UINT32[] =
{
	{ "MTLVertexFormatUInt", 4 },
	{ "MTLVertexFormatUInt2", 8 },
	{ "MTLVertexFormatUInt3", 12 },
	{ "MTLVertexFormatUInt4", 16 },
};


static const MetalInputFormatInfo METAL_INPUT_FORMATS_SINT8[] =
{
	{ "MTLVertexFormatChar", 1 },
	{ "MTLVertexFormatChar2", 2 },
	{ "MTLVertexFormatChar4", 4 },
	{ "MTLVertexFormatChar4", 4 },
};


static const MetalInputFormatInfo METAL_INPUT_FORMATS_SINT16[] =
{
	{ "MTLVertexFormatShort", 2 },
	{ "MTLVertexFormatShort2", 4 },
	{ "MTLVertexFormatShort4", 8 },
	{ "MTLVertexFormatShort4", 8 },
};


static const MetalInputFormatInfo METAL_INPUT_FORMATS_SINT32[] =
{
	{ "MTLVertexFormatInt", 4 },
	{ "MTLVertexFormatInt2", 8 },
	{ "MTLVertexFormatInt3", 12 },
	{ "MTLVertexFormatInt4", 16 },
};


static const MetalInputFormatInfo METAL_INPUT_FORMATS_FLOAT16[] =
{
	{ "MTLVertexFormatHalf", 2 },
	{ "MTLVertexFormatHalf2", 4 },
	{ "MTLVertexFormatHalf4", 8 },
	{ "MTLVertexFormatHalf4", 8 },
};


static const MetalInputFormatInfo METAL_INPUT_FORMATS_FLOAT32[] =
{
	{ "MTLVertexFormatFloat", 4 },
	{ "MTLVertexFormatFloat2", 8 },
	{ "MTLVertexFormatFloat3", 12 },
	{ "MTLVertexFormatFloat4", 16 },
};


static const MetalInputFormatInfo *const METAL_INPUT_FORMATS[] =
{
	METAL_INPUT_FORMATS_UNORM8, // InputFormat_UNORM8
	METAL_INPUT_FORMATS_UNORM16, // InputFormat_UNORM16
	METAL_INPUT_FORMATS_UINT32, // InputFormat_UNORM32 (NOTE: not officially supported)
	METAL_INPUT_FORMATS_SNORM8, // InputFormat_SNORM8
	METAL_INPUT_FORMATS_SNORM16, // InputFormat_SNORM16
	METAL_INPUT_FORMATS_SINT32, // InputFormat_SNORM32 (NOTE: not officially supported)
	METAL_INPUT_FORMATS_UINT8, // InputFormat_UINT8
	METAL_INPUT_FORMATS_UINT16, // InputFormat_UINT16
	METAL_INPUT_FORMATS_UINT32, // InputFormat_UINT32
	METAL_INPUT_FORMATS_SINT8, // InputFormat_SINT8
	METAL_INPUT_FORMATS_SINT16, // InputFormat_SINT16
	METAL_INPUT_FORMATS_SINT32, // InputFormat_SINT32
	METAL_INPUT_FORMATS_FLOAT16, // InputFormat_FLOAT16
	METAL_INPUT_FORMATS_FLOAT32, // InputFormat_FLOAT32
};


static const MetalInputFormatInfo *const metal_input_format( const Primitive primitve, const InputFormat format )
{
	switch( primitve )
	{
		case Primitive_Bool:
		case Primitive_Int:
		case Primitive_UInt:
		case Primitive_Float:
			return &METAL_INPUT_FORMATS[format][0];

		case Primitive_Bool2:
		case Primitive_Int2:
		case Primitive_UInt2:
		case Primitive_Float2:
			return &METAL_INPUT_FORMATS[format][1];

		case Primitive_Bool3:
		case Primitive_Int3:
		case Primitive_UInt3:
		case Primitive_Float3:
			return &METAL_INPUT_FORMATS[format][2];

		case Primitive_Bool4:
		case Primitive_Int4:
		case Primitive_UInt4:
		case Primitive_Float4:
			return &METAL_INPUT_FORMATS[format][3];

		default:
			return nullptr;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GeneratorMetal::append_structure_member_padded( String &output, const char *indent,
	Type &type, Variable &variable, int &structureByteOffset )
{
	auto round_up16 = []( int size ) { return ( size + 15 ) & ~15; };
	auto round_upAlign = [](int size, int alignment) { return ( size + ( alignment - 1 ) ) & ~( alignment - 1 ); };

	char typeNameBuffer[512];
	int typeSizeBytes = 0;

	StringView typeNameCPU;
	int size, align;
	switch( variable.typeID )
	{
		case Primitive_Bool: typeNameCPU = StringView( "u32" ); size = 4; align = 4; break;
		case Primitive_Bool2: typeNameCPU = StringView( "u32_v2" ); size = 8; align = 8; break;
		case Primitive_Bool3: typeNameCPU = StringView( "u32_v3" ); size = 12; align = 16; break;
		case Primitive_Bool4: typeNameCPU = StringView( "u32_v4" ); size = 16; align = 16; break;

		case Primitive_Int: typeNameCPU = StringView( "i32" ); size = 4; align = 4; break;
		case Primitive_Int2: typeNameCPU = StringView( "int_v2" ); size = 8; align = 8; break;
		case Primitive_Int3: typeNameCPU = StringView( "int_v3" ); size = 12; align = 16; break;
		case Primitive_Int4: typeNameCPU = StringView( "int_v4" ); size = 16; align = 16; break;

		case Primitive_UInt: typeNameCPU = StringView( "u32" ); size = 4; align = 4; break;
		case Primitive_UInt2: typeNameCPU = StringView( "u32_v2" ); size = 8; align = 8; break;
		case Primitive_UInt3: typeNameCPU = StringView( "u32_v3" ); size = 12; align = 16; break;
		case Primitive_UInt4: typeNameCPU = StringView( "u32_v4" ); size = 16; align = 16; break;

		case Primitive_Float: typeNameCPU = StringView( "float" ); size = 4; align = 4; break;
		case Primitive_Float2: typeNameCPU = StringView( "float_v2" ); size = 8; align = 8; break;
		case Primitive_Float3: typeNameCPU = StringView( "float_v3" ); size = 12; align = 16; break;
		case Primitive_Float4: typeNameCPU = StringView( "float_v4" ); size = 16; align = 16; break;

		case Primitive_Float2x2: typeNameCPU = StringView( "float_m44" ); size = 32; align = 16; break;
		case Primitive_Float3x3: typeNameCPU = StringView( "float_m44" ); size = 48; align = 16; break;
		case Primitive_Float4x4: typeNameCPU = StringView( "float_m44" ); size = 64; align = 16; break;

		default: typeNameCPU.data = nullptr; size = 0; align = 16; break;
	};

	// Struct Size
	if( type.tokenType == TokenType_SharedStruct ) { size = round_up16( type.sizeBytesPadded ); align = 16; }

	// Calculate required padding & increment byte offset
	structureByteOffset += Generator::append_structure_padding( output, indent, size, align, structureByteOffset );

	// Type
	if( variable.arrayLengthX > 0 && variable.arrayLengthY > 0 )
	{
		structureByteOffset += Generator::append_structure_padding( output, indent, 0, 16, structureByteOffset );

		snprintf( typeNameBuffer, sizeof( typeNameBuffer ), "metal_array_2d<%s%.*s, %d, %d, %d>",
			type.tokenType == TokenType_SharedStruct ? "GfxStructPadded::" : "",
			static_cast<int>( typeNameCPU.data == nullptr ? type.name.length : typeNameCPU.length ),
			typeNameCPU.data == nullptr ? type.name.data : typeNameCPU.data,
			align,
			variable.arrayLengthX, variable.arrayLengthY );

		typeSizeBytes = round_upAlign( size, align ) * ( variable.arrayLengthX * variable.arrayLengthY );

#if GFX_OUTPUT_STRUCTURE_SIZES
		output.append( indent ).append( "static_assert( sizeof( " ).append( typeNameBuffer );
		output.append( " ) == " ).append( typeSizeBytes ).append( ", \"size missmatch!\" );\n" );;
#endif

		// 2D Array
		output.append( indent ).append( typeNameBuffer );
	}
	else if( variable.arrayLengthX > 0 )
	{
		structureByteOffset += Generator::append_structure_padding( output, indent, 0, 16, structureByteOffset );

		snprintf( typeNameBuffer, sizeof( typeNameBuffer ), "metal_array_1d<%s%.*s, %d, %d>",
			type.tokenType == TokenType_SharedStruct ? "GfxStructPadded::" : "",
			static_cast<int>( typeNameCPU.data == nullptr ? type.name.length : typeNameCPU.length ),
			typeNameCPU.data == nullptr ? type.name.data : typeNameCPU.data,
			align,
			variable.arrayLengthX );

		typeSizeBytes = round_upAlign( size, align ) * ( variable.arrayLengthX );

#if GFX_OUTPUT_STRUCTURE_SIZES
		output.append( indent ).append( "static_assert( sizeof( " ).append( typeNameBuffer );
		output.append( " ) == " ).append( typeSizeBytes ).append( ", \"size missmatch!\" );\n" );
#endif

		// 1D Array
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

	// NOTE: Metal padding rules required vector3 types to be padded up to 16 bytes
	if( variable.arrayLengthX == 0 && variable.arrayLengthY == 0 )
	{
		switch( variable.typeID )
		{
			case Primitive_Bool3:
			case Primitive_Int3:
			case Primitive_UInt3:
			case Primitive_Float3:
				structureByteOffset +=
					Generator::append_structure_padding( output, indent, 4, 16, structureByteOffset );
			break;
		};
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GeneratorMetal::generate_stage( ShaderStage stage )
{
	entryParameters.clear();
	globalMembers.clear();
	globalConstructor.clear();
	globalHasSampler = false;

	// System-Value Semantics
	if( stage == ShaderStage_Vertex )
	{
		entryParameters.append( ",\n\tuint vertexID [[vertex_id]]" );
		globalMembers.append( "\tuint vertexID;\n" );
		globalConstructor.append( "vertexID" );

		entryParameters.append( ",\n\tuint instanceID [[instance_id]]" );
		globalMembers.append( "\tuint instanceID;\n" );
		globalConstructor.append( ", instanceID" );
	}
	else if( stage == ShaderStage_Fragment )
	{
		entryParameters.append( ",\n\tuint primitiveID [[primitive_id]]" );
		globalMembers.append( "\tuint primitiveID;\n" );
		globalConstructor.append( "primitiveID" );

		entryParameters.append( ",\n\tuint sampleID [[sample_id]]" );
		globalMembers.append( "\tuint sampleID;\n" );
		globalConstructor.append( ", sampleID" );

		entryParameters.append( ",\n\tbool isFrontFace [[front_facing]]" );
		globalMembers.append( "\tbool isFrontFace;\n" );
		globalConstructor.append( ", isFrontFace" );
	}
	else if( stage == ShaderStage_Compute )
	{
		entryParameters.append( "uint3 dispatchThreadID [[thread_position_in_grid]]" );
		globalMembers.append( "\tuint3 dispatchThreadID;\n" );
		globalConstructor.append( "dispatchThreadID" );

		entryParameters.append( ",\n\tuint3 groupThreadID [[thread_position_in_threadgroup]]" );
		globalMembers.append( "\tuint3 groupThreadID;\n" );
		globalConstructor.append( ", groupThreadID" );

		entryParameters.append( ",\n\tuint3 groupID [[threadgroup_position_in_grid]]" );
		globalMembers.append( "\tuint3 groupID;\n" );
		globalConstructor.append( ", groupID" );

		entryParameters.append( ",\n\tuint groupIndex [[thread_index_in_threadgroup]]" );
		globalMembers.append( "\tuint groupIndex;\n" );
		globalConstructor.append( ", groupIndex" );
	}

	Generator::generate_stage( stage );

	String header;
	header.append( "#include <metal_stdlib>\n" );
	header.append( "using namespace metal;\n\n" );
	header.append( "struct Global\n{\n" );
	header.append( globalMembers );
	header.append( "};\n\n" );
	output.insert( 0, header );
}


void GeneratorMetal::generate_statement_discard( NodeStatementDiscard *node )
{
	output.append( indent ).append( "discard_fragment();\n" );
}


void GeneratorMetal::generate_sv_semantic( NodeSVSemantic *node )
{
	switch( node->svSemanticType )
	{
		case SVSemanticType_VERTEX_ID: output.append( "global.vertexID" ); return;
		case SVSemanticType_INSTANCE_ID: output.append( "global.instanceID" ); return;
		case SVSemanticType_PRIMITIVE_ID: output.append( "global.primitiveID" ); return;
		case SVSemanticType_SAMPLE_ID: output.append( "global.sampleID" ); return;
		case SVSemanticType_FRONT_FACING: output.append( "global.isFrontFace" ); return;
		case SVSemanticType_DISPATCH_THREAD_ID: output.append( "global.dispatchThreadID" ); return;
		case SVSemanticType_GROUP_THREAD_ID: output.append( "global.groupThreadID" ); return;
		case SVSemanticType_GROUP_ID:  output.append( "global.groupID" ); return;
		case SVSemanticType_GROUP_INDEX: output.append( "global.groupIndex" ); return;
		default: Error( "Unexpected System-Value Semantic: %s", SVSemantics[node->svSemanticType] );
	}
}


void GeneratorMetal::generate_structure( NodeStruct *node )
{
	generate_structure_gfx( node );

	// Skip generating StructType_InstanceInput
	if( node->structType == StructType_InstanceInput ) { return; }

	// Append Member Variables Helper
	int slot = 0;
	auto append_structure_members = [&]( NodeStruct *node, const String &prefix ) -> void
	{
		const Struct &structure = parser.structs[node->structID];
		const Type &type = parser.types[structure.typeID];
		const String &typeName = type_name( structure.typeID );
		const VariableID first = type.memberFirst;
		const VariableID last = first + type.memberCount;

		const bool cpuStruct = ( node->structType == StructType_SharedStruct ||
			node->structType == StructType_UniformBuffer ||
			node->structType == StructType_ConstantBuffer ||
			node->structType == StructType_MutableBuffer );

		const bool expectSlot =  ( node->structType == StructType_UniformBuffer ||
			node->structType == StructType_ConstantBuffer ||
			node->structType == StructType_MutableBuffer );

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

			output.append( indent );

			// alignas( 16 )
			if( cpuStruct && ( memberVariable.arrayLengthX > 0 || memberVariable.arrayLengthY > 0 ) )
			{
				output.append( "alignas( 16 ) " );
			}

			// <type>
			if( cpuStruct )
			{
				switch( memberVariable.typeID )
				{
					case Primitive_Bool: output.append( "uint" ); break;
					case Primitive_Bool2: output.append( "uint2" ); break;
					case Primitive_Bool3: output.append( "uint3" ); break;
					case Primitive_Bool4: output.append( "uint4" ); break;
					default: output.append( memberTypeName ); break;
				}
			}
			else
			{
				output.append( memberTypeName );
			}
			output.append( " " );

			// <name>
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

			// [[<semantic>]]
			if( expectMeta )
			{
				switch( node->structType )
				{
					case StructType_VertexInput:
					case StructType_InstanceInput:
						output.append( " [[attribute(" ).append( slot ).append( ")]]" );
					break;

					case StructType_VertexOutput:
					case StructType_FragmentInput:
						if( memberVariable.semantic == SemanticType_POSITION )
						{
							output.append( " [[position]]" );
						}
					break;

					case StructType_FragmentOutput:
						if( memberVariable.semantic == SemanticType_COLOR )
						{
							output.append( " [[color(" ).append( memberVariable.slot ).append( ")]]" );
						}
						else if( memberVariable.semantic == SemanticType_DEPTH )
						{
							output.append( " [[depth(any)]]" );
						}
						else
						{
							Error( "Unexpected FragmentOutput type!" );
						}
					break;
				}
			}

			output.append( ";\n" );
			slot++;
		}
	};

	Struct &structure = parser.structs[node->structID];
	if( node->structType == StructType_SharedStruct ) { structure.alignment = 16; }
	const Type &type = parser.types[structure.typeID];
	const String &typeName = type_name( structure.typeID );

	// <name> <type>
	output.append( "struct " );
	if( structure.alignment > 0 )
	{
		output.append( "alignas( " ).append( structure.alignment ).append( " ) " );
	}
	output.append( typeName );

	// { <body> }
	output.append( "\n{\n" );
	indent_add();

	// Members
	append_structure_members( node, "" );

	// In Metal, instance_input members belong to the vertex format
	if( node->structType == StructType_VertexInput && shader.instanceFormatID != U32_MAX )
	{
		const InstanceFormat &instanceFormat = Gfx::instanceFormats[shader.instanceFormatID];
		if( instanceFormat.node != nullptr )
		{
			NodeStruct *nodeInstanceInput = reinterpret_cast<NodeStruct *>( instanceFormat.node );
			const String &instanceInputName = type_name( parser.structs[nodeInstanceInput->structID].typeID );
			append_structure_members( nodeInstanceInput, instanceInputName );
		}
	}

	indent_sub();
	output.append( "};\n\n" );
}


void GeneratorMetal::generate_function_declaration( NodeFunctionDeclaration *node )
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
		output.append( "( Global global" );
		VariableID parameterCount = 0;
		for( VariableID i = first; i < last; i++ )
		{
			if( i == first ) { output.append( "," ); }
			Variable &parameter = parser.variables[i];
			Type &parameterType = parser.types[parameter.typeID];

			output.append( parameterCount == 0 ? " " : ", " );
			const String &parameterTypeName = type_name( parameter.typeID );
			const String &parameterVariableName = variable_name( i );

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


void GeneratorMetal::generate_function_declaration_main_pipeline( NodeFunctionDeclaration *node )
{
	Function &function = parser.functions[node->functionID];
	Type &returnType = parser.types[function.typeID];

	// Enable Shader Stage
	switch( node->functionType )
	{
		case FunctionType_MainVertex: shader.stages |= Assets::ShaderStageFlag_Vertex; break;
		case FunctionType_MainFragment: shader.stages |= Assets::ShaderStageFlag_Fragment; break;
	}

	// In / Out
	Assert( function.parameterCount >= 2 );
	VariableID inID = function.parameterFirst;
	Type &inType = parser.types[parser.variables[inID].typeID];
	const String &inTypeName = type_name( parser.variables[inID].typeID );
	INSTANCE_INPUT_VARIABLE_SUBSTITUTION = variable_name( inID ); // instance_input -> vertex_input

	VariableID outID = function.parameterFirst + 1;
	Type &outType = parser.types[parser.variables[outID].typeID];
	const String &outTypeName = type_name( parser.variables[outID].typeID );

	const char *mainType = "";
	switch( node->functionType )
	{
		case FunctionType_MainVertex: mainType = "vertex"; break;
		case FunctionType_MainFragment: mainType = "fragment"; break;
		case FunctionType_MainCompute: mainType = "compute"; break;
	}

	const char *mainName = "";
	switch( node->functionType )
	{
		case FunctionType_MainVertex: mainName = "vs_main"; break;
		case FunctionType_MainFragment: mainName = "fs_main"; break;
		case FunctionType_MainCompute: mainName = "cs_main"; break;
	}

	const VariableID first = function.parameterFirst;
	const VariableID last = first + function.parameterCount;

	// Implementation
	output.append( indent ).append( "void " ).append( mainName ).append( "_impl( " );
	output.append( "Global global, " );
	output.append( inTypeName ).append( " " ).append( variable_name( inID ) );
	output.append( ", thread " ).append( outTypeName ).append( " &" ).append( variable_name( outID ) );
	for( VariableID i = first + 2; i < last; i++ )
	{
		const Variable &parameter = parser.variables[i];
		const Type &parameterType = parser.types[parameter.typeID];
		if( parameterType.tokenType == TokenType_UniformBuffer ||
			parameterType.tokenType == TokenType_ConstantBuffer ||
			parameterType.tokenType == TokenType_MutableBuffer )
		{
			output.append( ", " ).append( type_name( parameter.typeID ) );
			output.append( " " ).append( variable_name( i ) );
		}
	}
	output.append( " )\n" );
	output.append( indent ).append( "{\n" );
	generate_statement_block_no_braces( reinterpret_cast<NodeStatementBlock *>( node->block ) );
	output.append( indent ).append( "}\n\n" );

	// Main
	output.append( indent ).append( mainType ).append( " " );
	output.append( outTypeName ).append( " " ).append( mainName ).append( "( " );
	output.append( inTypeName ).append( " " ).append( variable_name( inID ) ).append( " [[stage_in]]" );
	output.append( entryParameters );
	for( VariableID i = first + 2; i < last; i++ )
	{
		const Variable &parameter = parser.variables[i];
		const Type &parameterType = parser.types[parameter.typeID];
		switch( parameterType.tokenType )
		{
			case TokenType_InstanceInput:
				continue; // Skip instance buffers
			break;

			case TokenType_UniformBuffer:
				output.append( ",\n\t" );
				output.append( "constant " );
				output.append( type_name( parameter.typeID ) ).append( " &" ).append( variable_name( i ) );
				output.append( " [[buffer(" ).append( parameterType.slot + 2 ).append( ")]]" );
			break;

			case TokenType_ConstantBuffer:
				output.append( ",\n\t" );
				output.append( "constant " );
				output.append( type_name( parameter.typeID ) ).append( " &" ).append( variable_name( i ) );
				output.append( " [[buffer(" ).append( parameter.slot + 2 ).append( ")]]" );
			break;

			case TokenType_MutableBuffer:
				output.append( ",\n\t" );
				output.append( type_name( parameter.typeID ) ).append( " &" ).append( variable_name( i ) );
				output.append( " [[buffer(" ).append( parameter.slot + 2 ).append( ")]]" );
			break;
		}
	}
	output.append( " )\n" );

	output.append( indent ).append( "{\n" );
	indent_add();
	{
		output.append( indent ).append( outTypeName ).append( " " );
		output.append( variable_name( outID ) ).append( ";\n" );

		output.append( indent ).append( "Global global = { " );
		output.append( globalConstructor ).append( " };\n" );

		output.append( indent ).append( mainName ).append( "_impl( " );
		output.append( "global, " );
		output.append( variable_name( inID ) ).append( ", " ).append( variable_name( outID ) );
		for( VariableID i = first + 2; i < last; i++ )
		{
			const Variable &parameter = parser.variables[i];
			const Type &parameterType = parser.types[parameter.typeID];
			if( parameterType.tokenType == TokenType_UniformBuffer ||
				parameterType.tokenType == TokenType_ConstantBuffer ||
				parameterType.tokenType == TokenType_MutableBuffer )
			{
				output.append( ", " ).append( variable_name( i ) );
			}
		}
		output.append( " );\n" );

		output.append( indent ).append( "return " ).append( variable_name( outID ) ).append( ";\n" );
	}
	indent_sub();
	output.append( indent ).append( "}\n\n" );
}


void GeneratorMetal::generate_function_declaration_main_compute( NodeFunctionDeclaration *node )
{
	Function &function = parser.functions[node->functionID];
	Type &returnType = parser.types[function.typeID];

	// Enable Shader Stage
	shader.stages |= Assets::ShaderStageFlag_Compute;

	// Thread Groups
	output.append( indent ).append( "[numthreads( " );
	output.append( parser.threadGroupX ).append( ", " );
	output.append( parser.threadGroupY ).append( ", " );
	output.append( parser.threadGroupZ ).append( " )]\n" );

	// Return Type & Name
	output.append( indent ).append( "compute void cs_main( " );
	output.append( entryParameters );
	output.append( " )\n" );

	generate_statement_block_main( reinterpret_cast<NodeStatementBlock *>( node->block ) );
	output.append( "\n" );
}


void GeneratorMetal::generate_statement_block_main( NodeStatementBlock *node )
{
	// ...
}


void GeneratorMetal::generate_expression_binary_dot( NodeExpressionBinary *node )
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
			generate_node( node->expr1 );
			output.append( "." );
			generate_node( node->expr2 );
			return;
		}
		else if( type.tokenType == TokenType_InstanceInput )
		{
			ErrorIf( stage != ShaderStage_Vertex,
				"TokenType_InstanceInput encounted in wrong stage: %d", stage );

			// In Metal, instance data is part of the vertex format vertex format: <Buffer>_<Variable>
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


bool GeneratorMetal::generate_structure_gfx_vertex( NodeStruct *node )
{
	// Register this vertex format in the system
	if( !Generator::generate_structure_gfx_vertex( node ) )
	{
		return false; // Already registered -- no need to continue
	}

	// Generate Metal Vertex Format interfaces
	const Struct &structure = parser.structs[node->structID];
	const Type &type = parser.types[structure.typeID];
	const VariableID first = type.memberFirst;
	const VariableID last = first + type.memberCount;

	int byteOffset = 0;
	String attributes;

	const usize count = last - first;
	if( count > 0 )
	{
		attributes.append( "constexpr MetalInputLayoutAttributes metalInputAttributesVertex_" );
		attributes.append( type.name ).append( "[" ).append( count ).append("]  =\n{\n" );

		for( usize i = first; i < last; i++ )
		{
			const Variable &memberVariable = parser.variables[i];
			const MetalInputFormatInfo *const inputFormat = metal_input_format( memberVariable.typeID,
				memberVariable.format );
			ErrorReturnIf( inputFormat == nullptr, false, "Unexpected vertex input format!" );

			attributes.append( "\t" "{ " ).append( byteOffset ).append( ", " );
			attributes.append( inputFormat->format ).append( " },\n" );
			byteOffset += inputFormat->size;
		}

		attributes.append( "};\n\n" );
	}
	else
	{
		attributes.append( "constexpr MetalInputLayoutAttributes *metalInputAttributesVertex_" );
		attributes.append( type.name ).append( " = nullptr;\n\n" );
	}

	attributes.append( "constexpr MetalInputLayoutFormats inputLayoutFormatVertex_" );
	attributes.append( type.name ).append( " =\n{\n\tmetalInputAttributesVertex_" );
	attributes.append( type.name ).append( ", " ).append( count ).append( ", " ).append( byteOffset );
	attributes.append( "\n};\n\n" );

	// Write to gfx.api.generated.hpp
	shader.header.append( attributes );

	return true;
}


bool GeneratorMetal::generate_structure_gfx_instance( NodeStruct *node )
{
	// Register this vertex format in the system
	if( !Generator::generate_structure_gfx_instance( node ) )
	{
		return false; // Already registered -- no need to continue
	}

	// Generate Metal Vertex Format interfaces
	const Struct &structure = parser.structs[node->structID];
	const Type &type = parser.types[structure.typeID];
	const VariableID first = type.memberFirst;
	const VariableID last = first + type.memberCount;

	int byteOffset = 0;
	String attributes;

	const usize count = last - first;
	if( count > 0 )
	{
		attributes.append( "constexpr MetalInputLayoutAttributes metalInputAttributesInstance_" );
		attributes.append( type.name ).append( "[" ).append( count ).append("]  =\n{\n" );

		for( usize i = first; i < last; i++ )
		{
			const Variable &memberVariable = parser.variables[i];
			const MetalInputFormatInfo *const inputFormat = metal_input_format( memberVariable.typeID,
				memberVariable.format );
			ErrorReturnIf( inputFormat == nullptr, false, "Unexpected instance input format!" );

			attributes.append( "\t" "{ " ).append( byteOffset ).append( ", " );
			attributes.append( inputFormat->format ).append( " },\n" );
			byteOffset += inputFormat->size;
		}

		attributes.append( "};\n\n" );
	}
	else
	{
		attributes.append( "constexpr MetalInputLayoutAttributes *metalInputAttributesInstance_" );
		attributes.append( type.name ).append( " = nullptr;\n\n" );
	}

	attributes.append( "constexpr MetalInputLayoutFormats inputLayoutFormatInstance_" );
	attributes.append( type.name ).append( " =\n{\n\tmetalInputAttributesInstance_" );
	attributes.append( type.name ).append( ", " ).append( count ).append( ", " ).append( byteOffset );
	attributes.append( "\n};\n\n" );

	// Write to gfx.api.generated.hpp
	shader.header.append( attributes );

	return true;
}


void GeneratorMetal::generate_function_call_parameters( NodeFunctionCall *node )
{
	output.append( "(" );

	// Metal: All custom functions must pass Global
	const bool isIntrinsic = node->functionID < INTRINSIC_COUNT;
	if( !isIntrinsic ) { output.append( " global" ); }

	if( node->param != nullptr )
	{
		output.append( isIntrinsic ? "" : "," );
		Node *param = node->param;
		usize parameterCount = 0;
		while( param != nullptr )
		{
			NodeExpressionList *paramNode = reinterpret_cast<NodeExpressionList *>( param );
			output.append( parameterCount == 0 ? " " : ", " );
			generate_node( paramNode->expr );
			parameterCount++;

			param = paramNode->next;
		}
	}

	output.append( " )" );
}


void GeneratorMetal::generate_function_call_intrinsics( NodeFunctionCall *node )
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
		case Intrinsic_Mod:
		{
			output.append( "fmod" );
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
				"dFdx", // Intrinsic_DDx
				"dFdy", // Intrinsic_DDy
				// NOTE: Metal has no fine/coarse derivatives, all map to dFdx/dFdy
				"dFdx", // Intrinsic_DDxCoarse
				"dFdx", // Intrinsic_DDxFine
				"dFdy", // Intrinsic_DDyCoarse
				"dFdy", // Intrinsic_DDyFine
			};
			static_assert( ARRAY_LENGTH( ddOperations ) == ( Intrinsic_DDyFine - Intrinsic_DDx + 1 ),
				"Missing Intrinsic!" );

			output.append( ddOperations[intrinsic - Intrinsic_DDx] );
			generate_function_call_parameters( node );
		}
		return;

		case Intrinsic_Abs:
		case Intrinsic_Pow:
		case Intrinsic_Sqrt:
		case Intrinsic_RSqrt:
		case Intrinsic_Clamp:
		case Intrinsic_Max:
		case Intrinsic_Min:
		case Intrinsic_Ldexp:
		case Intrinsic_Fma:
		case Intrinsic_Sign:
		{
			generate_function_call( node );
		}
		return;

	// Vector and Matrix Functions
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
				"popcount",                    // Intrinsic_BitCount
				"firstbithigh",                // Intrinsic_BitFirstHigh
				"firstbitlow",                 // Intrinsic_BitFirstLow
				"reversebits",                 // Intrinsic_BitReverse
				// TODO: Support these in Metal
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
				"as_type<int>",    // Intrinsic_FloatToIntBits
				"as_type<uint>",   // Intrinsic_FloatToUIntBits
				"as_type<float>",  // Intrinsic_IntToFloatBits
				"as_type<float>",  // Intrinsic_UIntToFloatBits
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
		case Intrinsic_TextureSample3D:
		case Intrinsic_TextureSample3DArray:
		case Intrinsic_TextureSampleCube:
		case Intrinsic_TextureSampleCubeArray:
		{
			// <texture>.sample( sampler, location );
			output.append( "global." );
			generate_node( get_param( node->param, 0 )->expr );
			output.append( ".sample( " );
			output.append( "global.GlobalSampler, " );
			generate_node( get_param( node->param, 1 )->expr );
			output.append( " )" );
		}
		return;

		case Intrinsic_TextureSample2DArray:
		{
			// <texture>.sample( sampler, location );
			output.append( "global." );
			generate_node( get_param( node->param, 0 )->expr );
			output.append( ".sample( " );
			output.append( "global.GlobalSampler, " );
			generate_node( get_param( node->param, 1 )->expr );
			output.append( ", " );
			generate_node( get_param( node->param, 2 )->expr );
			output.append( " )" );
		}
		return;

		case Intrinsic_TextureSample1DLevel:
		case Intrinsic_TextureSample2DLevel:
		case Intrinsic_TextureSample3DLevel:
		case Intrinsic_TextureSampleCubeLevel:
		{
			// <texture>.SampleLevel( sampler, location, lod );
			output.append( "global." );
			generate_node( get_param( node->param, 0 )->expr );
			output.append( ".sample( " );
			output.append( "global.GlobalSampler, " );
			generate_node( get_param( node->param, 1 )->expr );
			output.append( ", level( " );
			generate_node( get_param( node->param, 2 )->expr );
			output.append( ") )" );
		}
		return;

		case Intrinsic_TextureLoad1D:
		{
			// <texture>.Load( float2( location.x, lod ) );
			output.append( "global." );
			generate_node( get_param( node->param, 0 )->expr );
			output.append( ".read( ushort( " );
			generate_node( get_param( node->param, 1 )->expr ); // u
			output.append( " ), " );
			generate_node( get_param( node->param, 3 )->expr ); // lod
			output.append( " )" );
		}
		return;

		case Intrinsic_TextureLoad2D:
		{
			// <texture>.Load( float3( location.xy, lod ) );
			output.append( "global." );
			generate_node( get_param( node->param, 0 )->expr );
			output.append( ".read( ushort2( " );
			generate_node( get_param( node->param, 1 )->expr ); // u
			output.append( ", " );
			generate_node( get_param( node->param, 2 )->expr ); // v
			output.append( " ), " );
			generate_node( get_param( node->param, 5 )->expr ); // lod
			output.append( " )" );
		}
		return;

		case Intrinsic_TextureLoad3D:
		{
			// <texture>.Load( float4( location.xyz, lod ) );
			output.append( "global." );
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


void GeneratorMetal::generate_texture( NodeTexture *node )
{
	// TODO: Multiple sampler states for each texture (like OpenGL)
	Texture &texture = parser.textures[node->textureID];
	const String &typeName = type_name( parser.variables[texture.variableID].typeID );
	const String &variableName = variable_name( texture.variableID );

	const char *channelType = "float";
	switch( parser.variables[texture.variableID].typeID )
	{
		case Primitive_Bool:
		case Primitive_Bool2:
		case Primitive_Bool3:
		case Primitive_Bool4:
			channelType = "bool";
		break;

		case Primitive_UInt:
		case Primitive_UInt2:
		case Primitive_UInt3:
		case Primitive_UInt4:
			channelType = "uint";
		break;

		case Primitive_Int:
		case Primitive_Int2:
		case Primitive_Int3:
		case Primitive_Int4:
			channelType = "int";
		break;

		case Primitive_Float:
		case Primitive_Float2:
		case Primitive_Float3:
		case Primitive_Float4:
			channelType = "float";
		break;

		default:
			Error( "Unexpected texture channel type!" );
		break;
	}

	static const char *textureNames[] =
	{
		"texture2d",          // TextureType_Texture2D
		"texture2d_array",    // TextureType_Texture2DArray
		"texture3d",          // TextureType_Texture3D
		"texture_cube",       // TextureType_TextureCube
		"texture_cube_crray", // TextureType_TextureCubeArray
	};
	static_assert( ARRAY_LENGTH( textureNames ) == TEXTURETYPE_COUNT, "Missing TextureType" );

	if( !globalHasSampler )
	{
		globalHasSampler = true;
		entryParameters.append( ",\n\tsampler GlobalSampler [[sampler(0)]]" );
		globalMembers.append( "\tsampler GlobalSampler;\n" );
		globalConstructor.append( ", GlobalSampler" );
	}

	entryParameters.append( ",\n\t" );
	entryParameters.append( textureNames[node->textureType] );
	entryParameters.append( "<" ).append( channelType ).append( "> " );
	entryParameters.append( variableName );
	entryParameters.append( " [[texture(" ).append( texture.slot ).append( ")]]" );

	globalMembers.append( "\t" );
	globalMembers.append( textureNames[node->textureType] );
	globalMembers.append( "<" ).append( channelType ).append( "> " );
	globalMembers.append( variableName ).append( ";\n" );

	globalConstructor.append( ", " ).append( variableName );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}