#ifndef TEXTURE_H
#define TEXTURE_H

#include "common/shared.h"
#include "opengl/opengl.h"

#include <vector>

class PixelBuffer : public Shared
{
public:
	PixelBuffer( int width, int height ) : m_width( width ), m_height( height ) {}
	virtual ~PixelBuffer() {}

	int width() const { return m_width; }
	int height() const { return m_height; }
private:
	int m_width;
	int m_height;
};

// PixelType is a traits-type class, used to convert compile-time type
// information to opengl uniform type constants.
template< typename T >
struct PixelType{};

template<> struct PixelType< unsigned char >  {static int type() {return GL_UNSIGNED_BYTE;} };
template<> struct PixelType< signed char >    {static int type() {return GL_BYTE;} };
template<> struct PixelType< unsigned short > {static int type() {return GL_UNSIGNED_SHORT;} };
template<> struct PixelType< short >          {static int type() {return GL_SHORT;} };
template<> struct PixelType< unsigned int >   {static int type() {return GL_UNSIGNED_INT;} };
template<> struct PixelType< int >            {static int type() {return GL_INT;} };
template<> struct PixelType< float >          {static int type() {return GL_FLOAT;} };

class Texture : public PixelBuffer
{
public:
	Texture( int target, int width, int height, int depth, int channels, char const *options );
	~Texture();

	void bind( int unit );

	int channels() const;

	void gen_mipmaps();

	bool is_mipmapped() const {return m_is_mipmapped;}

	template< typename T >
	void get_image_data( std::vector< T > &buffer ) const
	{
		buffer.resize( width() * height() * depth() * m_channels );
		get_image_data( &buffer[0], PixelType< T >::type() );
	}

	friend class TextureTarget;

	int depth() const { return m_depth; }
	unsigned int type() const {return m_type;}
	unsigned int format() const {return m_format;}
	unsigned int int_format() const {return m_int_format;}

private:
	void get_image_data( void *buffer, int type ) const;

	int m_depth;
	unsigned int m_target;
	unsigned int m_id;
	int m_channels;
	unsigned int m_type;
	unsigned int m_format;
	unsigned int m_int_format;
	bool m_is_mipmapped;
};

class Texture2D : public Texture
{
public:
	typedef SharedPtr< Texture2D > Ptr;

	Texture2D( int width, int height, int channels, void *data = 0, char const *options = 0 );
};


class Texture2DArray : public Texture
{
public:
	typedef SharedPtr< Texture2DArray > Ptr;

	Texture2DArray( int width, int height, int size, int channels, void *data = 0, char const *options = 0 );
};

class TextureCube : public Texture
{
public:
	typedef SharedPtr< TextureCube > Ptr;

	TextureCube( int size, int channels,
	             void *pos_x, void *neg_x,
	             void *pos_y, void *neg_y,
	             void *pos_z, void *neg_z,
	             char const *options = 0 );
};

class Texture3D : public Texture
{
public:
	typedef SharedPtr< Texture3D > Ptr;

	Texture3D( int width, int height, int depth, int channels,
	           void *data, char const *options = 0 );
};

#endif // TEXTURE_H
