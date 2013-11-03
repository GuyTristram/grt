#include "resource/resourcepool.h"

#include "common/charrange.h"

#include "core/shaderprogram.h"
#include "core/texture.h"
#include "core/texturetarget.h"

#include "resource/material.h"
#include "resource/mesh.h"
#include "resource/font.h"
#include "resource/image.h"

#include "external/stb_image.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "external/stb_truetype.h"

#ifdef GRT_USE_FREETYPE
#include <ft2build.h>
#include FT_FREETYPE_H
#endif

#include <fstream>

namespace
{
unsigned int next_higher_power_of_two( unsigned int v ) // compute the next highest power of 2 of 32-bit v
{
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;
	return v;
}


Font::Ptr load_trd( CharRange range, ResourcePool &pool )
{
	read_int( range ); // Ignore major version
	ignore( range, '.' );
	read_int( range ); // Ignore minor version

	CharRange bitmap = read_token( range );

	Texture2D::Ptr texture = pool.texture2d( to_std_string( bitmap ).c_str() ) ;

	Material::Ptr font_material( new Material );

	font_material->state = RenderState::Ptr( new RenderState( RenderState::Blend ) );
	font_material->state->depth_write( false );
	font_material->state->depth_test( false );
	font_material->program = pool.shader_program( "ui.sp" );
	font_material->uniforms.set( "u_texture", texture );

	int width = 1, height = 1;
	if( texture )
	{
		width = texture->width();
		height = texture->height();
	}

	int char_height = read_int( range ); // baseline (not used)
	int size_hint = read_int( range );

	std::vector< Font::CharInfo > char_info;
	char_info.reserve( size_hint );

	int first = 256;

	while( !range.empty() )
	{
		int c = read_int( range );
		int left = read_int( range );
		int top = read_int( range );
		int right = read_int( range );
		int bottom = read_int( range );
		if( c < first )
			first = c;
		if( c >= int( char_info.size() ) )
			char_info.resize( c + 1 );

		char_info[c].ul = float2( left / float( width ),
		                          top / float( height ) );
		char_info[c].br = float2( right / float( width ),
		                          bottom / float( height ) );
		char_info[c].off = float2( 0, 0 );
		char_info[c].advance = float( right - left );
		ignore( range, '\n' );
	}
	return Font::Ptr( new Font( font_material, float( char_height ), width, height, first,
	                            char_info.size() - first, &char_info[first] ) );
}

Font::Ptr load_ttf( CharRange range, int size, ResourcePool &pool )
{
	const int first = 32, last = 256;
	int x_size = 1024, y_size = 4096;
	std::vector< unsigned char > bitmap( x_size * y_size );
	stbtt_bakedchar chardata[last-first];
	stbtt_BakeFontBitmap( ( unsigned char * )range.begin, 0, // font location (use offset=0 for plain .ttf)
	                      float( size ), &bitmap[0], x_size, y_size, // bitmap to be filled in
	                      32, last-first,          // characters to bake
	                      chardata );             // you allocate this, it's num_chars long

	Font::CharInfo char_info[last-first];

	y_size = next_higher_power_of_two( chardata[last-first-1].y1 );
	for( int i = 0; i < last-first; ++i )
	{
		char_info[i].ul = float2( chardata[i].x0 / float( x_size ),
		                          chardata[i].y0 / float( y_size ) );
		char_info[i].br = float2( chardata[i].x1 / float( x_size ),
		                          chardata[i].y1 / float( y_size ) );
		char_info[i].off = float2( chardata[i].xoff / float( x_size ),
		                           chardata[i].yoff / float( y_size ) );
		char_info[i].advance = chardata[i].xadvance;
	}
	Material::Ptr font_material( new Material );
	font_material->state = RenderState::Ptr( new RenderState( RenderState::Blend ) );
	//font_material->program = shader_program( "ui.sp" );
	font_material->program = pool.shader_program( "uifont.sp" );
	font_material->uniforms.set( "u_texture", Texture2D::Ptr( new Texture2D( x_size, y_size, 1, &bitmap[0] ) ) );
	return Font::Ptr( new Font( font_material, ( float )size, x_size, y_size, 32, last-first, char_info ) );
}

#ifdef GRT_USE_FREETYPE
Font::Ptr load_freetype( CharRange range, int size, ResourcePool &pool )
{
	const int first = 32, last = 255;
	int x_size = 512;

	FT_Library library;
	if( FT_Init_FreeType( &library ) )
		return Font::Ptr();

	FT_Face face;
	int error = FT_New_Memory_Face( library, ( const FT_Byte * )range.begin, range.length(), 0, &face );

	error = FT_Set_Pixel_Sizes( face, 0, size );

	Font::CharInfo char_info[last-first];

	int x = 0, y = 0;

	for( int i = 0; i < last-first; ++i )
	{
		error = FT_Load_Char( face, i + first, FT_LOAD_RENDER );
		int w = face->glyph->bitmap.width;
		int h = face->glyph->bitmap.rows;
		if( x + w > x_size )
		{
			x = 0;
			y += size + 1;
		}
		char_info[i].ul = float2( float( x ), float( y ) );
		char_info[i].br = float2( float( x + w ), float( y + h ) );
		char_info[i].off = float2( float( face->glyph->bitmap_left ),
		                           float( -face->glyph->bitmap_top ) );
		char_info[i].advance = face->glyph->advance.x / 64.f;
		x += w + 1;
	}

	int y_size = next_higher_power_of_two( y + size );

	std::vector< unsigned char > bitmap( x_size * y_size );

	float2 scale( 1 / float( x_size ), 1 / float( y_size ) );

	for( int i = 0; i < last-first; ++i )
	{
		error = FT_Load_Char( face, i + first, FT_LOAD_RENDER );
		int y = 0;
		int w = int( char_info[i].br.x - char_info[i].ul.x );

		for( int yb = int( char_info[i].ul.y ); yb < char_info[i].br.y; ++yb )
		{
			unsigned char *dest = &bitmap[ yb * x_size + int( char_info[i].ul.x ) ];
			unsigned char *source = face->glyph->bitmap.buffer +
			                        face->glyph->bitmap.pitch * y++;
			for( int xb = 0; xb < w; ++xb )
				*dest++ = *source++;
		}
		char_info[i].ul *= scale;
		char_info[i].br *= scale;
	}
	Material::Ptr font_material( new Material );
	font_material->state = RenderState::Ptr( new RenderState( RenderState::Blend ) );
	//font_material->program = shader_program( "ui.sp" );
	font_material->program = pool.shader_program( "uifont.sp" );
	font_material->uniforms.set( "u_texture", Texture2D::Ptr( new Texture2D( x_size, y_size, 1, &bitmap[0] ) ) );
	return Font::Ptr( new Font( font_material, float( face->size->metrics.height / 64 ),
	                            x_size, y_size, 32, last-first, char_info ) );
	FT_Done_FreeType( library );
}
#endif
}


