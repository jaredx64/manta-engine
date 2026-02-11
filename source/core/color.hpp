#pragma once

#include <core/types.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// TODO: 'Color' should potentially alias float_v4 and uint8v4 vectors

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum_type( ColorFormat, u8 )
{
	ColorFormat_RGBA8 = 0,
	ColorFormat_R10G10B10A2,
	ColorFormat_R8,
	ColorFormat_R8G8,
	ColorFormat_R16,
	ColorFormat_R16G16,
	ColorFormat_R24G8,
	ColorFormat_R32,
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Color;
extern struct Color color_hsv_to_rgb( const struct Color &hsv );
extern struct Color color_rgb_to_hsv( const struct Color &rgb );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Alpha
{
    constexpr Alpha() : value { 1.0f } { }
    constexpr Alpha( const Alpha &other ) : value { other.value } { }
    constexpr Alpha( float alpha ) : value { alpha } { }
    constexpr Alpha( double alpha ) : value { static_cast<float>( alpha ) } {}
    constexpr Alpha( u8 alpha ) : value { alpha / 255.0f } { }

    constexpr Alpha &operator=( const Alpha &other ) = default;

    constexpr Alpha operator+( const Alpha &other ) const { return Alpha { value + other.value }; }
    constexpr Alpha operator-( const Alpha &other ) const { return Alpha { value - other.value }; }
    constexpr Alpha operator*( const Alpha &other ) const { return Alpha { value * other.value }; }
    constexpr Alpha operator/( const Alpha &other ) const { return Alpha { value / other.value }; }

    constexpr Alpha operator+( float other ) const { return Alpha { value + other }; }
    constexpr Alpha operator-( float other ) const { return Alpha { value - other }; }
    constexpr Alpha operator*( float other ) const { return Alpha { value * other }; }
    constexpr Alpha operator/( float other ) const { return Alpha { value / other }; }

    constexpr friend Alpha operator+( float lhs, const Alpha &rhs ) { return Alpha{ lhs + rhs.value }; }
    constexpr friend Alpha operator-( float lhs, const Alpha &rhs ) { return Alpha{ lhs - rhs.value }; }
    constexpr friend Alpha operator*( float lhs, const Alpha &rhs ) { return Alpha{ lhs * rhs.value }; }
    constexpr friend Alpha operator/( float lhs, const Alpha &rhs ) { return Alpha{ lhs / rhs.value }; }

    constexpr Alpha &operator+=( const Alpha &other ) { value += other.value; return *this; }
    constexpr Alpha &operator-=( const Alpha &other ) { value -= other.value; return *this; }
    constexpr Alpha &operator*=( const Alpha &other ) { value *= other.value; return *this; }
    constexpr Alpha &operator/=( const Alpha &other ) { value /= other.value; return *this; }

    constexpr Alpha &operator+=( float other ) { value += other; return *this; }
    constexpr Alpha &operator-=( float other ) { value -= other; return *this; }
    constexpr Alpha &operator*=( float other ) { value *= other; return *this; }
    constexpr Alpha &operator/=( float other ) { value /= other; return *this; }

    constexpr bool operator==( const Alpha &other ) const { return value == other.value; }
    constexpr bool operator!=( const Alpha &other ) const { return value != other.value; }
    constexpr bool operator<( const Alpha &other ) const { return value < other.value; }
    constexpr bool operator<=( const Alpha &other ) const { return value <= other.value; }
    constexpr bool operator>( const Alpha &other ) const { return value > other.value; }
    constexpr bool operator>=( const Alpha &other ) const { return value >= other.value; }

    constexpr Alpha operator+() const { return *this; }
    constexpr Alpha operator-() const { return Alpha { -value }; }

    constexpr operator float() const { return value; }
	constexpr operator double() const { return static_cast<double>( value ); }
	constexpr operator u8() const { return static_cast<u8>( value * 255.0f ); }

    float value = 1.0f;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Color
{
	constexpr Color( u8 r = 0, u8 g = 0, u8 b = 0, u8 a = 255 ) :
		r { r },
		g { g },
		b { b },
		a { a } { }

	constexpr Color( u32 code ) :
		r { static_cast<u8>( ( code >> 24 ) & 0xFF ) },
		g { static_cast<u8>( ( code >> 16 ) & 0xFF ) },
		b { static_cast<u8>( ( code >> 8 ) & 0xFF ) },
		a { static_cast<u8>( ( code ) & 0xFF ) } { }

	Color( const u8_v4 &rgba );
	Color( const u8_v3 &rgb );
	Color( const int_v4 &rgba );
	Color( const int_v3 &rgb );
	Color( const float_v4 &rgba );
	Color( const float_v3 &rgb );
	Color( const double_v4 &rgba );
	Color( const double_v3 &rgb );


	Color operator*( const Color &c ) const;
	Color operator*( Color &c ) const;
	Color operator*( float s ) const;
	Color operator*( const Alpha &alpha ) const;

	Color &operator*=( const Color &c );
	Color &operator*=( Color &c );
	Color &operator*=( float s );
	Color &operator*=( const Alpha &alpha );

	Color operator/( const Color &c ) const;
	Color operator/( Color &c ) const;
	Color operator/( const float s ) const;

	Color &operator/=( const Color &c );
	Color &operator/=( Color &c );
	Color &operator/=( const float s );

	Color operator+( const Color &c ) const;
	Color operator+( Color &c ) const;

	Color &operator+=( const Color &c );
	Color &operator+=( Color &c );

	Color operator-( const Color &c ) const;
	Color operator-( Color &c ) const;

	Color &operator-=( const Color &c );
	Color &operator-=( Color &c );

	bool operator==( const Color &c ) const;

	static constexpr float INV_255_F = 1.0f / 255.0f;
	static constexpr double INV_255_D = 1.0f / 255.0f;

	u8_v4 as_u8_v4() const;
	operator const u8_v4() const;

	u8_v3 as_u8_v3() const;
	operator const u8_v3() const;

	int_v4 as_int_v4() const;
	operator const int_v4() const;

	int_v3 as_int_v3() const;
	operator const int_v3() const;

	float_v4 as_float_v4() const;
	operator const float_v4() const;

	float_v3 as_float_v3() const;
	operator const float_v3() const;

	double_v4 as_double_v4() const;
	operator const double_v4() const;

	double_v3 as_double_v3() const;
	operator const double_v3() const;

	u32 hash() const;

	union
	{
        struct { u8 r, g, b, a; };
        struct { u8 h, s, v, alpha; };
        struct { u8 x, y, z, w; };
        u8 xyzw[4];
    };
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct ColorGrade
{
	float hue = 1.0f;           //  0.0 to 1.0 - Default: 1.0
	float hueMergeRatio = 0.0f; //  0.0 to 1.0 - Default: 0.0
	float saturation = 1.0f;    //  0.0 to 2.0 - Default: 1.0
	float brightness = 1.10f;   //  0.0 to 2.0 - Default: 1.0
	float contrast = 1.25f;     //  0.0 to 2.0 - Default: 1.0
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class ColorWeight
{
public:
	ColorWeight() { clear(); }

	void clear();
	Color color();
	void add( Color color, float weight );

private:
	float r, g, b, a, w;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern Color color_make_bw( u8 l );
extern Color color_make_bw( float brightness );

extern Color color_change_brightness( Color color, float brightness );
extern Color color_swap_alpha( Color rgba, u8 a = 0xFF );

extern float color_value_srgb_to_linear( float v );
extern float color_value_linear_to_srgb( float v );

extern Color color_mix( Color sourceColor, Color targetColor, float amount );
extern Color color_mix_srgb( Color sourceColor, Color targetColor, float amount );
extern Color color_mix_alpha( Color sourceColor, Color targetColor, float amount );
extern void color_mix_alpha( const Color &sourceColor, Color targetColor, float amount, Color &outColor );

extern u32 color_pack_u32( Color color );
extern Color color_unpack_u32( u32 packedColor );

extern int color_pack_int( Color color );
extern Color color_unpack_int( int packedColor );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static constexpr int COLOR_GRAPH_COUNT = 32;


struct RadialColorNode
{
	RadialColorNode() { };
	RadialColorNode( Color color, float time ) : color { color }, time { time } { };
	Color color = { };
	float time = 0.0f;
};


class RadialColorGraph
{
public:
	int add_node( RadialColorNode node );
	void remove_node( int index );
	Color get_color( float time ) const;

public:
	RadialColorNode nodes[COLOR_GRAPH_COUNT] = { };
	int count = 0;
	float hint = 0.0f;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define c_invisible        ( Color {   0,   0,   0,   0 } )
#define c_aqua             ( Color {   0, 255, 255, 255 } )
#define c_black            ( Color {   0,   0,   0, 255 } )
#define c_blue             ( Color {   0,   0, 255, 255 } )
#define c_dkgray           ( Color {  64,  64,  64, 255 } )
#define c_fushia           ( Color { 255,   0, 255, 255 } )
#define c_gray             ( Color { 128, 128, 128, 255 } )
#define c_green            ( Color {   0, 128,   0, 255 } )
#define c_lime             ( Color {   0, 255,   0, 255 } )
#define c_ltgray           ( Color { 192, 192, 192, 255 } )
#define c_silver           ( Color { 192, 192, 192, 255 } )
#define c_maroon           ( Color { 128,   0,   0, 255 } )
#define c_navy             ( Color {   0,   0, 128, 255 } )
#define c_olive            ( Color { 128, 128,   0, 255 } )
#define c_orange           ( Color { 255, 128,   0, 255 } )
#define c_purple           ( Color { 128,   0, 128, 255 } )
#define c_red              ( Color { 255,   0,   0, 255 } )
#define c_brown            ( Color { 128,  64,  32, 255 } )
#define c_teal             ( Color {   0,   0, 128, 255 } )
#define c_white            ( Color { 255, 255, 255, 255 } )
#define c_yellow           ( Color { 255, 255,   0, 255 } )

#define c_console_true     ( Color { 130, 215,  65, 255 } )
#define c_console_false    ( Color { 240, 50,   50, 255 } )
#define c_console_arg_req  ( Color { 215, 220, 255, 255 } )
#define c_console_arg_opt  ( Color { 125, 125, 175, 255 } )
#define c_console_success  ( Color {  90, 210,  15, 255 } )
#define c_console_warning  ( Color { 255, 225,  90, 255 } )
#define c_console_error    ( Color { 225,   0,   0, 255 } )
#define c_console_message  ( Color { 255, 225, 185, 255 } )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////