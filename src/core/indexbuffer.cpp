#include "core/indexbuffer.h"
#include "opengl/opengl.h"


unsigned int *IndexBuffer::indices()
{
	if( m_dynamic )
	{
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER , 0 );
		return &m_indices[0];
	}

	if( !m_gl_buffer )
	{
		glGenBuffers( 1, &m_gl_buffer );
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER , m_gl_buffer );
		glBufferData( GL_ELEMENT_ARRAY_BUFFER , m_indices.size() * 4, &m_indices[0], GL_STATIC_DRAW );
	}
	else
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER , m_gl_buffer );
	return 0;
}
