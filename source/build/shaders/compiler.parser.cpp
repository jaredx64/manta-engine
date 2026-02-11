#include <build/shaders/compiler.parser.hpp>

#include <build/system.hpp>

#include <core/math.hpp>

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
	"cos",                       // Intrinsic_Cos
	"sin",                       // Intrinsic_Sin
	"tan",                       // Intrinsic_Tan
	"sinh",                      // Intrinsic_Sinh
	"cosh",                      // Intrinsic_Cosh
	"tanh",                      // Intrinsic_Tanh
	"asin",                      // Intrinsic_ASin
	"acos",                      // Intrinsic_ACos
	"atan",                      // Intrinsic_ATan
	"atan2",                     // Intrinsic_ATan2

	// Exponential and Logarithmic Functions
	"exp",                       // Intrinsic_Exp
	"exp2",                      // Intrinsic_Exp2
	"log",                       // Intrinsic_Log
	"log2",                      // Intrinsic_Log2

	// Conversion Functions
	"degrees",                   // Intrinsic_Degrees
	"radians",                   // Intrinsic_Radians
	"round",                     // Intrinsic_Round
	"trunc",                     // Intrinsic_Trunc
	"ceil",                      // Intrinsic_Ceil
	"floor",                     // Intrinsic_Floor

	// Arithmetic Functions
	"abs",                       // Intrinsic_Abs
	"pow",                       // Intrinsic_Pow
	"sqrt",                      // Intrinsic_Sqrt
	"rsqrt",                     // Intrinsic_RSqrt
	"clamp",                     // Intrinsic_Clamp
	"max",                       // Intrinsic_Max
	"min",                       // Intrinsic_Min
	"mod",                       // Intrinsic_Mod
	"frac",                      // Intrinsic_Frac
	"ldexp",                     // Intrinsic_Ldexp
	"fma",                       // Intrinsic_Fma
	"sign",                      // Intrinsic_Sign
	"saturate",                  // Intrinsic_Saturate
	"ddx",                       // Intrinsic_DDx
	"ddy",                       // Intrinsic_DDy
	"ddx_coarse",                // Intrinsic_DDxCoarse
	"ddx_fine",                  // Intrinsic_DDxFine
	"ddy_coarse",                // Intrinsic_DDyCoarse
	"ddy_fine",                  // Intrinsic_DDyFine

	// Vector and Matrix Functions
	"mul",                       // Intrinsic_Mul
	"length",                    // Intrinsic_Length
	"distance",                  // Intrinsic_Distance
	"dot",                       // Intrinsic_Dot
	"cross",                     // Intrinsic_Cross
	"normalize",                 // Intrinsic_Normalize
	"reflect",                   // Intrinsic_Reflect
	"refract",                   // Intrinsic_Refract
	"faceforward",               // Intrinsic_Faceforward
	"transpose",                 // Intrinsic_Transpose
	"determinant",               // Intrinsic_Determinant
	"lerp",                      // Intrinsic_Lerp
	"step",                      // Intrinsic_Step
	"smoothstep",                // Intrinsic_Smoothstep

	// Bitwise Operations
	"bit_count",                 // Intrinsic_BitCount
	"bit_firsthigh",             // Intrinsic_BitFirstHigh
	"bit_firstlow",              // Intrinsic_BitFirstLow
	"bit_reverse",               // Intrinsic_BitReverse
	"atomic_add",                // Intrinsic_AtomicAdd
	"atomic_compare_exchange",   // Intrinsic_AtomicCompareExchange
	"atomic_exchange",           // Intrinsic_AtomicExchange
	"atomic_max",                // Intrinsic_AtomicMax
	"atomic_min",                // Intrinsic_AtomicMin
	"atomic_and",                // Intrinsic_AtomicAnd
	"atomic_or",                 // Intrinsic_AtomicOr
	"atomic_xor",                // Intrinsic_AtomicXor

	// Type Bit Conversions
	"float_to_int_bits",         // Intrinsic_FloatToIntBits
	"float_to_uint_bits",        // Intrinsic_FloatToUIntBits
	"int_to_float_bits",         // Intrinsic_IntToFloatBits
	"uint_to_float_bits",        // Intrinsic_UIntToFloatBits

	// Texture Sampling Functions
	"texture_sample_1d",         // Intrinsic_TextureSample1D
	"texture_sample_1d_array",   // Intrinsic_TextureSample1DArray
	"texture_sample_1d_level",   // Intrinsic_TextureSample1DLevel
	"texture_sample_2d",         // Intrinsic_TextureSample2D
	"texture_sample_2d_array",   // Intrinsic_TextureSample2DArray
	"texture_sample_2d_level",   // Intrinsic_TextureSample2DLevel
	"texture_sample_3d",         // Intrinsic_TextureSample3D
	"texture_sample_3d_array",   // Intrinsic_TextureSample3DArray
	"texture_sample_3d_level",   // Intrinsic_TextureSample3DLevel
	"texture_sample_cube",       // Intrinsic_TextureSampleCube
	"texture_sample_cube_array", // Intrinsic_TextureSampleCubeArray
	"texture_sample_cube_level", // Intrinsic_TextureSampleCubeLevel
	"texture_index_1d",          // Intrinsic_TextureLoad1D
	"texture_index_2d",          // Intrinsic_TextureLoad2D
	"texture_index_3d",          // Intrinsic_TextureLoad3D
	"texture_index_cube",        // Intrinsic_TextureLoadCube

	// Depth
	"depth_normalize",           // Intrinsic_DepthNormalize
	"depth_linearize",           // Intrinsic_DepthLinearize
	"depth_unproject",           // Intrinsic_DepthUnproject
	"depth_unproject_zw",        // Intrinsic_DepthUnprojectZW
};
static_assert( ARRAY_LENGTH( Intrinsics ) == INTRINSIC_COUNT, "Missing Intrinsic!" );


const char *StructTypeNames[] =
{
	"struct",
	"shared_struct",
	"uniform_buffer",
	"constant_buffer",
	"mutable_buffer",
	"instance_input",
	"vertex_input",
	"vertex_output",
	"fragment_input",
	"fragment_output",
};
static_assert( ARRAY_LENGTH( StructTypeNames ) == STRUCTTYPE_COUNT, "Missing StructType!" );


