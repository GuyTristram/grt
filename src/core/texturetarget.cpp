#include "core/texturetarget.h"
#include "core/texture.h"
#include "opengl/opengl.h"
#include <cstdio>

namespace
{
GLenum fromFormat( RenderBuffer::Format format )
{
	switch( format )
	{
	case RenderBuffer::RGBA_4444: return GL_RGBA4;
		//case RenderBuffer::RGB_565:   return GL_RGB565;
	case RenderBuffer::RGBA_5551: return GL_RGB5_A1;
	case RenderBuffer::Depth:     return GL_DEPTH_COMPONENT16;
	case RenderBuffer::Stencil:   return GL_STENCIL_INDEX8;
	}
	return 0;
};
}

RenderBuffer::RenderBuffer( int width, int height, Format format )
	: PixelBuffer( width, height )
{
	glGenRenderbuffers( 1, &m_id );
	if( m_id )
	{
		glBindRenderbuffer( GL_RENDERBUFFER, m_id );
		glRenderbufferStorage( GL_RENDERBUFFER, fromFormat( format ), width, height );
	}
}

RenderBuffer::~RenderBuffer()
{
	if( m_id )
		glDeleteRenderbuffers( 1, &m_id );
}

TextureTarget::TextureTarget()
{
	glGenFramebuffers( 1, &m_id );
}

TextureTarget::~TextureTarget()
{
	if( m_id )
		glDeleteFramebuffers( 1, &m_id );
}

bool TextureTarget::is_complete()
{
	bind();
	GLenum res = glCheckFramebufferStatus( GL_FRAMEBUFFER );
	switch( res )
	{
	case GL_FRAMEBUFFER_COMPLETE:
		return true;

	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
		printf( "[ERROR] Framebuffer incomplete: Attachment is NOT complete.\n" );
		return false;

	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
		printf( "[ERROR] Framebuffer incomplete: No image is attached to FBO.\n" );
		return false;
		/*
		    case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS:
		        printf( "[ERROR] Framebuffer incomplete: Attached images have different dimensions.\n" );
		        return false;

		    case GL_FRAMEBUFFER_INCOMPLETE_FORMATS:
		        printf( "[ERROR] Framebuffer incomplete: Color attached images have different internal formats.\n" );
		        return false;
		*/
	case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
		printf( "[ERROR] Framebuffer incomplete: Draw buffer.\n" );
		return false;

	case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
		printf( "[ERROR] Framebuffer incomplete: Read buffer.\n" );
		return false;

	case GL_FRAMEBUFFER_UNSUPPORTED:
		printf( "[ERROR] Framebuffer incomplete: Unsupported by FBO implementation.\n" );
		return false;

	default:
		printf( "[ERROR] Framebuffer incomplete: Unknown error.\n" );
		return false;
	}
}

namespace
{
GLenum attachment( TextureTarget::BufferType type, int position )
{
	switch( type )
	{
	case TextureTarget::Colour:  return GL_COLOR_ATTACHMENT0 + position;
	case TextureTarget::Depth:   return GL_DEPTH_ATTACHMENT;
	case TextureTarget::Stencil: return GL_STENCIL_ATTACHMENT;
	}
	return 0;
}

int index( TextureTarget::BufferType type, int position )
{
	switch( type )
	{
	case TextureTarget::Depth:   return 0;
	case TextureTarget::Stencil: return 1;
	case TextureTarget::Colour:  return 2 + position;
	}
	return 0;
}
}

int TextureTarget::width() const
{
	for( int i = 0; i < 2 + MAX_COLOUR_BUFFERS; ++i )
		if( m_buffer[i].get() )
			return m_buffer[i]->width();
	return 0;
}

int TextureTarget::height() const
{
	for( int i = 0; i < 2 + MAX_COLOUR_BUFFERS; ++i )
		if( m_buffer[i].get() )
			return m_buffer[i]->height();
	return 0;
}

void TextureTarget::attach( SharedPtr<Texture2D> const &texture, BufferType type, int position )
{
	bind();
	GLuint id = texture.get() ? texture->m_id : 0;
	glFramebufferTexture2D( GL_FRAMEBUFFER, attachment( type, position ), GL_TEXTURE_2D, id, 0 );
	m_buffer[ index( type, position ) ] = texture;
	unbind();
}

void TextureTarget::attach( SharedPtr<Texture2DArray> const &texture, int ind, BufferType type, int position )
{
	bind( );
	GLuint id = texture.get( ) ? texture->m_id : 0;
	glFramebufferTextureLayer( GL_FRAMEBUFFER, attachment( type, position ), id, 0, ind );
	m_buffer[index( type, position )] = texture;
	unbind( );
}

void TextureTarget::attach( SharedPtr<Texture2DArray> const &texture, BufferType type, int position )
{
	bind( );
	GLuint id = texture.get( ) ? texture->m_id : 0;
	glFramebufferTexture( GL_FRAMEBUFFER, attachment( type, position ), id, 0 );
	m_buffer[index( type, position )] = texture;
	unbind( );
}

void TextureTarget::attach( SharedPtr<TextureCube> const &texture, int face, BufferType type, int position )
{
	bind();
	GLuint id = texture.get() ? texture->m_id : 0;
	GLenum target[6] = { GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
	                     GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
	                     GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
	                   };
	glFramebufferTexture2D( GL_FRAMEBUFFER, attachment( type, position ), target[ face ], id, 0 );
	m_buffer[ index( type, position ) ] = texture;
	unbind();
}

void TextureTarget::attach( SharedPtr<RenderBuffer> const &buffer, BufferType type, int position )
{
	bind();
	GLuint id = buffer.get() ? buffer->m_id : 0;
	glFramebufferRenderbuffer( GL_FRAMEBUFFER, attachment( type, position ), GL_RENDERBUFFER, id );
	m_buffer[ index( type, position ) ] = buffer;
	unbind();
}

void TextureTarget::do_bind()
{
	glBindFramebuffer( GL_FRAMEBUFFER, m_id );
	GLsizei n = 0;
	GLenum buffers[MAX_COLOUR_BUFFERS];
	for( int i = 0; i < MAX_COLOUR_BUFFERS; ++i )
		if( m_buffer[index( Colour, i )] )
			buffers[n++] = GL_COLOR_ATTACHMENT0 + i;
	glDrawBuffers( n, buffers );
}
