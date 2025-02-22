#include <shader_api.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

vertex_input BuiltinVertex
{
	float3 position semantic( POSITION ) format( FLOAT32 );
	float2 uv semantic( TEXCOORD ) format( UNORM16 );
	float4 color semantic( COLOR ) format( UNORM8 );
};

vertex_output VertexOutput
{
	float4 position semantic( POSITION );
	float2 uv semantic( TEXCOORD );
	float4 color semantic( COLOR );
};

fragment_input FragmentInput
{
	float4 position semantic( POSITION );
	float2 uv semantic( TEXCOORD );
	float4 color semantic( COLOR );
};

fragment_output FragmentOutput
{
	float depth semantic( DEPTH );
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

cbuffer( 0 ) ShaderGlobals
{
	float4x4 matrixModel;
	float4x4 matrixView;
	float4x4 matrixPerspective;
	float4x4 matrixMVP;
};

texture2D( float4, 0 ) textureDepth;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void vertex_main( BuiltinVertex In, VertexOutput Out, ShaderGlobals globals )
{
	Out.position = mul( globals.matrixMVP, float4( In.position, 1.0 ) );
	Out.uv = In.uv;
	Out.color = In.color;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void fragment_main( FragmentInput In, FragmentOutput Out )
{
	Out.depth = texture_sample_2d( textureDepth, In.uv ).r;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////