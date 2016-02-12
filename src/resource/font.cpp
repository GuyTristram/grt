#include "resource/font.h"
#include "resource/material.h"
//#include "opengl.h"

namespace grt
{

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

Font::CharInfo const *Font::char_info( int c ) const
{
	int g = glyph_index( c );
	return g == -1 ? 0 : &m_char_infos[g];
}

int Font::glyph_index( int c ) const
{
	if( c >= m_first_char && c - m_first_char < m_char_infos.size() )
		return c - m_first_char;
	return -1;
}


float2 Font::measure( char const *text, int len ) const
{
	float2 result( 0, 0 );
	while( *text && len-- )
	{
		int c = ( unsigned char )*text++;
		int glyph = glyph_index( c );
		if( glyph != -1 )
		{
			CharInfo const &info = m_char_infos[glyph];
			result.x += info.advance;
			float h = ( info.br.y - info.ul.y );
			if( h > result.y )
				result.y = h;
		}
		else if( c == '\t' )
		{
			int glyph = glyph_index( ' ' );
			if( glyph != -1 )
				result.x += m_char_infos[glyph].advance;
		}
	}
	result.x;
	result.y *= m_image_height;
	return result;
}
}
