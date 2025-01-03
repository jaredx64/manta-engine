#include <manta/math.hpp>

#include <core/debug.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void fast_sin_cos( const float angle, float &sin, float &cos )
{
	// XMScalarSinCos from <DirectXMath.h>

	static const float XM_PI = 3.141592654f;
	static const float XM_1DIV2PI = 0.159154943f;
	static const float XM_PIDIV2 = 1.570796327f;
	static const float XM_2PI = 6.283185307f;

	float quotient, sign, y, y2;

	// Map angle to y in [-pi,pi], x = 2*pi*quotient + remainder.
	quotient = XM_1DIV2PI * angle;

	if( angle >= 0.0f )
	{
		quotient = static_cast<float>( static_cast<int>( quotient + 0.5f ) );
	}
	else
	{
		quotient = static_cast<float>( static_cast<int>( quotient - 0.5f ) );
	}

	y = angle - XM_2PI * quotient;

	// Map y to [-pi/2,pi/2] with sin(y) = sin(angle).
	if( y > XM_PIDIV2 )
	{
		y = XM_PI - y;
		sign = -1.0f;
	}
	else if( y < -XM_PIDIV2 )
	{
		y = -XM_PI - y;
		sign = -1.0f;
	}
	else
	{
		sign = 1.0f;
	}

	y2 = y * y;
	sin = ( ( ( ( ( -2.3889859e-08f * y2 + 2.7525562e-06f ) * y2 - 0.00019840874f ) * y2 + 0.0083333310f ) * y2 - 0.16666667f ) * y2 + 1.0f ) * y;
	cos = ( ( ( ( ( -2.6051615e-07f * y2 + 2.4760495e-05f ) * y2 - 0.00138883780f ) * y2 + 0.0416666380f ) * y2 - 0.50000000f ) * y2 + 1.0f ) * sign;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool matrix_equal( const Matrix &a, const Matrix &b )
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


Matrix matrix_transpose( const Matrix &m )
{
	Matrix r;
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


Matrix matrix_inverse( const Matrix &m )
{
	Matrix r;
	Matrix inv;

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
		r = matrix_build_zeros();
		return r;
	}

	det = 1.0f / det;
	for( int i = 0; i < 16; i++ ) { r[i] = inv[i] * det; }
	return r;
}


Matrix matrix_multiply( const Matrix &a, const Matrix &b )
{
	Matrix r;
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


Matrix matrix_multiply_scalar( const Matrix &a, const float scalar )
{
	Matrix r;
	for( int i = 0; i < 16; i++ ) { r[i] = a[i] * scalar; }
	return r;
}


Matrix matrix_add( const Matrix &a, const Matrix &b )
{
	Matrix r;
	for( int i = 0; i < 16; i++ ) { r[i] = a[i] + b[i]; }
	return r;
}


Matrix matrix_sub( const Matrix &a, const Matrix &b )
{
	Matrix r;
	for( int i = 0; i < 16; i++ ) { r[i] = a[i] - b[i]; }
	return r;
}


Matrix matrix_build_zeros()
{
	Matrix r;
	for( int i = 0; i < 16; i++ ) { r[i] = 0.0f; }
	return r;
}


Matrix matrix_build_identity()
{
	Matrix r = matrix_build_zeros();
	r[0x0] = 1.0f;
	r[0x5] = 1.0f;
	r[0xA] = 1.0f;
	r[0xF] = 1.0f;
	return r;
}


Matrix matrix_build_scaling( const float xscale, const float yscale, const float zscale )
{
	Matrix r = matrix_build_zeros();
	r[0x0] = xscale;
	r[0x5] = yscale;
	r[0xA] = zscale;
	r[0xF] = 1.0f;
	return r;
}


Matrix matrix_build_translation( const float xtrans, const float ytrans, const float ztrans )
{
	Matrix r = matrix_build_identity();
	r[0xC] = xtrans;
	r[0xD] = ytrans;
	r[0xE] = ztrans;
	return r;
}


Matrix matrix_build_rotation_x( const float rad )
{
	float s = static_cast<float>( sinf( rad ) );
	float c = static_cast<float>( cosf( rad ) );

	Matrix r = matrix_build_identity();
	r[0x5] = c;
	r[0x6] = -s;
	r[0x9] = s;
	r[0xA] = c;
	return r;
}


Matrix matrix_build_rotation_y( const float rad )
{
	float s = static_cast<float>( sinf( rad ) );
	float c = static_cast<float>( cosf( rad ) );

	Matrix r = matrix_build_identity();
	r[0x0] = c;
	r[0x2] = s;
	r[0x8] = -s;
	r[0xA] = c;
	return r;
}


Matrix matrix_build_rotation_z( const float rad )
{
	float s = static_cast<float>( sinf( rad ) );
	float c = static_cast<float>( cosf( rad ) );

	Matrix r = matrix_build_identity();
	r[0x0] = c;
	r[0x1] = -s;
	r[0x4] = s;
	r[0x5] = c;
	return r;
}


Matrix matrix_build_orthographic( float left, float right, float top, float bottom, float znear, float zfar )
{
	Matrix r = matrix_build_zeros();
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
	r[0xA] = 1.0f / ( znear - zfar );
	r[0xB] = 0.0f;
	r[0xC] = ( left + right ) / ( left - right );
	r[0xD] = ( top + bottom ) / ( bottom - top );
	r[0xE] = znear / ( znear - zfar );
	r[0xF] = 1.0f;
	return r;
}


Matrix matrix_build_perspective( const float fov, const float aspect, const float znear, const float zfar )
{
	const float x = 1.0f / tanf( fov * 0.5f * DEG2RAD );

	Matrix r;
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
	r[0xA] =  zfar / ( znear - zfar );
	r[0xB] = -1.0f;
	r[0xC] =  0.0f;
	r[0xD] =  0.0f;
	r[0xE] =  ( znear * zfar ) / ( znear - zfar );
	r[0xF] =  0.0f;
	return r;
}


Matrix matrix_build_lookat( const float x, const float y, const float z, const float xto, const float yto, const float zto,
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

	Matrix r = matrix_build_zeros();
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


Matrix matrix_build_ndc( const float width, const float height )
{
    Matrix r;
    r[0x0] = width * 0.5;
    r[0x5] = height * -0.5;
    r[0xA] = 1.0f;
    r[0xC] = width * 0.5;
    r[0xD] = height * 0.5;
    r[0xF] = 1.0f;
    return r;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////