ResourcePool::ResourcePool()
{
}

ResourcePool::~ResourcePool()
{
}

namespace
{
struct StbLoader : public Uncopyable
{
	StbLoader() : data( 0 ), x( 0 ), y( 0 ), n( 0 ) {}
	StbLoader( char const *filename )   { load( filename ); }
	~StbLoader()                        { if( data ) stbi_image_free( data ); }

	void load( char const *filename )   { data = stbi_load( filename, &x, &y, &n, 0 ); }

	unsigned char *data;
	int x,y,n;
};
}

SharedPtr< Texture2D > ResourcePool::texture2d( char const *filename )
{
	auto tex = m_textures.find( filename );
	if( tex != m_textures.end() )
		return tex->second;

	StbLoader loader( filename );

	if( !loader.data )
	{
		printf( "Error: unable to load texture %s\n", filename );
		return Texture2D::Ptr();
	}

	Texture2D::Ptr new_tex( new Texture2D( loader.x, loader.y, loader.n, ( void * )loader.data ) );

	m_textures[ filename ] = new_tex;
	return new_tex;
}

SharedPtr< Texture2DArray > ResourcePool::texture2d_array( std::vector< std::string > const &filenames, int size )
{
	ResourcePool pool;
	int channels = pool.texture2d( filenames[0].c_str() )->channels();
	Texture2DArray::Ptr new_tex( new Texture2DArray( size, size, filenames.size(), channels ) );

	ShaderProgram::Ptr program = ResourcePool::stock().shader_program( "texturecopy.sp" );
	TextureTarget target;
	RenderState state;
	state.depth_test( false );
	Mesh quad = make_quad();

	for( int i = 0; i != filenames.size(); ++i )
	{
		program->set( "u_source", pool.texture2d( filenames[i].c_str() ) );
		target.attach( new_tex, i );
		quad.draw( *program, state, target );
	}
	new_tex->gen_mipmaps();


	return new_tex;
}

SharedPtr< Texture2DArray > ResourcePool::texture2d_array( std::string const &filename )
{
	auto tex = m_texture_arrays.find( filename );
	if( tex != m_texture_arrays.end() )
		return tex->second;

	std::ifstream in_stream( filename.c_str(), std::ofstream::binary );
	int count, size, channels, buffer_size;
	char name[4];
	in_stream.read( name, 4 );
	in_stream.read( (char*)&count, 4 );
	in_stream.read( (char*)&size, 4 );
	in_stream.read( (char*)&channels, 4 );
	in_stream.read( (char*)&buffer_size, 4 );
	std::vector< unsigned char > data( buffer_size );
	in_stream.read( (char*)&data[0], buffer_size );

	Texture2DArray::Ptr new_tex( new Texture2DArray( size, size, count, channels, &data[0] ) );

	m_texture_arrays[ filename ] = new_tex;
	return new_tex;
}

