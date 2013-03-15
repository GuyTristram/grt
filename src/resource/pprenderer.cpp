#include "resource/pprenderer.h"

#include "core/device.h"
#include "core/renderstate.h"
#include "core/shaderprogram.h"
#include "core/texturetarget.h"
#include "core/texture.h"

#include "resource/material.h"
#include "resource/mesh.h"
#include "resource/resourcepool.h"
#include "resource/light.h"


PPRenderer::PPRenderer( Device &device, ResourcePool &pool ) :
	m_gbuf_target( new TextureTarget() ),
	m_light_target( new TextureTarget() ),
	m_hdr_target( new TextureTarget() ),
	m_shadow_target( new TextureTarget() ),
	m_quad( make_quad() ),
	m_icosohedron( make_cube() )
{
	int w = device.width(), h = device.height();

	SharedPtr< Texture2D > depthTexture(  new Texture2D( w, h, 1, 0, "dfc" ) );
	SharedPtr< Texture2D > normalTexture( new Texture2D( w, h, 3, 0, "c" ) );
	SharedPtr< Texture2D > lightTexture(  new Texture2D( w, h, 4, 0, "cf" ) );
	SharedPtr< Texture2D > hdrTexture(    new Texture2D( w, h, 3, 0, "fc" ) );

	m_gbuf_target->attach( depthTexture, TextureTarget::Depth );
	m_gbuf_target->attach( normalTexture );
	m_gbuf_target->is_complete();

	m_light_target->attach( depthTexture, TextureTarget::Depth );
	m_light_target->attach( lightTexture );
	m_light_target->is_complete();

	m_hdr_target->attach( depthTexture, TextureTarget::Depth );
	m_hdr_target->attach( hdrTexture );
	m_hdr_target->is_complete();

	m_light_uniforms.set( "u_depth", depthTexture );
	m_light_uniforms.set( "u_normal", normalTexture );

	m_shade_uniforms.set( "u_light", lightTexture );

	m_hdr_uniforms.set( "u_texture", hdrTexture );

	m_gbuf_program     = pool.shader_program( "draw_normals.sp" );
	m_light_program    = pool.shader_program( "draw_lights.sp" );
	m_light_sh_program = pool.shader_program( "draw_lights_sh.sp" );
	m_shade_program    = pool.shader_program( "use_light_map.sp" );
	m_hdr_program      = pool.shader_program( "hdr.sp" );
	m_shadow_program   = pool.shader_program( "draw_shadow_map.sp" );

	m_shadow_state.colour_write( false );
	m_shadow_state.draw_back( false );
	m_shadow_state.draw_front( true );
}

PPRenderer::~PPRenderer()
{
}

void PPRenderer::render( Device &device, Mesh &mesh, Material &material,
                         float44 const &cam_pos, float44 const &cam_pers,
				         std::vector< float4 > const &light_positions,
						 std::vector< float > const &light_intensities )
{

	float44 cam_inv = inverse( cam_pos );
	float44 mod_view_pers = cam_pers * cam_inv;

	m_gbuf_program->set( material.uniforms );
	m_gbuf_program->set( "u_t_model_view_projection",  mod_view_pers );
	m_gbuf_program->set( "u_t_normal", float33() );
	
	m_gbuf_target->clear(true, true);

	mesh.draw( *m_gbuf_program, *RenderState::stock_opaque(), *m_gbuf_target );


	// Render lights into light target

	RenderState rs_light;

	rs_light.depth_test( true );
	rs_light.depth_write( false );

	m_light_program->set( m_light_uniforms );
	m_light_program->set( "u_t_inv_model_view_projection", inverse( mod_view_pers ) );
	m_light_program->set( "u_light_positions[0]", light_positions );
	m_light_program->set( "u_light_intensities[0]", light_intensities );
	m_light_program->set( "u_light_count", (int)light_positions.size() );
	m_light_program->set( "u_eye_position", cam_pos.t );
	m_quad.draw( *m_light_program, rs_light, *m_light_target );


	// Render geometry with material

	RenderState rs_material;

	rs_material.depth_test( true );
	rs_material.depth_write( false );

	m_shade_program->set( material.uniforms );
	m_shade_program->set( m_shade_uniforms );
	m_shade_program->set( "u_t_model_view_projection",  mod_view_pers );

	mesh.draw( *m_shade_program, rs_material, *m_hdr_target );


	// Render final image

	m_hdr_program->set( m_hdr_uniforms );

	RenderState rs_quad;

	rs_quad.depth_test( false );
	rs_quad.depth_write( false );

	m_quad.draw( *m_hdr_program, rs_quad, device );
}

