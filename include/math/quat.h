#ifndef QUAT_H
#define QUAT_H

#include "vec3.h"
#include "vec4.h"
#include <limits>

template< typename T >
struct quat
{
	typedef T scalar;
	quat()  : x( T(0) ), y( T(0) ), z( T(0) ), w( T(1) ) {}
	quat( T x, T y, T z, T w ) : x( x ), y( y ), z( z ), w( w ) {}

	//quat( vec3<T> const &v3, T w ) : x( v3.x ), y( v3.y ), z( v3.z ), w( w ) {}
	//quat( vec3<T> const &v3, vec3<T> const &v3 ) : x( v3.x ), y( v3.y ), z( v3.z ), w( w ) {}

	quat<T> &operator+=( quat<T> const &b ) {x += b.x; y += b.y; z += b.z; w += b.w; return *this;}
	quat<T> &operator-=( quat<T> const &b ) {x -= b.x; y -= b.y; z -= b.z; w -= b.w; return *this;}
	quat<T> &operator*=( quat<T> const &b );
	quat<T> &operator*=( T b ) {x *= b; y *= b; z *= b; w *= b; return *this;}
	quat<T> operator-() const {return quat<T>( -x, -y, -z, -w );}

	T &operator[]( int i ) {return *( &x + i );}
	T const &operator[]( int i ) const {return *( &x + i );}
	T x, y, z, w;
};

typedef quat<float> floatq;

template< typename T >                  quat<T> operator+( quat<T> const &a, quat<T> const &b );
template< typename T >                  quat<T> operator-( quat<T> const &a, quat<T> const &b );
template< typename T >                  quat<T> operator*( quat<T> const &a, quat<T> const & b );
template< typename T, typename S >      quat<T> operator*( S a, quat<T> const &b );
template< typename T, typename S >      quat<T> operator*( quat<T> const &a, S b );
template< typename T, typename S >      quat<T> operator/( quat<T> const &a, S b );

template< typename T >                  bool operator==( quat<T> const &a, quat<T> const & b );
template< typename T >                  bool operator!=( quat<T> const &a, quat<T> const & b );

template< typename T >                  quat<T> unit( quat<T> const &v );
template< typename T >                  T dot( quat<T> const &a, quat<T> const &b );
template< typename T >                  quat<T> conjugate( quat<T> const &q );
template< typename T >                  quat<T> inverse( quat<T> const &q );
template< typename T >                  quat<T> rotateq( vec3<T> const &axis, T angle );
template< typename T >                  quat<T> slerp( quat<T> const &a, quat<T> const &b, T t );

template< typename T, typename M >      void to_matrix( quat<T> const &q, M &m );
template< typename T, typename M >      void from_matrix( quat<T> &q, M const &m );


// Implementation
/*
template< typename T >
quat<T>::quat( vec3<T> const &v3, T w )
{
}

template< typename T >
quat<T>::quat( vec3<T> const &a, vec3<T> const &b ) : x( v3.x ), y( v3.y ), z( v3.z ), w( w ) {}
*/

template< typename T >
quat<T> &quat<T>::operator*=( quat<T> const &b )
{
	*this = *this * b;
	return *this;
}

template< typename T >
quat<T> operator+( quat<T> const &a, quat<T> const &b )
{
	return quat<T>( a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w );
}

template< typename T >
quat<T> operator-( quat<T> const &a, quat<T> const &b )
{
	return quat<T>( a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w );
}

template< typename T >
quat<T> operator*( quat<T> const &a, quat<T> const &b )
{
	return quat<T>( a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
	                a.w * b.y + a.y * b.w + a.z * b.x - a.x * b.z,
	                a.w * b.z + a.z * b.w + a.x * b.y - a.y * b.x,
					a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z );
}

template< typename T >
vec3<T> operator*( quat<T> const &q, vec3<T> const &v )
{
	vec3<T> qv( q.x, q.y, q.z );
	vec3<T> uv( cross( qv, v ) );
	vec3<T> uuv( cross( qv, uv) );
	uv *= (T(2) * q.w); 
	uuv *= T(2); 
	return v + uv + uuv;
}

template< typename T, typename S >
quat<T> operator*( S a, quat<T> const &b )
{
	return quat<T>( a * b.x, a * b.y, a * b.z, a * b.w );
}

template< typename T, typename S >
quat<T> operator*( quat<T> const &a, S b )
{
	return quat<T>( a.x * b, a.y * b, a.z * b, a.w * b );
}

template< typename T, typename S >
quat<T> operator/( quat<T> const &a, S b )
{
	return quat<T>( a.x / b, a.y / b, a.z / b, a.w / b );
}

template< typename T >
bool operator==( quat<T> const &a, quat<T> const & b )
{
	return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
}

template< typename T >
bool operator!=( quat<T> const &a, quat<T> const & b )
{
	return !( a == b );
}

