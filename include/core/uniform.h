#ifndef UNIFORM_H
#define UNIFORM_H

#include "common/shared.h" 
#include "core/texture.h"
#include "math/vec.h"

#include "opengl/opengl.h"

#include <algorithm>
#include <string>
#include <vector>

struct UniformInfo
{
    std::string name;
    int type;

	// These are set by ShaderProgram::bind() to reflect location etc. of the
	// uniform for that program.
	mutable int location;
    mutable int count;
	mutable int texture_unit;

    static UniformInfo const *get( char const *name, int type );

	// Provided so that UniformInfo objects can be stored in a map
	bool operator<( UniformInfo const &id2 ) const;
};



class UniformBase : public Shared
{
public:
	inline UniformBase( char const *name, int type, bool is_array );

protected:
	UniformInfo const * const info;

private:
	friend class ShaderProgram;
	friend class UniformGroup;
	virtual void bind() const = 0;
	const bool is_array;
};



template< typename T >
class Uniform : public UniformBase
{
public:
	Uniform( char const *name, T const &data = T() );

	T data;

private:
	virtual void bind() const;
};

class UniformGroup
{
public:
	template< typename T >
	void set( char const *name, T const &data );

private:
	friend class ShaderProgram;
	void bind() const;

	typedef SharedPtr< UniformBase > UniformPtr;
	UniformBase *get( char const *name, int type, bool is_array );
	std::vector< UniformPtr > m_uniforms;
};



////////////////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////////////////

// UniformType is a traits-type class, used to convert compile-time type
// information to opengl uniform type constants.
template< typename T >
struct UniformType {};

template<>           struct UniformType< int >              {static int type() {return GL_INT;} };
template<>           struct UniformType< float >            {static int type() {return GL_FLOAT;} };
template<>           struct UniformType< float2 >           {static int type() {return GL_FLOAT_VEC2;} };
template<>           struct UniformType< float3 >           {static int type() {return GL_FLOAT_VEC3;} };
template<>           struct UniformType< float4 >           {static int type() {return GL_FLOAT_VEC4;} };
template<>           struct UniformType< float22 >          {static int type() {return GL_FLOAT_MAT2;} };
template<>           struct UniformType< float33 >          {static int type() {return GL_FLOAT_MAT3;} };
template<>           struct UniformType< float44 >          {static int type() {return GL_FLOAT_MAT4;} };
template<>           struct UniformType< Texture2D::Ptr >   {static int type() {return GL_SAMPLER_2D;} };
template<>           struct UniformType< TextureCube::Ptr > {static int type() {return GL_SAMPLER_CUBE;} };
//template<typename T> struct UniformType< const std::vector< T > > {static int type() {return UniformType< T >::type();}};
template<typename T> struct UniformType< std::vector< T > > {static int type() {return UniformType< T >::type();}};

// UniformArrayInfo is another traits-type class, providing information on
// whether a uniform derived class is instanciated on a vector class or not.
template< typename T >
struct UniformArrayInfo { static int size( T const & ) {return 1;} static const bool is_array = false;};

template<typename T> struct UniformArrayInfo< std::vector< T > > {static int size(std::vector< T > const &v) {return v.size();} static const bool is_array = true;};

namespace UniformSetters
{
	inline void set( int location, float   const &data, int tex_unit, int size ) { glUniform1fv( location, size, &data ); }
	inline void set( int location, float2  const &data, int tex_unit, int size ) { glUniform2fv( location, size, &data.x );}
	inline void set( int location, float3  const &data, int tex_unit, int size ) { glUniform3fv( location, size, &data.x ); }
	inline void set( int location, float4  const &data, int tex_unit, int size ) { glUniform4fv( location, size, &data.x ); }
	inline void set( int location, float22 const &data, int tex_unit, int size ) { glUniformMatrix2fv( location, size, false, &data.i.x ); }
	inline void set( int location, float33 const &data, int tex_unit, int size ) { glUniformMatrix3fv( location, size, false, &data.i.x ); }
	inline void set( int location, float44 const &data, int tex_unit, int size ) { glUniformMatrix4fv( location, size, false, &data.i.x ); }
	inline void set( int location, int     const &data, int tex_unit, int size ) { glUniform1iv( location, size, &data ); }

	template< typename T >
	inline void set_texture( int location, T const &data, int tex_unit, int size )
	{
		if( location < 0 )
			return;

		if( size > 1 )
		{
			std::vector<int> tex_units( size );
			T const *tex = &data;
			for( int i = 0; i < size; ++i )
			{
				tex_units[i] = tex_unit + i;
				if( tex->get() )
					(*tex)->bind( tex_unit );
				++tex;
			}
			set( location, tex_units[0], 0, size );
		}
		else
		{
			set( location, tex_unit, 0, 1 );
			if( data.get() )
				data->bind( tex_unit );
		}
	}
	inline void set( int location, Texture2D::Ptr const &data, int tex_unit, int size )
	{
		set_texture( location, data, tex_unit, size );
	}
	inline void set( int location, TextureCube::Ptr const &data, int tex_unit, int size )
	{
		set_texture( location, data, tex_unit, size );
	}

	template< typename T >
	inline void set( int location, std::vector< T > const &data, int tex_unit, int size )
	{
		set( location, data[0], tex_unit, size );
	}
}

UniformBase::UniformBase( char const *name, int type, bool is_array )
	: info( UniformInfo::get( name, type ) ), is_array( is_array )
{
}

template< typename T >
Uniform< T >::Uniform( char const *name, T const &data )
	: UniformBase( name, UniformType< T >::type(), UniformArrayInfo< T >::is_array ),
	  data( data )
{
}

template< typename T >
void Uniform< T >::bind() const
{
	UniformSetters::set( info->location, data, info->texture_unit,
		                    std::min( UniformArrayInfo< T >::size(data), info->count ) );
}

template< typename T >
void UniformGroup::set( char const *name, T const &data )
{
	UniformBase *u = get( name, UniformType< T >::type(), UniformArrayInfo< T >::is_array );
	if( u )
		static_cast< Uniform< T > * >( u )->data = data;
	else
		m_uniforms.push_back( UniformPtr( new Uniform< T >( name, data ) ) );
}

#endif //UNIFORM_H