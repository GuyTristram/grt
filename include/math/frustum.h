#ifndef FRUSTUM_H
#define FRUSTUM_H

#include "math/mat44.h"

struct AABB;

class Frustum
{
public:
	explicit Frustum( float44 const &m );

	bool intersect_sphere( float3 const &p, float radius ) const;
	bool intersect_aabb( AABB const &aabb ) const;
private:
	float4 m_planes[6];
};

class FrustumD
{
public:
	explicit FrustumD( double44 const &m );

	double4 m_planes[6];
};

struct AABB
{
	float3 mid;
	float3 half_size;
};

#endif
