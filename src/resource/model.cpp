#include "resource/model.h"
#include "resource/resourcepool.h"
#include <algorithm>
#include <queue>
// assimp include files. These three are usually needed.
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class BoneFixer : public SceneNodeVisitor
{
public:
	BoneFixer( SceneNode & n ) : root( n ) {}

    virtual void visit( SceneMesh & n ) override
	{
		for( auto &bone : n.mesh.bones )
			n.bones.push_back( find_node( root, bone.node_name.c_str() ) );
	}

	SceneNode &root;
};

class AILoader
{
public:
	AILoader( char const *filename )
	{
		m_scene = aiImportFile( filename, aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_FlipUVs );
		load_meshes();
		load_materials();
		m_root.set( new SceneNode );
		load_node( *m_scene->mRootNode )->set_parent( m_root.get() );
		BoneFixer fixer( *m_root );
		visit_scene( *m_root, fixer );
		load_animations();
	}
	~AILoader()
	{
		aiReleaseImport( m_scene );
	}

	SceneNode::Ptr root() { return m_root; }
	Animation::Ptr animation() { return m_animation; }

	class AnimatePosition : public Animation
	{
	public:
		AnimatePosition( SceneNode::Ptr const &node, KeyData< float3 >::Ptr const &data )
			: m_node( node ), m_data( data ) {}
        virtual void update( double time ) override { if( m_node ) m_node->position( m_data->get( time ) ); }
	private:
		SceneNode::Ptr m_node;
		KeyData< float3 >::Ptr m_data;
	};

	class AnimateScale : public Animation
	{
	public:
		AnimateScale( SceneNode::Ptr const &node, KeyData< float3 >::Ptr const &data )
			: m_node( node ), m_data( data ) {}
        virtual void update( double time ) override { if( m_node ) m_node->scale( m_data->get( time ) ); }
	private:
		SceneNode::Ptr m_node;
		KeyData< float3 >::Ptr m_data;
	};

	class AnimateRotation : public Animation
	{
	public:
		AnimateRotation( SceneNode::Ptr const &node, KeyData< floatq >::Ptr const &data )
			: m_node( node ), m_data( data ) {}
        virtual void update( double time ) override { if( m_node ) m_node->rotation( m_data->get( time ) ); }
	private:
		SceneNode::Ptr m_node;
		KeyData< floatq >::Ptr m_data;
	};

	void load_animations()
	{
		m_animation.set( new AnimationGroup );
		for( int i = 0; i != m_scene->mNumAnimations; ++i )
		{
			aiAnimation *anim = m_scene->mAnimations[i];
			double time_mult = anim->mTicksPerSecond ? 1.0 / anim->mTicksPerSecond : 1.0;
			for( int j = 0; j != anim->mNumChannels; ++j )
			{
				aiNodeAnim *node_anim = anim->mChannels[j];
				SceneNode::Ptr node = find_node( *m_root, node_anim->mNodeName.C_Str() );
				if( node_anim->mNumPositionKeys )
				{
					KeyData< float3 >::Ptr data( new KeyData< float3 >() );
					data->reserve( node_anim->mNumPositionKeys );
					for( unsigned int i = 0; i < node_anim->mNumPositionKeys; ++i )
					{
						aiVector3D v = node_anim->mPositionKeys[i].mValue;
						data->push_back( node_anim->mPositionKeys[i].mTime * time_mult, float3( v.x, v.y, v.z ) );
					}
					m_animation->add( Animation::Ptr( new AnimatePosition( node, data) ) );
				}
				if( node_anim->mNumRotationKeys )
				{
					KeyData< floatq >::Ptr data( new KeyData< floatq >() );
					data->reserve( node_anim->mNumRotationKeys );
					for( unsigned int i = 0; i < node_anim->mNumRotationKeys; ++i )
					{
						aiQuaternion q = node_anim->mRotationKeys[i].mValue;
						data->push_back( node_anim->mRotationKeys[i].mTime * time_mult, floatq( q.x, q.y, q.z, q.w ) );
					}
					m_animation->add( Animation::Ptr( new AnimateRotation( node, data) ) );
				}
				if( node_anim->mNumScalingKeys )
				{
					KeyData< float3 >::Ptr data( new KeyData< float3 >() );
					data->reserve( node_anim->mNumScalingKeys );
					for( unsigned int i = 0; i < node_anim->mNumScalingKeys; ++i )
					{
						aiVector3D v = node_anim->mScalingKeys[i].mValue;
						data->push_back( node_anim->mScalingKeys[i].mTime * time_mult, float3( v.x, v.y, v.z ) );
					}
					m_animation->add( Animation::Ptr( new AnimateScale( node, data) ) );
				}
			}
		}
	}

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

