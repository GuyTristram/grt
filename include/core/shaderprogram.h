#ifndef SHADERPROGRAM_H
#define SHADERPROGRAM_H

#include "common/shared.h"
#include "core/uniform.h"

#include <vector>

class ShaderProgram : public Shared
{
public:
	typedef SharedPtr< ShaderProgram > Ptr;
	ShaderProgram( char const *vertex_source, char const *fragment_source );
	~ShaderProgram();

	void bind() const;
	void unbind() const;

	void set( UniformBase const &uniform );
	void set( UniformGroup const &group );

	template< typename T > void set( char const *name, T const &value );

	static Ptr const &stock_unlit();

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
