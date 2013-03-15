#ifndef VEC4_H
#define VEC4_H

#include "vec3.h"

template< typename T >
struct vec4
{
	typedef T scalar;
	vec4() {} // No default values for x & y
	vec4( T x, T y, T z, T w ) : x( x ), y( y ), z( z ), w( w ) {}
	vec4( vec3<T> const &v3, T w ) : x( v3.x ), y( v3.y ), z( v3.z ), w( w ) {}

	vec3<T> xyz() const {return vec3<T>( x, y, z );}

	vec4<T> &operator+=( vec4<T> const &b ) {x += b.x; y += b.y; z += b.z; w += b.w; return *this;}
	vec4<T> &operator-=( vec4<T> const &b ) {x -= b.x; y -= b.y; z -= b.z; w -= b.w; return *this;}
	vec4<T> &operator*=( vec4<T> const &b ) {x *= b.x; y *= b.y; z *= b.z; w *= b.w; return *this;}
	vec4<T> operator-() {return vec4<T>( -x, -y, -z, -w );}

	T &operator[]( int i ) {return *( &x + i );}
	T const &operator[]( int i ) const {return *( &x + i );}
	T x, y, z, w;
};

typedef vec4<float> float4;
typedef vec4<int> int4;
typedef vec4<short> short4;
typedef vec4<unsigned short> ushort4;
typedef vec4<signed char> char4;
typedef vec4<unsigned char> uchar4;

template< typename T >                  vec4<T> operator+( vec4<T> const &a, vec4<T> const &b );
template< typename T >                  vec4<T> operator-( vec4<T> const &a, vec4<T> const &b );
template< typename T >                  vec4<T> operator*( vec4<T> const &a, vec4<T> const & b );
template< typename T, typename S >      vec4<T> operator*( S a, vec4<T> const &b );
template< typename T, typename S >      vec4<T> operator*( vec4<T> const &a, S b );
template< typename T, typename S >      vec4<T> operator/( vec4<T> const &a, S b );

template< typename T >                  T length( vec4<T> const &v );
template< typename T >                  vec4<T> unit( vec4<T> const &v );
template< typename T >                  vec4<T> dot( vec4<T> const &a, vec4<T> const &b );


// Implementation

template< typename T >
vec4<T> operator+( vec4<T> const &a, vec4<T> const &b )
{
	return vec4<T>( a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w );
}

template< typename T >
vec4<T> operator-( vec4<T> const &a, vec4<T> const &b )
{
	return vec4<T>( a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w );
}

template< typename T >
vec4<T> operator*( vec4<T> const &a, vec4<T> const &b )
{
	return vec3<T>( a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w );
}

template< typename T, typename S >
vec4<T> operator*( S a, vec4<T> const &b )
{
	return vec4<T>( a * b.x, a * b.y, a * b.z, a * b.w );
}

template< typename T, typename S >
vec4<T> operator*( vec4<T> const &a, S b )
{
	return vec4<T>( a.x * b, a.y * b, a.z * b, a.w * b );
}

template< typename T, typename S >
vec4<T> operator/( vec4<T> const &a, S b )
{
	return vec4<T>( a.x / b, a.y / b, a.z / b, a.w / b );
}

template< typename T >
T length( vec4<T> const &v )
{
	return T( sqrt( v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w ) );
}

template< typename T >
vec4<T> unit( vec4<T> const &v )
{
	T l = length( v );
	if( l > 0 )
		return v / length( v );
	else
		return v;
}
template< typename T >
vec4<T> dot( vec4<T> const &a, vec4<T> const &b )
{
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

#endif
