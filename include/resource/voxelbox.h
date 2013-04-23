#ifndef VOXELBOX_H
#define VOXELBOX_H

#include <vector>
#include "math/vec3.h"

template< typename T >
class VoxelBox
{
public:
	VoxelBox( int size_x, int size_y, int size_z );
	VoxelBox( int3 const &size );

	T &at( int x, int y, int z );
	T const &at( int x, int y, int z ) const;
	T &at( int3 const &v );
	T const &at( int3 const &v ) const;

	T &fast_at( int x, int y, int z );
	T const &fast_at( int x, int y, int z ) const;
	T &fast_at( int3 const &v );
	T const &fast_at( int3 const &v ) const;

private:
	int3 m_size;
	std::vector< T > m_data;
	T m_default;
};

template< typename T >
void fill( VoxelBox< T > &b, T const &val,
           int x0, int y0, int z0,
           int x1, int y1, int z1 );

template< typename T, typename F >
void traverse( VoxelBox< T > &b, float3 p );

////////////////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////////////////

template< typename T >
VoxelBox< T >::VoxelBox( int size_x, int size_y, int size_z )
	: m_size( size_x, size_y, size_z )
{
	m_data.resize( m_size.x * m_size.y * m_size.z );
}

template< typename T >
VoxelBox< T >::VoxelBox( int3 const &size )
	: m_size( size )
{
	m_data.resize( m_size.x * m_size.y * m_size.z );
}

template< typename T >
T &VoxelBox< T >::at( int x, int y, int z )
{
	if( x < 0 || y < 0 || z < 0 || x >= m_size.x || y >= m_size.y || z >= m_size.z )
		return m_default;
	else
		return m_data[ x + m_size.x * ( y + m_size.y * z ) ];
}

template< typename T >
T &VoxelBox< T >::at( int3 const &v )
{
	return at( v.x, v.y, v.z );
}

template< typename T >
T const &VoxelBox< T >::at( int x, int y, int z ) const
{
	if( x < 0 || y < 0 || z < 0 || x >= m_size.x || y >= m_size.y || z >= m_size.z )
		return m_default;
	else
		return m_data[ x + m_size.x * ( y + m_size.y * z ) ];
}

template< typename T >
T const &VoxelBox< T >::at( int3 const &v ) const
{
	return at( v.x, v.y, v.z );
}

template< typename T >
T &VoxelBox< T >::fast_at( int x, int y, int z )
{
	return m_data[ x + m_size.x * ( y + m_size.y * z ) ];
}

template< typename T >
T &VoxelBox< T >::fast_at( int3 const &v )
{
	return fast_at( v.x, v.y, v.z );
}

template< typename T >
T const &VoxelBox< T >::fast_at( int x, int y, int z ) const
{
	return m_data[ x + m_size.x * ( y + m_size.y * z ) ];
}

template< typename T >
T const &VoxelBox< T >::fast_at( int3 const &v ) const
{
	return fast_at( v.x, v.y, v.z );
}

template< typename T >
void fill( VoxelBox< T > &b, T const &val,
           int x0, int y0, int z0,
           int x1, int y1, int z1 )
{
	for( int z = z0; z < z1; ++z )
		for( int y = y0; y < y1; ++y )
			for( int x = x0; x < x1; ++x )
				b.at( x, y, z ) = val;
}

template< typename F >
void traverse( float3 position, float3 const &dir, F &func )
{
	int3 ip( ( int )floor( position.x ), ( int )floor( position.y ), ( int )floor( position.z ) );
	int3 idir;

	float3 dt; // Time until ray hits next plane in each diretion
	float3 dt1; // Time between ray hitting consecutive planes in each direction
	for( int i = 0; i != 3; ++i )
	{
		if( dir[i] > FLT_MIN )
		{
			dt1[i] = 1.f / dir[i];
			dt[i] = dt1[i] * ( floor( position[i] ) - position[i] + 1 );
			idir[i] = 1;
		}
		else if( dir[i] < -FLT_MIN )
		{
			dt1[i] = -1.f / dir[i];
			dt[i] = dt1[i] * ( position[i] - floor( position[i] ) );
			idir[i] = -1;
		}
		else
		{
			dt[i] = dt1[i] = FLT_MAX;
		}
	}

	do
	{
		int next = 2;
		if( dt[0] < dt[1] )
		{
			if( dt[0] < dt[2] )
				next = 0;
		}
		else
		{
			if( dt[1] < dt[2] )
				next = 1;
		}

		float next_time = dt[next];

		position += dir * next_time;

		for( int i = 0; i != 3; ++i )
			dt[i] -= next_time;
		dt[next] = dt1[next];
		ip[next] += idir[next];

	} while( func( ip, position ) );
}

#endif //VOXELBOX_H
