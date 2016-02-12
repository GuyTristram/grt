#include "resource/mesh.h"
#include "resource/font.h"
#include "core/shaderprogram.h"
#include <cstring>

void Mesh::draw( ShaderProgram &sp, RenderState &rs, RenderTarget &rt )
{
	if( ib.get( ) )
		rt.draw( sp, rs, type, *vb, *ib, patch_vertices, instances );
	else if( vb.get() )
		rt.draw( sp, rs, type, *vb, patch_vertices, instances );
}

Mesh make_cube()
{
	static Mesh mesh;
	static bool init = false;
	if( init )
		return mesh;
	init = true;
	mesh.type = RenderTarget::Triangles;
	mesh.vb = VertexBuffer::Ptr( new VertexBuffer );
	mesh.ib = IndexBuffer::Ptr( new IndexBuffer );

	auto pos_att = mesh.vb->add_attribute< float3 >( "a_position" );
	auto uv_att = mesh.vb->add_attribute< float2 >( "a_uv0" );
	auto norm_att = mesh.vb->add_attribute< float3 >( "a_normal" );

	mesh.vb->vertex_count( 4 * 6 );

	auto pos_it = pos_att.begin();


	*pos_it++ = float3( -1.f, -1.f, 1.f );
	*pos_it++ = float3( 1.f, -1.f, 1.f );
	*pos_it++ = float3( 1.f, 1.f, 1.f );
	*pos_it++ = float3( -1.f, 1.f, 1.f );
	mesh.ib->add( 0 ).add( 1 ).add( 2 ).add( 3 ).add( 0 ).add( 2 );
	*pos_it++ = float3( 1.f, -1.f, 1.f );
	*pos_it++ = float3( 1.f, -1.f, -1.f );
	*pos_it++ = float3( 1.f, 1.f, -1.f );
	*pos_it++ = float3( 1.f, 1.f, 1.f );
	mesh.ib->add( 4 ).add( 5 ).add( 6 ).add( 7 ).add( 4 ).add( 6 );
	*pos_it++ = float3( 1.f, -1.f, -1.f );
	*pos_it++ = float3( -1.f, -1.f, -1.f );
	*pos_it++ = float3( -1.f, 1.f, -1.f );
	*pos_it++ = float3( 1.f, 1.f, -1.f );
	mesh.ib->add( 8 ).add( 9 ).add( 10 ).add( 11 ).add( 8 ).add( 10 );
	*pos_it++ = float3( -1.f, -1.f, -1.f );
	*pos_it++ = float3( -1.f, -1.f, 1.f );
	*pos_it++ = float3( -1.f, 1.f, 1.f );
	*pos_it++ = float3( -1.f, 1.f, -1.f );
	mesh.ib->add( 12 ).add( 13 ).add( 14 ).add( 15 ).add( 12 ).add( 14 );
	*pos_it++ = float3( -1.f, 1.f, 1.f );
	*pos_it++ = float3( 1.f, 1.f, 1.f );
	*pos_it++ = float3( 1.f, 1.f, -1.f );
	*pos_it++ = float3( -1.f, 1.f, -1.f );
	mesh.ib->add( 16 ).add( 17 ).add( 18 ).add( 19 ).add( 16 ).add( 18 );
	*pos_it++ = float3( -1.f, -1.f, -1.f );
	*pos_it++ = float3( 1.f, -1.f, -1.f );
	*pos_it++ = float3( 1.f, -1.f, 1.f );
	*pos_it++ = float3( -1.f, -1.f, 1.f );
	mesh.ib->add( 20 ).add( 21 ).add( 22 ).add( 23 ).add( 20 ).add( 22 );

	auto uv_it = uv_att.begin();
	*uv_it++ = float2( 0, 1 );
	*uv_it++ = float2( 1, 1 );
	*uv_it++ = float2( 1, 0 );
	*uv_it++ = float2( 0, 0 );

	*uv_it++ = float2( 0, 1 );
	*uv_it++ = float2( 1, 1 );
	*uv_it++ = float2( 1, 0 );
	*uv_it++ = float2( 0, 0 );

	*uv_it++ = float2( 0, 1 );
	*uv_it++ = float2( 1, 1 );
	*uv_it++ = float2( 1, 0 );
	*uv_it++ = float2( 0, 0 );

	*uv_it++ = float2( 0, 1 );
	*uv_it++ = float2( 1, 1 );
	*uv_it++ = float2( 1, 0 );
	*uv_it++ = float2( 0, 0 );

	*uv_it++ = float2( 0, 1 );
	*uv_it++ = float2( 1, 1 );
	*uv_it++ = float2( 1, 0 );
	*uv_it++ = float2( 0, 0 );

	*uv_it++ = float2( 0, 1 );
	*uv_it++ = float2( 1, 1 );
	*uv_it++ = float2( 1, 0 );
	*uv_it++ = float2( 0, 0 );

	auto norm_it = norm_att.begin();
	*norm_it++ = float3( 0.f, 0.f, 1.f );
	*norm_it++ = float3( 0.f, 0.f, 1.f );
	*norm_it++ = float3( 0.f, 0.f, 1.f );
	*norm_it++ = float3( 0.f, 0.f, 1.f );

	*norm_it++ = float3( 1.f, 0.f, 0.f );
	*norm_it++ = float3( 1.f, 0.f, 0.f );
	*norm_it++ = float3( 1.f, 0.f, 0.f );
	*norm_it++ = float3( 1.f, 0.f, 0.f );

	*norm_it++ = float3( 0.f, 0.f, -1.f );
	*norm_it++ = float3( 0.f, 0.f, -1.f );
	*norm_it++ = float3( 0.f, 0.f, -1.f );
	*norm_it++ = float3( 0.f, 0.f, -1.f );

	*norm_it++ = float3( -1.f, 0.f, 0.f );
	*norm_it++ = float3( -1.f, 0.f, 0.f );
	*norm_it++ = float3( -1.f, 0.f, 0.f );
	*norm_it++ = float3( -1.f, 0.f, 0.f );

	*norm_it++ = float3( 0.f, 1.f, 0.f );
	*norm_it++ = float3( 0.f, 1.f, 0.f );
	*norm_it++ = float3( 0.f, 1.f, 0.f );
	*norm_it++ = float3( 0.f, 1.f, 0.f );

	*norm_it++ = float3( 0.f, -1.f, 0.f );
	*norm_it++ = float3( 0.f, -1.f, 0.f );
	*norm_it++ = float3( 0.f, -1.f, 0.f );
	*norm_it++ = float3( 0.f, -1.f, 0.f );

	return mesh;

}

