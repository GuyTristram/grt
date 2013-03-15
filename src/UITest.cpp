// Bramble.cpp : Defines the entry point for the console application.
//
#include "UI/UIContext.h"
#include "UI/UIPicture.h"
#include "UI/UIFrame.h"
#include "UI/UIContext.h"
#include "UI/UIButton.h"
#include "UI/UIBorder.h"
#include "UI/UIText.h"
#include "device.h"
#include "resourcepool.h"
#include "font.h"
#include "material.h"
#include "textureatlas.h"
#include "texturetarget.h"
#include "input.h"
#include "mesh.h"
#include "time.h"
#include "perlin.h"
#include "pprenderer.h"
#include "light.h"

#include "external/stb_image.h"

#include <iostream>

using namespace UI;

int mainUI()
{
	Device device;
	Input input( device.width(), device.height() );

	ResourcePool pool;
	UIContext context( device );
	Rect screen_rect( 0.f, 0.f, float(device.width()), float(device.height()) );

	UIThing *root( new UIThing( 0, screen_rect ) );

	//Font::Ptr font = pool.font( "/usr/share/fonts/truetype/freefont/FreeSans.ttf", 14 );
	//Font::Ptr font = pool.font( "/usr/share/fonts/truetype/ttf-dejavu/DejaVuSans.ttf", 50 );
	Font::Ptr font = pool.font( "FreeSans.ttf", 50 );

	UIFrame *frame = new UIFrame( root , Rect( 400, 80, 720, 500 ), "Text" );
	UI::Rect text_rect( frame->child_rect() );
	text_rect.grow( -2 );
	UIBorder *border = new UIBorder( frame, text_rect );
	text_rect.grow( -4 );
	UIText *text = new UIText( frame, text_rect, "", font );

	Rect frame_rect = UIFrame::calc_frame_rect( Rect( 0, 0, font->image_width(), font->image_height() ) + float2(60, 50 ) );
	frame_rect.grow( 6 );
	frame = new UIFrame( root , frame_rect, "Font" );
	UI::Rect font_rect( frame->child_rect() );
	font_rect.grow( -2 );
	border = new UIBorder( frame, font_rect );
	font_rect.grow( -4 );
	UIPicture *pic = new UIPicture( frame, font_rect, Rect( 0, 0, 1, 1 ), font->material() );

	UIPicture *mouse = new UIPicture( root, Rect( 0, 0, 16, 16 ), Rect( 0, 0, 1, 1 ), pool.texture2d( "cursor.png" ) );

	while( true )//!device.has_quit() )
	{
		input.do_messages( *root );
		mouse->rect( Rect( root->mouse_position(), root->mouse_position() + float2( 32, 32 ) ) );
		root->move_to_end( mouse );
		root->draw_all( context );
		context.flush();
		device.swap();
	}
	delete root;
	return 0;
}

class CubeApp : public Input::EventHandler
{
public:
	void run()
	{
		Device device;
		Input input( device.width(), device.height() );
		ResourcePool pool;

		Texture2D::Ptr textureRender( new Texture2D( device.width(), device.height(), 3, 0 ) );
		//TextureTarget textureTarget;
		//textureTarget.attach( textureRender, TextureTarget::Colour );

		TextureCube::Ptr cubeTexture = pool.texture_cube( "../src/cube/", ".jpg" );
		Mesh cubeMesh = make_quad();
		Material cubeMaterial;
		cubeMaterial.program = pool.shader_program( "skybox.sp" );
		cubeMaterial.uniforms.set( "u_texture", cubeTexture );

		float angle = 0.f;
		float4 axis(0.f, 1.f, 0.f, 0.f);
		float4 axis2(0.f, 0.f, 1.f, 0.f);

		float aspect = device.width() / float( device.height() );
		RealTime timer;
		float next_sec = 1.0;
		int frame_count = 0;
		while( true )
		{
			++frame_count;
			if( timer.elapsed() > next_sec )
			{
				printf( "fps %d\n", frame_count );
				frame_count = 0;
				++next_sec;
			}
			input.do_messages( *this );
			angle = float( timer.elapsed() * 0.2f ); 
			float44 m = perspective( .5f, aspect, 1.f, 10.f ) * rotate(axis, angle) * rotate(axis2, angle*0.1f);
			float44 inverse_m = inverse( m );
			cubeMaterial.uniforms.set( "u_t_transform", inverse_m );
			cubeMaterial.bind();
			cubeMesh.draw( *cubeMaterial.program, *cubeMaterial.state, device );
			device.swap();
		}
	}
};

