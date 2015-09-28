#include "core/renderstate.h"
#include "opengl/opengl.h"

namespace
{
	RenderState g_current_renderstate;
}

RenderState::RenderState( BlendMode blend_mode_in )
{
    blend_mode( blend_mode_in );
    depth_compare( Compare::LEqual );
    draw_front( true );
    draw_back( false );
    depth_write( true );
    depth_test( true );
    colour_write( true );
}

void RenderState::bind()
{
    static bool first = true;
	if( g_current_renderstate.m_pack == m_pack && !first )
		return;

    if( first || differ<Property::BlendMode >( g_current_renderstate ) )
    {
        if( get<Property::BlendMode>() == BlendMode::Opaque )
            glDisable( GL_BLEND );
        else
            glEnable( GL_BLEND );

        switch( get<Property::BlendMode>() )
	    {
        case BlendMode::Transparent:
		    glBlendFunc( GL_ONE, GL_ONE_MINUS_SRC_ALPHA );
		    break;
        case BlendMode::Blend:
		    glBlendFuncSeparate( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE );
		    //glBlendFunc( GL_SRC_ALPHA_SATURATE, GL_ONE_MINUS_SRC_ALPHA );
		    break;
        case BlendMode::Add:
		    glBlendFunc( GL_ONE, GL_ONE );
		    break;
	    }
    }

    if( first || differ<Property::ColourWrite >( g_current_renderstate ) )
    {
        bool b = get<Property::ColourWrite>();
        glColorMask( b, b, b, b );
    }

    if( first || differ<Property::DepthWrite >( g_current_renderstate ) )
    {
        if( get<Property::DepthWrite>() )
            glDepthMask( GL_TRUE );
        else
            glDepthMask( GL_FALSE );
    }

    if( first || differ<Property::DepthTest >( g_current_renderstate )
        || differ<Property::DepthCompare >( g_current_renderstate ) )
    {
        if( get<Property::DepthTest>() )
        {
            glEnable( GL_DEPTH_TEST );
            GLenum f;
            switch( get<Property::DepthCompare>() )
            {
            case Compare::Never:    f = GL_NEVER; break;
            case Compare::Less:     f = GL_LESS; break;
            case Compare::Equal:    f = GL_EQUAL; break;
            case Compare::LEqual:   f = GL_LEQUAL; break;
            case Compare::Greater:  f = GL_GREATER; break;
            case Compare::NotEqual: f = GL_NOTEQUAL; break;
            case Compare::GEqual:   f = GL_GEQUAL; break;
            case Compare::Always:   f = GL_ALWAYS; break;
            }
            glDepthFunc( f );
        }
        else
            glDisable( GL_DEPTH_TEST );
    }

    if( first || differ<Property::DrawFront >( g_current_renderstate )
        || differ<Property::DrawBack >( g_current_renderstate ) )
    {
        if( get<Property::DrawFront>() && get<Property::DrawBack>() )
            glDisable( GL_CULL_FACE );
        else
            glEnable( GL_CULL_FACE );

        if( get<Property::DrawFront>() )
        {
            if( !get<Property::DrawBack>() )
                glCullFace( GL_BACK );
        }
        else
        {
            if( get<Property::DrawBack>() )
                glCullFace( GL_FRONT );
            else
                glCullFace( GL_FRONT_AND_BACK );
        }
    }

    first = false;
    g_current_renderstate.m_pack = m_pack;
}


void RenderState::blend_mode( BlendMode mode )
{
    set<Property::BlendMode>( mode );
}

void RenderState::depth_write( bool f )
{
    set<Property::DepthWrite>( f );
}

void RenderState::depth_test( bool f )
{
    set<Property::DepthTest>( f );
}

void RenderState::depth_compare( Compare comp )
{
    set<Property::DepthCompare>( comp );
}

void RenderState::colour_write( bool f )
{
    set<Property::ColourWrite>( f );
}

void RenderState::draw_front( bool f )
{
    set<Property::DrawFront>( f );
}

void RenderState::draw_back( bool f )
{
    set<Property::DrawBack>( f );
}

RenderState RenderState::stock_opaque()
{
	return RenderState();
}
