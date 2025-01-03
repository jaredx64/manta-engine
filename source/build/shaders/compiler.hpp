#pragma once

#include <core/memory.hpp>
#include <core/list.hpp>
#include <core/string.hpp>

#include <build/filesystem.hpp>


extern void compile_shader( struct Shader &shader, const char *path );


namespace ShaderCompiler
{
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define SHADER_MAX_BUFFER_SLOTS  ( 255 )
#define SHADER_MAX_TEXTURE_SLOTS ( 255 )
#define SHADER_MAX_TARGET_SLOTS  ( 8 )

#define SHADER_OUTPUT_PREFIX_IDENTIFIERS ( true )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum_type( TokenType, int )
{
	// Sentinel
	TokenType_Error = -2,
	TokenType_EndOfFile = -1,
	TokenType_Null = 0,

	// Single Character
	TokenType_LParen = 0,          // (
	TokenType_RParen,              // )
	TokenType_LCurly,              // {
	TokenType_RCurly,              // }
	TokenType_LBrack,              // [
	TokenType_RBrack,              // ]
	TokenType_Dot,                 // .
	TokenType_Comma,               // ,
	TokenType_Colon,               // :
	TokenType_Semicolon,           // ;
	TokenType_BitNot,              // ~
	TokenType_Question,            // ?

	// Double Character
	TokenType_Assign,              // =
	TokenType_Equals,              // ==
	TokenType_Bang,                // !
	TokenType_NotEquals,           // !=
	TokenType_Plus,                // +
	TokenType_PlusAssign,          // +=
	TokenType_PlusPlus,            // ++
	TokenType_Minus,               // -
	TokenType_MinusAssign,         // -=
	TokenType_MinusMinus,          // --
	TokenType_Star,                // *
	TokenType_StarAssign,          // *=
	TokenType_Slash,               // /
	TokenType_SlashAssign,         // /=
	TokenType_Mod,                 // %
	TokenType_ModAssign,           // %=
	TokenType_BitXor,              // ^
	TokenType_BitXorAssign,        // ^=
	TokenType_BitOr,               // |
	TokenType_BitOrAssign,         // |=
	TokenType_Or,                  // ||
	TokenType_BitAnd,              // &
	TokenType_BitAndAssign,        // &=
	TokenType_And,                 // &&

	// Triple Character
	TokenType_LessThan,            // <
	TokenType_LessThanEquals,      // <=
	TokenType_BitShiftLeft,        // <<
	TokenType_BitShiftLeftAssign,  // <<=
	TokenType_GreaterThan,         // >
	TokenType_GreaterThanEquals,   // >=
	TokenType_BitShiftRight,       // >>
	TokenType_BitShiftRightAssign, // >>=

	// Literals
	TokenType_Identifier,          //
	TokenType_Number,              // 64-bit IEEE-754 Double
	TokenType_Integer,             // 64-bit Integer

	Primitive_Struct,
	Primitive_CBuffer,
	Primitive_VertexInput,
	Primitive_VertexOutput,
	Primitive_FragmentInput,
	Primitive_FragmentOutput,
	Primitive_ComputeInput,
	Primitive_ComputeOutput,

	// Keywords
	TOKENTYPE_KEYWORD_FIRST,
	TokenType_In = TOKENTYPE_KEYWORD_FIRST,
	TokenType_Out,
	TokenType_InOut,
	TokenType_True,
	TokenType_False,
	TokenType_Const,
	TokenType_Return,
	TokenType_Break,
	TokenType_Switch,
	TokenType_Case,
	TokenType_Default,
	TokenType_Discard,
	TokenType_If,
	TokenType_Else,
	TokenType_While,
	TokenType_Do,
	TokenType_For,
	TokenType_Struct,
	TokenType_CBuffer,
	TokenType_VertexInput,
	TokenType_VertexOutput,
	TokenType_FragmentInput,
	TokenType_FragmentOutput,
	TokenType_ComputeInput,
	TokenType_ComputeOutput,
	TokenType_Texture1D,
	TokenType_Texture1DArray,
	TokenType_Texture2D,
	TokenType_Texture2DArray,
	TokenType_Texture3D,
	TokenType_TextureCube,
	TokenType_TextureCubeArray,
	TokenType_Target,
	TokenType_Semantic,
	TokenType_POSITION,
	TokenType_TEXCOORD,
	TokenType_NORMAL,
	TokenType_DEPTH,
	TokenType_COLOR,
	TokenType_InputFormat,
	TokenType_UNORM8,
	TokenType_UNORM16,
	TokenType_UNORM32,
	TokenType_SNORM8,
	TokenType_SNORM16,
	TokenType_SNORM32,
	TokenType_UINT8,
	TokenType_UINT16,
	TokenType_UINT32,
	TokenType_SINT8,
	TokenType_SINT16,
	TokenType_SINT32,
	TokenType_FLOAT16,
	TokenType_FLOAT32,
	TOKENTYPE_KEYWORD_LAST,
	TOKENTYPE_KEYWORD_COUNT = TOKENTYPE_KEYWORD_LAST - TOKENTYPE_KEYWORD_FIRST,
};


enum_type( Primitive, u32 )
{
	Primitive_Void,
	Primitive_Bool,
	Primitive_Bool2,
	Primitive_Bool3,
	Primitive_Bool4,
	Primitive_Int,
	Primitive_Int2,
	Primitive_Int3,
	Primitive_Int4,
	Primitive_UInt,
	Primitive_UInt2,
	Primitive_UInt3,
	Primitive_UInt4,
	Primitive_Float,
	Primitive_Float2,
	Primitive_Float3,
	Primitive_Float4,
	Primitive_Float2x2,
	Primitive_Float3x3,
	Primitive_Float4x4,
	Primitive_Double,
	Primitive_Double2,
	Primitive_Double3,
	Primitive_Double4,
	Primitive_Double2x2,
	Primitive_Double3x3,
	Primitive_Double4x4,
	Primitive_Texture1D,
	Primitive_Texture1DArray,
	Primitive_Texture2D,
	Primitive_Texture2DArray,
	Primitive_Texture3D,
	Primitive_TextureCube,
	Primitive_TextureCubeArray,
	PRIMITIVE_COUNT,
};

extern const char *Primitives[PRIMITIVE_COUNT];


enum_type( Intrinsic, u32 )
{
	// Trigonometric Functions
	Intrinsic_Cos,
	Intrinsic_Sin,
	Intrinsic_Tan,
	Intrinsic_Sinh,
	Intrinsic_Cosh,
	Intrinsic_Tanh,
	Intrinsic_ASin,
	Intrinsic_ACos,
	Intrinsic_ATan,
	Intrinsic_ATan2,

	// Exponential and Logarithmic Functions
	Intrinsic_Exp,
	Intrinsic_Exp2,
	Intrinsic_Log,
	Intrinsic_Log2,

	// Conversion Functions
	Intrinsic_Degrees,
	Intrinsic_Radians,
	Intrinsic_Round,
	Intrinsic_Trunc,
	Intrinsic_Ceil,
	Intrinsic_Floor,

	// Arithmetic Functions
	Intrinsic_Abs,
	Intrinsic_Pow,
	Intrinsic_Sqrt,
	Intrinsic_RSqrt,
	Intrinsic_Clamp,
	Intrinsic_Max,
	Intrinsic_Min,
	Intrinsic_Mod,
	Intrinsic_Frac,
	Intrinsic_Ldexp,
	Intrinsic_Fma,
	Intrinsic_Sign,
	Intrinsic_Saturate,
	Intrinsic_DDx,
	Intrinsic_DDy,
	Intrinsic_DDxCoarse,
	Intrinsic_DDxFine,
	Intrinsic_DDyCoarse,
	Intrinsic_DDyFine,

	// Vector and Matrix Functions
	Intrinsic_Mul,
	Intrinsic_Length,
	Intrinsic_Distance,
	Intrinsic_Dot,
	Intrinsic_Cross,
	Intrinsic_Normalize,
	Intrinsic_Reflect,
	Intrinsic_Refract,
	Intrinsic_Faceforward,
	Intrinsic_Transpose,
	Intrinsic_Determinant,
	Intrinsic_Lerp,
	Intrinsic_Step,
	Intrinsic_Smoothstep,

	// Bitwise Operations
	Intrinsic_BitCount,
	Intrinsic_BitFirstHigh,
	Intrinsic_BitFirstLow,
	Intrinsic_BitReverse,
	Intrinsic_AtomicAdd,
	Intrinsic_AtomicCompareExchange,
	Intrinsic_AtomicExchange,
	Intrinsic_AtomicMax,
	Intrinsic_AtomicMin,
	Intrinsic_AtomicAnd,
	Intrinsic_AtomicOr,
	Intrinsic_AtomicXor,

	// Texture Sampling Functions
	Intrinsic_SampleTexture1D,
	Intrinsic_SampleTexture1DArray,
	Intrinsic_SampleTexture2D,
	Intrinsic_SampleTexture2DArray,
	Intrinsic_SampleTexture2DLevel,
	Intrinsic_SampleTexture3D,
	Intrinsic_SampleTextureCube,
	Intrinsic_SampleTextureCubeArray,

	INTRINSIC_COUNT,
};

extern const char *Intrinsics[];


enum_type( StructType, u32 )
{
	StructType_Struct,
	StructType_CBuffer,
	StructType_VertexInput,
	StructType_VertexOutput,
	StructType_FragmentInput,
	StructType_FragmentOutput,
	StructType_ComputeInput,
	StructType_ComputeOutput,
	STRUCTTYPE_COUNT,
};

extern const char *StructTypeNames[];


enum_type( SemanticType, u32 )
{
	SemanticType_POSITION,  // TokenType_POSITION
	SemanticType_TEXCOORD,  // TokenType_TEXCOORD
	SemanticType_NORMAL,    // TokenType_NORMAL
	SemanticType_DEPTH,     // TokenType_DEPTH
	SemanticType_COLOR,     // TokenType_COLOR
	SEMANTICTYPE_COUNT,
};


enum_type( InputFormat, u32 )
{
	InputFormat_UNORM8,
	InputFormat_UNORM16,
	InputFormat_UNORM32,
	InputFormat_SNORM8,
	InputFormat_SNORM16,
	InputFormat_SNORM32,
	InputFormat_UINT8,
	InputFormat_UINT16,
	InputFormat_UINT32,
	InputFormat_SINT8,
	InputFormat_SINT16,
	InputFormat_SINT32,
	InputFormat_FLOAT16,
	InputFormat_FLOAT32,

	PACKASTYPE_COUNT,
};


enum_type( TextureType, u32 )
{
	TextureType_Texture1D,
	TextureType_Texture1DArray,
	TextureType_Texture2D,
	TextureType_Texture2DArray,
	TextureType_Texture3D,
	TextureType_TextureCube,
	TextureType_TextureCubeArray,
	TEXTURETYPE_COUNT,
};

extern const char *TextureTypeNames[];


struct Keyword
{
	const char *key;
	TokenType token;
};

extern const Keyword KeywordNames[];


extern const char *SwizzleTypeNames[];

using SwizzleID = u32;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using TypeID = usize;
using StructID = usize;
using VariableID = usize;
using TextureID = usize;
using FunctionID = usize;


struct Construct
{
	bool seen = false;
};


struct Type : public Construct
{
	Type() = default;
	StringView name { };
	VariableID memberFirst = 0;
	VariableID memberCount = 0;
	TokenType tokenType = TokenType_Null;
	bool builtin = false;
	bool global = false;
	bool pipelineIntermediate = false;
};


struct Struct : public Construct
{
	TypeID typeID;
	int slot = 0;
};


struct Variable : public Construct
{
	Variable() = default;
	StringView name { };
	TypeID typeID = USIZE_MAX;
	InputFormat format = InputFormat_UNORM8;
	SemanticType semantic = SemanticType_TEXCOORD;
	int arrayLengthX = 0;
	int arrayLengthY = 0;
	int slot = -1;
	bool constant = false;
	bool swizzle = false;
	bool in = false;
	bool out = false;
};


struct Texture : public Construct
{
	VariableID variableID;
	int slot = 0;
};


struct Function : public Construct
{
	Function() = default;
	StringView name { };
	bool builtin = false;
	TypeID typeID = 0;
	VariableID parameterFirst = 0;
	VariableID parameterCount = 0;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Node;


struct NodeBuffer
{
	NodeBuffer() { init(); }
	~NodeBuffer() { free(); }

