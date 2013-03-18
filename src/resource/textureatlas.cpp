#include "resource/textureatlas.h"

#include <algorithm>
#include <set>

#include "core/renderstate.h"
#include "core/shaderprogram.h"
#include "core/texture.h"
#include "core/texturetarget.h"

#include "resource/mesh.h"
#include "resource/resourcepool.h"


TextureAtlas::TextureAtlas( std::vector< std::string > const &sources,
                            int max_atlas_size, int max_source_size )
{
	int n_sources = sources.size();

	ResourcePool pool;
	std::vector< Texture2D::Ptr > textures;
	textures.reserve( n_sources );

	std::vector< SizeSrc > sizes( n_sources );
	int n_channels = 0;
	for( int i = 0; i != n_sources; ++i )
	{
		Texture2D::Ptr t = pool.texture2d( sources[i].c_str() );
		if( !t.get() )
		{
			printf( "Error: failed to load texture %s \n", sources[i].c_str() );
			const unsigned char c = 255;
			t = Texture2D::Ptr( new Texture2D( 1, 1, 1, 0, ( const char * )&c ) );
		}
		textures.push_back( t );
		sizes[i] = SizeSrc( int2( std::min( t->width(),  max_source_size ),
		                          std::min( t->height(), max_source_size ) ), i );
		n_channels = std::max( n_channels, t->channels() );
	}

	std::sort( sizes.begin(), sizes.end() );

	m_rects.resize( n_sources );

	int atlas_width = 16;
	int atlas_height = 16;

	while( atlas_width  <= max_atlas_size &&
	       atlas_height <= max_atlas_size &&
	       !fit_all( sizes, atlas_width, atlas_height ) )
	{
		if( atlas_width <= atlas_height )
			atlas_width *= 2;
		else
			atlas_height *= 2;
	}

	//Texture2D::Ptr t = ResourcePool::stock().texture2d( "../src/stone3.jpg" );

	if( atlas_width  <= max_atlas_size &&
	    atlas_height <= max_atlas_size )
	{
		//m_texture.set( new Texture2D( atlas_width, atlas_height, n_channels ) );
		m_texture = Texture2D::Ptr( new Texture2D( atlas_width, atlas_height, n_channels ) );
		m_mult = float2( 1.f / float( atlas_width ), 1.f / float( atlas_height ) );

		ShaderProgram::Ptr program = ResourcePool::stock().shader_program( "atlas.sp" );
		Mesh quad = make_quad();

		TextureTarget target;
		target.attach( m_texture );
		target.is_complete();
		target.clear();

		RenderState state;
		state.depth_test( false );

		for( int i = 0; i != n_sources; ++i )
		{
			program->set( "u_source", textures[i] );
			program->set( "u_min", float2( m_rects[i].min ) * m_mult );
			program->set( "u_max", float2( m_rects[i].max ) * m_mult );
			quad.draw( *program, state, target );
		}
		m_texture->gen_mipmaps();
	}
}

bool TextureAtlas::fit_all( std::vector< SizeSrc > const &sizes, int atlas_width, int atlas_height )
{
	int n_sources = sizes.size();

	std::set< Rect > rects;
	rects.insert( Rect( int2( 0, 0 ), int2( atlas_width, atlas_height ) ) );

	for( int i = 0; i != n_sources; ++i )
	{
		bool fit = false;
		for( auto r = rects.begin(); r != rects.end(); ++r )
		{
			Rect r1, r2;
			if( r->split( sizes[i].size, r1, r2 ) )
			{
				m_rects[ sizes[i].src ] = Rect( r->min, r->min + sizes[i].size );
				rects.erase( r );
				if( r1.area() > 0 )
					rects.insert( r1 );
				if( r2.area() > 0 )
					rects.insert( r2 );
				fit = true;
				break;
			}
		}
		if( !fit ) return false;
	}
	return true;
}

bool TextureAtlas::Rect::split( int2 s, Rect &r1, Rect &r2 ) const
{
	if( !fits( s ) ) return false;

	int2 diff = size() - s;

	int2 p1 = int2( min.x + s.x, min.y );
	int2 p2 = int2( min.x, min.y + s.y );
	if( diff.x > diff.y )
	{
		r1 = Rect( p1, int2( max.x, min.y + s.y ) );
		r2 = Rect( p2, max );
	}
	else
	{
		r1 = Rect( p1, max );
		r2 = Rect( p2, int2( min.x + s.x, max.y ) );
	}
	return true;
}


TextureAtlas::~TextureAtlas()
{
}

float2 TextureAtlas::min_uv( int i ) const
{
	return float2( m_rects[i].min ) * m_mult;
}

float2 TextureAtlas::max_uv( int i ) const
{
	return float2( m_rects[i].max ) * m_mult;
}

