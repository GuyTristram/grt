#include "core/shaderprogram.h"
#include "core/vertexbuffer.h"
#include "core/uniform.h"
#include <cstdlib>
#include "opengl/opengl.h"

namespace
{

GLuint compile( GLenum type, char const *source )
{
	GLuint shader;
	shader = glCreateShader( type );
	glShaderSource( shader, 1, &source, 0 );
	glCompileShader( shader );

	GLint status;
	glGetShaderiv( shader, GL_COMPILE_STATUS, &status );
	if( status == 0 )
	{
		GLint log_length;
		glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &log_length );
		if( log_length > 0 )
		{
			GLchar *log = ( GLchar * )malloc( log_length );
			glGetShaderInfoLog( shader, log_length, &log_length, log );
			printf( log );
			free( log );
		}
		glDeleteShader( shader );
		return 0;
	}
	return shader;
}

bool is_texture_type( int type )
{
	return
		type == GL_SAMPLER_2D ||
		type == GL_SAMPLER_2D_ARRAY ||
		type == GL_SAMPLER_3D ||
		type == GL_SAMPLER_CUBE;
}

ShaderProgram *g_last_shader = 0;
}

ShaderProgram::ShaderProgram( char const *vertex_source,
                              char const *fragment_source,
                              char const *geometry_source,
	                          char const *control_source,
	                          char const *evaluation_source ) : m_program( 0 )
{
	GLuint vert_shader = compile( GL_VERTEX_SHADER, vertex_source );
	GLuint frag_shader = compile( GL_FRAGMENT_SHADER, fragment_source );
	GLuint geom_shader = geometry_source   ? compile( GL_GEOMETRY_SHADER, geometry_source ) : 0;
	GLuint ctrl_shader = control_source    ? compile( GL_TESS_CONTROL_SHADER, control_source ) : 0;
	GLuint eval_shader = evaluation_source ? compile( GL_TESS_EVALUATION_SHADER, evaluation_source ) : 0;

	if( vert_shader && frag_shader )
	{
		m_program = glCreateProgram();
		glAttachShader( m_program, vert_shader );
		glAttachShader( m_program, frag_shader );
		if( geom_shader )
			glAttachShader( m_program, geom_shader );
		if( ctrl_shader )
			glAttachShader( m_program, ctrl_shader );
		if( eval_shader )
			glAttachShader( m_program, eval_shader );

		glLinkProgram( m_program );

		GLint status;
		glGetProgramiv( m_program, GL_LINK_STATUS, &status );
		if( status == 0 )
		{
			GLint log_length;
			glGetProgramiv( m_program, GL_INFO_LOG_LENGTH, &log_length );
			if( log_length > 0 )
			{
				GLchar *log = ( GLchar * )malloc( log_length );
				glGetProgramInfoLog( m_program, log_length, &log_length, log );
				printf( log );
				//NSLog(@"Program link log:\n%s", log);
				free( log );
			}
			glDeleteProgram( m_program );
			m_program = 0;
		}
		else
		{

			// Get vertex attribute info.
			GLint att_count;
			glGetProgramiv( m_program, GL_ACTIVE_ATTRIBUTES, &att_count );
			for( GLint i = 0; i < att_count; ++i )
			{
				GLchar name[256];
				GLint size;
				GLenum type;
				glGetActiveAttrib( m_program, i, 256, 0, &size, &type, name );

				m_att_locations.push_back( AttributeLocation() );
				m_att_locations.back().location = glGetAttribLocation( m_program, name );
				m_att_locations.back().shared_location =
				    VertexBuffer::attribute_location( name );
			}

			// Get uniform info. Temporarily set this as current program
			int current_program;
			glGetIntegerv( GL_CURRENT_PROGRAM, &current_program );
			glUseProgram( m_program );

			GLint uniform_count;
			glGetProgramiv( m_program, GL_ACTIVE_UNIFORMS, &uniform_count );
			int texture_count = 0;
			for( GLint i = 0; i < uniform_count; ++i )
			{
				GLchar name[256];
				GLint size;
				GLenum type;
				glGetActiveUniform( m_program, i, 256, 0, &size, &type, name );
				int location = glGetUniformLocation( m_program, name );

				if( type == GL_SAMPLER_CUBE_SHADOW ) type = GL_SAMPLER_CUBE;

				if( is_texture_type( type ) )
				{
					glUniform1i( location, texture_count );
					texture_count += size;
				}

				m_uniform_locations.push_back( UniformLocation() );
				m_uniform_locations.back().location = location;
				m_uniform_locations.back().count = size;
				m_uniform_locations.back().info = UniformInfo::get( name, type );
			}
			glUseProgram( current_program );
			m_bound_textures.resize( texture_count );
		}
	}
	// Get uniform locations.
	//uniforms[UNIFORM_TRANSLATE] = glGetUniformLocation(program, "translate");

	// Release vertex and fragment shaders.
	if( vert_shader )
		glDeleteShader( vert_shader );
	if( frag_shader )
		glDeleteShader( frag_shader );
}

