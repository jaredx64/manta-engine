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

void vertex_main( BuiltinVertex In, VertexOutput Out, PipelineUniforms Pipeline )
{
	Out.position = mul( Pipeline.matrixMVP, float4( In.position, 1.0 ) );
	Out.uv = In.uv;
	Out.color = In.color;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float3 hsv_to_rgb( float h, float s, float v )
{
    float c = v * s; // Chroma
    float x = c * ( 1.0 - abs( mod( h * 6.0, 2.0 ) - 1.0 ) );
    float m = v - c;

    float3 rgb = float3( 0.0, 0.0, 0.0 );
    if( h < 1.0 / 6.0 ) { rgb = float3( c, x, 0.0 ); } else
    if( h < 2.0 / 6.0 ) { rgb = float3( x, c, 0.0 ); } else
    if( h < 3.0 / 6.0 ) { rgb = float3( 0.0, c, x ); } else
	if( h < 4.0 / 6.0 ) { rgb = float3( 0.0, x, c ); } else
    if( h < 5.0 / 6.0 ) { rgb = float3( x, 0.0, c ); } else
     	                { rgb = float3( c, 0.0, x ); }

    return rgb + float3( m, m, m );
}


void fragment_main( FragmentInput In, FragmentOutput Out )
{
    Out.color0 = float4( hsv_to_rgb( In.color.r, In.color.g, In.color.b ), In.color.a );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////