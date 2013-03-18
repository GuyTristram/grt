#ifndef FRUSTUM_H
#define FRUSTUM_H

#include "math/mat44.h"

class Frustum
{
public:
	Frustum( float44 const &m );

	bool intersect_sphere( float3 const &p, float radius );
private:
	float4 m_planes[6];
};

#endif
