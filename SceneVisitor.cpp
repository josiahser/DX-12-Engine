#include "SceneVisitor.h"

#include "CommandList.h"
#include "IndexBuffer.h"
#include "Mesh.h"

SceneVisitor::SceneVisitor(CommandList& commandList)
	: m_CommandList(commandList)
{}

void SceneVisitor::Visit(Mesh& mesh)
{
	mesh.Draw(m_CommandList);
}