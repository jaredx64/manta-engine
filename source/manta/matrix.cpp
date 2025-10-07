#include <manta/matrix.hpp>

#include <core/math.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// DEPTH_RANGE_0_TO_1 for d3d11 (TODO: move these)
#if GRAPHICS_OPENGL
	#define DEPTH_RANGE_0_TO_1 ( false )
#else
	#define DEPTH_RANGE_0_TO_1 ( true )
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float_m44::float_m44()
{
	for( int i = 0; i < 16; i++ ) { data[i] = 0.0f; }
}


float_m44::float_m44( const float_m44 &m )
{
	for( int i = 0; i < 16; i++ ) { data[i] = m.data[i]; }
}


float_m44::float_m44( float_m44 &&m )
{
	for( int i = 0; i < 16; i++ ) { data[i] = m.data[i]; }
}


float_m44::float_m44( const double_m44 &m )
{
	for( int i = 0; i < 16; i++ ) { data[i] = static_cast<float>( m.data[i] ); }
}


float_m44::float_m44( double_m44 &&m )
{
	for( int i = 0; i < 16; i++ ) { data[i] = static_cast<float>( m.data[i] ); }
}


float_m44 &float_m44::operator=( const float_m44 &m )
{
	for( int i = 0; i < 16; i++ ) { data[i] = m.data[i]; } return *this;
}


float_m44 &float_m44::operator=( float_m44 &&m )
{
	for( int i = 0; i < 16; i++ ) { data[i] = m.data[i]; } return *this;
}


float_m44::operator double_m44() const
{
	double_m44 result;
	for( int i = 0; i < 16; ++i ) { result.data[i] = static_cast<double>( data[i] ); }
	return result;
}


bool float_m44_equal( const float_m44 &a, const float_m44 &b )
{
	return a[0x0] == b[0x0] &&
		a[0x1] == b[0x1] &&
		a[0x2] == b[0x2] &&
		a[0x3] == b[0x3] &&
		a[0x4] == b[0x4] &&
		a[0x5] == b[0x5] &&
		a[0x6] == b[0x6] &&
		a[0x7] == b[0x7] &&
		a[0x8] == b[0x8] &&
		a[0x9] == b[0x9] &&
		a[0xA] == b[0xA] &&
		a[0xB] == b[0xB] &&
		a[0xC] == b[0xC] &&
		a[0xD] == b[0xD] &&
		a[0xE] == b[0xE] &&
		a[0xF] == b[0xF];
}


float_m44 float_m44_transpose( const float_m44 &m )
{
    float_m44 r;
    // Column 0 becomes row 0
    r[0x0] = m[0x0];  // [0][0] stays
    r[0x1] = m[0x4];  // [1][0] → [0][1]
    r[0x2] = m[0x8];  // [2][0] → [0][2]
    r[0x3] = m[0xC];  // [3][0] → [0][3]

    // Column 1 becomes row 1
    r[0x4] = m[0x1];  // [0][1] → [1][0]
    r[0x5] = m[0x5];  // [1][1] stays
    r[0x6] = m[0x9];  // [2][1] → [1][2]
    r[0x7] = m[0xD];  // [3][1] → [1][3]

    // Column 2 becomes row 2
    r[0x8] = m[0x2];  // [0][2] → [2][0]
    r[0x9] = m[0x6];  // [1][2] → [2][1]
    r[0xA] = m[0xA];  // [2][2] stays
    r[0xB] = m[0xE];  // [3][2] → [2][3]

    // Column 3 becomes row 3
    r[0xC] = m[0x3];  // [0][3] → [3][0]
    r[0xD] = m[0x7];  // [1][3] → [3][1]
    r[0xE] = m[0xB];  // [2][3] → [3][2]
    r[0xF] = m[0xF];  // [3][3] stays

    return r;
}


