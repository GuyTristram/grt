#ifndef LIGHT_H
#define LIGHT_H

#include "common/shared.h"
#include "math/vec4.h"

class TextureCube;
struct Mesh;

struct Light// : public Shared
{
	Light() :
		position( 0.f, 0.f, 0.f, 1.f ),
		colour( 10.f, 10.f, 10.f, 10.f ),
		radius( 20.f ),
		dirty( true ),
		casts_shadows( true ) {}

	void update( Mesh &mesh );

	float4 position;
	float4 colour;
	float radius;
	bool dirty;
	bool casts_shadows;
	SharedPtr< TextureCube > shadow_map;
};

#endif // LIGHT_H
