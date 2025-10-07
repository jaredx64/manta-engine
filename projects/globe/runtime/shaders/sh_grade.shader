#include <shader_api.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

vertex_input VertexGrade
{
	// Empty...
};

vertex_output VertexOutput
{
	float4 position semantic( POSITION );
	float2 uv semantic( TEXCOORD );
};

fragment_input FragmentInput
{
	float4 position semantic( POSITION );
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
	float4 color = texture_sample_2d( texture, In.uv );
#if 0
	float contrast = 1.05;
	color.rgb = ( color.rgb - 0.5 ) * contrast + 0.5;
	float saturation = 1.05;
	float luminance = dot( color.rgb, float3( 0.299, 0.587, 0.114 ) );
	color.rgb = lerp( float3( luminance, luminance, luminance ), color.rgb, saturation );
#endif
	Out.color0 = color;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////