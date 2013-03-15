#ifndef TEXTURETARGET_H
#define TEXTURETARGET_H

#include "rendertarget.h"
#include "texture.h"

class Texture2D;
class TextureCube;

class RenderBuffer : public PixelBuffer
{
public:
	typedef SharedPtr< RenderBuffer > Ptr;
	enum Format
	{
	    RGBA_4444,
	    //RGB_565,
	    RGBA_5551,
	    Depth,
	    Stencil
	};
	RenderBuffer( int width, int height, Format format );
	~RenderBuffer();

	friend class TextureTarget;
private:
	unsigned int m_id;
};

class TextureTarget : public Shared, public RenderTarget
{
public:
	TextureTarget();
	~TextureTarget();

	static const int MAX_COLOUR_BUFFERS = 8;

	enum BufferType
	{
	    Colour,
	    Depth,
	    Stencil
	};

	void attach( SharedPtr<Texture2D> const &texture, BufferType type = Colour, int position = 0 );
	void attach( SharedPtr<TextureCube> const &texture, int face, BufferType type = Colour, int position = 0 );
	void attach( SharedPtr<RenderBuffer> const &buffer, BufferType type, int position = 0 );

	bool is_complete();

	virtual int width() const;
	virtual int height() const;

private:
	virtual void do_bind();

	unsigned int m_id;

	SharedPtr<PixelBuffer> m_buffer[2 + MAX_COLOUR_BUFFERS];
};

#endif // TEXTURETARGET_H
