#ifndef RENDERTARGET_H
#define RENDERTARGET_H

#include "math/vec4.h"
#include <vector>

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

	void draw( ShaderProgram &sp, RenderState &rs, PrimitiveType type, VertexBuffer &vb, IndexBuffer &ib, int patch_vertices, int instances = 1 );
	void draw( ShaderProgram &sp, RenderState &rs, PrimitiveType type, VertexBuffer &vb, int patch_vertices, int instances = 1);

	virtual int width() const = 0;
	virtual int height() const = 0;

    struct Viewport
    {
        int x, y, width, height;
    };

    void pushViewport( Viewport const &vp );
    void popViewport();

protected:
    void bind();
	void unbind(); //TODO Can we remove this?

private:
    void setViewport();
	virtual void do_bind() = 0;
	float4 m_clear_colour;
    std::vector<Viewport> m_viewports;
};

#endif // RENDERTARGET_H
