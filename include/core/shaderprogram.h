#ifndef SHADERPROGRAM_H
#define SHADERPROGRAM_H

#include "common/shared.h"
#include "core/uniform.h"

#include <vector>

class Texture;

class ShaderProgram : public Shared
{
public:
	typedef SharedPtr< ShaderProgram > Ptr;
	ShaderProgram( char const *vertex_source,
	               char const *fragment_source,
	               char const *geometry_source = 0,
	               char const *control_source = 0,
	               char const *evaluation_source = 0 );
	~ShaderProgram();

	void bind();
	void unbind() const;

	void bind_textures();

	void set( UniformBase const &uniform );
	void set( UniformGroup const &group );

	template< typename T > void set( char const *name, T const &value );

	static Ptr const &stock_unlit();

	// Only for use by Uniform class
	static void bind_texture_to_current_program( int unit, SharedPtr< Texture > texture );

private:
	int m_program;

	struct AttributeLocation
	{
		int location;
		int *shared_location;
	};
	std::vector< AttributeLocation > m_att_locations;
	struct UniformLocation
	{
		int location;
		int count;
		UniformInfo const *info;
	};
	std::vector< UniformLocation > m_uniform_locations;
	std::vector< SharedPtr< Texture > > m_bound_textures;
};


////////////////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////////////////

template< typename T >
void ShaderProgram::set( char const *name, T const &value )
{
	Uniform< T > u( name );
	u.data = value;
	set( u );
}


#endif //SHADERPROGRAM_H
