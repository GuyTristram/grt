#ifndef INDEXBUFFER_H
#define INDEXBUFFER_H

#include "common/shared.h"

#include <algorithm>
#include <vector>

class IndexBuffer : public Shared
{
public:
	typedef SharedPtr< IndexBuffer > Ptr;
	explicit IndexBuffer( int count = 0, unsigned int *indices = 0, bool dynamic = false ) : m_dynamic( dynamic ), m_gl_buffer( 0 )
	{
		if( count ) m_indices.resize( count );
		if( count && indices ) std::copy( indices, indices + count, m_indices.begin() );
	}

	IndexBuffer &add( unsigned int index ) {m_indices.push_back( index ); return *this;}

	unsigned int *indices();

	int count() const {return m_indices.size();}
	void clear() {m_indices.clear();}

private:
	std::vector< unsigned int > m_indices;
	bool m_dynamic;

	unsigned int m_gl_buffer;
};

#endif // INDEXBUFFER_H
