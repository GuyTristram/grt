#ifndef SCENENODE_H
#define SCENENODE_H

#include "common/shared.h"
#include "resource/mesh.h"
#include "resource/material.h"
#include "math/mat44.h"

#include <vector>

class SceneNode;
class SceneNodeMesh;
class SceneNodeLight;
class SceneNodeCamera;

class SceneNodeVisitor
{
public:
	virtual void visit( SceneNode & ) {}
	virtual void visit( SceneNodeMesh & ) {}
	virtual void visit( SceneNodeLight & ) {}
	virtual void visit( SceneNodeCamera & ) {}
};


class SceneNode : public Shared
{
public:
	typedef SharedPtr< SceneNode > Ptr;

	SceneNode();
	~SceneNode();

	void set_parent( SceneNode *parent );

	float44 const &world_from_model();

	virtual void accept( SceneNodeVisitor &visitor );

	typedef std::vector< Ptr >::iterator Iterator;

	Iterator begin();
	Iterator end();

protected:
	float44 m_world_from_model;
	void set_dirty();

private:
	virtual void modify_transform();
	bool m_dirty;	// If a node's m_dirty is true, m_dirty is true for all descendants
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


class SceneNodeMesh : public SceneNode
{
public:
	Mesh          mesh;
	Material::Ptr material;

	virtual void accept( SceneNodeVisitor &visitor );
};


void visit_scene( SceneNode &node, SceneNodeVisitor &visitor );

#endif // SCENENODE_H
