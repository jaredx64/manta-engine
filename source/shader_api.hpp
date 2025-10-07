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
//     6. build.exe packs the .glsl/.hlsl/.metal shaders into the project binary
//

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Compiler Supplied Preprocessor Defines:

#define SHADER_HLSL
#define SHADER_GLSL
#define SHADER_METAL

#define SHADER_D3D11
#define SHADER_D3D12
#define SHADER_OPENGL
#define SHADER_VULKAN
#define SHADER_METAL

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

template <typename T> T buffer( T buffer, uint index );

// Render Target Slot
#define target(slot)

// Options: POSITION, TEXCOORD, NORMAL, DEPTH, COLOR
#define semantic(s)
	#define POSITION
	#define TEXCOORD
	#define NORMAL
	#define DEPTH
	#define COLOR
	#define BINORMAL
	#define TANGENT
	#define INSTANCE
	#define TARGET
	#define COVERAGE
	#define IS_FRONT_FACE
	#define CLIP_DISTANCE
	#define CULL_DISTANCE
	#define INSTANCE_ID
	#define VERTEX_ID
	#define PRIMITIVE_ID
	#define RENDERTARGET_INDEX
	#define VIEWPORT_INDEX
	#define SAMPLE_ID
	#define GLOBAL_INVOCATION_ID
	#define LOCAL_INVOCATION_ID
	#define LOCAL_INVOCATION_INDEX
	#define WORKGROUP_ID

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

#define instance_input struct
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
#define shared

#define shared_struct struct

// HLSL:  cbuffer buff : register( bN ) { ... };
// GLSL:  layout( std140 ) uniform buff { ... };
// Metal: struct Uniforms { ... }; constant Uniforms &buff [[buffer(N)]]
#define uniform_buffer(slot) struct

// HLSL:  StructuredBuffer<T> buff : register(tN);
// GLSL:  layout(std430, binding = N) readonly buffer buff { T data[]; }
// Metal: const device T *buff [[buffer(N)]]
#define constant_buffer(count, slot) struct

// HLSL:  RWStructuredBuffer<T> buff : register(uN);
// GLSL:  layout(std430, binding = N) buffer buff { T data[]; };
// Metal: device T *buff [[buffer(N)]]
#define mutable_buffer(count, slot) struct

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

