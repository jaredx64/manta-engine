#pragma once

#include <core/types.hpp>

#include <build/shaders/compiler.hpp>
#include <build/shaders/compiler.parser.hpp>

namespace ShaderCompiler
{
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Optimizer
{
	Optimizer( Parser &parser ) : parser{ parser } { }
	Parser &parser;

	void optimize_stage( ShaderStage stage );

	void optimize_node( Node *node );

	void optimize_statement( NodeStatement *node );
	void optimize_statement_block( NodeStatementBlock *node );
	void optimize_statement_expression( NodeStatementExpression *node );
	void optimize_statement_if( NodeStatementIf *node );
	void optimize_statement_while( NodeStatementWhile *node );
	void optimize_statement_do_while( NodeStatementDoWhile *node );
	void optimize_statement_for( NodeStatementFor *node );
	void optimize_statement_switch( NodeStatementSwitch *node );
	void optimize_statement_case( NodeStatementCase *node );
	void optimize_statement_default( NodeStatementDefault *node );
	void optimize_statement_return( NodeStatementReturn *node );

	void optimize_expression_unary( NodeExpressionUnary *node );
	void optimize_expression_binary( NodeExpressionBinary *node );
	void optimize_expression_binary_dot( NodeExpressionBinary *node );
	void optimize_expression_ternary( NodeExpressionTernary *node );

	void optimize_function_declaration( NodeFunctionDeclaration *node );
	void optimize_function_call( NodeFunctionCall *node );

	void optimize_cast( NodeCast *node );

	void optimize_variable_declaration( NodeVariableDeclaration *node );
	void optimize_variable( NodeVariable *node );
	void optimize_group( NodeGroup *node );
	void optimize_structure( NodeStruct *node );
	void optimize_texture( NodeTexture *node );
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
};