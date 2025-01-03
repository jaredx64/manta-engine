////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//
// Shader API
//
// The purpose of this header is to provide syntax highlighting and autocomplete for .shader files without the
// need for a custom language server. In short, this file defines C++ macros, structs, and functions that
// let us to treat .shader files as C++ files.
//
// This header is NOT compiled!
//
// The shader compilation works as follows:
//     1. build.exe searches for '.shader' files
//     2. build.exe runs msvc/clang/gcc preprocessor on the '.shader' to resolve preprocessor directives
//     3. build.exe parses the preprocessed output and and builds an AST representation of the shader program
//     4. build.exe generates .glsl/.hlsl/.metal output code based on the AST (see: output/generated/shaders)
//     5. build.exe generates gfx.generated.hpp/cpp containing vertex layouts and shader metadata (for CPU engine use)
//     6. build.exe packs the .glsl/.hlsl/.metal shaders (compiled or source) into the project binary
//

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define operator_overloads( type )                    \
	template <typename T> type operator*( T other );  \
	template <typename T> type operator*=( T other ); \
	template <typename T> type operator/( T other );  \
	template <typename T> type operator/=( T other ); \
	template <typename T> type operator+( T other );  \
	template <typename T> type operator+=( T other ); \
	template <typename T> type operator-( T other );  \
	template <typename T> type operator-=( T other ); \
	template <typename T> bool operator==( T other ); \
	template <typename T> bool operator!=( T other );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Keywords

// Constant Buffer Slot
#define cbuffer(slot) struct

// Render Target Slot
#define target(slot)

// Options: POSITION, TEXCOORD, NORMAL, DEPTH, COLOR
#define semantic(s)
	#define POSITION
	#define TEXCOORD
	#define NORMAL
	#define DEPTH
	#define COLOR

// Options: UNORM8, UNORM16, UNORM32, SNORM8, SNORM16, SNORM32, UINT8, UINT16, UINT32, SINT8, SINT16, SINT32, FLOAT16, FLOAT32
#define format(f)
	#define UNORM8
	#define UNORM16
	#define UNORM32
	#define SNORM8
	#define SNORM16
	#define SNORM32
	#define UINT8
	#define UINT16
	#define UINT32
	#define SINT8
	#define SINT16
	#define SINT32
	#define FLOAT16
	#define FLOAT32

#define vertex_input struct
#define vertex_output struct
#define fragment_input struct
#define fragment_output struct
#define compute_input struct
#define compute_output struct

#define in
#define out
#define inout

#define discard

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Base Types

using uint = unsigned int;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Vector 2D

#define vector_type_2( type )          \
	struct type##2 {                   \
		type##2() = default;           \
		type##2( type v );             \
		type##2( type x, type y );     \
		operator_overloads( type##2 ); \
		type &operator[]( int index ); \
		union {                        \
			type x, y;                 \
			type r, g;                 \
		};                             \
	};

vector_type_2( float );
vector_type_2( int );
vector_type_2( uint );
vector_type_2( bool );
vector_type_2( double );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Vector 3D

#define vector_type_3( type )              \
	struct type##3 {                       \
		type##3() = default;               \
		type##3( type v );                 \
		type##3( type x, type y, type z ); \
		type##3( type##2 xy, type z );     \
		type##3( type x, type##2 yz );     \
		operator_overloads( type##3 );     \
		type &operator[]( int index );     \
		union {                            \
			type x, y, z;                  \
			type r, g, b;                  \
			type##2 xy;                    \
			type##2 rg;                    \
			type##2 yz;                    \
			type##2 gb;                    \
		};                                 \
	};

vector_type_3( float );
vector_type_3( int );
vector_type_3( uint );
vector_type_3( bool );
vector_type_3( double );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Vector 4D

#define vector_type_4( type )                      \
	struct type##4 {                               \
		type##4() = default;                       \
		type##4( type v );                         \
		type##4( type x, type y, type z, type w ); \
		type##4( type##3 xyz, type w );            \
		type##4( type x, type##3 yzw );            \
		type##4( type##2 xy, type z, type w );     \
		type##4( type x, type##2 yz, type w );     \
		type##4( type x, type w, type##2 zw );     \
		type##4( type##2 xy, type##2 zw );         \
		operator_overloads( type##4 );             \
		type &operator[]( int index );             \
		union {                                    \
			type x, y, z, w;                       \
			type r, g, b, a;                       \
			type##2 xy;                            \
			type##2 rg;                            \
			type##2 yz;                            \
			type##2 gb;                            \
			type##2 zw;                            \
			type##2 ba;                            \
			type##3 xyz;                           \
			type##3 rgb;                           \
			type##3 yzw;                           \
			type##3 gba;                           \
		};                                         \
	};

