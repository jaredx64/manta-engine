#include <core/color.hpp>

#include <core/math.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static constexpr float inv255 = 1.0f / 255.0f;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Color Color::operator*( const Color &c )
{
	Color outColor = *this;
	if( c.r == 255 && c.g == 255 && c.b == 255 && c.a == 255 ) { return outColor; }
	outColor.r = static_cast<u8>( clamp( ( r * inv255 ) * ( c.r * inv255 ), 0.0f, 1.0f ) * 255.0f );
	outColor.g = static_cast<u8>( clamp( ( g * inv255 ) * ( c.g * inv255 ), 0.0f, 1.0f ) * 255.0f );
	outColor.b = static_cast<u8>( clamp( ( b * inv255 ) * ( c.b * inv255 ), 0.0f, 1.0f ) * 255.0f );
	outColor.a = static_cast<u8>( clamp( ( a * inv255 ) * ( c.a * inv255 ), 0.0f, 1.0f ) * 255.0f );
	return outColor;
}


Color Color::operator*( Color &c )
{
	Color outColor = *this;
	if( c.r == 255 && c.g == 255 && c.b == 255 && c.a == 255 ) { return outColor; }
	outColor.r = static_cast<u8>( clamp( ( r * inv255 ) * ( c.r * inv255 ), 0.0f, 1.0f ) * 255.0f );
	outColor.g = static_cast<u8>( clamp( ( g * inv255 ) * ( c.g * inv255 ), 0.0f, 1.0f ) * 255.0f );
	outColor.b = static_cast<u8>( clamp( ( b * inv255 ) * ( c.b * inv255 ), 0.0f, 1.0f ) * 255.0f );
	outColor.a = static_cast<u8>( clamp( ( a * inv255 ) * ( c.a * inv255 ), 0.0f, 1.0f ) * 255.0f );
	return outColor;
}


Color Color::operator*( const float s )
{
	Color outColor;
	outColor.r = static_cast<u8>( clamp( ( r * inv255 ) * s, 0.0f, 1.0f ) * 255.0f );
	outColor.g = static_cast<u8>( clamp( ( g * inv255 ) * s, 0.0f, 1.0f ) * 255.0f );
	outColor.b = static_cast<u8>( clamp( ( b * inv255 ) * s, 0.0f, 1.0f ) * 255.0f );
	outColor.a = static_cast<u8>( clamp( ( a * inv255 ) * s, 0.0f, 1.0f ) * 255.0f );
	return outColor;
}


Color Color::operator*=( const Color &c )
{
	if( c.r == 255 && c.g == 255 && c.b == 255 && c.a == 255 ) { return *this; }
	r = static_cast<u8>( clamp( ( r * inv255 ) * ( c.r * inv255 ), 0.0f, 1.0f ) * 255.0f );
	g = static_cast<u8>( clamp( ( g * inv255 ) * ( c.g * inv255 ), 0.0f, 1.0f ) * 255.0f );
	b = static_cast<u8>( clamp( ( b * inv255 ) * ( c.b * inv255 ), 0.0f, 1.0f ) * 255.0f );
	a = static_cast<u8>( clamp( ( a * inv255 ) * ( c.a * inv255 ), 0.0f, 1.0f ) * 255.0f );
	return *this;
}


Color Color::operator*=( Color &c )
{
	if( c.r == 255 && c.g == 255 && c.b == 255 && c.a == 255 ) { return *this; }
	r = static_cast<u8>( clamp( ( r * inv255 ) * ( c.r * inv255 ), 0.0f, 1.0f ) * 255.0f );
	g = static_cast<u8>( clamp( ( g * inv255 ) * ( c.g * inv255 ), 0.0f, 1.0f ) * 255.0f );
	b = static_cast<u8>( clamp( ( b * inv255 ) * ( c.b * inv255 ), 0.0f, 1.0f ) * 255.0f );
	a = static_cast<u8>( clamp( ( a * inv255 ) * ( c.a * inv255 ), 0.0f, 1.0f ) * 255.0f );
	return *this;
}


