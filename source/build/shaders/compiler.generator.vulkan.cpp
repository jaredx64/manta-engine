#include <build/shaders/compiler.generator.hpp>

#include <vendor/stdio.hpp>
#include <core/string.hpp>

namespace ShaderCompiler
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GeneratorVulkan::generate_structure( NodeStruct *node )
{
	Struct &structure = parser.structs[node->structID];
	Type &type = parser.types[structure.typeID];
	const String &typeName = type_name( structure.typeID );
	const VariableID first = type.memberFirst;
	const VariableID last = first + type.memberCount;

	//
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
			const char *semantic = "";
			switch( memberVariable.semantic )
			{
				case TokenType_POSITION: semantic = "POSITION"; break;
				case TokenType_TEXCOORD: semantic = "TEXCOORD"; break;
				case TokenType_NORMAL:   semantic = "NORMAL"; break;
				case TokenType_DEPTH:    semantic = "DEPTH"; break;
				case TokenType_COLOR:    semantic = "COLOR"; break;
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