const char *SVSemantics[] =
{
	"SV_VertexID",
	"SV_InstanceID",
	"SV_PrimitiveID",
	"SV_SampleID",
	"SV_IsFrontFace",
	"SV_DispatchThreadID",
	"SV_GroupThreadID",
	"SV_GroupID",
	"SV_GroupIndex",
};
static_assert( ARRAY_LENGTH( SVSemantics ) == SVSEMANTICTYPE_COUNT, "Missing SVSemantic!" );


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
	{ "in", TokenType_In },
	{ "out", TokenType_Out },
	{ "inout", TokenType_InOut },
	{ "true", TokenType_True },
	{ "false", TokenType_False },
	{ "const", TokenType_Const },
	{ "return", TokenType_Return },
	{ "break", TokenType_Break },
	{ "continue", TokenType_Continue },
	{ "switch", TokenType_Switch },
	{ "case", TokenType_Case },
	{ "default", TokenType_Default },
	{ "discard", TokenType_Discard },
	{ "if", TokenType_If },
	{ "else", TokenType_Else },
	{ "while", TokenType_While },
	{ "do", TokenType_Do },
	{ "for", TokenType_For },
	{ "struct", TokenType_Struct },
	{ "shared_struct", TokenType_SharedStruct },
	{ "uniform_buffer", TokenType_UniformBuffer },
	{ "constant_buffer", TokenType_ConstantBuffer },
	{ "mutable_buffer", TokenType_MutableBuffer },
	{ "instance_input", TokenType_InstanceInput },
	{ "vertex_input", TokenType_VertexInput },
	{ "vertex_output", TokenType_VertexOutput },
	{ "fragment_input", TokenType_FragmentInput },
	{ "fragment_output", TokenType_FragmentOutput },
	{ "texture1D", TokenType_Texture1D },
	{ "texture1DArray", TokenType_Texture1DArray },
	{ "texture2D", TokenType_Texture2D },
	{ "texture2DArray", TokenType_Texture2DArray },
	{ "texture3D", TokenType_Texture3D },
	{ "textureCube", TokenType_TextureCube },
	{ "textureCubeArray", TokenType_TextureCubeArray },
	{ "position_out", TokenType_AttributePositionOut },
	{ "position_in", TokenType_AttributePositionIn },
	{ "target", TokenType_AttributeTarget },
	{ "DEPTH", TokenType_DEPTH },
	{ "COLOR", TokenType_COLOR },
	{ "packed_as", TokenType_AttributePackedAs },
	{ "UNORM8", TokenType_UNORM8 },
	{ "UNORM16", TokenType_UNORM16 },
	{ "UNORM32", TokenType_UNORM32 },
	{ "SNORM8", TokenType_SNORM8 },
	{ "SNORM16", TokenType_SNORM16 },
	{ "SNORM32", TokenType_SNORM32 },
	{ "UINT8", TokenType_UINT8 },
	{ "UINT16", TokenType_UINT16 },
	{ "UINT32", TokenType_UINT32 },
	{ "SINT8", TokenType_SINT8 },
	{ "SINT16", TokenType_SINT16 },
	{ "SINT32", TokenType_SINT32 },
	{ "FLOAT16", TokenType_FLOAT16 },
	{ "FLOAT32", TokenType_FLOAT32 },
};
static_assert( ARRAY_LENGTH( KeywordNames ) == TOKENTYPE_KEYWORD_COUNT, "Missing Keyword TokenType!" );


const Directive DirectiveNames[] =
{
	{ "#include", TokenType_Directive_Include },
	{ "#define", TokenType_Directive_Define },
	{ "#undef", TokenType_Directive_Undefine },
	{ "#if", TokenType_Directive_If },
	{ "#ifdef", TokenType_Directive_IfDefined },
	{ "#ifndef", TokenType_Directive_IfUndefined },
	{ "#else",  TokenType_Directive_Else },
	{ "#elif", TokenType_Directive_ElseIf },
	{ "#endif", TokenType_Directive_EndIf },
	{ "#pragma", TokenType_Directive_Pragma },
	{ "once", TokenType_Directive_Once },
	{ "defined", TokenType_Directive_Defined },
	{ "undefined", TokenType_Directive_Undefined },
};
static_assert( ARRAY_LENGTH( DirectiveNames ) == TOKENTYPE_DIRECTIVE_COUNT, "Missing Directive TokenType!" );