void PPRenderer::render( Device &device, Mesh &mesh, Material &material,
                         float44 const &world_from_camera,
						 float44 const &projected_from_camera,
				         std::vector< Light > &lights )
{

	float44 camera_from_world = inverse( world_from_camera );
	float44 projected_from_world = projected_from_camera * camera_from_world;

	m_gbuf_program->set( material.uniforms );
	m_gbuf_program->set( "u_t_model_view_projection",  projected_from_world );
	m_gbuf_program->set( "u_t_normal", float33() );
	
	m_gbuf_target->clear(true, true);

	mesh.draw( *m_gbuf_program, *RenderState::stock_opaque(), *m_gbuf_target );


	// Render lights into light target

	RenderState rs_light;
	rs_light.depth_test( false );
	rs_light.depth_write( false );
	rs_light.draw_back( true );
	rs_light.draw_front( false );
	rs_light.blend_mode( RenderState::Add );

	m_light_target->clear( true, false );
	m_light_sh_program->set( m_light_uniforms );
	m_light_sh_program->set( "u_eye_position", world_from_camera.t );
	m_light_sh_program->set( "u_world_from_projected", inverse( projected_from_world ) );

	for( int i = 0; i != lights.size(); ++i )
	{
		update_light( lights[i], mesh );
		m_light_sh_program->set( "u_light_position", lights[i].position );
		m_light_sh_program->set( "u_light_colour", lights[i].colour );
		m_light_sh_program->set( "u_light_radius2rec", 1.f / (lights[i].radius * lights[i].radius) );
		m_light_sh_program->set( "u_shadow", lights[i].shadow_map );
		m_light_sh_program->set( "u_near", lights[i].radius / 100.f );
		m_light_sh_program->set( "u_far", lights[i].radius );
		float s = lights[i].radius;
		float44 projected_from_model = projected_from_world *
		                               translation( lights[i].position ) *
		                               scale( float4( s, s, s, 1.f ) );
		m_light_sh_program->set( "u_projected_from_model",  projected_from_model );
		m_icosohedron.draw( *m_light_sh_program, rs_light, *m_light_target );
	}

	// Render geometry with material

	RenderState rs_material;

	rs_material.depth_test( true );
	rs_material.depth_write( false );

	m_shade_program->set( material.uniforms );
	m_shade_program->set( m_shade_uniforms );
	m_shade_program->set( "u_t_model_view_projection",  projected_from_world );

	m_hdr_target->clear( true, false );

	mesh.draw( *m_shade_program, rs_material, *m_hdr_target );


	// Render final image

	m_hdr_program->set( m_hdr_uniforms );

	RenderState rs_quad;

	rs_quad.depth_test( false );
	rs_quad.depth_write( false );
	device.clear();

	m_quad.draw( *m_hdr_program, rs_quad, device );
}

void PPRenderer::update_light( Light &light, Mesh &mesh )
{
	if( light.dirty )
	{
		if( !light.shadow_map )
		{
			int size = 512;
			light.shadow_map.set( new TextureCube( size, size, 1, 0, 0, 0, 0, 0, 0, "dfc" ) );
		}
		float44 proj = perspective( 1.57079632f, 1.f, light.radius / 100.f, light.radius );
		float4 dir[6] = { float4(1,0,0,0), float4(-1,0,0,0), 
		                  float4(0,1,0,0), float4(0,-1,0,0), 
						  float4(0,0,1,0), float4(0,0,-1,0) };

		float4 up[6] =  { float4(0,-1,0,0), float4(0,-1,0,0), 
		                  float4(0,0,1,0),  float4(0,0,-1,0), 
		                  float4(0,-1,0,0), float4(0,-1,0,0) };

		for( int i = 0; i != 6; ++i )
		{
			m_shadow_target->attach( light.shadow_map, i, TextureTarget::Depth );
			m_shadow_target->is_complete();
			m_shadow_target->clear( false, true );
			float44 face_from_world = look_at( light.position, light.position + dir[i], up[i] );
			m_shadow_program->set( "u_projected_from_model", proj * inverse( face_from_world ) );
			mesh.draw( *m_shadow_program, m_shadow_state, *m_shadow_target );
		}
		light.dirty = false;
		light.shadow_map->gen_mipmaps();
	}
}
