#include <shader_api.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define R_EARTH ( 6378137.0 )
#define R_EARTH_SQR ( R_EARTH * R_EARTH )
#define R_ATMOSPHERE ( 1000.0 * 200.0 )
#define R_ATMOSPHERE_SQR ( R_ATMOSPHERE * R_ATMOSPHERE )

#define PI ( 3.141592654 )
#define PI_INV ( 0.31830988614 )

#define NUM_STEPS ( 64 )
#define STEP_SIZE ( 8000.0 * ( 128.0 / NUM_STEPS ) )
#define ABSORPTION_AIR ( ( 1.0 / 48.0 ) * ( 128.0 / NUM_STEPS ) )
#define ABSORPTION_CLOUD ( ( 1.0 / 3.0 ) * ( 128.0 / NUM_STEPS ) )

#define COLOR_ATMOSPHERE_DAY ( float4( 0.75, 0.75, 0.97, 1.00 ) )
#define COLOR_ATMOSPHERE_NIGHT ( float4( 0.05, 0.05, 0.1, 1.00 ) )
#define COLOR_CLOUDS_DAY ( float3( 1.5, 1.5, 1.5 ) )
#define COLOR_CLOUDS_NIGHT ( float3( 0.2, 0.2, 0.2 ) )
#define COLOR_AURORA_G ( float3( 0.00, 2.25, 0.00 ) )
#define COLOR_AURORA_R ( float3( 2.25, 0.75, 0.00 ) )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

vertex_input VertexAtmosphere
{
	float3 position packed_as( FLOAT32 );
};

vertex_output VertexOutput
{
	float4 position position_out;
	float3 start;
};

