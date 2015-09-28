#ifndef mat22_H
#define mat22_H

#include <math.h>

#include "vec2.h"

template< typename T >
struct mat22
{
	typedef T scalar;
	typedef vec2< T > column;
	mat22() : i( 1, 0 ), j( 0,1 ) {}
	mat22( column const &i, column const &j ) : i( i ), j( j ) {}

	column &operator[]( int index ) {return *( &i + index );}
	column const &operator[]( int index ) const {return *( &i + index );}

	column i, j;
};

typedef mat22< float > float22;
typedef mat22< double > double22;

template< typename T >  vec2<T> operator*( mat22<T> const &a, vec2<T> const &b );
template< typename T >  mat22<T> operator*( mat22<T> const &a, mat22<T> const & b );
template< typename T >  mat22<T> transpose( mat22<T> const &m );
template< typename T >  mat22<T> rotate( T angle );

// Implementation

template< typename T >  vec2<T> operator*( mat22<T> const &a, vec2<T> const &b )
{
	return a.i * b.x + a.j * b.y;
}

template< typename T >  mat22<T> operator*( mat22<T> const &a, mat22<T> const & b )
{
	return mat22<T>( a * b.i, a * b.j );
}

template< typename T >  mat22<T> transpose( mat22<T> const &m )
{
	return mat22<T>( vec2<T>( m.i.x, m.j.x ),
	                 vec2<T>( m.i.y, m.j.y ) );
}


template< typename T >  mat22<T> rotate( vec2<T> const &axis, T angle )
{
	vec2<T> a = unit( axis );

	T c = cos( angle );
	T s = sin( angle );

	return mat22<T>( vec2<T>( c, s ),
	                 vec2<T>( -s, c ) );
}

#endif
