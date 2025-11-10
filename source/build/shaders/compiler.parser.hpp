#pragma once

#include <build/shaders/compiler.hpp>
#include <build/gfx.hpp>

#include <core/types.hpp>
#include <core/list.hpp>
#include <core/hashmap.hpp>
#include <core/buffer.hpp>
#include <core/string.hpp>

namespace ShaderCompiler
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Token
{
public:
	Token() = default;
	Token( TokenType type, StringView name ) : type{ type }, name{ name } { }
	Token( TokenType type, double number, StringView name ) : type{ type }, name{ name }, number{ number } { }
	Token( TokenType type, u64 integer, StringView name ) : type{ type }, name{ name }, integer{ integer } { }

public:
	usize position = 0;
	usize start = 0;
	u32 line = 0;

	TokenType type = TokenType_Null;
	StringView name { };
	double number = 0.0;
	u64 integer = 0;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Scanner
{
public:
	void init( const char *buffer )
	{
		this->buffer = buffer;
		this->position = 0LLU;
		stack.clear();
	}

	void init( const char *buffer, const usize offset )
	{
		this->buffer = buffer;
		this->position = offset;
		stack.clear();
	}

	bool consume( char c );
	void skip_whitespace();

	Token next( const bool skipWhitespace = true );
	Token back();
	Token current();

public:
	enum_type( ScannerMode, int )
	{
		ScannerMode_Compiler,
		ScannerMode_Preprocessor,
	};

	const char *buffer;
	List<Token> stack;
	ScannerMode mode = ScannerMode_Compiler;
	usize position = 0LLU;
	u32 line = 1;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Parser
{
public:
	Parser( Shader &shader, const char *path ) : shader{ shader }, path{ path } { }

	void init();
	bool parse( const char *string );

	Node *parse_statement();
	Node *parse_statement_block( const VariableID scopeIndex = USIZE_MAX );
	Node *parse_statement_expression();
	Node *parse_statement_function_call();
	Node *parse_statement_cast();
	Node *parse_statement_if();
	Node *parse_statement_while();
	Node *parse_statement_do_while();
	Node *parse_statement_for();
	Node *parse_statement_switch();
	Node *parse_statement_case();
	Node *parse_statement_default();
	Node *parse_statement_return();
	Node *parse_statement_break();
	Node *parse_statement_continue();
	Node *parse_statement_discard();

	Node *parse_structure();
	Node *parse_texture();

	Node *parse_variable_declaration();

	Node *parse_function_declaration();
	Node *parse_function_declaration_main( FunctionType functionType, const char *functionName,
		TokenType inToken, TokenType outToken );

	// https://en.cppreference.com/w/c/language/operator_precedence
	Node *parse_assignment();             // Lowest Precedence
	Node *parse_ternary_condition();
	Node *parse_logical_or();
	Node *parse_logical_and();
	Node *parse_bitwise_or();
	Node *parse_bitwise_xor();
	Node *parse_bitwise_and();
	Node *parse_equality();
	Node *parse_comparison();
	Node *parse_bitwise_shift();
	Node *parse_add_sub();
	Node *parse_mul_div_mod();
	Node *parse_prefix_operators();
	Node *parse_suffix_operators();
	Node *parse_dot_operator();
	Node *parse_subscript_operator();
	Node *parse_fundamental();            // Highest Precedence
	inline Node *parse_expression() { return parse_assignment(); }

	List<Struct> structs;
	StructID register_struct( const Struct structure );

	List<Texture> textures;
	HashMap<StringView, TextureID> textureMap;
	TextureID register_texture( const Texture texture );

	List<Type> types;
	HashMap<StringView, TypeID> typeMap;
	TypeID register_type( const Type type );

	List<Function> functions;
	HashMap<StringView, FunctionID> functionMap;
	FunctionID register_function( const Function function );

	List<Variable> variables;
	VariableID register_variable( const Variable variable );

	bool swizzling = false;
	HashMap<StringView, SwizzleID> swizzleMap;
	void register_swizzles();

	HashMap<StringView, SVSemanticType> svSemanticMap;
	void register_svsemantics();

	TypeID node_type( Node *node );
	bool node_is_constexpr( Node *node );

	List<VariableID> scope;
	VariableID scope_find_variable( const StringView name );
	void scope_reset( const VariableID target );

	void check_namespace_conflicts( const StringView name );
	void expect_semicolon();
	bool node_seen( Node *node );

	// Override Error Macros
	void ERROR_HANDLER_FUNCTION_DECL;

public:
	Shader &shader;
	const char *path;

	Scanner scanner;
	List<Node *> program;
	NodeBuffer ast;

	String vertexFormatTypeName;
	String instanceFormatTypeName;

	FunctionID mainVertex = USIZE_MAX;
	FunctionID mainFragment = USIZE_MAX;
	FunctionID mainCompute = USIZE_MAX;

	u64 threadGroupX;
	u64 threadGroupY;
	u64 threadGroupZ;

	bool bufferSlots[SHADER_MAX_BUFFER_SLOTS];
	bool textureSlots[SHADER_MAX_TEXTURE_SLOTS];
	bool targetColorSlots[SHADER_MAX_TARGET_SLOTS];
	bool targetDepthSlots[SHADER_MAX_TARGET_SLOTS];
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}