ShaderProgram::~ShaderProgram()
{
	if( g_last_shader == this )
	{
		unbind();
		g_last_shader = 0;
	}
	if( m_program )
		glDeleteProgram( m_program );
}

void ShaderProgram::set( UniformBase const &uniform )
{
	bind();
	uniform.bind();
}

void ShaderProgram::set( UniformGroup const &group )
{
	bind();
	group.bind();
}


void ShaderProgram::bind()
{
	if( g_last_shader == this )
		return;
	if( g_last_shader )
		g_last_shader->unbind();
	g_last_shader = this;
	glUseProgram( m_program );
	int texture_unit = 0;
	for( int i = 0; i != m_att_locations.size(); ++i )
		*m_att_locations[i].shared_location = m_att_locations[i].location;
	for( int i = 0; i != m_uniform_locations.size(); ++i )
	{
		m_uniform_locations[i].info->location = m_uniform_locations[i].location;
		m_uniform_locations[i].info->count = m_uniform_locations[i].count;
		if( is_texture_type( m_uniform_locations[i].info->type ) )
		{
			m_uniform_locations[i].info->texture_unit = texture_unit;
			texture_unit += m_uniform_locations[i].count;
		}
	}
}

void ShaderProgram::unbind() const
{
	glUseProgram( 0 );
	for( int i = 0; i != m_att_locations.size(); ++i )
		*m_att_locations[i].shared_location = -1;
	for( int i = 0; i != m_uniform_locations.size(); ++i )
		m_uniform_locations[i].info->location = -1;
}

void ShaderProgram::bind_textures()
{
	for( int i = 0; i < m_bound_textures.size(); ++i )
		if( m_bound_textures[i] )
			m_bound_textures[i]->bind( i );
}

void ShaderProgram::bind_texture_to_current_program( int unit, SharedPtr< Texture > texture )
{
	if( g_last_shader && unit < (int)g_last_shader->m_bound_textures.size() )
	{
		g_last_shader->m_bound_textures[unit] = texture;
	}
}

ShaderProgram::Ptr const &ShaderProgram::stock_unlit()
{

	static Ptr unlit( new ShaderProgram(
	                      "precision mediump float;\n"
	                      "uniform mat4 u_t_model_view_projection;\n"
	                      "uniform mat3 u_t_normal;\n"
	                      "attribute vec4 a_position;\n"
	                      "attribute vec3 a_normal;\n"
	                      "attribute vec2 a_uv0;\n"
	                      "attribute vec4 a_colour;\n"
	                      "varying vec4 v_colour;\n"
	                      "varying vec3 v_normal;\n"
	                      "varying vec2 v_uv;\n"
	                      "void main()\n"
	                      "{\n"
	                      "  v_colour = vec4(1.0,1.0,1.0,1.0);\n"
	                      "  v_uv = a_uv0;\n"
	                      "  v_normal = u_t_normal * a_normal;\n"
	                      "  gl_Position = u_t_model_view_projection * a_position;\n"
	                      "}\n",
	                      "precision mediump float;\n"
	                      "uniform vec4 u_colour;\n"
	                      "uniform sampler2D u_texture;\n"
	                      "varying vec4 v_colour;\n"
	                      "varying vec3 v_normal;\n"
	                      "varying vec2 v_uv;\n"
	                      "void main()\n"
	                      "{\n"
	                      "  //gl_FragColor = texture2D( u_texture, v_uv) * v_colour * u_colour;\n"
	                      "  //gl_FragColor = texture2D( u_texture, v_uv) * dot( v_normal, vec3( 0.0, 0.0, 1.0 ) );\n"
	                      "  gl_FragColor = texture2D( u_texture, v_uv);\n"
	                      "  gl_FragColor.a = 1.0;\n"
	                      " // gl_FragColor = vec4( v_uv, 0.0, 1.0 );\n"
	                      "}\n" ) );

	return unlit;
}