#include "voxelbox.h"

class VoxelApp : public Input::EventHandler
{
	typedef unsigned char Voxel;

	float3 m_player_pos;
	float3 m_player_vel;

	float m_player_heading;
	float m_player_pitch;
	Device m_device;
	Input m_input;
	float m_update_interval;
	bool m_mouse_rel;
	bool m_collided;
	bool m_collided_floor;
	std::vector< float4 > m_light_positions;
	std::vector< float > m_light_intensities;
	std::vector< Light > m_lights;

	static const int BOX_SIZE = 64;
	static const int MESH_BOX_SIZE = 16;
	VoxelBox< Mesh > m_meshes;

	Mesh m_mesh;
public:
	VoxelApp() :
		m_voxels( 64, 64, 64 ),
		m_meshes( BOX_SIZE/MESH_BOX_SIZE, BOX_SIZE/MESH_BOX_SIZE, BOX_SIZE/MESH_BOX_SIZE ),
		m_player_pos( 15, 11.5, 20 ),
		m_player_heading( 0.f ),
		m_player_pitch( 0.f ),
		m_player_vel( 0.f, 0.f, 0.f ),
		m_input( m_device.width(), m_device.height() ),
		m_update_interval( 1.0f / 60.0f ),
		m_mouse_rel( true ),
		m_collided( false ),
		m_collided_floor( false ),
		m_current_type( 1 )
	{
	}

	void render()
	{
	}