#define vector_type_4( type )                        \
	struct type##4 {                                 \
		type##4() = default;                         \
		type##4( type v );                           \
		type##4( type x, type y, type z, type w );   \
		type##4( type##3 xyz, type w );              \
		type##4( type x, type##3 yzw );              \
		type##4( type##2 xy, type z, type w );       \
		type##4( type x, type##2 yz, type w );       \
		type##4( type x, type w, type##2 zw );       \
		type##4( type##2 xy, type##2 zw );           \
		operator_overloads( type##4 );               \
		type &operator[]( int index );               \
		union {                                      \
			type x, y, z, w;                         \
			type r, g, b, a;                         \
			type##2 xy;                              \
			type##2 rg;                              \
			type##2 yz;                              \
			type##2 gb;                              \
			type##2 zw;                              \
			type##2 ba;                              \
			type##3 xyz;                             \
			type##3 rgb;                             \
			type##3 yzw;                             \
			type##3 gba;                             \
		};                                           \
		bool operator==( const type##4 &other );     \
		bool operator>=( const type##4 &other );     \
		bool operator>( const type##4 &other );      \
		bool operator<=( const type##4 &other );     \
		bool operator<( const type##4 &other );      \
		type##4 &operator*=( const type##4 &other ); \
		type##4 &operator/=( const type##4 &other ); \
		type##4 &operator+=( const type##4 &other ); \
		type##4 &operator-=( const type##4 &other ); \
		type##4 operator*( const type##4 &other );   \
		type##4 operator/( const type##4 &other );   \
		type##4 operator+( const type##4 &other );   \
		type##4 operator-( const type##4 &other );   \
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
#define texture1D(slot, type) Texture1D

struct Texture1DArray { };
#define texture1DArray(slot, type) Texture1DArray

struct Texture2D { };
#define texture2D(slot, type) Texture2D

struct Texture2DArray { };
#define texture2DArray(slot, type) Texture2DArray

struct Texture3D { };
#define texture3D(slot, type) Texture3D

struct TextureCube { };
#define textureCube(slot, type) TextureCube

struct TextureCubeArray { };
#define textureCubeArray(slot, type) TextureCubeArray

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Trigonometric Functions

template <typename T> auto sin( T x );
template <typename T> auto cos( T x );
template <typename T> auto tan( T x );
template <typename T> auto asin( T x );
template <typename T> auto acos( T x );
template <typename T> auto atan( T x );
template <typename T, typename U> auto atan2( T y, U x );
template <typename T> auto sinh( T x );
template <typename T> auto cosh( T x );
template <typename T> auto tanh( T x );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Exponential and Logarithmic Functions

template <typename T> auto exp( T x );
template <typename T> auto exp2( T x );
template <typename T> auto log( T x );
template <typename T> auto log2( T x );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Conversion Functions

template <typename T> auto degrees( T radians );
template <typename T> auto radians( T degrees );
template <typename T> auto round( T x );
template <typename T> auto trunc( T x );
template <typename T> auto ceil( T x );
template <typename T> auto floor( T x );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Arithmetic Functions

template <typename T> auto abs( T x );
template <typename T, typename U> auto pow( T x, U y );
template <typename T> auto sqrt( T x );
template <typename T> auto rsqrt( T x );
template <typename T, typename U, typename V> T clamp( T x, U min, V max );
template <typename T, typename U> auto max( T x, U y );
template <typename T, typename U> auto min( T x, U y );
template <typename T, typename U> auto mod( T x, U y );
template <typename T> auto frac( T x );
template <typename T, typename U> auto ldexp( T x, U exp );
template <typename T, typename U, typename V> auto fma( T a, U b, V c );
template <typename T> auto sign( T x );
template <typename T> auto saturate( T x );
template <typename T> auto ddx( T x );
template <typename T> auto ddy( T y );
template <typename T> auto ddx_coarse( T x );
template <typename T> auto ddx_fine( T x );
template <typename T> auto ddy_coarse( T y );
template <typename T> auto ddy_fine( T y );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Vector and Matrix Functions

template <typename T, typename U> U mul( T x, U y );
template <typename T> float length( T vector );
template <typename T> T inverse( T matrix );
template <typename T, typename U> float distance( T point1, U point2 );
template <typename T, typename U> auto dot( T vector1, U vector2 );
template <typename T, typename U> auto cross( T vector1, U vector2 );
template <typename T> auto normalize( T x );
template <typename T, typename U> auto reflect( T incident, U normal );
template <typename T, typename U, typename V> auto refract( T incident, U normal, V /*float*/ eta );
template <typename T, typename U, typename V> auto faceforward( T normal, U incident, V ng );
template <typename T> auto transpose( T matrix );
template <typename T> auto determinant( T matrix );
template <typename T, typename U, typename V> auto lerp( T x, U y, V s );
template <typename T> auto step( T threshold, T x );
template <typename T, typename U, typename V> auto smoothstep( T min, U max, V x );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Bitwise Operations

template <typename T> auto bit_count( T x );
template <typename T> auto bit_firsthigh( T x );
template <typename T> auto bit_firstlow( T x );
template <typename T> auto bit_reverse( T x );

template <typename T> int float_to_int_bits( T x );
template <typename T> uint float_to_uint_bits( T x );
template <typename T> float int_to_float_bits( T x );
template <typename T> float uint_to_float_bits( T x );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Texture Sampling Functions

float4 texture_sample_1d( Texture1D texture, float x );
float4 texture_sample_1d_array( Texture1DArray texture, float x );
float4 texture_sample_1d_level( Texture1DArray texture, float x, int lod );
float4 texture_index_1d( Texture2D texture, int x, int width, int lod );

float4 texture_sample_2d( Texture2D texture, float2 uv );
float4 texture_sample_2d_array( Texture2DArray texture, float2 uv );
float4 texture_sample_2d_level( Texture2D texture, float2 uv, int lod );
float4 texture_index_2d( Texture2D texture, int x, int y, int width, int height, int lod );

float4 texture_sample_3d( Texture2D texture, float2 uvw );
float4 texture_sample_3d_array( Texture2DArray texture, float2 uvw );
float4 texture_sample_3d_level( Texture2D texture, float2 uvw, int lod );
float4 texture_index_3d( Texture2D texture, int x, int y, int z, int width, int height, int depth, int lod );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Depth

float depth_unproject( float4 position );
float depth_unproject_zw( float depthZ, float depthW );
float depth_normalize( float depth, float near, float far );
float depth_linearize( float depth, float near, float far );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Built-in System-Value Semantics

uint3 SV_DispatchThreadID; // gl_GlobalInvocationID
uint3 SV_GroupID; // gl_WorkGroupID
uint3 SV_GroupThreadID; // gl_LocalInvocationID
uint SV_GroupIndex; // gl_LocalInvocationIndex
uint SV_VertexID; // gl_VertexID
uint SV_InstanceID; // gl_InstanceID
bool SV_IsFrontFace; // gl_FrontFacing
uint SV_SampleIndex; // gl_SampleID
uint SV_PrimitiveID; // gl_PrimitiveID;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Stages

#define vertex_main vertex_main
#define fragment_main fragment_main
#define compute_main(x,y,z) compute_main

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Runtime Compilation Preprocessor Macros

#define DEFINE( x )
#define IF( x )
#define IF_DEFINED( x )
#define IF_UNDEFINED( x )
#define IF_USING( x )
#define ELSE_IF( x )
#define ELSE
#define ENDIF

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////