#ifndef PPRENDERER_H
#define PPRENDERER_H

#include "core/renderstate.h"
#include "core/uniform.h"

#include "common/shared.h"

#include "math/mat44.h"
#include "math/mat33.h"

#include "resource/mesh.h"
#include "resource/scenenode.h"

class Device;
class Material;
class ResourcePool;
class ShaderProgram;
class TextureTarget;
class Texture2D;
struct Light;

class PPRenderer : public SceneNodeVisitor
{
public:
	PPRenderer( Device &device, ResourcePool &pool );
	~PPRenderer();

	void render( Device &device, Mesh &mesh, Material &material,
	             float44 const &cam_pos, float44 const &cam_pers,
	             std::vector< float4 > const &light_positions,
	             std::vector< float > const &light_intensities );

	void render( Device &device, float44 const &cam_pos, float44 const &cam_pers,
	             SceneNode &root );

	virtual void visit( SceneMesh &mesh );
	virtual void visit( SceneLight &light );

private:
	static const int SHADOW_SIZE = 256;
	void update_light( SceneLight &light );
	void draw_meshes( ShaderProgram &p, RenderState &s, RenderTarget &t, Frustum const &f, float44 const &projected_from_world );

	SharedPtr< ShaderProgram > m_depth_pass_program;
	SharedPtr< ShaderProgram > m_gbuf_program;
	SharedPtr< ShaderProgram > m_light_program;
	SharedPtr< ShaderProgram > m_light_sh_program;
	SharedPtr< ShaderProgram > m_shade_program;
	SharedPtr< ShaderProgram > m_hdr_program;
	SharedPtr< ShaderProgram > m_shadow_program;
	SharedPtr< ShaderProgram > m_shadow_combine_program;

	SharedPtr< TextureTarget > m_depth_pass_target;
	SharedPtr< TextureTarget > m_gbuf_target;
	SharedPtr< TextureTarget > m_light_target;
	SharedPtr< TextureTarget > m_shadow_target;
	SharedPtr< TextureTarget > m_hdr_target;
	SharedPtr< TextureTarget > m_near_shadow_target;
	SharedPtr< TextureTarget > m_far_shadow_target;

	SharedPtr< Texture2D > m_near_light_texture;
	SharedPtr< Texture2D > m_far_light_texture;

	UniformGroup m_shadow_uniforms;
	UniformGroup m_light_uniforms;
	UniformGroup m_shade_uniforms;
	UniformGroup m_hdr_uniforms;

	Mesh m_quad;
	Mesh m_icosohedron;

	RenderState m_shadow_state;

	std::vector< SceneLight * > m_lights;
	std::vector< SceneMesh * > m_meshes;
};


#endif // PPRENDERER_H
