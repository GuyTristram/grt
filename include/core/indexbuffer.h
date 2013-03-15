#ifndef INDEXBUFFER_H
#define INDEXBUFFER_H

#include "common/shared.h"

#include <algorithm>
#include <vector>

class IndexBuffer : public Shared
{
public:
	typedef SharedPtr< IndexBuffer > Ptr;
	explicit IndexBuffer( int count = 0, unsigned short *indices = 0 )
	{
		if( count ) m_indices.resize( count );
		if( count && indices ) std::copy( indices, indices + count, m_indices.begin() );
	}

	IndexBuffer &add( unsigned short index ) {m_indices.push_back( index ); return *this;}

	unsigned short *indices() {return &m_indices[0];}

	int count() const {return m_indices.size();}
	void clear() {m_indices.clear();}

private:
	std::vector< unsigned short > m_indices;
};

#endif // INDEXBUFFER_H
