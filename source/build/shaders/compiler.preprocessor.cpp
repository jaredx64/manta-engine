#include <build/shaders/compiler.preprocessor.hpp>

#include <build/shaders/compiler.hpp>
#include <build/shaders/compiler.parser.hpp>

#include <core/hashmap.hpp>
#include <core/string.hpp>
#include <core/math.hpp>

#include <build/system.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace ShaderCompiler;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Macro
{
	String name = "";
	String definition = "";
	int parameters = 0;
};

enum_type( IncludeFlag, u32 )
{
	IncludeFlag_None = 0,
	IncludeFlag_PragmaOnce = ( 1 << 0 ),
};

struct Include
{
	Include() : contents { }, flags { IncludeFlag_None } { }
	Include( const String contents ) : contents { contents }, flags { IncludeFlag_None } { }
	String contents;
	IncludeFlag flags;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static HashMap<u32, Macro> macros;
static HashMap<u32, Include> includes;
static int branchDepth = 0;
static bool branchEvaluateElseIf = false;
static String conditionLine;
static NodeBuffer conditionAST;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// https://en.cppreference.com/w/c/language/operator_precedence
static Node *condition_parse_ternary_condition( Scanner &scanner );   // Lowest Precedence
static Node *condition_parse_logical_or( Scanner &scanner );
static Node *condition_parse_logical_and( Scanner &scanner );
static Node *condition_parse_bitwise_or( Scanner &scanner );
static Node *condition_parse_bitwise_xor( Scanner &scanner );
static Node *condition_parse_bitwise_and( Scanner &scanner );
static Node *condition_parse_equality( Scanner &scanner );
static Node *condition_parse_comparison( Scanner &scanner );
static Node *condition_parse_bitwise_shift( Scanner &scanner );
static Node *condition_parse_add_sub( Scanner &scanner );
static Node *condition_parse_mul_div_mod( Scanner &scanner );
static Node *condition_parse_prefix_operators( Scanner &scanner );
static Node *condition_parse_fundamental( Scanner &scanner );         // Highest Precedence
static Node *condition_parse_expression( Scanner &scanner ) { return condition_parse_ternary_condition( scanner ); }


Node *condition_parse_ternary_condition( Scanner &scanner )
{
	Node *node = condition_parse_logical_or( scanner );

	if( scanner.current().type == TokenType_Question )
	{
		scanner.next();
		Node *expr2 = condition_parse_ternary_condition( scanner );

		ErrorIf( scanner.current().type != TokenType_Colon, "Expected ':' in ternary statement" );

		scanner.next();
		Node *expr3 = condition_parse_ternary_condition( scanner );

		return conditionAST.add( NodeExpressionTernary( ExpressionTernaryType_Conditional, node, expr2, expr3 ) );
	}

	return node;
}


Node *condition_parse_logical_or( Scanner &scanner )
{
	Node *node = condition_parse_logical_and( scanner );

	while( scanner.current().type == TokenType_Or )
	{
		scanner.next();
		node = conditionAST.add( NodeExpressionBinary( ExpressionBinaryType_Or, node,
			condition_parse_logical_and( scanner ) ) );
	}

	return node;
}


Node *condition_parse_logical_and( Scanner &scanner )
{
	Node *node = condition_parse_bitwise_or( scanner );

	while( scanner.current().type == TokenType_And )
	{
		scanner.next();
		node = conditionAST.add( NodeExpressionBinary( ExpressionBinaryType_And, node,
			condition_parse_bitwise_or( scanner ) ) );
	}

	return node;
}


Node *condition_parse_bitwise_or( Scanner &scanner )
{
	Node *node = condition_parse_bitwise_xor( scanner );

	while( scanner.current().type == TokenType_BitOr )
	{
		scanner.next();
		node = conditionAST.add( NodeExpressionBinary( ExpressionBinaryType_BitOr, node,
			condition_parse_bitwise_xor( scanner ) ) );
	}

	return node;
}


Node *condition_parse_bitwise_xor( Scanner &scanner )
{
	Node *node = condition_parse_bitwise_and( scanner );

	while( scanner.current().type == TokenType_BitXor )
	{
		scanner.next();
		node = conditionAST.add( NodeExpressionBinary( ExpressionBinaryType_BitXor, node,
			condition_parse_bitwise_and( scanner ) ) );
	}

	return node;
}


Node *condition_parse_bitwise_and( Scanner &scanner )
{
	Node *node = condition_parse_equality( scanner );

	while( scanner.current().type == TokenType_BitAnd )
	{
		scanner.next();
		node = conditionAST.add( NodeExpressionBinary( ExpressionBinaryType_BitAnd, node,
			condition_parse_equality( scanner ) ) );
	}

	return node;
}


Node *condition_parse_equality( Scanner &scanner )
{
	Node *node = condition_parse_comparison( scanner );

	while( scanner.current().type == TokenType_Equals ||
		scanner.current().type == TokenType_NotEquals )
	{
		ShaderCompiler::TokenType type = scanner.current().type;
		scanner.next();

		node = conditionAST.add( NodeExpressionBinary(
			type == TokenType_Equals ? ExpressionBinaryType_Equals : ExpressionBinaryType_NotEquals,
			node, condition_parse_comparison( scanner ) ) );
	}

	return node;
}


Node *condition_parse_comparison( Scanner &scanner )
{
	Node *node = condition_parse_bitwise_shift( scanner );

	while( scanner.current().type == TokenType_GreaterThan ||
		scanner.current().type == TokenType_GreaterThanEquals ||
		scanner.current().type == TokenType_LessThan ||
		scanner.current().type == TokenType_LessThanEquals )
	{
		ShaderCompiler::TokenType tokenType = scanner.current().type;
		scanner.next();

		ExpressionBinaryType binaryExpressionType;
		switch( tokenType )
		{
			default:
			case TokenType_GreaterThan: binaryExpressionType = ExpressionBinaryType_Greater; break;
			case TokenType_GreaterThanEquals: binaryExpressionType = ExpressionBinaryType_GreaterEquals; break;
			case TokenType_LessThan: binaryExpressionType = ExpressionBinaryType_Less; break;
			case TokenType_LessThanEquals: binaryExpressionType = ExpressionBinaryType_LessEquals; break;
		}

		node = conditionAST.add( NodeExpressionBinary( binaryExpressionType, node,
			condition_parse_bitwise_shift( scanner ) ) );
	}

	return node;
}


Node *condition_parse_bitwise_shift( Scanner &scanner )
{
	Node *node = condition_parse_add_sub( scanner );

	while( scanner.current().type == TokenType_BitShiftLeft ||
		scanner.current().type == TokenType_BitShiftRight )
	{
		ShaderCompiler::TokenType type = scanner.current().type;
		scanner.next();

		node = conditionAST.add( NodeExpressionBinary(
			type == TokenType_BitShiftLeft ? ExpressionBinaryType_BitShiftLeft : ExpressionBinaryType_BitShiftRight,
			node, condition_parse_add_sub( scanner ) ) );
	}

	return node;
}


Node *condition_parse_add_sub( Scanner &scanner )
{
	Node *node = condition_parse_mul_div_mod( scanner );

	while( scanner.current().type == TokenType_Plus ||
		scanner.current().type == TokenType_Minus )
	{
		ShaderCompiler::TokenType type = scanner.current().type;
		scanner.next();

		node = conditionAST.add( NodeExpressionBinary(
			type == TokenType_Plus ? ExpressionBinaryType_Add : ExpressionBinaryType_Sub,
			node, condition_parse_add_sub( scanner) ) );
	}

	return node;
}


Node *condition_parse_mul_div_mod( Scanner &scanner )
{
	Node *node = condition_parse_prefix_operators( scanner );

	while( scanner.current().type == TokenType_Star ||
		scanner.current().type == TokenType_Slash ||
		scanner.current().type == TokenType_Mod )
	{
		ShaderCompiler::TokenType tokenType = scanner.current().type;
		scanner.next();

		ExpressionBinaryType binaryExpressionType;
		switch( tokenType )
		{
			default:
			case TokenType_Star: binaryExpressionType = ExpressionBinaryType_Mul; break;
			case TokenType_Slash: binaryExpressionType = ExpressionBinaryType_Div; break;
			case TokenType_Mod: binaryExpressionType = ExpressionBinaryType_Mod; break;
		}

		node = conditionAST.add( NodeExpressionBinary( binaryExpressionType, node,
			condition_parse_prefix_operators( scanner ) ) );
	}

	return node;
}


Node *condition_parse_prefix_operators( Scanner &scanner )
{
	switch( scanner.current().type )
	{
		case TokenType_Plus:
		{
			scanner.next();
			return conditionAST.add( NodeExpressionUnary( ExpressionUnaryType_Plus,
				condition_parse_prefix_operators( scanner ) ) );
		}

		case TokenType_Minus:
		{
			scanner.next();
			return conditionAST.add( NodeExpressionUnary( ExpressionUnaryType_Minus,
				condition_parse_prefix_operators( scanner ) ) );
		}

		case TokenType_BitNot:
		{
			scanner.next();
			return conditionAST.add( NodeExpressionUnary( ExpressionUnaryType_BitNot,
				condition_parse_prefix_operators( scanner ) ) );
		}

		case TokenType_Bang:
		{
			scanner.next();
			Node *node = conditionAST.add( NodeExpressionUnary( ExpressionUnaryType_Not,
				condition_parse_prefix_operators( scanner ) ) );
			return node;
		}
	}

	return condition_parse_fundamental( scanner );
}


Node *condition_parse_fundamental( Scanner &scanner )
{
	Token token = scanner.current();
	switch( scanner.current().type )
	{
		case TokenType_Identifier:
		{
			const u32 hash = Hash::hash( token.name );
			return conditionAST.add( NodeBoolean( macros.contains( hash ) ) );
		}
		break;

		case TokenType_Integer:
		{
			scanner.next();
			return conditionAST.add( NodeInteger( token.integer ) );
		}
		break;

		case TokenType_Number:
		{
			scanner.next();
			return conditionAST.add( NodeNumber( token.number ) );
		}
		break;

		case TokenType_True:
		case TokenType_False:
		{
			scanner.next();
			return conditionAST.add( NodeBoolean( token.type == TokenType_True ? true : false ) );
		}
		break;

		case TokenType_LParen:
		{
			token = scanner.next();
			Node *node = conditionAST.add( NodeGroup( condition_parse_expression( scanner ) ) );

			token = scanner.current();
			ErrorIf( token.type != TokenType_RParen, "missing closing ')' on group" );

			scanner.next();
			return node;
		};
		break;

		default:
		{
			Error( "unexpected symbol (token: %d)", token.type );
		}
		break;
	}

	Error( "unreacheable" );
	return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct ExpressionValue
{
	i64 integer = 0LL;
	double number = 0.0;
	bool isInteger = false;
};

static ExpressionValue expression_evaluate_value( Node *node );
static bool expression_evaluate_condition( Node *node );


static ExpressionValue expression_evaluate_value( Node *node )
{
	Assert( node != nullptr );
	const double eps = 0.000001;

	switch( node->nodeType )
	{
		case NodeType_ExpressionTernary:
		{
			// expr1 ? expr2 : expr3
			NodeExpressionTernary *n = reinterpret_cast<NodeExpressionTernary *>( node );
			if( expression_evaluate_condition( n->expr1 ) )
			{
				return expression_evaluate_value( n->expr2 );
			}
			else
			{
				return expression_evaluate_value( n->expr3 );
			}
		}
		break;

		case NodeType_ExpressionBinary:
		{
			NodeExpressionBinary *n = reinterpret_cast<NodeExpressionBinary *>( node );

			ExpressionValue value1 = expression_evaluate_value( n->expr1 );
			ExpressionValue value2 = expression_evaluate_value( n->expr2 );
			ExpressionValue valueOut;

			if( n->exprType == ExpressionBinaryType_Add )
			{
				// ( expr1 ) + ( expr2 )
				valueOut.integer = value1.integer + value2.integer;
				valueOut.number = value1.number + value2.number;
				valueOut.isInteger = value1.isInteger && value2.isInteger;
			}
			else if( n->exprType == ExpressionBinaryType_Sub )
			{
				// ( expr1 ) - ( expr2 )
				valueOut.integer = value1.integer - value2.integer;
				valueOut.number = value1.number - value2.number;
				valueOut.isInteger = value1.isInteger && value2.isInteger;
			}
			else if( n->exprType == ExpressionBinaryType_Mul )
			{
				// ( expr1 ) * ( expr2 )
				valueOut.integer = value1.integer * value2.integer;
				valueOut.number = value1.number * value2.number;
				valueOut.isInteger = value1.isInteger && value2.isInteger;
			}
			else if( n->exprType == ExpressionBinaryType_Div )
			{
				// ( expr1 ) / ( expr2 )
				valueOut.integer = value1.integer / value2.integer;
				valueOut.number = value1.number / value2.number;
				valueOut.isInteger = value1.isInteger && value2.isInteger;
			}
			else if( n->exprType == ExpressionBinaryType_Mod )
			{
				// ( expr1 ) % ( expr2 )
				valueOut.integer = value1.integer % value2.integer;
				valueOut.number = fmod( value1.number, value2.number );
				valueOut.isInteger = value1.isInteger && value2.isInteger;
			}
			else if( n->exprType == ExpressionBinaryType_Equals )
			{
				// ( expr1 ) == ( expr2 )
				bool r = false;
				if( value1.isInteger && value2.isInteger )
				{
					r = ( value1.integer == value2.integer );
				}
				else if( value1.isInteger && !value2.isInteger )
				{
					r = fabs( static_cast<double>( value1.integer ) - value2.number ) < eps;
				}
				else if( !value1.isInteger && value2.isInteger )
				{
					r = fabs( value1.number - static_cast<double>( value2.integer ) ) < eps;
				}
				else
				{
					r = fabs( value1.number - value2.number ) < eps;
				}

				valueOut.integer = static_cast<i64>( r );
				valueOut.number = static_cast<double>( r );
				valueOut.isInteger = true;
			}
			else if( n->exprType == ExpressionBinaryType_Greater )
			{
				// ( expr1 ) > ( expr2 )
				bool r = false;
				if( value1.isInteger && value2.isInteger )
				{
					r = ( value1.integer > value2.integer );
				}
				else if( value1.isInteger && !value2.isInteger )
				{
					r = static_cast<double>( value1.integer ) > value2.number;
				}
				else if( !value1.isInteger && value2.isInteger )
				{
					r = value1.number > static_cast<double>( value2.integer );
				}
				else
				{
					r = value1.number > value2.number;
				}

				valueOut.integer = static_cast<i64>( r );
				valueOut.number = static_cast<double>( r );
				valueOut.isInteger = true;
			}
			else if( n->exprType == ExpressionBinaryType_GreaterEquals )
			{
				// ( expr1 ) >= ( expr2 )
				bool r = false;
				if( value1.isInteger && value2.isInteger )
				{
					r = ( value1.integer >= value2.integer );
				}
				else if( value1.isInteger && !value2.isInteger )
				{
					r = static_cast<double>( value1.integer ) >= value2.number;
				}
				else if( !value1.isInteger && value2.isInteger )
				{
					r = value1.number >= static_cast<double>( value2.integer );
				}
				else
				{
					r = value1.number >= value2.number;
				}

				valueOut.integer = static_cast<i64>( r );
				valueOut.number = static_cast<double>( r );
				valueOut.isInteger = true;
			}
			else if( n->exprType == ExpressionBinaryType_Less )
			{
				// ( expr1 ) < ( expr2 )
				bool r = false;
				if( value1.isInteger && value2.isInteger )
				{
					r = ( value1.integer < value2.integer );
				}
				else if( value1.isInteger && !value2.isInteger )
				{
					r = static_cast<double>( value1.integer ) < value2.number;
				}
				else if( !value1.isInteger && value2.isInteger )
				{
					r = value1.number < static_cast<double>( value2.integer );
				}
				else
				{
					r = value1.number < value2.number;
				}

				valueOut.integer = static_cast<i64>( r );
				valueOut.number = static_cast<double>( r );
				valueOut.isInteger = true;
			}
			else if( n->exprType == ExpressionBinaryType_LessEquals )
			{
				// ( expr1 ) <= ( expr2 )
				bool r = false;
				if( value1.isInteger && value2.isInteger )
				{
					r = ( value1.integer <= value2.integer );
				}
				else if( value1.isInteger && !value2.isInteger )
				{
					r = static_cast<double>( value1.integer ) <= value2.number;
				}
				else if( !value1.isInteger && value2.isInteger )
				{
					r = value1.number <= static_cast<double>( value2.integer );
				}
				else
				{
					r = value1.number <= value2.number;
				}

				valueOut.integer = static_cast<i64>( r );
				valueOut.number = static_cast<double>( r );
				valueOut.isInteger = true;
			}
			else if( n->exprType == ExpressionBinaryType_Or )
			{
				// ( expr1 ) || ( expr2 )
				bool r = false;

				if( value1.isInteger && value2.isInteger ) { r = value1.integer || value2.integer; } else
				if( value1.isInteger && !value2.isInteger ) { r = value1.integer || value2.number; } else
				if( !value1.isInteger && value2.isInteger ) { r = value1.number || value2.integer; } else
				if( !value1.isInteger && !value2.isInteger ) { r = value1.number || value2.number; }

				valueOut.integer = static_cast<i64>( r );
				valueOut.number = static_cast<double>( r );
				valueOut.isInteger = true;
			}
			else if( n->exprType == ExpressionBinaryType_And )
			{
				// ( expr1 ) && ( expr2 )
				bool r = false;

				if( value1.isInteger && value2.isInteger ) { r = value1.integer && value2.integer; } else
				if( value1.isInteger && !value2.isInteger ) { r = value1.integer && value2.number; } else
				if( !value1.isInteger && value2.isInteger ) { r = value1.number && value2.integer; } else
				if( !value1.isInteger && !value2.isInteger ) { r = value1.number && value2.number; }

				valueOut.integer = static_cast<i64>( r );
				valueOut.number = static_cast<double>( r );
				valueOut.isInteger = true;
			}
			else if( n->exprType == ExpressionBinaryType_BitOr )
			{
				// ( expr1 ) | ( expr2 )
				ErrorIf( !( value1.isInteger && value2.isInteger ), "Cannot bitwise or on non-integer expression!" );
				valueOut.integer = ( value1.integer ) | ( value2.integer );
				valueOut.number = static_cast<double>( valueOut.integer );
				valueOut.isInteger = true;
			}
			else if( n->exprType == ExpressionBinaryType_BitXor )
			{
				// ( expr1 ) ^ ( expr2 )
				ErrorIf( !( value1.isInteger && value2.isInteger ), "Cannot bitwise xor on non-integer expression!" );
				valueOut.integer = ( value1.integer ) ^ ( value2.integer );
				valueOut.number = static_cast<double>( valueOut.integer );
				valueOut.isInteger = true;
			}
			else if( n->exprType == ExpressionBinaryType_BitAnd )
			{
				// ( expr1 ) & ( expr2 )
				ErrorIf( !( value1.isInteger && value2.isInteger ), "Cannot bitwise and on non-integer expression!" );
				valueOut.integer = ( value1.integer ) & ( value2.integer );
				valueOut.number = static_cast<double>( valueOut.integer );
				valueOut.isInteger = true;
			}
			else if( n->exprType == ExpressionBinaryType_BitShiftLeft )
			{
				// ( expr1 ) << ( expr2 )
				ErrorIf( !( value1.isInteger && value2.isInteger ), "Cannot bitshift left on non-integer expression!" );
				valueOut.integer = ( value1.integer ) << ( value2.integer );
				valueOut.number = static_cast<double>( valueOut.integer );
				valueOut.isInteger = true;
			}
			else if( n->exprType == ExpressionBinaryType_BitShiftRight )
			{
				// ( expr1 ) >> ( expr2 )
				ErrorIf( !( value1.isInteger && value2.isInteger ), "Cannot bitshift left on non-integer expression!" );
				valueOut.integer = ( value1.integer ) >> ( value2.integer );
				valueOut.number = static_cast<double>( valueOut.integer );
				valueOut.isInteger = true;
			}
			else
			{
				Error( "Unexpected binary operator on expression!" );
			}

			return valueOut;
		}
		break;

		case NodeType_ExpressionUnary:
		{
			NodeExpressionUnary *n = reinterpret_cast<NodeExpressionUnary *>( node );
			ExpressionValue valueOut = expression_evaluate_value( n->expr );

			if( n->exprType == ExpressionUnaryType_Plus )
			{
				// +(expr)
				// ... do nothing
			}
			else if( n->exprType == ExpressionUnaryType_Minus )
			{
				// -(expr)
				valueOut.integer *= -1;
				valueOut.number *= -1.0;
			}
			else if( n->exprType == ExpressionUnaryType_Not )
			{
				// !(expr)
				if( valueOut.isInteger )
				{
					valueOut.integer = !( valueOut.integer );
					valueOut.number = static_cast<double>( valueOut.integer );
				}
				else
				{
					valueOut.integer = !( static_cast<i64>( valueOut.number ) );
					valueOut.number = static_cast<double>( valueOut.integer );
				}
				valueOut.isInteger = true;
			}
			else if( n->exprType == ExpressionUnaryType_BitNot )
			{
				// ~(expr)
				ErrorIf( !valueOut.isInteger, "Cannot bitwise not on non-integer expression!" );
				valueOut.integer = ~( valueOut.integer );
				valueOut.number = static_cast<double>( valueOut.number );
			}
			else
			{
				Error( "Unexpected unary operator on expression!" );
			}

			return valueOut;
		}
		break;

		case NodeType_Group:
		{
			return expression_evaluate_value( reinterpret_cast<NodeGroup *>( node )->expr );
		}
		break;

		case NodeType_Boolean:
		{
			NodeBoolean *n = reinterpret_cast<NodeBoolean *>( node );
			return ExpressionValue { static_cast<i64>( n->boolean ),
				static_cast<double>( n->boolean ), true };
		}
		break;

		case NodeType_Integer:
		{
			NodeInteger *n = reinterpret_cast<NodeInteger *>( node );
			return ExpressionValue { static_cast<i64>( n->integer ),
				static_cast<double>( n->integer ), true };
		}
		break;

		case NodeType_Number:
		{
			NodeNumber *n = reinterpret_cast<NodeNumber *>( node );
			return ExpressionValue { static_cast<i64>( n->number ),
				static_cast<double>( n->number ), false };
		}
		break;

		default:
		{
			Error( "Expression must be constexpr!" );
		}
		break;
	}

	// Unreachable
	return ExpressionValue { 0, 0.0, true };
}


static bool expression_evaluate_condition( Node *node )
{
	Assert( node != nullptr );
	ExpressionValue value = expression_evaluate_value( node );
	return value.isInteger ? static_cast<bool>( value.integer ) : static_cast<bool>( value.number );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void parse_directive_include( String &input, String &output, Scanner &scanner,
	const char *directory, const usize start )
{
	// Expect space after #include
	Token token = scanner.next( false );

	// Find < or "
	for( ;; )
	{
		token = scanner.next( false );
		if( token.type == TokenType_Space || token.type == TokenType_Tab ) { continue; }
		if( token.type == TokenType_LessThan || token.type == TokenType_Quote ) { break; }
		Error( "Unexpected token '%.*s' after #include", static_cast<int>( token.name.length ), token.name.data );
	}

	// Extract Path
	String path = "";
	usize end = start;
	for( ;; )
	{
		token = scanner.next( false );

		ErrorIf( token.type == TokenType_Newline || token.type == TokenType_EndOfFile,
			"#include must be terminated with \" or > before new line!" );

		if( token.type == TokenType_GreaterThan || token.type == TokenType_Quote )
		{
			end = token.position;
			break;
		}

		path.append( token.name );
	}
	path.trim();
	path.replace( "\\", "/" );

	// Skip built-in #includes
	if( path.equals( "shader_api.hpp" ) ) { return; }

	// Shader Libraries
	const char *directoryShaderlib = "source" SLASH;
	constexpr const char *shaderlibs[] =
	{
		"shaderlib/vertex_formats.h",
		"shaderlib/math.h",
		"shaderlib/color.h",
	};

	bool pathIsShaderLib = false;
	for( const char *lib : shaderlibs )
	{
		if( path.equals( lib ) ) { pathIsShaderLib = true; break; }
	}

	// Path Root
	path.insert( 0, pathIsShaderLib ? directoryShaderlib : directory );

	// Load File & Preprocess Contents
	String contents;
	const u32 hash = Hash::hash( path );
	if( includes.contains( hash ) )
	{
		// #include <...> already seen -- fetch it from cache
		Include &include = includes.get( hash );
		if( !( include.flags & IncludeFlag_PragmaOnce ) ) { preprocess_input( path, include.contents, output ); }
	}
	else
	{
		// #include <...> not yet seen -- load it
		ErrorIf( !contents.load( path ), "Failed open #include file '%s'", path.cstr() );
		includes.set( hash, Include { contents } );
		preprocess_input( path, contents, output );
	}

	// Insert Header
	input.remove( start, end - start );

	// scanner.init() is required since input.insert() could grow & invalidate pointers
	scanner.init( input.cstr(), start );
}


static void parse_directive_pragma( String &input, String &output, Scanner &scanner, const u32 hash )
{
	// Expect space after #pragma
	Token token = scanner.next( false );

	// Find #pragma ... directives
	for( ;; )
	{
		token = scanner.next( false );
		if( token.type == TokenType_Newline || token.type == TokenType_EndOfFile ) { return; }

		// #pragma once
		if( token.type == TokenType_Directive_Once ) { includes.get( hash ).flags |= IncludeFlag_PragmaOnce; }
	}
}


static void parse_directive_define( String &input, String &output, Scanner &scanner )
{
	static HashMap<u32, int> parameters;
	static char parameter[256];
	parameters.clear();

	Macro macro;
	scanner.next( false );

	// Macro Name
	for( ;; )
	{
		Token token = scanner.next( false );
		if( token.type == TokenType_Space || token.type == TokenType_Tab ) { continue; }
		if( token.type == TokenType_Identifier ) { macro.name = token.name; break; }
		Error( "Invalid #define directive!" );
	}

	// Macro Parameters
	if( scanner.next( false ).type == TokenType_LParen )
	{
		for( bool sawParameter = false; true; )
		{
			Token token = scanner.next( true );

			if( token.type == TokenType_Identifier )
			{
				const u32 hash = Hash::hash( token.name );
				ErrorIf( parameters.contains( hash ), "#define: cannot have duplicate parameter names!" );
				parameters.set( hash, macro.parameters );
				macro.parameters++;
				sawParameter = true;
				continue;
			}

			if( token.type == TokenType_Comma )
			{
				ErrorIf( !sawParameter, "#define: unexpected ',' in parameter list!" );
				sawParameter = false;
				continue;
			}

			if( token.type == TokenType_RParen )
			{
				break;
			}

			Error( "Invalid #define directive!" );
		}
	}
	else
	{
		scanner.back();
	}

	// Macro Definition
	Token token, previous;
	for( ;; )
	{
		previous = token;
		token = scanner.next( false );

		if( token.type == TokenType_Escape ) { continue; }

		if( token.type == TokenType_Newline )
		{
			if( previous.type == TokenType_Escape ) { macro.definition.trim(); continue; }
			break;
		}

		if( token.type == TokenType_EndOfFile )
		{
			break;
		}

		if( token.type == TokenType_Identifier )
		{
			const u32 hash = Hash::hash( token.name );
			if( parameters.contains( hash ) )
			{
				const int index = parameters.get( hash );
				macro.definition.append( "@" ).append( index );
				continue;
			}
		}

		if( token.type == TokenType_Tab || token.type == TokenType_Space )
		{
			macro.definition.append( " " );
			continue;
		}

		ErrorIf( token.type >= TOKENTYPE_DIRECTIVE_FIRST && token.type < TOKENTYPE_DIRECTIVE_LAST,
			"Shader preprocessor: #define does not allow preprocessor directives! %.*s",
			token.name.length, token.name.data );

		macro.definition.append( token.name );
	}

	// Register Macro
	macro.definition.trim();
	macros.set( Hash::hash( macro.name ), macro );
}


static void parse_directive_undefine( String &input, String &output, Scanner &scanner )
{
	// Expect space after #undef
	Token token = scanner.next( false );

	// Find & Remove Macros
	for( ;; )
	{
		token = scanner.next( false );

		if( token.type == TokenType_Identifier )
		{
			const u32 hash = Hash::hash( token.name );
			if( macros.contains( hash ) ) { macros.remove( hash ); }
		}

		if( token.type == TokenType_Newline || token.type == TokenType_EndOfFile ) { return; }
	}
}


static void parse_directive_macro( String &input, String &output, Scanner &scanner,
	const char *name, const u32 hash, const usize start, const bool condition )
{
	static List<String> parameters;
	static String substitute;
	parameters.clear();
	substitute.clear();

	usize end = start + strlen( name );
	const Macro &macro = macros.get( hash );

	// Parameters
	if( macro.parameters > 0 )
	{
		ErrorIf( scanner.next( false ).type != TokenType_LParen,
			"macro '%s' expects parameters, none provided", name );

		int depth = 0;
		String parameter = "";

		for( ;; )
		{
			Token token = scanner.next( false );

			// Parenthesis (allow nesting for ',')
			if( token.type == TokenType_LParen ) { depth++; }
			if( token.type == TokenType_RParen )
			{
				depth--;
				if( depth < 0 )
				{
					parameter.trim();
					parameters.add( parameter );
					parameter.clear();
					end = token.position;
					break;
				}
			}

			// Commas
			if( token.type == TokenType_Comma && depth == 0 )
			{
				parameter.trim();
				parameters.add( parameter );
				parameter.clear();
				continue;
			}

			// Parameter Tokens
			parameter.append( token.name );
		}

		ErrorIf( parameters.count() < static_cast<usize>( macro.parameters ),
			"macro '%s' given %llu parameters, expects %d", name, parameters.count(), macro.parameters );
	}

	// Substitute Parameters
	static char replace[16];
	substitute = macro.definition;
	for( int i = 0; i < macro.parameters; i++ )
	{
		snprintf( replace, sizeof( replace ), "@%d", i );
		substitute.replace( replace, parameters[i] );
	}

	// When expanding a macro in an #if/#elif, replace empty macros with '1'
	if( condition && substitute.is_empty() ) { substitute = "1"; }

	// Insert Definition
	input.remove( start, end - start );
	input.insert( start, substitute );

	// scanner.init() is required since input.insert() could grow & invalidate pointers
	Assert( start < input.length_bytes() );
	scanner.init( input.cstr(), start );
}


static void parse_directive_if_defined_undefined( String &input, String &output, Scanner &scanner,
	ShaderCompiler::TokenType type )
{
	const bool ifdef = ( type == TokenType_Directive_IfDefined );
	const char *directive = ifdef ? "#ifdef" : "#ifndef";

	// Increment Depth
	branchDepth++;

	// Expect space after #ifdef/#ifndef
	Token token = scanner.next( false );
	bool succeeded = false;

	// Find Macro
	for( ;; )
	{
		token = scanner.next( false );

		if( token.type == TokenType_Identifier )
		{
			const u32 hash = Hash::hash( token.name );
			succeeded = ifdef ? macros.contains( hash ) : !macros.contains( hash );

			// Find next line
			for( ;; )
			{
				token = scanner.next( false );
				if( token.type == TokenType_Newline || token.type == TokenType_EndOfFile ) { break; }
				if( token.type == TokenType_Space || token.type == TokenType_Tab ) { continue; };
				Error( "Unexpected token '%.*s' after %s macro", static_cast<int>( token.name.length ),
					token.name.data, directive );
			}

			break;
		}

		if( token.type == TokenType_Space || token.type == TokenType_Tab )
		{
			continue;
		}

		Error( "Unexpected token '%.*s' in %s", static_cast<int>( token.name.length ),
			token.name.data, directive );
	}

	// If true: return
	if( succeeded ) { branchEvaluateElseIf = false; return; }

	// If false: seek to #endif, #else, or #elif
	const int branchDepthThis = branchDepth;
	for( ;; )
	{
		token = scanner.next( true );

		if( token.type == TokenType_Directive_If ||
			token.type == TokenType_Directive_IfDefined ||
			token.type == TokenType_Directive_IfUndefined )
		{
			branchDepth++; // any nested #if, #ifdef, #ifndef need to be skipped over
			continue;
		}

		if( token.type == TokenType_Directive_Else )
		{
			if( branchDepth == branchDepthThis ) { return; } // else will always be true if #ifdef/#ifndef was false
			continue;
		}

		if( token.type == TokenType_Directive_ElseIf )
		{
			if( branchDepth == branchDepthThis )
			{
				scanner.back();
				branchEvaluateElseIf = true; // defer evaluation to outer loop
				return;
			}
			continue;
		}

		if( token.type == TokenType_Directive_EndIf )
		{
			branchDepth--;
			if( branchDepth == branchDepthThis - 1 ) { return; } // end of our conditional block
			continue;
		}

		ErrorIf( token.type == TokenType_EndOfFile, "Reached end of file before closing %s", directive );
	}
}


static void parse_directive_if( String &input, String &output, Scanner &scanner, const ShaderCompiler::TokenType type )
{
	const bool elif = ( type == TokenType_Else );
	const char *directive = elif ? "#elif" : "#if";
	char scratch[256];

	// Increment Depth (#if only)
	if( !elif ) { branchDepth++; }

	// Expect space after #if/#elif
	Token token = scanner.next( false );
	bool succeeded = false;

	// Build Condition Line
	conditionLine.clear();
	for( ;; )
	{
		token = scanner.next( false );

		if( token.type == TokenType_Identifier )
		{
			const u32 hash = Hash::hash( token.name );
			if( macros.contains( hash ) )
			{
				snprintf( scratch, sizeof( scratch ), "%.*s", static_cast<int>( token.name.length ),
					token.name.data );
				parse_directive_macro( input, conditionLine, scanner, scratch, hash, token.start, true );
				continue;
			}
			else
			{
				conditionLine.append( "0" );
				continue;
			}
		}

		if( token.type == TokenType_Directive_Defined )
		{
			// Expands: defined( MACRO ) -> ( 0/1 )
			continue;
		}

		if( token.type == TokenType_Directive_Undefined )
		{
			// Expands: undefined( MACRO ) -> !( 0/1 )
			conditionLine.append( "!" );
			continue;
		}

		if( token.type == TokenType_Newline )
		{
			break;
		}

		conditionLine.append( token.name );
	}

	// Build Condition AST & Evaluate
	Scanner conditionScanner;
	conditionScanner.init( conditionLine, 0LLU );
	conditionScanner.next( true );
	Node *expression = condition_parse_expression( conditionScanner );
	Assert( expression != nullptr );
	succeeded = expression_evaluate_condition( expression );
	conditionAST.clear();

	// If true: return
	if( succeeded ) { branchEvaluateElseIf = false; return; }

	// If false: seek to #endif, #else, or #elif
	const int branchDepthThis = branchDepth;
	for( ;; )
	{
		token = scanner.next( true );

		if( token.type == TokenType_Directive_If ||
			token.type == TokenType_Directive_IfDefined ||
			token.type == TokenType_Directive_IfUndefined )
		{
			branchDepth++; // any nested #if, #ifdef, #ifndef need to be skipped over
			continue;
		}

		if( token.type == TokenType_Directive_Else )
		{
			if( branchDepth == branchDepthThis ) { return; } // else will always be true if #if/#elif was false
			continue;
		}

		if( token.type == TokenType_Directive_ElseIf )
		{
			if( branchDepth == branchDepthThis )
			{
				scanner.back();
				branchEvaluateElseIf = true; // defer evaluation to outer loop
				return;
			}
			continue;
		}

		if( token.type == TokenType_Directive_EndIf )
		{
			branchDepth--;
			if( branchDepth == branchDepthThis - 1 ) { return; } // end of our conditional block
			continue;
		}

		ErrorIf( token.type == TokenType_EndOfFile, "Reached end of file before closing %s", directive );
	}
}


static void parse_directive_else( String &input, String &output, Scanner &scanner )
{
	// NOTE: The only time this function can run is if the conditional branch was true!

	// Seek to #endif
	const int branchDepthThis = branchDepth;
	for( ;; )
	{
		Token token = scanner.next( true );

		if( token.type == TokenType_Directive_If ||
			token.type == TokenType_Directive_IfDefined ||
			token.type == TokenType_Directive_IfUndefined )
		{
			branchDepth++; // any nested #if, #ifdef, #ifndef need to be skipped over
			continue;
		}

		if( token.type == TokenType_Directive_EndIf )
		{
			branchDepth--;
			if( branchDepth == branchDepthThis - 1 ) { return; } // end of our conditional block
			continue;
		}

		ErrorIf( token.type == TokenType_EndOfFile, "Reached end of file before closing #else" );
	}
}


static void parse_directive_endif( String &input, String &output, Scanner &scanner )
{
	branchDepth--;
	ErrorIf( branchDepth < 0, "Unexpected '#endif'!" );
}


void preprocess_input( const char *path, String &input, String &output )
{
	Scanner scanner;
	scanner.init( input, 0LLU );
	scanner.mode = Scanner::ScannerMode_Preprocessor;

	bool inCommentLine = false;
	bool inCommentBlock = false;

	char scratch[256];
	char directory[PATH_SIZE];
	path_get_directory( directory, sizeof( directory ), path );
	strappend( directory, SLASH );
	const u32 pathHash = Hash::hash( path );

	// Preprocess
	for( ;; )
	{
		const usize u = scanner.position;
		Token token = scanner.next( false );

		if( token.type == TokenType_EndOfFile )
		{
			break;
		}

		if( inCommentLine )
		{
			if( token.type == TokenType_Newline ) { inCommentLine = false; continue; }
			continue;
		}

		if( inCommentBlock )
		{
			if( token.type == TokenType_CommentEnd ) { inCommentBlock = false; continue; }
			continue;
		}

		switch( token.type )
		{
			case TokenType_CommentLine:
			{
				inCommentLine = true;
			}
			continue;

			case TokenType_CommentStart:
			{
				inCommentBlock = true;
			}
			continue;

			case TokenType_Directive_Include:
			{
				parse_directive_include( input, output, scanner, directory, token.start );
			}
			continue;

			case TokenType_Directive_Pragma:
			{
				parse_directive_pragma( input, output, scanner, pathHash );
			}
			continue;

			case TokenType_Directive_Define:
			{
				parse_directive_define( input, output, scanner );
			}
			continue;

			case TokenType_Directive_Undefine:
			{
				parse_directive_undefine( input, output, scanner );
			}
			continue;

			case TokenType_Identifier:
			{
				const u32 hash = Hash::hash( token.name );
				if( macros.contains( hash ) )
				{
					snprintf( scratch, sizeof( scratch ), "%.*s",
						static_cast<int>( token.name.length ), token.name.data );
					parse_directive_macro( input, output, scanner, scratch, hash, token.start, false );
				}
				else
				{
					output.append( token.name );
				}
			}
			continue;

			case TokenType_Directive_IfDefined:
			case TokenType_Directive_IfUndefined:
			{
				parse_directive_if_defined_undefined( input, output, scanner, token.type );
			}
			continue;

			case TokenType_Directive_If:
			{
				parse_directive_if( input, output, scanner, token.type );
			}
			continue;

			case TokenType_Directive_Else:
			{
				// #else is only encountered here if the preceeding #if evaluated true
				parse_directive_else( input, output, scanner );
			}
			continue;

			case TokenType_Directive_ElseIf:
			{
				if( branchEvaluateElseIf )
				{
					// If we encounter an #elseif, evaluate it as #if (i.e. the condition chain has not yet succeeded)
					branchEvaluateElseIf = false;
					parse_directive_if( input, output, scanner, token.type );
				}
				else
				{
					// Otherwise, evaluate it as an unsuccessful #else (i.e. #elif ... #endif block is skipped)
					parse_directive_else( input, output, scanner );
				}
			}
			continue;

			case TokenType_Directive_EndIf:
			{
				// #endif is only encountered here if closing #else or #elif
				parse_directive_endif( input, output, scanner );
			}
			continue;

			default:
			{
				if( token.name.length > 0 ) { output.append( token.name ); }
			}
			continue;
		}
	}
}


bool preprocess_shader( const char *path, String &output,
	const char **pipelineMacros, int pipelineMacrosCount )
{
	Timer timer;

	// Setup
	macros.clear();
	output.clear();
	char directory[PATH_SIZE];
	path_get_directory( directory, sizeof( directory ), path );
	for( auto &include : includes ) { include.value.flags = IncludeFlag_None; }

	// Load Shader
	String contents;
	const u32 hash = Hash::hash( path );
	if( includes.contains( hash ) )
	{
		// Shader already seen (previously from previous #include <...> or load) -- fetch from cache
		contents = includes.get( hash ).contents;
	}
	else
	{
		// Shader not yet seen -- load it
		ErrorReturnIf( !contents.load( path ), false, "Failed to read shader file: %s", path );
		includes.set( hash, Include { contents } );
	}

	// Insert Macros
	static char scratch[256];
	for( int i = 0; i < pipelineMacrosCount; i++ )
	{
		if( pipelineMacros[i][0] == '\0' ) { continue; }
		snprintf( scratch, sizeof( scratch ), "#define %s\n", pipelineMacros[i] );
		contents.insert( 0LLU, scratch );
	}

	// Preprocess
	preprocess_input( path, contents, output );

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////