Color Color::operator*=( const float s )
{
	r = static_cast<u8>( clamp( ( r * inv255 ) * s, 0.0f, 1.0f ) * 255.0f );
	g = static_cast<u8>( clamp( ( g * inv255 ) * s, 0.0f, 1.0f ) * 255.0f );
	b = static_cast<u8>( clamp( ( b * inv255 ) * s, 0.0f, 1.0f ) * 255.0f );
	a = static_cast<u8>( clamp( ( a * inv255 ) * s, 0.0f, 1.0f ) * 255.0f );
	return *this;
}


Color Color::operator/( const Color &c )
{
	Color outColor = *this;
	if( c.r == 255 && c.g == 255 && c.b == 255 && c.a == 255 ) { return outColor; }
	outColor.r = static_cast<u8>( clamp( ( r * inv255 ) / ( c.r * inv255 ), 0.0f, 1.0f ) * 255.0f );
	outColor.g = static_cast<u8>( clamp( ( g * inv255 ) / ( c.g * inv255 ), 0.0f, 1.0f ) * 255.0f );
	outColor.b = static_cast<u8>( clamp( ( b * inv255 ) / ( c.b * inv255 ), 0.0f, 1.0f ) * 255.0f );
	outColor.a = static_cast<u8>( clamp( ( a * inv255 ) / ( c.a * inv255 ), 0.0f, 1.0f ) * 255.0f );
	return outColor;
}


Color Color::operator/( Color &c )
{
	Color outColor = *this;
	if( c.r == 255 && c.g == 255 && c.b == 255 && c.a == 255 ) { return outColor; }
	outColor.r = static_cast<u8>( clamp( ( r * inv255 ) / ( c.r * inv255 ), 0.0f, 1.0f ) * 255.0f );
	outColor.g = static_cast<u8>( clamp( ( g * inv255 ) / ( c.g * inv255 ), 0.0f, 1.0f ) * 255.0f );
	outColor.b = static_cast<u8>( clamp( ( b * inv255 ) / ( c.b * inv255 ), 0.0f, 1.0f ) * 255.0f );
	outColor.a = static_cast<u8>( clamp( ( a * inv255 ) / ( c.a * inv255 ), 0.0f, 1.0f ) * 255.0f );
	return outColor;
}


Color Color::operator/( const float s )
{
	Color outColor;
	outColor.r = static_cast<u8>( clamp( ( r * inv255 ) / s, 0.0f, 1.0f ) * 255.0f );
	outColor.g = static_cast<u8>( clamp( ( g * inv255 ) / s, 0.0f, 1.0f ) * 255.0f );
	outColor.b = static_cast<u8>( clamp( ( b * inv255 ) / s, 0.0f, 1.0f ) * 255.0f );
	outColor.a = static_cast<u8>( clamp( ( a * inv255 ) / s, 0.0f, 1.0f ) * 255.0f );
	return outColor;
}


Color Color::operator/=( const Color &c )
{
	if( c.r == 255 && c.g == 255 && c.b == 255 && c.a == 255 ) { return *this; }
	r = static_cast<u8>( clamp( ( r * inv255 ) / ( c.r * inv255 ), 0.0f, 1.0f ) * 255.0f );
	g = static_cast<u8>( clamp( ( g * inv255 ) / ( c.g * inv255 ), 0.0f, 1.0f ) * 255.0f );
	b = static_cast<u8>( clamp( ( b * inv255 ) / ( c.b * inv255 ), 0.0f, 1.0f ) * 255.0f );
	a = static_cast<u8>( clamp( ( a * inv255 ) / ( c.a * inv255 ), 0.0f, 1.0f ) * 255.0f );
	return *this;
}


