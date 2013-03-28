#include "resource/scenenode.h"
#include "resource/resourcepool.h"
#include <algorithm>
// assimp include files. These three are usually needed.
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

SceneNode::SceneNode() : m_dirty( false ), m_parent( 0 ) {}

SceneNode::~SceneNode()
{
	for( auto i = begin(); i != end(); ++i )
		( *i )->m_parent = 0;
}


void SceneNode::set_parent( SceneNode *parent )
{
	if( m_parent == parent )
		return;

	// Make sure this is not deleted after removing from parent;
	Ptr p( this );

	if( m_parent )
	{
		auto vec = &m_parent->m_children;
		vec->erase( std::remove( vec->begin(), vec->end(), p ), vec->end() );
	}

	m_parent = parent;
	if( m_parent )
	{
		m_parent->m_children.push_back( p );
		set_dirty();
	}
}

float44 const &SceneNode::world_from_model()
{
	if( m_dirty )
	{
		if( m_parent )
			m_world_from_model = m_parent->world_from_model();
		modify_transform();
		m_dirty = false;
	}

	return m_world_from_model;
}

void SceneNode::accept( SceneNodeVisitor &visitor )
{
	visitor.visit( *this );
}

void SceneNode::parent_from_local( float44 const &m )
{
	m_dirty = true;
	m_parent_from_local = m;
}


void SceneNode::modify_transform()
{
	m_world_from_model = m_world_from_model * m_parent_from_local;
}

SceneNode::Iterator SceneNode::begin()
{
	return m_children.begin();
}

SceneNode::Iterator SceneNode::end()
{
	return m_children.end();
}

void SceneNode::set_dirty()
{
	// Avoid repeatedly setting dirty on all descendants
	if( m_dirty )
		return;

	m_dirty = true;

	for( auto i = begin(); i != end(); ++i )
		( *i )->set_dirty();
}

void SceneNodePosition::set( float4 const &position )
{
	m_position = position;
	set_dirty();
}

void SceneNodePosition::modify_transform()
{
	m_world_from_model.t = m_world_from_model * m_position;
}

void SceneMesh::accept( SceneNodeVisitor &visitor )
{
	visitor.visit( *this );
}

void SceneLight::accept( SceneNodeVisitor &visitor )
{
	visitor.visit( *this );
}

void visit_scene( SceneNode &node, SceneNodeVisitor &visitor )
{
	node.accept( visitor );
	for( auto i = node.begin(); i != node.end(); ++i )
		visit_scene( **i, visitor );
}


class AILoader
{
public:
	AILoader( char const *filename )
	{
		m_scene = aiImportFile( filename, aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_FlipUVs );
		load_meshes();
		load_materials();
		m_root = load_node( *m_scene->mRootNode );
	}

	SceneNode::Ptr root() { return m_root; }

	SceneNode::Ptr load_node( aiNode &ai_node )
	{
		SceneNode::Ptr node;
		if( ai_node.mNumMeshes == 1 )
		{
			SceneMesh::Ptr mesh( new SceneMesh() );
			node = mesh;
			mesh->mesh = m_meshes[ ai_node.mMeshes[0] ];
			mesh->material = m_materials[ m_scene->mMeshes[ ai_node.mMeshes[0] ]->mMaterialIndex ];
			mesh->aabb.half_size = float3( 10000.f, 10000.f, 10000.f );
			mesh->aabb.mid = float3( 0.f, 0.f, 0.f );
		}
		else
		{
			node.set( new SceneNode );
		}

		aiMatrix4x4 m = ai_node.mTransformation;
		aiTransposeMatrix4(&m);
		node->parent_from_local( float44( float4( m.a1, m.a2, m.a3, m.a4 ), float4( m.b1, m.b2, m.b3, m.b4 ),
		                                  float4( m.c1, m.c2, m.c3, m.c4 ), float4( m.d1, m.d2, m.d3, m.d4 ) ) );
		for( unsigned int i = 0; i < ai_node.mNumChildren; ++i )
			load_node( *ai_node.mChildren[i] )->set_parent( node.get() );

		return node;
	}

	void set_texture( Material &material, aiMaterial &ai_material, aiTextureType type, char const *uniform, char const *default_name )
	{
		if( ai_material.GetTextureCount( type ) )
		{
			aiString name;
			ai_material.GetTexture( type, 0, &name );
			material.uniforms.set( uniform, m_pool.texture2d( name.C_Str() ) );
		}
		else
			material.uniforms.set( uniform, m_pool.texture2d( default_name ) );
	}

	void load( Material &material, aiMaterial &ai_material )
	{
		material.program = m_pool.shader_program( "use_light_map2.sp" );
		set_texture( material, ai_material, aiTextureType_DIFFUSE, "u_texture", "white.png" );
		set_texture( material, ai_material, aiTextureType_SPECULAR, "u_specular", "white.png" );
		set_texture( material, ai_material, aiTextureType_NORMALS, "u_normal", "flat.png" );
	}

