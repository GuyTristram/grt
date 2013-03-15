#ifndef VEC3_H
#define VEC3_H

template< typename T >
struct vec3
{
	typedef T scalar;
	vec3() {} // No default values for x & y
	vec3( T x, T y, T z ) : x(x), y(y), z(z) {}
	template< typename U >
	explicit vec3( vec3<U> const &v ) : x(T(v.x)), y(T(v.y)), z(T(v.z)) {}

	vec3<T> &operator+=( vec3<T> const &b ) {x += b.x; y += b.y; z += b.z; return *this;}
	vec3<T> &operator-=( vec3<T> const &b ) {x -= b.x; y -= b.y; z -= b.z; return *this;}
	vec3<T> &operator*=( vec3<T> const &b ) {x *= b.x; y *= b.y; z *= b.z; return *this;}
	vec3<T> operator-() {return vec3<T>( -x, -y, -z );}

	T &operator[]( int i ) {return *( &x + i );}
	T const &operator[]( int i ) const {return *( &x + i );}
	T x, y, z;
};

typedef vec3<float> float3;
typedef vec3<int> int3;
typedef vec3<short> short3;
typedef vec3<unsigned short> ushort3;
typedef vec3<signed char> char3;
typedef vec3<unsigned char> uchar3;

template< typename T >					vec3<T> operator+( vec3<T> const &a, vec3<T> const &b );
template< typename T >					vec3<T> operator-( vec3<T> const &a, vec3<T> const &b );
template< typename T >					vec3<T> operator*( vec3<T> const &a, vec3<T> const & b );
template< typename T, typename S >		vec3<T> operator*( S a, vec3<T> const &b );
template< typename T, typename S >		vec3<T> operator*( vec3<T> const &a, S b );
template< typename T, typename S >		vec3<T> operator/( vec3<T> const &a, S b );

template< typename T >					T length( vec3<T> const &v );
template< typename T >					T manhattan( vec3<T> const &v );
template< typename T >					vec3<T> unit( vec3<T> const &v );
template< typename T >					vec3<T> cross( vec3<T> const &a, vec3<T> const &b );
template< typename T >					vec3<T> dot( vec3<T> const &a, vec3<T> const &b );


// Implementation

template< typename T >
vec3<T> operator+( vec3<T> const &a, vec3<T> const &b )
{
	return vec3<T>( a.x + b.x, a.y + b.y, a.z + b.z );
}

template< typename T >
vec3<T> operator-( vec3<T> const &a, vec3<T> const &b )
{
	return vec3<T>( a.x - b.x, a.y - b.y, a.z - b.z );
}

template< typename T >
vec3<T> operator*( vec3<T> const &a, vec3<T> const &b )
{
	return vec3<T>( a.x * b.x, a.y * b.y, a.z * b.z );
}

template< typename T, typename S >
vec3<T> operator*( S a, vec3<T> const &b )
{
	return vec3<T>( a * b.x, a * b.y, a * b.z );
}

template< typename T, typename S >
vec3<T> operator*( vec3<T> const &a, S b )
{
	return vec3<T>( a.x * b, a.y * b, a.z * b );
}

template< typename T, typename S >
vec3<T> operator/( vec3<T> const &a, S b )
{
	return vec3<T>( a.x / b, a.y / b, a.z / b );
}

template< typename T >
T length( vec3<T> const &v )
{
	return T(sqrt( v.x * v.x + v.y * v.y + v.z * v.z ));
}

template< typename T >
T manhattan( vec3<T> const &v )
{
	return T(abs(v.x) + abs(v.y) + abs(v.z));
}

template< typename T >
vec3<T> unit( vec3<T> const &v )
{
	T l = length(v);
	if( l > 0 )
		return v / length(v);
	else
		return v;
}
template< typename T >
vec3<T> cross( vec3<T> const &a, vec3<T> const &b )
{
	return vec3<T>( a.y * b.z - b.y * a.z, b.x * a.z - a.x * b.z, a.x * b.y - b.x * a.y );
} 

template< typename T >
vec3<T> dot( vec3<T> const &a, vec3<T> const &b )
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
} 

#endif