float_m44 float_m44_inverse( const float_m44 &m )
{
	float_m44 r;
	float_m44 inv;

	inv[0x0] = m[0x5] * m[0xA] * m[0xF] -
		m[0x5] * m[0xB] * m[0xE] -
		m[0x9] * m[0x6] * m[0xF] +
		m[0x9] * m[0x7] * m[0xE] +
		m[0xD] * m[0x6] * m[0xB] -
		m[0xD] * m[0x7] * m[0xA];

	inv[0x4] = -m[0x4] * m[0xA] * m[0xF] +
		m[0x4] * m[0xB] * m[0xE] +
		m[0x8] * m[0x6] * m[0xF] -
		m[0x8] * m[0x7] * m[0xE] -
		m[0xC] * m[0x6] * m[0xB] +
		m[0xC] * m[0x7] * m[0xA];

	inv[0x8] = m[0x4] * m[0x9] * m[0xF] -
		m[0x4] * m[0xB] * m[0xD] -
		m[0x8] * m[0x5] * m[0xF] +
		m[0x8] * m[0x7] * m[0xD] +
		m[0xC] * m[0x5] * m[0xB] -
		m[0xC] * m[0x7] * m[0x9];

	inv[0xC] = -m[0x4] * m[0x9] * m[0xE] +
		m[0x4] * m[0xA] * m[0xD] +
		m[0x8] * m[0x5] * m[0xE] -
		m[0x8] * m[0x6] * m[0xD] -
		m[0xC] * m[0x5] * m[0xA] +
		m[0xC] * m[0x6] * m[0x9];

	inv[0x1] = -m[0x1] * m[0xA] * m[0xF] +
		m[0x1] * m[0xB] * m[0xE] +
		m[0x9] * m[0x2] * m[0xF] -
		m[0x9] * m[0x3] * m[0xE] -
		m[0xD] * m[0x2] * m[0xB] +
		m[0xD] * m[0x3] * m[0xA];

	inv[0x5] = m[0x0] * m[0xA] * m[0xF] -
		m[0x0] * m[0xB] * m[0xE] -
		m[0x8] * m[0x2] * m[0xF] +
		m[0x8] * m[0x3] * m[0xE] +
		m[0xC] * m[0x2] * m[0xB] -
		m[0xC] * m[0x3] * m[0xA];

	inv[0x9] = -m[0x0] * m[0x9] * m[0xF] +
		m[0x0] * m[0xB] * m[0xD] +
		m[0x8] * m[0x1] * m[0xF] -
		m[0x8] * m[0x3] * m[0xD] -
		m[0xC] * m[0x1] * m[0xB] +
		m[0xC] * m[0x3] * m[0x9];

	inv[0xD] = m[0x0] * m[0x9] * m[0xE] -
		m[0x0] * m[0xA] * m[0xD] -
		m[0x8] * m[0x1] * m[0xE] +
		m[0x8] * m[0x2] * m[0xD] +
		m[0xC] * m[0x1] * m[0xA] -
		m[0xC] * m[0x2] * m[0x9];

	inv[0x2] = m[0x1] * m[0x6] * m[0xF] -
		m[0x1] * m[0x7] * m[0xE] -
		m[0x5] * m[0x2] * m[0xF] +
		m[0x5] * m[0x3] * m[0xE] +
		m[0xD] * m[0x2] * m[0x7] -
		m[0xD] * m[0x3] * m[0x6];

	inv[0x6] = -m[0x0] * m[0x6] * m[0xF] +
		m[0x0] * m[0x7] * m[0xE] +
		m[0x4] * m[0x2] * m[0xF] -
		m[0x4] * m[0x3] * m[0xE] -
		m[0xC] * m[0x2] * m[0x7] +
		m[0xC] * m[0x3] * m[0x6];

	inv[0xA] = m[0x0] * m[0x5] * m[0xF] -
		m[0x0] * m[0x7] * m[0xD] -
		m[0x4] * m[0x1] * m[0xF] +
		m[0x4] * m[0x3] * m[0xD] +
		m[0xC] * m[0x1] * m[0x7] -
		m[0xC] * m[0x3] * m[0x5];

	inv[0xE] = -m[0x0] * m[0x5] * m[0xE] +
		m[0x0] * m[0x6] * m[0xD] +
		m[0x4] * m[0x1] * m[0xE] -
		m[0x4] * m[0x2] * m[0xD] -
		m[0xC] * m[0x1] * m[0x6] +
		m[0xC] * m[0x2] * m[0x5];

	inv[0x3] = -m[0x1] * m[0x6] * m[0xB] +
		m[0x1] * m[0x7] * m[0xA] +
		m[0x5] * m[0x2] * m[0xB] -
		m[0x5] * m[0x3] * m[0xA] -
		m[0x9] * m[0x2] * m[0x7] +
		m[0x9] * m[0x3] * m[0x6];

	inv[0x7] = m[0x0] * m[0x6] * m[0xB] -
		m[0x0] * m[0x7] * m[0xA] -
		m[0x4] * m[0x2] * m[0xB] +
		m[0x4] * m[0x3] * m[0xA] +
		m[0x8] * m[0x2] * m[0x7] -
		m[0x8] * m[0x3] * m[0x6];

	inv[0xB] = -m[0x0] * m[0x5] * m[0xB] +
		m[0x0] * m[0x7] * m[0x9] +
		m[0x4] * m[0x1] * m[0xB] -
		m[0x4] * m[0x3] * m[0x9] -
		m[0x8] * m[0x1] * m[0x7] +
		m[0x8] * m[0x3] * m[0x5];

	inv[0xF] = m[0x0] * m[0x5] * m[0xA] -
		m[0x0] * m[0x6] * m[0x9] -
		m[0x4] * m[0x1] * m[0xA] +
		m[0x4] * m[0x2] * m[0x9] +
		m[0x8] * m[0x1] * m[0x6] -
		m[0x8] * m[0x2] * m[0x5];

	float det = m[0x0] * inv[0x0] + m[0x1] * inv[0x4] +
		m[0x2] * inv[0x8] + m[0x3] * inv[0xC];

	if( det == 0.0f )
	{
		r = float_m44_build_zeros();
		return r;
	}

	det = 1.0f / det;
	for( int i = 0; i < 16; i++ ) { r[i] = inv[i] * det; }
	return r;
}