	void run()
	{
		m_input.set_mouse_position( float2( 300.0f, 300.0f ) );
		PPRenderer renderer( m_device, m_pool );

		make_atlas();

		m_voxels.at( -1, -1, -1 ) = 1;
		fill( m_voxels, Voxel(2), 0, 0, 0, 64, 64, 64 );

		fill( m_voxels, Voxel(0), 10, 10, 10, 20, 16, 30 );
		fill( m_voxels, Voxel(4), 14, 10, 25, 26, 11, 26 );

		fill( m_voxels, Voxel(0), 15, 10, 1, 16, 13, 40 );
		fill( m_voxels, Voxel(0), 3, 10, 3, 40, 12, 4 );
		//fill( m_voxels, Voxel(0), 10, 10, 10, 11, 11, 11 );
		m_mesh = make_mesh( int3( 0, 0, 0), int3( 64, 64, 64 ) );

		Material mat;

		float aspect = m_device.width() / float( m_device.height() );
		float next_interval = 0.0f;
		RealTime timer;
		float next_sec = 1.0;
		int frame_count = 0;


		//mat.uniforms.set( "u_texture", m_pool.texture2d( "../src/Stone Rough 02_Diffuse.tga" ) );
		//mat.uniforms.set( "u_texture", m_pool.texture2d( "../src/Stone Medium 01_Diffuse.tga" ) );
		//mat.uniforms.set( "u_texture", m_pool.texture2d( "../src/Stone_Wall_01_Diffuse.tga" ) );


		//mat.uniforms.set( "u_texture2", m_pool.texture2d( "../src/Stone Mossy 01_Diffuse.tga" ) );
		//mat.uniforms.set( "u_specular", m_pool.texture2d( "../src/Stone Wall_03_Specular.tga" ) );
		//mat.uniforms.set( "u_texture", m_pool.texture2d( "../src/Stone_Wall_03_Diffuse.tga" ) );
		//mat.uniforms.set( "u_normal", m_pool.texture2d( "../src/Stone Wall 03_Normal.tga" ) );
		mat.uniforms.set( "u_specular", m_specularTexture );
		mat.uniforms.set( "u_texture", m_diffuseTexture );
		mat.uniforms.set( "u_normal", m_normalTexture );
		mat.uniforms.set( "u_texture2", m_grungeTexture );


		//mat.uniforms.set( "u_texture", m_pool.texture2d( "../src/Brick_Pavers_01_Diffuse_r.tga" ) );

		Mesh quad = make_quad();
		ShaderProgram::Ptr quadPog = m_pool.shader_program( "quad.sp" );
		Texture2D::Ptr t = ResourcePool::stock().texture2d( "../src/stone3.jpg" );
		quadPog->set( "u_source", m_normalTexture );

		while( true )
		{
			++frame_count;
			if( timer.elapsed() > next_sec )
			{
				printf( "fps %d\n", frame_count );
				frame_count = 0;
				++next_sec;
			}
			m_input.do_messages( *this );
			while( timer.elapsed() > next_interval )
			{
				do_tick();
				next_interval += m_update_interval;
			}

			//m_light_positions[0] = float4( distort( m_player_pos + float3( 0.f, 1.7f, 0.f ) ), 1.f );
			float44 cameraMatrix = rotate( float4(0,1,0,0), m_player_heading ) * rotate( float4(1,0,0,0), m_player_pitch );
			cameraMatrix.t =  float4( distort( m_player_pos + float3( 0.f, 1.5f, 0.f ) ), 1.f );
			float44 pers = perspective( 1.2f, aspect, 0.1f, 200.f );

			//renderer.render( m_device, m_mesh, mat, cameraMatrix, pers, m_light_positions, m_light_intensities );
			renderer.render( m_device, m_mesh, mat, cameraMatrix, pers, m_lights );
			//quad.draw( *quadPog, *RenderState::stock_opaque(), m_device );
			m_device.swap();
		}
	}

	bool uncollide( int3 const &b, float3 &c, float r )
	{
		float3 bmin( (float)b.x, (float)b.y, (float)b.z );
		float3 bmax( (float)b.x + 1.f, (float)b.y + 1.f, (float)b.z + 1.f );
		float r2 = r * r;
		float dmin2 = 0;
		float3 offset( 0, 0, 0 );
		for( int i = 0; i < 3; i++ )
		{
			if( c[i] < bmin[i] )
			{
				offset[i] = ( c[i] - bmin[i] );
				dmin2 += offset[i] * offset[i];
			}
			else if( c[i] > bmax[i] )
			{
				offset[i] = ( c[i] - bmax[i] );
				dmin2 += offset[i] * offset[i];
			}
		}
		if( dmin2 <= r2 )
		{
			float d = sqrt( dmin2 );
			c += unit( offset ) * ( r - d );
			return true;
		}
		return false;
	}

	bool uncollide( float3 &c, float r )
	{
		int3 b( (int)floor(c.x), (int)floor(c.y), (int)floor(c.z) );
		int3 b0( (int)floor(c.x-r), (int)floor(c.y-r), (int)floor(c.z-r) );
		int3 b1( (int)floor(c.x+r), (int)floor(c.y+r), (int)floor(c.z+r) );

		int3 face_cube[3], edge_cube[3], corner_cube;
		int n_face = 0, n_edge = 0, n_corner = 0;
		bool test_edge[3] = {true, true, true};

		bool collided = false;
		for( int z = b0.z; z <= b1.z; ++z )
			for( int y = b0.y; y <= b1.y; ++y )
				for( int x = b0.x; x <= b1.x; ++x )
				{
					int3 c( x, y, z );
					switch( manhattan( c - b ) )
					{
					case 1:
						face_cube[n_face++] = c;
						break;
					case 2:
						edge_cube[n_edge++] = c;
						break;
					case 3:
						corner_cube = c;
						n_corner = 1;
						break;
					};
				}

		for( int f = 0; f < n_face; ++f )
		{
			if( m_voxels.at( face_cube[f] ) != 0 )
			{
				collided = uncollide( face_cube[f], c, r ) || collided;
				for( int e = 0; e < n_edge; ++e )
					if( manhattan( face_cube[f] - edge_cube[e] ) == 1 )
						test_edge[e] = false;
				n_corner = 0;
			}
		}

		for( int e = 0; e < n_edge; ++e )
			if( test_edge[e] && m_voxels.at( edge_cube[e] ) != 0 )
			{
				collided = uncollide( edge_cube[e], c, r ) || collided;
				n_corner = 0;
			}

		if( n_corner && m_voxels.at( corner_cube ) != 0 )
			collided = uncollide( corner_cube, c, r ) || collided;

		return collided;
	}

