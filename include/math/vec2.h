#ifndef VEC2_H
#define VEC2_H

template< typename T >
struct vec2
{
	typedef T scalar;
	vec2() {} // No default values for x & y
	vec2( T x, T y ) : x( x ), y( y ) {}
	template< typename U >
	explicit vec2( vec2<U> const &v ) : x( T( v.x ) ), y( T( v.y ) ) {}

	vec2<T> &operator+=( vec2<T> const &b ) {x += b.x; y += b.y; return *this;}
	vec2<T> &operator-=( vec2<T> const &b ) {x -= b.x; y -= b.y; return *this;}
	vec2<T> &operator*=( vec2<T> const &b ) {x *= b.x; y *= b.y; return *this;}
	vec2<T> operator-() {return vec2<T>( -x, -y );}

	T &operator[]( int i ) {return *( &x + i );}
	T const &operator[]( int i ) const {return *( &x + i );}
	T x, y;
};

typedef vec2<float> float2;
typedef vec2<double> double2;
typedef vec2<int> int2;
typedef vec2<short> short2;
typedef vec2<unsigned short> ushort2;
typedef vec2<signed char> char2;
typedef vec2<unsigned char> uchar2;

template< typename T >                  vec2<T> operator+( vec2<T> const &a, vec2<T> const &b );
template< typename T >                  vec2<T> operator-( vec2<T> const &a, vec2<T> const &b );
template< typename T >                  vec2<T> operator*( vec2<T> const &a, vec2<T> const & b );
template< typename T, typename S >      vec2<T> operator*( S a, vec2<T> const &b );
template< typename T, typename S >      vec2<T> operator*( vec2<T> const &a, S b );
template< typename T, typename S >      vec2<T> operator/( vec2<T> const &a, S b );

template< typename T >                  T length( vec2<T> const &v );
template< typename T >                  vec2<T> unit( vec2<T> const &v );
template< typename T >                  vec2<T> dot( vec2<T> const &a, vec2<T> const &b );


// Implementation

template< typename T >
vec2<T> operator+( vec2<T> const &a, vec2<T> const &b )
{
	return vec2<T>( a.x + b.x, a.y + b.y );
}

template< typename T >
vec2<T> operator-( vec2<T> const &a, vec2<T> const &b )
{
	return vec2<T>( a.x - b.x, a.y - b.y );
}

template< typename T >
vec2<T> operator*( vec2<T> const &a, vec2<T> const &b )
{
	return vec2<T>( a.x * b.x, a.y * b.y );
}

template< typename T, typename S >
vec2<T> operator*( S a, vec2<T> const &b )
{
	return vec2<T>( a * b.x, a * b.y );
}

template< typename T, typename S >
vec2<T> operator*( vec2<T> const &a, S b )
{
	return vec2<T>( a.x * b, a.y * b );
}

template< typename T, typename S >
vec2<T> operator/( vec2<T> const &a, S b )
{
	return vec2<T>( a.x / b, a.y / b );
}

template< typename T >
T length( vec2<T> const &v )
{
	return T( sqrt( v.x * v.x + v.y * v.y ) );
}

template< typename T >
vec2<T> unit( vec2<T> const &v )
{
	T l = length( v );
	if( l > 0 )
		return v / length( v );
	else
		return v;
}

template< typename T >
vec2<T> dot( vec2<T> const &a, vec2<T> const &b )
{
	return a.x * b.x + a.y * b.y;
}

#endif
