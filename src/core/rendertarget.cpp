#include "core/rendertarget.h"
#include "core/shaderprogram.h"
#include "core/renderstate.h"
#include "core/vertexbuffer.h"
#include "core/indexbuffer.h"

#include "opengl/opengl.h"

namespace
{
GLenum gl_primitive( RenderTarget::PrimitiveType type )
{
	switch( type )
	{
	case RenderTarget::Triangles:
		return GL_TRIANGLES;
	case RenderTarget::Fan:
		return GL_TRIANGLE_FAN;
	case RenderTarget::Strip:
		return GL_TRIANGLE_STRIP;
	case RenderTarget::Points:
		return GL_POINTS;
	case RenderTarget::Lines:
		return GL_LINES;
	case RenderTarget::Patches:
		return GL_PATCHES;
	}
	return GL_TRIANGLES;
}

RenderTarget *g_current_rendertarget = 0;
}

RenderTarget::~RenderTarget()
{
	if( g_current_rendertarget == this )
		g_current_rendertarget = 0;
}

void RenderTarget::draw( ShaderProgram &sp, RenderState &rs, PrimitiveType type, VertexBuffer &vb, IndexBuffer &ib, int patch_vertices, int instances )
{
	if( ib.count() == 0 )
		return;
	bind();
	sp.bind();
	rs.bind();
	vb.bind();
	sp.bind_textures();
	if( type == Patches )
		glPatchParameteri( GL_PATCH_VERTICES, patch_vertices );
	if( instances == 1 )
		glDrawElements( gl_primitive( type ), ib.count(), GL_UNSIGNED_INT, ib.indices() );
	else
		glDrawElementsInstanced( gl_primitive( type ), ib.count( ), GL_UNSIGNED_INT, ib.indices( ), instances );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
	vb.unbind();
}

void RenderTarget::draw( ShaderProgram &sp, RenderState &rs, PrimitiveType type, VertexBuffer &vb, int patch_vertices, int instances )
{
	bind();
	sp.bind();
	rs.bind();
	vb.bind();
	sp.bind_textures();
	if( type == Patches )
		glPatchParameteri( GL_PATCH_VERTICES, patch_vertices );
	if( instances == 1 )
        glDrawArrays( gl_primitive( type ), 0, vb.vertex_count() );
    else
        glDrawArraysInstanced( gl_primitive( type ), 0, vb.vertex_count(), instances );
	vb.unbind();
}

void RenderTarget::clear( bool colour, bool depth, bool stencil )
{
	bind();
	GLbitfield flags = 0;

	if( colour )
	{
		flags = flags | GL_COLOR_BUFFER_BIT;
		glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
		glClearColor( m_clear_colour.x, m_clear_colour.y, m_clear_colour.z, m_clear_colour.w );
	}

	if( depth )
	{
		flags = flags | GL_DEPTH_BUFFER_BIT;
		glDepthMask( GL_TRUE );
	}

	glViewport( 0, 0, width(), height() );
	glClear( flags );
}

void RenderTarget::clear_colour( float4 const &c )
{
	m_clear_colour = c;
}

void RenderTarget::bind()
{
	if( g_current_rendertarget != this )
	{
		do_bind();
        setViewport();
		g_current_rendertarget = this;
	}
}

void RenderTarget::unbind()
{
	g_current_rendertarget = 0;
}

void RenderTarget::pushViewport( Viewport const &vp )
{
    m_viewports.push_back( vp );
    if( g_current_rendertarget == this )
        setViewport();
}

void RenderTarget::popViewport()
{
    if( m_viewports.empty() )
        return;
    m_viewports.pop_back();
    if( g_current_rendertarget == this )
        setViewport();
}

void RenderTarget::setViewport()
{
    if( m_viewports.empty() )
    {
        glViewport( 0, 0, width(), height() );
    }
    else
    {
        Viewport const &vp = m_viewports.back();
        glViewport( vp.x, vp.y, vp.width, vp.height );
    }
}
