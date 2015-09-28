#ifndef MESH_H
#define MESH_H

#include "core/indexbuffer.h"
#include "core/rendertarget.h"
#include "core/vertexbuffer.h"

class ShaderProgram;
class RenderState;

struct Mesh
{
	RenderTarget::PrimitiveType type;
	int patch_vertices;
	int instances = 1;
	IndexBuffer::Ptr  ib;
	VertexBuffer::Ptr vb;
	struct Bone
	{
		std::string node_name;
		float44 bone_from_model;
	};
	std::vector< Bone > bones;
	void draw( ShaderProgram &sp, RenderState &rs, RenderTarget &rt );
};

Mesh make_cube();
Mesh make_quad();

#endif // MESH_H
