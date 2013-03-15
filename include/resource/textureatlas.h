#ifndef TEXTUREATLAS_H
#define TEXTUREATLAS_H

#include "math/vec2.h"
#include "common/shared.h"

#include <vector>
#include <string>

class Device;
class Material;
class ResourcePool;
class ShaderProgram;
class TextureTarget;
class Texture2D;

class TextureAtlas
{
public:
	TextureAtlas( std::vector< std::string > const &sources,
	              int max_atlas_size, int max_source_size );
	~TextureAtlas();

	struct Rect
	{
		Rect() : min( 0, 0 ), max( 0, 0 ) {}
		Rect( int2 min, int2 max ) : min( min ), max( max ) {}
		int2 min; int2 max;
		int area() const {return ( max.x - min.x ) * ( max.y - min.y );}
		int2 size() const {return max - min;}
		bool fits( int2 r ) const {return r.x <= max.x - min.x && r.y <= max.y - min.y;}
		bool split( int2 r, Rect &r1, Rect &r2 ) const;
		bool operator<( Rect const &r ) const {return area() <  r.area();}
	};

	struct SizeSrc
	{
		SizeSrc() : size( 0, 0 ), src( 0 ) {}
		SizeSrc( int2 size, int src ) : size( size ), src( src ) {}
		bool operator<( SizeSrc const &rs ) const {return size.x * size.y > rs.size.x * rs.size.y;}
		int2 size;
		int src;
	};

	float2 min_uv( int i ) const;
	float2 max_uv( int i ) const;
	SharedPtr< Texture2D > const &texture() const {return m_texture;}

private:

	bool fit_all( std::vector< SizeSrc > const &sizes, int atlas_width, int atlas_height );
	SharedPtr< Texture2D > m_texture;
	std::vector< Rect > m_rects;
	float2 m_mult;
};


#endif // TEXTUREATLAS_H
