#include "core/renderstate.h"
#include "opengl/opengl.h"

RenderState::RenderState( BlendMode blend_mode )
	: m_blend_mode( blend_mode ),
	  m_draw_front( true ),
	  m_draw_back( false ),
	  m_depth_write( true ),
	  m_depth_test( true ),
	  m_colour_write( true )
{
}

void RenderState::bind()
{
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
		glDepthFunc( GL_LEQUAL );
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

void RenderState::blend_mode( BlendMode mode )
{
	m_blend_mode = mode;
}

void RenderState::depth_write( bool f )
{
	m_depth_write = f;
}

void RenderState::depth_test( bool f )
{
	m_depth_test = f;
}

void RenderState::colour_write( bool f )
{
	m_colour_write = f;
}

void RenderState::draw_front( bool f )
{
	m_draw_front = f;
}

void RenderState::draw_back( bool f )
{
	m_draw_back = f;
}

RenderState::Ptr const &RenderState::stock_opaque()
{
	static Ptr opaque( new RenderState );
	return opaque;
}
