#include <core/color.hpp>

#include <core/math.hpp>
#include <core/memory.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Color::Color( const u8_v4 &rgba ) :
	r { rgba.x },
	g { rgba.y },
	b { rgba.z },
	a { rgba.w } { }


Color::Color( const u8_v3 &rgb ) :
	r { rgb.x },
	g { rgb.y },
	b { rgb.z },
	a { 255U } { }


Color::Color( const int_v4 &rgba ) :
	r { static_cast<u8>( rgba.x ) },
	g { static_cast<u8>( rgba.y ) },
	b { static_cast<u8>( rgba.z ) },
	a { static_cast<u8>( rgba.w ) } { }


Color::Color( const int_v3 &rgb ) :
	r { static_cast<u8>( rgb.x ) },
	g { static_cast<u8>( rgb.y ) },
	b { static_cast<u8>( rgb.z ) },
	a { 255U } { }


Color::Color( const float_v4 &rgba ) :
	r { static_cast<u8>( clamp( rgba.x * 255.0f, 0.0f, 255.0f ) ) },
	g { static_cast<u8>( clamp( rgba.y * 255.0f, 0.0f, 255.0f ) ) },
	b { static_cast<u8>( clamp( rgba.z * 255.0f, 0.0f, 255.0f ) ) },
	a { static_cast<u8>( clamp( rgba.w * 255.0f, 0.0f, 255.0f ) ) } { }


Color::Color( const float_v3 &rgb ) :
	r { static_cast<u8>( clamp( rgb.x * 255.0f, 0.0f, 255.0f ) ) },
	g { static_cast<u8>( clamp( rgb.y * 255.0f, 0.0f, 255.0f ) ) },
	b { static_cast<u8>( clamp( rgb.z * 255.0f, 0.0f, 255.0f ) ) },
	a { 255U } { }


Color::Color( const double_v4 &rgba ) :
	r { static_cast<u8>( clamp( rgba.x * 255.0, 0.0, 255.0 ) ) },
	g { static_cast<u8>( clamp( rgba.y * 255.0, 0.0, 255.0 ) ) },
	b { static_cast<u8>( clamp( rgba.z * 255.0, 0.0, 255.0 ) ) },
	a { static_cast<u8>( clamp( rgba.w * 255.0, 0.0, 255.0 ) ) } { }


Color::Color( const double_v3 &rgb ) :
	r { static_cast<u8>( clamp( rgb.x * 255.0, 0.0, 255.0 ) ) },
	g { static_cast<u8>( clamp( rgb.y * 255.0, 0.0, 255.0 ) ) },
	b { static_cast<u8>( clamp( rgb.z * 255.0, 0.0, 255.0 ) ) },
	a { 255U } { }


Color Color::operator*( const Color &c ) const
{
	Color outColor = *this;
	if( c.r == 255 && c.g == 255 && c.b == 255 && c.a == 255 ) { return outColor; }
	outColor.r = static_cast<u8>( clamp( ( r * INV_255_F ) * ( c.r * INV_255_F ), 0.0f, 1.0f ) * 255.0f );
	outColor.g = static_cast<u8>( clamp( ( g * INV_255_F ) * ( c.g * INV_255_F ), 0.0f, 1.0f ) * 255.0f );
	outColor.b = static_cast<u8>( clamp( ( b * INV_255_F ) * ( c.b * INV_255_F ), 0.0f, 1.0f ) * 255.0f );
	outColor.a = static_cast<u8>( clamp( ( a * INV_255_F ) * ( c.a * INV_255_F ), 0.0f, 1.0f ) * 255.0f );
	return outColor;
}


Color Color::operator*( Color &c ) const
{
	Color outColor = *this;
	if( c.r == 255 && c.g == 255 && c.b == 255 && c.a == 255 ) { return outColor; }
	outColor.r = static_cast<u8>( clamp( ( r * INV_255_F ) * ( c.r * INV_255_F ), 0.0f, 1.0f ) * 255.0f );
	outColor.g = static_cast<u8>( clamp( ( g * INV_255_F ) * ( c.g * INV_255_F ), 0.0f, 1.0f ) * 255.0f );
	outColor.b = static_cast<u8>( clamp( ( b * INV_255_F ) * ( c.b * INV_255_F ), 0.0f, 1.0f ) * 255.0f );
	outColor.a = static_cast<u8>( clamp( ( a * INV_255_F ) * ( c.a * INV_255_F ), 0.0f, 1.0f ) * 255.0f );
	return outColor;
}


