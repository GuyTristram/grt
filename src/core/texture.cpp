#include "core/texture.h"
#include "opengl/opengl.h"

namespace
{

void tex_params( GLenum target, int channels, char const *options,
                 GLenum &int_format, GLenum &format, GLenum &type, bool &is_mipmapped )
{
	type = GL_UNSIGNED_BYTE;
	GLint wrap_s = GL_REPEAT;
	GLint wrap_t = GL_REPEAT;
	GLint min_filter = GL_LINEAR_MIPMAP_LINEAR;
	GLint max_filter = GL_LINEAR;

	bool is_compare = false;
	while( options && *options )
	{
		if( *options == 'f' )      { type = GL_FLOAT; }
		else if( *options == 'c' ) { wrap_s = GL_CLAMP_TO_EDGE; wrap_t = GL_CLAMP_TO_EDGE; }
		else if( *options == 'd' ) { channels = 0; }
		else if( *options == 's' ) { type = GL_UNSIGNED_SHORT; }
		else if( *options == 'i' ) { type = GL_UNSIGNED_INT; }
		else if( *options == 'l' ) { max_filter = GL_LINEAR; }
		else if( *options == 'n' ) { max_filter = GL_NEAREST; }
		else if( *options == '0' ) { min_filter = GL_NEAREST; }
		else if( *options == '1' ) { min_filter = GL_LINEAR; }
		else if( *options == '2' ) { min_filter = GL_NEAREST_MIPMAP_NEAREST; }
		else if( *options == '3' ) { min_filter = GL_LINEAR_MIPMAP_NEAREST; }
		else if( *options == '4' ) { min_filter = GL_NEAREST_MIPMAP_LINEAR; }
		else if( *options == '5' ) { min_filter = GL_LINEAR_MIPMAP_LINEAR; }
		else if( *options == 'p' ) { is_compare = true; }
		++options;
	}

	static const GLenum formats[5]       = {GL_DEPTH_COMPONENT,    GL_RED,   GL_RG,     GL_RGB,     GL_RGBA};
	static const GLenum float_formats[5] = {GL_DEPTH_COMPONENT32F, GL_R32F,  GL_RG32F,  GL_RGB32F,  GL_RGBA32F};
	static const GLenum short_formats[5] = {GL_DEPTH_COMPONENT16,  GL_R16,   GL_RG16,   GL_RGB16,   GL_RGBA16};
	static const GLenum int_formats[5]   = {GL_DEPTH_COMPONENT32,  GL_R32UI, GL_RG32UI, GL_RGB32UI, GL_RGBA32UI};

	format = formats[channels];
	if( type == GL_FLOAT )
		int_format = float_formats[channels];
	else if( type == GL_UNSIGNED_SHORT )
		int_format = short_formats[channels];
	else if( type == GL_UNSIGNED_INT )
		int_format = int_formats[channels];
	else
		int_format = format;

	glTexParameteri( target, GL_TEXTURE_WRAP_S, wrap_s );
	glTexParameteri( target, GL_TEXTURE_WRAP_T, wrap_t );
	glTexParameteri( target, GL_TEXTURE_MIN_FILTER, min_filter );
	glTexParameteri( target, GL_TEXTURE_MAG_FILTER, max_filter );
	glTexParameteri( target, GL_TEXTURE_COMPARE_MODE, GL_NONE );

	if( is_compare )
	{
		glTexParameteri( target, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE );
		glTexParameteri( target, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL );
	}
	else
		glTexParameteri( target, GL_TEXTURE_COMPARE_MODE, GL_NONE );

	is_mipmapped = min_filter != GL_NEAREST && min_filter != GL_LINEAR;

}
}

Texture::Texture( int target, int width, int height, int channels )
	: PixelBuffer( width, height ), m_target( target ), m_channels( channels )
{
	glGenTextures( 1, &m_id );
	glBindTexture( m_target, m_id );
}

Texture::~Texture()
{
	glDeleteTextures( 1, &m_id );
}

void Texture::bind( int unit )
{
	glActiveTexture( GL_TEXTURE0 + unit );
	glBindTexture( m_target, m_id );
}

int Texture::channels() const 
{
	return m_channels;
}


void Texture::gen_mipmaps()
{
	glBindTexture( m_target, m_id );
	glGenerateMipmap( m_target );
}


Texture2D::Texture2D( int width, int height, int channels, void *data, char const *options )
	: Texture( GL_TEXTURE_2D, width, height, channels )
{

	GLenum type, format, int_format;
	bool is_mipmapped;

	tex_params( GL_TEXTURE_2D, channels, options,
                int_format, format, type, is_mipmapped );

	glTexImage2D( GL_TEXTURE_2D, 0, int_format, width, height, 0, format, type, data );

	if( is_mipmapped )//&& data )
		glGenerateMipmap( GL_TEXTURE_2D );
	glBindTexture( GL_TEXTURE_2D, 0 );

}


Texture2DArray::Texture2DArray( int width, int height, int size, int channels, void *data, char const *options )
	: Texture( GL_TEXTURE_2D_ARRAY, width, height, channels )
{

	GLenum type, format, int_format;
	bool is_mipmapped;

	tex_params( GL_TEXTURE_2D_ARRAY, channels, options,
                int_format, format, type, is_mipmapped );

	glTexImage3D( GL_TEXTURE_2D_ARRAY, 0, int_format, width, height, size, 0, format, type, data );

	if( is_mipmapped )//&& data )
		glGenerateMipmap( GL_TEXTURE_2D_ARRAY );
	glBindTexture( GL_TEXTURE_2D_ARRAY, 0 );

}

Texture3D::Texture3D( int width, int height, int depth, int channels, void *data, char const *options )
	: Texture( GL_TEXTURE_3D, width, height, channels )
{

	GLenum type, format, int_format;
	bool is_mipmapped;

	tex_params( GL_TEXTURE_3D, channels, options,
                int_format, format, type, is_mipmapped );

	glTexImage3D( GL_TEXTURE_3D, 0, int_format, width, height, depth, 0, format, type, data );

	if( is_mipmapped )//&& data )
		glGenerateMipmap( GL_TEXTURE_3D );
	glBindTexture( GL_TEXTURE_3D, 0 );

}

TextureCube::TextureCube( int size, int channels,
                          void *pos_x, void *neg_x,
                          void *pos_y, void *neg_y,
                          void *pos_z, void *neg_z,
                          char const *options )
	: Texture( GL_TEXTURE_CUBE_MAP, size, size, channels )
{
	GLenum type, format, int_format;
	bool is_mipmapped;

	tex_params( GL_TEXTURE_CUBE_MAP, channels, options,
                int_format, format, type, is_mipmapped );

	glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, int_format, size, size, 0, format, GL_UNSIGNED_BYTE, pos_x );
	glTexImage2D( GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, int_format, size, size, 0, format, GL_UNSIGNED_BYTE, neg_x );
	glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, int_format, size, size, 0, format, GL_UNSIGNED_BYTE, pos_y );
	glTexImage2D( GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, int_format, size, size, 0, format, GL_UNSIGNED_BYTE, neg_y );
	glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, int_format, size, size, 0, format, GL_UNSIGNED_BYTE, pos_z );
	glTexImage2D( GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, int_format, size, size, 0, format, GL_UNSIGNED_BYTE, neg_z );

	if( is_mipmapped )//&& data )
		glGenerateMipmap( GL_TEXTURE_CUBE_MAP );

	glBindTexture( GL_TEXTURE_CUBE_MAP, 0 );
}