	void load_materials()
	{
		m_materials.reserve( m_scene->mNumMaterials );
		for( int i = 0; i != m_scene->mNumMaterials; ++i )
		{
			m_materials.push_back( Material::Ptr( new Material ) );
			load( *m_materials.back(), *m_scene->mMaterials[i] );
		}
	}

	void load( Mesh &mesh, aiMesh &ai_mesh )
	{
		mesh.vb.set( new VertexBuffer );
		mesh.ib.set( new IndexBuffer );
		switch( ai_mesh.mPrimitiveTypes )
		{
		case aiPrimitiveType_POINT:      mesh.type = RenderTarget::Points;    break;
		case aiPrimitiveType_LINE:       mesh.type = RenderTarget::Lines;     break;
		case aiPrimitiveType_TRIANGLE:   mesh.type = RenderTarget::Triangles; break;
		default:                         mesh.type = RenderTarget::Points;
			printf( "unsupported primitive\n" );
		}

		VertexAttribute< float3 > pos_att;
		VertexAttribute< float2 > uv_att;
		VertexAttribute< float3 > norm_att;
		VertexAttribute< float3 > tan_att;

		if( ai_mesh.HasPositions() )
			pos_att = mesh.vb->add_attribute< float3 >( "a_position" );
		if( ai_mesh.HasTextureCoords( 0 ) )
			uv_att = mesh.vb->add_attribute< float2 >( "a_uv0" );
		if( ai_mesh.HasNormals() )
			norm_att = mesh.vb->add_attribute< float3 >( "a_normal" );
		if( ai_mesh.HasTangentsAndBitangents() )
			tan_att = mesh.vb->add_attribute< float3 >( "a_tangent" );

		int vertex_count = ai_mesh.mNumVertices;
		mesh.vb->vertex_count( vertex_count );
		float3 av;

		if( ai_mesh.HasPositions() )
		{
			auto pos_it = pos_att.begin();
			for( int i = 0; i < vertex_count; ++i )
				*pos_it++ = float3( ai_mesh.mVertices[i].x, ai_mesh.mVertices[i].y, ai_mesh.mVertices[i].z );

			av = float3( ai_mesh.mVertices[0].x, ai_mesh.mVertices[0].y, ai_mesh.mVertices[0].z );

			for( int i = 1; i < vertex_count; ++i )
				av += float3( ai_mesh.mVertices[i].x, ai_mesh.mVertices[i].y, ai_mesh.mVertices[i].z );

			av = av * 1.f/float( vertex_count );
		}
		if( ai_mesh.HasNormals() )
		{
			auto norm_it = norm_att.begin();
			for( int i = 0; i < vertex_count; ++i )
				*norm_it++ = float3( ai_mesh.mNormals[i].x, ai_mesh.mNormals[i].y, ai_mesh.mNormals[i].z );
		}
		if( ai_mesh.HasTextureCoords( 0 ) )
		{
			auto uv_it = uv_att.begin();
			for( int i = 0; i < vertex_count; ++i )
				*uv_it++ = float2( ai_mesh.mTextureCoords[0][i].x, ai_mesh.mTextureCoords[0][i].y );
		}
		if( ai_mesh.HasTangentsAndBitangents() )
		{
			auto tan_it = tan_att.begin();
			for( int i = 0; i < vertex_count; ++i )
				*tan_it++ = float3( ai_mesh.mTangents[i].x, ai_mesh.mTangents[i].y, ai_mesh.mTangents[i].z );
		}

		std::vector< unsigned short > indices;
		for( unsigned int f = 0; f < ai_mesh.mNumFaces; ++f )
			for( unsigned int i = 0; i < ai_mesh.mFaces[i].mNumIndices; ++i )
				indices.push_back( ai_mesh.mFaces[f].mIndices[i] );

		mesh.ib.set( new IndexBuffer(indices.size(), &indices[0] ) );

	}

	void load_meshes()
	{
		m_meshes.reserve( m_scene->mNumMeshes );
		for( int i = 0; i != m_scene->mNumMeshes; ++i )
		{
			m_meshes.push_back( Mesh() );
			load( m_meshes.back(), *m_scene->mMeshes[i] );
		}
	}
private:
	std::vector< Mesh > m_meshes;
	std::vector< Material::Ptr > m_materials;
	const struct aiScene* m_scene;
	ResourcePool m_pool;
	SceneNode::Ptr m_root;
};

/*
SceneNode::Ptr load_node( aiNode *node )
{
	if( node->mNumMeshes == 1 )
		return load_mesh( aiNode
}
*/

SceneNode::Ptr load_model( char const *filename )
{
	AILoader loader( filename );

	return loader.root();
}
