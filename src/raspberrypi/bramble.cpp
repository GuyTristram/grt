// Bramble.cpp : Defines the entry point for the console application.
//
#include "mesh.h"
#include "material.h"
#include "device.h"
#include "keyboard.h"
#include "resourcepool.h"

#include "external/stb_image.h"

#include <iostream>

int main2()
{
	Device device;
	ResourcePool pool;

	VertexBuffer vb;
	
	vb.vertex_count( 3 );

	auto pos = vb.add_attribute< float3 >( "a_position" );
	auto col = vb.add_attribute< uchar3 >( "a_colour" );

	auto pos_it = pos.begin();
	*pos_it++ = float3( -1.f, -1.f, 0.f );
	*pos_it++ = float3( 1.f, -1.f, 0.f );
	*pos_it++ = float3( 0.f, 1.f, 0.f );

	auto col_it = col.begin();
	*col_it++ = uchar3( 0, 0, 255 );
	*col_it++ = uchar3( 255, 0, 0 );
	*col_it++ = uchar3( 0, 255, 0 );

	Material material( pool.shader_program( "simple.sp" ) );

	material.uniforms.set( "u_colour", float4( 1.f, 0.f, 1.f, 1.f ) );

	//material.bind();
	//device.draw( Fan, vb );


	Mesh mesh = make_cube();

/*
	int x,y,n;
	unsigned char *data = stbi_load( "koala.png", &x, &y, &n, 0);

	Texture2D::Ptr tex( new Texture2D( x, y, n, (void *)data ) );
	stbi_image_free( data );
*/
	material.uniforms.set( "u_texture", pool.texture2d( "koala.png" ) );
	
	float44 camera;
	camera.t.z = 6.f;

	float aspect = 1920.f / 1080.f;
	float44 projection = perspective( 0.25f, aspect, 0.1f, 100.f );

	for( int i = 0; i != 10000; ++i )
	{
		float44 camera_inverse = inverse( camera );
		float44 mvp = projection * camera_inverse * rotate( float4( 0,1,0,0), i * 0.004f ) * rotate( float4( 1,0,0,0), i * 0.002f );
		float33 norm_trans = rotate( float3( 0,1,0), i * 0.004f ) * rotate( float3( 1,0,0), i * 0.002f );

		material.uniforms.set( "u_t_model_view_projection", mvp );
		material.uniforms.set( "u_t_normal", norm_trans );

		material.bind();
		mesh.draw( *material.program, *material.state, device );
		device.swap();
/*
		if( key_down( KEY_W ) )
			camera.t.z -= 0.01f;
		if( key_down( KEY_S ) )
			camera.t.z += 0.01f;
		if( key_down( KEY_A ) )
			camera.t.x -= 0.01f;
		if( key_down( KEY_D ) )
			camera.t.x += 0.01f;
		if( key_down( KEY_ESC ) )
			break;
*/
	}

	std::cout << "hello\n";
	int i;

	std::cin >> i;
	return 0;
}
