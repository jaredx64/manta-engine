#include <build/shaders/compiler.generator.hpp>

#include <core/string.hpp>
#include <core/checksum.hpp>

#include <vendor/stdio.hpp>

namespace ShaderCompiler
{
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Generator::generate_stage( ShaderStage stage )
{
	// Reset Output
	indent[indentLevel] = '\0';

	// Symbols
	process_names();

	// Generate
	for( Node *node : parser.program )
	{
		// Skip "unseen" nodes (dead-code elimination)
		if( !parser.node_seen( node ) ) { continue; }

		// Generate Node
		generate_node( node );
	}

	// Resolve Vertex Format
	shader.vertexFormatID = U32_MAX;
	if( !parser.vertexFormatTypeName.is_empty() )
	{
		for( VertexFormat format : Gfx::vertexFormats )
		{
			if( format.name.equals( parser.vertexFormatTypeName ) )
			{
				shader.vertexFormatID = format.id;
				break;
			}
		}
	}

	// Resolve Instance Format
	shader.instanceFormatID = U32_MAX;
	if( !parser.instanceFormatTypeName.is_empty() )
	{
		for( InstanceFormat format : Gfx::instanceFormats )
		{
			if( format.name.equals( parser.instanceFormatTypeName ) )
			{
				shader.instanceFormatID = format.id;
				break;
			}
		}
	}
}


void Generator::generate_node( Node *node )
{
	switch( node->nodeType )
	{
		case NodeType_Statement:
		{
			generate_statement( reinterpret_cast<NodeStatement *>( node ) );
		}
		break;

		case NodeType_ExpressionUnary:
		{
			generate_expression_unary( reinterpret_cast<NodeExpressionUnary *>( node ) );
		}
		break;

		case NodeType_ExpressionBinary:
		{
			generate_expression_binary( reinterpret_cast<NodeExpressionBinary *>( node ) );
		}
		break;

		case NodeType_ExpressionTernary:
		{
			generate_expression_ternary(reinterpret_cast<NodeExpressionTernary *>( node ) );
		}
		break;

		case NodeType_FunctionDeclaration:
		{
			generate_function_declaration( reinterpret_cast<NodeFunctionDeclaration *>( node ) );
		}
		break;

		case NodeType_FunctionCall:
		{
			if( reinterpret_cast<NodeFunctionCall *>( node )->functionID < INTRINSIC_COUNT )
			{
				// Built-in Functions
				generate_function_call_intrinsics( reinterpret_cast<NodeFunctionCall *>( node ) );
			}
			else
			{
				// Custom Functions
				generate_function_call( reinterpret_cast<NodeFunctionCall *>( node ) );
			}
		}
		break;

		case NodeType_Cast:
		{
			generate_cast( reinterpret_cast<NodeCast *>( node ) );
		}
		break;

		case NodeType_VariableDeclaration:
		{
			generate_variable_declaration( reinterpret_cast<NodeVariableDeclaration *>( node ) );
		}
		break;

		case NodeType_Variable:
		{
			generate_variable( reinterpret_cast<NodeVariable *>( node ) );
		}
		break;

		case NodeType_Swizzle:
		{
			generate_swizzle( reinterpret_cast<NodeSwizzle *>( node ) );
		}
		break;

		case NodeType_SVSemantic:
		{
			generate_sv_semantic( reinterpret_cast<NodeSVSemantic *>( node ) );
		}
		break;

		case NodeType_Group:
		{
			generate_group( reinterpret_cast<NodeGroup *>( node ) );
		}
		break;

		case NodeType_Integer:
		{
			generate_integer( reinterpret_cast<NodeInteger *>( node ) );
		}
		break;

		case NodeType_Number:
		{
			generate_number( reinterpret_cast<NodeNumber *>( node ) );
		}
		break;

		case NodeType_Boolean:
		{
			generate_boolean( reinterpret_cast<NodeBoolean *>( node ) );
		}
		break;

		case NodeType_Struct:
		{
			generate_structure( reinterpret_cast<NodeStruct *>( node ) );
		}
		break;

		case NodeType_Texture:
		{
			generate_texture( reinterpret_cast<NodeTexture *>( node ) );
		}
		break;

		default:
			Error( "%s: unexpected NodeType! %u", __FUNCTION__, node->nodeType );
		break;
	}
}


void Generator::generate_node_parenthesis( Node *node )
{
	output.append( "( " );
	generate_node( node );
	output.append( " )" );
}


void Generator::generate_statement( NodeStatement *node )
{
	switch( node->statementType )
	{
		case StatementType_Block:
		{
			generate_statement_block( reinterpret_cast<NodeStatementBlock *>( node ) );
		}
		break;

		case StatementType_Expression:
		{
			generate_statement_expression( reinterpret_cast<NodeStatementExpression *>( node ) );
		}
		break;

		case StatementType_If:
		{
			leading_newline();
			generate_statement_if( reinterpret_cast<NodeStatementIf *>( node ) );
			trailing_newline();
		}
		break;

		case StatementType_While:
		{
			leading_newline();
			generate_statement_while( reinterpret_cast<NodeStatementWhile *>( node ) );
			trailing_newline();
		}
		break;

		case StatementType_DoWhile:
		{
			leading_newline();
			generate_statement_do_while( reinterpret_cast<NodeStatementDoWhile *>( node ) );
			trailing_newline();
		}
		break;

		case StatementType_For:
		{
			leading_newline();
			generate_statement_for( reinterpret_cast<NodeStatementFor *>( node ) );
			trailing_newline();
		}
		break;

		case StatementType_Switch:
		{
			leading_newline();
			generate_statement_switch( reinterpret_cast<NodeStatementSwitch *>( node ) );
			trailing_newline();
		}
		break;

		case StatementType_Case:
		{
			generate_statement_case( reinterpret_cast<NodeStatementCase *>( node ) );
		}
		break;

		case StatementType_Default:
		{
			generate_statement_default( reinterpret_cast<NodeStatementDefault *>( node ) );
		}
		break;

		case StatementType_Return:
		{
			generate_statement_return( reinterpret_cast<NodeStatementReturn *>( node ) );
		}
		break;

		case StatementType_Break:
		{
			generate_statement_break( reinterpret_cast<NodeStatementBreak *>( node ) );
		}
		break;

		case StatementType_Continue:
		{
			generate_statement_continue( reinterpret_cast<NodeStatementContinue *>( node ) );
		}
		break;

		case StatementType_Discard:
		{
			generate_statement_discard( reinterpret_cast<NodeStatementDiscard *>( node ) );
		}
		break;
	}
}


void Generator::generate_statement_block_no_braces( NodeStatementBlock *node )
{
	indent_add();

	while( node != nullptr )
	{
		if( node->expr != nullptr ) { generate_node( node->expr ); }
		node = reinterpret_cast<NodeStatementBlock *>( node->next );
	}

	indent_sub();
}


void Generator::generate_statement_block( NodeStatementBlock *node )
{
	output.append( indent ).append( "{\n" );
	generate_statement_block_no_braces( node );
	output.append( indent ).append( "}\n" );
}


void Generator::generate_statement_expression( NodeStatementExpression *node )
{
	output.append( indent );
	generate_node( node->expr );
	output.append( ";\n" );
}


void Generator::generate_statement_if( NodeStatementIf *node )
{
	// if( ... ) { }
	output.append( indent ).append( "if( ");
	generate_node( node->expr );
	output.append( " )\n" );
	generate_node( node->blockIf );

	// else ...
	if( node->blockElse != nullptr )
	{
		output.append( indent ).append( "else\n" );
		generate_node( node->blockElse );
	}
}


void Generator::generate_statement_while( NodeStatementWhile *node )
{
	output.append( indent ).append( "while( ");
	generate_node( node->expr );
	output.append( " )\n" );

	if( node->block != nullptr )
	{
		generate_node( node->block );
	}
}


void Generator::generate_statement_do_while( NodeStatementDoWhile *node )
{
	output.append( indent ).append( "do\n" );

	if( node->block != nullptr )
	{
		generate_node( node->block );
	}

	output.append( indent ).append( "while( " );
	generate_node( node->expr );
	output.append( " );\n " );
}


void Generator::generate_statement_for( NodeStatementFor *node )
{
	output.append( indent ).append( "for( " );
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


void Generator::generate_statement_switch( NodeStatementSwitch *node )
{
	output.append( indent ).append( "switch( " );
	generate_node( node->expr );
	output.append( " )\n" );

	if( node->block != nullptr )
	{
		generate_node( node->block );
	}
}


void Generator::generate_statement_case( NodeStatementCase *node )
{
	output.append( indent ).append( "case " );
	generate_node( node->expr );
	output.append( ":\n" );

	if( node->block != nullptr )
	{
		generate_node( node->block );
	}
}


void Generator::generate_statement_default( NodeStatementDefault *node )
{
	output.append( indent ).append( "default:\n" );

	if( node->block != nullptr )
	{
		generate_node( node->block );
	}
}


void Generator::generate_statement_return( NodeStatementReturn *node )
{
	output.append( indent ).append( "return" );

	if( node->expr != nullptr )
	{
		output.append( " " );
		generate_node( node->expr );
	}

	output.append( ";\n" );
}


void Generator::generate_statement_break( NodeStatementBreak *node )
{
	output.append( indent ).append( "break;\n" );
}


void Generator::generate_statement_continue( NodeStatementContinue *node )
{
	output.append( indent ).append( "continue;\n" );
}


void Generator::generate_statement_discard( NodeStatementDiscard *node )
{
	output.append( indent ).append( "discard;\n" );
}


void Generator::generate_expression_unary( NodeExpressionUnary *node )
{
	switch( node->exprType )
	{
		// Prefix Operators
		case ExpressionUnaryType_PreDecrement:
		{
			output.append( "--" );
			generate_node( node->expr );
		}
		break;

		case ExpressionUnaryType_PreIncrement:
		{
			output.append( "++" );
			generate_node( node->expr );
		}
		break;

		case ExpressionUnaryType_Plus:
		{
			generate_node( node->expr );
		}
		break;

		case ExpressionUnaryType_Minus:
		{
			output.append( "-" );
			generate_node( node->expr );
		}
		break;

		case ExpressionUnaryType_Not:
		{
			output.append( "!" );
			generate_node( node->expr );
		}
		break;

		case ExpressionUnaryType_BitNot:
		{
			output.append( "~" );
			generate_node( node->expr );
		}
		break;

		// Suffix Operators
		case ExpressionUnaryType_PostDecrement:
		{
			generate_node( node->expr );
			output.append( "--" );
		}
		break;

		case ExpressionUnaryType_PostIncrement:
		{
			generate_node( node->expr );
			output.append( "++" );
		}
		break;
	}

}


void Generator::generate_expression_binary( NodeExpressionBinary *node )
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
			generate_expression_binary_dot( node );
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


void Generator::generate_expression_binary_dot( NodeExpressionBinary *node )
{
	Assert( node->exprType == ExpressionBinaryType_Dot );

	generate_node( node->expr1 );
	output.append( "." );
	generate_node( node->expr2 );
}


void Generator::generate_expression_ternary( NodeExpressionTernary *node )
{
	output.append( "( ( " );
	generate_node( node->expr1 );
	output.append( " ) ? ( " );
	generate_node( node->expr2 );
	output.append( " ) : ( " );
	generate_node( node->expr3 );
	output.append( " ) )" );
}


void Generator::generate_function_declaration( NodeFunctionDeclaration *node )
{
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


void Generator::generate_function_declaration_main_pipeline( NodeFunctionDeclaration *node )
{
	// Do nothing
}


void Generator::generate_function_declaration_main_compute( NodeFunctionDeclaration *node )
{
	// Do nothing
}


void Generator::generate_function_call( NodeFunctionCall *node )
{
	// Name
	Function &function = parser.functions[node->functionID];
	const String &functionName = function_name( node->functionID );
	output.append( functionName );

	// Parameters
	generate_function_call_parameters( node );
}


void Generator::generate_function_call_parameters( NodeFunctionCall *node )
{
	output.append( "(" );

	if( node->param != nullptr )
	{
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
		output.append( parameterCount > 0 ? " " : "" );
	}

	output.append( ")" );
}


void Generator::generate_function_call_intrinsics( NodeFunctionCall *node )
{
	Intrinsic intrinsic = node->functionID;
	Assert( intrinsic < INTRINSIC_COUNT );

	generate_function_call( node );
}


void Generator::generate_cast( NodeCast *node )
{
	Type &type = parser.types[node->typeID];
	const String &typeName = type_name( node->typeID );

	output.append( typeName );

	if( node->param != nullptr )
	{
		output.append( "(" );
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
		output.append( parameterCount > 0 ? " )" : ")" );
	}
	else
	{
		output.append( "()" );
	}
}


void Generator::generate_variable_declaration( NodeVariableDeclaration *node )
{
	Variable &variable = parser.variables[node->variableID];
	const String &variableName = variable_name( node->variableID );

	Type &type = parser.types[variable.typeID];
	const String &typeName = type_name( variable.typeID );

	// <in> / <out> / <inout>
	if(  variable.in && !variable.out ) { output.append( "in " ); } else
	if( !variable.in &&  variable.out ) { output.append( "out " ); } else
	if(  variable.in &&  variable.out ) { output.append( "inout " ); }

	// <const>
	if( variable.constant ) { output.append( "const " ); }

	// <type> <variable>
	output.append( typeName ).append( " " ).append( variableName );

	// Array X/Y
	if( variable.arrayLengthX != 0 ) { output.append( "[" ).append( variable.arrayLengthX ).append( "]" ); }
	if( variable.arrayLengthY != 0 ) { output.append( "[" ).append( variable.arrayLengthY ).append( "]" ); }

	// Assignment
	if( node->assignment != nullptr )
	{
		output.append( " = " );
		generate_node( node->assignment );
	}
}


void Generator::generate_variable( NodeVariable *node )
{
	const String &variableName = variable_name( node->variableID );
	output.append( variableName );
}


void Generator::generate_swizzle( NodeSwizzle *node )
{
	output.append( SwizzleTypeNames[node->swizzleID] );
}


void Generator::generate_sv_semantic( NodeSVSemantic *node )
{
	output.append( SVSemantics[node->svSemanticType] );
}


void Generator::generate_group( NodeGroup *node )
{
	switch( node->expr->nodeType )
	{
		case NodeType_Variable:
		case NodeType_Integer:
		case NodeType_Number:
		case NodeType_Boolean:
		case NodeType_FunctionCall:
		case NodeType_Cast:
		{
			generate_node( node->expr );
		}
		break;

		default:
		{
			output.append( "( " );
			generate_node( node->expr );
			output.append( " )" );
		}
		break;
	}
}


void Generator::generate_integer( NodeInteger *node )
{
	output.append( node->integer );
}


void Generator::generate_number( NodeNumber *node )
{
	output.append( node->number );
}


void Generator::generate_boolean( NodeBoolean *node )
{
	output.append( node->boolean ? "true" : "false" );
}


void Generator::generate_structure( NodeStruct *node )
{
	Struct &structure = parser.structs[node->structID];

	generate_structure_gfx( node );

	Type &type = parser.types[structure.typeID];
	const String &typeName = type_name( structure.typeID );
	const VariableID first = type.memberFirst;
	const VariableID last = first + type.memberCount;

	static const char *structNames[] =
	{
		"struct",          // StructType_Struct
		"shared_struct",   // StructType_SharedStruct
		"uniform_buffer",  // StructType_UniformBuffer
		"constant_buffer", // StructType_ConstantBuffer
		"mutable_buffer",  // StructType_MutableBuffer
		"instance_input",  // StructType_InstanceInput
		"vertex_input",    // StructType_VertexInput
		"vertex_output",   // StructType_VertexOutput
		"fragment_input",  // StructType_FragmentInput
		"fragment_output", // StructType_FragmentOutput
	};
	static_assert( ARRAY_LENGTH( structNames ) == STRUCTTYPE_COUNT, "Missing TextureType" );

	const bool expectSlot =  ( node->structType == StructType_UniformBuffer ||
		node->structType == StructType_ConstantBuffer || node->structType == StructType_MutableBuffer );

	const bool expectMeta = !( node->structType == StructType_UniformBuffer ||
		node->structType == StructType_ConstantBuffer || node->structType == StructType_MutableBuffer ||
		node->structType == StructType_Struct || node->structType == StructType_SharedStruct );

	// <name>(<slot>) <type>
	output.append( structNames[node->structType] );
	if( expectSlot ) { output.append( "( " ).append( structure.slot ).append( " )" ); }
	output.append( " " ).append( typeName );

	// { <body> }
	output.append( "\n{\n" );
	indent_add();
	for( usize i = first; i < last; i++ )
	{
		output.append( indent );
		Variable &memberVariable = parser.variables[i];
		const String &memberVariableName = variable_name( i );

		Type &memberType = parser.types[memberVariable.typeID];
		const String &memberTypeName = type_name( memberVariable.typeID );

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

		// semantic(...) format(...)
		if( expectMeta )
		{
			const char *semantic = "";
			switch( memberVariable.semantic )
			{
				case SemanticType_VERTEX: semantic = "VERTEX"; break;
				case SemanticType_INSTANCE: semantic = "INSTANCE"; break;
				case SemanticType_POSITION: semantic = "POSITION"; break;
				case SemanticType_DEPTH: semantic = "DEPTH"; break;
				case SemanticType_COLOR: semantic = "COLOR"; break;
				default: Error( "Unexpected semantic type: %u", memberVariable.semantic ); return;
			};
			output.append( " " ).append( "semantic( " ).append( semantic ).append( " )" );

			const char *format = "";
			switch( memberVariable.format )
			{
				case InputFormat_UNORM8: format = "UNORM8"; break;
				case InputFormat_UNORM16: format = "UNORM16"; break;
				case InputFormat_UNORM32: format = "UNORM32"; break;
				case InputFormat_SNORM8: format = "SNORM8"; break;
				case InputFormat_SNORM16: format = "SNORM16"; break;
				case InputFormat_SNORM32: format = "SNORM32"; break;
				case InputFormat_UINT8: format = "UINT8"; break;
				case InputFormat_UINT16: format = "UINT16"; break;
				case InputFormat_UINT32: format = "UINT32"; break;
				case InputFormat_SINT8: format = "SINT8"; break;
				case InputFormat_SINT16: format = "SINT16"; break;
				case InputFormat_SINT32: format = "SINT32"; break;
				case InputFormat_FLOAT16: format = "FLOAT16"; break;
				case InputFormat_FLOAT32: format = "FLOAT32"; break;
				default: Error( "Unexpected input format: %u", memberVariable.format ); return;
			};
			output.append( " " ).append( "format( " ).append( format ).append( " )" );
		}

		output.append( ";\n" );
	}
	indent_sub();
	output.append( "};\n\n" );
}


void Generator::generate_structure_gfx( NodeStruct *node )
{
	switch( node->structType )
	{
		case StructType_SharedStruct:
			generate_structure_gfx_shared_struct( node );
		break;

		case StructType_UniformBuffer:
			generate_structure_gfx_uniform_buffer( node );
		break;

		case StructType_ConstantBuffer:
			generate_structure_gfx_uniform_buffer( node ); // TODO: Implement this
		break;

		case StructType_MutableBuffer:
			generate_structure_gfx_uniform_buffer( node ); // TODO: Implement this
		break;

		case StructType_InstanceInput:
			generate_structure_gfx_instance( node );
		break;

		case StructType_VertexInput:
			generate_structure_gfx_vertex( node );
		break;
	}
}


bool Generator::generate_structure_gfx_shared_struct( NodeStruct *node )
{
	Struct &structure = parser.structs[node->structID];
	Type &type = parser.types[structure.typeID];
	const VariableID first = type.memberFirst;
	const VariableID last = type.memberFirst + type.memberCount;

	// Layout Identifier
	String layout;
	for( VariableID i = first; i < last; i++ )
	{
		Type &memberType = parser.types[parser.variables[i].typeID];
		layout.append( memberType.name );
	};

	// SharedStruct Cache
	const u32 checksumKey = checksum_xcrc32( type.name.data, type.name.length, 0 );
	const u32 checksumBuffer = checksum_xcrc32( layout.data, layout.length_bytes(), 0 );

	// SharedStruct with this name already exists
	if( Gfx::sharedStructCache.contains( checksumKey ) )
	{
		SharedStruct &sharedStruct = Gfx::sharedStructs[Gfx::sharedStructCache.get( checksumKey )];
		if( sharedStruct.checksum == checksumBuffer ) { return false; }
		Error( "SharedStruct with name '%.*s' already declared with a different layout",
			type.name.length, type.name.data );
		return false;
	}

	// Register SharedStruct & Generate C++ Struct
	SharedStruct &sharedStruct = Gfx::sharedStructs.add( { } );
	sharedStruct.name.append( type.name );
	sharedStruct.id = static_cast<u32>( Gfx::sharedStructs.size() - 1 );
	sharedStruct.checksum = checksumBuffer;

	Gfx::sharedStructCache.add( checksumKey, sharedStruct.id );

	// Tight Struct
	String &headerTight = sharedStruct.headerTight;
	if( sharedStruct.id != 0 ) { headerTight.append( "\n" ); }
	headerTight.append( "\tstruct " );
	headerTight.append( type.name ).append( "\n" );
	headerTight.append( "\t{\n" );
	for( VariableID i = first; i < last; i++ )
	{
		Variable &memberVariable = parser.variables[i];
		Type &memberType = parser.types[memberVariable.typeID];
		append_structure_member_packed( headerTight, "\t\t",
			memberType, memberVariable, type.sizeBytesPacked );
	}
	headerTight.append( "\t};\n" );

	// Aligned Struct
	String &headerAlign = sharedStruct.headerAlign;
	if( sharedStruct.id != 0 ) { headerAlign.append( "\n" ); }
	headerAlign.append( "\tstruct " );
	if( structure.alignment > 1 )
	{
		headerAlign.append( "alignas( " ).append( structure.alignment ).append( " ) " );
	}
	headerAlign.append( type.name ).append( "\n" );
	headerAlign.append( "\t{\n" );
	for( VariableID i = first; i < last; i++ )
	{
		Variable &memberVariable = parser.variables[i];
		Type &memberType = parser.types[memberVariable.typeID];
		append_structure_member_padded( headerAlign, "\t\t", memberType,
			memberVariable, type.sizeBytesPadded );
	}
	headerAlign.append( "\t};\n" );

	return true;
}


bool Generator::generate_structure_gfx_uniform_buffer( NodeStruct *node )
{
	Struct &structure = parser.structs[node->structID];
	Type &type = parser.types[structure.typeID];
	const VariableID first = type.memberFirst;
	const VariableID last = type.memberFirst + type.memberCount;
	int structureByteOffset = 0;

	// Layout Identifier
	String layout;
	for( VariableID i = first; i < last; i++ )
	{
		Type &memberType = parser.types[parser.variables[i].typeID];
		layout.append( memberType.name );
	};

	// UniformBuffer Cache
	const u32 checksumKey = checksum_xcrc32( type.name.data, type.name.length, 0 );
	const u32 checksumBuffer = checksum_xcrc32( layout.data, layout.length_bytes(), 0 );

	// UniformBuffer with this name already exists
	if( Gfx::uniformBufferCache.contains( checksumKey ) )
	{
		UniformBuffer &uniformBuffer = Gfx::uniformBuffers[Gfx::uniformBufferCache.get( checksumKey )];
		shader.uniformBufferIDs[stage].add( uniformBuffer.id ); // Add this UniformBuffer to the shader
		shader.uniformBufferSlots[stage].add( structure.slot );
		if( uniformBuffer.checksum == checksumBuffer ) { return false; }
		Error( "UniformBuffer with name '%.*s' already declared with a different layout",
			type.name.length, type.name.data );
		return false;
	}

	// Register UniformBuffer & Generate C++ Struct
	UniformBuffer &uniformBuffer = Gfx::uniformBuffers.add( { } );
	String &header = uniformBuffer.header;
	String &source = uniformBuffer.source;
	uniformBuffer.name.append( type.name );
	uniformBuffer.id = static_cast<u32>( Gfx::uniformBuffers.size() - 1 );
	uniformBuffer.checksum = checksumBuffer;

	Gfx::uniformBufferCache.add( checksumKey, uniformBuffer.id );
	shader.uniformBufferIDs[stage].add( uniformBuffer.id ); // Add this UniformBuffer to the shader
	shader.uniformBufferSlots[stage].add( structure.slot );

	if( uniformBuffer.id != 0 ) { header.append( "\n" ); }
	header.append( "\tstruct alignas( 16 ) " ).append( type.name ).append( "_t\n" );
	header.append( "\t{\n" );
	for( VariableID i = first; i < last; i++ )
	{
		Variable &memberVariable = parser.variables[i];
		Type &memberType = parser.types[memberVariable.typeID];
		append_structure_member_padded( header, "\t\t", memberType, memberVariable, structureByteOffset );
	}
	header.append( "\n" );
	header.append( "\t\tvoid zero();\n" );
	header.append( "\t\tvoid upload() const;\n" );
	header.append( "\t\tbool operator==( const " ).append( type.name ).append( "_t &other ) " );
	header.append( "{ return ( memory_compare( this, &other, sizeof( " );
	header.append( type.name ).append( "_t ) ) == 0 ); }\n" );
	header.append( "\t\tbool operator!=( const " ).append( type.name ).append( "_t &other ) " );
	header.append( "{ return ( memory_compare( this, &other, sizeof( " );
	header.append( type.name ).append( "_t ) ) != 0 ); }\n" );
	header.append( "\t};\n" );

	if( uniformBuffer.id != 0 ) { source.append( "\n" ); }
	source.append( "\tvoid ").append( uniformBuffer.name ).append( "_t::zero()\n\t{\n" );
	source.append( "\t#if GRAPHICS_ENABLED\n" );
	source.append( "\t\tmemory_set( this, 0, sizeof( " ).append( uniformBuffer.name ).append( "_t ) );\n" );
	source.append( "\t#endif\n" );
	source.append( "\t}\n" );

	source.append( "\n\tvoid ").append( uniformBuffer.name ).append( "_t::upload() const\n\t{\n" );
	source.append( "\t#if GRAPHICS_ENABLED\n" );
	source.append( "\t\tauto *&resource = CoreGfx::uniformBuffers[" );
	source.append( static_cast<int>( uniformBuffer.id ) ).append( "];\n" );
	source.append( "\t\tCoreGfx::api_uniform_buffer_write_begin( resource );\n" );
	source.append( "\t\tCoreGfx::api_uniform_buffer_write( resource, this );\n" );
	source.append( "\t\tCoreGfx::api_uniform_buffer_write_end( resource );\n" );
	source.append( "\t#endif\n" );
	source.append( "\t}\n" );
	return true;
}


bool Generator::generate_structure_gfx_instance( NodeStruct *node )
{
	Struct &structure = parser.structs[node->structID];
	Type &type = parser.types[structure.typeID];
	const VariableID first = type.memberFirst;
	const VariableID last = type.memberFirst + type.memberCount;

	// Layout identifier
	String layout;
	for( VariableID i = first; i < last; i++ )
	{
		Type &memberType = parser.types[parser.variables[i].typeID];
		layout.append( memberType.name );
	};

	// Instance format cache
	const u32 checksumKey = checksum_xcrc32( type.name.data, type.name.length, 0 );
	const u32 checksumBuffer = checksum_xcrc32( layout.data, layout.length_bytes(), 0 );

	// Instance format with this name already exists
	if( Gfx::instanceFormatCache.contains( checksumKey ) )
	{
		InstanceFormat &instanceFormat = Gfx::instanceFormats[Gfx::instanceFormatCache.get( checksumKey )];
		instanceFormat.node = node;
		if( parser.instanceFormatTypeName.equals( type.name ) ) { shader.instanceFormatID = instanceFormat.id; }
		if( instanceFormat.checksum == checksumBuffer ) { return false; }
		Error( "Instance format with name '%.*s' already declared with a different layout",
			type.name.length, type.name.data );
		return false;
	}

	// Register Instance Format & Generate C++ Struct
	InstanceFormat &instanceFormat = Gfx::instanceFormats.add( { } );
	String &header = instanceFormat.header;
	String &source = instanceFormat.source;
	instanceFormat.name.append( type.name );
	instanceFormat.id = static_cast<u32>( Gfx::instanceFormats.size() - 1 );
	instanceFormat.checksum = checksumBuffer;
	instanceFormat.node = node;

	Gfx::instanceFormatCache.add( checksumKey, instanceFormat.id );
	if( parser.instanceFormatTypeName.equals( type.name ) ) { shader.instanceFormatID = instanceFormat.id; }

	if( instanceFormat.id != 0 ) { header.append( "\n" ); }
	header.append( "\tstruct " ).append( type.name ).append( "\n" );
	header.append( "\t{\n" );

	for( usize i = first; i < last; i++ )
	{
		Variable &memberVariable = parser.variables[i];

		// Get member type
		const char *memberTypeName = "";
		switch( memberVariable.format )
		{
			// 1 Byte
			case InputFormat_UNORM8:
			case InputFormat_UINT8:
				memberTypeName = "u8";
			break;

			case InputFormat_SNORM8:
			case InputFormat_SINT8:
				memberTypeName = "i8";
			break;

			// 2 Bytes
			case InputFormat_UNORM16:
			case InputFormat_UINT16:
				memberTypeName = "u16";
			break;

			case InputFormat_SNORM16:
			case InputFormat_SINT16:
				memberTypeName = "i16";
			break;

			// 4 Bytes
			case InputFormat_UNORM32:
			case InputFormat_UINT32:
				memberTypeName = "u32";
			break;

			case InputFormat_SNORM32:
			case InputFormat_SINT32:
				memberTypeName = "int";
			break;

			case InputFormat_FLOAT16:
			case InputFormat_FLOAT32:
				memberTypeName = "float";
			break;
		}

		// Get member type dimensions (vector length)
		int dimensions = 0;
		switch( memberVariable.typeID )
		{
			case Primitive_Bool:
			case Primitive_Int:
			case Primitive_UInt:
			case Primitive_Float:
				dimensions = 1;
			break;

			case Primitive_Bool2:
			case Primitive_Int2:
			case Primitive_UInt2:
			case Primitive_Float2:
				dimensions = 2;
			break;

			case Primitive_Bool3:
			case Primitive_Int3:
			case Primitive_UInt3:
			case Primitive_Float3:
				dimensions = 3;
			break;

			case Primitive_Bool4:
			case Primitive_Int4:
			case Primitive_UInt4:
			case Primitive_Float4:
				dimensions = 4;
			break;

			default:
				Error( "Unexpected instance input type: %llu (%.*s)", memberVariable.typeID,
				       memberVariable.name.length, memberVariable.name.data );
			break;
		}

		header.append( "\t\t" );
		header.append( memberTypeName );
		if( dimensions > 1 ) { header.append( "_v" ).append( dimensions ); }
		header.append( " " );
		header.append( memberVariable.name );
		header.append( ";\n" );
	}

	header.append( "\t};\n" );
	return true;
}


bool Generator::generate_structure_gfx_vertex( NodeStruct *node )
{
	Struct &structure = parser.structs[node->structID];
	Type &type = parser.types[structure.typeID];
	const VariableID first = type.memberFirst;
	const VariableID last = type.memberFirst + type.memberCount;

	// Layout identifier
	String layout;
	for( VariableID i = first; i < last; i++ )
	{
		Type &memberType = parser.types[parser.variables[i].typeID];
		layout.append( memberType.name );
	};

	// Vertex format cache
	const u32 checksumKey = checksum_xcrc32( type.name.data, type.name.length, 0 );
	const u32 checksumBuffer = checksum_xcrc32( layout.data, layout.length_bytes(), 0 );

	// Vertex format with this name already exists
	if( Gfx::vertexFormatCache.contains( checksumKey ) )
	{
		VertexFormat &vertexFormat = Gfx::vertexFormats[Gfx::vertexFormatCache.get( checksumKey )];
		if( parser.vertexFormatTypeName.equals( type.name ) ) { shader.vertexFormatID = vertexFormat.id; }
		if( vertexFormat.checksum == checksumBuffer ) { return false; }
		Error( "Vertex format with name '%.*s' already declared with a different layout",
			type.name.length, type.name.data );
		return false;
	}

	// Register Vertex Format & Generate C++ Struct
	VertexFormat &vertexFormat = Gfx::vertexFormats.add( { } );
	String &header = vertexFormat.header;
	String &source = vertexFormat.source;
	vertexFormat.name.append( type.name );
	vertexFormat.id = static_cast<u32>( Gfx::vertexFormats.size() - 1 );
	vertexFormat.checksum = checksumBuffer;
	vertexFormat.node = node;

	Gfx::vertexFormatCache.add( checksumKey, vertexFormat.id );
	if( parser.vertexFormatTypeName.equals( type.name ) ) { shader.vertexFormatID = vertexFormat.id; }

	if( vertexFormat.id != 0 ) { header.append( "\n" ); }
	header.append( "\tstruct " ).append( type.name ).append( "\n" );
	header.append( "\t{\n" );

	for( usize i = first; i < last; i++ )
	{
		Variable &memberVariable = parser.variables[i];

		// Get member type
		const char *memberTypeName = "";
		switch( memberVariable.format )
		{
			// 1 Byte
			case InputFormat_UNORM8:
			case InputFormat_UINT8:
				memberTypeName = "u8";
			break;

			case InputFormat_SNORM8:
			case InputFormat_SINT8:
				memberTypeName = "i8";
			break;

			// 2 Bytes
			case InputFormat_UNORM16:
			case InputFormat_UINT16:
				memberTypeName = "u16";
			break;

			case InputFormat_SNORM16:
			case InputFormat_SINT16:
				memberTypeName = "i16";
			break;

			// 4 Bytes
			case InputFormat_UNORM32:
			case InputFormat_UINT32:
				memberTypeName = "u32";
			break;

			case InputFormat_SNORM32:
			case InputFormat_SINT32:
				memberTypeName = "int";
			break;

			case InputFormat_FLOAT16:
			case InputFormat_FLOAT32:
				memberTypeName = "float";
			break;
		}

		// Get member type dimensions (vector length)
		int dimensions = 0;
		switch( memberVariable.typeID )
		{
			case Primitive_Bool:
			case Primitive_Int:
			case Primitive_UInt:
			case Primitive_Float:
				dimensions = 1;
			break;

			case Primitive_Bool2:
			case Primitive_Int2:
			case Primitive_UInt2:
			case Primitive_Float2:
				dimensions = 2;
			break;

			case Primitive_Bool3:
			case Primitive_Int3:
			case Primitive_UInt3:
			case Primitive_Float3:
				dimensions = 3;
			break;

			case Primitive_Bool4:
			case Primitive_Int4:
			case Primitive_UInt4:
			case Primitive_Float4:
				dimensions = 4;
			break;

			default:
				Error( "Unexpected vertex input type: %llu (%.*s)", memberVariable.typeID,
				       memberVariable.name.length, memberVariable.name.data );
			break;
		}

		header.append( "\t\t" );
		header.append( memberTypeName );
		if( dimensions > 1 ) { header.append( "_v" ).append( dimensions ); }
		header.append( " " );
		header.append( memberVariable.name );
		header.append( ";\n" );
	}

	header.append( "\t};\n" );
	return true;
}


void Generator::generate_texture( NodeTexture *node )
{
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
	output.append( indent ).append( "SamplerState" ).append( " " );
	output.append( variableName ).append( "_Sampler" );
	output.append( " : register( s" ).append( texture.slot ).append( " );\n\n" );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Generator::process_names()
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
				name.append( type.name );
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

			#if SHADER_OUTPUT_PREFIX_IDENTIFIERS
				name.append( "v_" ).append( variable.name );
			#else
				name.append( variable.name );
			#endif
		}
	}
}


String &Generator::type_name( TypeID typeID )
{
	Assert( typeID < typeNames.size() );
	return typeNames[typeID];
}


String &Generator::function_name( FunctionID functionID )
{
	Assert( functionID < functionNames.size() );
	return functionNames[functionID];
}


String &Generator::variable_name( VariableID variableID )
{
	Assert( variableID < variableNames.size() );
	return variableNames[variableID];
}


void Generator::leading_newline()
{
	const usize pos = output.length_bytes();
	if( pos >= 2 && output[pos - 2] != '\n' ) { output.append( "\n" ); }
}


void Generator::trailing_newline()
{
	output.append( "\n" );
}


void Generator::indent_add()
{
	indent[indentLevel] = '\t';
	if( indentLevel < 256 ) { indentLevel++; }
	indent[indentLevel] = '\0';
}


void Generator::indent_sub()
{
	if( indentLevel > 0 ) { indentLevel--; }
	indent[indentLevel] = '\0';
}


int Generator::append_structure_padding( String &output, const char *indent,
	int sizeType, int alignmentType, int current )
{
	const int sizeBlock = max( 16, sizeType );

	// Pad to next alignment interval
	int padding = ( alignmentType - ( current % alignmentType ) ) % alignmentType;

	// Ensure type at alignment interval fits within a block (16 bytes)
	int byteStart = current + padding;
	int byteStartBlock = ( byteStart / sizeBlock ) * sizeBlock;
	int byteEndBlock = byteStartBlock + sizeBlock;

	// If not, pad to the next block interval
	if( byteStart + sizeType > byteEndBlock )
	{
		padding += ( sizeBlock - ( byteStart % sizeBlock ) );
	}

	// Append padding
	if( padding > 0 )
	{
		output.append( indent );
		output.append( "char __pad").append( current );
		output.append( "[" ).append( padding ).append( "];\n" );
	}

	return padding;
}


void Generator::append_structure_member_padded( String &output, const char *indent,
	Type &type, Variable &variable, int &structureByteOffset )
{
	auto round_up16 = []( int size ) { return ( size + 15 ) & ~15; };

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

		case Primitive_Float2x2: typeNameCPU = StringView( "float_m44" ); size = 64; align = 16; break;
		case Primitive_Float3x3: typeNameCPU = StringView( "float_m44" ); size = 64; align = 16; break;
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

		snprintf( typeNameBuffer, sizeof( typeNameBuffer ), "std140_array_2d<%s%.*s, %d, %d>",
			type.tokenType == TokenType_SharedStruct ? "GfxStructPadded::" : "",
			static_cast<int>( typeNameCPU.data == nullptr ? type.name.length : typeNameCPU.length ),
			typeNameCPU.data == nullptr ? type.name.data : typeNameCPU.data,
			variable.arrayLengthX, variable.arrayLengthY );

		typeSizeBytes = round_up16( size ) * ( variable.arrayLengthX * variable.arrayLengthY );

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

		snprintf( typeNameBuffer, sizeof( typeNameBuffer ), "std140_array_1d<%s%.*s, %d>",
			type.tokenType == TokenType_SharedStruct ? "GfxStructPadded::" : "",
			static_cast<int>( typeNameCPU.data == nullptr ? type.name.length : typeNameCPU.length ),
			typeNameCPU.data == nullptr ? type.name.data : typeNameCPU.data,
			variable.arrayLengthX );

		typeSizeBytes = round_up16( size ) * ( variable.arrayLengthX );

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


void Generator::append_structure_member_packed( String &output, const char *indent,
	Type &type, Variable &variable, int &structureByteOffset )
{
	output.append( indent );

	StringView typeNameCPU;
	int align;
	switch( variable.typeID )
	{
		case Primitive_Bool: typeNameCPU = StringView( "u32" ); break;
		case Primitive_Bool2: typeNameCPU = StringView( "u32_v2" ); break;
		case Primitive_Bool3: typeNameCPU = StringView( "u32_v3" ); break;
		case Primitive_Bool4: typeNameCPU = StringView( "u32_v4" ); break;

		case Primitive_Int: typeNameCPU = StringView( "i32" ); break;
		case Primitive_Int2: typeNameCPU = StringView( "int_v2" ); break;
		case Primitive_Int3: typeNameCPU = StringView( "int_v3" ); break;
		case Primitive_Int4: typeNameCPU = StringView( "int_v4" ); break;

		case Primitive_UInt: typeNameCPU = StringView( "u32" ); break;
		case Primitive_UInt2: typeNameCPU = StringView( "u32_v2" ); break;
		case Primitive_UInt3: typeNameCPU = StringView( "u32_v3" ); break;
		case Primitive_UInt4: typeNameCPU = StringView( "u32_v4" ); break;

		case Primitive_Float: typeNameCPU = StringView( "float" ); break;
		case Primitive_Float2: typeNameCPU = StringView( "float_v2" ); break;
		case Primitive_Float3: typeNameCPU = StringView( "float_v3" ); break;
		case Primitive_Float4: typeNameCPU = StringView( "float_v4" ); break;

		case Primitive_Float2x2: typeNameCPU = StringView( "float_m44" ); break;
		case Primitive_Float3x3: typeNameCPU = StringView( "float_m44" ); break;
		case Primitive_Float4x4: typeNameCPU = StringView( "float_m44" ); break;

		case Primitive_Void: typeNameCPU = StringView( "void" ); break;
		default: typeNameCPU.data = nullptr; break;
	};

	// Type
	if( type.tokenType == TokenType_SharedStruct ) { output.append( "GfxStructPacked::" ); }
	output.append( typeNameCPU.data == nullptr ? type.name : typeNameCPU );

	// Variable
	output.append( " " ).append( variable.name );

	// Array X/Y
	if( variable.arrayLengthX > 0 ) { output.append( "[" ).append( variable.arrayLengthX ).append( "]" ); }
	if( variable.arrayLengthY > 0 ) { output.append( "[" ).append( variable.arrayLengthY ).append( "]" ); }

	output.append( ";\n" );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}