Color Color::operator*( float s ) const
{
	Color outColor;
	outColor.r = static_cast<u8>( clamp( ( r * INV_255_F ) * s, 0.0f, 1.0f ) * 255.0f );
	outColor.g = static_cast<u8>( clamp( ( g * INV_255_F ) * s, 0.0f, 1.0f ) * 255.0f );
	outColor.b = static_cast<u8>( clamp( ( b * INV_255_F ) * s, 0.0f, 1.0f ) * 255.0f );
	outColor.a = static_cast<u8>( clamp( ( a * INV_255_F ) * s, 0.0f, 1.0f ) * 255.0f );
	return outColor;
}


Color Color::operator*( const Alpha &alpha ) const
{
	Color outColor = *this;
	outColor.a = static_cast<u8>( clamp( ( a * INV_255_F ) * alpha.value, 0.0f, 1.0f ) * 255.0f );
	return outColor;
}


Color &Color::operator*=( const Color &c )
{
	if( c.r == 255 && c.g == 255 && c.b == 255 && c.a == 255 ) { return *this; }
	r = static_cast<u8>( clamp( ( r * INV_255_F ) * ( c.r * INV_255_F ), 0.0f, 1.0f ) * 255.0f );
	g = static_cast<u8>( clamp( ( g * INV_255_F ) * ( c.g * INV_255_F ), 0.0f, 1.0f ) * 255.0f );
	b = static_cast<u8>( clamp( ( b * INV_255_F ) * ( c.b * INV_255_F ), 0.0f, 1.0f ) * 255.0f );
	a = static_cast<u8>( clamp( ( a * INV_255_F ) * ( c.a * INV_255_F ), 0.0f, 1.0f ) * 255.0f );
	return *this;
}


Color &Color::operator*=( Color &c )
{
	if( c.r == 255 && c.g == 255 && c.b == 255 && c.a == 255 ) { return *this; }
	r = static_cast<u8>( clamp( ( r * INV_255_F ) * ( c.r * INV_255_F ), 0.0f, 1.0f ) * 255.0f );
	g = static_cast<u8>( clamp( ( g * INV_255_F ) * ( c.g * INV_255_F ), 0.0f, 1.0f ) * 255.0f );
	b = static_cast<u8>( clamp( ( b * INV_255_F ) * ( c.b * INV_255_F ), 0.0f, 1.0f ) * 255.0f );
	a = static_cast<u8>( clamp( ( a * INV_255_F ) * ( c.a * INV_255_F ), 0.0f, 1.0f ) * 255.0f );
	return *this;
}


Color &Color::operator*=( float s )
{
	r = static_cast<u8>( clamp( ( r * INV_255_F ) * s, 0.0f, 1.0f ) * 255.0f );
	g = static_cast<u8>( clamp( ( g * INV_255_F ) * s, 0.0f, 1.0f ) * 255.0f );
	b = static_cast<u8>( clamp( ( b * INV_255_F ) * s, 0.0f, 1.0f ) * 255.0f );
	a = static_cast<u8>( clamp( ( a * INV_255_F ) * s, 0.0f, 1.0f ) * 255.0f );
	return *this;
}


Color &Color::operator*=( const Alpha &alpha )
{
	a = static_cast<u8>( clamp( ( a * INV_255_F ) * alpha.value, 0.0f, 1.0f ) * 255.0f );
	return *this;
}


Color Color::operator/( const Color &c ) const
{
	Color outColor = *this;
	if( c.r == 255 && c.g == 255 && c.b == 255 && c.a == 255 ) { return outColor; }
	outColor.r = static_cast<u8>( clamp( ( r * INV_255_F ) / ( c.r * INV_255_F ), 0.0f, 1.0f ) * 255.0f );
	outColor.g = static_cast<u8>( clamp( ( g * INV_255_F ) / ( c.g * INV_255_F ), 0.0f, 1.0f ) * 255.0f );
	outColor.b = static_cast<u8>( clamp( ( b * INV_255_F ) / ( c.b * INV_255_F ), 0.0f, 1.0f ) * 255.0f );
	outColor.a = static_cast<u8>( clamp( ( a * INV_255_F ) / ( c.a * INV_255_F ), 0.0f, 1.0f ) * 255.0f );
	return outColor;
}