const char *SwizzleTypeNames[] =
{
	"x", "y", "z", "w", "r", "g", "b", "a",
	"xx", "yx", "zx", "wx", "xy", "yy", "zy", "wy",
	"xz", "yz", "zz", "wz", "xw", "yw", "zw", "ww",
	"rr", "gr", "br", "ar", "rg", "gg", "bg", "ag",
	"rb", "gb", "bb", "ab", "ra", "ga", "ba", "aa",
	"xxx", "yxx", "zxx", "wxx", "xyx", "yyx", "zyx", "wyx",
	"xzx", "yzx", "zzx", "wzx", "xwx", "ywx", "zwx", "wwx",
	"xxy", "yxy", "zxy", "wxy", "xyy", "yyy", "zyy", "wyy",
	"xzy", "yzy", "zzy", "wzy", "xwy", "ywy", "zwy", "wwy",
	"xxz", "yxz", "zxz", "wxz", "xyz", "yyz", "zyz", "wyz",
	"xzz", "yzz", "zzz", "wzz", "xwz", "ywz", "zwz", "wwz",
	"xxw", "yxw", "zxw", "wxw", "xyw", "yyw", "zyw", "wyw",
	"xzw", "yzw", "zzw", "wzw", "xww", "yww", "zww", "www",
	"rrr", "grr", "brr", "arr", "rgr", "ggr", "bgr", "agr",
	"rbr", "gbr", "bbr", "abr", "rar", "gar", "bar", "aar",
	"rrg", "grg", "brg", "arg", "rgg", "ggg", "bgg", "agg",
	"rbg", "gbg", "bbg", "abg", "rag", "gag", "bag", "aag",
	"rrb", "grb", "brb", "arb", "rgb", "ggb", "bgb", "agb",
	"rbb", "gbb", "bbb", "abb", "rab", "gab", "bab", "aab",
	"rra", "gra", "bra", "ara", "rga", "gga", "bga", "aga",
	"rba", "gba", "bba", "aba", "raa", "gaa", "baa", "aaa",
	"xxxx", "yxxx", "zxxx", "wxxx", "xyxx", "yyxx", "zyxx", "wyxx",
	"xzxx", "yzxx", "zzxx", "wzxx", "xwxx", "ywxx", "zwxx", "wwxx",
	"xxyx", "yxyx", "zxyx", "wxyx", "xyyx", "yyyx", "zyyx", "wyyx",
	"xzyx", "yzyx", "zzyx", "wzyx", "xwyx", "ywyx", "zwyx", "wwyx",
	"xxzx", "yxzx", "zxzx", "wxzx", "xyzx", "yyzx", "zyzx", "wyzx",
	"xzzx", "yzzx", "zzzx", "wzzx", "xwzx", "ywzx", "zwzx", "wwzx",
	"xxwx", "yxwx", "zxwx", "wxwx", "xywx", "yywx", "zywx", "wywx",
	"xzwx", "yzwx", "zzwx", "wzwx", "xwwx", "ywwx", "zwwx", "wwwx",
	"xxxy", "yxxy", "zxxy", "wxxy", "xyxy", "yyxy", "zyxy", "wyxy",
	"xzxy", "yzxy", "zzxy", "wzxy", "xwxy", "ywxy", "zwxy", "wwxy",
	"xxyy", "yxyy", "zxyy", "wxyy", "xyyy", "yyyy", "zyyy", "wyyy",
	"xzyy", "yzyy", "zzyy", "wzyy", "xwyy", "ywyy", "zwyy", "wwyy",
	"xxzy", "yxzy", "zxzy", "wxzy", "xyzy", "yyzy", "zyzy", "wyzy",
	"xzzy", "yzzy", "zzzy", "wzzy", "xwzy", "ywzy", "zwzy", "wwzy",
	"xxwy", "yxwy", "zxwy", "wxwy", "xywy", "yywy", "zywy", "wywy",
	"xzwy", "yzwy", "zzwy", "wzwy", "xwwy", "ywwy", "zwwy", "wwwy",
	"xxxz", "yxxz", "zxxz", "wxxz", "xyxz", "yyxz", "zyxz", "wyxz",
	"xzxz", "yzxz", "zzxz", "wzxz", "xwxz", "ywxz", "zwxz", "wwxz",
	"xxyz", "yxyz", "zxyz", "wxyz", "xyyz", "yyyz", "zyyz", "wyyz",
	"xzyz", "yzyz", "zzyz", "wzyz", "xwyz", "ywyz", "zwyz", "wwyz",
	"xxzz", "yxzz", "zxzz", "wxzz", "xyzz", "yyzz", "zyzz", "wyzz",
	"xzzz", "yzzz", "zzzz", "wzzz", "xwzz", "ywzz", "zwzz", "wwzz",
	"xxwz", "yxwz", "zxwz", "wxwz", "xywz", "yywz", "zywz", "wywz",
	"xzwz", "yzwz", "zzwz", "wzwz", "xwwz", "ywwz", "zwwz", "wwwz",
	"xxxw", "yxxw", "zxxw", "wxxw", "xyxw", "yyxw", "zyxw", "wyxw",
	"xzxw", "yzxw", "zzxw", "wzxw", "xwxw", "ywxw", "zwxw", "wwxw",
	"xxyw", "yxyw", "zxyw", "wxyw", "xyyw", "yyyw", "zyyw", "wyyw",
	"xzyw", "yzyw", "zzyw", "wzyw", "xwyw", "ywyw", "zwyw", "wwyw",
	"xxzw", "yxzw", "zxzw", "wxzw", "xyzw", "yyzw", "zyzw", "wyzw",
	"xzzw", "yzzw", "zzzw", "wzzw", "xwzw", "ywzw", "zwzw", "wwzw",
	"xxww", "yxww", "zxww", "wxww", "xyww", "yyww", "zyww", "wyww",
	"xzww", "yzww", "zzww", "wzww", "xwww", "ywww", "zwww", "wwww",
	"rrrr", "grrr", "brrr", "arrr", "rgrr", "ggrr", "bgrr", "agrr",
	"rbrr", "gbrr", "bbrr", "abrr", "rarr", "garr", "barr", "aarr",
	"rrgr", "grgr", "brgr", "argr", "rggr", "gggr", "bggr", "aggr",
	"rbgr", "gbgr", "bbgr", "abgr", "ragr", "gagr", "bagr", "aagr",
	"rrbr", "grbr", "brbr", "arbr", "rgbr", "ggbr", "bgbr", "agbr",
	"rbbr", "gbbr", "bbbr", "abbr", "rabr", "gabr", "babr", "aabr",
	"rrar", "grar", "brar", "arar", "rgar", "ggar", "bgar", "agar",
	"rbar", "gbar", "bbar", "abar", "raar", "gaar", "baar", "aaar",
	"rrrg", "grrg", "brrg", "arrg", "rgrg", "ggrg", "bgrg", "agrg",
	"rbrg", "gbrg", "bbrg", "abrg", "rarg", "garg", "barg", "aarg",
	"rrgg", "grgg", "brgg", "argg", "rggg", "gggg", "bggg", "aggg",
	"rbgg", "gbgg", "bbgg", "abgg", "ragg", "gagg", "bagg", "aagg",
	"rrbg", "grbg", "brbg", "arbg", "rgbg", "ggbg", "bgbg", "agbg",
	"rbbg", "gbbg", "bbbg", "abbg", "rabg", "gabg", "babg", "aabg",
	"rrag", "grag", "brag", "arag", "rgag", "ggag", "bgag", "agag",
	"rbag", "gbag", "bbag", "abag", "raag", "gaag", "baag", "aaag",
	"rrrb", "grrb", "brrb", "arrb", "rgrb", "ggrb", "bgrb", "agrb",
	"rbrb", "gbrb", "bbrb", "abrb", "rarb", "garb", "barb", "aarb",
	"rrgb", "grgb", "brgb", "argb", "rggb", "gggb", "bggb", "aggb",
	"rbgb", "gbgb", "bbgb", "abgb", "ragb", "gagb", "bagb", "aagb",
	"rrbb", "grbb", "brbb", "arbb", "rgbb", "ggbb", "bgbb", "agbb",
	"rbbb", "gbbb", "bbbb", "abbb", "rabb", "gabb", "babb", "aabb",
	"rrab", "grab", "brab", "arab", "rgab", "ggab", "bgab", "agab",
	"rbab", "gbab", "bbab", "abab", "raab", "gaab", "baab", "aaab",
	"rrra", "grra", "brra", "arra", "rgra", "ggra", "bgra", "agra",
	"rbra", "gbra", "bbra", "abra", "rara", "gara", "bara", "aara",
	"rrga", "grga", "brga", "arga", "rgga", "ggga", "bgga", "agga",
	"rbga", "gbga", "bbga", "abga", "raga", "gaga", "baga", "aaga",
	"rrba", "grba", "brba", "arba", "rgba", "ggba", "bgba", "agba",
	"rbba", "gbba", "bbba", "abba", "raba", "gaba", "baba", "aaba",
	"rraa", "graa", "braa", "araa", "rgaa", "ggaa", "bgaa", "agaa",
	"rbaa", "gbaa", "bbaa", "abaa", "raaa", "gaaa", "baaa", "aaaa",
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
				position++;
			break;

			case '\r':
			case '\n':
				line++;
				position++;
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


static bool is_letter( char c )
{
	return ( static_cast<unsigned int>( c ) | 32 ) - 'a' < 26;
}


static bool is_digit( char c )
{
	return ( static_cast<unsigned int>( c ) ) - '0' < 10;
}


static bool is_digit_hex( char c )
{
	return is_digit( c ) || ( c >= 'A' && c <= 'F' );
}


static TokenType get_keyword( const char *buffer, u32 length )
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


static TokenType get_directive( const char *buffer, u32 length )
{
	const u32 directiveCount = sizeof( DirectiveNames ) / sizeof( DirectiveNames[0] );
	for( u32 i = 0; i < directiveCount; i++ )
	{
		const Directive &directive = DirectiveNames[i];
		if( strlen( directive.key ) != length ) { continue; }
		if( strncmp( buffer, directive.key, length ) == 0 )
		{
			// Token Found
			return directive.token;
		}
	}

	return TokenType_Error;
}


Token Scanner::next( bool skipWhitespace )
{
	Token token;
	#define RETURN_TOKEN_NAME(...) \
		token = Token { __VA_ARGS__ }; goto success;
	#define RETURN_TOKEN(...) \
		token = Token { __VA_ARGS__, StringView { &buffer[start], position - start } }; goto success;

	if( skipWhitespace ) { skip_whitespace(); }
	usize start = position;

	const char c = buffer[position++];
	switch( c )
	{
		// End of File
		case '\0':
		{
			position--;
			RETURN_TOKEN( TokenType_EndOfFile );
		}

		// Whitespace
		case '\r':
		case '\n': { RETURN_TOKEN( TokenType_Newline ); }
		case ' ': { RETURN_TOKEN( TokenType_Space ); }
		case '\t': { RETURN_TOKEN( TokenType_Tab ); }

		// Single Character
		case '(': { RETURN_TOKEN( TokenType_LParen ); }
		case ')': { RETURN_TOKEN( TokenType_RParen ); }
		case '{': { RETURN_TOKEN( TokenType_LCurly ); }
		case '}': { RETURN_TOKEN( TokenType_RCurly ); }
		case '[': { RETURN_TOKEN( TokenType_LBrack ); }
		case ']': { RETURN_TOKEN( TokenType_RBrack ); }
		case '.': { RETURN_TOKEN( TokenType_Dot ); }
		case ',': { RETURN_TOKEN( TokenType_Comma ); }
		case ':': { RETURN_TOKEN( TokenType_Colon ); }
		case ';': { RETURN_TOKEN( TokenType_Semicolon ); }
		case '~': { RETURN_TOKEN( TokenType_BitNot ); }
		case '?': { RETURN_TOKEN( TokenType_Question ); }
		case '\"': { RETURN_TOKEN( TokenType_Quote ); }
		case '\\': { RETURN_TOKEN( TokenType_Escape ); }

		// Double Character
		case '_':
		{
			if( consume( '=' ) ) { RETURN_TOKEN( TokenType_Equals ); }
			RETURN_TOKEN( TokenType_Assign );
		}

		case '=':
		{
			if( consume( '=' ) ) { RETURN_TOKEN( TokenType_Equals ); }
			RETURN_TOKEN( TokenType_Assign );
		}

		case '!':
		{
			if( consume( '=' ) ) { RETURN_TOKEN( TokenType_NotEquals ); }
			RETURN_TOKEN( TokenType_Bang );
		}

		case '+':
		{
			if( consume( '=' ) ) { RETURN_TOKEN( TokenType_PlusAssign ); }
			if( consume( '+' ) ) { RETURN_TOKEN( TokenType_PlusPlus ); }
			RETURN_TOKEN( TokenType_Plus );
		}

		case '-':
		{
			if( consume( '=' ) ) { RETURN_TOKEN( TokenType_MinusAssign ); }
			if( consume( '-' ) ) { RETURN_TOKEN( TokenType_MinusMinus ); }
			RETURN_TOKEN( TokenType_Minus );
		}

		case '*':
		{
			if( consume( '=' ) ) { RETURN_TOKEN( TokenType_StarAssign ); }
			if( consume( '/' ) ) { RETURN_TOKEN( TokenType_CommentEnd ); }
			RETURN_TOKEN( TokenType_Star );
		}

		case '/':
		{
			if( consume( '=' ) ) { RETURN_TOKEN( TokenType_SlashAssign ); }
			if( consume( '/' ) ) { RETURN_TOKEN( TokenType_CommentLine ); }
			if( consume( '*' ) ) { RETURN_TOKEN( TokenType_CommentStart ); }
			RETURN_TOKEN( TokenType_Slash );
		}

		case '%':
		{
			if( consume( '=' ) ) { RETURN_TOKEN( TokenType_ModAssign ); }
			RETURN_TOKEN( TokenType_Mod );
		}

		case '^':
		{
			if( consume( '=' ) ) { RETURN_TOKEN( TokenType_BitXorAssign ); }
			RETURN_TOKEN( TokenType_BitXor );
		}

		case '|':
		{
			if( consume( '=' ) ) { RETURN_TOKEN( TokenType_BitOrAssign ); }
			if( consume( '|' ) ) { RETURN_TOKEN( TokenType_Or ); }
			RETURN_TOKEN( TokenType_BitOr );
		}

		case '&':
		{
			if( consume( '=' ) ) { RETURN_TOKEN( TokenType_BitAndAssign ); }
			if( consume( '&' ) ) { RETURN_TOKEN( TokenType_And ); }
			RETURN_TOKEN( TokenType_BitAnd );
		}

		// Triple Character
		case '<':
		{
			if( consume( '<' ) )
			{
				if( consume( '=' ) ) { RETURN_TOKEN( TokenType_BitShiftLeftAssign ); }
				RETURN_TOKEN( TokenType_BitShiftLeft );
			}

			if( consume( '=' ) ) { RETURN_TOKEN( TokenType_LessThanEquals ); }
			RETURN_TOKEN( TokenType_LessThan );
		}

		case '>':
		{
			if( consume( '>' ) )
			{
				if( consume( '=' ) ) { RETURN_TOKEN( TokenType_BitShiftRightAssign ); }
				RETURN_TOKEN( TokenType_BitShiftRight );
			}

			if( consume( '=' ) ) { RETURN_TOKEN( TokenType_GreaterThanEquals ); }
			RETURN_TOKEN( TokenType_GreaterThan );
		}

		// Other
		default:
		{
			// Directives / Identifiers
			const bool preprocessing = ( mode == ScannerMode_Preprocessor );
			if( is_letter( c ) || c == '_' || ( preprocessing && c == '#' ) )
			{
				while( ( buffer[position] != '\0' ) &&
					( is_digit( buffer[position] ) || is_letter( buffer[position] ) || buffer[position] == '_' ) )
				{
					position++;
				}

				TokenType keyword = get_keyword( &buffer[start], position - start );
				if( keyword < 0 && preprocessing ) { keyword = get_directive( &buffer[start], position - start ); }

				if( keyword >= 0 ) { RETURN_TOKEN( keyword ); }
				RETURN_TOKEN_NAME( TokenType_Identifier, StringView { &buffer[start], position - start } );
			}

			// Numbers
			if( is_digit( c ) )
			{
				bool seenDot = false;
				bool seenHex = false;

				while
				(
					buffer[position] != '\0' &&
					( is_digit_hex( buffer[position] ) || buffer[position] == '.' || buffer[position] == 'x' )
				)
				{
					if( buffer[position] == '.' )
					{
						if( seenDot ) { RETURN_TOKEN( TokenType_Error ); }
						seenDot = true;
					}
					else if( buffer[position] == 'x' )
					{
						if( seenHex ) { RETURN_TOKEN( TokenType_Error ); }
						seenHex = true;
					}

					position++;
				}

				// Why would you have a number this big??
				char number[64];
				if( position - start >= sizeof( number ) - 1 )
				{
					RETURN_TOKEN( TokenType_Error );
				}
				strncpy( number, &buffer[start], position - start );
				number[position - start] = '\0';

				if( seenDot )
				{
					// Floating Point
					RETURN_TOKEN( TokenType_Number, atof( number ) );
				}
				else if( seenHex )
				{
					// Hex
					const u64 hexInteger = strtoull( number + 2, nullptr, 16 );
					RETURN_TOKEN( TokenType_Integer, hexInteger );
				}
				else
				{
					// Integer
					RETURN_TOKEN( TokenType_Integer, static_cast<u64>( atoll( number ) ) );
				}
			}

			// Error: Unknown token?
			RETURN_TOKEN( TokenType_Error );
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
	if( stack.size() == 0 ) { return Token { TokenType_Error, StringView { } }; }
	stack.remove( stack.size() - 1 );

	Token token = stack[stack.size() - 1];
	position = token.position;
	line = token.line;
	return token;
}


Token Scanner::current()
{
	if( stack.size() == 0 ) { return Token { TokenType_Error, StringView { } }; }
	return stack[stack.size() - 1];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

StructID Parser::register_struct( Struct structure )
{
	StructID structID = structs.size();
	structs.add( structure );
	return structID;
}


TextureID Parser::register_texture( Texture texture )
{
	TextureID textureID = textures.size();
	textures.add( texture );
	Variable &textureVariable = variables[texture.variableID];
	textureMap.add( textureVariable.name, textureID );
	return textureID;
}


TypeID Parser::register_type( Type type )
{
	TypeID typeID = types.size();
	types.add( type );
	typeMap.add( type.name, typeID );
	return typeID;
}


FunctionID Parser::register_function( Function function )
{
	FunctionID functionID = functions.size();
	functions.add( function );
	functionMap.add( function.name, functionID );
	return functionID;
}


VariableID Parser::register_variable( Variable variable )
{
	VariableID variableID = variables.size();
	variables.add( variable );
	scope.add( variableID );
	return variableID;
}


void Parser::register_swizzles()
{
	const int swizzleCount = ARRAY_LENGTH( SwizzleTypeNames );
	for( int i = 0; i < swizzleCount; i++ )
	{
		swizzleMap.add( StringView { SwizzleTypeNames[i], strlen( SwizzleTypeNames[i] ) }, i );
	}
}


void Parser::register_svsemantics()
{
	for( u32 i = 0; i < SVSEMANTICTYPE_COUNT; i++ )
	{
		svSemanticMap.add( StringView { SVSemantics[i], strlen( SVSemantics[i] ) }, i );
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
			return variables[texture.variableID].texture;
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


VariableID Parser::scope_find_variable( StringView name )
{
	for( VariableID i = 0; i < scope.size(); i++ )
	{
		Variable &variable = variables[scope[i]];
		if( variable.name.length != name.length ) { continue; }
		if( strncmp( variable.name.data, name.data, name.length ) == 0 ) { return scope[i]; }
	}
	return USIZE_MAX;
}


void Parser::scope_reset( VariableID target )
{
	for( VariableID i = scope.size(); i > target; i-- )
	{
		scope.remove( i - 1 );
	}
}


void Parser::check_namespace_conflicts( StringView name )
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

	// Built-in SVSemantics
	register_svsemantics();

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
	for( u32 i = 0; i < SHADER_MAX_TARGET_SLOTS; i++ ) { targetColorSlots[i] = false; }
	for( u32 i = 0; i < SHADER_MAX_TARGET_SLOTS; i++ ) { targetDepthSlots[i] = false; }
}


bool Parser::parse( const char *string )
{
	// Initialize
	init();
	scanner.init( string );
	scanner.mode = Scanner::ScannerMode_Compiler;

	// Parse Definitions
	for( ;; )
	{
		Token token = scanner.next();
		ErrorIf( token.type == TokenType_Error, "unknown token '%.*s'",
			token.name.length, token.name.data  );
		if( token.type == TokenType_EndOfFile ) { break; }

		// Program-level Tokens
		switch( token.type )
		{
			case TokenType_Struct:
			case TokenType_SharedStruct:
			case TokenType_UniformBuffer:
			case TokenType_ConstantBuffer:
			case TokenType_MutableBuffer:
			case TokenType_InstanceInput:
			case TokenType_VertexInput:
			case TokenType_VertexOutput:
			case TokenType_FragmentInput:
			case TokenType_FragmentOutput:
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
		Error( "shader does not implement a main function!" );
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool vertex_input_type_allowed( TypeID typeID )
{
	switch( typeID )
	{
		case Primitive_Bool:
		case Primitive_Int:
		case Primitive_UInt:
		case Primitive_Float:
		case Primitive_Bool2:
		case Primitive_Int2:
		case Primitive_UInt2:
		case Primitive_Float2:
		case Primitive_Bool3:
		case Primitive_Int3:
		case Primitive_UInt3:
		case Primitive_Float3:
		case Primitive_Bool4:
		case Primitive_Int4:
		case Primitive_UInt4:
		case Primitive_Float4:
			// Allowed type
		return true;
	}

	// Illegal type
	return false;
}


static bool instance_input_type_allowed( TypeID typeID )
{
	switch( typeID )
	{
		case Primitive_Bool:
		case Primitive_Int:
		case Primitive_UInt:
		case Primitive_Float:
		case Primitive_Bool2:
		case Primitive_Int2:
		case Primitive_UInt2:
		case Primitive_Float2:
		case Primitive_Bool3:
		case Primitive_Int3:
		case Primitive_UInt3:
		case Primitive_Float3:
		case Primitive_Bool4:
		case Primitive_Int4:
		case Primitive_UInt4:
		case Primitive_Float4:
		case Primitive_Float4x4:
			// Allowed type
		return true;
	}

	// Illegal type
	return false;
}


static bool buffer_type_allowed( TokenType typeToken, TypeID typeID )
{
	// Shared Structs
	if( typeToken == TokenType_SharedStruct ) { return true; }

	// Primitives
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
	bool expectSize = false;
	switch( token.type )
	{
		case TokenType_Struct:
			structType = StructType_Struct;
			type.global = false;
			type.pipelineVarying = false;
		break;

		case TokenType_SharedStruct:
			structType = StructType_SharedStruct;
			type.global = false;
			type.pipelineVarying = false;
		break;

		case TokenType_UniformBuffer:
			structType = StructType_UniformBuffer;
			expectSlot = true;
			type.global = true;
			type.pipelineVarying = false;
		break;

		case TokenType_ConstantBuffer:
			structType = StructType_ConstantBuffer;
			expectSlot = true;
			expectSize = true;
			type.global = true;
			type.pipelineVarying = false;
		break;

		case TokenType_MutableBuffer:
			structType = StructType_MutableBuffer;
			expectSlot = true;
			expectSize = true;
			type.global = true;
			type.pipelineVarying = false;
		break;

		case TokenType_InstanceInput:
			structType = StructType_InstanceInput;
			type.global = true;
			type.pipelineVarying = false;
		break;

		case TokenType_VertexInput:
			structType = StructType_VertexInput;
			type.global = true;
			type.pipelineVarying = false;
		break;

		case TokenType_VertexOutput:
			structType = StructType_VertexOutput;
			type.global = true;
			type.pipelineVarying = true;
		break;

		case TokenType_FragmentInput:
			structType = StructType_FragmentInput;
			type.global = true;
			type.pipelineVarying = true;
		break;

		case TokenType_FragmentOutput:
			structType = StructType_FragmentOutput;
			type.global = true;
			type.pipelineVarying = false;
		break;

		default: Error( "unknown structure type!" ); break;
	}
	const char *structName = StructTypeNames[structType];

	// Slot
	if( expectSlot )
	{
		// '('
		token = scanner.next();
		ErrorIf( token.type != TokenType_LParen, "%s: expected '(' before slot", structName );

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

		// ')' or ','
		token = scanner.next();
		if( expectSize )
		{
			ErrorIf( token.type != TokenType_Comma, "%s: expected ',' before size", structName );
		}
		else
		{
			ErrorIf( token.type != TokenType_RParen, "%s: expected ')' after slot", structName );
		}
	}

	// Size
	if( expectSize )
	{
		// Integer
		token = scanner.next();
		ErrorIf( token.type != TokenType_Integer,
			"%s: size must be a positive, non-zero integer", structName );
		ErrorIf( token.integer > U32_MAX,
			"%s: size exceeded maximum: %u", structName, U32_MAX );
		ErrorIf( token.integer == 0,
			"%s: size must be at least 1", structName );
		structure.size = static_cast<int>( token.integer );

		// ')' or ','
		token = scanner.next();
		ErrorIf( token.type != TokenType_RParen, "%s: expected ')' after size", structName );
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
	type.slot = structure.slot;
	bool parseAttribute = false;
	bool seenAttributePosition = false;
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

		if( structType == StructType_SharedStruct ||
			structType == StructType_UniformBuffer ||
			structType == StructType_ConstantBuffer ||
			structType == StructType_MutableBuffer )
		{
			const bool allowedType = buffer_type_allowed( variableType.tokenType, variable.typeID );
			ErrorIf( !allowedType, "Type not allowed in this structure! Must be a primitive or shared_struct" );
		}
		else if( structType == StructType_InstanceInput )
		{
			const bool allowedType = instance_input_type_allowed( variable.typeID );
			ErrorIf( !allowedType, "Type not allowed in instance_input!" );
		}
		else if( structType == StructType_VertexInput )
		{
			const bool allowedType = vertex_input_type_allowed( variable.typeID );
			ErrorIf( !allowedType, "Type not allowed in vertex_input! Must be a primitive, non-matrix type" );
		}

		// Parse Attributes
		auto parse_attribute_position_vertex = [&]()
			{
				ErrorIf( token.type != TokenType_AttributePositionOut,
					"unexpected attribute '%.*s' -- must be 'position_out'",
					token.name.length, token.name.data );
				token = scanner.next();

				variable.semantic = SemanticType_POSITION;
				seenAttributePosition = true;
			};

		auto parse_attribute_position_fragment = [&]()
			{
				ErrorIf( token.type != TokenType_AttributePositionIn,
					"unexpected attribute '%.*s' -- must be 'position_in'",
					token.name.length, token.name.data );
				token = scanner.next();

				variable.semantic = SemanticType_POSITION;
				seenAttributePosition = true;
			};

		auto parse_attribute_pack_as = [&]()
			{
				ErrorIf( token.type != TokenType_AttributePackedAs,
					"unexpected attribute '%.*s' -- must be 'packed_as(...)'",
					token.name.length, token.name.data );
				token = scanner.next();

				ErrorIf( token.type != TokenType_LParen,
					"expected '(' before format type" );
				token = scanner.next();

				ErrorIf( token.type < TokenType_UNORM8 || token.type > TokenType_FLOAT32,
					"unexpected format type '%.*s'", token.name.length, token.name.data );
				variable.format = ( token.type - TokenType_UNORM8 );
				token = scanner.next();

				ErrorIf( token.type != TokenType_RParen,
					"expected ')' after format type" );
				token = scanner.next();
			};

		auto parse_attribute_target = [&]()
			{
				ErrorIf( token.type != TokenType_AttributeTarget,
					"unexpected attribute '%.*s' -- must be 'target(slot, type)'",
					token.name.length, token.name.data );
				token = scanner.next();

				ErrorIf( token.type != TokenType_LParen,
					"expected '(' before format type" );
				token = scanner.next();

				ErrorIf( token.type != TokenType_Integer,
					"slot must be a positive, constant integer" );
				const int slot = static_cast<int>( token.integer );
				ErrorIf( slot >= SHADER_MAX_TARGET_SLOTS,
					"slot exceeded maximum: %u", SHADER_MAX_TARGET_SLOTS );
				variable.slot = slot;
				token = scanner.next();

				ErrorIf( token.type != TokenType_Comma,
					"expected ',' after slot" );
				token = scanner.next();

				switch( token.type )
				{
					case TokenType_COLOR:
					{
						variable.semantic = SemanticType_COLOR;
						ErrorIf( targetColorSlots[slot] == true,
							"target( %u, COLOR ) is already bound!", variable.slot );
						targetColorSlots[slot] = true;
					}
					break;

					case TokenType_DEPTH:
					{
						variable.semantic = SemanticType_DEPTH;
						ErrorIf( slot != 0, "DEPTH targets can only be bound to slot 0!" );
						ErrorIf( targetDepthSlots[slot] == true,
							"target( %u, DEPTH ) is already bound!", variable.slot );
						targetColorSlots[slot] = true;
					}
					break;

					default:
						Error( "unexpected format '%.*s' -- must be COLOR or DEPTH",
							token.name.length, token.name.data );
					break;
				}
				token = scanner.next();

				ErrorIf( token.type != TokenType_RParen,
					"expected ')' after format type" );
				token = scanner.next();
			};

		switch( structType )
		{
			case StructType_VertexInput:
				// format(...)
				parse_attribute_pack_as();
				variable.semantic = SemanticType_VERTEX;
			break;

			case StructType_InstanceInput:
				// format(...)
				parse_attribute_pack_as();
				variable.semantic = SemanticType_INSTANCE;
			break;

			case StructType_VertexOutput:
				// position_out
				if( !seenAttributePosition ) { parse_attribute_position_vertex(); break; }
			break;

			case StructType_FragmentInput:
				// position_in
				if( !seenAttributePosition ) { parse_attribute_position_fragment(); }
			break;

			case StructType_FragmentOutput:
				// target(slot, type)
				parse_attribute_target();
			break;
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
			variable.texture = Primitive_Texture1D;
		break;

		case TokenType_Texture1DArray:
			textureType = TextureType_Texture1D;
			variable.texture = Primitive_Texture1DArray;
		break;

		case TokenType_Texture2D:
			textureType = TextureType_Texture2D;
			variable.texture = Primitive_Texture2D;
		break;

		case TokenType_Texture2DArray:
			textureType = TextureType_Texture2D;
			variable.texture = Primitive_Texture2DArray;
		break;

		case TokenType_Texture3D:
			textureType = TextureType_Texture3D;
			variable.texture = Primitive_Texture3D;
		break;

		case TokenType_TextureCube:
			textureType = TextureType_TextureCube;
			variable.texture = Primitive_TextureCube;
		break;

		case TokenType_TextureCubeArray:
			textureType = TextureType_TextureCubeArray;
			variable.texture = Primitive_TextureCubeArray;
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

	// Type
	token = scanner.next();
	ErrorIf( token.type != TokenType_Comma, "%s: expected ',' before type", textureName );
	token = scanner.next();
	ErrorIf( token.type != TokenType_Identifier, "%s: expected a texture type", textureName );
	ErrorIf( !typeMap.contains( token.name ),
		"%s: unknown type '%.*s'", textureName, token.name.length, token.name.data );
	variable.typeID = typeMap.get( token.name );
	token = scanner.next();
	ErrorIf( token.type != TokenType_RParen, "%s: expected ')' after type", textureName );

	// Name
	token = scanner.next();
	ErrorIf( token.type != TokenType_Identifier,
		"%s: expected name after %s(slot, type) keyword", textureName, textureName );
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

		case TokenType_Continue:
		{
			Node *node = parse_statement_continue();
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

			case TokenType_Continue:
			{
				current = ast.add( NodeStatementBlock( parse_statement_continue() ) );
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
	else if( token.type == TokenType_Case || token.type == TokenType_Break ||
		token.type == TokenType_Default || token.type == TokenType_Return ||
		token.type == TokenType_Continue )
	{
		block = nullptr;
	}
	else
	{
		block = ast.add( NodeStatementBlock( parse_statement() ) );
	}

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
	else if( token.type == TokenType_Case || token.type == TokenType_Break ||
		token.type == TokenType_Default || token.type == TokenType_Return ||
		token.type == TokenType_Continue )
	{
		block = nullptr;
	}
	else
	{
		block = ast.add( NodeStatementBlock( parse_statement() ) );
	}

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

	return ast.add( NodeStatementReturn( expr ) );
}


Node *Parser::parse_statement_break()
{
	// Expect 'break'
	Token token = scanner.current();
	ErrorIf( token.type != TokenType_Break, "unexpected token" );

	scanner.next();
	return ast.add( NodeStatementBreak() );
}


Node *Parser::parse_statement_continue()
{
	// Expect 'break'
	Token token = scanner.current();
	ErrorIf( token.type != TokenType_Continue, "unexpected token" );

	scanner.next();
	return ast.add( NodeStatementContinue() );
}


Node *Parser::parse_statement_discard()
{
	// Expect 'discard'
	Token token = scanner.current();
	ErrorIf( token.type != TokenType_Discard, "unexpected token" );

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
	if( token.type == TokenType_InstanceInput ||
		token.type == TokenType_VertexInput ||
		token.type == TokenType_FragmentInput )
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
		return parse_function_declaration_main_pipeline( FunctionType_MainVertex, "vertex_main",
			TokenType_VertexInput, TokenType_VertexOutput );
	}
	else if( token.name.length == strlen( "fragment_main" ) &&
		strncmp( "fragment_main", token.name.data, token.name.length ) == 0 )
	{
		scanner.back();
		return parse_function_declaration_main_pipeline( FunctionType_MainFragment, "fragment_main",
			TokenType_FragmentInput, TokenType_FragmentOutput );
	}
	else if( token.name.length == strlen( "compute_main" ) &&
		strncmp( "compute_main", token.name.data, token.name.length ) == 0 )
	{
		scanner.back();
		return parse_function_declaration_main_compute( FunctionType_MainCompute, "compute_main" );
	}
	else if( token.name.length == strlen( "ray_generate" ) &&
		strncmp( "ray_generate", token.name.data, token.name.length ) == 0 )
	{
		scanner.back();
		return parse_function_declaration_main_raytracing( FunctionType_MainRayGenerate, "ray_generate" );
	}
	else if( token.name.length == strlen( "ray_hit_any" ) &&
		strncmp( "ray_hit_any", token.name.data, token.name.length ) == 0 )
	{
		scanner.back();
		return parse_function_declaration_main_raytracing( FunctionType_MainRayHitAny, "ray_hit_any" );
	}
	else if( token.name.length == strlen( "ray_hit_closest" ) &&
		strncmp( "ray_hit_closest", token.name.data, token.name.length ) == 0 )
	{
		scanner.back();
		return parse_function_declaration_main_raytracing( FunctionType_MainRayHitClosest, "ray_hit_closest" );
	}
	else if( token.name.length == strlen( "ray_miss" ) &&
		strncmp( "ray_miss", token.name.data, token.name.length ) == 0 )
	{
		scanner.back();
		return parse_function_declaration_main_raytracing( FunctionType_MainRayMiss, "ray_miss" );
	}
	else if( token.name.length == strlen( "ray_intersection" ) &&
		strncmp( "ray_intersection", token.name.data, token.name.length ) == 0 )
	{
		scanner.back();
		return parse_function_declaration_main_raytracing( FunctionType_MainRayIntersection, "ray_intersection" );
	}
	else if( token.name.length == strlen( "ray_callable" ) &&
		strncmp( "ray_callable", token.name.data, token.name.length ) == 0 )
	{
		scanner.back();
		return parse_function_declaration_main_raytracing( FunctionType_MainRayCallable, "ray_callable" );
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
			   paramType.tokenType == TokenType_SharedStruct ||
			   paramType.tokenType == TokenType_UniformBuffer ||
			   paramType.tokenType == TokenType_ConstantBuffer ||
			   paramType.tokenType == TokenType_MutableBuffer ||
			   paramType.builtin ) )
		{
			scanner.back();
			Error( "function parameter types can only be primitives, struct, or *_buffer" );
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

	return ast.add( NodeFunctionDeclaration( FunctionType_Custom, register_function( function ), block ) );
}


Node *Parser::parse_function_declaration_main_pipeline( FunctionType functionType,
	const char *functionName, TokenType inToken, TokenType outToken )
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
	bool hasInstance = false; // vertex main only

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

			// Vertex Format
			if( functionType == FunctionType_MainVertex )
			{
				vertexFormatTypeName = paramType.name;
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
		// Instance Format (vertex main only)
		if( functionType == FunctionType_MainVertex && paramType.tokenType == TokenType_InstanceInput )
		{
			if( hasInstance )
			{
				scanner.back();
				scanner.back();
				Error( "%s() can only take one instance_input '%s'",
					functionName, KeywordNames[inToken - TOKENTYPE_KEYWORD_FIRST].key );
			}

			instanceFormatTypeName = paramType.name;
			hasInstance = true;
		} else
		// Buffer Parameters Only
		{
			const bool isBufferType = paramType.tokenType == TokenType_UniformBuffer ||
				paramType.tokenType == TokenType_ConstantBuffer ||
				paramType.tokenType == TokenType_MutableBuffer;

			const bool isInstanceInput = paramType.tokenType == TokenType_InstanceInput;

			if( parameterID > 1 )
			{
				// Type 'instance_input' is only allowed in vertex_main
				if( isInstanceInput && functionType != FunctionType_MainVertex )
				{
					scanner.back(); scanner.back();
					Error( "instance_input is not allowed as a parameter to %s()", functionName );
				}
				// Only buffer types are allowed in shader stage entry points
				if( !isBufferType && !isInstanceInput )
				{
					scanner.back(); scanner.back();
					Error( "%s() can only take in additional parameters of type *_buffer", functionName );
				}
			}
		}

		// Comma or ')'
		if( token.type == TokenType_RParen ) { break; }
		ErrorIf( token.type != TokenType_Comma, "function declaration: expected ',' between parameters" );
		token = scanner.next();
		parameterID++;
	};
	function.parameterCount = variables.size() - function.parameterFirst;

	if( functionType == FunctionType_MainVertex )
	{
		ErrorIf( !hasIn, "%s() requires a first parameter of type 'vertex_input'", functionName );
		ErrorIf( !hasOut, "%s() requires a second parameter of type 'vertex_output'", functionName );
	}
	else if( functionType == FunctionType_MainFragment )
	{
		ErrorIf( !hasIn, "%s() requires a first parameter of type 'fragment_input'", functionName );
		ErrorIf( !hasOut, "%s() requires a second parameter of type 'fragment_output'", functionName );
	}

	// Parse Block
	token = scanner.next();
	Node *block = parse_statement_block( scopeIndex );

	FunctionID functionID = register_function( function );
	switch( functionType )
	{
		case FunctionType_MainVertex: mainVertex = functionID; break;
		case FunctionType_MainFragment: mainFragment = functionID; break;
	}

	return ast.add( NodeFunctionDeclaration( functionType, functionID, block ) );
}


Node *Parser::parse_function_declaration_main_compute( FunctionType functionType, const char *functionName )
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

	// Group IDs (compute_main only)
	if( functionType == FunctionType_MainCompute )
	{
		token = scanner.next();
		ErrorIf( token.type != TokenType_LParen, "compute_main: expected '(' before thread groups" );

		token = scanner.next();
		ErrorIf( token.type != TokenType_Integer, "compute_main: expected thread group x" );
		threadGroupX = token.integer;

		token = scanner.next();
		ErrorIf( token.type != TokenType_Comma, "compute_main: expected ','" );

		token = scanner.next();
		ErrorIf( token.type != TokenType_Integer, "compute_main: expected thread group y" );
		threadGroupY = token.integer;

		token = scanner.next();
		ErrorIf( token.type != TokenType_Comma, "compute_main: expected ','" );

		token = scanner.next();
		ErrorIf( token.type != TokenType_Integer, "compute_main: expected thread group z" );
		threadGroupZ = token.integer;

		token = scanner.next();
		ErrorIf( token.type != TokenType_RParen, "compute_main: expected ')' after thread groups" );
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

		Variable &paramVariable = variables[nodeDecl->variableID];
		Type &paramType = types[paramVariable.typeID];

		const bool isBufferType = paramType.tokenType == TokenType_UniformBuffer ||
			paramType.tokenType == TokenType_ConstantBuffer ||
			paramType.tokenType == TokenType_MutableBuffer;

		if( !isBufferType )
		{
			scanner.back(); scanner.back();
			Error( "%s() can only take parameters of type *_buffer", functionName );
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

	FunctionID functionID = register_function( function );
	switch( functionType )
	{
		case FunctionType_MainCompute: mainCompute = functionID; break;
	}

	return ast.add( NodeFunctionDeclaration( functionType, functionID, block ) );
}


Node *Parser::parse_function_declaration_main_raytracing( FunctionType functionType, const char *functionName )
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

		Variable &paramVariable = variables[nodeDecl->variableID];
		Type &paramType = types[paramVariable.typeID];

		const bool isBufferType = paramType.tokenType == TokenType_UniformBuffer ||
			paramType.tokenType == TokenType_ConstantBuffer ||
			paramType.tokenType == TokenType_MutableBuffer;

		if( !isBufferType )
		{
			scanner.back(); scanner.back();
			Error( "%s() can only take parameters of type *_buffer", functionName );
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

	FunctionID functionID = register_function( function );
	switch( functionType )
	{
		case FunctionType_MainRayGenerate: mainRayGenerate = functionID; break;
		case FunctionType_MainRayHitAny: mainRayHitAny = functionID; break;
		case FunctionType_MainRayHitClosest: mainRayHitClosest = functionID; break;
		case FunctionType_MainRayMiss: mainRayMiss = functionID; break;
		case FunctionType_MainRayIntersection: mainRayIntersection = functionID; break;
		case FunctionType_MainRayCallable: mainRayCallable = functionID; break;
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

	return node;
}


Node *Parser::parse_logical_or()
{
	Node *node = parse_logical_and();

	while( scanner.current().type == TokenType_Or )
	{
		scanner.next();
		node = ast.add( NodeExpressionBinary( ExpressionBinaryType_Or, node, parse_logical_and() ) );
	}

	return node;
}


Node *Parser::parse_logical_and()
{
	Node *node = parse_bitwise_or();

	while( scanner.current().type == TokenType_And )
	{
		scanner.next();
		node = ast.add( NodeExpressionBinary( ExpressionBinaryType_And, node,
			parse_bitwise_or() ) );
	}

	return node;
}


Node *Parser::parse_bitwise_or()
{
	Node *node = parse_bitwise_xor();

	while( scanner.current().type == TokenType_BitOr )
	{
		scanner.next();
		node = ast.add( NodeExpressionBinary( ExpressionBinaryType_BitOr, node,
			parse_bitwise_xor() ) );
	}

	return node;
}


Node *Parser::parse_bitwise_xor()
{
	Node *node = parse_bitwise_and();

	while( scanner.current().type == TokenType_BitXor )
	{
		scanner.next();
		node = ast.add( NodeExpressionBinary( ExpressionBinaryType_BitXor, node,
			parse_bitwise_and() ) );
	}

	return node;
}


Node *Parser::parse_bitwise_and()
{
	Node *node = parse_equality();

	while( scanner.current().type == TokenType_BitAnd )
	{
		scanner.next();
		node = ast.add( NodeExpressionBinary( ExpressionBinaryType_BitAnd, node,
			parse_equality() ) );
	}

	return node;
}


Node *Parser::parse_equality()
{
	Node *node = parse_comparison();

	while( scanner.current().type == TokenType_Equals ||
		scanner.current().type == TokenType_NotEquals )
	{
		TokenType type = scanner.current().type;
		scanner.next();

		node = ast.add( NodeExpressionBinary(
			type == TokenType_Equals ? ExpressionBinaryType_Equals : ExpressionBinaryType_NotEquals,
			node, parse_comparison() ) );
	}

	return node;
}


Node *Parser::parse_comparison()
{
	Node *node = parse_bitwise_shift();

	while( scanner.current().type == TokenType_GreaterThan ||
		scanner.current().type == TokenType_GreaterThanEquals ||
		scanner.current().type == TokenType_LessThan ||
		scanner.current().type == TokenType_LessThanEquals )
	{
		TokenType tokenType = scanner.current().type;
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

		node = ast.add( NodeExpressionBinary( binaryExpressionType, node, parse_bitwise_shift() ) );
	}

	return node;
}


Node *Parser::parse_bitwise_shift()
{
	Node *node = parse_add_sub();

	while( scanner.current().type == TokenType_BitShiftLeft ||
		scanner.current().type == TokenType_BitShiftRight )
	{
		TokenType type = scanner.current().type;
		scanner.next();

		node = ast.add( NodeExpressionBinary(
			type == TokenType_BitShiftLeft ? ExpressionBinaryType_BitShiftLeft : ExpressionBinaryType_BitShiftRight,
			node, parse_add_sub() ) );
	}

	return node;
}


Node *Parser::parse_add_sub()
{
	Node *node = parse_mul_div_mod();

	while( scanner.current().type == TokenType_Plus ||
		scanner.current().type == TokenType_Minus )
	{
		TokenType type = scanner.current().type;
		scanner.next();

		node = ast.add( NodeExpressionBinary(
			type == TokenType_Plus ? ExpressionBinaryType_Add : ExpressionBinaryType_Sub,
			node, parse_mul_div_mod() ) );
	}

	return node;
}


Node *Parser::parse_mul_div_mod()
{
	Node *node = parse_prefix_operators();

	while( scanner.current().type == TokenType_Star ||
		scanner.current().type == TokenType_Slash ||
		scanner.current().type == TokenType_Mod )
	{
		TokenType tokenType = scanner.current().type;
		scanner.next();

		ExpressionBinaryType binaryExpressionType;
		switch( tokenType )
		{
			default:
			case TokenType_Star: binaryExpressionType = ExpressionBinaryType_Mul; break;
			case TokenType_Slash: binaryExpressionType = ExpressionBinaryType_Div; break;
			case TokenType_Mod: binaryExpressionType = ExpressionBinaryType_Mod; break;
		}

		node = ast.add( NodeExpressionBinary( binaryExpressionType, node, parse_prefix_operators() ) );
	}

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

	for (;;)
	{
		if( scanner.current().type == TokenType_Dot )
		{
			// Consume '.'
			Token token = scanner.next();
			ErrorIf( token.type != TokenType_Identifier,
				"RHS for '.' operator must be an identifier");

			// Find LHS type (if possible)
			TypeID typeID = node_type( node );
			if( typeID != USIZE_MAX )
			{
				Type &type = types[typeID];

				// Custom Type: check if RHS token matches a LHS structure member variable
				if( typeID >= PRIMITIVE_COUNT )
				{
					// Struct/UDT member
					VariableID first = type.memberFirst;
					VariableID last = first + type.memberCount;
					bool found = false;

					for( VariableID i = first; i < last; i++ )
					{
						Variable &variable = variables[i];

						// Variable name comparison
						if( variable.name.length == token.name.length &&
							strncmp( variable.name.data, token.name.data, token.name.length ) == 0 )
						{
							scope.add( i );
							Node *expr = parse_dot_operator();
							scope.remove( scope.size() - 1 );
							node = ast.add( NodeExpressionBinary( ExpressionBinaryType_Dot, node, expr ) );
							found = true;
							break;
						}
					}

					// If reached, this is an error
					if( !found )
					{
						Error( "'%.*s' is not a member of LHS type '%.*s'",
							token.name.length, token.name.data, type.name.length, type.name.data);
					}
				}
				else
				{
					// Built-in type: check swizzle
					if( swizzleMap.contains( token.name ) )
					{
						swizzling = true;
						Node *expr = parse_fundamental();
						swizzling = false;
						node = ast.add( NodeExpressionBinary( ExpressionBinaryType_Dot, node, expr ) );
					}
					else
					{
						Error( "invalid swizzle on built-in type '%.*s'", type.name.length, type.name.data );
					}
				}
			}
			else
			{
				scanner.back();
				Error( "invalid LHS for '.' operator" );
			}

			continue;
		}

		break; // No more dots
	}

	return node;
}


Node *Parser::parse_subscript_operator()
{
	Node *node = parse_fundamental();

	while( scanner.current().type == TokenType_LBrack )
    {
		Token token = scanner.next();
		Node *expr = parse_expression();

		token = scanner.current();
		ErrorIf( token.type != TokenType_RBrack, "Expected ']' after array indexing" );

		scanner.next();
		node = ast.add( NodeExpressionBinary( ExpressionBinaryType_Subscript, node, expr ) );
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
			else if( svSemanticMap.contains( token.name ) )
			{
				scanner.next();
				return ast.add( NodeSVSemantic( svSemanticMap.get( token.name ) ) );
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
			Error( "unexpected symbol (token: %d)", token.type );
		}
		break;
	}

	// Unreachable
	Error( "unreacheable" );
	return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static String line_string( const Scanner &scanner, u32 line, u32 &linePosition )
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
	Print( PrintColor_Red, "\n\nSHADER COMPILE ERROR:\n\n" );

	// Print Message
	va_list args;
	va_start( args, message );
	Print( PrintColor_White, "    " );
	Debug::print_formatted_variadic_color( true, PrintColor_White, message, args );
	va_end( args );

	// Print File
	Print( PrintColor_Red, "\n    %s:%u\n", path, scanner.current().line );

	// Print Line
	u32 linePosition = 0;
	String errorLine = line_string( scanner, scanner.current().line, linePosition );
	u32 spacePosition = scanner.current().start - linePosition;
	String spaces = "";
	for( usize i = 0; i < spacePosition; i++ ) { spaces += ( errorLine[i] == '\t' ?  "" : "~" ); }

	Print( PrintColor_Red, "\n" );
	Print( PrintColor_Red, "Line %u:\n", scanner.current().line );
	Print( PrintColor_Red, TAB "%s\n", errorLine.replace( "\t", "" ).cstr() );
	Print( PrintColor_Red, "~~~~%s^\n", spaces.cstr() );

	// Exit
	Debug::exit( 1 );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}