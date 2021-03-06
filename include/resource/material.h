#ifndef MATERIAL_H
#define MATERIAL_H

#include "core/uniform.h"
#include "core/renderstate.h"
#include "core/shaderprogram.h"

class Material : public Shared
{
public:
	typedef SharedPtr< Material > Ptr;
	explicit Material( ShaderProgram::Ptr const &program = ShaderProgram::stock_unlit(),
	                   RenderState state = RenderState::stock_opaque() );

	ShaderProgram::Ptr depth_program;
	ShaderProgram::Ptr geom_program;
	ShaderProgram::Ptr program;
	RenderState state;
	UniformGroup uniforms;

	void bind();
};

#endif //MATERIAL_H
