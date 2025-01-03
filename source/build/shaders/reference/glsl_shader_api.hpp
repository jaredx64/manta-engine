#pragma once

#define mul(x, y) x * y

#define float4 vec4
#define float3 vec3
#define float2 vec2

#define float4x4 mat4x4
#define float3x3 mat3x3
#define float2x2 mat2x2

#if 0

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Keywords

#define float2 vec222
#define float3 vec3
#define float4 vec4

#define float4x4 mat4
#define float3x3 mat3
#define float2x2 mat2

// Removed the HLSL function 'void abort()' as GLSL does not have the same function.

float abs( float x ); // GLSL: float abs( x );

float acos( float x ); // GLSL: float acos( x );

// Removed the HLSL function 'bool all(float x)' as GLSL does not have the same function.

void AllMemoryBarrier(); // GLSL: barrier();

void AllMemoryBarrierWithGroupSync(); // GLSL: barrier();

// Removed the HLSL function 'bool any(float x)' as GLSL does not have the same function.

// double asdouble(long long value); // GLSL: double asdouble(int64_t value);

float asfloat( int x ); // GLSL: float float( x );

float asin( float x ); // GLSL: float asin( x );

int asint( float x ); // GLSL: int int( x );

uint asuint( float x ); // GLSL: uint uint( x );

//uint asuint(double x); // GLSL: uint asuint( double x );

float atan( float x ); // GLSL: float atan( x );

float atan2( float y, float x ); // GLSL: float atan( y, x );

float ceil( float x ); // GLSL: float ceil( x );

// Removed the HLSL function 'bool CheckAccessFullyMapped(int value)' as GLSL does not have the same function.

float clamp( float x, float minVal, float maxVal ); // GLSL: vec3 clamp( x, minVal, maxVal );

// Removed the HLSL function 'void clip(float x)' as GLSL uses 'discard' for this purpose.

float cos( float x ); // GLSL: float cos( x );

float cosh( float x ); // GLSL: float cosh( x );

int countbits( int x ); // GLSL: int bitCount( x );

float3 cross( float3 x, float3 y ); // GLSL: cross( x, y );

float ddx( float x ); // GLSL: dFdx( x );

float ddx_coarse( float x ); // GLSL: dFdxCoarse( x );

float ddx_fine( float x ); // GLSL: dFdxFine( x );

float ddy( float x ); // GLSL: dFdy( x );

float ddy_coarse( float x ); // GLSL: dFdyCoarse( x );

float ddy_fine( float x ); // GLSL: dFdyFine( x );

float degrees( float radians ); // GLSL: degrees( radians );

float determinant( float4x4 m ); // GLSL: float determinant( m );

void DeviceMemoryBarrier(); // GLSL: barrier();

void DeviceMemoryBarrierWithGroupSync(); // GLSL: barrier();

float distance( float3 p0, float3 p1 ); // GLSL: float distance( p0,  p1 );

float dot( float3 x, float3 y ); // GLSL: float dot( x, y );

//float3 dst( float3 p0, float3 p1 ); // GLSL: vec3 distance( p0,  p1 );

// Removed the HLSL function 'void errorf(const char* message)' as GLSL does not have the same function.

// Removed the HLSL functions related to attribute evaluation as GLSL does not have the same functionality.

float exp( float x ); // GLSL: float exp( x );

float exp2( float x ); // GLSL: float exp2( x );

// Removed the HLSL function 'float f16tof32(uint value)' as GLSL does not have the same function.

// Removed the HLSL function 'uint f32tof16(float x)' as GLSL does not have the same function.

// float3 faceforward( float3 n, float3 i, float3 ng ); // GLSL: reflect( i, ng );

int firstbithigh( int x ); // GLSL: int findMSB( x );

int firstbitlow( int x ); // GLSL: int findLSB( x );

float floor( float x ); // GLSL: float floor( x );

double3 frexp( double x, out int exp ); // GLSL: frexp( x, exp );

float2 fwidth( float2 x ); // GLSL: vec2 fwidth( x );

// Removed the HLSL function 'int GetRenderTargetSampleCount()' as GLSL does not have the same function.

