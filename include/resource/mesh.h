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
	IndexBuffer::Ptr  ib;
	VertexBuffer::Ptr vb;
	void draw( ShaderProgram &sp, RenderState &rs, RenderTarget &rt );
};

Mesh make_cube();
Mesh make_quad();

#endif // MESH_H
