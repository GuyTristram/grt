#ifndef MAT44_H
#define MAT44_H

#include <math.h>

#include "vec4.h"

template< typename T >
struct mat44
{
	typedef T scalar;
	typedef vec4< T > row;
	mat44() : i( 1,0,0,0 ), j( 0,1,0,0 ), k( 0,0,1,0 ), t( 0,0,0,1 ) {}
	mat44( row const &i, row const &j, row const &k, row const &t ) : i( i ), j( j ), k( k ), t( t ) {}

	row &operator[]( int index ) {return *( &i + index );}
	row const &operator[]( int index ) const {return *( &i + index );}

	row i, j, k, t;
};

typedef mat44< float > float44;

template< typename T >  vec4<T> operator*( mat44<T> const &a, vec4<T> const &b );
template< typename T >  mat44<T> operator*( mat44<T> const &a, mat44<T> const & b );
template< typename T >  mat44<T> transpose( mat44<T> const &m );
template< typename T >  mat44<T> inverse( mat44<T> const &m );
template< typename T >  mat44<T> rotate( vec4<T> const &axis, T angle );
template< typename T >  mat44<T> translation( vec4<T> const s );
template< typename T >  mat44<T> scale( vec4<T> const s );
template< typename T >  mat44<T> look_at( vec4<T> const &pos, vec4<T> const &target, vec4<T> const &up = vec4<T>( T( 0 ), T( 1 ), T( 0 ), T( 0 ) ) );
template< typename T >  mat44<T> perspective( T fovy_radians, T aspect, T znear, T zfar );
template< typename T >  mat44<T> frustum( T left, T right, T bottom, T top, T znear, T zfar );

// Implementation

template< typename T >  vec4<T> operator*( mat44<T> const &a, vec4<T> const &b )
{
	return a.i * b.x + a.j * b.y + a.k * b.z + a.t * b.w;
}

template< typename T >  mat44<T> operator*( mat44<T> const &a, mat44<T> const & b )
{
	return mat44<T>( a * b.i, a * b.j, a * b.k, a * b.t );
}

template< typename T >  mat44<T> transpose( mat44<T> const &m )
{
	return mat44<T>( vec4<T>( m.i.x, m.j.x, m.k.x, m.t.x ),
	                 vec4<T>( m.i.y, m.j.y, m.k.y, m.t.y ),
	                 vec4<T>( m.i.z, m.j.z, m.k.z, m.t.z ),
	                 vec4<T>( m.i.w, m.j.w, m.k.w, m.t.w ) );
}

template< typename T >  mat44<T> inverse( mat44<T> const &m_in )
{
	mat44<T> inverted;
	T *inv = &inverted.i.x;
	T const *m = &m_in.i.x;

	inv[0] = m[5]*m[10]*m[15] - m[5]*m[11]*m[14] - m[9]*m[6]*m[15] + m[9]*m[7]*m[14] + m[13]*m[6]*m[11] - m[13]*m[7]*m[10];
	inv[4] = -m[4]*m[10]*m[15] + m[4]*m[11]*m[14] + m[8]*m[6]*m[15] - m[8]*m[7]*m[14] - m[12]*m[6]*m[11] + m[12]*m[7]*m[10];
	inv[8] = m[4]*m[9]*m[15] - m[4]*m[11]*m[13] - m[8]*m[5]*m[15] + m[8]*m[7]*m[13] + m[12]*m[5]*m[11] - m[12]*m[7]*m[9];
	inv[12] = -m[4]*m[9]*m[14] + m[4]*m[10]*m[13] + m[8]*m[5]*m[14] - m[8]*m[6]*m[13] - m[12]*m[5]*m[10] + m[12]*m[6]*m[9];
	inv[1] = -m[1]*m[10]*m[15] + m[1]*m[11]*m[14] + m[9]*m[2]*m[15] - m[9]*m[3]*m[14] - m[13]*m[2]*m[11] + m[13]*m[3]*m[10];
	inv[5] = m[0]*m[10]*m[15] - m[0]*m[11]*m[14] - m[8]*m[2]*m[15] + m[8]*m[3]*m[14] + m[12]*m[2]*m[11] - m[12]*m[3]*m[10];
	inv[9] = -m[0]*m[9]*m[15] + m[0]*m[11]*m[13] + m[8]*m[1]*m[15] - m[8]*m[3]*m[13] - m[12]*m[1]*m[11] + m[12]*m[3]*m[9];
	inv[13] = m[0]*m[9]*m[14] - m[0]*m[10]*m[13] - m[8]*m[1]*m[14] + m[8]*m[2]*m[13] + m[12]*m[1]*m[10] - m[12]*m[2]*m[9];
	inv[2] = m[1]*m[6]*m[15] - m[1]*m[7]*m[14] - m[5]*m[2]*m[15] + m[5]*m[3]*m[14] + m[13]*m[2]*m[7] - m[13]*m[3]*m[6];
	inv[6] = -m[0]*m[6]*m[15] + m[0]*m[7]*m[14] + m[4]*m[2]*m[15] - m[4]*m[3]*m[14] - m[12]*m[2]*m[7] + m[12]*m[3]*m[6];
	inv[10] = m[0]*m[5]*m[15] - m[0]*m[7]*m[13] - m[4]*m[1]*m[15] + m[4]*m[3]*m[13] + m[12]*m[1]*m[7] - m[12]*m[3]*m[5];
	inv[14] = -m[0]*m[5]*m[14] + m[0]*m[6]*m[13] + m[4]*m[1]*m[14] - m[4]*m[2]*m[13] - m[12]*m[1]*m[6] + m[12]*m[2]*m[5];
	inv[3] = -m[1]*m[6]*m[11] + m[1]*m[7]*m[10] + m[5]*m[2]*m[11] - m[5]*m[3]*m[10] - m[9]*m[2]*m[7] + m[9]*m[3]*m[6];
	inv[7] = m[0]*m[6]*m[11] - m[0]*m[7]*m[10] - m[4]*m[2]*m[11] + m[4]*m[3]*m[10] + m[8]*m[2]*m[7] - m[8]*m[3]*m[6];
	inv[11] = -m[0]*m[5]*m[11] + m[0]*m[7]*m[9] + m[4]*m[1]*m[11] - m[4]*m[3]*m[9] - m[8]*m[1]*m[7] + m[8]*m[3]*m[5];
	inv[15] = m[0]*m[5]*m[10] - m[0]*m[6]*m[9] - m[4]*m[1]*m[10] + m[4]*m[2]*m[9] + m[8]*m[1]*m[6] - m[8]*m[2]*m[5];

	T det = m[0]*inv[0] + m[1]*inv[4] + m[2]*inv[8] + m[3]*inv[12];

	if( det != 0 )
	{
		det = 1 / det;
		for( int i = 0; i < 16; i++ )
			inv[i] *= det;
	}

	return inverted;
}