	void init();
	void free();
	void grow();

	template <typename T>
	Node *add( const T node )
	{
		// Exceeded current memory block size?
		if( current + sizeof( T ) >= capacity ) { grow(); }

		// Write Node
		byte *ptr = data + current;
		memory_copy( ptr, &node, sizeof( T ) );
		current += sizeof( T );

		// Success
		return reinterpret_cast<Node *>( ptr );
	}

	List<byte *> pages;
	byte *data = nullptr;
	usize current = 0;
	usize capacity = 0;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum_type( NodeType, u32 )
{
	NodeType_Statement,
	NodeType_ExpressionListNode,
	NodeType_ExpressionUnary,
	NodeType_ExpressionBinary,
	NodeType_ExpressionTernary,
	NodeType_FunctionDeclaration,
	NodeType_FunctionCall,
	NodeType_Cast,
	NodeType_VariableDeclaration,
	NodeType_Variable,
	NodeType_Swizzle,
	NodeType_Group,
	NodeType_Integer,
	NodeType_Number,
	NodeType_Boolean,
	NodeType_Struct,
	NodeType_Texture,
};


enum_type( StatementType, u32 )
{
	StatementType_Block,
	StatementType_Expression,
	StatementType_If,
	StatementType_Else,
	StatementType_While,
	StatementType_DoWhile,
	StatementType_For,
	StatementType_Switch,
	StatementType_Case,
	StatementType_Default,
	StatementType_Return,
	StatementType_Break,
	StatementType_Discard,
};


enum_type( ExpressionUnaryType, u32 )
{
	ExpressionUnaryType_Plus,
	ExpressionUnaryType_PreIncrement,
	ExpressionUnaryType_PostIncrement,
	ExpressionUnaryType_Minus,
	ExpressionUnaryType_PreDecrement,
	ExpressionUnaryType_PostDecrement,
	ExpressionUnaryType_Not,
	ExpressionUnaryType_BitNot,
};


enum_type( ExpressionBinaryType, u32 )
{
	ExpressionBinaryType_Dot,
	ExpressionBinaryType_Subscript,
	ExpressionBinaryType_Assign,
	ExpressionBinaryType_Add,
	ExpressionBinaryType_AddAssign,
	ExpressionBinaryType_Sub,
	ExpressionBinaryType_SubAssign,
	ExpressionBinaryType_Mul,
	ExpressionBinaryType_MulAssign,
	ExpressionBinaryType_Div,
	ExpressionBinaryType_DivAssign,
	ExpressionBinaryType_Mod,
	ExpressionBinaryType_ModAssign,
	ExpressionBinaryType_BitAnd,
	ExpressionBinaryType_BitAndAssign,
	ExpressionBinaryType_BitOr,
	ExpressionBinaryType_BitOrAssign,
	ExpressionBinaryType_BitXor,
	ExpressionBinaryType_BitXorAssign,
	ExpressionBinaryType_BitShiftLeft,
	ExpressionBinaryType_BitShiftLeftAssign,
	ExpressionBinaryType_BitShiftRight,
	ExpressionBinaryType_BitShiftRightAssign,
	ExpressionBinaryType_Equals,
	ExpressionBinaryType_NotEquals,
	ExpressionBinaryType_And,
	ExpressionBinaryType_Or,
	ExpressionBinaryType_Greater,
	ExpressionBinaryType_GreaterEquals,
	ExpressionBinaryType_Less,
	ExpressionBinaryType_LessEquals,
	EXPRESSIONBINARYTYPE_COUNT,
};


enum_type( ExpressionTernaryType, u32 )
{
	ExpressionTernaryType_Conditional,
};


enum_type( FunctionType, u32 )
{
	FunctionType_Custom,
	FunctionType_Builtin,
	FunctionType_MainVertex,
	FunctionType_MainFragment,
	FunctionType_MainCompute,
	FUNCTIONTYPE_COUNT,
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Node
{
	NodeType nodeType;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct NodeStatement : public Node
{
	StatementType statementType;
};


struct NodeStatementBlock : public NodeStatement
{
	NodeStatementBlock( Node *expr ) : expr{ expr }
	{
		nodeType = NodeType_Statement;
		statementType = StatementType_Block;
	}

	Node *expr = nullptr;
	Node *next = nullptr;
};


struct NodeStatementExpression : public NodeStatement
{
	NodeStatementExpression( Node *expr ) : expr{ expr }
	{
		nodeType = NodeType_Statement;
		statementType = StatementType_Expression;
	}

	Node *expr = nullptr;
};



struct NodeStatementIf : public NodeStatement
{
	NodeStatementIf( Node *expr, Node *blockIf, Node *blockElse ) :
		expr{ expr }, blockIf{ blockIf }, blockElse{ blockElse }
	{
		nodeType = NodeType_Statement;
		statementType = StatementType_If;
	}

	Node *expr = nullptr;
	Node *blockIf = nullptr;
	Node *blockElse = nullptr;
};


struct NodeStatementElse : public NodeStatement
{
	NodeStatementElse( Node *block ) : block{ block }
	{
		nodeType = NodeType_Statement;
		statementType = StatementType_Else;
	}

	Node *block = nullptr;
};


struct NodeStatementWhile : public NodeStatement
{
	NodeStatementWhile( Node *expr, Node *block ) : expr{ expr }, block{ block }
	{
		nodeType = NodeType_Statement;
		statementType = StatementType_While;
	}

	Node *expr = nullptr;
	Node *block = nullptr;
};


struct NodeStatementDoWhile : public NodeStatement
{
	NodeStatementDoWhile( Node *expr, Node *block ) : expr{ expr }, block{ block }
	{
		nodeType = NodeType_Statement;
		statementType = StatementType_DoWhile;
	}

	Node *expr = nullptr;
	Node *block = nullptr;
};


struct NodeStatementFor : public NodeStatement
{
	NodeStatementFor( Node *expr1, Node *expr2, Node *expr3, Node *block ) :
		expr1{ expr1 }, expr2{ expr2 }, expr3{ expr3 }, block{ block }
	{
		nodeType = NodeType_Statement;
		statementType = StatementType_For;
	}

	Node *expr1 = nullptr;
	Node *expr2 = nullptr;
	Node *expr3 = nullptr;
	Node *block = nullptr;
};


struct NodeStatementSwitch : public NodeStatement
{
	NodeStatementSwitch( Node *expr, Node *block ) : expr{ expr }, block{ block }
	{
		nodeType = NodeType_Statement;
		statementType = StatementType_Switch;
	}

	Node *expr = nullptr;
	Node *block = nullptr;
};


struct NodeStatementCase : public NodeStatement
{
	NodeStatementCase( Node *expr, Node *block ) : expr{ expr }, block{ block }
	{
		nodeType = NodeType_Statement;
		statementType = StatementType_Case;
	}

	Node *expr = nullptr;
	Node *block = nullptr;
};


struct NodeStatementDefault : public NodeStatement
{
	NodeStatementDefault( Node *block ) : block{ block }
	{
		nodeType = NodeType_Statement;
		statementType = StatementType_Default;
	}

	Node *block = nullptr;
};


struct NodeStatementReturn : public NodeStatement
{
	NodeStatementReturn( Node *expr ) : expr{ expr }
	{
		nodeType = NodeType_Statement;
		statementType = StatementType_Return;
	}

	Node *expr = nullptr;
};


struct NodeStatementBreak : public NodeStatement
{
	NodeStatementBreak()
	{
		nodeType = NodeType_Statement;
		statementType = StatementType_Break;
	}
};


struct NodeStatementDiscard : public NodeStatement
{
	NodeStatementDiscard()
	{
		nodeType = NodeType_Statement;
		statementType = StatementType_Discard;
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct NodeExpressionUnary : public Node
{
	NodeExpressionUnary( ExpressionUnaryType exprType, Node *expr ) : exprType{ exprType }, expr{ expr }
	{
		nodeType = NodeType_ExpressionUnary;
	}

	ExpressionUnaryType exprType;
	Node *expr;
};


struct NodeExpressionList : public Node
{
	NodeExpressionList( Node *expr ) : expr{ expr }
	{
		nodeType = NodeType_ExpressionListNode;
	}

	Node *expr = nullptr;
	Node *next = nullptr; // traverse as singly-linked list
};


struct NodeExpressionBinary : public Node
{
	NodeExpressionBinary( ExpressionBinaryType exprType, Node *expr1, Node *expr2 ) :
		exprType{ exprType }, expr1{ expr1 }, expr2{ expr2 }
	{
		nodeType = NodeType_ExpressionBinary;
	}

	ExpressionBinaryType exprType;
	Node *expr1 = nullptr;
	Node *expr2 = nullptr;
};


struct NodeExpressionTernary : public Node
{
	NodeExpressionTernary( ExpressionTernaryType exprType, Node *expr1, Node *expr2, Node *expr3 ) :
		exprType{ exprType }, expr1{ expr1 }, expr2{ expr2 }, expr3{ expr3 }
	{
		nodeType = NodeType_ExpressionTernary;
	}

	ExpressionBinaryType exprType;
	Node *expr1 = nullptr;
	Node *expr2 = nullptr;
	Node *expr3 = nullptr;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct NodeFunctionDeclaration : public Node
{
	NodeFunctionDeclaration( FunctionType functionType, const FunctionID functionID, Node *block ) :
		functionType{ functionType }, functionID{ functionID }, block{ block }
	{
		nodeType = NodeType_FunctionDeclaration;
	}

	FunctionType functionType;
	FunctionID functionID;
	Node *block = nullptr;
};


struct NodeFunctionCall : public Node
{
	NodeFunctionCall( const FunctionID functionID, Node *param ) : functionID{ functionID }, param{ param }
	{
		nodeType = NodeType_FunctionCall;
	}

	FunctionID functionID;
	Node *param = nullptr;
};


struct NodeCast : public Node
{
	NodeCast( const TypeID typeID, Node *param ) : typeID{ typeID }, param{ param }
	{
		nodeType = NodeType_Cast;
	}

	TypeID typeID;
	Node *param = nullptr;
};


struct NodeVariableDeclaration : public Node
{
	NodeVariableDeclaration( const VariableID variableID, Node *assignment ) :
		variableID{ variableID }, assignment{ assignment }
	{
		nodeType = NodeType_VariableDeclaration;
	}

	VariableID variableID;
	Node *assignment = nullptr;
};


struct NodeGroup : public Node
{
	NodeGroup( Node *expr ) : expr{ expr }
	{
		nodeType = NodeType_Group;
	}

	Node *expr = nullptr;
};


struct NodeVariable : public Node
{
	NodeVariable( const VariableID variableID ) : variableID{ variableID }
	{
		nodeType = NodeType_Variable;
	}

	VariableID variableID;
};


struct NodeSwizzle : public Node
{
	NodeSwizzle( const SwizzleID swizzleID ) : swizzleID{ swizzleID }
	{
		nodeType = NodeType_Swizzle;
	}

	SwizzleID swizzleID;
};


struct NodeInteger : public Node
{
	NodeInteger( const u64 integer ) : integer{ integer }
	{
		nodeType = NodeType_Integer;
	}

	u64 integer;
};


struct NodeNumber : public Node
{
	NodeNumber( const double number ) : number{ number }
	{
		nodeType = NodeType_Number;
	}

	double number;
};


struct NodeBoolean : public Node
{
	NodeBoolean( const bool boolean ) : boolean{ boolean }
	{
		nodeType = NodeType_Boolean;
	}

	bool boolean;
};


struct NodeStruct : public Node
{
	NodeStruct( const StructType structType, const StructID structID ) :
		structType{ structType }, structID{ structID }
	{
		nodeType = NodeType_Struct;
	}

	StructType structType;
	StructID structID;
};


struct NodeTexture : public Node
{
	NodeTexture( const TextureType textureType, const TextureID textureID ) :
		textureType{ textureType }, textureID{ textureID }
	{
		nodeType = NodeType_Texture;
	}

	TextureType textureType;
	TextureID textureID;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}