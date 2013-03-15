#include "resource/material.h"

Material::Material( ShaderProgram::Ptr const &program,
                    RenderState::Ptr const &state )
	: program( program ), state( state ) {}

void Material::bind()
{
	if( program )
	{
		program->bind();
		program->set( uniforms );
	}
	if( state )
		state->bind();
}
