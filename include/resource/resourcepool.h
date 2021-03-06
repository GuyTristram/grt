#ifndef RESOURCEPOOL_H
#define RESOURCEPOOL_H

#include "common/shared.h"

#include <map>
#include <string>
#include <vector>

class Texture2D;
class Texture2DArray;
class TextureCube;
class ShaderProgram;
namespace grt
{
class Font;
}
class Image;

class ResourcePool : public Shared
{
public:
	ResourcePool();
	~ResourcePool();

	void add_path( char const *path );

	SharedPtr< ShaderProgram >  shader_program( char const *filename );
	SharedPtr< Texture2D >      texture2d( char const *filename, char const *options = 0 );
	SharedPtr< Texture2DArray > texture2d_array( std::vector< std::string > const &filenames, int size );
	SharedPtr< Texture2DArray > texture2d_array( std::string const &filename );
	SharedPtr< TextureCube >    texture_cube( char const *filename, char const *ext );
	SharedPtr< grt::Font >      font( char const *filename, int size );
	SharedPtr< Image >          image( char const *filename );

	void reload_shader_program( char const *filename );


	static ResourcePool &stock();

private:
	std::string full_name( char const *filename );
	std::map< std::string, SharedPtr< Image > > m_images;
	std::map< std::string, SharedPtr< Texture2D > > m_textures;
	std::map< std::string, SharedPtr< Texture2DArray > > m_texture_arrays;
	std::map< std::string, SharedPtr< TextureCube > > m_cube_textures;
	std::map< std::string, SharedPtr< ShaderProgram > > m_shader_programs;
	std::map< std::pair< std::string, int >, SharedPtr< grt::Font > > m_fonts;
	std::vector< std::string > m_path;
};

#endif // RESOURCEPOOL_H