	void do_tick()
	{
		float speed = 5.0f * m_update_interval;
		const float radius = 0.4f;
		const float height = 1.5f;

		int ix = (int)m_player_pos.x;
		int iy = (int)m_player_pos.y;
		int iz = (int)m_player_pos.z;


		m_player_vel += m_update_interval * float3(0.f, -10.0f, 0.f);

		if( true )//m_collided )
		{
			float3 acc( 0.f, 0.f, 0.f );
			float33 player_rel = rotate( float3(0,1,0), m_player_heading );
			if( m_input.key_down( Keys::w ) )
				acc += player_rel * float3(0.f, 0.f, -1.f);
			if( m_input.key_down( Keys::s ) )
				acc += player_rel * float3(0.f, 0.f, 1.f);
			if( m_input.key_down( Keys::a ) )
				acc += player_rel * float3(-1.f, 0.f, 0.f);
			if( m_input.key_down( Keys::d ) )
				acc += player_rel * float3(1.f, 0.f, 0.f);
			m_player_vel += unit( acc ) * 0.7f ;
			m_player_vel.x = m_player_vel.x * 0.8f;
			m_player_vel.z = m_player_vel.z * 0.8f;
		}


		if( m_input.key_down( Keys::space ) )
		{
			/*
			if( m_voxels.at( (int)floor( m_player_pos.x ),
			                 (int)floor( m_player_pos.y - 0.01f ),
			                 (int)floor( m_player_pos.z ) ) != 0 )
			*/
			if( m_collided_floor )
				//m_player_vel += float3(0.f, 5.0f, 0.f);
				m_player_vel.y = 4.2f;//std::min( m_player_vel.y + 5.0, 5.0 );
		}
		float3 new_pos = m_player_pos + m_update_interval * m_player_vel;
		float3 uncol_pos = new_pos;

		m_collided = false;
		new_pos.y += radius;
		m_collided = uncollide( new_pos, radius ) || m_collided;
		new_pos.y -= radius;

		new_pos.y += height;
		m_collided = uncollide( new_pos, radius ) || m_collided;
		new_pos.y -= height;

		m_collided_floor = (new_pos - uncol_pos).y > 0.f;

		m_player_vel = (new_pos - m_player_pos) / m_update_interval;
		m_player_pos = new_pos;

	}
	
	class Traverser
	{
	public:
		Traverser( VoxelBox< Voxel > &b ) : m_b( b ), passed_through( false ) {}
		bool operator()( const int3 &i, const float3 &p )
		{
			if( m_b.at( i ) != 0 )
			{
				result = i;
				hit = p;
				return false;
			}
			else
			{
				result2 = i;
				passed_through = true;
			}
			return true;
		}

		VoxelBox< Voxel > &m_b;
		int3 result;
		int3 result2;
		bool passed_through;
		float3 hit;
	};

