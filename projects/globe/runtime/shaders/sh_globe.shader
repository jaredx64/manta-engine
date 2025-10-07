#include <shader_api.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

vertex_input VertexGlobe
{
	float3 position semantic( POSITION ) format( FLOAT32 );
	float3 normal semantic( NORMAL ) format( FLOAT32 );
	float2 uv semantic( TEXCOORD ) format( UNORM16 );
};

vertex_output VertexOutput
{
	float4 position semantic( POSITION );
	float3 normal semantic( NORMAL );
	float2 uv semantic( TEXCOORD );
};

fragment_input FragmentInput
{
	float4 position semantic( POSITION );
	float3 normal semantic( NORMAL );
	float2 uv semantic( TEXCOORD );
};

fragment_output FragmentOutput
{
	float4 color0 semantic( COLOR ) target( 0 );
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

uniform_buffer( 0 ) UniformsPipeline
{
	float4x4 matrixModel;
	float4x4 matrixView;
	float4x4 matrixPerspective;
	float4x4 matrixMVP;
	float4x4 matrixModelInverse;
	float4x4 matrixViewInverse;
	float4x4 matrixPerspectiveInverse;
	float4x4 matrixMVPInverse;
};

uniform_buffer( 1 ) UniformsGlobe
{
	float3 sun;
};

texture2D( 0, float4 ) textureColor;
texture2D( 1, float4 ) textureLight;
texture2D( 2, float4 ) textureDebug;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void vertex_main( VertexGlobe In, VertexOutput Out, UniformsPipeline Pipeline )
{
	Out.position = mul( Pipeline.matrixMVP, float4( In.position, 1.0 ) );
	Out.normal = In.normal;
	Out.uv = In.uv;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void fragment_main( FragmentInput In, FragmentOutput Out, UniformsGlobe Uniforms )
{
	float4 color = texture_sample_2d( textureColor, In.uv );
	float4 light = texture_sample_2d( textureLight, In.uv );

	float normalDotSun = dot( In.normal, Uniforms.sun );
	float day = saturate( normalDotSun );
	float night = smoothstep( 0.05, -0.25, normalDotSun ); // tune -0.2 for how wide twilight is

	float glow = light.r * night;
	float sunShade = max( 0.06, pow( day, 0.5 ) * 1.5 );
	float3 city = float3( 1.0, 0.82, 0.54 ) * glow * 1.5;

	Out.color0 = float4( saturate( color.rgb * sunShade + city ), 1.0 );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////