#ifndef SCENENODE_H
#define SCENENODE_H

#include "common/shared.h"
#include "resource/mesh.h"
#include "resource/material.h"
#include "math/mat44.h"
#include "math/quat.h"
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
	virtual ~SceneNode();

	void set_parent( SceneNode *parent );

	float44 const &world_from_local() const;

	void parent_from_local( float44 const &m );
	float44 const &parent_from_local() const;

	void rotation( floatq const &q );
	floatq const &rotation() const;

	void scale( float3 const &s );
	float3 const &scale() const;

	void position( float3 const &p );
	float3 const &position() const;

    virtual void accept( SceneNodeVisitor &visitor );

	typedef std::vector< Ptr >::iterator Iterator;

	Iterator begin();
	Iterator end();

	std::string name;

private:
	mutable float44 m_parent_from_local;
	mutable float44 m_world_from_local;
	mutable floatq m_rotation;
	mutable float3 m_position;
	mutable float3 m_scale;
	void set_dirty();
	void decompose() const;

	mutable bool m_dirty;   // If a node's m_dirty is true, m_dirty is true for all descendants
	mutable bool m_local_dirty;   // Set if rotate, scale or position are used
	mutable bool m_local_set;   // true if parent_from_local set directly (rotate/scale/position out of date)

	std::vector< Ptr > m_children;
	SceneNode *m_parent;
};

class SceneMesh : public SceneNode
{
public:
	SceneMesh( ) : bone_transforms( "u_t_bone_transforms[0]" ) {}
	typedef SharedPtr< SceneMesh > Ptr;

	Mesh          mesh;
	Material::Ptr material;
	AABB          aabb;      // \todo This will need to be updated as soon as we're using a heirarchy
	float distance_from_eye2;

	std::vector< SceneNode::Ptr > bones;
	Uniform< std::vector< float44 > > bone_transforms;

	void update_bones();
	void set_bones( ShaderProgram &sp );

    virtual void accept( SceneNodeVisitor &visitor ) override;
};

class SceneLight : public SceneNode
{
public:
	typedef SharedPtr< SceneLight > Ptr;

	SceneLight() :
		position( 0.f, 0.f, 0.f, 1.f ),
		colour( 2.f, 2.f, 2.f, 1.f ),
		radius( 10.f ),
		dirty( true ),
		casts_shadows( true ),
		shadow_map_size( 256 ) {}

	float4 position;
	float4 colour;
	float radius;
	bool dirty;
	bool casts_shadows;
	int shadow_map_size;
	SharedPtr< TextureCube > shadow_map;

    virtual void accept( SceneNodeVisitor &visitor ) override;
};

void visit_scene( SceneNode &node, SceneNodeVisitor &visitor );
SceneNode::Ptr find_node( SceneNode &root, char const *name );

#endif // SCENENODE_H
