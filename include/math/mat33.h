#ifndef MAT33_H
#define MAT33_H

#include <math.h>

#include "vec3.h"

template< typename T >
struct mat33
{
	typedef T scalar;
	typedef vec3< T > column;
	mat33() : i( 1, 0, 0 ), j( 0,1,0 ), k( 0,0,1 ) {}
	mat33( column const &i, column const &j, column const &k ) : i( i ), j( j ), k( k ) {}

	column &operator[]( int index ) {return *( &i + index );}
	column const &operator[]( int index ) const {return *( &i + index );}

	column i, j, k;
};

typedef mat33< float > float33;
typedef mat33< double > double33;

template< typename T >  vec3<T> operator*( mat33<T> const &a, vec3<T> const &b );
template< typename T >  mat33<T> operator*( mat33<T> const &a, mat33<T> const & b );
template< typename T >  mat33<T> transpose( mat33<T> const &m );
template< typename T >  mat33<T> inverse( mat33<T> const &m );
template< typename T >  mat33<T> rotate( vec3<T> const &axis, T angle );

// Implementation

template< typename T >  vec3<T> operator*( mat33<T> const &a, vec3<T> const &b )
{
	return a.i * b.x + a.j * b.y + a.k * b.z;
}

template< typename T >  mat33<T> operator*( mat33<T> const &a, mat33<T> const & b )
{
	return mat33<T>( a * b.i, a * b.j, a * b.k );
}

template< typename T >  mat33<T> transpose( mat33<T> const &m )
{
	return mat33<T>( vec3<T>( m.i.x, m.j.x, m.k.x ),
	                 vec3<T>( m.i.y, m.j.y, m.k.y ),
	                 vec3<T>( m.i.z, m.j.z, m.k.z ) );
}

template< typename T >  mat33<T> inverse( mat33<T> const &m_in )
{
	mat33<T> inverted;
	T *inv = &inverted.i.x, *m = &m_in.i.x;

	inv[0] =   m[8]*m[4] - m[5]*m[7];
	inv[1] = -( m[8]*m[1] - m[2]*m[7] ) ;
	inv[2] =   m[5]*m[1] - m[2]*m[4];
	inv[3] = -( m[8]*m[3] - m[5]*m[6] );
	inv[4] =   m[8]*m[0] - m[2]*m[6];
	inv[5] = -( m[5]*m[0] - m[2]*m[3] );
	inv[6] =   m[7]*m[3] - m[4]*m[6];
	inv[7] = -( m[7]*m[0] - m[1]*m[6] );
	inv[8] =   m[4]*m[0] - m[1]*m[3];

	T det = m[0]*inv[0] + m[1]*inv[3] + m[2]*inv[6];

	if( det != 0 )
	{
		det = 1.0 / det;
		for( int i = 0; i < 9; i++ )
			inv[i] *= det;
	}

	return inverted;
}

template< typename T >  mat33<T> rotate( vec3<T> const &axis, T angle )
{
	vec3<T> a = unit( axis );

	T c = cos( angle );
	T s = sin( angle );
	T t = 1 - c;

	return mat33<T>( vec3<T>( t*a.x*a.x + c,      t*a.x*a.y - s*a.z,  t*a.x*a.z + s*a.y ),
	                 vec3<T>( t*a.x*a.y + s*a.z,  t*a.y*a.y + c,      t*a.y*a.z - s*a.x ),
	                 vec3<T>( t*a.x*a.z - s*a.y,  t*a.y*a.z + s*a.x,  t*a.z*a.z + c ) );
}

#endif