Color Color::operator/( Color &c ) const
{
	Color outColor = *this;
	if( c.r == 255 && c.g == 255 && c.b == 255 && c.a == 255 ) { return outColor; }
	outColor.r = static_cast<u8>( clamp( ( r * INV_255_F ) / ( c.r * INV_255_F ), 0.0f, 1.0f ) * 255.0f );
	outColor.g = static_cast<u8>( clamp( ( g * INV_255_F ) / ( c.g * INV_255_F ), 0.0f, 1.0f ) * 255.0f );
	outColor.b = static_cast<u8>( clamp( ( b * INV_255_F ) / ( c.b * INV_255_F ), 0.0f, 1.0f ) * 255.0f );
	outColor.a = static_cast<u8>( clamp( ( a * INV_255_F ) / ( c.a * INV_255_F ), 0.0f, 1.0f ) * 255.0f );
	return outColor;
}


Color Color::operator/( float s ) const
{
	Color outColor;
	outColor.r = static_cast<u8>( clamp( ( r * INV_255_F ) / s, 0.0f, 1.0f ) * 255.0f );
	outColor.g = static_cast<u8>( clamp( ( g * INV_255_F ) / s, 0.0f, 1.0f ) * 255.0f );
	outColor.b = static_cast<u8>( clamp( ( b * INV_255_F ) / s, 0.0f, 1.0f ) * 255.0f );
	outColor.a = static_cast<u8>( clamp( ( a * INV_255_F ) / s, 0.0f, 1.0f ) * 255.0f );
	return outColor;
}


Color &Color::operator/=( const Color &c )
{
	if( c.r == 255 && c.g == 255 && c.b == 255 && c.a == 255 ) { return *this; }
	r = static_cast<u8>( clamp( ( r * INV_255_F ) / ( c.r * INV_255_F ), 0.0f, 1.0f ) * 255.0f );
	g = static_cast<u8>( clamp( ( g * INV_255_F ) / ( c.g * INV_255_F ), 0.0f, 1.0f ) * 255.0f );
	b = static_cast<u8>( clamp( ( b * INV_255_F ) / ( c.b * INV_255_F ), 0.0f, 1.0f ) * 255.0f );
	a = static_cast<u8>( clamp( ( a * INV_255_F ) / ( c.a * INV_255_F ), 0.0f, 1.0f ) * 255.0f );
	return *this;
}


Color &Color::operator/=( Color &c )
{
	if( c.r == 255 && c.g == 255 && c.b == 255 && c.a == 255 ) { return *this; }
	r = static_cast<u8>( clamp( ( r * INV_255_F ) / ( c.r * INV_255_F ), 0.0f, 1.0f ) * 255.0f );
	g = static_cast<u8>( clamp( ( g * INV_255_F ) / ( c.g * INV_255_F ), 0.0f, 1.0f ) * 255.0f );
	b = static_cast<u8>( clamp( ( b * INV_255_F ) / ( c.b * INV_255_F ), 0.0f, 1.0f ) * 255.0f );
	a = static_cast<u8>( clamp( ( a * INV_255_F ) / ( c.a * INV_255_F ), 0.0f, 1.0f ) * 255.0f );
	return *this;
}


Color &Color::operator/=( float s )
{
	r = static_cast<u8>( clamp( ( r * INV_255_F ) / s, 0.0f, 1.0f ) * 255.0f );
	g = static_cast<u8>( clamp( ( g * INV_255_F ) / s, 0.0f, 1.0f ) * 255.0f );
	b = static_cast<u8>( clamp( ( b * INV_255_F ) / s, 0.0f, 1.0f ) * 255.0f );
	a = static_cast<u8>( clamp( ( a * INV_255_F ) / s, 0.0f, 1.0f ) * 255.0f );
	return *this;
}


Color Color::operator+( const Color &c ) const
{
	Color outColor;
	outColor.r = static_cast<u8>( clamp( r + c.r, 0, 255 ) );
	outColor.g = static_cast<u8>( clamp( g + c.g, 0, 255 ) );
	outColor.b = static_cast<u8>( clamp( b + c.b, 0, 255 ) );
	outColor.a = static_cast<u8>( clamp( a + c.a, 0, 255 ) );
	return outColor;
}