fragment_input FragmentInput
{
	float4 position position_in;
	float3 start;
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

uniform_buffer( 1 ) UniformsAtmosphere
{
	float4x4 matrixLookInverse;
	float3 eye;
	float near;
	float3 forward;
	float far;
	float3 sun;
	float time;
	float2 viewport;
	int cloudsMip;
};

texture2D( 0, float4 ) textureAurora;

#ifdef CLOUDS
texture2D( 1, float4 ) textureLightHeightCloud;
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void vertex_main( VertexAtmosphere In, VertexOutput Out, UniformsPipeline Pipeline, UniformsAtmosphere Uniforms )
{
	Out.position = mul( Pipeline.matrixMVP, float4( In.position, 1.0 ) );
	Out.start =  mul( Pipeline.matrixModel, float4( In.position, 1.0 ) ).xyz;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float length_sqr( float3 v )
{
	return ( v.x * v.x ) + ( v.y * v.y ) + ( v.z * v.z );
}


float3 ecef_to_uv_altitude( float3 pos )
{
	float3 n = normalize( pos );
	float u = atan2( n.y, n.x ) * ( 0.5 * PI_INV ) + 0.5;
	float v = 0.5 - ( asin( n.z ) * PI_INV );
	return float3( 1.0 - u, v, max( length( pos ) - R_EARTH, 0.0 ) );
}


float3 screen_uv_to_world_ray( float2 uv, UniformsPipeline Pipeline, UniformsAtmosphere Uniforms )
{
	float xNDC = uv.x * 2.0 - 1.0;
	float yNDC = uv.y * 2.0 - 1.0;

	float4 pClip = float4( xNDC, yNDC, -1.0, 1.0 );
	float4 pView = mul( Pipeline.matrixPerspectiveInverse, pClip );
	float4 pWorld = mul( Uniforms.matrixLookInverse, pView );

	return normalize( pWorld.xyz );
}


struct Absorption
{
	float air;
#ifdef CLOUDS
	float cloud;
#endif
};


Absorption calculate_absorption_coefficients( float3 point, float fraction )
{
	float elevation = clamp( length( point ) - R_EARTH, 0.0, R_ATMOSPHERE );
	Absorption absorption;

	float falloffAir = clamp( 1.0 - ( elevation / R_ATMOSPHERE ), 0.0, 1.0 );
	absorption.air = pow( falloffAir, 3.0 ) * ABSORPTION_AIR * fraction;

#ifdef CLOUDS
	float falloffCloud = clamp( 1.0 - ( elevation / ( R_ATMOSPHERE * 0.333 ) ), 0.0, 1.0 );
	absorption.cloud = pow( falloffCloud, 2.0 ) * ABSORPTION_CLOUD * fraction;
#endif

	return absorption;
}


float4 integrate_atmosphere( float3 start, float3 forward,
	UniformsPipeline Pipeline, UniformsAtmosphere Uniforms )
{
	float3 colorAtmosphereDay = COLOR_ATMOSPHERE_DAY.xyz;
	float3 colorAtmosphereNight = COLOR_ATMOSPHERE_NIGHT.xyz;
	float3 colorFinal = float3( 0.0, 0.0, 0.0 );
	float alphaFinal = 0.0;

	// Aurora Noise (Animated)
	float2 auroraUV = ecef_to_uv_altitude( start ).xy;
	float2 auroraNoiseUV = auroraUV;
	auroraNoiseUV.y = mod( auroraNoiseUV.y + Uniforms.time, 1.0 );
	float auroraNoise = texture_sample_2d( textureAurora, auroraNoiseUV ).y;
	float2 auroraColorUV = auroraUV;
	auroraColorUV.x = mod( auroraColorUV.x - Uniforms.time, 1.0 );
	auroraColorUV.y = mod( auroraColorUV.y - Uniforms.time, 1.0 );
	float auroraColor = auroraNoise * pow( texture_sample_2d( textureAurora, auroraColorUV ).y, 1.0 );

	// Atmosphere Raymarch
	float3 sample = start;
	for( int i = 0; i < NUM_STEPS; i++ )
	{
		float distancePreviousSqr = length_sqr( sample );
		sample += forward * STEP_SIZE;
		float3 sampleUVAltitude = ecef_to_uv_altitude( sample );
		float distanceCurrentSqr = length_sqr( sample );
		float sampleFraction = saturate( ( distancePreviousSqr - R_EARTH_SQR ) /
			( distancePreviousSqr - distanceCurrentSqr ) );

		Absorption absorption = calculate_absorption_coefficients( sample, sampleFraction );

		float sampleDotSun = dot( normalize( sample ), Uniforms.sun );
		float nightness = 1.0 - saturate( ( sampleDotSun + 0.05 ) / 0.2 );

		// Atmosphere
		float3 colorAtmosphere = lerp( colorAtmosphereDay, colorAtmosphereNight, nightness );
		colorFinal += colorAtmosphere * absorption.air * ( 1.0 - nightness * 0.33 );
		alphaFinal = min( 1.0, alphaFinal + absorption.air * ( 1.0 - nightness * 0.33 ) );

#ifdef CLOUDS
		// int_v2
		float cloudStrength = texture_sample_2d_level( textureLightHeightCloud,
			sampleUVAltitude.xy, Uniforms.cloudsMip ).z;
		if( cloudStrength > 0.0 )
		{
			float3 colorCloud = lerp( COLOR_CLOUDS_DAY, COLOR_CLOUDS_NIGHT, nightness );
			absorption.cloud *= cloudStrength;
			colorFinal = lerp( colorFinal, colorCloud, absorption.cloud * ( 1.0 - alphaFinal ) );
			alphaFinal += absorption.cloud * ( 1.0 - alphaFinal );
		}
#endif

		// Aurora
		float auroraStrength = texture_sample_2d( textureAurora, sampleUVAltitude.xy ).x;
		if( auroraStrength > 0.0 )
		{
			float3 colorAurora = lerp( COLOR_AURORA_G, COLOR_AURORA_R, auroraColor );
			auroraStrength = auroraStrength + ( auroraStrength * ( auroraNoise - 0.5 ) );

			absorption.air *= pow( auroraStrength, 2.0 ) * ( 0.25 + ( 1.75 * nightness ) );
			colorFinal = lerp( colorFinal, colorAurora, absorption.air * ( 1.0 - alphaFinal ) );
			alphaFinal += absorption.air * ( 1.0 - alphaFinal );
		}

		if( length_sqr( sample ) < R_EARTH * R_EARTH ) { break; }
	}

	return float4( colorFinal, alphaFinal );
}


void fragment_main( FragmentInput In, FragmentOutput Out, UniformsPipeline Pipeline, UniformsAtmosphere Uniforms )
{
#if SHADER_GLSL
	float2 uv = float2( In.position.x / Uniforms.viewport.x, In.position.y / Uniforms.viewport.y );
#else
	float2 uv = float2( In.position.x / Uniforms.viewport.x, 1.0 - In.position.y / Uniforms.viewport.y );
#endif
	float3 direction = screen_uv_to_world_ray( uv, Pipeline, Uniforms );
	float4 color = integrate_atmosphere( In.start, direction, Pipeline, Uniforms );
	Out.color0 = float4( color.rgb, color.a );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////