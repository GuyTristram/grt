#ifndef RESOURCEPOOL_H
#define RESOURCEPOOL_H

#include "common/shared.h"

#include <map>
#include <string>
#include <vector>

class Texture2D;
class TextureCube;
class ShaderProgram;
class Font;

class ResourcePool : public Shared
{
public:
	ResourcePool();
	~ResourcePool();

	void add_path( char const *path );

	SharedPtr< ShaderProgram > shader_program( char const *filename );
	SharedPtr< Texture2D > texture2d( char const *filename );
	SharedPtr< TextureCube > texture_cube( char const *filename, char const *ext );
	SharedPtr< Font > font( char const *filename, int size );

	static ResourcePool &stock();

private:
	std::string full_name( char const *filename );
	std::map< std::string, SharedPtr< Texture2D > > m_textures;
	std::map< std::string, SharedPtr< TextureCube > > m_cube_textures;
	std::map< std::string, SharedPtr< ShaderProgram > > m_shader_programs;
	std::map< std::pair< std::string, int >, SharedPtr< Font > > m_fonts;
	std::vector< std::string > m_path;
};

#endif // RESOURCEPOOL_H