Color Color::operator+( Color &c ) const
{
	Color outColor;
	outColor.r = static_cast<u8>( clamp( r + c.r, 0, 255 ) );
	outColor.g = static_cast<u8>( clamp( g + c.g, 0, 255 ) );
	outColor.b = static_cast<u8>( clamp( b + c.b, 0, 255 ) );
	outColor.a = static_cast<u8>( clamp( a + c.a, 0, 255 ) );
	return outColor;
}


Color &Color::operator+=( const Color &c )
{
	r = static_cast<u8>( clamp( r + c.r, 0, 255 ) );
	g = static_cast<u8>( clamp( g + c.g, 0, 255 ) );
	b = static_cast<u8>( clamp( b + c.b, 0, 255 ) );
	a = static_cast<u8>( clamp( a + c.a, 0, 255 ) );
	return *this;
}


Color &Color::operator+=( Color &c )
{
	r = static_cast<u8>( clamp( r + c.r, 0, 255 ) );
	g = static_cast<u8>( clamp( g + c.g, 0, 255 ) );
	b = static_cast<u8>( clamp( b + c.b, 0, 255 ) );
	a = static_cast<u8>( clamp( a + c.a, 0, 255 ) );
	return *this;
}


Color Color::operator-( const Color &c ) const
{
	Color outColor;
	outColor.r = static_cast<u8>( clamp( r - c.r, 0, 255 ) );
	outColor.g = static_cast<u8>( clamp( g - c.g, 0, 255 ) );
	outColor.b = static_cast<u8>( clamp( b - c.b, 0, 255 ) );
	outColor.a = static_cast<u8>( clamp( a - c.a, 0, 255 ) );
	return outColor;
}


Color Color::operator-( Color &c ) const
{
	Color outColor;
	outColor.r = static_cast<u8>( clamp( r - c.r, 0, 255 ) );
	outColor.g = static_cast<u8>( clamp( g - c.g, 0, 255 ) );
	outColor.b = static_cast<u8>( clamp( b - c.b, 0, 255 ) );
	outColor.a = static_cast<u8>( clamp( a - c.a, 0, 255 ) );
	return outColor;
}


Color &Color::operator-=( const Color &c )
{
	r = static_cast<u8>( clamp( r - c.r, 0, 255 ) );
	g = static_cast<u8>( clamp( g - c.g, 0, 255 ) );
	b = static_cast<u8>( clamp( b - c.b, 0, 255 ) );
	a = static_cast<u8>( clamp( a - c.a, 0, 255 ) );
	return *this;
}


Color &Color::operator-=( Color &c )
{
	r = static_cast<u8>( clamp( r - c.r, 0, 255 ) );
	g = static_cast<u8>( clamp( g - c.g, 0, 255 ) );
	b = static_cast<u8>( clamp( b - c.b, 0, 255 ) );
	a = static_cast<u8>( clamp( a - c.a, 0, 255 ) );
	return *this;
}


bool Color::operator==( const Color &c ) const
{
	return ( r == c.r && g == c.g && b == c.b && a == c.a );
}


u8_v4 Color::as_u8_v4() const { return u8_v4 { r, g, b, a }; }
Color::operator const u8_v4() const { return as_u8_v4(); }


u8_v3 Color::as_u8_v3() const { return u8_v3 { r, g, b }; }
Color::operator const u8_v3() const { return as_u8_v3(); }


int_v4 Color::as_int_v4() const { return int_v4 { r, g, b, a }; }
Color::operator const int_v4() const { return as_int_v4(); }


int_v3 Color::as_int_v3() const { return int_v3 { r, g, b }; }
Color::operator const int_v3() const { return as_int_v3(); }


float_v4 Color::as_float_v4() const { return float_v4 { r * INV_255_F, g * INV_255_F, b * INV_255_F, a * INV_255_F }; }
Color::operator const float_v4() const { return as_float_v4(); }


float_v3 Color::as_float_v3() const { return float_v3 { r * INV_255_F, g * INV_255_F, b * INV_255_F }; }
Color::operator const float_v3() const { return as_float_v3(); }


double_v4 Color::as_double_v4() const { return double_v4 { r * INV_255_D, g * INV_255_D, b * INV_255_D, a * INV_255_D }; }
Color::operator const double_v4() const { return as_double_v4(); }


double_v3 Color::as_double_v3() const { return double_v3 { r * INV_255_D, g * INV_255_D, b * INV_255_D }; }
Color::operator const double_v3() const { return as_double_v3(); }


