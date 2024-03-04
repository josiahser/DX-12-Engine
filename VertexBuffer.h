#pragma once

#include "Buffer.h"

class VertexBuffer : public Buffer
{
public:
	//Get the vertex buffer view for binding to the input assembler stage
	D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView() const
	{
		return m_VertexBufferView;
	}

	size_t GetNumVertices() const
	{
		return m_NumVertices;
	}

	size_t GetVertexStride() const
	{
		return m_VertexStride;
	}

	////Get the SRV for a resource
	//virtual D3D12_CPU_DESCRIPTOR_HANDLE GetShaderResourceView(const D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc = nullptr) const override;

	////Get the UAV for a resource or subresource
	//virtual D3D12_CPU_DESCRIPTOR_HANDLE GetUnorderedAccessView(const D3D12_UNORDERED_ACCESS_VIEW_DESC* uavDesc = nullptr) const override;

protected:
	VertexBuffer(Device& device, size_t numVertices, size_t vertexStride);
	VertexBuffer(Device& device, Microsoft::WRL::ComPtr<ID3D12Resource> resource, size_t numVertices, size_t vertexStride);
	virtual ~VertexBuffer();

	void CreateVertexBufferView();

private:
	size_t m_NumVertices;
	size_t m_VertexStride;

	D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;
};