float_m44 float_m44_inverse_row_major(const float_m44 &m)
{
    float_m44 m_transposed = float_m44_transpose( m );

    // Compute inverse (using your current code)
    float_m44 inv = float_m44_inverse( m_transposed );

    // Transpose result back
    return float_m44_transpose( inv );
}


float_m44 float_m44_multiply( const float_m44 &a, const float_m44 &b )
{
	float_m44 r;
	r[0x0] = a[0x0] * b[0x0] + a[0x4] * b[0x1] + a[0x8] * b[0x2] + a[0xC] * b[0x3];
	r[0x4] = a[0x0] * b[0x4] + a[0x4] * b[0x5] + a[0x8] * b[0x6] + a[0xC] * b[0x7];
	r[0x8] = a[0x0] * b[0x8] + a[0x4] * b[0x9] + a[0x8] * b[0xA] + a[0xC] * b[0xB];
	r[0xC] = a[0x0] * b[0xC] + a[0x4] * b[0xD] + a[0x8] * b[0xE] + a[0xC] * b[0xF];
	r[0x1] = a[0x1] * b[0x0] + a[0x5] * b[0x1] + a[0x9] * b[0x2] + a[0xD] * b[0x3];
	r[0x5] = a[0x1] * b[0x4] + a[0x5] * b[0x5] + a[0x9] * b[0x6] + a[0xD] * b[0x7];
	r[0x9] = a[0x1] * b[0x8] + a[0x5] * b[0x9] + a[0x9] * b[0xA] + a[0xD] * b[0xB];
	r[0xD] = a[0x1] * b[0xC] + a[0x5] * b[0xD] + a[0x9] * b[0xE] + a[0xD] * b[0xF];
	r[0x2] = a[0x2] * b[0x0] + a[0x6] * b[0x1] + a[0xA] * b[0x2] + a[0xE] * b[0x3];
	r[0x6] = a[0x2] * b[0x4] + a[0x6] * b[0x5] + a[0xA] * b[0x6] + a[0xE] * b[0x7];
	r[0xA] = a[0x2] * b[0x8] + a[0x6] * b[0x9] + a[0xA] * b[0xA] + a[0xE] * b[0xB];
	r[0xE] = a[0x2] * b[0xC] + a[0x6] * b[0xD] + a[0xA] * b[0xE] + a[0xE] * b[0xF];
	r[0x3] = a[0x3] * b[0x0] + a[0x7] * b[0x1] + a[0xB] * b[0x2] + a[0xF] * b[0x3];
	r[0x7] = a[0x3] * b[0x4] + a[0x7] * b[0x5] + a[0xB] * b[0x6] + a[0xF] * b[0x7];
	r[0xB] = a[0x3] * b[0x8] + a[0x7] * b[0x9] + a[0xB] * b[0xA] + a[0xF] * b[0xB];
	r[0xF] = a[0x3] * b[0xC] + a[0x7] * b[0xD] + a[0xB] * b[0xE] + a[0xF] * b[0xF];
	return r;
}


float_m44 float_m44_multiply_scalar( const float_m44 &a, const float scalar )
{
	float_m44 r;
	for( int i = 0; i < 16; i++ ) { r[i] = a[i] * scalar; }
	return r;
}


float_m44 float_m44_add( const float_m44 &a, const float_m44 &b )
{
	float_m44 r;
	for( int i = 0; i < 16; i++ ) { r[i] = a[i] + b[i]; }
	return r;
}


float_m44 float_m44_sub( const float_m44 &a, const float_m44 &b )
{
	float_m44 r;
	for( int i = 0; i < 16; i++ ) { r[i] = a[i] - b[i]; }
	return r;
}


float_m44 float_m44_build_zeros()
{
	float_m44 r;
	for( int i = 0; i < 16; i++ ) { r[i] = 0.0f; }
	return r;
}


float_m44 float_m44_build_identity()
{
	float_m44 r = float_m44_build_zeros();
	r[0x0] = 1.0f;
	r[0x5] = 1.0f;
	r[0xA] = 1.0f;
	r[0xF] = 1.0f;
	return r;
}


float_m44 float_m44_build_scaling( const float xscale, const float yscale, const float zscale )
{
	float_m44 r = float_m44_build_zeros();
	r[0x0] = xscale;
	r[0x5] = yscale;
	r[0xA] = zscale;
	r[0xF] = 1.0f;
	return r;
}


float_m44 float_m44_build_translation( const float xtrans, const float ytrans, const float ztrans )
{
	float_m44 r = float_m44_build_identity();
	r[0xC] = xtrans;
	r[0xD] = ytrans;
	r[0xE] = ztrans;
	return r;
}


