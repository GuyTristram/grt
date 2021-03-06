#include "core/texture.h"
#include "opengl/opengl.h"

namespace
{

void tex_params( GLenum target, int channels, char const *options,
                 GLenum &int_format, GLenum &format, GLenum &type, bool &is_mipmapped )
{
	type = GL_UNSIGNED_BYTE;
    bool is_signed = true;
    bool is_integer = false;
    bool is_short = false;
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
        else if( *options == 's' ) { is_short = true; }
        else if( *options == 'i' ) { is_integer = true; }
        else if( *options == 'u' ) { is_signed = false; }
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

    if( is_short )
        type = is_signed ? GL_SHORT : GL_UNSIGNED_SHORT;
    else if( is_integer )
        type = is_signed ? GL_INT : GL_UNSIGNED_INT;

	static const GLenum formats[5]       = {GL_DEPTH_COMPONENT,    GL_RED,   GL_RG,     GL_RGB,     GL_RGBA};
	static const GLenum float_formats[5] = {GL_DEPTH_COMPONENT32F, GL_R32F,  GL_RG32F,  GL_RGB32F,  GL_RGBA32F};
    static const GLenum short_formats[5] = { GL_DEPTH_COMPONENT16, GL_R16, GL_RG16, GL_RGB16, GL_RGBA16 };
    static const GLenum short_int_formats[5] = { GL_DEPTH_COMPONENT16, GL_R16I, GL_RG16I, GL_RGB16I, GL_RGBA16I };
    static const GLenum short_uint_formats[5] = { GL_DEPTH_COMPONENT16, GL_R16UI, GL_RG16UI, GL_RGB16UI, GL_RGBA16UI };
    //static const GLenum int_formats[5] = { GL_DEPTH_COMPONENT32, GL_R32UI, GL_RG32UI, GL_RGB32UI, GL_RGBA32UI };
    static const GLenum int_formats[5] = { GL_DEPTH_COMPONENT32, GL_R32I, GL_RG32I, GL_RGB32I, GL_RGBA32I };
    static const GLenum uint_formats[5] = { GL_DEPTH_COMPONENT32, GL_R32UI, GL_RG32UI, GL_RGB32UI, GL_RGBA32UI };

	format = formats[channels];
    if( type == GL_FLOAT )
    {
        int_format = float_formats[channels];
    }
    else if( is_short )
    {
        if( is_integer )
        {
            if( is_signed )
                int_format = short_int_formats[channels];
            else
                int_format = short_uint_formats[channels];
        }
        else
            int_format = short_formats[channels];
    }
    else if( is_integer )
    {
        if( is_signed )
            int_format = int_formats[channels];
        else
            int_format = uint_formats[channels];
    }
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

Texture::Texture( int target, int width, int height, int depth, int channels, char const *options )
	: PixelBuffer( width, height ), m_depth( depth ), m_target( target ), m_channels( channels )
{
	glGenTextures( 1, &m_id );
	glBindTexture( target, m_id );

	tex_params( target, channels, options,
                m_int_format, m_format, m_type, m_is_mipmapped );
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

void Texture::get_image_data( void *buffer, int type ) const
{
	if( m_channels < 1 || m_channels > 4 )
		return;
	glBindTexture( m_target, m_id );
	static const GLenum formats[5] = {0, GL_RED, GL_RG, GL_RGB, GL_RGBA};
	glGetTexImage( m_target, 0, formats[m_channels], type, buffer );
}


Texture2D::Texture2D( int width, int height, int channels, void *data, char const *options )
	: Texture( GL_TEXTURE_2D, width, height, 1, channels, options )
{
	glTexImage2D( GL_TEXTURE_2D, 0, int_format(), width, height, 0, format(), type(), data );

	if( is_mipmapped() )//&& data )
		glGenerateMipmap( GL_TEXTURE_2D );
	glBindTexture( GL_TEXTURE_2D, 0 );

}


Texture2DArray::Texture2DArray( int width, int height, int size, int channels, void *data, char const *options )
	: Texture( GL_TEXTURE_2D_ARRAY, width, height, size, channels, options )
{
	glTexImage3D( GL_TEXTURE_2D_ARRAY, 0, int_format(), width, height, size, 0, format(), type(), data );

	if( is_mipmapped() )//&& data )
		glGenerateMipmap( GL_TEXTURE_2D_ARRAY );
	glBindTexture( GL_TEXTURE_2D_ARRAY, 0 );

}

Texture3D::Texture3D( int width, int height, int depth, int channels, void *data, char const *options )
	: Texture( GL_TEXTURE_3D, width, height, depth, channels, options )
{
	glTexImage3D( GL_TEXTURE_3D, 0, int_format(), width, height, depth, 0, format(), type(), data );

	if( is_mipmapped() )//&& data )
		glGenerateMipmap( GL_TEXTURE_3D );
	glBindTexture( GL_TEXTURE_3D, 0 );

}

TextureCube::TextureCube( int size, int channels,
                          void *pos_x, void *neg_x,
                          void *pos_y, void *neg_y,
                          void *pos_z, void *neg_z,
                          char const *options )
	: Texture( GL_TEXTURE_CUBE_MAP, size, size, 1, channels, options )
{
	glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, int_format(), size, size, 0, format(), type(), pos_x );
	glTexImage2D( GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, int_format(), size, size, 0, format(), type(), neg_x );
	glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, int_format(), size, size, 0, format(), type(), pos_y );
	glTexImage2D( GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, int_format(), size, size, 0, format(), type(), neg_y );
	glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, int_format(), size, size, 0, format(), type(), pos_z );
	glTexImage2D( GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, int_format(), size, size, 0, format(), type(), neg_z );

	if( is_mipmapped() )//&& data )
		glGenerateMipmap( GL_TEXTURE_CUBE_MAP );

	glBindTexture( GL_TEXTURE_CUBE_MAP, 0 );
}