Color Color::operator/=( Color &c )
{
	if( c.r == 255 && c.g == 255 && c.b == 255 && c.a == 255 ) { return *this; }
	r = static_cast<u8>( clamp( ( r * inv255 ) / ( c.r * inv255 ), 0.0f, 1.0f ) * 255.0f );
	g = static_cast<u8>( clamp( ( g * inv255 ) / ( c.g * inv255 ), 0.0f, 1.0f ) * 255.0f );
	b = static_cast<u8>( clamp( ( b * inv255 ) / ( c.b * inv255 ), 0.0f, 1.0f ) * 255.0f );
	a = static_cast<u8>( clamp( ( a * inv255 ) / ( c.a * inv255 ), 0.0f, 1.0f ) * 255.0f );
	return *this;
}


Color Color::operator/=( const float s )
{
	r = static_cast<u8>( clamp( ( r * inv255 ) / s, 0.0f, 1.0f ) * 255.0f );
	g = static_cast<u8>( clamp( ( g * inv255 ) / s, 0.0f, 1.0f ) * 255.0f );
	b = static_cast<u8>( clamp( ( b * inv255 ) / s, 0.0f, 1.0f ) * 255.0f );
	a = static_cast<u8>( clamp( ( a * inv255 ) / s, 0.0f, 1.0f ) * 255.0f );
	return *this;
}


Color Color::operator+( const Color &c )
{
	Color outColor;
	outColor.r = static_cast<u8>( clamp( r + c.r, 0, 255 ) );
	outColor.g = static_cast<u8>( clamp( g + c.g, 0, 255 ) );
	outColor.b = static_cast<u8>( clamp( b + c.b, 0, 255 ) );
	outColor.a = static_cast<u8>( clamp( a + c.a, 0, 255 ) );
	return outColor;
}


Color Color::operator+( Color &c )
{
	Color outColor;
	outColor.r = static_cast<u8>( clamp( r + c.r, 0, 255 ) );
	outColor.g = static_cast<u8>( clamp( g + c.g, 0, 255 ) );
	outColor.b = static_cast<u8>( clamp( b + c.b, 0, 255 ) );
	outColor.a = static_cast<u8>( clamp( a + c.a, 0, 255 ) );
	return outColor;
}


Color Color::operator+=( const Color &c )
{
	r = static_cast<u8>( clamp( r + c.r, 0, 255 ) );
	g = static_cast<u8>( clamp( g + c.g, 0, 255 ) );
	b = static_cast<u8>( clamp( b + c.b, 0, 255 ) );
	a = static_cast<u8>( clamp( a + c.a, 0, 255 ) );
	return *this;
}


Color Color::operator+=( Color &c )
{
	r = static_cast<u8>( clamp( r + c.r, 0, 255 ) );
	g = static_cast<u8>( clamp( g + c.g, 0, 255 ) );
	b = static_cast<u8>( clamp( b + c.b, 0, 255 ) );
	a = static_cast<u8>( clamp( a + c.a, 0, 255 ) );
	return *this;
}


Color Color::operator-( const Color &c )
{
	Color outColor;
	outColor.r = static_cast<u8>( clamp( r - c.r, 0, 255 ) );
	outColor.g = static_cast<u8>( clamp( g - c.g, 0, 255 ) );
	outColor.b = static_cast<u8>( clamp( b - c.b, 0, 255 ) );
	outColor.a = static_cast<u8>( clamp( a - c.a, 0, 255 ) );
	return outColor;
}


Color Color::operator-( Color &c )
{
	Color outColor;
	outColor.r = static_cast<u8>( clamp( r - c.r, 0, 255 ) );
	outColor.g = static_cast<u8>( clamp( g - c.g, 0, 255 ) );
	outColor.b = static_cast<u8>( clamp( b - c.b, 0, 255 ) );
	outColor.a = static_cast<u8>( clamp( a - c.a, 0, 255 ) );
	return outColor;
}


Color Color::operator-=( const Color &c )
{
	r = static_cast<u8>( clamp( r - c.r, 0, 255 ) );
	g = static_cast<u8>( clamp( g - c.g, 0, 255 ) );
	b = static_cast<u8>( clamp( b - c.b, 0, 255 ) );
	a = static_cast<u8>( clamp( a - c.a, 0, 255 ) );
	return *this;
}