float_m44 float_m44_build_rotation_x( const float rad )
{
	float s = static_cast<float>( sinf( rad ) );
	float c = static_cast<float>( cosf( rad ) );

	float_m44 r = float_m44_build_identity();
	r[0x5] = c;
	r[0x6] = -s;
	r[0x9] = s;
	r[0xA] = c;
	return r;
}


float_m44 float_m44_build_rotation_y( const float rad )
{
	float s = static_cast<float>( sinf( rad ) );
	float c = static_cast<float>( cosf( rad ) );

	float_m44 r = float_m44_build_identity();
	r[0x0] = c;
	r[0x2] = s;
	r[0x8] = -s;
	r[0xA] = c;
	return r;
}


float_m44 float_m44_build_rotation_z( const float rad )
{
	float s = static_cast<float>( sinf( rad ) );
	float c = static_cast<float>( cosf( rad ) );

	float_m44 r = float_m44_build_identity();
	r[0x0] = c;
	r[0x1] = -s;
	r[0x4] = s;
	r[0x5] = c;
	return r;
}


float_m44 float_m44_build_orthographic( float left, float right, float top, float bottom,
	float znear, float zfar )
{
	float_m44 r = float_m44_build_zeros();
	r[0x0] = 2.0f / ( right - left );
	r[0x1] = 0.0f;
	r[0x2] = 0.0f;
	r[0x3] = 0.0f;
	r[0x4] = 0.0f;
	r[0x5] = 2.0f / ( top - bottom );
	r[0x6] = 0.0f;
	r[0x7] = 0.0f;
	r[0x8] = 0.0f;
	r[0x9] = 0.0f;

#if DEPTH_RANGE_0_TO_1
	r[0xA] = 1.0f / ( zfar - znear );
#else
	r[0xA] = 2.0f / ( zfar - znear );
#endif

	r[0xB] = 0.0f;
	r[0xC] = ( left + right ) / ( left - right );
	r[0xD] = ( top + bottom ) / ( bottom - top );

#if DEPTH_RANGE_0_TO_1
	r[0xE] = znear / ( znear - zfar );
#else
	r[0xE] = ( zfar + znear ) / ( znear - zfar );
#endif

	r[0xF] = 1.0f;
	return r;
}


float_m44 float_m44_build_perspective( const float fov, const float aspect,
	const float znear, const float zfar )
{
	const float x = 1.0f / tanf( fov * ( 0.5f * DEG2RAD_F ) );

	float_m44 r;
	r[0x0] =  x / aspect;
	r[0x1] =  0.0f;
	r[0x2] =  0.0f;
	r[0x3] =  0.0f;
	r[0x4] =  0.0f;
	r[0x5] =  x;
	r[0x6] =  0.0f;
	r[0x7] =  0.0f;
	r[0x8] =  0.0f;
	r[0x9] =  0.0f;
#if DEPTH_RANGE_0_TO_1
	r[0xA] = zfar / ( znear - zfar );
	r[0xB] = -1.0f;
	r[0xC] =  0.0f;
	r[0xD] =  0.0f;
	r[0xE] = ( znear * zfar ) / ( znear - zfar );
#else
	r[0xA] = ( zfar + znear ) / ( znear - zfar );
	r[0xB] = -1.0f;
	r[0xC] =  0.0f;
	r[0xD] =  0.0f;
	r[0xE] = ( 2.0f * znear * zfar ) / ( znear - zfar );
#endif
	r[0xF] =  0.0f;
	return r;
}


float_m44 float_m44_build_lookat( const float x, const float y, const float z,
	const float xto, const float yto, const float zto,
	const float xup, const float yup, const float zup )
{
	float fX = xto - x;
	float fY = yto - y;
	float fZ = zto - z;
	float invLength = 1.0f / sqrt( fX * fX + fY * fY + fZ * fZ );
	fX *= invLength;
	fY *= invLength;
	fZ *= invLength;

	float rX = yup * fZ - zup * fY;
	float rY = zup * fX - xup * fZ;
	float rZ = xup * fY - yup * fX;
	invLength = 1.0f / sqrt( rX * rX + rY * rY + rZ * rZ );
	rX *= invLength;
	rY *= invLength;
	rZ *= invLength;

	float uX = fY * rZ - fZ * rY;
	float uY = fZ * rX - fX * rZ;
	float uZ = fX * rY - fY * rX;
	invLength = 1.0f / sqrt( uX * uX + uY * uY + uZ * uZ );
	uX *= invLength;
	uY *= invLength;
	uZ *= invLength;

	float_m44 r = float_m44_build_zeros();
	r[0x0] = rX;
	r[0x1] = uX;
	r[0x2] = -fX;
	r[0x3] = 0.0f;
	r[0x4] = rY;
	r[0x5] = uY;
	r[0x6] = -fY;
	r[0x7] = 0.0f;
	r[0x8] = rZ;
	r[0x9] = uZ;
	r[0xA] = -fZ;
	r[0xB] = 0.0f;
	r[0xC] = -( rX * x + rY * y + rZ * z );
	r[0xD] = -( uX * x + uY * y + uZ * z );
	r[0xE] = fX * x + fY * y + fZ * z;
	r[0xF] = 1.0f;
	return r;
}


