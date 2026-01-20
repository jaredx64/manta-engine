#include <core/color.hpp>

#include <core/math.hpp>
#include <core/memory.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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