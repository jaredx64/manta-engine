#pragma once

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
//     2. build.exe preprocesses the '.shader' to resolve # directives
//     3. build.exe parses the preprocessed output and and builds an AST representation of the shader program
//     4. build.exe generates .glsl/.hlsl/.metal output code from AST (see: output/generated/shaders)
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

#define operator_overloads( type ) \
	type operator*( auto other );  \
	type operator*=( auto other ); \
	type operator/( auto other );  \
	type operator/=( auto other ); \
	type operator+( auto other );  \
	type operator+=( auto other ); \
	type operator-( auto other );  \
	type operator-=( auto other ); \
	bool operator==( auto other ); \
	bool operator!=( auto other );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Keywords

template <typename A> A buffer( A buffer, int index );

#define position_out
#define position_in

// Options: COLOR, DEPTH
#define target(slot, type)
	#define DEPTH
	#define COLOR

// Options: UNORM8, UNORM16, UNORM32, SNORM8, SNORM16, SNORM32, UINT8, UINT16, UINT32, SINT8, SINT16, SINT32, FLOAT16, FLOAT32
#define packed_as(f)
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

#define vector_type_2_alias( type )                                              \
	struct type##2;                                                              \
	struct _##type##2 {                                                          \
		_##type##2();                                                            \
		_##type##2( const type##2 &xy );                                         \
		_##type##2( const _##type##2 &xy );                                      \
		_##type##2( auto x, auto y );                                            \
		_##type##2 &operator*=( auto other );                                    \
		_##type##2 &operator*=( const type##2 &other );                          \
		_##type##2 &operator*=( const _##type##2 &other );                       \
		_##type##2 &operator/=( auto other );                                    \
		_##type##2 &operator/=( const type##2 &other );                          \
		_##type##2 &operator/=( const _##type##2 &other );                       \
		_##type##2 &operator+=( auto other );                                    \
		_##type##2 &operator+=( const type##2 &other );                          \
		_##type##2 &operator+=( const _##type##2 &other );                       \
		_##type##2 &operator-=( auto other );                                    \
		_##type##2 &operator-=( const type##2 &other );                          \
		_##type##2 &operator-=( const _##type##2 &other );                       \
		_##type##2 operator*( auto other );                                      \
		_##type##2 operator*( const type##2 &other );                            \
		_##type##2 operator*( const _##type##2 &other );                         \
		_##type##2 operator/( auto other );                                      \
		_##type##2 operator/( const type##2 &other );                            \
		_##type##2 operator/( const _##type##2 &other );                         \
		_##type##2 operator+( auto other );                                      \
		_##type##2 operator+( const type##2 &other );                            \
		_##type##2 operator+( const _##type##2 &other );                         \
		_##type##2 operator-( auto other );                                      \
		_##type##2 operator-( const type##2 &other );                            \
		_##type##2 operator-( const _##type##2 &other );                         \
		operator type##2() const;                                                \
		bool operator==( const type##2 &other );                                 \
		bool operator==( const _##type##2 &other );                              \
		bool operator>=( const type##2 &other );                                 \
		bool operator>=( const _##type##2 &other );                              \
		bool operator>( const type##2 &other );                                  \
		bool operator>( const _##type##2 &other );                               \
		bool operator<=( const type##2 &other );                                 \
		bool operator<=( const _##type##2 &other );                              \
		bool operator<( const type##2 &other );                                  \
		bool operator<( const _##type##2 &other );                               \
		type &operator[]( int index );                                           \
	};                                                                           \
	inline _##type##2 operator*( auto l, const _##type##2 &r ) { return r * l; } \
	inline _##type##2 operator/( auto l, const _##type##2 &r ) { return r / l; } \
	inline _##type##2 operator+( auto l, const _##type##2 &r ) { return r + l; } \
	inline _##type##2 operator-( auto l, const _##type##2 &r ) { return r - l; }

#define vector_type_2( type )                                                    \
	struct type##2 : _##type##2 {                                                \
		type##2();                                                               \
		type##2( const type##2 &xy );                                            \
		type##2( auto x, auto y );                                               \
		type##2 &operator*=( auto other );                                       \
		type##2 &operator*=( const type##2 &other );                             \
		type##2 &operator*=( const _##type##2 &other );                          \
		type##2 &operator/=( auto other );                                       \
		type##2 &operator/=( const type##2 &other );                             \
		type##2 &operator/=( const _##type##2 &other );                          \
		type##2 &operator+=( auto other );                                       \
		type##2 &operator+=( const type##2 &other );                             \
		type##2 &operator+=( const _##type##2 &other );                          \
		type##2 &operator-=( auto other );                                       \
		type##2 &operator-=( const type##2 &other );                             \
		type##2 &operator-=( const _##type##2 &other );                          \
		type##2 operator*( auto other );                                         \
		type##2 operator*( const type##2 &other );                               \
		type##2 operator*( const _##type##2 &other );                            \
		type##2 operator/( auto other );                                         \
		type##2 operator/( const type##2 &other );                               \
		type##2 operator/( const _##type##2 &other );                            \
		type##2 operator+( auto other );                                         \
		type##2 operator+( const type##2 &other );                               \
		type##2 operator+( const _##type##2 &other );                            \
		type##2 operator-( auto other );                                         \
		type##2 operator-( const type##2 &other );                               \
		type##2 operator-( const _##type##2 &other );                            \
		union {                                                                  \
			type x, y;                                                           \
			type r, g;                                                           \
			_##type##2 xx;   _##type##2 yx;   _##type##2 xy;   _##type##2 yy;    \
			_##type##2 rr;   _##type##2 gr;   _##type##2 rg;   _##type##2 gg;    \
			_##type##3 xxx;  _##type##3 yxx;  _##type##3 xyx;  _##type##3 yyx;   \
			_##type##3 xxy;  _##type##3 yxy;  _##type##3 xyy;  _##type##3 yyy;   \
			_##type##3 rrr;  _##type##3 grr;  _##type##3 rgr;  _##type##3 ggr;   \
			_##type##3 rrg;  _##type##3 grg;  _##type##3 rgg;  _##type##3 ggg;   \
			_##type##4 xxxx; _##type##4 yxxx; _##type##4 xyxx; _##type##4 yyxx;  \
			_##type##4 xxyx; _##type##4 yxyx; _##type##4 xyyx; _##type##4 yyyx;  \
			_##type##4 xxxy; _##type##4 yxxy; _##type##4 xyxy; _##type##4 yyxy;  \
			_##type##4 xxyy; _##type##4 yxyy; _##type##4 xyyy; _##type##4 yyyy;  \
			_##type##4 rrrr; _##type##4 grrr; _##type##4 rgrr; _##type##4 ggrr;  \
			_##type##4 rrgr; _##type##4 grgr; _##type##4 rggr; _##type##4 gggr;  \
			_##type##4 rrrg; _##type##4 grrg; _##type##4 rgrg; _##type##4 ggrg;  \
			_##type##4 rrgg; _##type##4 grgg; _##type##4 rggg; _##type##4 gggg;  \
		};                                                                       \
	};                                                                           \
	inline type##2 operator*( auto l, const type##2 &r ) { return r * l; }       \
	inline type##2 operator/( auto l, const type##2 &r ) { return r / l; }       \
	inline type##2 operator+( auto l, const type##2 &r ) { return r + l; }       \
	inline type##2 operator-( auto l, const type##2 &r ) { return r - l; }

#define vector_type_3_alias( type )                                              \
	struct type##3;                                                              \
	struct _##type##3 {                                                          \
		_##type##3();                                                            \
		_##type##3( const type##3 &xyz );                                        \
		_##type##3( const _##type##3 &xyz );                                     \
		_##type##3( type##2 xy, auto z );                                        \
		_##type##3( _##type##2 xy, auto z );                                     \
		_##type##3( auto x, type##2 yz );                                        \
		_##type##3( auto x, _##type##2 yz );                                     \
		_##type##3( auto x, auto y, auto z );                                    \
		_##type##3 &operator*=( auto other );                                    \
		_##type##3 &operator*=( const type##3 &other );                          \
		_##type##3 &operator*=( const _##type##3 &other );                       \
		_##type##3 &operator/=( auto other );                                    \
		_##type##3 &operator/=( const type##3 &other );                          \
		_##type##3 &operator/=( const _##type##3 &other );                       \
		_##type##3 &operator+=( auto other );                                    \
		_##type##3 &operator+=( const type##3 &other );                          \
		_##type##3 &operator+=( const _##type##3 &other );                       \
		_##type##3 &operator-=( auto other );                                    \
		_##type##3 &operator-=( const type##3 &other );                          \
		_##type##3 &operator-=( const _##type##3 &other );                       \
		_##type##3 operator*( auto other );                                      \
		_##type##3 operator*( const type##3 &other );                            \
		_##type##3 operator*( const _##type##3 &other );                         \
		_##type##3 operator/( auto other );                                      \
		_##type##3 operator/( const type##3 &other );                            \
		_##type##3 operator/( const _##type##3 &other );                         \
		_##type##3 operator+( auto other );                                      \
		_##type##3 operator+( const type##3 &other );                            \
		_##type##3 operator+( const _##type##3 &other );                         \
		_##type##3 operator-( auto other );                                      \
		_##type##3 operator-( const type##3 &other );                            \
		_##type##3 operator-( const _##type##3 &other );                         \
		operator type##3() const;                                                \
		bool operator==( const type##3 &other );                                 \
		bool operator==( const _##type##3 &other );                              \
		bool operator>=( const type##3 &other );                                 \
		bool operator>=( const _##type##3 &other );                              \
		bool operator>( const type##3 &other );                                  \
		bool operator>( const _##type##3 &other );                               \
		bool operator<=( const type##3 &other );                                 \
		bool operator<=( const _##type##3 &other );                              \
		bool operator<( const type##3 &other );                                  \
		bool operator<( const _##type##3 &other );                               \
		type &operator[]( int index );                                           \
	};                                                                           \
	inline _##type##3 operator*( auto l, const _##type##3 &r ) { return r * l; } \
	inline _##type##3 operator/( auto l, const _##type##3 &r ) { return r / l; } \
	inline _##type##3 operator+( auto l, const _##type##3 &r ) { return r + l; } \
	inline _##type##3 operator-( auto l, const _##type##3 &r ) { return r - l; }

#define vector_type_3( type )                                                    \
	struct type##3 : _##type##3 {                                                \
		type##3();                                                               \
		type##3( const type##3 &xyz );                                           \
		type##3( type##2 xy, auto z );                                           \
		type##3( _##type##2 xy, auto z );                                        \
		type##3( auto x, type##2 yz );                                           \
		type##3( auto x, _##type##2 yz );                                        \
		type##3( auto x, auto y, auto z );                                       \
		type##3 &operator*=( auto other );                                       \
		type##3 &operator*=( const type##3 &other );                             \
		type##3 &operator*=( const _##type##3 &other );                          \
		type##3 &operator/=( auto other );                                       \
		type##3 &operator/=( const type##3 &other );                             \
		type##3 &operator/=( const _##type##3 &other );                          \
		type##3 &operator+=( auto other );                                       \
		type##3 &operator+=( const type##3 &other );                             \
		type##3 &operator+=( const _##type##3 &other );                          \
		type##3 &operator-=( auto other );                                       \
		type##3 &operator-=( const type##3 &other );                             \
		type##3 &operator-=( const _##type##3 &other );                          \
		type##3 operator*( auto other );                                         \
		type##3 operator*( const type##3 &other );                               \
		type##3 operator*( const _##type##3 &other );                            \
		type##3 operator/( auto other );                                         \
		type##3 operator/( const type##3 &other );                               \
		type##3 operator/( const _##type##3 &other );                            \
		type##3 operator+( auto other );                                         \
		type##3 operator+( const type##3 &other );                               \
		type##3 operator+( const _##type##3 &other );                            \
		type##3 operator-( auto other );                                         \
		type##3 operator-( const type##3 &other );                               \
		type##3 operator-( const _##type##3 &other );                            \
		union {                                                                  \
			type x, y, z;                                                        \
			type r, g, b;                                                        \
			_##type##2 xx;   _##type##2 yx;   _##type##2 zx;   _##type##2 xy;    \
			_##type##2 yy;   _##type##2 zy;   _##type##2 xz;   _##type##2 yz;    \
			_##type##2 zz;   _##type##2 rr;   _##type##2 gr;   _##type##2 br;    \
			_##type##2 rg;   _##type##2 gg;   _##type##2 bg;   _##type##2 rb;    \
			_##type##2 gb;   _##type##2 bb;                                      \
			_##type##3 xxx;  _##type##3 yxx;  _##type##3 zxx;  _##type##3 xyx;   \
			_##type##3 yyx;  _##type##3 zyx;  _##type##3 xzx;  _##type##3 yzx;   \
			_##type##3 zzx;  _##type##3 xxy;  _##type##3 yxy;  _##type##3 zxy;   \
			_##type##3 xyy;  _##type##3 yyy;  _##type##3 zyy;  _##type##3 xzy;   \
			_##type##3 yzy;  _##type##3 zzy;  _##type##3 xxz;  _##type##3 yxz;   \
			_##type##3 zxz;  _##type##3 xyz;  _##type##3 yyz;  _##type##3 zyz;   \
			_##type##3 xzz;  _##type##3 yzz;  _##type##3 zzz;  _##type##3 rrr;   \
			_##type##3 grr;  _##type##3 brr;  _##type##3 rgr;  _##type##3 ggr;   \
			_##type##3 bgr;  _##type##3 rbr;  _##type##3 gbr;  _##type##3 bbr;   \
			_##type##3 rrg;  _##type##3 grg;  _##type##3 brg;  _##type##3 rgg;   \
			_##type##3 ggg;  _##type##3 bgg;  _##type##3 rbg;  _##type##3 gbg;   \
			_##type##3 bbg;  _##type##3 rrb;  _##type##3 grb;  _##type##3 brb;   \
			_##type##3 rgb;  _##type##3 ggb;  _##type##3 bgb;  _##type##3 rbb;   \
			_##type##3 gbb;  _##type##3 bbb;                                     \
			_##type##4 xxxx; _##type##4 yxxx; _##type##4 zxxx; _##type##4 xyxx;  \
			_##type##4 yyxx; _##type##4 zyxx; _##type##4 xzxx; _##type##4 yzxx;  \
			_##type##4 zzxx; _##type##4 xxyx; _##type##4 yxyx; _##type##4 zxyx;  \
			_##type##4 xyyx; _##type##4 yyyx; _##type##4 zyyx; _##type##4 xzyx;  \
			_##type##4 yzyx; _##type##4 zzyx; _##type##4 xxzx; _##type##4 yxzx;  \
			_##type##4 zxzx; _##type##4 xyzx; _##type##4 yyzx; _##type##4 zyzx;  \
			_##type##4 xzzx; _##type##4 yzzx; _##type##4 zzzx; _##type##4 xxxy;  \
			_##type##4 yxxy; _##type##4 zxxy; _##type##4 xyxy; _##type##4 yyxy;  \
			_##type##4 zyxy; _##type##4 xzxy; _##type##4 yzxy; _##type##4 zzxy;  \
			_##type##4 xxyy; _##type##4 yxyy; _##type##4 zxyy; _##type##4 xyyy;  \
			_##type##4 yyyy; _##type##4 zyyy; _##type##4 xzyy; _##type##4 yzyy;  \
			_##type##4 zzyy; _##type##4 xxzy; _##type##4 yxzy; _##type##4 zxzy;  \
			_##type##4 xyzy; _##type##4 yyzy; _##type##4 zyzy; _##type##4 xzzy;  \
			_##type##4 yzzy; _##type##4 zzzy; _##type##4 xxxz; _##type##4 yxxz;  \
			_##type##4 zxxz; _##type##4 xyxz; _##type##4 yyxz; _##type##4 zyxz;  \
			_##type##4 xzxz; _##type##4 yzxz; _##type##4 zzxz; _##type##4 xxyz;  \
			_##type##4 yxyz; _##type##4 zxyz; _##type##4 xyyz; _##type##4 yyyz;  \
			_##type##4 zyyz; _##type##4 xzyz; _##type##4 yzyz; _##type##4 zzyz;  \
			_##type##4 xxzz; _##type##4 yxzz; _##type##4 zxzz; _##type##4 xyzz;  \
			_##type##4 yyzz; _##type##4 zyzz; _##type##4 xzzz; _##type##4 yzzz;  \
			_##type##4 zzzz; _##type##4 rrrr; _##type##4 grrr; _##type##4 brrr;  \
			_##type##4 rgrr; _##type##4 ggrr; _##type##4 bgrr; _##type##4 rbrr;  \
			_##type##4 gbrr; _##type##4 bbrr; _##type##4 rrgr; _##type##4 grgr;  \
			_##type##4 brgr; _##type##4 rggr; _##type##4 gggr; _##type##4 bggr;  \
			_##type##4 rbgr; _##type##4 gbgr; _##type##4 bbgr; _##type##4 rrbr;  \
			_##type##4 grbr; _##type##4 brbr; _##type##4 rgbr; _##type##4 ggbr;  \
			_##type##4 bgbr; _##type##4 rbbr; _##type##4 gbbr; _##type##4 bbbr;  \
			_##type##4 rrrg; _##type##4 grrg; _##type##4 brrg; _##type##4 rgrg;  \
			_##type##4 ggrg; _##type##4 bgrg; _##type##4 rbrg; _##type##4 gbrg;  \
			_##type##4 bbrg; _##type##4 rrgg; _##type##4 grgg; _##type##4 brgg;  \
			_##type##4 rggg; _##type##4 gggg; _##type##4 bggg; _##type##4 rbgg;  \
			_##type##4 gbgg; _##type##4 bbgg; _##type##4 rrbg; _##type##4 grbg;  \
			_##type##4 brbg; _##type##4 rgbg; _##type##4 ggbg; _##type##4 bgbg;  \
			_##type##4 rbbg; _##type##4 gbbg; _##type##4 bbbg; _##type##4 rrrb;  \
			_##type##4 grrb; _##type##4 brrb; _##type##4 rgrb; _##type##4 ggrb;  \
			_##type##4 bgrb; _##type##4 rbrb; _##type##4 gbrb; _##type##4 bbrb;  \
			_##type##4 rrgb; _##type##4 grgb; _##type##4 brgb; _##type##4 rggb;  \
			_##type##4 gggb; _##type##4 bggb; _##type##4 rbgb; _##type##4 gbgb;  \
			_##type##4 bbgb; _##type##4 rrbb; _##type##4 grbb; _##type##4 brbb;  \
			_##type##4 rgbb; _##type##4 ggbb; _##type##4 bgbb; _##type##4 rbbb;  \
			_##type##4 gbbb; _##type##4 bbbb;                                    \
		};                                                                       \
	};                                                                           \
	inline type##3 operator*( auto l, const type##3 &r ) { return r * l; }       \
	inline type##3 operator/( auto l, const type##3 &r ) { return r / l; }       \
	inline type##3 operator+( auto l, const type##3 &r ) { return r + l; }       \
	inline type##3 operator-( auto l, const type##3 &r ) { return r - l; }

#define vector_type_4_alias( type )                                              \
	struct type##4;                                                              \
	struct _##type##4 {                                                          \
		_##type##4();                                                            \
		_##type##4( const type##4 &xyzw );                                       \
		_##type##4( const _##type##4 &xyzw );                                    \
		_##type##4( type##3 xyz, auto w );                                       \
		_##type##4( _##type##3 xyz, auto w );                                    \
		_##type##4( auto x, type##3 yzw );                                       \
		_##type##4( auto x, _##type##3 yzw );                                    \
		_##type##4( type##2 xy, auto z, auto w );                                \
		_##type##4( _##type##2 xy, auto z, auto w );                             \
		_##type##4( auto x, type##2 yz, auto w );                                \
		_##type##4( auto x, _##type##2 yz, auto w );                             \
		_##type##4( auto x, auto y, type##2 zw );                                \
		_##type##4( auto x, auto y, _##type##2 zw );                             \
		_##type##4( auto x, auto y, auto z, auto w );                            \
		_##type##4( type##2 xy, type##2 zw );                                    \
		_##type##4( _##type##2 xy, _##type##2 zw );                              \
		_##type##4 &operator*=( auto other );                                    \
		_##type##4 &operator*=( const type##4 &other );                          \
		_##type##4 &operator*=( const _##type##4 &other );                       \
		_##type##4 &operator/=( auto other );                                    \
		_##type##4 &operator/=( const type##4 &other );                          \
		_##type##4 &operator/=( const _##type##4 &other );                       \
		_##type##4 &operator+=( auto other );                                    \
		_##type##4 &operator+=( const type##4 &other );                          \
		_##type##4 &operator+=( const _##type##4 &other );                       \
		_##type##4 &operator-=( auto other );                                    \
		_##type##4 &operator-=( const type##4 &other );                          \
		_##type##4 &operator-=( const _##type##4 &other );                       \
		_##type##4 operator*( auto other );                                      \
		_##type##4 operator*( const type##4 &other );                            \
		_##type##4 operator*( const _##type##4 &other );                         \
		_##type##4 operator/( auto other );                                      \
		_##type##4 operator/( const type##4 &other );                            \
		_##type##4 operator/( const _##type##4 &other );                         \
		_##type##4 operator+( auto other );                                      \
		_##type##4 operator+( const type##4 &other );                            \
		_##type##4 operator+( const _##type##4 &other );                         \
		_##type##4 operator-( auto other );                                      \
		_##type##4 operator-( const type##4 &other );                            \
		_##type##4 operator-( const _##type##4 &other );                         \
		operator type##4() const;                                                \
		bool operator==( const type##4 &other );                                 \
		bool operator==( const _##type##4 &other );                              \
		bool operator>=( const type##4 &other );                                 \
		bool operator>=( const _##type##4 &other );                              \
		bool operator>( const type##4 &other );                                  \
		bool operator>( const _##type##4 &other );                               \
		bool operator<=( const type##4 &other );                                 \
		bool operator<=( const _##type##4 &other );                              \
		bool operator<( const type##4 &other );                                  \
		bool operator<( const _##type##4 &other );                               \
		type &operator[]( int index );                                           \
	};                                                                           \
	inline _##type##4 operator*( auto l, const _##type##4 &r ) { return r * l; } \
	inline _##type##4 operator/( auto l, const _##type##4 &r ) { return r / l; } \
	inline _##type##4 operator+( auto l, const _##type##4 &r ) { return r + l; } \
	inline _##type##4 operator-( auto l, const _##type##4 &r ) { return r - l; }

#define vector_type_4( type )                                                    \
	struct type##4 : _##type##4 {                                                \
		type##4();                                                               \
		type##4( const type##4 &xyzw );                                          \
		type##4( auto x, auto y, auto z, auto w );                               \
		type##4( type##3 xyz, auto w );                                          \
		type##4( _##type##3 xyz, auto w );                                       \
		type##4( auto x, type##3 yzw );                                          \
		type##4( auto x, _##type##3 yzw );                                       \
		type##4( type##2 xy, auto z, auto w );                                   \
		type##4( _##type##2 xy, auto z, auto w );                                \
		type##4( auto x, type##2 yz, auto w );                                   \
		type##4( auto x, _##type##2 yz, auto w );                                \
		type##4( auto x, auto w, type##2 zw );                                   \
		type##4( auto x, auto w, _##type##2 zw );                                \
		type##4( type##2 xy, type##2 zw );                                       \
		type##4( _##type##2 xy, _##type##2 zw );                                 \
		type##4 &operator*=( auto other );                                       \
		type##4 &operator*=( const type##4 &other );                             \
		type##4 &operator*=( const _##type##4 &other );                          \
		type##4 &operator/=( auto other );                                       \
		type##4 &operator/=( const type##4 &other );                             \
		type##4 &operator/=( const _##type##4 &other );                          \
		type##4 &operator+=( auto other );                                       \
		type##4 &operator+=( const type##4 &other );                             \
		type##4 &operator+=( const _##type##4 &other );                          \
		type##4 &operator-=( auto other );                                       \
		type##4 &operator-=( const type##4 &other );                             \
		type##4 &operator-=( const _##type##4 &other );                          \
		type##4 operator*( auto other );                                         \
		type##4 operator*( const type##4 &other );                               \
		type##4 operator*( const _##type##4 &other );                            \
		type##4 operator/( auto other );                                         \
		type##4 operator/( const type##4 &other );                               \
		type##4 operator/( const _##type##4 &other );                            \
		type##4 operator+( auto other );                                         \
		type##4 operator+( const type##4 &other );                               \
		type##4 operator+( const _##type##4 &other );                            \
		type##4 operator-( auto other );                                         \
		type##4 operator-( const type##4 &other );                               \
		type##4 operator-( const _##type##4 &other );                            \
		union {                                                                  \
			type x, y, z, w;                                                     \
			type r, g, b, a;                                                     \
			_##type##2 xx;   _##type##2 yx;   _##type##2 zx;   _##type##2 wx;    \
			_##type##2 xy;   _##type##2 yy;   _##type##2 zy;   _##type##2 wy;    \
			_##type##2 xz;   _##type##2 yz;   _##type##2 zz;   _##type##2 wz;    \
			_##type##2 xw;   _##type##2 yw;   _##type##2 zw;   _##type##2 ww;    \
			_##type##2 rr;   _##type##2 gr;   _##type##2 br;   _##type##2 ar;    \
			_##type##2 rg;   _##type##2 gg;   _##type##2 bg;   _##type##2 ag;    \
			_##type##2 rb;   _##type##2 gb;   _##type##2 bb;   _##type##2 ab;    \
			_##type##2 ra;   _##type##2 ga;   _##type##2 ba;   _##type##2 aa;    \
			_##type##3 xxx;  _##type##3 yxx;  _##type##3 zxx;  _##type##3 wxx;   \
			_##type##3 xyx;  _##type##3 yyx;  _##type##3 zyx;  _##type##3 wyx;   \
			_##type##3 xzx;  _##type##3 yzx;  _##type##3 zzx;  _##type##3 wzx;   \
			_##type##3 xwx;  _##type##3 ywx;  _##type##3 zwx;  _##type##3 wwx;   \
			_##type##3 xxy;  _##type##3 yxy;  _##type##3 zxy;  _##type##3 wxy;   \
			_##type##3 xyy;  _##type##3 yyy;  _##type##3 zyy;  _##type##3 wyy;   \
			_##type##3 xzy;  _##type##3 yzy;  _##type##3 zzy;  _##type##3 wzy;   \
			_##type##3 xwy;  _##type##3 ywy;  _##type##3 zwy;  _##type##3 wwy;   \
			_##type##3 xxz;  _##type##3 yxz;  _##type##3 zxz;  _##type##3 wxz;   \
			_##type##3 xyz;  _##type##3 yyz;  _##type##3 zyz;  _##type##3 wyz;   \
			_##type##3 xzz;  _##type##3 yzz;  _##type##3 zzz;  _##type##3 wzz;   \
			_##type##3 xwz;  _##type##3 ywz;  _##type##3 zwz;  _##type##3 wwz;   \
			_##type##3 xxw;  _##type##3 yxw;  _##type##3 zxw;  _##type##3 wxw;   \
			_##type##3 xyw;  _##type##3 yyw;  _##type##3 zyw;  _##type##3 wyw;   \
			_##type##3 xzw;  _##type##3 yzw;  _##type##3 zzw;  _##type##3 wzw;   \
			_##type##3 xww;  _##type##3 yww;  _##type##3 zww;  _##type##3 www;   \
			_##type##3 rrr;  _##type##3 grr;  _##type##3 brr;  _##type##3 arr;   \
			_##type##3 rgr;  _##type##3 ggr;  _##type##3 bgr;  _##type##3 agr;   \
			_##type##3 rbr;  _##type##3 gbr;  _##type##3 bbr;  _##type##3 abr;   \
			_##type##3 rar;  _##type##3 gar;  _##type##3 bar;  _##type##3 aar;   \
			_##type##3 rrg;  _##type##3 grg;  _##type##3 brg;  _##type##3 arg;   \
			_##type##3 rgg;  _##type##3 ggg;  _##type##3 bgg;  _##type##3 agg;   \
			_##type##3 rbg;  _##type##3 gbg;  _##type##3 bbg;  _##type##3 abg;   \
			_##type##3 rag;  _##type##3 gag;  _##type##3 bag;  _##type##3 aag;   \
			_##type##3 rrb;  _##type##3 grb;  _##type##3 brb;  _##type##3 arb;   \
			_##type##3 rgb;  _##type##3 ggb;  _##type##3 bgb;  _##type##3 agb;   \
			_##type##3 rbb;  _##type##3 gbb;  _##type##3 bbb;  _##type##3 abb;   \
			_##type##3 rab;  _##type##3 gab;  _##type##3 bab;  _##type##3 aab;   \
			_##type##3 rra;  _##type##3 gra;  _##type##3 bra;  _##type##3 ara;   \
			_##type##3 rga;  _##type##3 gga;  _##type##3 bga;  _##type##3 aga;   \
			_##type##3 rba;  _##type##3 gba;  _##type##3 bba;  _##type##3 aba;   \
			_##type##3 raa;  _##type##3 gaa;  _##type##3 baa;  _##type##3 aaa;   \
			_##type##4 xxxx; _##type##4 yxxx; _##type##4 zxxx; _##type##4 wxxx;  \
			_##type##4 xyxx; _##type##4 yyxx; _##type##4 zyxx; _##type##4 wyxx;  \
			_##type##4 xzxx; _##type##4 yzxx; _##type##4 zzxx; _##type##4 wzxx;  \
			_##type##4 xwxx; _##type##4 ywxx; _##type##4 zwxx; _##type##4 wwxx;  \
			_##type##4 xxyx; _##type##4 yxyx; _##type##4 zxyx; _##type##4 wxyx;  \
			_##type##4 xyyx; _##type##4 yyyx; _##type##4 zyyx; _##type##4 wyyx;  \
			_##type##4 xzyx; _##type##4 yzyx; _##type##4 zzyx; _##type##4 wzyx;  \
			_##type##4 xwyx; _##type##4 ywyx; _##type##4 zwyx; _##type##4 wwyx;  \
			_##type##4 xxzx; _##type##4 yxzx; _##type##4 zxzx; _##type##4 wxzx;  \
			_##type##4 xyzx; _##type##4 yyzx; _##type##4 zyzx; _##type##4 wyzx;  \
			_##type##4 xzzx; _##type##4 yzzx; _##type##4 zzzx; _##type##4 wzzx;  \
			_##type##4 xwzx; _##type##4 ywzx; _##type##4 zwzx; _##type##4 wwzx;  \
			_##type##4 xxwx; _##type##4 yxwx; _##type##4 zxwx; _##type##4 wxwx;  \
			_##type##4 xywx; _##type##4 yywx; _##type##4 zywx; _##type##4 wywx;  \
			_##type##4 xzwx; _##type##4 yzwx; _##type##4 zzwx; _##type##4 wzwx;  \
			_##type##4 xwwx; _##type##4 ywwx; _##type##4 zwwx; _##type##4 wwwx;  \
			_##type##4 xxxy; _##type##4 yxxy; _##type##4 zxxy; _##type##4 wxxy;  \
			_##type##4 xyxy; _##type##4 yyxy; _##type##4 zyxy; _##type##4 wyxy;  \
			_##type##4 xzxy; _##type##4 yzxy; _##type##4 zzxy; _##type##4 wzxy;  \
			_##type##4 xwxy; _##type##4 ywxy; _##type##4 zwxy; _##type##4 wwxy;  \
			_##type##4 xxyy; _##type##4 yxyy; _##type##4 zxyy; _##type##4 wxyy;  \
			_##type##4 xyyy; _##type##4 yyyy; _##type##4 zyyy; _##type##4 wyyy;  \
			_##type##4 xzyy; _##type##4 yzyy; _##type##4 zzyy; _##type##4 wzyy;  \
			_##type##4 xwyy; _##type##4 ywyy; _##type##4 zwyy; _##type##4 wwyy;  \
			_##type##4 xxzy; _##type##4 yxzy; _##type##4 zxzy; _##type##4 wxzy;  \
			_##type##4 xyzy; _##type##4 yyzy; _##type##4 zyzy; _##type##4 wyzy;  \
			_##type##4 xzzy; _##type##4 yzzy; _##type##4 zzzy; _##type##4 wzzy;  \
			_##type##4 xwzy; _##type##4 ywzy; _##type##4 zwzy; _##type##4 wwzy;  \
			_##type##4 xxwy; _##type##4 yxwy; _##type##4 zxwy; _##type##4 wxwy;  \
			_##type##4 xywy; _##type##4 yywy; _##type##4 zywy; _##type##4 wywy;  \
			_##type##4 xzwy; _##type##4 yzwy; _##type##4 zzwy; _##type##4 wzwy;  \
			_##type##4 xwwy; _##type##4 ywwy; _##type##4 zwwy; _##type##4 wwwy;  \
			_##type##4 xxxz; _##type##4 yxxz; _##type##4 zxxz; _##type##4 wxxz;  \
			_##type##4 xyxz; _##type##4 yyxz; _##type##4 zyxz; _##type##4 wyxz;  \
			_##type##4 xzxz; _##type##4 yzxz; _##type##4 zzxz; _##type##4 wzxz;  \
			_##type##4 xwxz; _##type##4 ywxz; _##type##4 zwxz; _##type##4 wwxz;  \
			_##type##4 xxyz; _##type##4 yxyz; _##type##4 zxyz; _##type##4 wxyz;  \
			_##type##4 xyyz; _##type##4 yyyz; _##type##4 zyyz; _##type##4 wyyz;  \
			_##type##4 xzyz; _##type##4 yzyz; _##type##4 zzyz; _##type##4 wzyz;  \
			_##type##4 xwyz; _##type##4 ywyz; _##type##4 zwyz; _##type##4 wwyz;  \
			_##type##4 xxzz; _##type##4 yxzz; _##type##4 zxzz; _##type##4 wxzz;  \
			_##type##4 xyzz; _##type##4 yyzz; _##type##4 zyzz; _##type##4 wyzz;  \
			_##type##4 xzzz; _##type##4 yzzz; _##type##4 zzzz; _##type##4 wzzz;  \
			_##type##4 xwzz; _##type##4 ywzz; _##type##4 zwzz; _##type##4 wwzz;  \
			_##type##4 xxwz; _##type##4 yxwz; _##type##4 zxwz; _##type##4 wxwz;  \
			_##type##4 xywz; _##type##4 yywz; _##type##4 zywz; _##type##4 wywz;  \
			_##type##4 xzwz; _##type##4 yzwz; _##type##4 zzwz; _##type##4 wzwz;  \
			_##type##4 xwwz; _##type##4 ywwz; _##type##4 zwwz; _##type##4 wwwz;  \
			_##type##4 xxxw; _##type##4 yxxw; _##type##4 zxxw; _##type##4 wxxw;  \
			_##type##4 xyxw; _##type##4 yyxw; _##type##4 zyxw; _##type##4 wyxw;  \
			_##type##4 xzxw; _##type##4 yzxw; _##type##4 zzxw; _##type##4 wzxw;  \
			_##type##4 xwxw; _##type##4 ywxw; _##type##4 zwxw; _##type##4 wwxw;  \
			_##type##4 xxyw; _##type##4 yxyw; _##type##4 zxyw; _##type##4 wxyw;  \
			_##type##4 xyyw; _##type##4 yyyw; _##type##4 zyyw; _##type##4 wyyw;  \
			_##type##4 xzyw; _##type##4 yzyw; _##type##4 zzyw; _##type##4 wzyw;  \
			_##type##4 xwyw; _##type##4 ywyw; _##type##4 zwyw; _##type##4 wwyw;  \
			_##type##4 xxzw; _##type##4 yxzw; _##type##4 zxzw; _##type##4 wxzw;  \
			_##type##4 xyzw; _##type##4 yyzw; _##type##4 zyzw; _##type##4 wyzw;  \
			_##type##4 xzzw; _##type##4 yzzw; _##type##4 zzzw; _##type##4 wzzw;  \
			_##type##4 xwzw; _##type##4 ywzw; _##type##4 zwzw; _##type##4 wwzw;  \
			_##type##4 xxww; _##type##4 yxww; _##type##4 zxww; _##type##4 wxww;  \
			_##type##4 xyww; _##type##4 yyww; _##type##4 zyww; _##type##4 wyww;  \
			_##type##4 xzww; _##type##4 yzww; _##type##4 zzww; _##type##4 wzww;  \
			_##type##4 xwww; _##type##4 ywww; _##type##4 zwww; _##type##4 wwww;  \
			_##type##4 rrrr; _##type##4 grrr; _##type##4 brrr; _##type##4 arrr;  \
			_##type##4 rgrr; _##type##4 ggrr; _##type##4 bgrr; _##type##4 agrr;  \
			_##type##4 rbrr; _##type##4 gbrr; _##type##4 bbrr; _##type##4 abrr;  \
			_##type##4 rarr; _##type##4 garr; _##type##4 barr; _##type##4 aarr;  \
			_##type##4 rrgr; _##type##4 grgr; _##type##4 brgr; _##type##4 argr;  \
			_##type##4 rggr; _##type##4 gggr; _##type##4 bggr; _##type##4 aggr;  \
			_##type##4 rbgr; _##type##4 gbgr; _##type##4 bbgr; _##type##4 abgr;  \
			_##type##4 ragr; _##type##4 gagr; _##type##4 bagr; _##type##4 aagr;  \
			_##type##4 rrbr; _##type##4 grbr; _##type##4 brbr; _##type##4 arbr;  \
			_##type##4 rgbr; _##type##4 ggbr; _##type##4 bgbr; _##type##4 agbr;  \
			_##type##4 rbbr; _##type##4 gbbr; _##type##4 bbbr; _##type##4 abbr;  \
			_##type##4 rabr; _##type##4 gabr; _##type##4 babr; _##type##4 aabr;  \
			_##type##4 rrar; _##type##4 grar; _##type##4 brar; _##type##4 arar;  \
			_##type##4 rgar; _##type##4 ggar; _##type##4 bgar; _##type##4 agar;  \
			_##type##4 rbar; _##type##4 gbar; _##type##4 bbar; _##type##4 abar;  \
			_##type##4 raar; _##type##4 gaar; _##type##4 baar; _##type##4 aaar;  \
			_##type##4 rrrg; _##type##4 grrg; _##type##4 brrg; _##type##4 arrg;  \
			_##type##4 rgrg; _##type##4 ggrg; _##type##4 bgrg; _##type##4 agrg;  \
			_##type##4 rbrg; _##type##4 gbrg; _##type##4 bbrg; _##type##4 abrg;  \
			_##type##4 rarg; _##type##4 garg; _##type##4 barg; _##type##4 aarg;  \
			_##type##4 rrgg; _##type##4 grgg; _##type##4 brgg; _##type##4 argg;  \
			_##type##4 rggg; _##type##4 gggg; _##type##4 bggg; _##type##4 aggg;  \
			_##type##4 rbgg; _##type##4 gbgg; _##type##4 bbgg; _##type##4 abgg;  \
			_##type##4 ragg; _##type##4 gagg; _##type##4 bagg; _##type##4 aagg;  \
			_##type##4 rrbg; _##type##4 grbg; _##type##4 brbg; _##type##4 arbg;  \
			_##type##4 rgbg; _##type##4 ggbg; _##type##4 bgbg; _##type##4 agbg;  \
			_##type##4 rbbg; _##type##4 gbbg; _##type##4 bbbg; _##type##4 abbg;  \
			_##type##4 rabg; _##type##4 gabg; _##type##4 babg; _##type##4 aabg;  \
			_##type##4 rrag; _##type##4 grag; _##type##4 brag; _##type##4 arag;  \
			_##type##4 rgag; _##type##4 ggag; _##type##4 bgag; _##type##4 agag;  \
			_##type##4 rbag; _##type##4 gbag; _##type##4 bbag; _##type##4 abag;  \
			_##type##4 raag; _##type##4 gaag; _##type##4 baag; _##type##4 aaag;  \
			_##type##4 rrrb; _##type##4 grrb; _##type##4 brrb; _##type##4 arrb;  \
			_##type##4 rgrb; _##type##4 ggrb; _##type##4 bgrb; _##type##4 agrb;  \
			_##type##4 rbrb; _##type##4 gbrb; _##type##4 bbrb; _##type##4 abrb;  \
			_##type##4 rarb; _##type##4 garb; _##type##4 barb; _##type##4 aarb;  \
			_##type##4 rrgb; _##type##4 grgb; _##type##4 brgb; _##type##4 argb;  \
			_##type##4 rggb; _##type##4 gggb; _##type##4 bggb; _##type##4 aggb;  \
			_##type##4 rbgb; _##type##4 gbgb; _##type##4 bbgb; _##type##4 abgb;  \
			_##type##4 ragb; _##type##4 gagb; _##type##4 bagb; _##type##4 aagb;  \
			_##type##4 rrbb; _##type##4 grbb; _##type##4 brbb; _##type##4 arbb;  \
			_##type##4 rgbb; _##type##4 ggbb; _##type##4 bgbb; _##type##4 agbb;  \
			_##type##4 rbbb; _##type##4 gbbb; _##type##4 bbbb; _##type##4 abbb;  \
			_##type##4 rabb; _##type##4 gabb; _##type##4 babb; _##type##4 aabb;  \
			_##type##4 rrab; _##type##4 grab; _##type##4 brab; _##type##4 arab;  \
			_##type##4 rgab; _##type##4 ggab; _##type##4 bgab; _##type##4 agab;  \
			_##type##4 rbab; _##type##4 gbab; _##type##4 bbab; _##type##4 abab;  \
			_##type##4 raab; _##type##4 gaab; _##type##4 baab; _##type##4 aaab;  \
			_##type##4 rrra; _##type##4 grra; _##type##4 brra; _##type##4 arra;  \
			_##type##4 rgra; _##type##4 ggra; _##type##4 bgra; _##type##4 agra;  \
			_##type##4 rbra; _##type##4 gbra; _##type##4 bbra; _##type##4 abra;  \
			_##type##4 rara; _##type##4 gara; _##type##4 bara; _##type##4 aara;  \
			_##type##4 rrga; _##type##4 grga; _##type##4 brga; _##type##4 arga;  \
			_##type##4 rgga; _##type##4 ggga; _##type##4 bgga; _##type##4 agga;  \
			_##type##4 rbga; _##type##4 gbga; _##type##4 bbga; _##type##4 abga;  \
			_##type##4 raga; _##type##4 gaga; _##type##4 baga; _##type##4 aaga;  \
			_##type##4 rrba; _##type##4 grba; _##type##4 brba; _##type##4 arba;  \
			_##type##4 rgba; _##type##4 ggba; _##type##4 bgba; _##type##4 agba;  \
			_##type##4 rbba; _##type##4 gbba; _##type##4 bbba; _##type##4 abba;  \
			_##type##4 raba; _##type##4 gaba; _##type##4 baba; _##type##4 aaba;  \
			_##type##4 rraa; _##type##4 graa; _##type##4 braa; _##type##4 araa;  \
			_##type##4 rgaa; _##type##4 ggaa; _##type##4 bgaa; _##type##4 agaa;  \
			_##type##4 rbaa; _##type##4 gbaa; _##type##4 bbaa; _##type##4 abaa;  \
			_##type##4 raaa; _##type##4 gaaa; _##type##4 baaa; _##type##4 aaaa;  \
		};                                                                       \
	};                                                                           \
	inline type##4 operator*( auto l, const type##4 &r ) { return r * l; }       \
	inline type##4 operator/( auto l, const type##4 &r ) { return r / l; }       \
	inline type##4 operator+( auto l, const type##4 &r ) { return r + l; }       \
	inline type##4 operator-( auto l, const type##4 &r ) { return r - l; }

#define vector_type( type )      \
	vector_type_2_alias( type ); \
	vector_type_3_alias( type ); \
	vector_type_4_alias( type ); \
	vector_type_2( type );       \
	vector_type_3( type );       \
	vector_type_4( type );

vector_type( float );
vector_type( int );
vector_type( uint );
vector_type( bool );

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

template <typename A> auto sin( A x ) { return x; }
template <typename A> auto cos( A x ) { return x; }
template <typename A> auto tan( A x ) { return x; }
template <typename A> auto asin( A x ) { return x; }
template <typename A> auto acos( A x ) { return x; }
template <typename A> auto atan( A x ) { return x; }
template <typename A, typename B> auto atan2( A y, B x ) { return x; }
template <typename A> auto sinh( A x ) { return x; }
template <typename A> auto cosh( A x ) { return x; }
template <typename A> auto tanh( A x ) { return x; }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Exponential and Logarithmic Functions

template <typename A> auto exp( A x ) { return x; }
template <typename A> auto exp2( A x ) { return x; }
template <typename A> auto log( A x ) { return x; }
template <typename A> auto log2( A x ) { return x; }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Conversion Functions

template <typename A> auto degrees( A rad ) { return rad; }
template <typename A> auto radians( A deg ) { return deg; }
template <typename A> auto round( A x ) { return x; }
template <typename A> auto trunc( A x ) { return x; }
template <typename A> auto ceil( A x ) { return x; }
template <typename A> auto floor( A x ) { return x; }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Arithmetic Functions

template <typename A> auto abs( A x ) { return x; }
template <typename A, typename B> auto pow( A x, B y ) { return x; }
template <typename A> auto sqrt( A x ) { return x; }
template <typename A> auto rsqrt( A x ) { return x; }
template <typename A, typename B, typename C> auto clamp( A x, B min, C max ) { return x; }
template <typename A, typename B> auto max( A x, B y ) { return x; }
template <typename A, typename B> auto min( A x, B y ) { return x; }
template <typename A, typename B> auto mod( A x, B y ) { return x; }
template <typename A> auto frac( A x ) { return x; }
template <typename A, typename B> auto ldexp( A x, B exp ) { return x; }
template <typename A, typename B, typename C> auto fma( A a, B b, C c ) { return a; }
template <typename A> auto sign( A x ) { return x; }
template <typename A> auto saturate( A x ) { return x; }
template <typename A> auto ddx( A x ) { return x; }
template <typename A> auto ddy( A y ) { return y; }
template <typename A> auto ddx_coarse( A x ) { return x; }
template <typename A> auto ddx_fine( A x ) { return x; }
template <typename A> auto ddy_coarse( A y ) { return y; }
template <typename A> auto ddy_fine( A y ) { return y; }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Vector and Matrix Functions

template <typename A, typename B> B mul( A x, B y ) { return y; }
template <typename A> float length( A vector ) { return 0.0f; }
template <typename A> A inverse( A matrix ) { return matrix; }
template <typename A, typename B> float distance( A point1, B point2 ) { return 0.0f; }
template <typename A, typename B> float dot( A vector1, B vector2 ) { return 0.0f; }
template <typename A, typename B> auto cross( A vector1, B vector2 ) { return vector1; }
template <typename A> auto normalize( A x ) { return x; }
template <typename A, typename B> auto reflect( A incident, B normal ) { return incident; }
template <typename A, typename B, typename C> auto refract( A incident, B normal, C eta ) { return incident; }
template <typename A, typename B, typename C> auto faceforward( A normal, B incident, C ng ) { return normal; }
template <typename A> auto transpose( A matrix ) { return matrix; }
template <typename A> float determinant( A matrix ) { return 0.0f; }
template <typename A, typename B, typename C> auto lerp( A x, B y, C s ) { return x; }
template <typename A, typename B> auto step( A threshold, B x ) { return x; }
template <typename A, typename B, typename C> auto smoothstep( A min, B max, C x ) { return x; }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Bitwise Operations

template <typename A> auto bit_count( A x ) { return 0; }
template <typename A> auto bit_firsthigh( A x ) { return 0; }
template <typename A> auto bit_firstlow( A x ) { return 0; }
template <typename A> auto bit_reverse( A x ) { return 0; }

template <typename A> int float_to_int_bits( A x ) { return 0; }
template <typename A> uint float_to_uint_bits( A x ) { return 0; }
template <typename A> float int_to_float_bits( A x ) { return 0.0f; }
template <typename A> float uint_to_float_bits( A x ) { return 0.0f; }

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

inline float depth_unproject( float4 position ) { return 0.0f; }
inline float depth_unproject_zw( float depthZ, float depthW ) { return 0.0f; }
inline float depth_normalize( float depth, float near, float far ) { return 0.0f; }
inline float depth_linearize( float depth, float near, float far ) { return 0.0f; }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Built-in System-Value Semantics

inline uint SV_VertexID;
inline uint SV_InstanceID;
inline uint SV_PrimitiveID;
inline uint SV_SampleID;
inline bool SV_IsFrontFace;
inline uint3 SV_DispatchThreadID;
inline uint3 SV_GroupThreadID;
inline uint3 SV_GroupID;
inline uint SV_GroupIndex;

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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Stages

#define vertex_main vertex_main
#define fragment_main fragment_main

#define compute_main( x, y, z ) compute_main

#define ray_generate ray_generate
#define ray_hit_any ray_hit_any
#define ray_hit_closest ray_hit_closest
#define ray_miss ray_miss
#define ray_intersection ray_intersection
#define ray_callable ray_callable

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////