float_m44 float_m44_build_ndc( const float width, const float height )
{
	float_m44 r;
	r[0x0] = width * 0.5;
	r[0x5] = height * -0.5;
	r[0xA] = 1.0f;
	r[0xC] = width * 0.5;
	r[0xD] = height * 0.5;
	r[0xF] = 1.0f;
	return r;
}


float_m44 float_m44_from_double_m44( const double_m44 &m )
{
	float_m44 result;
	for( int i = 0; i < 16; ++i ) { result.data[i] = static_cast<float>( m.data[i] ); }
	return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

double_m44::double_m44()
{
	for( int i = 0; i < 16; i++ ) { data[i] = 0.0; }
}


double_m44::double_m44( const float_m44 &m )
{
	for( int i = 0; i < 16; i++ ) { data[i] = static_cast<double>( m.data[i] ); }
}


double_m44::double_m44( float_m44 &&m )
{
	for( int i = 0; i < 16; i++ ) { data[i] = static_cast<double>( m.data[i] ); }
}


double_m44::double_m44( const double_m44 &m )
{
	for( int i = 0; i < 16; i++ ) { data[i] = m.data[i]; }
}


double_m44::double_m44( double_m44 &&m )
{
	for( int i = 0; i < 16; i++ ) { data[i] = m.data[i]; }
}


double_m44 &double_m44::operator=( const double_m44 &m )
{
	for( int i = 0; i < 16; i++ ) { data[i] = m.data[i]; } return *this;
}


double_m44 &double_m44::operator=( double_m44 &&m )
{
	for( int i = 0; i < 16; i++ ) { data[i] = m.data[i]; } return *this;
}


double_m44::operator float_m44() const
{
	float_m44 result;
	for( int i = 0; i < 16; ++i ) { result.data[i] = static_cast<float>( data[i] ); }
	return result;
}


bool double_m44_equal( const double_m44 &a, const double_m44 &b )
{
	return a[0x0] == b[0x0] &&
		a[0x1] == b[0x1] &&
		a[0x2] == b[0x2] &&
		a[0x3] == b[0x3] &&
		a[0x4] == b[0x4] &&
		a[0x5] == b[0x5] &&
		a[0x6] == b[0x6] &&
		a[0x7] == b[0x7] &&
		a[0x8] == b[0x8] &&
		a[0x9] == b[0x9] &&
		a[0xA] == b[0xA] &&
		a[0xB] == b[0xB] &&
		a[0xC] == b[0xC] &&
		a[0xD] == b[0xD] &&
		a[0xE] == b[0xE] &&
		a[0xF] == b[0xF];
}


double_m44 double_m44_transpose( const double_m44 &m )
{
	double_m44 r;
	r[0x0] = m[0x0];
	r[0x1] = m[0x4];
	r[0x2] = m[0x8];
	r[0x3] = m[0xC];
	r[0x4] = m[0x1];
	r[0x5] = m[0x5];
	r[0x6] = m[0x9];
	r[0x7] = m[0xD];
	r[0x8] = m[0x2];
	r[0x9] = m[0x6];
	r[0xA] = m[0xA];
	r[0xB] = m[0xE];
	r[0xC] = m[0x3];
	r[0xD] = m[0x7];
	r[0xE] = m[0xB];
	r[0xF] = m[0xF];
	return r;
}


double_m44 double_m44_inverse( const double_m44 &m )
{
	double_m44 r;
	double_m44 inv;

	inv[0x0] = m[0x5] * m[0xA] * m[0xF] -
		m[0x5] * m[0xB] * m[0xE] -
		m[0x9] * m[0x6] * m[0xF] +
		m[0x9] * m[0x7] * m[0xE] +
		m[0xD] * m[0x6] * m[0xB] -
		m[0xD] * m[0x7] * m[0xA];

	inv[0x4] = -m[0x4] * m[0xA] * m[0xF] +
		m[0x4] * m[0xB] * m[0xE] +
		m[0x8] * m[0x6] * m[0xF] -
		m[0x8] * m[0x7] * m[0xE] -
		m[0xC] * m[0x6] * m[0xB] +
		m[0xC] * m[0x7] * m[0xA];

	inv[0x8] = m[0x4] * m[0x9] * m[0xF] -
		m[0x4] * m[0xB] * m[0xD] -
		m[0x8] * m[0x5] * m[0xF] +
		m[0x8] * m[0x7] * m[0xD] +
		m[0xC] * m[0x5] * m[0xB] -
		m[0xC] * m[0x7] * m[0x9];

	inv[0xC] = -m[0x4] * m[0x9] * m[0xE] +
		m[0x4] * m[0xA] * m[0xD] +
		m[0x8] * m[0x5] * m[0xE] -
		m[0x8] * m[0x6] * m[0xD] -
		m[0xC] * m[0x5] * m[0xA] +
		m[0xC] * m[0x6] * m[0x9];

	inv[0x1] = -m[0x1] * m[0xA] * m[0xF] +
		m[0x1] * m[0xB] * m[0xE] +
		m[0x9] * m[0x2] * m[0xF] -
		m[0x9] * m[0x3] * m[0xE] -
		m[0xD] * m[0x2] * m[0xB] +
		m[0xD] * m[0x3] * m[0xA];

	inv[0x5] = m[0x0] * m[0xA] * m[0xF] -
		m[0x0] * m[0xB] * m[0xE] -
		m[0x8] * m[0x2] * m[0xF] +
		m[0x8] * m[0x3] * m[0xE] +
		m[0xC] * m[0x2] * m[0xB] -
		m[0xC] * m[0x3] * m[0xA];

	inv[0x9] = -m[0x0] * m[0x9] * m[0xF] +
		m[0x0] * m[0xB] * m[0xD] +
		m[0x8] * m[0x1] * m[0xF] -
		m[0x8] * m[0x3] * m[0xD] -
		m[0xC] * m[0x1] * m[0xB] +
		m[0xC] * m[0x3] * m[0x9];

	inv[0xD] = m[0x0] * m[0x9] * m[0xE] -
		m[0x0] * m[0xA] * m[0xD] -
		m[0x8] * m[0x1] * m[0xE] +
		m[0x8] * m[0x2] * m[0xD] +
		m[0xC] * m[0x1] * m[0xA] -
		m[0xC] * m[0x2] * m[0x9];

	inv[0x2] = m[0x1] * m[0x6] * m[0xF] -
		m[0x1] * m[0x7] * m[0xE] -
		m[0x5] * m[0x2] * m[0xF] +
		m[0x5] * m[0x3] * m[0xE] +
		m[0xD] * m[0x2] * m[0x7] -
		m[0xD] * m[0x3] * m[0x6];

	inv[0x6] = -m[0x0] * m[0x6] * m[0xF] +
		m[0x0] * m[0x7] * m[0xE] +
		m[0x4] * m[0x2] * m[0xF] -
		m[0x4] * m[0x3] * m[0xE] -
		m[0xC] * m[0x2] * m[0x7] +
		m[0xC] * m[0x3] * m[0x6];

	inv[0xA] = m[0x0] * m[0x5] * m[0xF] -
		m[0x0] * m[0x7] * m[0xD] -
		m[0x4] * m[0x1] * m[0xF] +
		m[0x4] * m[0x3] * m[0xD] +
		m[0xC] * m[0x1] * m[0x7] -
		m[0xC] * m[0x3] * m[0x5];

	inv[0xE] = -m[0x0] * m[0x5] * m[0xE] +
		m[0x0] * m[0x6] * m[0xD] +
		m[0x4] * m[0x1] * m[0xE] -
		m[0x4] * m[0x2] * m[0xD] -
		m[0xC] * m[0x1] * m[0x6] +
		m[0xC] * m[0x2] * m[0x5];

	inv[0x3] = -m[0x1] * m[0x6] * m[0xB] +
		m[0x1] * m[0x7] * m[0xA] +
		m[0x5] * m[0x2] * m[0xB] -
		m[0x5] * m[0x3] * m[0xA] -
		m[0x9] * m[0x2] * m[0x7] +
		m[0x9] * m[0x3] * m[0x6];

	inv[0x7] = m[0x0] * m[0x6] * m[0xB] -
		m[0x0] * m[0x7] * m[0xA] -
		m[0x4] * m[0x2] * m[0xB] +
		m[0x4] * m[0x3] * m[0xA] +
		m[0x8] * m[0x2] * m[0x7] -
		m[0x8] * m[0x3] * m[0x6];

	inv[0xB] = -m[0x0] * m[0x5] * m[0xB] +
		m[0x0] * m[0x7] * m[0x9] +
		m[0x4] * m[0x1] * m[0xB] -
		m[0x4] * m[0x3] * m[0x9] -
		m[0x8] * m[0x1] * m[0x7] +
		m[0x8] * m[0x3] * m[0x5];

	inv[0xF] = m[0x0] * m[0x5] * m[0xA] -
		m[0x0] * m[0x6] * m[0x9] -
		m[0x4] * m[0x1] * m[0xA] +
		m[0x4] * m[0x2] * m[0x9] +
		m[0x8] * m[0x1] * m[0x6] -
		m[0x8] * m[0x2] * m[0x5];

	double det = m[0x0] * inv[0x0] + m[0x1] * inv[0x4] +
		m[0x2] * inv[0x8] + m[0x3] * inv[0xC];

	if( det == 0.0 )
	{
		r = double_m44_build_zeros();
		return r;
	}

	det = 1.0 / det;
	for( int i = 0; i < 16; i++ ) { r[i] = inv[i] * det; }
	return r;
}


double_m44 double_m44_multiply( const double_m44 &a, const double_m44 &b )
{
	double_m44 r;
	r[0x0] = a[0x0] * b[0x0] + a[0x4] * b[0x1] + a[0x8] * b[0x2] + a[0xC] * b[0x3];
	r[0x4] = a[0x0] * b[0x4] + a[0x4] * b[0x5] + a[0x8] * b[0x6] + a[0xC] * b[0x7];
	r[0x8] = a[0x0] * b[0x8] + a[0x4] * b[0x9] + a[0x8] * b[0xA] + a[0xC] * b[0xB];
	r[0xC] = a[0x0] * b[0xC] + a[0x4] * b[0xD] + a[0x8] * b[0xE] + a[0xC] * b[0xF];
	r[0x1] = a[0x1] * b[0x0] + a[0x5] * b[0x1] + a[0x9] * b[0x2] + a[0xD] * b[0x3];
	r[0x5] = a[0x1] * b[0x4] + a[0x5] * b[0x5] + a[0x9] * b[0x6] + a[0xD] * b[0x7];
	r[0x9] = a[0x1] * b[0x8] + a[0x5] * b[0x9] + a[0x9] * b[0xA] + a[0xD] * b[0xB];
	r[0xD] = a[0x1] * b[0xC] + a[0x5] * b[0xD] + a[0x9] * b[0xE] + a[0xD] * b[0xF];
	r[0x2] = a[0x2] * b[0x0] + a[0x6] * b[0x1] + a[0xA] * b[0x2] + a[0xE] * b[0x3];
	r[0x6] = a[0x2] * b[0x4] + a[0x6] * b[0x5] + a[0xA] * b[0x6] + a[0xE] * b[0x7];
	r[0xA] = a[0x2] * b[0x8] + a[0x6] * b[0x9] + a[0xA] * b[0xA] + a[0xE] * b[0xB];
	r[0xE] = a[0x2] * b[0xC] + a[0x6] * b[0xD] + a[0xA] * b[0xE] + a[0xE] * b[0xF];
	r[0x3] = a[0x3] * b[0x0] + a[0x7] * b[0x1] + a[0xB] * b[0x2] + a[0xF] * b[0x3];
	r[0x7] = a[0x3] * b[0x4] + a[0x7] * b[0x5] + a[0xB] * b[0x6] + a[0xF] * b[0x7];
	r[0xB] = a[0x3] * b[0x8] + a[0x7] * b[0x9] + a[0xB] * b[0xA] + a[0xF] * b[0xB];
	r[0xF] = a[0x3] * b[0xC] + a[0x7] * b[0xD] + a[0xB] * b[0xE] + a[0xF] * b[0xF];
	return r;
}


double_m44 double_m44_multiply_scalar( const double_m44 &a, const double scalar )
{
	double_m44 r;
	for( int i = 0; i < 16; i++ ) { r[i] = a[i] * scalar; }
	return r;
}


double_m44 double_m44_add( const double_m44 &a, const double_m44 &b )
{
	double_m44 r;
	for( int i = 0; i < 16; i++ ) { r[i] = a[i] + b[i]; }
	return r;
}


double_m44 double_m44_sub( const double_m44 &a, const double_m44 &b )
{
	double_m44 r;
	for( int i = 0; i < 16; i++ ) { r[i] = a[i] - b[i]; }
	return r;
}


double_m44 double_m44_build_zeros()
{
	double_m44 r;
	for( int i = 0; i < 16; i++ ) { r[i] = 0.0; }
	return r;
}


double_m44 double_m44_build_identity()
{
	double_m44 r = double_m44_build_zeros();
	r[0x0] = 1.0;
	r[0x5] = 1.0;
	r[0xA] = 1.0;
	r[0xF] = 1.0;
	return r;
}


double_m44 double_m44_build_scaling( const double xscale, const double yscale, const double zscale )
{
	double_m44 r = double_m44_build_zeros();
	r[0x0] = xscale;
	r[0x5] = yscale;
	r[0xA] = zscale;
	r[0xF] = 1.0;
	return r;
}


double_m44 double_m44_build_translation( const double xtrans, const double ytrans, const double ztrans )
{
	double_m44 r = double_m44_build_identity();
	r[0xC] = xtrans;
	r[0xD] = ytrans;
	r[0xE] = ztrans;
	return r;
}


double_m44 double_m44_build_rotation_x( const double rad )
{
	double s = sin( rad );
	double c = cos( rad );

	double_m44 r = double_m44_build_identity();
	r[0x5] = c;
	r[0x6] = -s;
	r[0x9] = s;
	r[0xA] = c;
	return r;
}


double_m44 double_m44_build_rotation_y( const double rad )
{
	double s = sin( rad );
	double c = cos( rad );

	double_m44 r = double_m44_build_identity();
	r[0x0] = c;
	r[0x2] = s;
	r[0x8] = -s;
	r[0xA] = c;
	return r;
}


double_m44 double_m44_build_rotation_z( const double rad )
{
	double s = sin( rad );
	double c = cos( rad );

	double_m44 r = double_m44_build_identity();
	r[0x0] = c;
	r[0x1] = -s;
	r[0x4] = s;
	r[0x5] = c;
	return r;
}


double_m44 double_m44_build_orthographic( double left, double right, double top, double bottom,
	double znear, double zfar )
{
	double_m44 r = double_m44_build_zeros();
	r[0x0] = 2.0 / ( right - left );
	r[0x1] = 0.0;
	r[0x2] = 0.0;
	r[0x3] = 0.0;
	r[0x4] = 0.0;
	r[0x5] = 2.0 / ( top - bottom );
	r[0x6] = 0.0;
	r[0x7] = 0.0;
	r[0x8] = 0.0;
	r[0x9] = 0.0;

	#if DEPTH_RANGE_0_TO_1
		r[0xA] = 1.0 / ( zfar - znear );
	#else
		r[0xA] = 2.0 / ( zfar - znear );
	#endif

	r[0xB] = 0.0f;
	r[0xC] = ( left + right ) / ( left - right );
	r[0xD] = ( top + bottom ) / ( bottom - top );

	#if DEPTH_RANGE_0_TO_1
		r[0xE] = znear / ( znear - zfar );
	#else
		r[0xE] = ( zfar + znear ) / ( znear - zfar );
	#endif

	r[0xF] = 1.0;
	return r;
}


double_m44 double_m44_build_perspective( const double fov, const double aspect,
	const double znear, const double zfar )
{
	const double x = 1.0 / tan( fov * ( 0.5 * DEG2RAD ) );

	double_m44 r;
	r[0x0] =  x / aspect;
	r[0x1] =  0.0;
	r[0x2] =  0.0;
	r[0x3] =  0.0;
	r[0x4] =  0.0;
	r[0x5] =  x;
	r[0x6] =  0.0;
	r[0x7] =  0.0;
	r[0x8] =  0.0;
	r[0x9] =  0.0;
#if DEPTH_RANGE_0_TO_1
	r[0xA] = zfar / ( znear - zfar );
	r[0xB] = -1.0;
	r[0xC] =  0.0;
	r[0xD] =  0.0;
	r[0xE] = ( znear * zfar ) / ( znear - zfar );
#else
	r[0xA] = ( zfar + znear ) / ( znear - zfar );
	r[0xB] = -1.0;
	r[0xC] =  0.0;
	r[0xD] =  0.0;
	r[0xE] = ( 2.0 * znear * zfar ) / ( znear - zfar );
#endif
	r[0xF] =  0.0;
	return r;
}


double_m44 double_m44_build_lookat( const double x, const double y, const double z,
	const double xto, const double yto, const double zto,
	const double xup, const double yup, const double zup )
{
	double fX = xto - x;
	double fY = yto - y;
	double fZ = zto - z;
	double invLength = 1.0 / sqrt( fX * fX + fY * fY + fZ * fZ );
	fX *= invLength;
	fY *= invLength;
	fZ *= invLength;

	double rX = yup * fZ - zup * fY;
	double rY = zup * fX - xup * fZ;
	double rZ = xup * fY - yup * fX;
	invLength = 1.0 / sqrt( rX * rX + rY * rY + rZ * rZ );
	rX *= invLength;
	rY *= invLength;
	rZ *= invLength;

	double uX = fY * rZ - fZ * rY;
	double uY = fZ * rX - fX * rZ;
	double uZ = fX * rY - fY * rX;
	invLength = 1.0 / sqrt( uX * uX + uY * uY + uZ * uZ );
	uX *= invLength;
	uY *= invLength;
	uZ *= invLength;

	double_m44 r = double_m44_build_zeros();
	r[0x0] = rX;
	r[0x1] = uX;
	r[0x2] = -fX;
	r[0x3] = 0.0;
	r[0x4] = rY;
	r[0x5] = uY;
	r[0x6] = -fY;
	r[0x7] = 0.0;
	r[0x8] = rZ;
	r[0x9] = uZ;
	r[0xA] = -fZ;
	r[0xB] = 0.0;
	r[0xC] = -( rX * x + rY * y + rZ * z );
	r[0xD] = -( uX * x + uY * y + uZ * z );
	r[0xE] = fX * x + fY * y + fZ * z;
	r[0xF] = 1.0;
	return r;
}


double_m44 double_m44_build_ndc( const double width, const double height )
{
	double_m44 r;
	r[0x0] = width * 0.5;
	r[0x5] = height * -0.5;
	r[0xA] = 1.0;
	r[0xC] = width * 0.5;
	r[0xD] = height * 0.5;
	r[0xF] = 1.0;
	return r;
}


double_m44 double_m44_from_float_m44( const double_m44 &m )
{
	double_m44 result;
	for( int i = 0; i < 16; ++i ) { result.data[i] = static_cast<double>( m.data[i] ); }
	return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////