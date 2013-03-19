#ifndef FRUSTUM_H
#define FRUSTUM_H

#include "math/mat44.h"

struct AABB;

class Frustum
{
public:
	Frustum( float44 const &m );

	bool intersect_sphere( float3 const &p, float radius ) const;
	bool intersect_aabb( AABB const &aabb ) const;
private:
	float4 m_planes[6];
};

struct AABB
{
	float3 mid;
	float3 half_size;
};

#endif
