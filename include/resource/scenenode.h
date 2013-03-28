#ifndef SCENENODE_H
#define SCENENODE_H

#include "common/shared.h"
#include "resource/mesh.h"
#include "resource/material.h"
#include "math/mat44.h"
#include "math/frustum.h"

#include <vector>

class TextureCube;

class SceneNode;
class SceneMesh;
class SceneLight;
class SceneCamera;

class SceneNodeVisitor
{
public:
	virtual void visit( SceneNode & ) {}
	virtual void visit( SceneMesh & ) {}
	virtual void visit( SceneLight & ) {}
	virtual void visit( SceneCamera & ) {}
};


class SceneNode : public Shared
{
public:
	typedef SharedPtr< SceneNode > Ptr;

	SceneNode();
	~SceneNode();

	void set_parent( SceneNode *parent );

	void parent_from_local( float44 const &m );
	float44 const &world_from_model();

	virtual void accept( SceneNodeVisitor &visitor );

	typedef std::vector< Ptr >::iterator Iterator;

	Iterator begin();
	Iterator end();

protected:
	float44 m_parent_from_local;
	float44 m_world_from_model;
	void set_dirty();

private:
	virtual void modify_transform();
	bool m_dirty;   // If a node's m_dirty is true, m_dirty is true for all descendants
	std::vector< Ptr > m_children;
	SceneNode *m_parent;
};

class SceneNodePosition : public SceneNode
{
public:
	void set( float4 const &position );

	virtual void modify_transform();

private:
	float4 m_position;
};


class SceneMesh : public SceneNode
{
public:
	typedef SharedPtr< SceneMesh > Ptr;

	Mesh          mesh;
	Material::Ptr material;
	AABB          aabb;      // \todo This will need to be updated as soon as we're using a heirarchy
	float distance_from_eye2;

	virtual void accept( SceneNodeVisitor &visitor );
};

class SceneLight : public SceneNode
{
public:
	typedef SharedPtr< SceneLight > Ptr;

	SceneLight() :
		position( 0.f, 0.f, 0.f, 1.f ),
		colour( 10.f, 10.f, 10.f, 10.f ),
		radius( 20.f ),
		dirty( true ),
		casts_shadows( true ) {}

	float4 position;
	float4 colour;
	float radius;
	bool dirty;
	bool casts_shadows;
	SharedPtr< TextureCube > shadow_map;

	virtual void accept( SceneNodeVisitor &visitor );
};


void visit_scene( SceneNode &node, SceneNodeVisitor &visitor );

SceneNode::Ptr load_model( char const *filename );

#endif // SCENENODE_H
