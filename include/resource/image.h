#ifndef IMAGE_H
#define IMAGE_H

#include "math/vec2.h"
#include "math/vec4.h"
#include "common/shared.h"

#include <vector>

class ShaderProgram;
class RenderState;

class Image : public Shared
{
public:
	typedef SharedPtr< Image > Ptr;
	Image( int width, int height, int channels, void *data );

	uchar4 get( int x, int y );
	uchar4 get( int2 p ) { return get( p.x, p.y ); }

	int width() const { return m_width; }
	int height() const { return m_height; }
	int channels() const { return m_channels; }

private:
	int m_channels;
	int m_width;
	int m_height;
	std::vector< unsigned char > m_data;
};

#endif // IMAGE_H
