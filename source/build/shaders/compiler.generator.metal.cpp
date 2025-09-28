#include <build/shaders/compiler.generator.hpp>

#include <vendor/stdio.hpp>
#include <core/string.hpp>

namespace ShaderCompiler
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GeneratorMetal::generate_structure( NodeStruct *node )
{
	Struct &structure = parser.structs[node->structID];
	Type &type = parser.types[structure.typeID];
	const String &typeName = type_name( structure.typeID );
	const VariableID first = type.memberFirst;
	const VariableID last = first + type.memberCount;

	//
	static const char *structNames[] =
	{
		"struct",          // StructType_Struct
		"struct",          // StructType_SharedStruct
		"uniform_buffer",  // StructType_UniformBuffer
		"constant_buffer", // StructType_ConstantBuffer
		"mutable_buffer",  // StructType_MutuableBuffer
		"struct",          // StructType_InstanceInput
		"struct",          // StructType_VertexInput
		"struct",          // StructType_VertexOutput
		"struct",          // StructType_FragmentInput
		"struct",          // StructType_FragmentOutput
		"struct",          // StructType_ComputeInput
		"struct",          // StructType_ComputeOutput
	};
	static_assert( ARRAY_LENGTH( structNames ) == STRUCTTYPE_COUNT, "Missing TextureType" );

	const bool expectSlot =  ( node->structType == StructType_UniformBuffer ||
		node->structType == StructType_ConstantBuffer || node->structType == StructType_MutableBuffer );

	const bool expectMeta = !( node->structType == StructType_UniformBuffer ||
		node->structType == StructType_ConstantBuffer || node->structType == StructType_MutableBuffer ||
		node->structType == StructType_Struct || node->structType == StructType_SharedStruct );

	// <name> <type>
	output.append( structNames[node->structType] ).append( " " ).append( typeName );

	// : <register>
	if( expectSlot )
	{
		output.append( " : register( b" ).append( structure.slot ).append( " )" );
	}

	// { <body> }
	output.append( "\n{\n" );
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
		if( node->structType == StructType_UniformBuffer ||
			node->structType == StructType_ConstantBuffer ||
			node->structType == StructType_MutableBuffer )
		{
			// buffer members are in global namespace, so we prefix them with the structure name
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
			const char *semantic = "";
			switch( memberVariable.semantic )
			{
				case SemanticType_POSITION: semantic = "POSITION"; break;
				case SemanticType_TEXCOORD: semantic = "TEXCOORD"; break;
				case SemanticType_NORMAL: semantic = "NORMAL"; break;
				case SemanticType_DEPTH: semantic = "DEPTH"; break;
				case SemanticType_COLOR: semantic = "COLOR"; break;
				case SemanticType_BINORMAL: semantic = "BINORMAL"; break;
				case SemanticType_TANGENT: semantic = "TANGENT"; break;
				case SemanticType_INSTANCE: semantic = "INSTANCE"; break;
				default: Error( "Unexpected semantic type: %u", memberVariable.semantic ); break;
			};
			output.append( " : " ).append( semantic );
		}

		output.append( ";\n" );
	}
	indent_sub();
	output.append( "};\n\n" );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}