u32 Color::hash() const
{
	return ( static_cast<u32>( r ) << 24 ) |
		( static_cast<u32>( g ) << 16 ) |
		( static_cast<u32>( b ) << 8 ) |
		( static_cast<u32>( a ) );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Color color_make_bw( u8 l )
{
	return Color { l, l, l, 255U };
}


Color color_make_bw( float brightness )
{
	const u8 intensity = static_cast<u8>( 255.0f * brightness );
	return Color( intensity, intensity, intensity, 255 );
}


Color color_change_brightness( Color color, float brightness )
{
	const u8 intensityR = static_cast<u8>( clamp( static_cast<float>( color.r ) * brightness, 0.0f, 255.0f ) );
	const u8 intensityG = static_cast<u8>( clamp( static_cast<float>( color.g ) * brightness, 0.0f, 255.0f ) );
	const u8 intensityB = static_cast<u8>( clamp( static_cast<float>( color.b ) * brightness, 0.0f, 255.0f ) );
	return Color( intensityR, intensityG, intensityB, color.a );
}


Color color_swap_alpha( Color rgba, u8 a )
{
	return Color { rgba.r, rgba.g, rgba.b, a };
}


float color_value_srgb_to_linear( float v )
{
	return ( v <= 0.04045f ) ? ( v / 12.92f ) : powf( ( v + 0.055f ) / 1.055f, 2.4f );
}


float color_value_linear_to_srgb( float v )
{
	return ( v <= 0.0031308f ) ? ( v * 12.92f ) : ( 1.055f * powf( v, 1.0f / 2.4f ) - 0.055f );
}


Color color_mix( Color sourceColor, Color targetColor, float amount )
{
	sourceColor.r = static_cast<u8>( sourceColor.r + ( targetColor.r - sourceColor.r ) * amount );
	sourceColor.g = static_cast<u8>( sourceColor.g + ( targetColor.g - sourceColor.g ) * amount );
	sourceColor.b = static_cast<u8>( sourceColor.b + ( targetColor.b - sourceColor.b ) * amount );
	return sourceColor;
}


Color color_mix_srgb( Color sourceColor, Color targetColor, float amount )
{
	static constexpr float inv255F = 1.0f / 255.0f;

	const float ar = color_value_srgb_to_linear( sourceColor.r * inv255F );
	const float ag = color_value_srgb_to_linear( sourceColor.g * inv255F );
	const float ab = color_value_srgb_to_linear( sourceColor.b * inv255F );
	const float br = color_value_srgb_to_linear( targetColor.r * inv255F );
	const float bg = color_value_srgb_to_linear( targetColor.g * inv255F );
	const float bb = color_value_srgb_to_linear( targetColor.b * inv255F );

	float r = ar + ( br - ar ) * amount;
	float g = ag + ( bg - ag ) * amount;
	float b = ab + ( bb - ab ) * amount;

	Color out;
	out.r = static_cast<u8>( color_value_linear_to_srgb( r ) * 255.0f );
	out.g = static_cast<u8>( color_value_linear_to_srgb( g ) * 255.0f );
	out.b = static_cast<u8>( color_value_linear_to_srgb( b ) * 255.0f );
	return out;
}


Color color_mix_alpha( Color sourceColor, Color targetColor, float amount )
{
	sourceColor.r = static_cast<u8>( sourceColor.r + ( targetColor.r - sourceColor.r ) * amount );
	sourceColor.g = static_cast<u8>( sourceColor.g + ( targetColor.g - sourceColor.g ) * amount );
	sourceColor.b = static_cast<u8>( sourceColor.b + ( targetColor.b - sourceColor.b ) * amount );
	sourceColor.a = static_cast<u8>( sourceColor.a + ( targetColor.a - sourceColor.a ) * amount );
	return sourceColor;
}


void color_mix_alpha( const Color &sourceColor, Color targetColor, float amount, Color &outColor )
{
	outColor.r = static_cast<u8>( sourceColor.r + ( targetColor.r - sourceColor.r ) * amount );
	outColor.g = static_cast<u8>( sourceColor.g + ( targetColor.g - sourceColor.g ) * amount );
	outColor.b = static_cast<u8>( sourceColor.b + ( targetColor.b - sourceColor.b ) * amount );
	outColor.a = static_cast<u8>( sourceColor.a + ( targetColor.a - sourceColor.a ) * amount );
}


u32 color_pack_u32( Color color )
{
	u32 packedColor = 0;
	packedColor |= ( static_cast<u32>( color.r )       );
	packedColor |= ( static_cast<u32>( color.g ) << 8  );
	packedColor |= ( static_cast<u32>( color.b ) << 16 );
	packedColor |= ( static_cast<u32>( color.a ) << 24 );
	return packedColor;
}


Color color_unpack_u32( u32 packedColor )
{
	Color color;
	color.r = static_cast<u8>( ( packedColor       ) & 0xFF );
	color.g = static_cast<u8>( ( packedColor >> 8  ) & 0xFF );
	color.b = static_cast<u8>( ( packedColor >> 16 ) & 0xFF );
	color.a = static_cast<u8>( ( packedColor >> 24 ) & 0xFF );
	return color;
}


int color_pack_int( Color color )
{
	int packedColor = 0;
	packedColor |= ( static_cast<int>( color.r )       );
	packedColor |= ( static_cast<int>( color.g ) << 8  );
	packedColor |= ( static_cast<int>( color.b ) << 16 );
	packedColor |= ( static_cast<int>( color.a ) << 24 );
	return packedColor;
}


Color color_unpack_int( int packedColor )
{
	Color color;
	color.r = static_cast<u8>( ( packedColor       ) & 0xFF );
	color.g = static_cast<u8>( ( packedColor >> 8  ) & 0xFF );
	color.b = static_cast<u8>( ( packedColor >> 16 ) & 0xFF );
	color.a = static_cast<u8>( ( packedColor >> 24 ) & 0xFF );
	return color;
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


void ColorWeight::add( Color color, float weight )
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

	return Color
	{
		static_cast<u8>( ( r + m ) * 255 ),
		static_cast<u8>( ( g + m ) * 255 ),
		static_cast<u8>( ( b + m ) * 255 ),
		hsv.a
	};
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

	return Color
	{
		static_cast<u8>( h / 360.0f * 255.0f ),
		static_cast<u8>( s * 255.0f ),
		static_cast<u8>( v * 255.0f ),
		rgb.a
	};
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int RadialColorGraph::add_node( RadialColorNode node )
{
	if( count >= COLOR_GRAPH_COUNT ) { return -1; }

	int insertIndex = 0;
	while( insertIndex < count && nodes[insertIndex].time < node.time ) { insertIndex++; }

	if( insertIndex < count )
	{
		memory_move( &nodes[insertIndex + 1], &nodes[insertIndex],
			( count - insertIndex ) * sizeof( RadialColorNode ) );
	}

	nodes[insertIndex] = node;
	count++;
	return insertIndex;
}


void RadialColorGraph::remove_node( int index )
{
	if( index < count - 1 )
	{
		memory_move( &nodes[index], &nodes[index + 1],
			( count - index - 1 ) * sizeof( RadialColorNode ) );
	}

	count--;
}


Color RadialColorGraph::get_color( float time ) const
{
	if( count == 0 ) { return Color { 0, 0, 0, 0 }; }
	if( count == 1 ) { return nodes[0].color; }
	const float t = fmod( time, 1.0f );

	int right = -1;
	for( int i = 0; i < count; i++ )
	{
		if( nodes[i].time == t )
		{
			return nodes[i].color;
		}

		if( nodes[i].time > t )
		{
			right = i;
			break;
		}
	}

	int left;
	float span;
	float localT;

	if( right == -1 )
	{
		left = count - 1;
		right = 0;
		const float t0 = nodes[left].time;
		const float t1 = nodes[right].time + 1.0f;
		span = t1 - t0;
		localT = ( t - t0 ) / span;
	}
	else
	{
		left = right - 1;
		if( left < 0 )
		{
			left = count - 1;
			const float t0 = nodes[left].time;
			const float t1 = nodes[right].time;
			span = ( t1 + 1.0f ) - t0;
			localT = ( t + 1.0f - t0 ) / span;
		}
		else
		{
			const float t0 = nodes[left].time;
			const float t1 = nodes[right].time;
			span = t1 - t0;
			localT = ( t - t0 ) / span;
		}
	}

	if( span <= 0.0f ) { return nodes[left].color; }
	return color_mix_alpha( nodes[left].color, nodes[right].color, localT );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////