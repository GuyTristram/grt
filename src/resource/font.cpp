#include "resource/font.h"
#include "resource/material.h"
//#include "opengl.h"

Font::Font( SharedPtr< Material > const &material, float height,
            int image_width, int image_height,
            int first_char, int char_count,
            CharInfo const *char_infos )
	: m_material( material ),
	  m_height( height ),
	  m_image_width( float( image_width ) ),
	  m_image_height( float( image_height ) ),
	  m_first_char( first_char ),
	  m_char_infos( char_infos, char_infos + char_count )
{
}

Font::~Font()
{
}

Font::CharInfo const &Font::char_info( int c )
{
	c -= m_first_char;
	if( c >= int( m_char_infos.size() ) )
		c = m_char_infos.size() - 1;
	return m_char_infos[c];
}

float2 Font::measure( char const *text, int len ) const
{
	float2 result( 0, 0 );
	while( *text && len-- )
	{
		CharInfo const &info = m_char_infos[( unsigned char )*text++ - m_first_char ];
		result.x += info.advance;
		float h = ( info.br.y - info.ul.y );
		if( h > result.y )
			result.y = h;
	}
	result.x;
	result.y *= m_image_height;
	return result;
}
