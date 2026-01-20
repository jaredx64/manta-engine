#include <shader_api.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

vertex_input BuiltinVertexPositionColor
{
	float3 position packed_as( FLOAT32 );
	float4 color packed_as( UNORM8 );
};

vertex_output VertexOutput
{
	float4 position position_out;
	float4 color;
};

fragment_input FragmentInput
{
	float4 position position_in;
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void vertex_main( BuiltinVertexPositionColor In, VertexOutput Out, UniformsPipeline Pipeline )
{
	Out.position = mul( Pipeline.matrixMVP, float4( In.position, 1.0 ) );
	Out.color = In.color;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void fragment_main( FragmentInput In, FragmentOutput Out )
{
	Out.color = In.color;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////