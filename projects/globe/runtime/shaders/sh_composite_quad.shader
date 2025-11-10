#include <shader_api.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

vertex_input VertexGrade
{
	// Empty...
};

vertex_output VertexOutput
{
	float4 position position_out;
	float2 uv;
};

fragment_input FragmentInput
{
	float4 position position_in;
	float2 uv;
};

fragment_output FragmentOutput
{
	float4 color0 target( 0, COLOR );
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

texture2D( 0, float4 ) texture;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void vertex_main( VertexGrade In, VertexOutput Out, UniformsPipeline Pipeline )
{
	float2 positions[4];
	positions[0] = float2( -1.0,  1.0 );
	positions[1] = float2(  1.0,  1.0 );
	positions[2] = float2( -1.0, -1.0 );
	positions[3] = float2(  1.0, -1.0 );

	Out.position = float4( positions[SV_VertexID], 0.0, 1.0 );

	float2 uvs[4];
	uvs[0] = float2( 0.0, 0.0 );
	uvs[1] = float2( 1.0, 0.0 );
	uvs[2] = float2( 0.0, 1.0 );
	uvs[3] = float2( 1.0, 1.0 );

	Out.uv = uvs[SV_VertexID];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void fragment_main( FragmentInput In, FragmentOutput Out )
{
	Out.color0 = texture_sample_2d( texture, In.uv );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////