template< typename T >
T length( quat<T> const &q )
{
	return T( sqrt( dot( q, q ) ) );
}

template< typename T >
quat<T> unit( quat<T> const &q )
{
	T l = length( q );
	if( l > 0 )
		return q / length( q );
	else
		return q;
}
template< typename T >
T dot( quat<T> const &a, quat<T> const &b )
{
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}


template< typename T > 
quat<T> conjugate( quat<T> const & q )
{
	return quat<T>( -q.x, -q.y, -q.z, q.w );
}

template< typename T > 
quat<T> inverse( quat<T> const & q )
{
	return conjugate( q ) / dot( q, q );
}

template< typename T >
quat<T> rotateq( vec3<T> const &axis, T angle )
{
	return quat<T>( sin( angle * T(0.5) ) * unit( axis ), cos( angle * T(0.5) ) );
}


template< typename T >
quat<T> slerp( quat<T> const &a, quat<T> const &b, T t )
{
	quat<T> c = b;

	T cos_theta = dot( a, b );

	// Avoid taking the long way round. 
	if( cos_theta < T(0) )
	{
		c = -b;
		cos_theta = -cos_theta;
	}

	// Perform a linear interpolation when cos_theta is close to 1
	if( cos_theta > T(1) - std::numeric_limits<T>::epsilon() )
	{
		return a + ( c - a ) * t;
	}
	else
	{
		T angle = acos( cos_theta );
		return ( sin( ( T(1) - t ) * angle ) * a + sin( t * angle ) * c ) / sin( angle );
	}
}

template< typename T, typename M >
void to_matrix( quat<T> const &q, M &m ) 
{
	m[0][0] = T(1) - T(2) * ( q.y * q.y + q.z * q.z );
	m[0][1] = T(2) * ( q.x * q.y + q.w * q.z );
	m[0][2] = T(2) * ( q.x * q.z - q.w * q.y );

	m[1][0] = T(2) * ( q.x * q.y - q.w * q.z );
	m[1][1] = T(1) - T(2) * ( q.x * q.x + q.z * q.z );
	m[1][2] = T(2) * ( q.y * q.z + q.w * q.x );

	m[2][0] = T(2) * ( q.x * q.z + q.w * q.y );
	m[2][1] = T(2) * ( q.y * q.z - q.w * q.x );
	m[2][2] = T(1) - T(2) * ( q.x * q.x + q.y * q.y );
}

namespace quaternion_detail
{
	template< typename T >	T sign(T x) {return (x >= T(0) ) ? T(1) : T(-1);}
}

template< typename T, typename M >
void from_matrix( quat<T> &q, M const &m ) 
{
	// Adapted from http://www.cg.info.hiroshima-cu.ac.jp/~miyazaki/knowledge/teche52.html

	using quaternion_detail::sign;

	q.w = ( m[0][0] + m[1][1] + m[2][2] + 1.0f) / 4.0f;
	q.x = ( m[0][0] - m[1][1] - m[2][2] + 1.0f) / 4.0f;
	q.y = (-m[0][0] + m[1][1] - m[2][2] + 1.0f) / 4.0f;
	q.z = (-m[0][0] - m[1][1] + m[2][2] + 1.0f) / 4.0f;
	if(q.w < 0.0f) q.w = 0.0f;
	if(q.x < 0.0f) q.x = 0.0f;
	if(q.y < 0.0f) q.y = 0.0f;
	if(q.z < 0.0f) q.z = 0.0f;
	q.w = sqrt(q.w);
	q.x = sqrt(q.x);
	q.y = sqrt(q.y);
	q.z = sqrt(q.z);
	if(q.w >= q.x && q.w >= q.y && q.w >= q.z)
	{
		q.w *= +1.0f;
		q.x *= sign(m[1][2] - m[2][1]);
		q.y *= sign(m[2][0] - m[0][2]);
		q.z *= sign(m[0][1] - m[1][0]);
	}
	else if(q.x >= q.w && q.x >= q.y && q.x >= q.z)
	{
		q.w *= sign(m[1][2] - m[2][1]);
		q.x *= +1.0f;
		q.y *= sign(m[0][1] + m[1][0]);
		q.z *= sign(m[2][0] + m[0][2]);
	}
	else if(q.y >= q.w && q.y >= q.x && q.y >= q.z)
	{
		q.w *= sign(m[2][0] - m[0][2]);
		q.x *= sign(m[0][1] + m[1][0]);
		q.y *= +1.0f;
		q.z *= sign(m[1][2] + m[2][1]);
	}
	else // if(q.z >= q.w && q.z >= q.x && q.z >= q.y)
	{
		q.w *= sign(m[0][1] - m[1][0]);
		q.x *= sign(m[0][2] + m[2][0]);
		q.y *= sign(m[1][2] + m[2][1]);
		q.z *= +1.0f;
	}
	q = unit( q );
}

#endif
