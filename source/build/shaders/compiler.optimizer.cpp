
#include <build/shaders/compiler.optimizer.hpp>

#include <build/shaders/compiler.hpp>

#include <build/time.hpp>

namespace ShaderCompiler
{
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Optimizer::optimize_stage( ShaderStage stage )
{
	Timer time;

	// Reset 'seen' flags
	for( Type &type : parser.types ) { type.seen = false; }
	for( Texture &texture : parser.textures ) { texture.seen = false; }
	for( Function &function : parser.functions ) { function.seen = false; }

	// Find Entry Point Node
	int entryPoint = -1;
	int nodesCount = static_cast<int>( parser.program.size() );
	for( int i = 0; i < nodesCount; i++ )
	{
		Node *node = parser.program[i];

		if( node->nodeType != NodeType_FunctionDeclaration ) { continue; }
		const NodeFunctionDeclaration *const declaration = reinterpret_cast<NodeFunctionDeclaration *>( node );

		if( ( stage == ShaderStage_Vertex   && declaration->functionType == FunctionType_MainVertex ) ||
		    ( stage == ShaderStage_Fragment && declaration->functionType == FunctionType_MainFragment ) ||
		    ( stage == ShaderStage_Compute  && declaration->functionType == FunctionType_MainCompute ) )
		{
			entryPoint = i;
			break;
		}
	}

	// Traverse Program Backwards & Visit Nodes
	for( int i = entryPoint; i >= 0; i-- )
	{
		// Skip "unseen" nodes
		Node *node = parser.program[i];
		if( i < entryPoint && !parser.node_seen( node ) )
		{
			continue;
		}

		// Optimize Node
		optimize_node( node );
	}
}



void Optimizer::optimize_node( Node *node )
{
	switch( node->nodeType )
	{
		case NodeType_Statement:
		{
			optimize_statement( reinterpret_cast<NodeStatement *>( node ) );
		}
		break;

		case NodeType_ExpressionUnary:
		{
			optimize_expression_unary( reinterpret_cast<NodeExpressionUnary *>( node ) );
		}
		break;

		case NodeType_ExpressionBinary:
		{
			optimize_expression_binary( reinterpret_cast<NodeExpressionBinary *>( node ) );
		}
		break;

		case NodeType_ExpressionTernary:
		{
			optimize_expression_ternary(reinterpret_cast<NodeExpressionTernary *>( node ) );
		}
		break;

		case NodeType_FunctionDeclaration:
		{
			optimize_function_declaration( reinterpret_cast<NodeFunctionDeclaration *>( node ) );
		}
		break;

		case NodeType_FunctionCall:
		{
			optimize_function_call( reinterpret_cast<NodeFunctionCall *>( node ) );
		}
		break;

		case NodeType_Cast:
		{
			optimize_cast( reinterpret_cast<NodeCast *>( node ) );
		}
		break;

		case NodeType_VariableDeclaration:
		{
			optimize_variable_declaration( reinterpret_cast<NodeVariableDeclaration *>( node ) );
		}
		break;

		case NodeType_Variable:
		{
			optimize_variable( reinterpret_cast<NodeVariable *>( node ) );
		}
		break;

		case NodeType_Swizzle:
		{
			// Do nothing
		}
		break;

		case NodeType_Group:
		{
			optimize_group( reinterpret_cast<NodeGroup *>( node ) );
		}
		break;

		case NodeType_Integer:
		{
			// Do nothing
		}
		break;

		case NodeType_Number:
		{
			// Do nothing
		}
		break;

		case NodeType_Boolean:
		{
			// Do nothing
		}
		break;

		case NodeType_Struct:
		{
			optimize_structure( reinterpret_cast<NodeStruct *>( node ) );
		}
		break;

		case NodeType_Texture:
		{
			optimize_texture( reinterpret_cast<NodeTexture *>( node ) );
		}
		break;

		default:
			Error( "%s: unexpected NodeType! %u", __FUNCTION__, node->nodeType );
		break;
	}
}


void Optimizer::optimize_statement( NodeStatement *node )
{
	switch( node->statementType )
	{
		case StatementType_Block:
		{
			optimize_statement_block( reinterpret_cast<NodeStatementBlock *>( node ) );
		}
		break;

		case StatementType_Expression:
		{
			optimize_statement_expression( reinterpret_cast<NodeStatementExpression *>( node ) );
		}
		break;

		case StatementType_If:
		{
			optimize_statement_if( reinterpret_cast<NodeStatementIf *>( node ) );
		}
		break;

		case StatementType_While:
		{
			optimize_statement_while( reinterpret_cast<NodeStatementWhile *>( node ) );
		}
		break;

		case StatementType_DoWhile:
		{
			optimize_statement_do_while( reinterpret_cast<NodeStatementDoWhile *>( node ) );
		}
		break;

		case StatementType_For:
		{
			optimize_statement_for( reinterpret_cast<NodeStatementFor *>( node ) );
		}
		break;

		case StatementType_Switch:
		{
			optimize_statement_switch( reinterpret_cast<NodeStatementSwitch *>( node ) );
		}
		break;

		case StatementType_Case:
		{
			optimize_statement_case( reinterpret_cast<NodeStatementCase *>( node ) );
		}
		break;

		case StatementType_Default:
		{
			optimize_statement_default( reinterpret_cast<NodeStatementDefault *>( node ) );
		}
		break;

		case StatementType_Return:
		{
			optimize_statement_return( reinterpret_cast<NodeStatementReturn *>( node ) );
		}
		break;

		case StatementType_Break:
		{
			// Do nothing
		}
		break;

		case StatementType_Discard:
		{
			// Do nothing
		}
		break;
	}
}


void Optimizer::optimize_statement_block( NodeStatementBlock *node )
{
	while( node != nullptr )
	{
		if( node->expr != nullptr ) { optimize_node( node->expr ); }
		node = reinterpret_cast<NodeStatementBlock *>( node->next );
	}
}


void Optimizer::optimize_statement_expression( NodeStatementExpression *node )
{
	optimize_node( node->expr );
}


void Optimizer::optimize_statement_if( NodeStatementIf *node )
{
	// if( ... ) { }
	optimize_node( node->expr );
	optimize_node( node->blockIf );

	// else ...
	if( node->blockElse != nullptr )
	{
		optimize_node( node->blockElse );
	}
}


void Optimizer::optimize_statement_while( NodeStatementWhile *node )
{
	optimize_node( node->expr );

	if( node->block != nullptr )
	{
		optimize_node( node->block );
	}
}


void Optimizer::optimize_statement_do_while( NodeStatementDoWhile *node )
{
	if( node->block != nullptr )
	{
		optimize_node( node->block );
	}

	optimize_node( node->expr );
}


void Optimizer::optimize_statement_for( NodeStatementFor *node )
{
	if( node->expr1 != nullptr ) { optimize_node( node->expr1 ); }
	if( node->expr2 != nullptr ) { optimize_node( node->expr2 ); }
	if( node->expr3 != nullptr ) { optimize_node( node->expr3 ); }

	if( node->block != nullptr )
	{
		optimize_node( node->block );
	}
}


void Optimizer::optimize_statement_switch( NodeStatementSwitch *node )
{
	optimize_node( node->expr );

	if( node->block != nullptr )
	{
		optimize_node( node->block );
	}
}


void Optimizer::optimize_statement_case( NodeStatementCase *node )
{
	optimize_node( node->expr );

	if( node->block != nullptr )
	{
		optimize_node( node->block );
	}
}


void Optimizer::optimize_statement_default( NodeStatementDefault *node )
{
	if( node->block != nullptr )
	{
		optimize_node( node->block );
	}
}


void Optimizer::optimize_statement_return( NodeStatementReturn *node )
{
	if( node->expr != nullptr )
	{
		optimize_node( node->expr );
	}
}


void Optimizer::optimize_expression_unary( NodeExpressionUnary *node )
{
	optimize_node( node->expr );
}


void Optimizer::optimize_expression_binary( NodeExpressionBinary *node )
{
	optimize_node( node->expr1 );
	optimize_node( node->expr2 );
}


void Optimizer::optimize_expression_ternary( NodeExpressionTernary *node )
{
	optimize_node( node->expr1 );
	optimize_node( node->expr2 );
	optimize_node( node->expr3 );
}


void Optimizer::optimize_function_declaration( NodeFunctionDeclaration *node )
{
	// Function
	Function &function = parser.functions[node->functionID];
	function.seen = true;

	// Return Type
	Type &type = parser.types[function.typeID];
	type.seen = true;

	// Parameters
	const VariableID first = function.parameterFirst;
	const VariableID last = first + function.parameterCount;

	for( VariableID i = first; i < last; i++ )
	{
		Type &parameterType = parser.types[parser.variables[i].typeID];
		parameterType.seen = true;
	}

	// Body
	optimize_statement_block( reinterpret_cast<NodeStatementBlock *>( node->block ) );
}


void Optimizer::optimize_function_call( NodeFunctionCall *node )
{
	// Function
	Function &function = parser.functions[node->functionID];
	function.seen = true;

	// Parameters
	Node *param = node->param;
	while( param != nullptr )
	{
		NodeExpressionList *paramNode = reinterpret_cast<NodeExpressionList *>( param );
		optimize_node( paramNode->expr );
		param = paramNode->next;
	}
}


void Optimizer::optimize_cast( NodeCast *node )
{
	// Type
	Type &type = parser.types[node->typeID];
	type.seen = true;

	// Parameters
	Node *param = node->param;
	while( param != nullptr )
	{
		NodeExpressionList *paramNode = reinterpret_cast<NodeExpressionList *>( param );
		optimize_node( paramNode->expr );
		param = paramNode->next;
	}
}


void Optimizer::optimize_variable_declaration( NodeVariableDeclaration *node )
{
	// Type
	Type &type = parser.types[parser.variables[node->variableID].typeID];
	type.seen = true;

	// Assignment
	if( node->assignment != nullptr )
	{
		optimize_node( node->assignment );
	}
}


void Optimizer::optimize_variable( NodeVariable *node )
{
	// Type
	Variable &variable = parser.variables[node->variableID];
	Type &type = parser.types[variable.typeID];
	type.seen = true;

	// Texture?
	if( parser.textureMap.contains( variable.name ) )
	{
		Texture &texture = parser.textures[parser.textureMap.get( variable.name )];
		texture.seen = true;
	}
}


void Optimizer::optimize_group( NodeGroup *node )
{
	optimize_node( node->expr );
}


void Optimizer::optimize_structure( NodeStruct *node )
{
	// Type
	Type &type = parser.types[parser.structs[node->structID].typeID];
	type.seen = true;

	// Members
	const VariableID first = type.memberFirst;
	const VariableID last = first + type.memberCount;
	for( usize i = first; i < last; i++ )
	{
		Type &memberType = parser.types[parser.variables[i].typeID];
		memberType.seen = true;
	}
}


void Optimizer::optimize_texture( NodeTexture *node )
{
	Texture &texture = parser.textures[node->textureID];
	texture.seen = true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}