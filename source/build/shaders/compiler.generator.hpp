#pragma once

#include <core/types.hpp>

#include <build/shaders/compiler.hpp>
#include <build/shaders/compiler.parser.hpp>

#include <core/string.hpp>

namespace ShaderCompiler
{
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Generator
{
public:
	Generator( Shader &shader, ShaderStage stage, Parser &parser ) :
		shader{ shader }, stage{ stage }, parser{ parser }, output{ shader.outputs[stage] } { }

public:
	Shader &shader;
	ShaderStage stage;
	Parser &parser;
	String &output;

	char indent[256];
	u32 indentLevel = 0;

	List<String> typeNames;
	List<String> functionNames;
	List<String> variableNames;

public:
	virtual void process_names();

	virtual String &type_name( TypeID typeID );
	virtual String &function_name( FunctionID functionID );
	virtual String &variable_name( VariableID variableID );

	virtual void indent_add();
	virtual void indent_sub();
	virtual void leading_newline();
	virtual void trailing_newline();

	virtual int append_structure_padding( String &output, const char *indent,
		int sizeType, int alignmentType, int current );

	virtual void append_structure_member_padded( String &output, const char *indent,
		Type &type, Variable &variable, int &structureByteOffset );
	virtual void append_structure_member_packed( String &output, const char *indent,
		Type &type,Variable &variable, int &structureByteOffset );

	virtual void generate_stage( ShaderStage stage );
	virtual void generate_node( Node *node );
	virtual void generate_node_parenthesis( Node *node );

	virtual void generate_statement( NodeStatement *node );
	virtual void generate_statement_block_no_braces( NodeStatementBlock *node );
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
	virtual void generate_statement_continue( NodeStatementContinue *node );
	virtual void generate_statement_discard( NodeStatementDiscard *node );

	virtual void generate_expression_unary( NodeExpressionUnary *node );
	virtual void generate_expression_binary( NodeExpressionBinary *node );
	virtual void generate_expression_binary_dot( NodeExpressionBinary *node );
	virtual void generate_expression_ternary( NodeExpressionTernary *node );

	virtual void generate_function_declaration( NodeFunctionDeclaration *node );
	virtual void generate_function_declaration_main_pipeline( NodeFunctionDeclaration *node );
	virtual void generate_function_declaration_main_compute( NodeFunctionDeclaration *node );

	virtual void generate_function_call( NodeFunctionCall *node );
	virtual void generate_function_call_parameters( NodeFunctionCall *node );
	virtual void generate_function_call_intrinsics( NodeFunctionCall *node );

	virtual void generate_cast( NodeCast *node );

	virtual void generate_variable_declaration( NodeVariableDeclaration *node );
	virtual void generate_variable( NodeVariable *node );
	virtual void generate_swizzle( NodeSwizzle *node );
	virtual void generate_sv_semantic( NodeSVSemantic *node );
	virtual void generate_group( NodeGroup *node );
	virtual void generate_integer( NodeInteger *node );
	virtual void generate_number( NodeNumber *node );
	virtual void generate_boolean( NodeBoolean *node );

	virtual void generate_structure( NodeStruct *node );
	virtual void generate_structure_gfx( NodeStruct *node );
	virtual bool generate_structure_gfx_shared_struct( NodeStruct *node );
	virtual bool generate_structure_gfx_uniform_buffer( NodeStruct *node );
	virtual bool generate_structure_gfx_vertex( NodeStruct *node );
	virtual bool generate_structure_gfx_instance( NodeStruct *node );

	virtual void generate_texture( NodeTexture *node );
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class GeneratorHLSL : public Generator
{
public:
	GeneratorHLSL( Shader &shader, ShaderStage stage, Parser &parser ) :
		Generator{ shader, stage, parser } { }

public:
	virtual void append_structure_member_padded( String &output, const char *indent,
		Type &type, Variable &variable, int &structureByteOffset );

	virtual void generate_stage( ShaderStage stage );

	virtual void generate_statement_for( NodeStatementFor *node );

	virtual void generate_function_declaration( NodeFunctionDeclaration *node );
	virtual void generate_function_declaration_main_pipeline( NodeFunctionDeclaration *node );
	virtual void generate_function_declaration_main_compute( NodeFunctionDeclaration *node );

	virtual void generate_expression_binary_dot( NodeExpressionBinary *node );

	virtual void generate_function_call_parameters( NodeFunctionCall *node );
	virtual void generate_function_call_intrinsics( NodeFunctionCall *node );

	virtual void generate_sv_semantic( NodeSVSemantic *node );

	virtual void generate_structure( NodeStruct *node );
	virtual bool generate_structure_gfx_vertex( NodeStruct *node );
	virtual bool generate_structure_gfx_instance( NodeStruct *node );

	virtual void generate_texture( NodeTexture *node );

	void generate_sv_semantic_struct();
	void generate_sv_semantic_entry_parameters();
	void generate_sv_semantic_entry_caching();

	void generate_statement_block_main( NodeStatementBlock *node );
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class GeneratorGLSL : public Generator
{
public:
	GeneratorGLSL( Shader &shader, ShaderStage stage, Parser &parser ) :
		Generator{ shader, stage, parser } { }

public:
	virtual void process_names();

	virtual void generate_stage( ShaderStage stage );

	virtual void generate_function_declaration( NodeFunctionDeclaration *node );
	virtual void generate_function_declaration_main_pipeline( NodeFunctionDeclaration *node );
	virtual void generate_function_declaration_main_compute( NodeFunctionDeclaration *node );

	virtual void generate_function_call_intrinsics( NodeFunctionCall *node );

	virtual void generate_sv_semantic( NodeSVSemantic *node );

	virtual void generate_expression_binary( NodeExpressionBinary *node );

	virtual void generate_structure( NodeStruct *node );
	virtual bool generate_structure_gfx_vertex( NodeStruct *node );
	virtual bool generate_structure_gfx_instance( NodeStruct *node );

	virtual void generate_texture( NodeTexture *node );
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class GeneratorMetal : public Generator
{
public:
	GeneratorMetal( Shader &shader, ShaderStage stage, Parser &parser ) :
		Generator{ shader, stage, parser } { }

public:
	virtual void generate_structure( NodeStruct *node );
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class GeneratorVulkan : public Generator
{
public:
	GeneratorVulkan( Shader &shader, ShaderStage stage, Parser &parser ) :
		Generator{ shader, stage, parser } { }

public:
	virtual void generate_structure( NodeStruct *node );
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}