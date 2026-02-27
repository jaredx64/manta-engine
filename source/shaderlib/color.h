#pragma once

#include <shader_api.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// NOTE: Meant to be included in *.shader files. Supplies common built-in color macros and helper functions.

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define COLOR_RED float4( 1.0, 0.0, 0.0, 1.0 )
#define COLOR_GREEN float4( 0.0, 1.0, 0.0, 1.0 )
#define COLOR_BLUE float4( 0.0, 0.0, 1.0, 1.0 )
#define COLOR_WHITE float4( 1.0, 1.0, 1.0, 1.0 )
#define COLOR_BLACK float4( 0.0, 0.0, 0.0, 1.0 )
#define COLOR_GRAY float4( 0.5, 0.5, 0.5, 1.0 )
#define COLOR_LIGHT_GRAY float4( 0.75, 0.75, 0.75, 1.0 )
#define COLOR_DARK_GRAY float4( 0.25, 0.25, 0.25, 1.0 )
#define COLOR_YELLOW float4( 1.0, 1.0, 0.0, 1.0 )
#define COLOR_CYAN float4( 0.0, 1.0, 1.0, 1.0 )
#define COLOR_MAGENTA float4( 1.0, 0.0, 1.0, 1.0 )
#define COLOR_ORANGE float4( 1.0, 0.5, 0.0, 1.0 )
#define COLOR_PURPLE float4( 0.5, 0.0, 0.5, 1.0 )
#define COLOR_PINK float4( 1.0, 0.4, 0.7, 1.0 )
#define COLOR_LIME float4( 0.5, 1.0, 0.0, 1.0 )
#define COLOR_TEAL float4( 0.0, 0.5, 0.5, 1.0 )
#define COLOR_NAVY float4( 0.0, 0.0, 0.5, 1.0 )
#define COLOR_MAROON float4( 0.5, 0.0, 0.0, 1.0 )
#define COLOR_OLIVE float4( 0.5, 0.5, 0.0, 1.0 )
#define COLOR_BROWN float4( 0.6, 0.3, 0.1, 1.0 )
#define COLOR_GOLD float4( 1.0, 0.84, 0.0, 1.0 )
#define COLOR_SILVER float4( 0.75, 0.75, 0.75, 1.0 )
#define COLOR_SKY_BLUE float4( 0.4, 0.7, 1.0, 1.0 )
#define COLOR_TURQUOISE float4( 0.25, 0.88, 0.82, 1.0 )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float rgb_get_luminance( float3 colorRGB )
{
	return dot( colorRGB, float3( 0.2126, 0.7152, 0.0722 ) );
}


float3 rgb_adjust_saturation( float3 colorRGB, float saturation )
{
	float luminance = rgb_get_luminance( colorRGB );
	return lerp( float3( luminance, luminance, luminance ), colorRGB, saturation );
}


float3 rgb_to_srgb( float3 colorRGB )
{
	float3 lo = 12.92 * colorRGB;
	float3 hi = 1.055 * pow( colorRGB, float3( 1.0 / 2.4, 1.0 / 2.4, 1.0 / 2.4 ) ) - 0.055;
	return lerp( lo, hi, step( float3( 0.0031308, 0.0031308, 0.0031308 ), colorRGB ) );
}


float3 rgb_to_hsv( float3 colorRGB )
{
	float maxComponent = max( colorRGB.r, max( colorRGB.g, colorRGB.b ) );
	float minComponent = min( colorRGB.r, min( colorRGB.g, colorRGB.b ) );
	float delta = maxComponent - minComponent;

	float h = 0.0;
	if( delta > 0.0 )
	{
		if( maxComponent == colorRGB.r )
		{
			h = mod((colorRGB.g - colorRGB.b) / delta, 6.0);
		}
		else if( maxComponent == colorRGB.g )
		{
			h = ( colorRGB.b - colorRGB.r ) / delta + 2.0;
		}
		else
		{
			h = ( colorRGB.r - colorRGB.g ) / delta + 4.0;
		}

		h /= 6.0;
		if( h < 0.0 ) { h += 1.0; }
	}

	float s = ( maxComponent == 0.0 ) ? 0.0 : delta / maxComponent;
	float v = maxComponent;

	return float3( h, s, v );
}


float3 hsv_to_rgb( float3 colorHSV )
{
	float3 p = abs( frac( colorHSV.xxx + float3( 1.0, 2.0 / 3.0, 1.0 / 3.0 ) ) * 6.0 - 3.0 );
	return lerp( float3( 1.0, 1.0, 1.0 ), clamp( p - 1.0, 0.0, 1.0 ), colorHSV.y ) * colorHSV.z;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////