	virtual void do_event( InputEvent const &event )
	{
		if( event.type == InputEvent::MouseMove )
		{
			if( m_mouse_rel )
			{
				//printf( "mouse %d %d\n", (int)event.position.x, (int)event.position.y );
				float2 new_pos = m_input.get_mouse_position();
				m_player_heading += ( new_pos.x - 300 ) * 0.001f;
				m_player_pitch += ( new_pos.y - 300 ) * 0.001f;
				if( new_pos.x != 300 || new_pos.y != 300 )
				{
					m_input.set_mouse_position( float2( 300.f, 300.f ) );
				}
			}
		}
		else if( event.type == InputEvent::KeyDown )
		{
			if( event.character == Keys::escape )
			{
				printf( "esc\n" );
				m_mouse_rel = !m_mouse_rel;
			}
			if( event.character == Keys::n1 )
			{
				m_current_type = 1;
			}
			if( event.character == Keys::n2 )
			{
				m_current_type = 2;
			}
			if( event.character == Keys::n3 )
			{
				m_current_type = 3;
			}
			if( event.character == Keys::n4 )
			{
				m_current_type = 4;
			}
			if( event.character == Keys::mouse0 )
			{
				float33 m = rotate( float3(0,1,0), m_player_heading ) *
				            rotate( float3(1,0,0), m_player_pitch );
				Traverser t( m_voxels );
				traverse( m_player_pos + float3( 0.f, 1.5f, 0.f ), -m.k, t );
				m_voxels.at( t.result ) = 0;
				m_mesh = make_mesh( int3( 0, 0, 0), int3( 64, 64, 64 ) );
				for( int i = 0; i != m_light_positions.size(); ++i )
					m_lights[i].dirty = true;
			}
			if( event.character == Keys::mouse1 )
			{
				float33 m = rotate( float3(0,1,0), m_player_heading ) *
				            rotate( float3(1,0,0), m_player_pitch );
				Traverser t( m_voxels );
				traverse( m_player_pos + float3( 0.f, 1.5f, 0.f ), -m.k, t );
				if( t.passed_through )
				{
					m_voxels.at( t.result2 ) = m_current_type;
					m_mesh = make_mesh( int3( 0, 0, 0), int3( 64, 64, 64 ) );
					for( int i = 0; i != m_light_positions.size(); ++i )
						m_lights[i].dirty = true;
				}
			}
			if( event.character == Keys::l && m_light_positions.size() < 256 )
			{
				float33 m = rotate( float3(0,1,0), m_player_heading ) *
				            rotate( float3(1,0,0), m_player_pitch );
				Traverser t( m_voxels );
				traverse( m_player_pos + float3( 0.f, 1.5f, 0.f ), -m.k, t );
				if( t.passed_through )
				{
					float3 p = t.hit + 0.5f * ( float3( t.result2 ) - float3( t.result ) );
					//float4 pos( t.result2.x + 0.5f, t.result2.y + 0.5f, t.result2.z + 0.5f, 1.f );
					float4 pos( p, 1.f );
					for( int i = 0; i != m_light_positions.size(); ++i )
						if( length( pos - m_light_positions[i] ) < 0.8f )
						{
							//m_light_intensities[i] += 1.0f;
							m_lights[i].colour += 10.0f * float4( 1.f, 1.f, 1.f, 0.f );
							m_lights[i].radius += 1.f;
							m_lights[i].dirty = true;
							return;
						}
					m_light_positions.push_back( pos );
					m_light_intensities.push_back( 1.0f );
					m_lights.push_back( Light() );
					m_lights.back().position = pos;
				}
			}
		}
	}

	float3 distort( float3 const &in )
	{
		/*
		static const float3 off1( 17.f, 23.f, 31.f );
		static const float3 off2( 5.f, 11.f, 37.f );
		return in + 
			//4.f * float3( perlin( 0.1f * in ), perlin( 0.1f * (in + off1) ), perlin( 0.1f * (in + off2) ) );

			0.4f * float3( perlin( 0.125f * in ), perlin( 0.125f * (in + off1) ), perlin( 0.125f * (in + off2) ) ) +
			0.2f * float3( perlin( 0.25f * in ), perlin( 0.25f * (in + off1) ), perlin( 0.25f * (in + off2) ) ) +
			0.1f * float3( perlin( 0.5f * in ), perlin( 0.5f * (in + off1) ), perlin( 0.5f * (in + off2) ) ) +
			0.05f * float3( perlin( 1.0f * in ), perlin( 1.0f * (in + off1) ), perlin( 1.0f * (in + off2) ) );
			*/
		return in;
	}

