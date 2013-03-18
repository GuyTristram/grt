#include "math/frustum.h"


Frustum::Frustum( float44 const &m )
{
	for( int i = 0; i < 3; ++i )
	{
		m_planes[ i * 2 ]    = m.row( 3 ) - m.row( i );
		m_planes[ i * 2 + 1] = m.row( 3 ) + m.row( i );
	}
	for( int i = 0; i < 6; ++i )
		m_planes[ i ] *= (1.f / length( m_planes[i].xyz() ) );
}

bool Frustum::intersect_sphere( float3 const &p, float radius )
{
	for(int i = 0; i < 6; ++i)
		if( dot( m_planes[i].xyz(), p ) + m_planes[i].w < -radius )
			return false;

	return true;
}