SharedPtr< Image > ResourcePool::image( char const *filename )
{
	auto im = m_images.find( filename );
	if( im != m_images.end() )
		return im->second;

	StbLoader loader( filename );

	if( !loader.data )
		return Image::Ptr();

	Image::Ptr new_image( new Image( loader.x, loader.y, loader.n, ( void * )loader.data ) );

	m_images[ filename ] = new_image;
	return new_image;
}

SharedPtr< TextureCube > ResourcePool::texture_cube( char const *filename, char const *ext )
{
	const char *suffix[6] = {"posx", "negx", "posy", "negy", "posz", "negz"};

	auto tex = m_cube_textures.find( filename );
	if( tex != m_cube_textures.end() )
		return tex->second;

	StbLoader loader[6];
	int width, height; // Initialized first time through loop
	for( int i = 0; i != 6; ++i )
	{
		std::string face_name( filename );
		face_name += suffix[i];
		face_name += ext;
		loader[i].load( face_name.c_str() );
		if( !loader[i].data )
			return TextureCube::Ptr();
		if( i == 0 )
		{
			width = loader[0].x;
			height = loader[0].y;
		}
		else
		{
			if( loader[i].x != width || loader[i].y != height )
				return TextureCube::Ptr();
		}
	}


	TextureCube::Ptr new_tex( new TextureCube( width, loader[0].n,
	                          ( void * )loader[0].data, ( void * )loader[1].data,
	                          ( void * )loader[2].data, ( void * )loader[3].data,
	                          ( void * )loader[4].data, ( void * )loader[5].data ) );

	m_cube_textures[ filename ] = new_tex;
	return new_tex;
}

namespace
{
	std::string read_source_file( CharRange &range )
	{
		eat_white( range );
		CharRange name = read_line( range );
		CharRangeFile file( to_std_string( name ).c_str() );
		return to_std_string( file.range() );
	}
	std::string read_source( CharRange &range )
	{
		return to_std_string( read_to_token( range, "#endshader" ) );
	}
}

SharedPtr< ShaderProgram > ResourcePool::shader_program( char const *filename )
{
	auto sp = m_shader_programs.find( filename );
	if( sp != m_shader_programs.end() )
		return sp->second;

	CharRangeFile file( full_name( filename ).c_str() );
	CharRange range = file.range();
	//CharRange vertex_source, fragment_source;
	std::string vertex_source, fragment_source, geometry_source, control_source, evaluation_source;

	while( !range.empty() )
	{
		CharRange token = read_token( range );
		if( token == "vertex_source" )
			vertex_source += read_source( range );
		else if( token == "vertex_file" )
			vertex_source += read_source_file( range );
		else if( token == "fragment_source" )
			fragment_source += read_source( range );
		else if( token == "fragment_file" )
			fragment_source += read_source_file( range );
		else if( token == "geometry_source" )
			geometry_source += read_source( range );
		else if( token == "geometry_file" )
			geometry_source += read_source_file( range );
		else if( token == "control_source" )
			control_source += read_source( range );
		else if( token == "control_file" )
			control_source += read_source_file( range );
		else if( token == "evaluation_source" )
			evaluation_source += read_source( range );
		else if( token == "evaluation_file" )
			evaluation_source += read_source_file( range );
	}

	if( vertex_source.empty() || fragment_source.empty() )
		return ShaderProgram::Ptr();

	ShaderProgram::Ptr new_sp(
		new ShaderProgram( vertex_source.c_str(),
		                   fragment_source.c_str(),
		                   geometry_source.empty() ? 0 : geometry_source.c_str(),
		                   control_source.empty() ? 0 : control_source.c_str(),
		                   evaluation_source.empty() ? 0 : evaluation_source.c_str() ) );

	m_shader_programs[ filename ] = new_sp;
	return new_sp;
}

SharedPtr< Font > ResourcePool::font( char const *filename, int size )
{
	auto key = std::make_pair( std::string( filename ), size );
	auto f = m_fonts.find( key );
	if( f != m_fonts.end() )
		return f->second;

	CharRangeFile file( filename );
	CharRange range = file.range();

	if( range.empty() )
		return Font::Ptr();

#ifdef GRT_USE_FREETYPE
	Font::Ptr new_font = std::string( filename ).find( ".trd" ) == std::string::npos ?
	                     load_freetype( range, size, *this ) : load_trd( range, *this );
#else
	Font::Ptr new_font = std::string( filename ).find( ".trd" ) == std::string::npos ?
	                     load_ttf( range, size, *this ) : load_trd( range, *this );
#endif
	m_fonts[ key ] = new_font;
	return new_font;
}

void ResourcePool::add_path( char const *path )
{
	m_path.push_back( std::string( path ) );
}

ResourcePool &ResourcePool::stock()
{
	static ResourcePool rp;
	return rp;
}

std::string ResourcePool::full_name( char const *filename )
{
	for( auto p = m_path.begin(); p != m_path.end(); ++p )
	{
		std::string full = *p + filename;
		std::ifstream stream( full.c_str() );
		if( stream.good() )
		{
			return full;
		}
	}
	return filename;
}
