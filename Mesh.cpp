#include "framework.h"

#include "CommandList.h"
#include "IndexBuffer.h"
#include "Mesh.h"
#include "VertexBuffer.h"
#include "Visitor.h"

//const D3D12_INPUT_ELEMENT_DESC VertexPositionNormalTexture::InputElements[] =
//{
//	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
//	{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
//	{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
//};

Mesh::Mesh()
	: m_PrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST)
{}

void Mesh::SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY primitiveTopology)
{
	m_PrimitiveTopology = primitiveTopology;
}

D3D12_PRIMITIVE_TOPOLOGY Mesh::GetPrimitiveTopology() const
{
	return m_PrimitiveTopology;
}

void Mesh::SetVertexBuffer(uint32_t slotID, const std::shared_ptr<VertexBuffer>& vertexBuffer)
{
	m_VertexBuffers[slotID] = vertexBuffer;
}

std::shared_ptr<VertexBuffer> Mesh::GetVertexBuffer(uint32_t slotID) const
{
	auto iter = m_VertexBuffers.find(slotID);
	auto vertexBuffer = iter != m_VertexBuffers.end() ? iter->second : nullptr;

	return vertexBuffer;
}

void Mesh::SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer)
{
	m_IndexBuffer = indexBuffer;
}

std::shared_ptr<IndexBuffer> Mesh::GetIndexBuffer()
{
	return m_IndexBuffer;
}

size_t Mesh::GetIndexCount() const
{
	size_t indexCount = 0;
	if (m_IndexBuffer)
	{
		indexCount = m_IndexBuffer->GetNumIndicies();
	}

	return indexCount;
}

size_t Mesh::GetVertexCount() const
{
	size_t vertexCount = 0;

	//To count the number of vertices in the mesh, just take the number of vertices in the first vertex buffer
	BufferMap::const_iterator iter = m_VertexBuffers.cbegin();
	if (iter != m_VertexBuffers.cend())
	{
		vertexCount = iter->second->GetNumVertices();
	}

	return vertexCount;
}

void Mesh::SetMaterial(std::shared_ptr<Material> material)
{
	m_Material = material;
}

std::shared_ptr<Material> Mesh::GetMaterial() const
{
	return m_Material;
}

void Mesh::Draw(CommandList& commandList, uint32_t instanceCount, uint32_t startInstance)
{
	commandList.SetPrimitiveTopology(GetPrimitiveTopology());

	for (auto vertexBuffer : m_VertexBuffers)
	{
		commandList.SetVertexBuffer(vertexBuffer.first, vertexBuffer.second);
	}

	auto indexCount = GetIndexCount();
	auto vertexCount = GetVertexCount();

	if (indexCount > 0)
	{
		commandList.SetIndexBuffer(m_IndexBuffer);
		commandList.DrawIndexed(indexCount, instanceCount, 0u, 0u, startInstance);
	}
	else if (vertexCount > 0)
	{
		commandList.Draw(vertexCount, instanceCount, 0u, startInstance);
	}
}

void Mesh::Accept(Visitor& visitor)
{
	visitor.Visit(*this);
}

void Mesh::SetAABB(const DirectX::BoundingBox& aabb)
{
	m_AABB = aabb;
}

const DirectX::BoundingBox& Mesh::GetAABB() const
{
	return m_AABB;
}