#pragma once

#include <DirectXCollision.h>
#include <DirectXMath.h>

#include <d3d12.h>

#include <wrl.h>

#include <map>
#include <memory>
#include <vector>

class CommandList;
class IndexBuffer;
class Material;
class VertexBuffer;
//class Visitor;
//Vertex struct holding position, normal vector, and texture mapping info
//struct VertexPositionNormalTexture
//{
//	VertexPositionNormalTexture()
//	{}
//
//	VertexPositionNormalTexture(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& normal, const DirectX::XMFLOAT2& textureCoordinate)
//		:position(position),
//		normal(normal),
//		textureCoordinate(textureCoordinate)
//	{}
//
//	VertexPositionNormalTexture(DirectX::FXMVECTOR position, DirectX::FXMVECTOR normal, DirectX::FXMVECTOR textureCoordinate)
//	{
//		XMStoreFloat3(&this->position, position);
//		XMStoreFloat3(&this->normal, normal);
//		XMStoreFloat2(&this->textureCoordinate, textureCoordinate);
//	}
//
//	DirectX::XMFLOAT3 position;
//	DirectX::XMFLOAT3 normal;
//	DirectX::XMFLOAT2 textureCoordinate;
//
//	static const int InputElementCount = 3;
//	static const D3D12_INPUT_ELEMENT_DESC InputElements[InputElementCount];
//};

//using VertexCollection = std::vector<VertexPositionNormalTexture>;
//using IndexCollection = std::vector<uint16_t>;

class Mesh
{
public:
	using BufferMap = std::map<uint32_t, std::shared_ptr<VertexBuffer>>;

	Mesh();
	~Mesh() = default;

	void SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY primitiveTopology);
	D3D12_PRIMITIVE_TOPOLOGY GetPrimitiveTopology() const;

	void SetVertexBuffer(uint32_t slotID, const std::shared_ptr<VertexBuffer>& vertexBuffer);
	std::shared_ptr<VertexBuffer> GetVertexBuffer(uint32_t slotID) const;
	const BufferMap& GetVertexBuffers() const
	{
		return m_VertexBuffers;
	}

	void SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer);
	std::shared_ptr<IndexBuffer> GetIndexBuffer();

	//Get the # of indices in the index buffer, if no index buffer is bound to the mesh, returns 0
	size_t GetIndexCount() const;

	//Get the # of verticies in the mesh, if no vertex buffer is bound to the mesh, returns 0
	size_t GetVertexCount() const;

	void SetMaterial(std::shared_ptr<Material> material);
	std::shared_ptr<Material> GetMaterial() const;

	//Set the AABB bounding volume for the geometry in this mesh
	void SetAABB(const DirectX::BoundingBox& aabb);
	const DirectX::BoundingBox& GetAABB() const;

	//Draw a mesh to a command list.
	//@param commandList The command list to draw to
	//@param instanceCount # of instances to draw
	//@param startInstance the offset added to the instance ID when reading from the instance buffers
	void Draw(CommandList& commandList, uint32_t instanceCount = 1, uint32_t startInstance = 0);

	//static std::unique_ptr<Mesh> CreateCube(CommandList& commandList, float size = 1, bool rhcoords = false);
	//static std::unique_ptr<Mesh> CreateSphere(CommandList& commandList, float diameter = 1, size_t tessellation = 16, bool rhcoords = false);
	//static std::unique_ptr<Mesh> CreateCone(CommandList& commandList, float diameter = 1, float height = 1, size_t tessellation = 32, bool rhcoords = false);
	//static std::unique_ptr<Mesh> CreateTorus(CommandList& commandList, float diameter = 1, float thickness = 0.333f, size_t tessellation = 32, bool rhcoords = false);
	//static std::unique_ptr<Mesh> CreatePlane(CommandList& commandList, float width = 1, float height = 1, bool rhcoords = false);

	//Accept a visitor
	//void Accept(Visitor& visitor);
private:
	//void Initialize(CommandList& commandList, VertexCollection& vertices, IndexCollection& indices, bool rhcoords);

	BufferMap m_VertexBuffers;
	std::shared_ptr<IndexBuffer> m_IndexBuffer;
	std::shared_ptr<Material> m_Material;
	D3D12_PRIMITIVE_TOPOLOGY m_PrimitiveTopology;
	DirectX::BoundingBox m_AABB;
};