vector_type_4( float );
vector_type_4( int );
vector_type_4( uint );
vector_type_4( bool );
vector_type_4( double );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Matrix 2x2

struct float2x2
{
	float2x2() = default;
	float2x2( float x1y1, float x2y1,
	          float x1y2, float x2y2 );
	operator_overloads( float2x2 );
	float2 &operator[]( int index );
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Matrix 3x3

struct float3x3
{
	float3x3() = default;
	float3x3( float x1y1, float x2y1, float x3y1,
	          float x1y2, float x2y2, float x3y2,
	          float x1y3, float x2y3, float x3y3 );
	operator_overloads( float3x3 );
	float3 &operator[]( int index );
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Matrix 4x4

struct float4x4
{
	float4x4() = default;
	float4x4( float x1y1, float x2y1, float x3y1, float x4y1,
	          float x1y2, float x2y2, float x3y2, float x4y2,
			  float x1y3, float x2y3, float x3y3, float x4y3,
	          float x1y4, float x2y4, float x3y4, float x4y4 );
	operator_overloads( float4x4 );
	float4 &operator[]( int index );
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Texures

struct Texture1D { };
#define texture1D(slot) Texture1D

struct Texture1DArray { };
#define texture1DArray(slot) Texture1DArray

struct Texture2D { };
#define texture2D(slot) Texture2D

struct Texture2DArray { };
#define texture2DArray(slot) Texture2DArray

struct Texture3D { };
#define texture3D(slot) Texture3D

struct TextureCube { };
#define textureCube(slot) TextureCube

struct TextureCubeArray { };
#define textureCubeArray(slot) TextureCubeArray

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Trigonometric Functions

template <typename T> T sin( T x );
template <typename T> T cos( T x );
template <typename T> T tan( T x );
template <typename T> T asin( T x );
template <typename T> T acos( T x );
template <typename T> T atan( T x );
template <typename T> T atan2( T y, T x );
template <typename T> T sinh( T x );
template <typename T> T cosh( T x );
template <typename T> T tanh( T x );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Exponential and Logarithmic Functions

template <typename T> T exp( T x );
template <typename T> T exp2( T x );
template <typename T> T log( T x );
template <typename T> T log2( T x );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Conversion Functions

template <typename T> T degrees( T radians );
template <typename T> T radians( T degrees );
template <typename T> T round( T x );
template <typename T> T trunc( T x );
template <typename T> T ceil( T x );
template <typename T> T floor( T x );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Arithmetic Functions

template <typename T> T abs( T x );
template <typename T> T pow( T x, T y );
template <typename T> T sqrt( T x );
template <typename T> T rsqrt( T x );
template <typename T> T clamp( T x, T min, T max );
template <typename T> T max( T x, T y );
template <typename T> T min( T x, T y );
template <typename T> T mod( T x, T y );
template <typename T> T frac( T x );
template <typename A, typename B> A ldexp( A x, B exp );
template <typename T> T fma( T a, T b, T c );
template <typename T> T sign( T x );
template <typename T> T saturate( T x );
template <typename T> T ddx( T x );
template <typename T> T ddy( T y );
template <typename T> T ddx_coarse( T x );
template <typename T> T ddx_fine( T x );
template <typename T> T ddy_coarse( T y );
template <typename T> T ddy_fine( T y );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Vector and Matrix Functions

template <typename A, typename B> B mul( A x, B y );
template <typename T> float length( T vector );
template <typename T> float distance( T point1, T point2 );
template <typename T> auto dot( T vector1, T vector2 );
template <typename T> auto cross( T vector1, T vector2 );
template <typename T> auto normalize( T x );
template <typename T> auto reflect( T incident, T normal );
template <typename T> auto refract( T incident, T normal, float eta );
template <typename T> auto faceforward( T normal, T incident, T ng );
template <typename T> auto transpose( T matrix );
template <typename T> auto determinant( T matrix );
template <typename T> auto lerp( T x, T y, float s );
template <typename T> auto step( T threshold, T x );
template <typename T> auto smoothstep( T min, T max, T x );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Bitwise Operations

template <typename T> auto bit_count( T x );
template <typename T> auto bit_firsthigh( T x );
template <typename T> auto bit_firstlow( T x );
template <typename T> auto bit_reverse( T x );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Texture Sampling Functions

float4 sample_texture1D( Texture1D texture, float x );
float4 sample_texture1DArray( Texture1DArray texture, float x );

float4 sample_texture2D( Texture2D texture, float2 uv );
float4 sample_texture2DArray( Texture2DArray texture, float2 uv );
float4 sample_texture2DLevel( Texture2D texture, float2 uv );

float4 sample_texture3D( Texture3D texture, float3 xyz );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////