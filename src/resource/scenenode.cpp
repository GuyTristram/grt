#include "resource/scenenode.h"
#include "resource/resourcepool.h"
#include <algorithm>

SceneNode::SceneNode() : 
	m_scale( 1.f, 1.f, 1.f ),
	m_position( 0.f, 0.f, 0.f ),
	m_dirty( false ), m_local_dirty( false ), m_local_set( false ),
	m_parent( 0 ) {}

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

float44 const &SceneNode::world_from_local() const
{
	if( m_dirty )
	{
		if( m_parent )
			m_world_from_local = m_parent->world_from_local() * parent_from_local();
		else
			m_world_from_local = parent_from_local();
		m_dirty = false;
	}

	return m_world_from_local;
}

void SceneNode::accept( SceneNodeVisitor &visitor )
{
	visitor.visit( *this );
}

void SceneNode::parent_from_local( float44 const &m )
{
	set_dirty();
	m_local_dirty = false;
	m_local_set = true;
	m_parent_from_local = m;
}

float44 const &SceneNode::parent_from_local() const
{
	if( m_local_dirty )
	{
		float44 r;
		to_matrix( m_rotation, r );
		//m_parent_from_local = r * ::scale( float4( m_scale, 1.0f ) );
		for( int i = 0; i != 3; ++i )
			m_parent_from_local[i].xyz() *= m_scale[i];
		m_parent_from_local = r * ::scale( float4( m_scale, 1.0f ) );
		m_parent_from_local.t.xyz() = m_position;
		m_local_dirty = false;
	}
	return m_parent_from_local;
}


void SceneNode::rotation( floatq const &q )
{
	decompose();
	if( q != m_rotation )
	{
		set_dirty();
		m_local_dirty = true;
		m_rotation = q;
	}
}

floatq const &SceneNode::rotation() const
{
	decompose();
	return m_rotation;
}

void SceneNode::scale( float3 const &s )
{
	decompose();
	if( s != m_scale )
	{
		set_dirty();
		m_local_dirty = true;
		m_scale = s;
	}
}

float3 const &SceneNode::scale() const
{
	decompose();
	return m_scale;
}

void SceneNode::position( float3 const &p )
{
	decompose();
	if( p != m_position )
	{
		set_dirty();
		m_local_dirty = true;
		m_position = p;
	}
}

float3 const &SceneNode::position() const
{
	decompose();
	return m_position;
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

void SceneNode::decompose() const
{
	if( m_local_set )
	{
		m_position = m_parent_from_local.t.xyz();
		m_scale = float3( length( m_parent_from_local.i ),
		                  length( m_parent_from_local.j ),
						  length( m_parent_from_local.k ) );
		float33 rot( m_parent_from_local.i.xyz() / m_scale.x,
			         m_parent_from_local.j.xyz() / m_scale.y,
					 m_parent_from_local.k.xyz() / m_scale.z );
		from_matrix( m_rotation, rot );
		m_local_set = false;
	}
}

void SceneMesh::update_bones()
{
	if( bones.size() )
	{
		bone_transforms.data.resize( bones.size() );
		for( int i = 0; i != bones.size(); ++i )
			bone_transforms.data[i] = bones[i]->world_from_local( ) * mesh.bones[i].bone_from_model;
	}
}

void SceneMesh::set_bones( ShaderProgram &sp )
{
	static Uniform< bool > t( "u_skinned", true );
	static Uniform< bool > f( "u_skinned", false );
	if( bones.size( ) )
	{
		sp.set( t );
		sp.set( bone_transforms );
	}
	else
	{
		sp.set( f );
	}
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


class NodeFinder : public SceneNodeVisitor
{
public:
	NodeFinder( char const *name ) : name( name ) {}

    virtual void visit( SceneNode & n ) override { if( n.name == name ) node = SceneNode::Ptr( &n ); }

	SceneNode::Ptr node;
	std::string name;
};

SceneNode::Ptr find_node( SceneNode &root, char const *name )
{
	NodeFinder finder( name );
	visit_scene( root, finder );
	return finder.node;
}

