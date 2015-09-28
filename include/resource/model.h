#ifndef MODEL_H
#define MODEL_H

#include "resource/scenenode.h"
#include "resource/animation.h"

struct Model
{
	SceneNode::Ptr scene_node;
	Animation::Ptr animation;

};

Model load_model( char const *filename );

#endif // MODEL_H
