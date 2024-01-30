#pragma once

#include "Visitor.h"

class CommandList;

class SceneVisitor : public Visitor
{
public:
	//Constructor for the scene visitor, @param commandList is the commandlist that is used to render meshes
	SceneVisitor(CommandList& commandList);

	//Placeholder methods
	virtual void Visit(Scene& scene) override {};
	virtual void Visit(SceneNode& sceneNode) override {};
	//Mesh must be rendered
	virtual void Visit(Mesh& mesh) override;

private:
	CommandList& m_CommandList;
};