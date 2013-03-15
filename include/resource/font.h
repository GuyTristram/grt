#ifndef BFONT_H
#define BFONT_H

#include "common/shared.h"

#include "math/vec2.h"

#include <vector>

class Material;

class Font : public Shared
{
public:
	typedef SharedPtr< Font > Ptr;
	struct CharInfo { float2 ul, br, off; float advance; };

	Font( SharedPtr< Material > const &material, float height,
	      int image_width, int image_height,
	      int first_char, int char_count,
	      CharInfo const *char_infos );
	~Font();

	SharedPtr< Material > &material() { return m_material; }
	float image_width() const { return m_image_width; }
	float image_height() const { return m_image_height; }
	float height() const { return m_height; }
	CharInfo const &char_info( int c );

	float2 measure( char const *text, int len = -1 ) const;

private:
	SharedPtr< Material > m_material;
	float m_height;
	float m_image_width;
	float m_image_height;
	int m_first_char;
	std::vector< CharInfo > m_char_infos;
};

#endif // BFONT_H