// Removed the HLSL function 'vec2 GetRenderTargetSamplePosition(int sampleIndex)' as GLSL does not have the same function.

void GroupMemoryBarrier(); // GLSL: groupMemoryBarrier();

void GroupMemoryBarrierWithGroupSync(); // GLSL: groupMemoryBarrier();

// Removed the HLSL functions related to interlocked operations as GLSL does not have the same functionality.

bool isinf( float x ); // GLSL: isinf( x );

bool isfinite( float x ); // GLSL: ( !isinf( x ) );

bool isnan( float x ); // GLSL: isnan( x );

float ldexp( float x, int exp ); // GLSL: ldexp( x, exp );

float length( float3 v ); // GLSL: length( v );

float lerp( float x, float y, float s ); // GLSL: mix( x, y, s );

// float4 lit( float nDotL, float nDotH, float m ); // GLSL: vec4 lit( nDotL, nDotH, m );

float log( float x ); // GLSL: float log( x );

//float log10( float x ); // GLSL: float log10( x );

float log2( float x ); // GLSL: float log2( x );

float mad( float a, float b, float c ); // GLSL: ( a * b + c );

float max( float x, float y ); // GLSL: max( x, y );

float min( float x, float y ); // GLSL: min( x, y );

float2 modf( float x, out float ip ); // GLSL: vec2 modf( x, ip );

//int4 msad4( int4 reference, int8 source ); // GLSL: uvec4 msad4(uvec4 reference, uvec4 source);

float4 mul( mat4 x, float4 y ); // GLSL: ( x * y );

float noise( float3 x ); // GLSL: float noise( x );

float3 normalize( float3 x ); // GLSL: normalize( x );

float pow( float x, float y ); // GLSL: pow( x, y );

// Removed the HLSL functions 'printf' and 'error' as GLSL does not have the same functionality.

void Process2DQuadTessFactorsAvg( float2 tessFactors[4] ); // GLSL: tessFactors[0] = ( tessFactors[0] + tessFactors[1] + tessFactors[2] + tessFactors[3]) * 0.25;

void Process2DQuadTessFactorsMax( float2 tessFactors[4] ); // GLSL: tessFactors[0] = max( max( tessFactors[0], tessFactors[1] ), max( tessFactors[2], tessFactors[3] ) );

void Process2DQuadTessFactorsMin( float2 tessFactors[4] ); // GLSL: tessFactors[0] = min( min( tessFactors[0], tessFactors[1] ), min( tessFactors[2], tessFactors[3] ) );

void ProcessQuadTessFactorsAvg( float3 tessFactors[4] ); // GLSL: tessFactors[0] = ( tessFactors[0] + tessFactors[1] + tessFactors[2] + tessFactors[3] ) * 0.25;

void ProcessQuadTessFactorsMax( float3 tessFactors[4] ); // GLSL: tessFactors[0] = max( max( tessFactors[0], tessFactors[1] ), max( tessFactors[2], tessFactors[3] ) );

void ProcessQuadTessFactorsMin( float3 tessFactors[4] ); // GLSL: tessFactors[0] = min( min( tessFactors[0], tessFactors[1] ), min( tessFactors[2], tessFactors[3] ) );

void ProcessTriTessFactorsAvg( float3 tessFactors[3] ); // GLSL: tessFactors[0] = ( tessFactors[0] + tessFactors[1] + tessFactors[2] ) * 0.3333;

void ProcessTriTessFactorsMax( float3 tessFactors[3] ); // GLSL: tessFactors[0] = max( max( tessFactors[0], tessFactors[1]), max( tessFactors[2] ) );

void ProcessTriTessFactorsMin( float3 tessFactors[3] ); // GLSL: tessFactors[0] = min( min( tessFactors[0], tessFactors[1]), min( tessFactors[2] ) );

float radians( float degrees ); // GLSL: radians( degrees );

float rcp( float x ); // GLSL: ( 1.0 / x );

float3 reflect( float3 i, float3 n ); // GLSL: reflect( i, n );

float3 refract( float3 i, float3 n, float eta ); // GLSL: refract( i, n, eta );