	Mesh make_mesh( int3 const v0, int3 const v1 )
	{
		struct SideFlags { bool fill[6]; };
		VoxelBox< SideFlags > flags( v1 - v0 );

		int nFaces = 0;
		int3 adj[6] = {int3(1,0,0), int3(-1,0,0), int3(0,1,0), int3(0,-1,0), int3(0,0,1), int3(0,0,-1)};

		int3 v;
		for( v.z = v0.z; v.z < v1.z; ++v.z )
			for( v.y = v0.y; v.y < v1.y; ++v.y )
				for( v.x = v0.x; v.x < v1.x; ++v.x )
				{
					SideFlags &f = flags.at( v - v0 );
					if( m_voxels.at( v ) != 0 )
					{
						for( int i = 0; i != 6; ++i )
							if( f.fill[i] = ( m_voxels.at( v + adj[i] ) == 0 ) )
								++nFaces;
					}
					else
						for( int i = 0; i != 6; ++i )
							f.fill[i] = false;
				}

		Mesh mesh;
		mesh.type = RenderTarget::Triangles;
		mesh.vb = VertexBuffer::Ptr( new VertexBuffer );
		mesh.ib = IndexBuffer::Ptr( new IndexBuffer );
		mesh.vb->vertex_count( nFaces * 4 );

		auto pos_att = mesh.vb->add_attribute< float3 >( "a_position" );
		auto norm_att = mesh.vb->add_attribute< char3 >( "a_normal" );
		auto tan_att = mesh.vb->add_attribute< char3 >( "a_tangent" );
		auto uv_att = mesh.vb->add_attribute< float2 >( "a_uv0" );
		auto index_att = mesh.vb->add_attribute< uchar3 >( "a_index" );
		auto noise_att = mesh.vb->add_attribute< signed char >( "a_noise" );

		auto pos_it = pos_att.begin();
		auto norm_it = norm_att.begin();
		auto tan_it = tan_att.begin();
		auto uv_it = uv_att.begin();
		auto index_it = index_att.begin();
		auto noise_it = noise_att.begin();

		int nVert = 0;
		float3 pos_off[4*6] = { float3(1,0,1), float3(1,0,0), float3(1,1,0), float3(1,1,1),
		                        float3(0,0,0), float3(0,0,1), float3(0,1,1), float3(0,1,0),
		                        float3(0,1,1), float3(1,1,1), float3(1,1,0), float3(0,1,0),
		                        float3(0,0,0), float3(1,0,0), float3(1,0,1), float3(0,0,1),
								float3(0,0,1), float3(1,0,1), float3(1,1,1), float3(0,1,1),
		                        float3(1,0,0), float3(0,0,0), float3(0,1,0), float3(1,1,0) };

		float2 uvs[4] = { float2( 0, 1 ), float2( 1, 1 ), float2( 1, 0 ), float2( 0, 0 ) };
		char3 tan[6] = {char3(0,0,-127), char3(0,0,127), char3(127,0,0), char3(127,0,0), char3(127,0,0), char3(-127,0,0)};

		int index = 0;

		for( v.z = v0.z; v.z < v1.z; ++v.z )
			for( v.y = v0.y; v.y < v1.y; ++v.y )
				for( v.x = v0.x; v.x < v1.x; ++v.x )
				{
					SideFlags &f = flags.at( v - v0 );
					int type = m_voxels.at( v );
					VoxType &t = m_vox_type[ type ];
					for( int i = 0; i != 6; ++i )
						if( f.fill[i]  )
						{
							float3 origin( v );
							float3 pos;
							char3 norm( 127 * adj[i] );
							for( int vert = 0, ind = i * 4; vert !=4; ++vert, ++ind )
							{
								*pos_it++ = distort( origin + pos_off[ind] );
								*norm_it++ = norm;
								*tan_it++ = tan[i];
								*uv_it++ = ( uvs[vert] * ( t.max_uv - t.min_uv ) + t.min_uv );
								*index_it++ = uchar3( v );
								*noise_it++ = (unsigned char)127 * perlin( 4.0f * (origin + pos_off[ind]) );
							}
							++index;
							mesh.ib->add( nVert + 0 ).add( nVert + 1 ).add( nVert + 2 );
							mesh.ib->add( nVert + 3 ).add( nVert + 0 ).add( nVert + 2 );
							nVert += 4;
						}
				}
		return mesh;
	}

