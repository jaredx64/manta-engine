#include <build/shaders/compiler.parser.hpp>

#include <build/filesystem.hpp>
#include <build/math.hpp>

namespace ShaderCompiler
{
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const char *Primitives[] =
{
	"void",             // Primitive_Void
	"bool",             // Primitive_Bool
	"bool2",            // Primitive_Bool2
	"bool3",            // Primitive_Bool3
	"bool4",            // Primitive_Bool4
	"int",              // Primitive_Int
	"int2",             // Primitive_Int2
	"int3",             // Primitive_Int3
	"int4",             // Primitive_Int4
	"uint",             // Primitive_UInt
	"uint2",            // Primitive_UInt2
	"uint3",            // Primitive_UInt3
	"uint4",            // Primitive_UInt4
	"float",            // Primitive_Float
	"float2",           // Primitive_Float2
	"float3",           // Primitive_Float3
	"float4",           // Primitive_Float4
	"float2x2",         // Primitive_Float2x2
	"float3x3",         // Primitive_Float3x3
	"float4x4",         // Primitive_Float4x4
	"double",           // Primitive_Double
	"double2",          // Primitive_Double2
	"double3",          // Primitive_Double3
	"double4",          // Primitive_Double4
	"double2x2",        // Primitive_Double2x2
	"double3x3",        // Primitive_Double3x3
	"double4x4",        // Primitive_Double4x4
	"Texture1D",        // Primitive_Texture1D
	"Texture1DArray",   // Primitive_Texture1DArray
	"Texture2D",        // Primitive_Texture2D
	"Texture2DArray",   // Primitive_Texture2DArray
	"Texture3D",        // Primitive_Texture3D
	"TextureCube",      // Primitive_TextureCube
	"TextureCubeArray", // Primitive_TextureCubeArray
};
static_assert( ARRAY_LENGTH( Primitives ) == PRIMITIVE_COUNT, "Missing Primitive!" );


const char *Intrinsics[] =
{
	// Trigonometric Functions
	"cos",                     // Intrinsic_Cos
	"sin",                     // Intrinsic_Sin
	"tan",                     // Intrinsic_Tan
	"sinh",                    // Intrinsic_Sinh
	"cosh",                    // Intrinsic_Cosh
	"tanh",                    // Intrinsic_Tanh
	"asin",                    // Intrinsic_ASin
	"acos",                    // Intrinsic_ACos
	"atan",                    // Intrinsic_ATan
	"atan2",                   // Intrinsic_ATan2

	// Exponential and Logarithmic Functions
	"exp",                     // Intrinsic_Exp
	"exp2",                    // Intrinsic_Exp2
	"log",                     // Intrinsic_Log
	"log2",                    // Intrinsic_Log2

	// Conversion Functions
	"degrees",                 // Intrinsic_Degrees
	"radians",                 // Intrinsic_Radians
	"round",                   // Intrinsic_Round
	"trunc",                   // Intrinsic_Trunc
	"ceil",                    // Intrinsic_Ceil
	"floor",                   // Intrinsic_Floor

	// Arithmetic Functions
	"abs",                     // Intrinsic_Abs
	"pow",                     // Intrinsic_Pow
	"sqrt",                    // Intrinsic_Sqrt
	"rsqrt",                   // Intrinsic_RSqrt
	"clamp",                   // Intrinsic_Clamp
	"max",                     // Intrinsic_Max
	"min",                     // Intrinsic_Min
	"mod",                     // Intrinsic_Mod
	"frac",                    // Intrinsic_Frac
	"ldexp",                   // Intrinsic_Ldexp
	"fma",                     // Intrinsic_Fma
	"sign",                    // Intrinsic_Sign
	"saturate",                // Intrinsic_Saturate
	"ddx",                     // Intrinsic_DDx
	"ddy",                     // Intrinsic_DDy
	"ddx_coarse",              // Intrinsic_DDxCoarse
	"ddx_fine",                // Intrinsic_DDxFine
	"ddy_coarse",              // Intrinsic_DDyCoarse
	"ddy_fine",                // Intrinsic_DDyFine

	// Vector and Matrix Functions
	"mul",                     // Intrinsic_Mul
	"length",                  // Intrinsic_Length
	"distance",                // Intrinsic_Distance
	"dot",                     // Intrinsic_Dot
	"cross",                   // Intrinsic_Cross
	"normalize",               // Intrinsic_Normalize
	"reflect",                 // Intrinsic_Reflect
	"refract",                 // Intrinsic_Refract
	"faceforward",             // Intrinsic_Faceforward
	"transpose",               // Intrinsic_Transpose
	"determinant",             // Intrinsic_Determinant
	"lerp",                    // Intrinsic_Lerp
	"step",                    // Intrinsic_Step
	"smoothstep",              // Intrinsic_Smoothstep

	// Bitwise Operations
	"bit_count",               // Intrinsic_BitCount
	"bit_firsthigh",           // Intrinsic_BitFirstHigh
	"bit_firstlow",            // Intrinsic_BitFirstLow
	"bit_reverse",             // Intrinsic_BitReverse
	"atomic_add",              // Intrinsic_AtomicAdd
	"atomic_compare_exchange", // Intrinsic_AtomicCompareExchange
	"atomic_exchange",         // Intrinsic_AtomicExchange
	"atomic_max",              // Intrinsic_AtomicMax
	"atomic_min",              // Intrinsic_AtomicMin
	"atomic_and",              // Intrinsic_AtomicAnd
	"atomic_or",               // Intrinsic_AtomicOr
	"atomic_xor",              // Intrinsic_AtomicXor

	// Texture Sampling Functions
	"sample_texture1D",        // Intrinsic_SampleTexture1D
	"sample_texture1DArray",   // Intrinsic_SampleTexture1DArray
	"sample_texture2D",        // Intrinsic_SampleTexture2D
	"sample_texture2DArray",   // Intrinsic_SampleTexture2DArray
	"sample_texture2DLevel",   // Intrinsic_SampleTexture2DLevel
	"sample_texture3D",        // Intrinsic_SampleTexture3D
	"sample_textureCube",      // Intrinsic_SampleTextureCube
	"sample_textureCubeArray", // Intrinsic_SampleTextureCubeArray
};
static_assert( ARRAY_LENGTH( Intrinsics ) == INTRINSIC_COUNT, "Missing Intrinsic!" );


const char *StructTypeNames[] =
{
	"struct",
	"cbuffer",
	"vertex_input",
	"vertex_output",
	"fragment_input",
	"fragment_output",
	"compute_input",
	"compute_output",
};
static_assert( ARRAY_LENGTH( StructTypeNames ) == STRUCTTYPE_COUNT, "Missing StructType!" );


const char *TextureTypeNames[] =
{
	"texture1D",
	"texture1DArray",
	"texture2D",
	"texture2DArray",
	"texture3D",
	"textureCube",
	"textureCubeArray",
};
static_assert( ARRAY_LENGTH( TextureTypeNames ) == TEXTURETYPE_COUNT, "Missing TextureType!" );


const Keyword KeywordNames[] =
{
	{ "in",               TokenType_In },
	{ "out",              TokenType_Out },
	{ "inout",            TokenType_InOut },
	{ "true",             TokenType_True },
	{ "false",            TokenType_False },
	{ "const",            TokenType_Const },
	{ "return",           TokenType_Return },
	{ "break",            TokenType_Break },
	{ "switch",           TokenType_Switch },
	{ "case",             TokenType_Case },
	{ "default",          TokenType_Default },
	{ "discard",          TokenType_Discard },
	{ "if",               TokenType_If },
	{ "else",             TokenType_Else },
	{ "while",            TokenType_While },
	{ "do",               TokenType_Do },
	{ "for",              TokenType_For },
	{ "struct",           TokenType_Struct },
	{ "cbuffer",          TokenType_CBuffer },
	{ "vertex_input",     TokenType_VertexInput },
	{ "vertex_output",    TokenType_VertexOutput },
	{ "fragment_input",   TokenType_FragmentInput },
	{ "fragment_output",  TokenType_FragmentOutput },
	{ "compute_input",    TokenType_ComputeInput },
	{ "compute_output",   TokenType_ComputeOutput },
	{ "texture1D",        TokenType_Texture1D },
	{ "texture1DArray",   TokenType_Texture1DArray },
	{ "texture2D",        TokenType_Texture2D },
	{ "texture2DArray",   TokenType_Texture2DArray },
	{ "texture3D",        TokenType_Texture3D },
	{ "textureCube",      TokenType_TextureCube },
	{ "textureCubeArray", TokenType_TextureCubeArray },
	{ "target",           TokenType_Target },
	{ "semantic",         TokenType_Semantic },
	{ "POSITION",         TokenType_POSITION },
	{ "TEXCOORD",         TokenType_TEXCOORD },
	{ "NORMAL",           TokenType_NORMAL },
	{ "DEPTH",            TokenType_DEPTH },
	{ "COLOR",            TokenType_COLOR },
	{ "format",           TokenType_InputFormat },
	{ "UNORM8",           TokenType_UNORM8 },
	{ "UNORM16",          TokenType_UNORM16 },
	{ "UNORM32",          TokenType_UNORM32 },
	{ "SNORM8",           TokenType_SNORM8 },
	{ "SNORM16",          TokenType_SNORM16 },
	{ "SNORM32",          TokenType_SNORM32 },
	{ "UINT8",            TokenType_UINT8 },
	{ "UINT16",           TokenType_UINT16 },
	{ "UINT32",           TokenType_UINT32 },
	{ "SINT8",            TokenType_SINT8 },
	{ "SINT16",           TokenType_SINT16 },
	{ "SINT32",           TokenType_SINT32 },
	{ "FLOAT16",          TokenType_FLOAT16 },
	{ "FLOAT32",          TokenType_FLOAT32 },
};
static_assert( ARRAY_LENGTH( KeywordNames ) == TOKENTYPE_KEYWORD_COUNT, "Missing Keyword TokenType!" );


const char *SwizzleTypeNames[] =
{
	"x",
	"y",
	"z",
	"w",
	"xy",
	"yz",
	"zw",
	"xyz",
	"yzw",
	"xyzw",
	"r",
	"g",
	"b",
	"a",
	"rg",
	"gb",
	"ba",
	"rgb",
	"gba",
	"rgba",
	"u",
	"v",
	"uv",
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Scanner::skip_whitespace()
{
	for( ;; )
	{
		switch( buffer[position] )
		{
			case ' ':
			case '\t':
			case '\r':
				position++;
			break;

			case '\n':
				position++;
				line++;
			break;

			default: return;
		}
	}
}


bool Scanner::consume( char c )
{
	if( buffer[position] == c )
	{
		position++;
		return true;
	}

	return false;
}


static bool is_letter( const char c )
{
	return ( static_cast<unsigned int>( c ) | 32 ) - 'a' < 26;
}


static bool is_digit( const char c )
{
	return ( static_cast<unsigned int>( c ) ) - '0' < 10;
}


static TokenType get_keyword( const char *buffer, const u32 length )
{
	const u32 keywordCount = sizeof( KeywordNames ) / sizeof( KeywordNames[0] );
	for( u32 i = 0; i < keywordCount; i++ )
	{
		const Keyword &keyword = KeywordNames[i];
		if( strlen( keyword.key ) != length ) { continue; }
		if( strncmp( buffer, keyword.key, length ) == 0 )
		{
			// Token Found
			return keyword.token;
		}
	}

	return TokenType_Error;
}


Token Scanner::next()
{
	Token token;
	#define RETURN_TOKEN(x) token = x; goto success;

	skip_whitespace();
	u32 start = position;

	const char c = buffer[position++];
	switch( c )
	{
		// End of File
		case '\0':
		{
			position--;
			RETURN_TOKEN( Token( TokenType_EndOfFile ) );
		}

		// Single Character
		case '(': { RETURN_TOKEN( Token( TokenType_LParen ) ); }
		case ')': { RETURN_TOKEN( Token( TokenType_RParen ) ); }
		case '{': { RETURN_TOKEN( Token( TokenType_LCurly ) ); }
		case '}': { RETURN_TOKEN( Token( TokenType_RCurly ) ); }
		case '[': { RETURN_TOKEN( Token( TokenType_LBrack ) ); }
		case ']': { RETURN_TOKEN( Token( TokenType_RBrack ) ); }
		case '.': { RETURN_TOKEN( Token( TokenType_Dot ) ); }
		case ',': { RETURN_TOKEN( Token( TokenType_Comma ) ); }
		case ':': { RETURN_TOKEN( Token( TokenType_Colon ) ); }
		case ';': { RETURN_TOKEN( Token( TokenType_Semicolon ) ); }
		case '~': { RETURN_TOKEN( Token( TokenType_BitNot ) ); }
		case '?': { RETURN_TOKEN( Token( TokenType_Question ) ); }

		// Double Character
		case '=':
		{
			RETURN_TOKEN( Token( consume( '=' ) ? TokenType_Equals : TokenType_Assign ) );
		}

		case '!':
		{
			RETURN_TOKEN( Token( consume( '=' ) ? TokenType_NotEquals : TokenType_Bang ) );
		}

		case '+':
		{
			RETURN_TOKEN( Token( consume( '=' ) ?
				TokenType_PlusAssign : ( consume( '+' ) ? TokenType_PlusPlus : TokenType_Plus ) ) );
		}

		case '-':
		{
			RETURN_TOKEN( Token( consume( '=' ) ?
				TokenType_MinusAssign : ( consume( '-' ) ? TokenType_MinusMinus : TokenType_Minus ) ) );
		}

		case '*':
		{
			RETURN_TOKEN( Token( consume( '=' ) ? TokenType_StarAssign : TokenType_Star ) );
		}

		case '/':
		{
			RETURN_TOKEN( Token( consume( '=' ) ? TokenType_SlashAssign : TokenType_Slash ) );
		}

		case '%':
		{
			RETURN_TOKEN( Token( consume( '=' ) ? TokenType_ModAssign : TokenType_Mod ) );
		}

		case '^':
		{
			RETURN_TOKEN( Token( consume( '=' ) ? TokenType_BitXorAssign : TokenType_BitXor ) );
		}

		case '|':
		{
			if( consume( '=' ) ) { RETURN_TOKEN( Token( TokenType_BitOrAssign ) ); }
			if( consume( '|' ) ) { RETURN_TOKEN( Token( TokenType_Or ) ); }
			RETURN_TOKEN( { TokenType_BitOr } );
		}

		case '&':
		{
			if( consume( '=' ) ) { RETURN_TOKEN( Token( TokenType_BitAndAssign ) ); }
			if( consume( '&' ) ) { RETURN_TOKEN( Token( TokenType_And ) ); }
			RETURN_TOKEN( { TokenType_BitAnd } );
		}

		// Triple Character
		case '<':
		{
			if( consume( '<' ) )
			{
				RETURN_TOKEN( Token( consume( '=' ) ? TokenType_BitShiftLeftAssign : TokenType_BitShiftLeft ) );
			}
			RETURN_TOKEN( Token( consume( '=' ) ? TokenType_LessThanEquals : TokenType_LessThan ) );
		}

		case '>':
		{
			if( consume( '>' ) )
			{
				RETURN_TOKEN( Token( consume( '=' ) ? TokenType_BitShiftRightAssign : TokenType_BitShiftRight ) );
			}
			RETURN_TOKEN( Token( consume( '=' ) ? TokenType_GreaterThanEquals : TokenType_GreaterThan ) );
		}

		// Other
		default:
		{
			// Identifier
			if( is_letter( c ) || c == '_' )
			{
				while( ( buffer[position] != '\0' ) &&
				       ( is_digit( buffer[position] ) || is_letter( buffer[position] ) || buffer[position] == '_' ) )
				{
					position++;
				}

				const TokenType keyword = get_keyword( &buffer[start], position - start );

				if( keyword >= 0 )
				{
					RETURN_TOKEN( Token( keyword ) );
				}

				RETURN_TOKEN( Token( TokenType_Identifier, StringView{ &buffer[start], position - start } ) );
			}

			// Numbers
			if( is_digit( c ) )
			{
				bool seenDot = false;
				while( buffer[position] != '\0' && ( is_digit( buffer[position] ) || buffer[position] == '.' ) )
				{
					if( buffer[position] == '.' )
					{
						if( seenDot )
						{
							// Can't have more than one dot
							RETURN_TOKEN( Token( TokenType_Error ) );
						}
						seenDot = true;
					}

					position++;
				}

				// Why would you have a number this big??
				char number[64];
				if( position - start >= sizeof( number ) - 1 )
				{
					RETURN_TOKEN( Token( TokenType_Error ) );
				}
				strncpy( number, &buffer[start], position - start );
				number[position - start] = '\0';

				if( seenDot )
				{
					// Double
					RETURN_TOKEN( Token( TokenType_Number, atof( number ) ) );
				}

				// Integer
				RETURN_TOKEN( Token( TokenType_Integer, static_cast<u64>( atoll( number ) ) ) );
			}

			// Error: Unknown token?
			RETURN_TOKEN( Token( TokenType_Error ) );
		}
	}

success:
	token.position = position;
	token.start = start;
	token.line = line;
	stack.add( token );
	return token;
}


Token Scanner::back()
{
	if( stack.size() == 0 ) { return Token( TokenType_Error ); }
	stack.remove( stack.size() - 1 );

	Token token = stack[stack.size() - 1];
	position = token.position;
	line = token.line;
	return token;
}


Token Scanner::current()
{
	if( stack.size() == 0 ) { return Token( TokenType_Error ); }
	return stack[stack.size() - 1];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

StructID Parser::register_struct( const Struct structure )
{
	StructID structID = structs.size();
	structs.add( structure );
	return structID;
}


TextureID Parser::register_texture( const Texture texture )
{
	TextureID textureID = textures.size();
	textures.add( texture );
	Variable &textureVariable = variables[texture.variableID];
	textureMap.add( textureVariable.name, textureID );
	return textureID;
}


TypeID Parser::register_type( const Type type )
{
	TypeID typeID = types.size();
	types.add( type );
	typeMap.add( type.name, typeID );
	return typeID;
}


FunctionID Parser::register_function( const Function function )
{
	FunctionID functionID = functions.size();
	functions.add( function );
	functionMap.add( function.name, functionID );
	return functionID;
}


VariableID Parser::register_variable( const Variable variable )
{
	VariableID variableID = variables.size();
	variables.add( variable );
	scope.add( variableID );
	return variableID;
}


void Parser::register_swizzles()
{
	u32 swizzleCount = ARRAY_LENGTH( SwizzleTypeNames );
	for( u32 i = 0; i < swizzleCount; i++ )
	{
		StringView name;
		name.data = SwizzleTypeNames[i];
		name.length = strlen( name.data );
		swizzleMap.add( name, i );
	}
}


TypeID Parser::node_type( Node *node )
{
	if( node == nullptr )
	{
		return USIZE_MAX;
	}

	switch( node->nodeType )
	{
		case NodeType_ExpressionBinary:
		{
			NodeExpressionBinary *nodeExpression = reinterpret_cast<NodeExpressionBinary *>( node );
			if( nodeExpression->exprType == ExpressionBinaryType_Dot )
			{
				return node_type( nodeExpression->expr2 );
			}
			else
			{
				return node_type( nodeExpression->expr1 );
			}
		}
		break;

		case NodeType_ExpressionUnary:
		{
			return node_type( reinterpret_cast<NodeExpressionUnary *>( node )->expr );
		}
		break;

		case NodeType_FunctionCall:
		{
			Function &function = functions[reinterpret_cast<NodeFunctionCall *>( node )->functionID];
			return function.typeID;
		}
		break;

		case NodeType_Cast:
		{
			return reinterpret_cast<NodeCast *>( node )->typeID;
		}
		break;

		case NodeType_Variable:
		{
			Variable &variable = variables[reinterpret_cast<NodeVariable *>( node )->variableID];
			return variable.typeID;
		}
		break;

		case NodeType_Group:
		{
			return node_type( reinterpret_cast<NodeGroup *>( node )->expr );
		}

		case NodeType_Texture:
		{
			Texture &texture = textures[reinterpret_cast<NodeTexture *>( node )->textureID];
			return variables[texture.variableID].typeID;
		}

		// Node has no type
		default: return USIZE_MAX;
	}
}


bool Parser::node_is_constexpr( Node *node )
{
	if( node == nullptr )
	{
		return true;
	}

	switch( node->nodeType )
	{
		case NodeType_ExpressionBinary:
		{
			NodeExpressionBinary *nodeExpression = reinterpret_cast<NodeExpressionBinary *>( node );

			if( nodeExpression->exprType == ExpressionBinaryType_Dot )
			{
				return node_is_constexpr( nodeExpression->expr2 );
			}
			else
			{
				return node_is_constexpr( nodeExpression->expr1 );
			}
		}
		break;

		case NodeType_ExpressionUnary:
		{
			return node_is_constexpr( reinterpret_cast<NodeExpressionUnary *>( node )->expr );
		}
		break;

		case NodeType_VariableDeclaration:
		{
			Variable &variable = variables[reinterpret_cast<NodeVariableDeclaration *>( node )->variableID];
			return variable.constant;
		}
		break;

		case NodeType_Variable:
		{
			Variable &variable = variables[reinterpret_cast<NodeVariable *>( node )->variableID];
			return variable.constant;
		}
		break;

		case NodeType_Swizzle:
		{
			return false; // TODO: Fix
		}
		break;

		case NodeType_Group:
		{
			return node_is_constexpr( reinterpret_cast<NodeGroup *>( node )->expr );
		}

		// Node has no type
		default: return true;
	}
}


VariableID Parser::scope_find_variable( const StringView name )
{
	for( VariableID i = 0; i < scope.size(); i++ )
	{
		Variable &variable = variables[scope[i]];
		if( variable.name.length != name.length ) { continue; }
		if( strncmp( variable.name.data, name.data, name.length ) == 0 ) { return scope[i]; }
	}
	return USIZE_MAX;
}


void Parser::scope_reset( const VariableID target )
{
	for( VariableID i = scope.size(); i > target; i-- )
	{
		scope.remove( i - 1 );
	}
}


void Parser::check_namespace_conflicts( const StringView name )
{
	ErrorIf( typeMap.contains( name ),
		"namespace: '%.*s' conflicts with existing type", name.length, name.data );
	ErrorIf( functionMap.contains( name ),
		"namespace: '%.*s' conflicts with existing function", name.length, name.data );
	ErrorIf( scope_find_variable( name ) != USIZE_MAX,
		"namespace: '%.*s' conflicts with existing variable", name.length, name.data );
	// TODO: Scope variables
}


void Parser::expect_semicolon()
{
	Token token = scanner.current();
	if( token.type != TokenType_Semicolon )
	{
		scanner.back();
		Error( "missing semicolon" );
	}
	scanner.next();
}


bool Parser::node_seen( Node *node )
{
	if( node == nullptr )
	{
		return false;
	}

	switch( node->nodeType )
	{
		case NodeType_Struct:
		{
			Type &type = types[structs[reinterpret_cast<NodeStruct *>( node )->structID].typeID];
			return type.seen;
		}
		break;

		case NodeType_Texture:
		{
			Texture &texture = textures[reinterpret_cast<NodeTexture *>( node )->textureID];
			return texture.seen;
		}
		break;

		case NodeType_FunctionDeclaration:
		{
			Function &function = functions[reinterpret_cast<NodeFunctionDeclaration *>( node )->functionID];
			return function.seen;
		}
		break;

		default:
		{
			Error( "Unsupported program level token!" );
		}
		break;
	}

	// Unreachable
	Error( "unreachable!" );
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Parser::init()
{
	// Built-in Types
	for( u32 i = 0; i < PRIMITIVE_COUNT; i++ )
	{
		Type type;
		type.name = StringView( Primitives[i], strlen( Primitives[i] ) );
		type.builtin = true;
		register_type( type );
	}

	// Built-in Swizzles
	register_swizzles();

	// Built-in Functions
	for( u32 i = 0; i < INTRINSIC_COUNT; i++ )
	{
		usize functionID = functions.size();
		Function &function = functions.add( { } );
		function.name = StringView( Intrinsics[i], strlen( Intrinsics[i] ) );
		function.builtin = true;
		functionMap.add( function.name, functionID );
	}

	// Slots
	for( u32 i = 0; i < SHADER_MAX_BUFFER_SLOTS; i++ ) { bufferSlots[i] = false; }
	for( u32 i = 0; i < SHADER_MAX_TEXTURE_SLOTS; i++ ) { textureSlots[i] = false; }
	for( u32 i = 0; i < SHADER_MAX_TARGET_SLOTS; i++ ) { targetSlots[i] = false; }
}


bool Parser::parse( const char *string )
{
	// Initialize
	init();
	scanner.init( string );

	// Parse Definitions
	for( ;; )
	{
		Token token = scanner.next();
		ErrorIf( token.type == TokenType_Error, "unknown token" );
		if( token.type == TokenType_EndOfFile ) { break; }

		// Program-level Tokens
		switch( token.type )
		{
			case TokenType_Struct:
			case TokenType_CBuffer:
			case TokenType_VertexInput:
			case TokenType_VertexOutput:
			case TokenType_FragmentInput:
			case TokenType_FragmentOutput:
			case TokenType_ComputeInput:
			case TokenType_ComputeOutput:
			{
				program.add( parse_structure() );
			}
			break;

			case TokenType_Texture1D:
			case TokenType_Texture1DArray:
			case TokenType_Texture2D:
			case TokenType_Texture2DArray:
			case TokenType_Texture3D:
			case TokenType_TextureCube:
			case TokenType_TextureCubeArray:
			{
				program.add( parse_texture() );
			}
			break;

			case TokenType_Identifier:
			{
				program.add( parse_function_declaration() );
			}
			break;

			default:
				Error( "unexpected program-level token!" );
			break;
		}
	}

	// Main
	const bool hasMain = mainVertex   != USIZE_MAX ||
	                     mainFragment != USIZE_MAX ||
	                     mainCompute  != USIZE_MAX;

	if( !hasMain )
	{
		scanner.back();
		Error( "shader does not implement a main function!\n\nmust implement vertex_main(), fragment_main(), or compute_main()" );
	}

	// Success
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool vertex_input_type_allowed( const TypeID typeID )
{
	switch( typeID )
	{
		case Primitive_Bool:
		case Primitive_Int:
		case Primitive_UInt:
		case Primitive_Float:
		case Primitive_Double:
		case Primitive_Bool2:
		case Primitive_Int2:
		case Primitive_UInt2:
		case Primitive_Float2:
		case Primitive_Double2:
		case Primitive_Bool3:
		case Primitive_Int3:
		case Primitive_UInt3:
		case Primitive_Float3:
		case Primitive_Double3:
		case Primitive_Bool4:
		case Primitive_Int4:
		case Primitive_UInt4:
		case Primitive_Float4:
		case Primitive_Double4:
			// Allowed type
		return true;
	}

	// Illegal type
	return false;
}


static bool cbuffer_type_allowed( const TypeID typeID )
{
	switch( typeID )
	{
		case Primitive_Void:
		case Primitive_Bool:
		case Primitive_Bool2:
		case Primitive_Bool3:
		case Primitive_Bool4:
		case Primitive_Int:
		case Primitive_Int2:
		case Primitive_Int3:
		case Primitive_Int4:
		case Primitive_UInt:
		case Primitive_UInt2:
		case Primitive_UInt3:
		case Primitive_UInt4:
		case Primitive_Float:
		case Primitive_Float2:
		case Primitive_Float3:
		case Primitive_Float4:
		case Primitive_Float2x2:
		case Primitive_Float3x3:
		case Primitive_Float4x4:
		case Primitive_Double:
		case Primitive_Double2:
		case Primitive_Double3:
		case Primitive_Double4:
		case Primitive_Double2x2:
		case Primitive_Double3x3:
		case Primitive_Double4x4:
			// Allowed type
		return true;
	}

	// Illegal type
	return false;
}


Node *Parser::parse_structure()
{
	Token token = scanner.current();
	VariableID scopeIndex = scope.size();

	Type type;
	type.tokenType = token.type;
	type.global = false;
	Struct structure;

	// Classify
	StructType structType = StructType_Struct;
	bool expectSlot = false;
	bool expectTags = false;
	switch( token.type )
	{
		case TokenType_Struct:
			structType = StructType_Struct;
			type.global = false;
			type.pipelineIntermediate = false;
		break;

		case TokenType_CBuffer:
			structType = StructType_CBuffer;
			expectSlot = true;
			type.global = true;
			type.pipelineIntermediate = false;
		break;

		case TokenType_VertexInput:
			structType = StructType_VertexInput;
			expectTags = true;
			type.global = true;
			type.pipelineIntermediate = false;
		break;

		case TokenType_VertexOutput:
			structType = StructType_VertexOutput;
			expectTags = true;
			type.global = true;
			type.pipelineIntermediate = true;
		break;

		case TokenType_FragmentInput:
			structType = StructType_FragmentInput;
			expectTags = true;
			type.global = true;
			type.pipelineIntermediate = true;
		break;

		case TokenType_FragmentOutput:
			structType = StructType_FragmentOutput;
			expectTags = true;
			type.global = true;
			type.pipelineIntermediate = false;
		break;

		case TokenType_ComputeInput:
			structType = StructType_ComputeInput;
			expectTags = true;
			type.global = true;
			type.pipelineIntermediate = false;
		break;

		case TokenType_ComputeOutput:
			structType = StructType_ComputeOutput;
			expectTags = true;
			type.global = true;
			type.pipelineIntermediate = true;
		break;

		default: Error( "unknown structure type!" ); break;
	}
	const char *structName = StructTypeNames[structType];

	// Slot
	if( expectSlot )
	{
		// '('
		token = scanner.next();
		ErrorIf( token.type != TokenType_LParen, "%s: expected '(' before slot id", structName );

		// Integer
		token = scanner.next();
		ErrorIf( token.type != TokenType_Integer,
			"%s: slot id must be a positive, constant integer", structName );
		structure.slot = static_cast<int>( token.integer );
		ErrorIf( structure.slot >= SHADER_MAX_BUFFER_SLOTS,
			"%s: slot id exceeded maximum: %u", structName, SHADER_MAX_BUFFER_SLOTS );
		ErrorIf( bufferSlots[structure.slot],
			"%s: slot id '%d' is already bound!", structName, structure.slot );
		bufferSlots[structure.slot] = true;

		// ')'
		token = scanner.next();
		ErrorIf( token.type != TokenType_RParen, "%s: expected ')' after slot id", structName );
	}

	// Name
	token = scanner.next();
	ErrorIf( token.type != TokenType_Identifier,"%s: expected name after struct keyword", structName );
	check_namespace_conflicts( token.name );
	type.name = token.name;

	// '{'
	token = scanner.next();
	ErrorIf( token.type != TokenType_LCurly, "%s: expected '{' after name", structName );

	// Members
	token = scanner.next();
	type.memberFirst = variables.size();
	while( token.type != TokenType_RCurly )
	{
		// Variables
		NodeVariableDeclaration *nodeDecl = reinterpret_cast<NodeVariableDeclaration *>( parse_variable_declaration() );
		ErrorIf( nodeDecl->assignment != nullptr, "%s: member variable assignment not allowed", structName );
		Variable &variable = variables[variables.size() - 1];
		Type &variableType = types[variable.typeID];
		token = scanner.current();

		// Restrictions
		if( variableType.global )
		{
			scanner.back();
			scanner.back();
			Error( "%s: member variables cannot be of constant structure type", structName );
		}

		if( variable.constant )
		{
			scanner.back();
			scanner.back();
			Error( "%s: member variables cannot be const", structName );
		}

		if( variable.in || variable.out )
		{
			scanner.back();
			scanner.back();
			Error( "%s: member variables cannot be declared with 'in', 'out', or 'inout'", structName );
		}

		if( structType == StructType_VertexInput )
		{
			const bool allowedType = vertex_input_type_allowed( variable.typeID );
			ErrorIf( !allowedType, "Type not allowed in vertex_input! Must be a primitive, non-matrix type" );
		}

		if( structType == StructType_CBuffer )
		{
			const bool allowedType = cbuffer_type_allowed( variable.typeID );
			ErrorIf( !allowedType, "Type not allowed in cbuffer! Must be a primitive type" );
		}

		// Parse Tags
		if( expectTags )
		{
			// Semantic
			variable.semantic = SemanticType_TEXCOORD;
			if( token.type == TokenType_Semantic )
			{
				token = scanner.next();
				ErrorIf( token.type != TokenType_LParen, "%s: expected '(' before semantic type", structName );

				token = scanner.next();
				ErrorIf( token.type < TokenType_POSITION || token.type > TokenType_COLOR,
					"%s: unknown semantic", structName );
				variable.semantic = ( token.type - TokenType_POSITION ); // TODO: Wrapper for this?

				token = scanner.next();
				ErrorIf( token.type != TokenType_RParen, "%s: expected ')' after semantic type", structName );

				token = scanner.next();
			}
			else
			{
				// fragment_output requirements
				ErrorIf( structType == StructType_FragmentOutput,
					"%s members require a semantic() of 'COLOR' or 'DEPTH'", structName );
			}

			// vertex_input
			if( structType == StructType_VertexInput )
			{
				// Input Format
				ErrorIf( token.type != TokenType_InputFormat, "vertex_input members require a format()" );

				token = scanner.next();
				ErrorIf( token.type != TokenType_LParen, "%s: expected '(' before format type", structName );

				token = scanner.next();
				ErrorIf( token.type < TokenType_UNORM8 || token.type > TokenType_FLOAT32,
					"%s: invalid format() type", structName );
				variable.format = ( token.type - TokenType_UNORM8 ); // TODO: Wrapper for this?

				token = scanner.next();
				ErrorIf( token.type != TokenType_RParen, "%s: expected ')' after format type", structName );

				token = scanner.next();
			}

			// fragment_output
			if( structType == StructType_FragmentOutput )
			{
				// Restrictions
				const bool validSemantic = ( variable.semantic == SemanticType_COLOR ||
				                             variable.semantic == SemanticType_DEPTH );
				if( !validSemantic )
				{
					scanner.back();
					scanner.back();
					Error( "%s must only be semantic 'COLOR' or 'DEPTH", structName );
				}

				// Target
				if( variable.semantic == SemanticType_COLOR )
				{
					ErrorIf( token.type != TokenType_Target,
						"%s semantic 'COLOR' requires a target() slot", structName );

					token = scanner.next();
					ErrorIf( token.type != TokenType_LParen,
						"%s: expected '(' before target type", structName );

					token = scanner.next();
					ErrorIf( token.type != TokenType_Integer,
						"%s: target() must be a positive, constant integer", structName );
					variable.slot = static_cast<int>( token.integer );
					ErrorIf( variable.slot >= SHADER_MAX_TARGET_SLOTS,
						"%s: target() exceeded maximum: %u", structName, SHADER_MAX_TARGET_SLOTS );
					ErrorIf( targetSlots[variable.slot],
						"%s: target( '%u' ) is already bound!", structName, variable.slot );
					targetSlots[variable.slot] = true;

					token = scanner.next();
					ErrorIf( token.type != TokenType_RParen, "%s: expected ')' after target", structName );

					token = scanner.next();
				}
			}
		}

		// Semicolon
		if( token.type != TokenType_Semicolon )
		{
			scanner.back();
			Error( "%s member: expected semicolon after variable declaration", structName );
		}
		token = scanner.next();
	}
	type.memberCount = variables.size() - type.memberFirst;

	// Semicolon
	token = scanner.next();
	if( token.type != TokenType_Semicolon )
	{
		scanner.back();
		Error( "%s: expected semicolon after final closing '}'", structName );
	}

	// Success
	structure.typeID = register_type( type );
	scope_reset( scopeIndex );
	return ast.add( NodeStruct( structType, register_struct( structure ) ) );
}


Node *Parser::parse_texture()
{
	Token token = scanner.current();
	Texture texture;
	Variable variable;

	// Classify Texture
	TextureType textureType = TextureType_Texture2D;
	switch( token.type )
	{
		case TokenType_Texture1D:
			textureType = TextureType_Texture1D;
			variable.typeID = Primitive_Texture1D;
		break;

		case TokenType_Texture1DArray:
			textureType = TextureType_Texture1D;
			variable.typeID = Primitive_Texture1DArray;
		break;

		case TokenType_Texture2D:
			textureType = TextureType_Texture2D;
			variable.typeID = Primitive_Texture2D;
		break;

		case TokenType_Texture2DArray:
			textureType = TextureType_Texture2D;
			variable.typeID = Primitive_Texture2DArray;
		break;

		case TokenType_Texture3D:
			textureType = TextureType_Texture3D;
			variable.typeID = Primitive_Texture3D;
		break;

		case TokenType_TextureCube:
			textureType = TextureType_TextureCube;
			variable.typeID = Primitive_TextureCube;
		break;

		case TokenType_TextureCubeArray:
			textureType = TextureType_TextureCubeArray;
			variable.typeID = Primitive_TextureCubeArray;
		break;

		default: Error( "unknown texture type!" ); break;
	}
	const char *textureName = TextureTypeNames[textureType];

	// Slot
	token = scanner.next();
	ErrorIf( token.type != TokenType_LParen, "%s: expected '(' before slot", textureName );
	token = scanner.next();
	ErrorIf( token.type != TokenType_Integer, "%s: slot must be a positive, constant integer", textureName );
	texture.slot = static_cast<int>( token.integer );
	ErrorIf( texture.slot >= SHADER_MAX_TEXTURE_SLOTS,
		"%s: slot exceeded maximum: %u", textureName, SHADER_MAX_TEXTURE_SLOTS );
	ErrorIf( textureSlots[texture.slot],
		"%s: slot '%d' is already bound!", textureName, texture.slot );
	textureSlots[texture.slot] = true;
	token = scanner.next();
	ErrorIf( token.type != TokenType_RParen, "%s: expected ')' after slot", textureName );

	// Name
	token = scanner.next();
	ErrorIf( token.type != TokenType_Identifier,
		"%s: expected name after %s(slot) keyword", textureName, textureName );
	check_namespace_conflicts( token.name );
	variable.name = token.name;
	variable.slot = texture.slot;

	// Semicolon
	token = scanner.next();
	if( token.type != TokenType_Semicolon )
	{
		scanner.back();
		Error( "%s: expected semicolon", textureName );
	}

	// Success
	texture.variableID = register_variable( variable );
	return ast.add( NodeTexture( textureType, register_texture( texture ) ) );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Node *Parser::parse_statement()
{
	Token token = scanner.current();

	switch( token.type )
	{
		case TokenType_Identifier:
		{
			Node *node = parse_statement_expression();
			expect_semicolon();
			return node;
		}

		case TokenType_LCurly:
		{
			Node *node = parse_statement_block();
			scanner.next();
			return node;
		}

		case TokenType_If:
		{
			return parse_statement_if();
		}

		case TokenType_While:
		{
			return parse_statement_while();
		}

		case TokenType_Do:
		{
			Node *node = parse_statement_do_while();
			expect_semicolon();
			return node;
		}

		case TokenType_For:
		{
			return parse_statement_for();
		}

		case TokenType_Switch:
		{
			return parse_statement_switch();
		}

		case TokenType_Case:
		{
			Error( "'case' keyword used outside of switch statement" );
		}

		case TokenType_Default:
		{
			Error( "'default' keyword used outside of switch statement" );
		}

		case TokenType_Return:
		{
			Node *node = parse_statement_return();
			expect_semicolon();
			return node;
		}

		case TokenType_Break:
		{
			Node *node = parse_statement_break();
			expect_semicolon();
			return node;
		}

		case TokenType_Discard:
		{
			Node *node = parse_statement_discard();
			expect_semicolon();
			return node;
		}

		default:
		{
			// Try any expression
			Node *node = parse_statement_expression();
			expect_semicolon();
			return node;
		}
	}

	// Unreachable
	Error( "unreachable" );
	return nullptr;
}


Node *Parser::parse_statement_block( VariableID scopeIndex )
{
	Token token = scanner.current();
	ErrorIf( token.type != TokenType_LCurly, "block must start with '{'" );

	// Set Scope
	if( scopeIndex == USIZE_MAX ) { scopeIndex = scope.size(); }

	// Block Body
	Node *first = nullptr;
	Node *current = nullptr;
	Node *previous = nullptr;
	token = scanner.next();
	while( token.type != TokenType_RCurly )
	{
		current = ast.add( NodeStatementBlock( parse_statement() ) );

		if( first == nullptr ) { first = current; }
		if( previous != nullptr ) { reinterpret_cast<NodeStatementBlock *>( previous )->next = current; }

		previous = current;
		token = scanner.current();
	}

	// Reset Scope
	scope_reset( scopeIndex );
	return first == nullptr ? ast.add( NodeStatementBlock( nullptr ) ) : first;
}


Node *Parser::parse_statement_expression()
{
	Token token = scanner.current();

	// Variable Declaration
	if( token.type == TokenType_Const || typeMap.contains( token.name ) )
	{
		Node *node = ast.add( NodeStatementExpression( parse_variable_declaration() ) );
		Variable &variable = variables[variables.size() - 1];
		Type &type = types[variable.typeID];
		ErrorIf( variable.in || variable.out,
			"cannot declare a variable with 'in' or 'out' in a function body" );
		ErrorIf( type.global,
			"cannot declare a variable of constant structure type in a function body" );
		return node;
	}
	// Other Expressions
	else
	{
		return ast.add( NodeStatementExpression( parse_expression() ) );
	}
}


Node *Parser::parse_statement_function_call()
{
	// Expect Function
	Token token = scanner.current();
	ErrorIf( token.type != TokenType_Identifier,
		"unexpected token" );
	ErrorIf( !functionMap.contains( token.name ),
		"identifier '%.*s' is not a function", token.name.length, token.name.data );
	FunctionID functionID = functionMap.get( token.name );

	// Expect '('
	token = scanner.next();
	if( token.type != TokenType_LParen )
	{
		scanner.back();
		Error( "function call requires '('" );
	}

	// Parameters
	Node *first = nullptr;
	Node *current = nullptr;
	Node *previous = nullptr;
	token = scanner.next();
	while( token.type != TokenType_RParen )
	{
		// Expect Expression
		current = ast.add( NodeExpressionList( parse_expression() ) );
		if( first == nullptr ) { first = current; }
		if( previous != nullptr ) { reinterpret_cast<NodeExpressionList *>( previous )->next = current; }
		previous = current;

		// Expect Comma
		token = scanner.current();
		if( token.type == TokenType_RParen ) { break; }
		ErrorIf( token.type != TokenType_Comma, "function parameters require ',' separation" );
		token = scanner.next();
	}

	// Success
	scanner.next();
	return ast.add( NodeFunctionCall( functionID, first ) );
}


Node *Parser::parse_statement_cast()
{
	// Expect Type
	Token token = scanner.current();
	ErrorIf( token.type != TokenType_Identifier,
		"unexpected token" );
	ErrorIf( !typeMap.contains( token.name ),
		"identifier '%.*s' is not a type", token.name.length, token.name.data );
	TypeID typeID = typeMap.get( token.name );

	// Expect '('
	token = scanner.next();
	if( token.type != TokenType_LParen )
	{
		scanner.back();
		Error( "type cast requires '('" );
	}

	// Parameters
	Node *first = nullptr;
	Node *current = nullptr;
	Node *previous = nullptr;
	token = scanner.next();
	while( token.type != TokenType_RParen )
	{
		// Expect Expression
		current = ast.add( NodeExpressionList( parse_expression() ) );
		if( first == nullptr ) { first = current; }
		if( previous != nullptr ) { reinterpret_cast<NodeExpressionList *>( previous )->next = current; }
		previous = current;

		// Expect Comma
		token = scanner.current();
		if( token.type == TokenType_RParen ) { break; }
		ErrorIf( token.type != TokenType_Comma, "type cast parameters require ',' separation" );
		token = scanner.next();
	}

	// Success
	scanner.next();
	return ast.add( NodeCast( typeID, first ) );
}


Node *Parser::parse_statement_if()
{
	// Expect 'if'
	Token token = scanner.current();
	ErrorIf( token.type != TokenType_If, "unexpected token" );
	Node *blockIf = nullptr;
	Node *blockElse = nullptr;

	// Expect '('
	token = scanner.next();
	if( token.type != TokenType_LParen )
	{
		scanner.back();
		Error( "'if' condition requires '('" );
	}

	// Condition
	token = scanner.next();
	Node *expr = parse_expression();

	// Expect ')'
	token = scanner.current();
	if( token.type != TokenType_RParen )
	{
		scanner.back();
		Error(  "'if' condition missing ')'" );
	}

	// Expect '{'
	token = scanner.next();
	if( token.type != TokenType_LCurly )
	{
		scanner.back();
		Error( "'if' condition body requires '{'" );
	}
	blockIf = parse_statement_block();

	// Expect 'else'
	token = scanner.next();
	if( token.type == TokenType_Else )
	{
		token = scanner.next();
		if( token.type == TokenType_If )
		{
			blockElse = parse_statement_if();
			scanner.back();
		}
		else if( token.type == TokenType_LCurly )
		{
			blockElse = parse_statement_block();
		}
		else
		{
			scanner.back();
			Error( "else must be followed by '{ ... }' or 'if'" );
		}
	}
	else
	{
		scanner.back();
	}

	// Success
	scanner.next();
	return ast.add( NodeStatementIf( expr, blockIf, blockElse ) );
}


Node *Parser::parse_statement_while()
{
	// Expect 'while'
	Token token = scanner.current();
	ErrorIf( token.type != TokenType_While, "unexpected token" );

	// Expect '('
	token = scanner.next();
	if( token.type != TokenType_LParen )
	{
		scanner.back();
		Error( "'if' condition requires '('" );
	}

	// Condition
	token = scanner.next();
	Node *expr = parse_expression();

	// Expect ')'
	token = scanner.current();
	if( token.type != TokenType_RParen )
	{
		scanner.back();
		Error(  "'if' condition missing ')'" );
	}

	// Expect '{'
	token = scanner.next();
	if( token.type != TokenType_LCurly )
	{
		scanner.back();
		Error( "'if' condition body requires '{'" );
	}
	Node *block = parse_statement_block();

	// Success
	scanner.next();
	return ast.add( NodeStatementWhile( expr, block ) );
}


Node *Parser::parse_statement_do_while()
{
	// Expect 'do'
	Token token = scanner.current();
	ErrorIf( token.type != TokenType_Do, "unexpected token" );

	// Expect '{'
	token = scanner.next();
	if( token.type != TokenType_LCurly )
	{
		scanner.back();
		Error( "'if' condition body requires '{'" );
	}
	Node *block = parse_statement_block();

	// Expect 'while'
	token = scanner.next();
	ErrorIf( token.type != TokenType_While, "unexpected token" );

	// Expect '('
	token = scanner.next();
	if( token.type != TokenType_LParen )
	{
		scanner.back();
		Error( "'if' condition requires '('" );
	}

	// Condition
	token = scanner.next();
	Node *expr = parse_expression();

	// Expect ')'
	token = scanner.current();
	if( token.type != TokenType_RParen )
	{
		scanner.back();
		Error(  "'if' condition missing ')'" );
	}

	// Success
	scanner.next();
	return ast.add( NodeStatementDoWhile( expr, block ) );
}


Node *Parser::parse_statement_for()
{
	// Expect 'for'
	Token token = scanner.current();
	ErrorIf( token.type != TokenType_For, "unexpected token" );
	VariableID scopeIndex = scope.size();

	// Expect '('
	token = scanner.next();
	if( token.type != TokenType_LParen )
	{
		scanner.back();
		Error( "'for' loop requires '('" );
	}

	// Initialization
	token = scanner.next();
	Node *expr1 = nullptr;
	if( token.type == TokenType_Identifier && typeMap.contains( token.name ) )
	{
		expr1 = parse_variable_declaration();
	}
	else if( token.type != TokenType_Semicolon )
	{
		expr1 = parse_expression();
	}

	// Expect ';'
	token = scanner.current();
	if( token.type != TokenType_Semicolon )
	{
		scanner.back();
		Error(  "missing ';' after initialization expression" );
	}

	// Condition
	token = scanner.next();
	Node *expr2 = nullptr;
	if( token.type != TokenType_Semicolon )
	{
		expr2 = parse_expression();
	}

	// Expect ';'
	token = scanner.current();
	if( token.type != TokenType_Semicolon )
	{
		scanner.back();
		Error(  "missing ';' after condition expression" );
	}

	// Increment
	token = scanner.next();
	Node *expr3 = nullptr;
	if( token.type != TokenType_RParen )
	{
		expr3 = parse_expression();
	}

	// Expect ')'
	token = scanner.current();
	if( token.type != TokenType_RParen )
	{
		scanner.back();
		Error(  "missing ')' after increment expression" );
	}

	// Expect '{'
	token = scanner.next();
	if( token.type != TokenType_LCurly )
	{
		scanner.back();
		Error( "'for' loop body requires '{'" );
	}
	Node *block = parse_statement_block( scopeIndex );

	// Success
	scanner.next();
	return ast.add( NodeStatementFor( expr1, expr2, expr3, block ) );
}


Node *Parser::parse_statement_switch()
{
	// Expect 'switch'
	Token token = scanner.current();
	ErrorIf( token.type != TokenType_Switch, "unexpected token" );

	// Expect '('
	token = scanner.next();
	if( token.type != TokenType_LParen )
	{
		scanner.back();
		Error( "'switch' statement requires '('" );
	}

	// Expression
	token = scanner.next();
	Node *expr = parse_expression();

	// Expect ')'
	token = scanner.current();
	if( token.type != TokenType_RParen )
	{
		scanner.back();
		Error( "'switch' statement requires ')'" );
	}

	// Expect '{'
	token = scanner.next();
	if( token.type != TokenType_LCurly )
	{
		scanner.back();
		Error( "'switch' statement requires '{'" );
	}

	// Block Body
	Node *first = nullptr;
	Node *current = nullptr;
	Node *previous = nullptr;
	token = scanner.next();
	while( token.type != TokenType_RCurly )
	{
		switch( token.type )
		{
			case TokenType_Case:
			{
				current = ast.add( NodeStatementBlock( parse_statement_case() ) );
			}
			break;

			case TokenType_Default:
			{
				current = ast.add( NodeStatementBlock( parse_statement_default() ) );
			}
			break;

			case TokenType_Return:
			{
				current = ast.add( NodeStatementBlock( parse_statement_return() ) );
				expect_semicolon();
			}
			break;

			case TokenType_Break:
			{
				current = ast.add( NodeStatementBlock( parse_statement_break() ) );
				expect_semicolon();
			}
			break;

			default:
			{
				Error( "unexpected statement/expression in switch statement! %d", token.type );
			}
			break;
		}

		if( first == nullptr ) { first = current; }
		if( previous != nullptr ) { reinterpret_cast<NodeStatementBlock *>( previous )->next = current; }

		previous = current;
		token = scanner.current();
	}

	// Success
	first = first == nullptr ? ast.add( NodeStatementBlock( nullptr ) ) : first;
	scanner.next();
	return ast.add( NodeStatementSwitch( expr, first ) );
}


Node *Parser::parse_statement_case()
{
	// Expect 'case'
	Token token = scanner.current();
	ErrorIf( token.type != TokenType_Case, "unexpected token" );

	// Expression
	token = scanner.next();
	Node *expr = parse_expression();

	// Expect ':'
	token = scanner.current();
	if( token.type != TokenType_Colon )
	{
		scanner.back();
		Error( "'case' requires ':' after expression" );
	}

	// Expect '{'
	token = scanner.next();
	Node *block = nullptr;
	if( token.type == TokenType_LCurly )
	{
		block = parse_statement_block();
		scanner.next();
	}
	else if( token.type == TokenType_Case    || token.type == TokenType_Break ||
	         token.type == TokenType_Default || token.type == TokenType_Return )
	{
		block = nullptr;
	}
	else
	{
		block = ast.add( NodeStatementBlock( parse_statement() ) );
	}

	// Success
	return ast.add( NodeStatementCase( expr, block ) );
}


Node *Parser::parse_statement_default()
{
	// Expect 'default'
	Token token = scanner.current();
	ErrorIf( token.type != TokenType_Default, "unexpected token" );

	// Expect ':'
	token = scanner.next();
	if( token.type != TokenType_Colon )
	{
		scanner.back();
		Error( "'case' requires ':' after expression" );
	}

	// Expect '{'
	token = scanner.next();
	Node *block = nullptr;
	if( token.type == TokenType_LCurly )
	{
		block = parse_statement_block();
		scanner.next();
	}
	else if( token.type == TokenType_Case    || token.type == TokenType_Break ||
	         token.type == TokenType_Default || token.type == TokenType_Return )
	{
		block = nullptr;
	}
	else
	{
		block = ast.add( NodeStatementBlock( parse_statement() ) );
	}

	// Success
	return ast.add( NodeStatementDefault( block ) );
}


Node *Parser::parse_statement_return()
{
	// Expect 'return'
	Token token = scanner.current();
	ErrorIf( token.type != TokenType_Return, "unexpected token" );

	// Expression
	token = scanner.next();
	Node *expr = nullptr;
	if( token.type != TokenType_Semicolon )
	{
		expr = parse_expression();
	}

	// Success
	return ast.add( NodeStatementReturn( expr ) );
}


Node *Parser::parse_statement_break()
{
	// Expect 'break'
	Token token = scanner.current();
	ErrorIf( token.type != TokenType_Break, "unexpected token" );

	// Success
	scanner.next();
	return ast.add( NodeStatementBreak() );
}


Node *Parser::parse_statement_discard()
{
	// Expect 'discard'
	Token token = scanner.current();
	ErrorIf( token.type != TokenType_Discard, "unexpected token" );

	// Success
	scanner.next();
	return ast.add( NodeStatementDiscard() );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Node *Parser::parse_variable_declaration()
{
	Variable variable;
	Token token = scanner.current();

	// Is the first token in / out / inout
	if( token.type == TokenType_In || token.type == TokenType_Out || token.type == TokenType_InOut )
	{
		variable.in = ( token.type == TokenType_In || token.type == TokenType_InOut );
		variable.out = ( token.type == TokenType_Out || token.type == TokenType_InOut );
		token = scanner.next();
	}

	// Is the first token const?
	if( token.type == TokenType_Const )
	{
		ErrorIf( variable.out, "const variable cannot be tagged 'out' or 'inout'" );
		variable.constant = true;
		token = scanner.next();
	}

	// Parse Type
	ErrorIf( token.type != TokenType_Identifier,
		"variable declaration: expected a type" );
	ErrorIf( !typeMap.contains( token.name ),
		"variable declaration: unknown type '%.*s'", token.name.length, token.name.data );
	variable.typeID = typeMap.get( token.name );

	// Is the variable a constant structure type?
	if( token.type == TokenType_VertexInput ||
	    token.type == TokenType_FragmentInput ||
	    token.type == TokenType_ComputeInput )
	{
		ErrorIf( variable.out, "const variable cannot be tagged 'out' or 'inout'" );
		variable.constant = true;
	}

	// Parse Name
	token = scanner.next();
	ErrorIf( token.type != TokenType_Identifier,
		"variable declaration: expected identifier after type" );
	check_namespace_conflicts( token.name );
	variable.name = token.name;

	// Arrays
	token = scanner.next();
	if( token.type == TokenType_LBrack )
	{
		// Length X
		token = scanner.next();
		ErrorIf( token.type != TokenType_Integer,
			"variable declaration: array length must be a constant integer" );
		ErrorIf( token.integer == 0,
			"variable declaration: array length cannot be zero" );
		variable.arrayLengthX = static_cast<int>( token.integer );
		token = scanner.next();
		ErrorIf( token.type != TokenType_RBrack,
			"variable declaration: expected ']' after array length" );

		// Length Y
		token = scanner.next();
		if( token.type == TokenType_LBrack )
		{
			token = scanner.next();
			ErrorIf( token.type != TokenType_Integer,
				"variable declaration: array length must be a constant integer" );
			ErrorIf( token.integer == 0,
				"variable declaration: array length cannot be zero" );
			variable.arrayLengthY = static_cast<int>( token.integer );
			token = scanner.next();
			ErrorIf( token.type != TokenType_RBrack,
				"variable declaration: expected ']' after array length" );
		}
		else
		{
			scanner.back();
		}
	}
	else
	{
		scanner.back();
	}

	// Assignment
	Node *assignment = nullptr;
	token = scanner.next();
	if( token.type == TokenType_Assign )
	{
		ErrorIf( variable.arrayLengthX != 0 || variable.arrayLengthY != 0,
			"arrays do not support initialization assignment" );
		scanner.next();
		assignment = parse_expression();
	}

	// Success
	return ast.add( NodeVariableDeclaration( register_variable( variable ), assignment ) );
}


Node *Parser::parse_function_declaration()
{
	Function function;
	Token token = scanner.current();

	// Parse Return Type
	ErrorIf( token.type != TokenType_Identifier,
		"function declaration: expected a return type" );
	ErrorIf( !typeMap.contains( token.name ),
		"function declaration: unknown type '%.*s'", token.name.length, token.name.data );
	function.typeID = typeMap.get( token.name );

	// Parse Function Name
	token = scanner.next();
	ErrorIf( token.type != TokenType_Identifier, "function declaration: expected a name" );
	check_namespace_conflicts( token.name );
	function.name = token.name;

	// Check for vertex_main, fragment_main, or compute_main()
	if( token.name.length == strlen( "vertex_main" ) &&
	    strncmp( "vertex_main", token.name.data, token.name.length ) == 0 )
	{
		scanner.back();
		return parse_function_declaration_main( FunctionType_MainVertex, "vertex_main",
		                                        TokenType_VertexInput, TokenType_VertexOutput );
	}
	else if( token.name.length == strlen( "fragment_main" ) &&
	         strncmp( "fragment_main", token.name.data, token.name.length ) == 0 )
	{
		scanner.back();
		return parse_function_declaration_main( FunctionType_MainFragment, "fragment_main",
		                                        TokenType_FragmentInput, TokenType_FragmentOutput );
	}
	else if( token.name.length == strlen( "compute_main" ) &&
	          strncmp( "compute_main", token.name.data, token.name.length ) == 0 )
	{
		scanner.back();
		return parse_function_declaration_main( FunctionType_MainCompute, "compute_main",
		                                        TokenType_ComputeInput, TokenType_ComputeOutput );
	}

	// Function Parameters
	VariableID scopeIndex = scope.size();
	token = scanner.next();
	ErrorIf( token.type != TokenType_LParen, "function declaration: '(' before parameter list" );
	token = scanner.next();
	function.parameterFirst = variables.size();
	while( token.type != TokenType_RParen )
	{
		// Variables
		NodeVariableDeclaration *nodeDecl = reinterpret_cast<NodeVariableDeclaration *>( parse_variable_declaration() );
		ErrorIf( nodeDecl->assignment != nullptr, "function parameters cannot have assignment" );
		token = scanner.current();

		// In/Out Keywords
		Type &paramType = types[variables[nodeDecl->variableID].typeID];
		if( !( paramType.tokenType == TokenType_Struct ||
		       paramType.tokenType == TokenType_CBuffer ||
		       paramType.builtin ) )
		{
			scanner.back();
			Error( "function parameter types can only be primitives, 'struct', or 'cbuffer''" );
		}

		// Comma or ')'
		if( token.type == TokenType_RParen ) { break; }
		ErrorIf( token.type != TokenType_Comma, "function declaration: expected ',' between parameters" );
		token = scanner.next();
	};
	function.parameterCount = variables.size() - function.parameterFirst;

	// Parse Block
	token = scanner.next();
	Node *block = parse_statement_block( scopeIndex );

	// Success
	return ast.add( NodeFunctionDeclaration( FunctionType_Custom, register_function( function ), block ) );
}


Node *Parser::parse_function_declaration_main( FunctionType functionType, const char *functionName,
                                               TokenType inToken, TokenType outToken )
{
	Function function;
	Token token = scanner.current();

	// Parse Return Type
	ErrorIf( token.type != TokenType_Identifier,
		"function declaration: expected a return type" );
	ErrorIf( !typeMap.contains( token.name ),
		"function declaration: unknown type '%.*s'", token.name.length, token.name.data );
	function.typeID = typeMap.get( token.name );

	// Enforce Return Type
	ErrorIf( function.typeID != Primitive_Void,
		"%s() must have a 'void' return type", functionName );

	// Parse Function Name
	token = scanner.next();
	ErrorIf( token.type != TokenType_Identifier, "function declaration: expected a name" );
	check_namespace_conflicts( token.name );
	function.name = token.name;

	// Requirements
	bool hasIn = false;
	bool hasOut = false;

	// Function Parameters
	u32 parameterID = 0;
	VariableID scopeIndex = scope.size();
	token = scanner.next();
	ErrorIf( token.type != TokenType_LParen, "function declaration: '(' before parameter list" );
	token = scanner.next();
	function.parameterFirst = variables.size();
	while( token.type != TokenType_RParen )
	{
		// Variables
		NodeVariableDeclaration *nodeDecl = reinterpret_cast<NodeVariableDeclaration *>( parse_variable_declaration() );
		ErrorIf( nodeDecl->assignment != nullptr, "function parameters cannot have assignment" );
		token = scanner.current();

		Variable &paramVariable = variables[nodeDecl->variableID];
		Type &paramType = types[paramVariable.typeID];

		// Input Parameter
		if( !hasIn )
		{
			if( parameterID != 0 || paramType.tokenType != inToken )
			{
				scanner.back();
				scanner.back();
				Error( "%s() first parameter must be type '%s'",
					functionName, KeywordNames[inToken - TOKENTYPE_KEYWORD_FIRST].key );
			}
			hasIn = true;
		} else
		// Output Parameter
		if( !hasOut )
		{
			if( parameterID != 1 || paramType.tokenType != outToken )
			{
				scanner.back();
				scanner.back();
				Error( "%s() second parameter must be type '%s'",
					functionName, KeywordNames[outToken - TOKENTYPE_KEYWORD_FIRST].key );
			}
			hasOut = true;
		} else
		// CBuffer Parameters Only
		{
			if( parameterID > 1 && paramType.tokenType != TokenType_CBuffer )
			{
				scanner.back();
				scanner.back();
				Error( "%s() can only take additional parameters of type 'cbuffer'", functionName );
			}
		}

		// cbuffer params only

		// Comma or ')'
		if( token.type == TokenType_RParen ) { break; }
		ErrorIf( token.type != TokenType_Comma, "function declaration: expected ',' between parameters" );
		token = scanner.next();
		parameterID++;
	};
	function.parameterCount = variables.size() - function.parameterFirst;

	ErrorIf( !hasIn, "%s() requires a first parameter of type 'vertex_input'", functionName );
	ErrorIf( !hasOut, "%s() requires a second parameter of type 'vertex_output'", functionName );

	// Parse Block
	token = scanner.next();
	Node *block = parse_statement_block( scopeIndex );

	// Success
	FunctionID functionID = register_function( function );

	switch( functionType )
	{
		case FunctionType_MainVertex: mainVertex = functionID; break;
		case FunctionType_MainFragment: mainFragment = functionID; break;
		case FunctionType_MainCompute: mainCompute = functionID; break;
	}

	return ast.add( NodeFunctionDeclaration( functionType, functionID, block ) );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Node *Parser::parse_assignment()
{
	Node *node = parse_ternary_condition();

	switch( scanner.current().type )
	{
		case TokenType_Assign:
		{
			ErrorIf( node_is_constexpr( node ), "LHS must be a modifiable expression" );
			scanner.next();
			return ast.add( NodeExpressionBinary( ExpressionBinaryType_Assign, node, parse_assignment() ) );
		}

		case TokenType_PlusAssign:
		{
			ErrorIf( node_is_constexpr( node ), "LHS must be a modifiable expression" );
			scanner.next();
			return ast.add( NodeExpressionBinary( ExpressionBinaryType_AddAssign, node, parse_assignment() ) );
		}

		case TokenType_MinusAssign:
		{
			ErrorIf( node_is_constexpr( node ), "LHS must be a modifiable expression" );
			scanner.next();
			return ast.add( NodeExpressionBinary( ExpressionBinaryType_SubAssign, node, parse_assignment() ) );
		}

		case TokenType_StarAssign:
		{
			ErrorIf( node_is_constexpr( node ), "LHS must be a modifiable expression" );
			scanner.next();
			return ast.add( NodeExpressionBinary( ExpressionBinaryType_MulAssign, node, parse_assignment() ) );
		}

		case TokenType_SlashAssign:
		{
			ErrorIf( node_is_constexpr( node ), "LHS must be a modifiable expression" );
			scanner.next();
			return ast.add( NodeExpressionBinary( ExpressionBinaryType_DivAssign, node, parse_assignment() ) );
		}

		case TokenType_ModAssign:
		{
			ErrorIf( node_is_constexpr( node ), "LHS must be a modifiable expression" );
			scanner.next();
			return ast.add( NodeExpressionBinary( ExpressionBinaryType_ModAssign, node, parse_assignment() ) );
		}

		case TokenType_BitShiftLeftAssign:
		{
			ErrorIf( node_is_constexpr( node ), "LHS must be a modifiable expression" );
			scanner.next();
			return ast.add( NodeExpressionBinary( ExpressionBinaryType_BitShiftLeftAssign, node, parse_assignment() ) );
		}

		case TokenType_BitShiftRightAssign:
		{
			ErrorIf( node_is_constexpr( node ), "LHS must be a modifiable expression" );
			scanner.next();
			return ast.add( NodeExpressionBinary( ExpressionBinaryType_BitShiftRightAssign, node, parse_assignment() ) );
		}

		case TokenType_BitAndAssign:
		{
			ErrorIf( node_is_constexpr( node ), "LHS must be a modifiable expression" );
			scanner.next();
			return ast.add( NodeExpressionBinary( ExpressionBinaryType_BitAndAssign, node, parse_assignment() ) );
		}

		case TokenType_BitOrAssign:
		{
			ErrorIf( node_is_constexpr( node ), "LHS must be a modifiable expression" );
			scanner.next();
			return ast.add( NodeExpressionBinary( ExpressionBinaryType_BitOrAssign, node, parse_assignment() ) );
		}

		case TokenType_BitXorAssign:
		{
			ErrorIf( node_is_constexpr( node ), "LHS must be a modifiable expression" );
			scanner.next();
			return ast.add( NodeExpressionBinary( ExpressionBinaryType_BitXorAssign, node, parse_assignment() ) );
		}
	}

	// No Assignment
	return node;
}


Node *Parser::parse_ternary_condition()
{
	Node *node = parse_logical_or();

	if( scanner.current().type == TokenType_Question )
	{
		scanner.next();
		Node *expr2 = parse_ternary_condition();

		ErrorIf( scanner.current().type != TokenType_Colon, "Expected ':' in ternary statement" );

		scanner.next();
		Node *expr3 = parse_ternary_condition();

		return ast.add( NodeExpressionTernary( ExpressionTernaryType_Conditional, node, expr2, expr3 ) );
	}

	// No ternary condition
	return node;
}


Node *Parser::parse_logical_or()
{
	Node *node = parse_logical_and();

	if( scanner.current().type == TokenType_Or )
	{
		scanner.next();
		return ast.add( NodeExpressionBinary( ExpressionBinaryType_Or, node, parse_logical_or() ) );
	}

	// No logical or
	return node;
}


Node *Parser::parse_logical_and()
{
	Node *node = parse_bitwise_or();

	if( scanner.current().type == TokenType_And )
	{
		scanner.next();
		return ast.add( NodeExpressionBinary( ExpressionBinaryType_And, node, parse_logical_and() ) );
	}

	// No logical and
	return node;
}


Node *Parser::parse_bitwise_or()
{
	Node *node = parse_bitwise_xor();

	if( scanner.current().type == TokenType_BitOr )
	{
		scanner.next();
		return ast.add( NodeExpressionBinary( ExpressionBinaryType_BitOr, node, parse_bitwise_or() ) );
	}

	// No bitwise or
	return node;
}


Node *Parser::parse_bitwise_xor()
{
	Node *node = parse_bitwise_and();

	if( scanner.current().type == TokenType_BitXor )
	{
		scanner.next();
		return ast.add( NodeExpressionBinary( ExpressionBinaryType_BitXor, node, parse_bitwise_xor() ) );
	}

	// No bitwise xor
	return node;
}


Node *Parser::parse_bitwise_and()
{
	Node *node = parse_equality();

	if( scanner.current().type == TokenType_BitAnd )
	{
		scanner.next();
		return ast.add( NodeExpressionBinary( ExpressionBinaryType_BitAnd, node, parse_bitwise_and() ) );
	}

	// No bitwise and
	return node;
}


Node *Parser::parse_equality()
{
	Node *node = parse_comparison();

	switch( scanner.current().type )
	{
		case TokenType_Equals:
		{
			scanner.next();
			return ast.add( NodeExpressionBinary( ExpressionBinaryType_Equals, node, parse_equality() ) );
		}

		case TokenType_NotEquals:
		{
			scanner.next();
			return ast.add( NodeExpressionBinary( ExpressionBinaryType_NotEquals, node, parse_equality() ) );
		}
	}

	// No equality
	return node;
}


Node *Parser::parse_comparison()
{
	Node *node = parse_bitwise_shift();

	switch( scanner.current().type )
	{
		case TokenType_GreaterThan:
		{
			scanner.next();
			return ast.add( NodeExpressionBinary( ExpressionBinaryType_Greater, node, parse_comparison() ) );
		}

		case TokenType_GreaterThanEquals:
		{
			scanner.next();
			return ast.add( NodeExpressionBinary( ExpressionBinaryType_GreaterEquals, node, parse_comparison() ) );
		}

		case TokenType_LessThan:
		{
			scanner.next();
			return ast.add( NodeExpressionBinary( ExpressionBinaryType_Less, node, parse_comparison() ) );
		}

		case TokenType_LessThanEquals:
		{
			scanner.next();
			return ast.add( NodeExpressionBinary( ExpressionBinaryType_LessEquals, node, parse_comparison() ) );
		}
	}

	// No comparison
	return node;
}


Node *Parser::parse_bitwise_shift()
{
	Node *node = parse_add_sub();

	switch( scanner.current().type )
	{
		case TokenType_BitShiftLeft:
		{
			scanner.next();
			return ast.add( NodeExpressionBinary( ExpressionBinaryType_BitShiftLeft, node, parse_bitwise_shift() ) );
		}

		case TokenType_BitShiftRight:
		{
			scanner.next();
			return ast.add( NodeExpressionBinary( ExpressionBinaryType_BitShiftRight, node, parse_bitwise_shift() ) );
		}
	}

	// No bitshift
	return node;
}


Node *Parser::parse_add_sub()
{
	Node *node = parse_mul_div_mod();

	switch( scanner.current().type )
	{
		case TokenType_Plus:
		{
			scanner.next();
			return ast.add( NodeExpressionBinary( ExpressionBinaryType_Add, node, parse_add_sub() ) );
		}

		case TokenType_Minus:
		{
			scanner.next();
			return ast.add( NodeExpressionBinary( ExpressionBinaryType_Sub, node, parse_add_sub() ) );
		}
	}

	// No add or sub
	return node;
}


Node *Parser::parse_mul_div_mod()
{
	Node *node = parse_prefix_operators();

	switch( scanner.current().type )
	{
		case TokenType_Star:
		{
			scanner.next();
			return ast.add( NodeExpressionBinary( ExpressionBinaryType_Mul, node, parse_mul_div_mod() ) );
		}

		case TokenType_Slash:
		{
			scanner.next();
			return ast.add( NodeExpressionBinary( ExpressionBinaryType_Div, node, parse_mul_div_mod() ) );
		}

		case TokenType_Mod:
		{
			scanner.next();
			return ast.add( NodeExpressionBinary( ExpressionBinaryType_Mod, node, parse_mul_div_mod() ) );
		}
	}

	// No mul, div, or mod
	return node;
}


Node *Parser::parse_prefix_operators()
{
	switch( scanner.current().type )
	{
		case TokenType_PlusPlus:
		{
			scanner.next();
			return ast.add( NodeExpressionUnary( ExpressionUnaryType_PreIncrement, parse_expression() ) );
		}

		case TokenType_MinusMinus:
		{
			scanner.next();
			return ast.add( NodeExpressionUnary( ExpressionUnaryType_PreDecrement, parse_expression() ) );
		}

		case TokenType_Plus:
		{
			scanner.next();
			return ast.add( NodeExpressionUnary( ExpressionUnaryType_Plus, parse_expression() ) );
		}

		case TokenType_Minus:
		{
			scanner.next();
			return ast.add( NodeExpressionUnary( ExpressionUnaryType_Minus, parse_expression() ) );
		}

		case TokenType_BitNot:
		{
			scanner.next();
			return ast.add( NodeExpressionUnary( ExpressionUnaryType_BitNot, parse_expression() ) );
		}

		case TokenType_Bang:
		{
			scanner.next();
			Node *node = ast.add( NodeExpressionUnary( ExpressionUnaryType_Not, parse_expression() ) );
			return node;
		}
	}

	// No prefix operators
	return parse_suffix_operators();
}


Node *Parser::parse_suffix_operators()
{
	Node *node = parse_dot_operator();

	Token token = scanner.current();
	switch( token.type )
	{
		case TokenType_PlusPlus:
		{
			scanner.next();
			return ast.add( NodeExpressionUnary( ExpressionUnaryType_PostIncrement, node ) );
		}

		case TokenType_MinusMinus:
		{
			scanner.next();
			return ast.add( NodeExpressionUnary( ExpressionUnaryType_PostDecrement, node ) );
		}
	}

	// No suffix operators
	return node;
}


Node *Parser::parse_dot_operator()
{
	Node *node = parse_subscript_operator();

	switch( scanner.current().type )
	{
		case TokenType_Dot:
		{
			// Find LHS type (if possible)
			Token token = scanner.next();
			ErrorIf( token.type != TokenType_Identifier, "RHS for '.' operator must be an identifier" );
			TypeID typeID = node_type( node );

			if( typeID != USIZE_MAX )
			{
				Type &type = types[typeID];

				// Custom Type: check if RHS token matches a LHS structure member variable
				if( typeID >= PRIMITIVE_COUNT )
				{
					VariableID first = type.memberFirst;
					VariableID last = first + type.memberCount;

					for( VariableID i = first; i < last; i++ )
					{
						Variable &variable = variables[i];

						// Variable name comparison
						if( variable.name.length == token.name.length &&
							strncmp( variable.name.data, token.name.data, token.name.length ) == 0 )
						{
							scope.add( i );
							Node *expr = parse_expression();
							scope.remove( scope.size() - 1 );
							return ast.add( NodeExpressionBinary( ExpressionBinaryType_Dot, node, expr ) );
						}
					}

					// If reached, this is an error
					Error( "'%.*s' is not a member of LHS type '%.*s'", token.name.length, token.name.data,
					       type.name.length, type.name.data );
				}
				else
				// Built-in Type: Check if RHS token matches a swizzle
				{
					if( swizzleMap.contains( token.name ) )
					{
						swizzling = true;
						Node *expr = parse_fundamental();
						swizzling = false;
						return ast.add( NodeExpressionBinary( ExpressionBinaryType_Dot, node, expr ) );
					}

					// If reached, this is an error
					Error( "invalid swizzle on built-in type '%.*s'", type.name.length, type.name.data );
				}
			}

			// Unexpected identifier
			scanner.back();
			Error( "invalid LHS for '.' operator" );
		}
	}

	// No access operators
	return node;
}


Node *Parser::parse_subscript_operator()
{
	Node *node = parse_fundamental();

	switch( scanner.current().type )
	{
		case TokenType_LBrack:
		{
			Token token = scanner.next();
			Node *expr = parse_expression();

			token = scanner.current();
			ErrorIf( token.type != TokenType_RBrack, "Expected ']' after array indexing" );

			scanner.next();
			return ast.add( NodeExpressionBinary( ExpressionBinaryType_Subscript, node, expr ) );
		}
	}

	// No access operators
	return node;
}


Node *Parser::parse_fundamental()
{
	Token token = scanner.current();
	switch( scanner.current().type )
	{
		case TokenType_Identifier:
		{
			VariableID variableID = scope_find_variable( token.name );

			// Swizzle
			if( swizzling && swizzleMap.contains( token.name ) )
			{
				scanner.next();
				return ast.add( NodeSwizzle( swizzleMap.get( token.name ) ) );
			}
			// Variable
			else if( variableID != USIZE_MAX )
			{
				scanner.next();
				return ast.add( NodeVariable( variableID ) );
			}
			// Function Call
			else if( functionMap.contains( token.name ) )
			{
				return parse_statement_function_call();
			}
			// Type Cast
			else if( typeMap.contains( token.name ) )
			{
				return parse_statement_cast();
			}
			else
			// Unknown identifier
			{
				Error( "undeclared identifier '%.*s'", token.name.length, token.name.data );
			}
		}
		break;

		case TokenType_Integer:
		{
			scanner.next();
			return ast.add( NodeInteger( token.integer ) );
		}
		break;

		case TokenType_Number:
		{
			scanner.next();
			return ast.add( NodeNumber( token.number ) );
		}
		break;

		case TokenType_True:
		case TokenType_False:
		{
			scanner.next();
			return ast.add( NodeBoolean( token.type == TokenType_True ? true : false ) );
		}
		break;

		case TokenType_LParen:
		{
			token = scanner.next();
			Node *node = ast.add( NodeGroup( parse_expression() ) );

			token = scanner.current();
			ErrorIf( token.type != TokenType_RParen, "missing closing ')' on group" );

			scanner.next();
			return node;
		};
		break;

		default:
		{
			Error( "unexpected symbol (token: %u)", token.type );
		}
		break;
	}

	// Unreachable
	Error( "unreacheable" );
	return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static String line_string( const Scanner &scanner, const u32 line, u32 &linePosition )
{
	usize l = 1;
	usize s = 0;

	String file = scanner.buffer;
	while( l < line && s != USIZE_MAX ) { s = file.find( "\n", s ) + 1; l++; }
	if( s == USIZE_MAX ) { return "unknown line"; }
	usize e = min( file.find( "\n", s ), file.capacity );
	if( s == e ) { return "unknown line"; }

	linePosition = s;
	return file.substr( s, e );
}

// Override Error Macros
void Parser::ERROR_HANDLER_FUNCTION_DECL
{
	// Early-out if we've already errored
	if( Debug::exitCode != 0 ) { return; }

	// Print Label
	PrintColor( LOG_RED, "\n\nSHADER COMPILE ERROR:\n\n" );

	// Print Message
	va_list args;
	va_start( args, message );
	PrintColor( LOG_WHITE, "    " );
	Debug::print_formatted_variadic_color( true, LOG_WHITE, message, args );
	va_end( args );

	// Print File
	PrintColor( LOG_RED, "\n    %s:%u\n", path, scanner.current().line );

	// Print Line
	u32 linePosition;
	String errorLine = line_string( scanner, scanner.current().line, linePosition );
	u32 spacePosition = scanner.current().start - linePosition;
	String spaces = "";
	for( usize i = 0; i < spacePosition; i++ ) { spaces += ( errorLine[i] == '\t' ?  "" : "~" ); }

	PrintColor( LOG_RED, "\n" );
	PrintColor( LOG_RED, "Line %u:\n", scanner.current().line );
	PrintColor( LOG_RED, TAB "%s\n", errorLine.replace( "\t", "" ).cstr() );
	PrintColor( LOG_RED, "~~~~%s^\n", spaces.cstr() );

	// Exit
	Debug::exit( 1 );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}