// int reversebits( int x ); // GLSL: int reverseBits(int x);

float round( float x ); // GLSL: round( x );

float rsqrt( float x ); // GLSL: ( 1.0 / sqrt( x ) );

float saturate( float x ); // GLSL: clamp( x, 0.0, 1.0 );

float sign( float x ); // GLSL: sign( x );

float sin( float x ); // GLSL: sin( x );

//float2 sincos( float x ); // GLSL: vec2 sincos( float x );

float sinh( float x ); // GLSL: sinh( x );

float smoothstep( float edge0, float edge1, float x ); // GLSL: smoothstep( edge0, edge1, x );

float sqrt( float x ); // GLSL: sqrt( x );

float step( float edge, float x ); // GLSL: step( edge, x );

float tan( float x ); // GLSL: tan( x );

float tanh( float x ); // GLSL: tanh( x );

float tex1D( sampler1D s, float t ); // GLSL: texture( s, t );

float tex1D( sampler1D s, float t, float ddx, float ddy ); // GLSL: textureGrad( s, t, ddx, ddy );

//float tex1Dbias( sampler1D s, float t, float bias ); // GLSL: textureBias( s, t, bias );

float tex1Dgrad( sampler1D s, float t, float dPdx, float dPdy ); // GLSL: textureGrad( s, t, dPdx, dPdy );

float tex1Dlod( sampler1D s, float4 coord ); // GLSL: textureLod( s, coord.x, coord.w );

float tex1Dproj( sampler1D s, float2 coord ); // GLSL: textureProj(s, coord);

float4 tex2D( sampler2D s, float2 coord ); // GLSL: texture(s, coord);

float4 tex2D( sampler2D s, float2 coord, float2 ddx, float2 ddy ); // GLSL: textureGrad( s, coord, ddx, ddy );

//float4 tex2Dbias( sampler2D s, float2 coord, float bias ); // GLSL: textureBias( s, coord, bias );

float4 tex2Dgrad( sampler2D s, float2 coord, float2 dPdx, float2 dPdy ); // GLSL: textureGrad( s, coord, dPdx, dPdy );

float4 tex2Dlod( sampler2D s, float4 coord ); // GLSL: textureLod( s, coord.xy, coord.w );

float4 tex2Dproj( sampler2D s, float3 coord ); // GLSL: textureProj( s, coord );

float4 tex3D( sampler3D s, float3 coord ); // GLSL: texture( s, coord );

float4 tex3D( sampler3D s, float3 coord, float3 ddx, float3 ddy ); // GLSL: textureGrad( s, coord, ddx, ddy );

//float4 tex3Dbias( sampler3D s, float3 coord, float bias ); // GLSL: textureBias( s, coord, bias );

float4 tex3Dgrad( sampler3D s, float3 coord, float3 dPdx, float3 dPdy ); // GLSL: textureGrad( s, coord, dPdx, dPdy );

float4 tex3Dlod( sampler3D s, float4 coord ); // GLSL: textureLod( s, coord.xyz, coord.w );

float4 tex3Dproj( sampler3D s, float4 coord ); // GLSL: textureProj( s, coord );

float4 texCUBE( samplerCUBE s, float3 coord ); // GLSL: texture( s, coord );

float4 texCUBE( samplerCUBE s, float3 coord, float3 ddx, float3 ddy ); // GLSL: textureGrad( s, coord, ddx, ddy );

//float4 texCUBEbias( samplerCUBE s, float4 coord, float bias ); // GLSL: textureBias( s, coord, bias );

float4 texCUBEgrad( samplerCUBE s, float3 coord, float3 dPdx, float3 dPdy ); // GLSL: textureGrad( s, coord, dPdx, dPdy );

float4 texCUBElod( samplerCUBE s, float4 coord ); // GLSL: textureLod( s, coord.xyz, coord.w );

float4 texCUBEproj( samplerCUBE s, float4 coord ); // GLSL: textureProj( s, coord );

float4 transpose( mat4 m ); // GLSL: transpose( m );

float trunc( float x ); // GLSL: trunc( x );

#endif