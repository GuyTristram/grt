#ifndef RENDERTARGET_H
#define RENDERTARGET_H

#include "math/vec4.h"

class VertexBuffer;
class IndexBuffer;
class ShaderProgram;
class RenderState;

class RenderTarget
{
public:
	RenderTarget() : m_clear_colour( 0.f, 0.f, 0.f, 0.f ) {}
	virtual ~RenderTarget();

	enum PrimitiveType
	{
	    Triangles,
	    Fan,
	    Strip,
	    Points,
	    Lines,
		Patches
	};

	void clear_colour( float4 const &c );
	void clear( bool colour = true, bool depth = true, bool stencil = true );

	void draw( ShaderProgram &sp, RenderState &rs, PrimitiveType type, VertexBuffer &vb, IndexBuffer &ib, int patch_vertices );
	void draw( ShaderProgram &sp, RenderState &rs, PrimitiveType type, VertexBuffer &vb, int patch_vertices );

	//void clip( int2 )

	virtual int width() const = 0;
	virtual int height() const = 0;

	void bind();
	void unbind();

private:
	virtual void do_bind() = 0;
	float4 m_clear_colour;
};

#endif // RENDERTARGET_H
