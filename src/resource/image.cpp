#include "resource/image.h"
#include <algorithm>

Image::Image( int width, int height, int channels, void *data )
	: m_width( width ), m_height( height ), m_channels( channels )
{
	unsigned int size = width * height * channels;
	m_data.resize( size );
	std::copy( (unsigned char *)data, (unsigned char *)data + size, m_data.begin() );
}


uchar4 Image::get( int x, int y )
{
	uchar4 res( 0, 0, 0, 0 );
	int index = ( y * m_width + x ) * m_channels;

	for( int i = 0; i < m_channels; ++i )
		res[i] = m_data[ index++ ];

	return res;
}
