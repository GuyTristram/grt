#include "resource/scenenode.h"
#include <algorithm>

SceneNode::SceneNode() : m_dirty( false ), m_parent( 0 ) {}

SceneNode::~SceneNode()
{
	for( auto i = begin(); i != end(); ++i )
		(*i)->m_parent = 0;
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
		if( m_parent->m_dirty )
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

void SceneNode::modify_transform()
{
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
		(*i)->set_dirty();
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

void SceneNodeMesh::accept( SceneNodeVisitor &visitor )
{
	visitor.visit( *this );
}

void visit_scene( SceneNode &node, SceneNodeVisitor &visitor )
{
	node.accept( visitor );
	for( auto i = node.begin(); i != node.end(); ++i )
		visit_scene( **i, visitor );
}