	void make_atlas()
	{
		const char *names[] = {"Stone Wall 03_"};
        const char *suffix[] = {"Diffuse.tga", "Normal.tga", "Specular.tga"};

		std::vector< std::string > n_names;
		n_names.push_back( "../src/Stone_Wall_03_Diffuse.tga" );
		n_names.push_back( "../src/Stone Rough 03_Diffuse.tga" );
		n_names.push_back( "../src/Brick_Pavers_01_Diffuse.tga" );
		n_names.push_back( "../src/Concrete Bare 01_Diffuse.tga" );

		TextureAtlas diffuse( n_names, 2048, 256 );

		n_names.clear();
		n_names.push_back( "../src/Stone Wall 03_Normal.tga" );
		n_names.push_back( "../src/Stone Rough 03_Normal.tga" );
		n_names.push_back( "../src/Brick_Pavers_01_Normal.tga" );
		n_names.push_back( "../src/Concrete Bare 01_Normal.tga" );
		TextureAtlas normal( n_names, 2048, 256 );

		n_names.clear();
		n_names.push_back( "../src/Stone Wall_03_Specular.tga" );
		n_names.push_back( "../src/Stone Rough 03_Specular.tga" );
		n_names.push_back( "../src/Brick_Pavers_01_Specular.tga" );
		n_names.push_back( "../src/Concrete Bare 01_Specular.tga" );
		TextureAtlas specular( n_names, 2048, 256 );

		n_names.clear();
		n_names.push_back( "../src/Stone Mossy 01_Diffuse.tga" );
		n_names.push_back( "../src/Stone Mossy 01_Diffuse.tga" );
		n_names.push_back( "../src/Stone Mossy 01_Diffuse.tga" );
		n_names.push_back( "../src/Concrete Bare 01_Diffuse.tga" );
		TextureAtlas grunge( n_names, 2048, 256 );

		m_diffuseTexture  = diffuse.texture();
		m_normalTexture   = normal.texture();
		m_specularTexture = specular.texture();
		m_grungeTexture   = grunge.texture();

		m_vox_type[1].min_uv = diffuse.min_uv( 0 );
		m_vox_type[1].max_uv = diffuse.max_uv( 0 );
		m_vox_type[2].min_uv = diffuse.min_uv( 1 );
		m_vox_type[2].max_uv = diffuse.max_uv( 1 );
		m_vox_type[3].min_uv = diffuse.min_uv( 2 );
		m_vox_type[3].max_uv = diffuse.max_uv( 2 );
		m_vox_type[4].min_uv = diffuse.min_uv( 3 );
		m_vox_type[4].max_uv = diffuse.max_uv( 3 );
	}

	struct VoxType
	{
		float2 min_uv;
		float2 max_uv;
	};
private:
	VoxType m_vox_type[5];
	int m_current_type;
	VoxelBox< Voxel > m_voxels;
	ResourcePool m_pool;
	Texture2D::Ptr m_diffuseTexture;
	Texture2D::Ptr m_grungeTexture;
	Texture2D::Ptr m_normalTexture;
	Texture2D::Ptr m_specularTexture;
	int m_block_type_count;
	Material m_material;
};

int main()
{
	//CubeApp app;
	VoxelApp app;
	app.run();
}