		node->name = ai_node.mName.C_Str();

		aiMatrix4x4 m = ai_node.mTransformation;
		aiVector3D scaling, position;
		aiQuaternion rotation;
		m.Decompose( scaling, rotation, position );
		aiTransposeMatrix4(&m);
		//node->parent_from_local( float44( float4( m.a1, m.a2, m.a3, m.a4 ), float4( m.b1, m.b2, m.b3, m.b4 ),
		//                                  float4( m.c1, m.c2, m.c3, m.c4 ), float4( m.d1, m.d2, m.d3, m.d4 ) ) );
		node->position( float3( position.x, position.y, position.z ) );
		node->scale( float3( scaling.x, scaling.y, scaling.z ) );
		node->rotation( floatq( rotation.x, rotation.y, rotation.z, rotation.w ) );
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
		material.geom_program = m_pool.shader_program( "draw_normals.sp" );
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

	struct VertexWeight
	{
		VertexWeight() : weight( 0.0 ), index( 0 ) {}
		VertexWeight( float weight, unsigned int index ) : weight( weight ), index( index ) {}
		float weight;
		unsigned int index;
		bool operator<( VertexWeight const &other ) const { return weight < other.weight; } 
	};

	struct VertexWeights
	{
		VertexWeights() : weight_vector( 0, 0, 0, 0 ), index_vector( 0, 0, 0, 0 ) {}
		std::priority_queue< VertexWeight > weights;
		uchar4 weight_vector;
		uchar4 index_vector;
		void add_weight( float weight, unsigned int index )
		{
			weights.push( VertexWeight( weight, index ) );
			if( weights.size() > 4 ) weights.pop();
		}
		void normalize()
		{
			float total = 0.f;
			int count = weights.size();
			for( int i = 0; i != count; ++i )
			{
				total += weights.top().weight;
				weight_vector[i] = unsigned char( weights.top().weight * 255 );
				index_vector[i] = weights.top().index;
				weights.pop();
			}

			if( total > 0.f ) 
				for( int i = 0; i != count; ++i )
					weight_vector[i] /= total;
		}
	};

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
		VertexAttribute< uchar4 > index_att;
		VertexAttribute< uchar4 > weight_att;

		std::vector< VertexWeights > vertex_weights;

		if( ai_mesh.HasPositions() )
			pos_att = mesh.vb->add_attribute< float3 >( "a_position" );
		if( ai_mesh.HasTextureCoords( 0 ) )
			uv_att = mesh.vb->add_attribute< float2 >( "a_uv0" );
		if( ai_mesh.HasNormals() )
			norm_att = mesh.vb->add_attribute< float3 >( "a_normal" );
		if( ai_mesh.HasTangentsAndBitangents() )
			tan_att = mesh.vb->add_attribute< float3 >( "a_tangent" );
		if( ai_mesh.HasBones() )
		{
			index_att = mesh.vb->add_attribute< uchar4 >( "a_indices", false, false );
			weight_att = mesh.vb->add_attribute< uchar4 >( "a_weights" );

			vertex_weights.resize( ai_mesh.mNumVertices );
			mesh.bones.resize( ai_mesh.mNumBones );

			for( int ibone = 0; ibone != ai_mesh.mNumBones; ++ibone )
			{
				aiBone *bone = ai_mesh.mBones[ibone];
				for( int iweight = 0; iweight != bone->mNumWeights; ++iweight )
				{
					aiVertexWeight weight = bone->mWeights[iweight];
					vertex_weights[ weight.mVertexId ].add_weight( weight.mWeight, ibone );
				}
				mesh.bones[ibone].node_name = bone->mName.C_Str();
				aiMatrix4x4 m = bone->mOffsetMatrix;
				aiTransposeMatrix4(&m);
				mesh.bones[ibone].bone_from_model = 
					float44( float4( m.a1, m.a2, m.a3, m.a4 ), float4( m.b1, m.b2, m.b3, m.b4 ),
					         float4( m.c1, m.c2, m.c3, m.c4 ), float4( m.d1, m.d2, m.d3, m.d4 ) );
			}

			for( auto &weights : vertex_weights ) weights.normalize();
		}

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
		if( ai_mesh.HasBones() )
		{
			auto index_it = index_att.begin();
			auto weight_it = weight_att.begin();
			for( int i = 0; i < vertex_count; ++i )
			{
				*index_it++ = vertex_weights[i].index_vector;
				*weight_it++ = vertex_weights[i].weight_vector;
			}
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
	AnimationGroup::Ptr m_animation;
};

Model load_model( char const *filename )
{
	AILoader loader( filename );

	Model model;
	model.scene_node = loader.root();
	model.animation = loader.animation();

	return model;
}