Mesh make_quad()
{
	Mesh mesh;
	mesh.type = RenderTarget::Fan;
	mesh.vb = VertexBuffer::Ptr( new VertexBuffer );

	auto pos_att  = mesh.vb->add_attribute< float3 >( "a_position" );
	auto uv_att   = mesh.vb->add_attribute< float2 >( "a_uv0" );
	auto norm_att = mesh.vb->add_attribute< float3 >( "a_normal" );

	mesh.vb->vertex_count( 4 );

	auto pos_it = pos_att.begin();

	*pos_it++ = float3( 1.f, 1.f, 0.f );
	*pos_it++ = float3( -1.f, 1.f, 0.f );
	*pos_it++ = float3( -1.f, -1.f, 0.f );
	*pos_it++ = float3( 1.f, -1.f, 0.f );

	auto uv_it = uv_att.begin();
	*uv_it++ = float2( 1, 1 );
	*uv_it++ = float2( 0, 1 );
	*uv_it++ = float2( 0, 0 );
	*uv_it++ = float2( 1, 0 );

	auto norm_it = norm_att.begin();
	*norm_it++ = float3( 0.f, 0.f, 1.f );
	*norm_it++ = float3( 0.f, 0.f, 1.f );
	*norm_it++ = float3( 0.f, 0.f, 1.f );
	*norm_it++ = float3( 0.f, 0.f, 1.f );
	return mesh;
}

Mesh make_text( grt::Font const& font, char const *text )
{
	Mesh mesh;
	mesh.type = RenderTarget::Triangles;
	mesh.vb = VertexBuffer::Ptr( new VertexBuffer );
	mesh.ib = IndexBuffer::Ptr( new IndexBuffer );

	auto pos_att = mesh.vb->add_attribute< float3 >( "a_position" );
	auto uv_att = mesh.vb->add_attribute< float2 >( "a_uv0" );
	auto norm_att = mesh.vb->add_attribute< float3 >( "a_normal" );

	mesh.vb->vertex_count( 4 * std::strlen(text) );

	auto pos_it = pos_att.begin();
	auto uv_it = uv_att.begin();
	auto norm_it = norm_att.begin();
	float x = 0.f;
	int i = 0;
	float xscale = font.image_width() / font.height();
	float yscale = -font.image_height() / font.height();
	while( *text )
	{
		auto char_info = font.char_info( *text );
		*pos_it++ = float3( x + char_info->ul.x * xscale, char_info->ul.y * yscale, 0.f );
		*pos_it++ = float3( x + char_info->ul.x * xscale, char_info->br.y * yscale, 0.f );
		*pos_it++ = float3( x + char_info->br.x * xscale, char_info->ul.y * yscale, 0.f );
		*pos_it++ = float3( x + char_info->br.x * xscale, char_info->br.y * yscale, 0.f );
		*uv_it++ = float2( char_info->ul.x, char_info->ul.y );
		*uv_it++ = float2( char_info->ul.x, char_info->br.y );
		*uv_it++ = float2( char_info->br.x, char_info->ul.y );
		*uv_it++ = float2( char_info->br.x, char_info->br.y );
		*norm_it++ = float3( 0.f, 0.f, 1.f );
		*norm_it++ = float3( 0.f, 0.f, 1.f );
		*norm_it++ = float3( 0.f, 0.f, 1.f );
		*norm_it++ = float3( 0.f, 0.f, 1.f );
		mesh.ib->add( i + 0 ).add( i + 1 ).add( i + 2 ).add( i + 2 ).add( i + 1 ).add( i + 3 );
		i += 4;
		++text;
		x += xscale * char_info->advance / font.image_width();
	}

	return mesh;
}
