// Declares VertexBuffer class

#include "common/shared.h"
#include "math/vec.h"
#include "opengl/opengl.h"

#include <vector>
#include <string>
#include <cstdlib>
#include <type_traits>

template< typename T >
struct VertexType
{
};

#define GRT_GL_TYPE( type_in, gl_type ) template<> struct VertexType< type_in > {static const int type = gl_type; static const int count = 1;}; \
     template<int N> struct VertexType< type_in[N] > {static const int type = gl_type; static const int count = N;};


GRT_GL_TYPE( signed char, GL_BYTE )
GRT_GL_TYPE( char, GL_BYTE )
GRT_GL_TYPE( unsigned char, GL_UNSIGNED_BYTE )
GRT_GL_TYPE( short int, GL_SHORT )
GRT_GL_TYPE( unsigned short int, GL_UNSIGNED_SHORT )
GRT_GL_TYPE( int, GL_INT )
GRT_GL_TYPE( unsigned int, GL_UNSIGNED_INT )
GRT_GL_TYPE( float, GL_FLOAT )
GRT_GL_TYPE( double, GL_DOUBLE )
template<typename T> struct VertexType< vec2<T> >
{
	static const int type = VertexType<T>::type;
	static const int count = 2;
};
template<typename T> struct VertexType< vec3<T> >
{
	static const int type = VertexType<T>::type;
	static const int count = 3;
};
template<typename T> struct VertexType< vec4<T> >
{
	static const int type = VertexType<T>::type;
	static const int count = 4;
};

class VertexBuffer;

template< typename T >
class VertexAttribute
{
public:
	VertexAttribute() : m_index( 0 ) {}
	VertexAttribute( VertexBuffer *vb, int index ) : m_buffer( vb ), m_index( index ) {}

	class Iterator
	{
	public:
		Iterator( unsigned char *p = 0, int stride = 0 ) : m_p( p ), m_stride( stride ) {}

		T &operator*() const {return *reinterpret_cast< T* >( m_p );}
		T *operator->() const {return reinterpret_cast< T* >( m_p );}

		Iterator &operator++() {m_p += m_stride; return *this;}
		Iterator operator++( int ) { Iterator temp( *this ); m_p += m_stride; return temp;}

	private:
		unsigned char *m_p;
		int m_stride;
	};

	Iterator begin();

private:
	SharedPtr< VertexBuffer > m_buffer;
	int m_index;
};

class VertexBuffer : public Shared
{
public:
	typedef SharedPtr< VertexBuffer > Ptr;
	VertexBuffer() : m_gl_buffer( 0 ), m_vertex_count( 0 ),
		m_static_vertex_size( 0 ), m_dynamic_vertex_size( 0 ),
        m_static_data( 0 ), m_dynamic_data( 0 ),
		m_reserved( false ) {}
	~VertexBuffer();

	void add_attribute( char const *name, int type, int count, int size, bool dyn = false, bool norm = true, int div = 0 );
	void set_static_data( void *data, int size );

	template< typename T >
	VertexAttribute<T> add_attribute( char const *name, bool dyn = false, bool norm = true, int div = 0 )
	{
		add_attribute( name, VertexType< T >::type, VertexType< T >::count, sizeof( T ), dyn, norm, div );
		return VertexAttribute<T>( this, m_attributes.size() - 1 );
	}

	void vertex_count( int count ) { m_vertex_count = count; }
	int vertex_count() const { return m_vertex_count; }

	template< typename T >
	typename VertexAttribute< T >::Iterator attribute_begin( int index )
	{
		reserve();
		Attribute &att = m_attributes[ index ];
		if( att.dynamic )
			return typename VertexAttribute< T >::Iterator( m_dynamic_data + att.offset, m_dynamic_vertex_size );
		else
			return typename VertexAttribute< T >::Iterator( m_static_data + att.offset, m_static_vertex_size );
	}

	void bind();
	void unbind();

	static int *attribute_location( char const *name );

private:
	static int gl_typesize( int gl_type );
	void reserve();
	void commit();

	struct Attribute
	{
		Attribute( int *location, int type, int count, int offset, bool dyn, bool norm, int div ) :
		location( location ), type( type ), count( count ), offset( offset ), dynamic( dyn ), normalize( norm ), divisor( div ){}
		int *location;
		int type;
		int count;
		int offset;
		bool dynamic;
		bool normalize;
		int divisor;
	};

	std::vector< Attribute > m_attributes;

	GLuint m_gl_buffer;

	int m_vertex_count;

	int m_static_vertex_size;
	int m_dynamic_vertex_size;

	unsigned char *m_static_data;
	unsigned char *m_dynamic_data;

	bool m_reserved;
};

template< typename T >
typename VertexAttribute<T>::Iterator VertexAttribute<T>::begin()
{
	return m_buffer->attribute_begin<T>( m_index );
}

