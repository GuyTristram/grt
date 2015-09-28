#include "core/vertexbuffer.h"

#include <map>
#include <string>

namespace
{
std::map< std::string, int > g_attribute_locations;
}

VertexBuffer::~VertexBuffer()
{
	if( m_static_data )
		free( m_static_data );

	if( m_dynamic_data )
		free( m_dynamic_data );

	if( m_gl_buffer )
		glDeleteBuffers( 1, &m_gl_buffer );
}


void VertexBuffer::add_attribute( char const *name, int type, int count, int size, bool dynamic, bool normalize, int div )
{
	m_attributes.push_back( Attribute( attribute_location( name ),
	                                   type, count,
	                                   dynamic ? m_dynamic_vertex_size : m_static_vertex_size,
	                                   dynamic, normalize, div ) );

	if( dynamic )
		m_dynamic_vertex_size += size;
	else
		m_static_vertex_size += size;
}


void VertexBuffer::bind()
{
	commit();

	if( m_static_vertex_size )
	{
		glBindBuffer( GL_ARRAY_BUFFER, m_gl_buffer );
		for( auto att = m_attributes.begin(); att != m_attributes.end(); ++att )
		{
			if( *att->location >= 0 && !att->dynamic )
			{
				glEnableVertexAttribArray( *att->location );
				glVertexAttribPointer( *att->location, att->count, att->type,
				                       att->normalize, m_static_vertex_size,
				                       reinterpret_cast< GLvoid * >( att->offset ) );
				glVertexAttribDivisor( *att->location, att->divisor );
			}
		}
		glBindBuffer( GL_ARRAY_BUFFER, 0 );
	}

	if( m_dynamic_vertex_size )
	{
		for( auto att = m_attributes.begin(); att != m_attributes.end(); ++att )
		{
			if( *att->location >= 0 && att->dynamic )
			{
				glEnableVertexAttribArray( *att->location );
				if( att->type == GL_UNSIGNED_INT )
					glVertexAttribIPointer( *att->location, att->count, att->type,
						m_dynamic_vertex_size,
						m_dynamic_data + att->offset );
				else
					glVertexAttribPointer( *att->location, att->count, att->type,
						att->normalize, m_dynamic_vertex_size,
						m_dynamic_data + att->offset );
				glVertexAttribDivisor( *att->location, att->divisor );
			}
		}
	}
}

void VertexBuffer::unbind()
{
	for( auto att = m_attributes.begin(); att != m_attributes.end(); ++att )
		if( *att->location >= 0 )
		{
			glDisableVertexAttribArray( *att->location );
			if( att->divisor )
				glVertexAttribDivisor( *att->location, 0 );
		}
}


void VertexBuffer::reserve()
{
	if( m_reserved )
		return;

	if( m_static_vertex_size )
		m_static_data = reinterpret_cast< unsigned char * >( malloc( m_static_vertex_size * m_vertex_count ) );

	if( m_dynamic_vertex_size )
		m_dynamic_data = reinterpret_cast< unsigned char * >( malloc( m_dynamic_vertex_size * m_vertex_count ) );

	m_reserved = true;
}

void VertexBuffer::set_static_data( void *data, int size )
{
	if( m_static_data || m_gl_buffer )
	{
		//TODO handle error
	}
	else
	{
		glGenBuffers( 1, &m_gl_buffer );
		glBindBuffer( GL_ARRAY_BUFFER, m_gl_buffer );
		glBufferData( GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW );
		glBindBuffer( GL_ARRAY_BUFFER, 0 );
	}
}

void VertexBuffer::commit()
{
	if( m_static_data )
	{
		glGenBuffers( 1, &m_gl_buffer );
		glBindBuffer( GL_ARRAY_BUFFER, m_gl_buffer );
		glBufferData( GL_ARRAY_BUFFER, m_static_vertex_size * m_vertex_count, m_static_data, GL_STATIC_DRAW );
		free( m_static_data );
		m_static_data = 0;
		glBindBuffer( GL_ARRAY_BUFFER, 0 );
	}
}

int *VertexBuffer::attribute_location( char const *name )
{
	std::string str_name( name );
	auto loc = g_attribute_locations.find( str_name );
	if( loc != g_attribute_locations.end() )
		return &loc->second;

	int *new_loc = &g_attribute_locations[ str_name ];
	*new_loc = -1;
	return new_loc;
}

int VertexBuffer::gl_typesize( int gl_type )
{
	switch( gl_type )
	{
	case GL_BYTE:
		return 1;
	case GL_UNSIGNED_BYTE:
		return 1;
	case GL_SHORT:
		return 2;
	case GL_UNSIGNED_SHORT:
		return 2;
	case GL_FLOAT:
		return 4;
	default:
		return 0;
	}
}
