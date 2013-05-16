#include "resource/pprenderer.h"

#include "core/device.h"
#include "core/renderstate.h"
#include "core/shaderprogram.h"
#include "core/texturetarget.h"
#include "core/texture.h"

#include "math/frustum.h"

#include "resource/material.h"
#include "resource/mesh.h"
#include "resource/resourcepool.h"
#include "resource/light.h"


PPRenderer::PPRenderer( Device &device, ResourcePool &pool ) :
	m_depth_pass_target( new TextureTarget() ),
	m_gbuf_target( new TextureTarget() ),
	m_light_target( new TextureTarget() ),
	m_hdr_target( new TextureTarget() ),
	m_shadow_target( new TextureTarget() ),
	m_near_shadow_target( new TextureTarget() ),
	m_far_shadow_target( new TextureTarget() ),
	m_quad( make_quad() ),
	m_icosohedron( make_cube() )
{
	int w = device.width(), h = device.height();

	SharedPtr< Texture2D > depthTexture( new Texture2D( w, h, 1, 0, "dfc" ) );
	SharedPtr< Texture2D > normalTexture( new Texture2D( w, h, 3, 0, "c" ) );
	SharedPtr< Texture2D > lightTexture( new Texture2D( w, h, 4, 0, "cs" ) );
	SharedPtr< Texture2D > hdrTexture( new Texture2D( w, h, 3, 0, "cs" ) );

	m_depth_pass_target->attach( depthTexture, TextureTarget::Depth );
	m_depth_pass_target->is_complete();

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
	m_hdr_uniforms.set( "u_depth_texture", depthTexture );

	m_depth_pass_program       = pool.shader_program( "depth_pass.sp" );
	m_gbuf_program             = pool.shader_program( "draw_normals_ar.sp" );
	m_light_program            = pool.shader_program( "draw_lights.sp" );
	m_light_sh_program         = pool.shader_program( "draw_lights_sh.sp" );
	m_shade_program            = pool.shader_program( "use_light_map_ar.sp" );
	m_hdr_program              = pool.shader_program( "hdr.sp" );
	m_shadow_program           = pool.shader_program( "draw_shadow_map.sp" );
	m_shadow_combine_program   = pool.shader_program( "combine_shadow_maps.sp" );

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

	m_gbuf_target->clear( true, true );

	mesh.draw( *m_gbuf_program, *RenderState::stock_opaque(), *m_gbuf_target );


	// Render lights into light target

	RenderState rs_light;

	rs_light.depth_test( true );
	rs_light.depth_write( false );

	m_light_program->set( m_light_uniforms );
	m_light_program->set( "u_t_inv_model_view_projection", inverse( mod_view_pers ) );
	m_light_program->set( "u_light_positions[0]", light_positions );
	m_light_program->set( "u_light_intensities[0]", light_intensities );
	m_light_program->set( "u_light_count", ( int )light_positions.size() );
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

bool dist_sort( SceneMesh const *a, SceneMesh const *b )
{
	return a->distance_from_eye2 < b->distance_from_eye2;
}

void PPRenderer::render( Device &device,
                         float44 const &world_from_camera,
                         float44 const &projected_from_camera,
                         SceneNode &root )
{
	m_lights.clear();
	m_meshes.clear();

	visit_scene( root, *this );

	float3 eye_pos = world_from_camera.t.xyz();

	for( auto m = m_meshes.begin(); m != m_meshes.end(); ++m )
		( *m )->distance_from_eye2 = length_sqr( ( *m )->aabb.mid - eye_pos );

	std::sort( m_meshes.begin(), m_meshes.end(), dist_sort );

	float44 camera_from_world = inverse( world_from_camera );
	float44 projected_from_world = projected_from_camera * camera_from_world;

	Frustum frustum( projected_from_world );

	// Depth pass

	m_depth_pass_program->set( "u_t_model_view_projection",  projected_from_world );
	m_depth_pass_target->clear( false, true );

	RenderState depth_pass_state;
	depth_pass_state.colour_write( false );

	draw_meshes( *m_depth_pass_program, depth_pass_state, *m_depth_pass_target, frustum, projected_from_world );

	// Geometery pass

	m_gbuf_program->set( "u_t_model_view_projection",  projected_from_world );
	m_gbuf_program->set( "u_t_normal", float33() );

	//m_gbuf_target->clear( true, false );
	RenderState gbuf_state;
	gbuf_state.depth_write( false );

	draw_meshes( *m_gbuf_program, gbuf_state, *m_gbuf_target, frustum, projected_from_world );



	// Render lights into light target

	RenderState rs_light;
	rs_light.depth_test( true );
	rs_light.depth_write( false );
	rs_light.blend_mode( RenderState::Add );

	m_light_target->clear( true, false );
	m_light_sh_program->set( m_light_uniforms );
	m_light_sh_program->set( "u_eye_position", world_from_camera.t );
	m_light_sh_program->set( "u_world_from_projected", inverse( projected_from_world ) );


	for( int i = 0; i != m_lights.size(); ++i )
	{
		if( !frustum.intersect_sphere( m_lights[i]->position.xyz(), m_lights[i]->radius ) )
			continue;
		if( length( ( m_lights[i]->position - world_from_camera.t ).xyz() ) > m_lights[i]->radius * 1.5f )
		{
			rs_light.depth_compare( RenderState::LEqual );
			rs_light.draw_back( false );
			rs_light.draw_front( true );
		}
		else
		{
			rs_light.depth_compare( RenderState::Greater );
			rs_light.draw_back( true );
			rs_light.draw_front( false );
		}
		update_light( *m_lights[i] );
		m_light_sh_program->set( m_light_uniforms );
		m_light_sh_program->set( "u_light_position", m_lights[i]->position );
		m_light_sh_program->set( "u_light_colour", m_lights[i]->colour );
		m_light_sh_program->set( "u_light_radius", m_lights[i]->radius );
		m_light_sh_program->set( "u_light_radius2", m_lights[i]->radius * m_lights[i]->radius );
		m_light_sh_program->set( "u_light_radius2rec", 1.f / ( m_lights[i]->radius * m_lights[i]->radius ) );
		m_light_sh_program->set( "u_shadow", m_lights[i]->shadow_map );
		m_light_sh_program->set( "u_near", m_lights[i]->radius / 100.f );
		m_light_sh_program->set( "u_far", m_lights[i]->radius );
		float s = m_lights[i]->radius;
		float44 projected_from_model = projected_from_world *
		                               translation( m_lights[i]->position ) *
		                               scale( float4( s, s, s, 1.f ) );
		m_light_sh_program->set( "u_projected_from_model",  projected_from_model );
		m_icosohedron.draw( *m_light_sh_program, rs_light, *m_light_target );
	}

	// Render geometry with material

	RenderState rs_material;

	rs_material.depth_test( true );
	rs_material.depth_write( false );

	m_shade_program->set( m_shade_uniforms );
	m_shade_program->set( "u_t_model_view_projection",  projected_from_world );

	m_hdr_target->clear( true, false );

	draw_meshes( *m_shade_program, rs_material, *m_hdr_target, frustum, projected_from_world );


	// Render final image

	m_hdr_program->set( m_hdr_uniforms );

	RenderState rs_quad;

	rs_quad.depth_test( true );
	rs_quad.depth_compare( RenderState::Less );
	rs_quad.depth_write( true );

	m_quad.draw( *m_hdr_program, rs_quad, device );
}

void PPRenderer::draw_meshes( ShaderProgram &p, RenderState &s, RenderTarget &t, Frustum const &f, float44 const &projected_from_world )
{
	for( auto m = m_meshes.begin(); m != m_meshes.end(); ++m )
	{
		if( f.intersect_aabb( ( *m )->aabb ) )
		{
			p.set( "u_t_world_from_model",  ( *m )->world_from_model() );
			p.set( "u_t_model_view_projection",  projected_from_world * ( *m )->world_from_model() );
			p.set( ( *m )->material->uniforms );
			( *m )->mesh.draw( p, s, t );
		}
	}
}

void PPRenderer::update_light( SceneLight &light )
{
	if( light.dirty )
	{
		int size = light.shadow_map_size;
		if( !light.shadow_map )
		{
			light.shadow_map.set( new TextureCube( size, 1, 0, 0, 0, 0, 0, 0, "dscp1" ) );
		}

		auto temp_map = m_temp_maps.find( size );
		if( temp_map == m_temp_maps.end() )
		{
			temp_map = m_temp_maps.insert( std::make_pair( size, TempMaps() ) ).first;
			temp_map->second.near_light_texture.set( new Texture2D( size, size, 1,  0, "dfc1" ) );
			temp_map->second.far_light_texture.set(  new Texture2D( size, size, 1,  0, "dfc1" ) );
		}
		SharedPtr< Texture2D > near_light_texture = temp_map->second.near_light_texture;
		SharedPtr< Texture2D > far_light_texture = temp_map->second.far_light_texture;

		m_near_shadow_target->attach( near_light_texture, TextureTarget::Depth );
		m_far_shadow_target->attach( far_light_texture, TextureTarget::Depth );

		float44 proj = perspective( 1.57079632f, 1.f, light.radius / 100.f, light.radius );
		float4 dir[6] = { float4( 1,0,0,0 ), float4( -1,0,0,0 ),
		                  float4( 0,1,0,0 ), float4( 0,-1,0,0 ),
		                  float4( 0,0,1,0 ), float4( 0,0,-1,0 )
		                };

		float4 up[6] =  { float4( 0,-1,0,0 ), float4( 0,-1,0,0 ),
		                  float4( 0,0,1,0 ),  float4( 0,0,-1,0 ),
		                  float4( 0,-1,0,0 ), float4( 0,-1,0,0 )
		                };

		for( int i = 0; i != 6; ++i )
		{
			float44 face_from_world = inverse( look_at( light.position, light.position + dir[i], up[i] ) );
			float44 proj_from_world = proj * face_from_world;
			Frustum frustum( proj_from_world );
			m_shadow_program->set( "u_projected_from_model", proj_from_world );
			m_shadow_state.depth_test( true );

			m_shadow_state.draw_back( false );
			m_shadow_state.draw_front( true );
			m_near_shadow_target->clear( false, true );
			for( auto m = m_meshes.begin(); m != m_meshes.end(); ++m )
			{
				if( frustum.intersect_aabb( (*m)->aabb ) )
				{
					m_shadow_program->set( "u_projected_from_model", proj_from_world * ( *m )->world_from_model() );
					( *m )->mesh.draw( *m_shadow_program, m_shadow_state, *m_near_shadow_target );
				}
			}

			m_shadow_state.draw_back( true );
			m_shadow_state.draw_front( false );
			m_far_shadow_target->clear( false, true );
			for( auto m = m_meshes.begin(); m != m_meshes.end(); ++m )
			{
				if( frustum.intersect_aabb( (*m)->aabb ) )
				{
					m_shadow_program->set( "u_projected_from_model", proj_from_world * ( *m )->world_from_model() );
					( *m )->mesh.draw( *m_shadow_program, m_shadow_state, *m_far_shadow_target );
				}
			}

			m_shadow_target->attach( light.shadow_map, i, TextureTarget::Depth );
			m_shadow_target->is_complete();
			//m_shadow_state.depth_test( false );
			m_shadow_state.draw_back( false );
			m_shadow_state.draw_front( true );
			m_shadow_target->clear( false, true );
			m_shadow_combine_program->set( "u_near_depth", near_light_texture );
			m_shadow_combine_program->set( "u_far_depth", far_light_texture );
			m_shadow_combine_program->set( "u_near", light.radius / 100.f );
			m_shadow_combine_program->set( "u_far", light.radius );
			m_quad.draw( *m_shadow_combine_program, m_shadow_state, *m_shadow_target );
		}
		light.dirty = false;
		light.shadow_map->gen_mipmaps();
	}
}

void PPRenderer::visit( SceneMesh &mesh )
{
	m_meshes.push_back( &mesh );
}

void PPRenderer::visit( SceneLight &light )
{
	m_lights.push_back( &light );
}
