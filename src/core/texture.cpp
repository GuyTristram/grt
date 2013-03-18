#include "core/texture.h"
#include "opengl/opengl.h"

namespace
{
GLenum format_from_channel_count( int channels )
{
	GLenum format = GL_RGB;
	switch( channels )
	{
	case 1: return GL_RED;
	case 2: return GL_RG;
	case 3: return GL_RGB;
	case 4: return GL_RGBA;
	default: ;
		// \todo throw exception?
	}
	return GL_RGB;
}

void calc_format(
    int channels,
    char const *options,
    GLenum &type,
    GLenum &format,
    GLenum &int_format,
    GLint &wrap_s,
    GLint &wrap_t,
    GLint &min_filter,
    GLint &max_filter )
{
	type = GL_UNSIGNED_BYTE;
	format = format_from_channel_count( channels );
	int_format = format;
	wrap_s = GL_REPEAT;
	wrap_t = GL_REPEAT;
	min_filter = GL_LINEAR_MIPMAP_LINEAR;
	max_filter = GL_LINEAR;

	bool is_float = false;
	bool is_short = false;
	bool is_depth = false;
	while( options && *options )
	{
		if( *options == 'f' ) { is_float = true; }
		else if( *options == 'c' ) { wrap_s = GL_CLAMP_TO_EDGE; wrap_t = GL_CLAMP_TO_EDGE; }
		else if( *options == 'd' ) { is_depth = true; }
		else if( *options == 's' ) { is_short = true; }
		else if( *options == 'i' ) { type = GL_UNSIGNED_INT; int_format = GL_R32UI; format = GL_RED_INTEGER; }
		++options;
	}

	if( is_float )
	{
		type = GL_FLOAT;
		if( is_depth )
		{
			int_format = GL_DEPTH_COMPONENT32F;
			format = GL_DEPTH_COMPONENT;
		}
		else
		{
			switch( channels )
			{
			case 1: int_format = GL_R32F; break;
			case 2: int_format = GL_RG32F; break;
			case 3: int_format = GL_RGB32F; break;
			case 4: int_format = GL_RGBA32F; break;
			}
		}
	}
	else if( is_short )
	{
		type = GL_FLOAT;
		if( is_depth )
		{
			int_format = GL_DEPTH_COMPONENT16;
			format = GL_DEPTH_COMPONENT;
		}
		else
		{
			switch( channels )
			{
			case 1: int_format = GL_R16; break;
			case 2: int_format = GL_RG16; break;
			case 3: int_format = GL_RGB16; break;
			case 4: int_format = GL_RGBA16; break;
			}
		}
	}
	else
	{
		if( is_depth )
		{
			int_format = GL_DEPTH_COMPONENT;
			format = GL_DEPTH_COMPONENT;
		}
	}
}
}

Texture2D::Texture2D( int width, int height, int channels, void *data, char const *options )
	: PixelBuffer( width, height ), m_channels( channels )
{
	glGenTextures( 1, &m_id );
	glBindTexture( GL_TEXTURE_2D, m_id );

	GLenum type, format, int_format;
	GLint wrap_s, wrap_t, min_filter, max_filter;

	calc_format( channels, options,
	             type, format, int_format, wrap_s, wrap_t, min_filter, max_filter );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, max_filter );

	glTexImage2D( GL_TEXTURE_2D, 0, int_format, width, height, 0, format, type, data );

	glGenerateMipmap( GL_TEXTURE_2D );

}

Texture2D::~Texture2D()
{
	glDeleteTextures( 1, &m_id );
}

void Texture2D::bind( int unit )
{
	glActiveTexture( GL_TEXTURE0 + unit );
	//glEnable( GL_TEXTURE_2D );
	glBindTexture( GL_TEXTURE_2D, m_id );
}

void Texture2D::gen_mipmaps()
{
	glBindTexture( GL_TEXTURE_2D, m_id );
	glGenerateMipmap( GL_TEXTURE_2D );
}

TextureCube::TextureCube( int width, int height, int channels,
                          void *pos_x, void *neg_x,
                          void *pos_y, void *neg_y,
                          void *pos_z, void *neg_z,
                          char const *options )
	: PixelBuffer( width, height )
{
	glGenTextures( 1, &m_id );
	glBindTexture( GL_TEXTURE_CUBE_MAP, m_id );

	GLenum type, format, int_format;
	GLint wrap_s, wrap_t, min_filter, max_filter;

	calc_format( channels, options,
	             type, format, int_format,
	             wrap_s, wrap_t,
	             min_filter, max_filter );

	glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, int_format, width, height, 0, format, GL_UNSIGNED_BYTE, pos_x );
	glTexImage2D( GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, int_format, width, height, 0, format, GL_UNSIGNED_BYTE, neg_x );
	glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, int_format, width, height, 0, format, GL_UNSIGNED_BYTE, pos_y );
	glTexImage2D( GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, int_format, width, height, 0, format, GL_UNSIGNED_BYTE, neg_y );
	glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, int_format, width, height, 0, format, GL_UNSIGNED_BYTE, pos_z );
	glTexImage2D( GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, int_format, width, height, 0, format, GL_UNSIGNED_BYTE, neg_z );

	glGenerateMipmap( GL_TEXTURE_CUBE_MAP );
	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	//glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, min_filter );
	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE );
	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL );
	//glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_DEPTH_STENCIL_TEXTURE_MODE  , GL_LUMINANCE );

}

TextureCube::~TextureCube()
{
	glDeleteTextures( 1, &m_id );
}

void TextureCube::bind( int unit )
{
	glActiveTexture( GL_TEXTURE0 + unit );
	glEnable( GL_TEXTURE_CUBE_MAP );
	glBindTexture( GL_TEXTURE_CUBE_MAP, m_id );
}

void TextureCube::gen_mipmaps()
{
	glBindTexture( GL_TEXTURE_CUBE_MAP, m_id );
	//glGenerateMipmap( GL_TEXTURE_CUBE_MAP );
}

