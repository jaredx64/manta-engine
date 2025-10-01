#include <shader_api.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

vertex_input BuiltinVertexPositionColor
{
	float3 position semantic( POSITION ) format( FLOAT32 );
	float4 color semantic( COLOR ) format( UNORM8 );
};

vertex_output VertexOutput
{
	float4 position semantic( POSITION );
	float4 color semantic( COLOR );
};

fragment_input FragmentInput
{
	float4 position semantic( POSITION );
	float4 color semantic( COLOR );
};

fragment_output FragmentOutput
{
	float4 color0 semantic( COLOR ) target( 0 );
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

uniform_buffer( 0 ) PipelineUniforms
{
	float4x4 matrixModel;
	float4x4 matrixView;
	float4x4 matrixPerspective;
	float4x4 matrixMVP;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void vertex_main( BuiltinVertexPositionColor In, VertexOutput Out, PipelineUniforms Pipeline )
{
	Out.position = mul( Pipeline.matrixMVP, float4( In.position, 1.0 ) );
	Out.color = In.color;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void fragment_main( FragmentInput In, FragmentOutput Out )
{
	Out.color0 = In.color;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////