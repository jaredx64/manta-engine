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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void vertex_main( BuiltinVertex In, VertexOutput Out, UniformsPipeline Pipeline )
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
    Out.color = float4( hsv_to_rgb( In.color.r, In.color.g, In.color.b ), In.color.a );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////