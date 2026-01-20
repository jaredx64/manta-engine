#include <shader_api.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

vertex_input BuiltinVertex
{
	float3 position packed_as( FLOAT32 );
	float2 uv packed_as( UNORM16 );
	float4 color packed_as( UNORM8 );
};

vertex_output VertexOutput
{
	float4 position position_out;
	float2 uv;
	float4 color;
};

fragment_input FragmentInput
{
	float4 position position_in;
	float2 uv;
	float4 color;
};

fragment_output FragmentOutput
{
	float4 color target( 0, COLOR );
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

texture2D( 0, float4 ) textureColor;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void vertex_main( BuiltinVertex In, VertexOutput Out, UniformsPipeline Pipeline )
{
	Out.position = mul( Pipeline.matrixMVP, float4( In.position, 1.0 ) );
	Out.uv = In.uv;
	Out.color = In.color;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void fragment_main( FragmentInput In, FragmentOutput Out )
{
	float4 colorTexture = texture_sample_2d( textureColor, In.uv );
	if( colorTexture.a <= 0.0 ) { discard; }
	Out.color = colorTexture * In.color;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////