template< typename T >  mat44<T> rotate( vec4<T> const &axis, T angle )
{
	vec4<T> a = unit( axis );

	T c = cos( angle );
	T s = sin( angle );
	T t = 1 - c;

	return mat44<T>( vec4<T>( t*a.x*a.x + c,      t*a.x*a.y - s*a.z,  t*a.x*a.z + s*a.y, 0 ),
	                 vec4<T>( t*a.x*a.y + s*a.z,  t*a.y*a.y + c,      t*a.y*a.z - s*a.x, 0 ),
	                 vec4<T>( t*a.x*a.z - s*a.y,  t*a.y*a.z + s*a.x,  t*a.z*a.z + c,     0 ),
	                 vec4<T>( 0,                  0,                  0,                 1 ) );
}

template< typename T >  mat44<T> translation( vec4<T> const t )
{
	return mat44<T>( vec4<T>( T( 1 ), T( 0 ), T( 0 ), T( 0 ) ),
	                 vec4<T>( T( 0 ), T( 1 ), T( 0 ), T( 0 ) ),
	                 vec4<T>( T( 0 ), T( 0 ), T( 1 ), T( 0 ) ),
	                 t );
}

template< typename T >  mat44<T> scale( vec4<T> const s )
{
	return mat44<T>( vec4<T>( s.x,  T( 0 ), T( 0 ), T( 0 ) ),
	                 vec4<T>( T( 0 ), s.y,  T( 0 ), T( 0 ) ),
	                 vec4<T>( T( 0 ), T( 0 ), s.z,  T( 0 ) ),
	                 vec4<T>( T( 0 ), T( 0 ), T( 0 ), s.w ) );
}


template< typename T >  mat44<T> perspective( T fovy_radians, T aspect_ratio, T znear, T zfar )
{
	T ymax, xmax;
	ymax = znear * tanf( fovy_radians * T( 0.5 ) );
	xmax = ymax * aspect_ratio;
	return frustum( -xmax, xmax, -ymax, ymax, znear, zfar );
}

template< typename T >  mat44<T> frustum( T left, T right, T bottom, T top, T znear, T zfar )
{
	T a, b, c, d;
	a = 2 * znear;
	b = right - left;
	c = top - bottom;
	d = zfar - znear;
	return mat44<T>( vec4<T>( a / b, 0, 0, 0 ),
	                 vec4<T>( 0, a / c, 0, 0 ),
	                 vec4<T>( ( right + left ) / b, ( top + bottom ) / c, ( -zfar - znear ) / d, -1 ),
	                 vec4<T>( 0, 0, ( -a * zfar ) / d, 0 ) );
}

template< typename T >  mat44<T> look_at( vec4<T> const &pos, vec4<T> const &target, vec4<T> const &up )
{
	mat44<T> ret;
	ret.k = unit( pos - target );
	ret.i = unit( vec4<T>( cross( up.xyz(),    ret.k.xyz() ), T( 0 ) ) );
	ret.j = unit( vec4<T>( cross( ret.k.xyz(), ret.i.xyz() ), T( 0 ) ) );
	ret.t = pos;
	return ret;
}

#endif
