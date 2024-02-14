#pragma once

#include "Visitor.h"

class Camera;
class EffectPSO;

class CommandList;

class SceneVisitor : public Visitor
{
public:
	/**
	 * Constructor for the SceneVisitor.
	 * @param commandList The CommandList that is used to render the meshes in the scene.
	 * @param camera The camera that is used to render the scene. This is required for setting up the MVP matrix.
	 * @param pso The Pipeline state object to use for rendering the geometry in the scene.
	 * @param transparent Whether to draw transparent geometry during this pass.
	 */
	SceneVisitor(CommandList& commandList, const Camera& camera, EffectPSO& pso, bool transparent);

	//Placeholder methods
	virtual void Visit(Scene& scene) override;
	//We need to set the MVP matrix of the scene Node
	virtual void Visit(SceneNode& sceneNode) override;
	//Mesh must be rendered
	virtual void Visit(Mesh& mesh) override;

private:
	CommandList& m_CommandList;
	const Camera& m_Camera;
	EffectPSO& m_LightingPSO;
	bool m_TransparentPass;
};