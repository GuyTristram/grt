#include "core/renderstate.h"
#include "opengl/opengl.h"

namespace
{
	RenderState *g_current_renderstate = 0;
}

RenderState::RenderState( BlendMode blend_mode )
	: m_blend_mode( blend_mode ),
	  m_depth_compare( LEqual ),
	  m_draw_front( true ),
	  m_draw_back( false ),
	  m_depth_write( true ),
	  m_depth_test( true ),
	  m_colour_write( true )
{
}

RenderState::~RenderState()
{
	unbind();
}

void RenderState::bind()
{
	if( g_current_renderstate == this )
		return;

	if( m_blend_mode == Opaque )
		glDisable( GL_BLEND );
	else
		glEnable( GL_BLEND );

	switch( m_blend_mode )
	{
	case Transparent:
		glBlendFunc( GL_ONE, GL_ONE_MINUS_SRC_ALPHA );
		break;
	case Blend:
		glBlendFuncSeparate( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE );
		//glBlendFunc( GL_SRC_ALPHA_SATURATE, GL_ONE_MINUS_SRC_ALPHA );
		break;
	case Add:
		glBlendFunc( GL_ONE, GL_ONE );
		break;
	}

	glColorMask( m_colour_write, m_colour_write, m_colour_write, m_colour_write );

	if( m_depth_write )
		glDepthMask( GL_TRUE );
	else
		glDepthMask( GL_FALSE );

	if( m_depth_test )
	{
		glEnable( GL_DEPTH_TEST );
		GLenum f;
		switch( m_depth_compare )
		{
		case Never:    f = GL_NEVER; break;
		case Less:     f = GL_LESS; break;
		case Equal:    f = GL_EQUAL; break;
		case LEqual:   f = GL_LEQUAL; break;
		case Greater:  f = GL_GREATER; break;
		case NotEqual: f = GL_NOTEQUAL; break;
		case GEqual:   f = GL_GEQUAL; break;
		case Always:   f = GL_ALWAYS; break;
		}
		glDepthFunc( f );
	}
	else
		glDisable( GL_DEPTH_TEST );

	if( m_draw_front && m_draw_back )
		glDisable( GL_CULL_FACE );
	else
		glEnable( GL_CULL_FACE );

	if( m_draw_front )
	{
		if( !m_draw_back )
			glCullFace( GL_BACK );
	}
	else
	{
		if( m_draw_back )
			glCullFace( GL_FRONT );
		else
			glCullFace( GL_FRONT_AND_BACK );
	}
}

void RenderState::unbind()
{
	if( g_current_renderstate == this )
		g_current_renderstate = 0;
}


void RenderState::blend_mode( BlendMode mode )
{
	if( m_blend_mode != mode )
	{
		m_blend_mode = mode;
		unbind();
	}
}

void RenderState::depth_write( bool f )
{
	if( m_depth_write != f )
	{
		m_depth_write = f;
		unbind();
	}
}

void RenderState::depth_test( bool f )
{
	if( m_depth_test != f )
	{
		m_depth_test = f;
		unbind();
	}
}

void RenderState::depth_compare( Compare comp )
{
	if( m_depth_compare != comp )
	{
		m_depth_compare = comp;
		unbind();
	}
}

void RenderState::colour_write( bool f )
{
	if( m_colour_write != f )
	{
		m_colour_write = f;
		unbind();
	}
}

void RenderState::draw_front( bool f )
{
	if( m_draw_front != f )
	{
		m_draw_front = f;
		unbind();
	}
}

void RenderState::draw_back( bool f )
{
	if( m_draw_back != f )
	{
		m_draw_back = f;
		unbind();
	}
}

RenderState::Ptr const &RenderState::stock_opaque()
{
	static Ptr opaque( new RenderState );
	return opaque;
}
