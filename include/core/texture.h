#ifndef TEXTURE_H
#define TEXTURE_H

#include "common/shared.h"

class PixelBuffer : public Shared
{
public:
	PixelBuffer( int width, int height ) : m_width( width ), m_height( height ) {}
	int width() const { return m_width; }
	int height() const { return m_height; }
private:
	int m_width;
	int m_height;
};

class Texture2D : public PixelBuffer
{
public:
	typedef SharedPtr< Texture2D > Ptr;

	Texture2D( int width, int height, int channels, void *data = 0, char const *options = 0 );
	~Texture2D();

	void bind( int unit );

	int channels() const {return m_channels;}

	void gen_mipmaps();

	friend class TextureTarget;
private:
	unsigned int m_id;
	int m_channels;
};

class TextureCube : public PixelBuffer
{
public:
	typedef SharedPtr< TextureCube > Ptr;

	TextureCube( int size, int channels,
	             void *pos_x, void *neg_x,
	             void *pos_y, void *neg_y,
	             void *pos_z, void *neg_z,
	             char const *options = 0 );
	~TextureCube();

	void bind( int unit );

	void gen_mipmaps();

	friend class TextureTarget;
private:
	unsigned int m_id;
};

#endif // TEXTURE_H
