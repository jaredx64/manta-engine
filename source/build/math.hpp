#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T> inline T min( const T val1, const T val2 )
{
	return ( val1 < val2 ) ? val1 : val2;
}


template <typename T> inline T max( const T val1, const T val2 )
{
	return ( val1 > val2 ) ? val1 : val2;
}


template <typename T> inline T clamp( T value, const T min, const T max )
{
	if( value < min ) { value = min; }
	if( value > max ) { value = max; }
	return value;
}


template <typename T> inline T wrap( const T &value, const T &min_value, const T &max_value )
{
	return ( value > max_value ? min_value : ( value < min_value ? max_value : value ) );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////