Color Color::operator-=( Color &c )
{
	r = static_cast<u8>( clamp( r - c.r, 0, 255 ) );
	g = static_cast<u8>( clamp( g - c.g, 0, 255 ) );
	b = static_cast<u8>( clamp( b - c.b, 0, 255 ) );
	a = static_cast<u8>( clamp( a - c.a, 0, 255 ) );
	return *this;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ColorWeight::clear()
{
	r = 0.0f;
	g = 0.0f;
	b = 0.0f;
	a = 0.0f;
	w = 0.0f;
}


Color ColorWeight::color()
{
	if( w == 0.0 ) { return Color { 0, 0, 0, 0 }; }
	const u8 rR = static_cast<u8>( clamp( r / w, 0.0f, 255.0f ) );
	const u8 rG = static_cast<u8>( clamp( g / w, 0.0f, 255.0f ) );
	const u8 rB = static_cast<u8>( clamp( b / w, 0.0f, 255.0f ) );
	const u8 rA = static_cast<u8>( clamp( w > 1.0f ? a / w : a, 0.0f, 255.0f ) );
	return Color { rR, rG, rB, rA };
}


void ColorWeight::add( const Color color, const float weight )
{
	float alphaWeight = ( color.a / 255.0f ) * weight;
	r += static_cast<float>( color.r * alphaWeight );
	g += static_cast<float>( color.g * alphaWeight );
	b += static_cast<float>( color.b * alphaWeight );
	a += static_cast<float>( color.a * alphaWeight );
	w += alphaWeight;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Color color_hsv_to_rgb( const Color &hsv )
{
    const float h = hsv.r / 255.0f * 360.0f; // Hue: 0.0f - 360.0f degrees
    const float s = hsv.g / 255.0f; // Saturation: 0.0f - 1.0f
    const float v = hsv.b / 255.0f; // Value: 0.0f - 1.0f

    const float c = v * s; // Chroma
    const float x = c * ( 1.0f - fabsf( static_cast<float>( fmod( h / 60.0, 2.0 ) ) - 1.0f ) );
    const float m = v - c;

    float r, g, b;
    if( h >= 0.0f && h < 60.0f )
	{
        r = c; g = x; b = 0.0f;
    }
	else if( h >= 60.0f && h < 120.0f )
	{
        r = x; g = c; b = 0.0f;
    }
	else if( h >= 120.0f && h < 180.0f )
	{
        r = 0.0f; g = c; b = x;
    }
	else if( h >= 180.0f && h < 240.0f )
	{
        r = 0.0f; g = x; b = c;
    }
	else if( h >= 240.0f && h < 300.0f )
	{
        r = x; g = 0.0f; b = c;
    }
	else
	{
        r = c; g = 0.0f; b = x;
    }

    return Color { static_cast<u8>( ( r + m ) * 255 ),
	               static_cast<u8>( ( g + m ) * 255 ),
	               static_cast<u8>( ( b + m ) * 255 ),
	               hsv.a };
}


Color color_rgb_to_hsv( const Color &rgb )
{
    const float r = rgb.r / 255.0f;
    const float g = rgb.g / 255.0f;
    const float b = rgb.b / 255.0f;

    const float cmax = max( r, max( g, b ) );
    const float cmin = min( r, min( g, b ) );
    const float delta = cmax - cmin;

    const float s = ( cmax == 0.0f ) ? 0.0f : ( delta / cmax );
    const float v = cmax;
    float h;

    if( delta == 0 )
	{
        h = 0.0f;
    }
	else if( cmax == r )
	{
        h = 60.0f * static_cast<float>( fmod( ( g - b ) / delta, 6.0 ) );
        if( h < 0.0f ) { h += 360.0f; }
    }
	else if( cmax == g )
	{
        h = 60.0f * ( ( b - r ) / delta + 2.0f );
    }
	else
	{
        h = 60.0f * ( ( r - g ) / delta + 4.0f );
    }

    return Color { static_cast<u8>( h / 360.0f * 255.0f ),
	               static_cast<u8>( s * 255.0f ),
	               static_cast<u8>( v * 255.0f ),
	               rgb.a };
}
