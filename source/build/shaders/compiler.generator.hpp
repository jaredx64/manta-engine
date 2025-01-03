#pragma once

#include <core/types.hpp>

#include <build/shaders/compiler.hpp>
#include <build/shaders/compiler.parser.hpp>

#include <core/string.hpp>

namespace ShaderCompiler
{
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Generator
{
	Generator( Shader &shader, ShaderStage stage, Parser &parser ) :
		shader{ shader }, stage{ stage }, parser{ parser }, output{ shader.outputs[stage] } { }

	Shader &shader;
	ShaderStage stage;
	Parser &parser;
	String &output;

	char indent[256];
	u32 indentLevel = 0;

	List<String> typeNames;
	List<String> functionNames;
	List<String> variableNames;

	virtual void process_names();

	virtual String &type_name( TypeID typeID );
	virtual String &function_name( FunctionID functionID );
	virtual String &variable_name( VariableID variableID );

	virtual void indent_add();
	virtual void indent_sub();
	virtual void leading_newline();
	virtual void trailing_newline();

	virtual void generate_stage( ShaderStage stage );

	virtual void generate_node( Node *node );

	virtual void generate_statement( NodeStatement *node );
	virtual void generate_statement_block( NodeStatementBlock *node );
	virtual void generate_statement_expression( NodeStatementExpression *node );
	virtual void generate_statement_if( NodeStatementIf *node );
	virtual void generate_statement_while( NodeStatementWhile *node );
	virtual void generate_statement_do_while( NodeStatementDoWhile *node );
	virtual void generate_statement_for( NodeStatementFor *node );
	virtual void generate_statement_switch( NodeStatementSwitch *node );
	virtual void generate_statement_case( NodeStatementCase *node );
	virtual void generate_statement_default( NodeStatementDefault *node );
	virtual void generate_statement_return( NodeStatementReturn *node );
	virtual void generate_statement_break( NodeStatementBreak *node );
	virtual void generate_statement_discard( NodeStatementDiscard *node );

	virtual void generate_expression_unary( NodeExpressionUnary *node );
	virtual void generate_expression_binary( NodeExpressionBinary *node );
	virtual void generate_expression_binary_dot( NodeExpressionBinary *node );
	virtual void generate_expression_ternary( NodeExpressionTernary *node );

	virtual void generate_function_declaration( NodeFunctionDeclaration *node );
	virtual void generate_function_declaration_main( NodeFunctionDeclaration *node );
	virtual void generate_function_call( NodeFunctionCall *node );
	virtual void generate_function_call_parameters( NodeFunctionCall *node );
	virtual void generate_function_call_intrinsics( NodeFunctionCall *node );

	virtual void generate_cast( NodeCast *node );

	virtual void generate_variable_declaration( NodeVariableDeclaration *node );
	virtual void generate_variable( NodeVariable *node );
	virtual void generate_swizzle( NodeSwizzle *node );
	virtual void generate_group( NodeGroup *node );
	virtual void generate_integer( NodeInteger *node );
	virtual void generate_number( NodeNumber *node );
	virtual void generate_boolean( NodeBoolean *node );

	virtual void generate_structure( NodeStruct *node );
	virtual void generate_structure_gfx( NodeStruct *node );
	virtual bool generate_structure_gfx_vertex( NodeStruct *node );
	virtual bool generate_structure_gfx_cbuffer( NodeStruct *node );

	virtual void generate_texture( NodeTexture *node );
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct GeneratorHLSL : public Generator
{
	GeneratorHLSL( Shader &shader, ShaderStage stage, Parser &parser ) :
		Generator{ shader, stage, parser } { }

	virtual void generate_stage( ShaderStage stage );

	virtual void generate_function_declaration( NodeFunctionDeclaration *node );
	virtual void generate_function_declaration_main( NodeFunctionDeclaration *node );

	virtual void generate_expression_binary_dot( NodeExpressionBinary *node );

	virtual void generate_function_call_intrinsics( NodeFunctionCall *node );

	virtual void generate_structure( NodeStruct *node );
	virtual bool generate_structure_gfx_vertex( NodeStruct *node );

	virtual void generate_texture( NodeTexture *node );
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct GeneratorGLSL : public Generator
{
	GeneratorGLSL( Shader &shader, ShaderStage stage, Parser &parser ) :
		Generator{ shader, stage, parser } { }

	virtual void process_names();
	virtual void generate_stage( ShaderStage stage );

	virtual void generate_function_declaration( NodeFunctionDeclaration *node );
	virtual void generate_function_declaration_main( NodeFunctionDeclaration *node );

	virtual void generate_function_call_intrinsics( NodeFunctionCall *node );

	virtual void generate_expression_binary( NodeExpressionBinary *node );

	virtual void generate_structure( NodeStruct *node );
	virtual bool generate_structure_gfx_vertex( NodeStruct *node );

	virtual void generate_texture( NodeTexture *node );
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct GeneratorMetal : public Generator
{
	GeneratorMetal( Shader &shader, ShaderStage stage, Parser &parser ) :
		Generator{ shader, stage, parser } { }

	virtual void generate_structure( NodeStruct *node );
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}