#include <manta/simplex.hpp>

#include <core/types.hpp>
#include <core/math.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define F2 0.366025403f
#define G2 0.211324865f

static float grad( int hash, float x, float y )
{
	int h = hash & 0x3F;
	float u = h < 4 ? x : y;
	float v = h < 4 ? y : x;
	return ( ( h & 1 ) ? -u : u ) + ( ( h & 2 ) ? -2.0f * v : 2.0f * v );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Simplex::seed( u64 seed )
{
	this->s = seed;
	if( seed < 256 ) { seed |= seed << 8; }

	for( int i = 0; i < 256; i++ )
	{
		perm[i] = permTable[i];
		const int t = i & 1;
		const int v = t == 1 ? ( perm[i] ^ ( seed & 0xFF ) ) : ( perm[i] ^ ( ( seed >> 8 ) & 0xFF ) );
		perm[i] = static_cast<u8>( v );
	}
}


float Simplex::sample_fast( float x, float y ) const
{
	float n0, n1, n2;
	float t0, t1, t2;

	float s  = ( x + y ) * F2;
	int i = fast_floor( x + s );
	int j = fast_floor( y + s );
	float t  = static_cast<float>( i + j ) * G2;
	float xf = i - t;
	float yf = j - t;
	float x0 = x - xf;
	float y0 = y - yf;
	int i1 = x0 > y0;
	int j1 = !i1;
	float x1 = x0 - i1 + G2;
	float y1 = y0 - j1 + G2;
	float x2 = x0 - 1.0f + 2.0f * G2;
	float y2 = y0 - 1.0f + 2.0f * G2;

	// Work out the hashed gradient indices of the three simplex corners
	int gi0 = perm[ static_cast<u8>( i      + perm[ static_cast<u8>( j ) ] ) ];
	int gi1 = perm[ static_cast<u8>( i + i1 + perm[ static_cast<u8>( j + j1 ) ] ) ];
	int gi2 = perm[ static_cast<u8>( i + 1  + perm[ static_cast<u8>( j + 1 ) ] ) ];

	// Calculate the contribution from the first corner
	t0 = 0.5f - x0 * x0 - y0 * y0;

	if( t0 < 0.0f )
	{
		n0 = 0.0f;
	}
	else
	{
		t0 *= t0;
		n0  = t0 * t0 * grad( gi0, x0, y0 );
	}

	// Calculate the contribution from the second corner
	t1 = 0.5f - x1 * x1 - y1 * y1;

	if( t1 < 0.0f )
	{
		n1 = 0.0f;
	}
	else
	{
		t1 *= t1;
		n1  = t1 * t1 * grad( gi1, x1, y1 );
	}

	// Calculate the contribution from the third corner
	t2 = 0.5f - x2 * x2 - y2 * y2;

	if( t2 < 0.0f )
	{
		n2 = 0.0f;
	}
	else
	{
		t2 *= t2;
		n2  = t2 * t2 * grad(gi2, x2, y2);
	}

	return 45.23065f * ( n0 + n1 + n2 );
}


float Simplex::sample_unorm_fast( float x, float y ) const
{
	return Simplex::sample_fast( x, y ) * 0.5f + 0.5f;
}


float Simplex::sample_fbm_fast( float x, float y, float f, float a, float l, float p, int o ) const
{
	float output = 0.0f;

	for( int i = 0; i < o; i++ )
	{
		output += a * Simplex::sample_fast( x * f, y * f );
		f *= l;
		a *= p;
	}

	return output;
}


float Simplex::sample_fbm_unorm_fast( float x, float y, float f, float a, float l, float p, int o ) const
{
	return Simplex::sample_fbm_fast( x, y, f, a, l, p, o ) * 0.5f + 0.5f;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float Simplex::sample( float x, float y ) const
{
	float n0, n1, n2;
	float t0, t1, t2;

	float s = ( x + y ) * F2;
	int i = fast_floor( x + s );
	int j = fast_floor( y + s );
	float t  = static_cast<float>( i + j ) * G2;
	float xf = i - t;
	float yf = j - t;
	float x0 = x - xf;
	float y0 = y - yf;
	int i1 = x0 > y0;
	int j1 = !i1;
	float x1 = x0 - i1 + G2;
	float y1 = y0 - j1 + G2;
	float x2 = x0 - 1.0f + 2.0f * G2;
	float y2 = y0 - 1.0f + 2.0f * G2;

	// Work out the hashed gradient indices of the three simplex corners
	int gi0 = hash( i, j );
	int gi1 = hash( i + i1, j + j1 );
	int gi2 = hash( i + 1, j + 1 );

	// Calculate the contribution from the first corner
	t0 = 0.5f - x0 * x0 - y0 * y0;

	if( t0 < 0.0f )
	{
		n0 = 0.0f;
	}
	else
	{
		t0 *= t0;
		n0  = t0 * t0 * grad( gi0, x0, y0 );
	}

	// Calculate the contribution from the second corner
	t1 = 0.5f - x1 * x1 - y1 * y1;

	if( t1 < 0.0f )
	{
		n1 = 0.0f;
	}
	else
	{
		t1 *= t1;
		n1  = t1 * t1 * grad( gi1, x1, y1 );
	}

	// Calculate the contribution from the third corner
	t2 = 0.5f - x2 * x2 - y2 * y2;

	if( t2 < 0.0f )
	{
		n2 = 0.0f;
	}
	else
	{
		t2 *= t2;
		n2  = t2 * t2 * grad( gi2, x2, y2 );
	}

	return 45.23065f * ( n0 + n1 + n2 );
}


float Simplex::sample_unorm( float x, float y ) const
{
	return Simplex::sample( x, y ) * 0.5f + 0.5f;
}


float Simplex::sample_fbm( float x, float y, float f, float a, float l, float p, int o ) const
{
	float output = 0.0f;

	for( int i = 0; i < o; i++ )
	{
		output += a * Simplex::sample( x * f, y * f );

		f *= l;
		a *= p;
	}

	return output;
}


float Simplex::sample_fbm_unorm( float x, float y, float f, float a, float l, float p, int o ) const
{
	return Simplex::sample_fbm( x, y, f, a, l, p, o ) * 0.5f + 0.5f;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Simplex::hash( int x, int y ) const
{
	u32 h = static_cast<u32>( s );
	h = h * 0x45d9f3b + x;
	h = h * 0x45d9f3b + y;
	h ^= h >> 16;
	h *= 0x85ebca6b;
	h ^= h >> 13;
	h *= 0xc2b2ae35;
	h ^= h >> 16;
	return h & 0x3F;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////