#pragma once

#include "Buffer.h"

class IndexBuffer : public Buffer
{
public:
	//Inherited from Buffer
	//virtual void CreateViews(size_t numElements, size_t elementSize) override;

	size_t GetNumIndicies() const
	{
		return m_NumIndicies;
	}

	DXGI_FORMAT GetIndexFormat() const
	{
		return m_IndexFormat;
	}

	//Get the index buffer view for binding to the input assembler stage
	D3D12_INDEX_BUFFER_VIEW GetIndexBufferView() const
	{
		return m_IndexBufferView;
	}

	//Get the SRV for a resource
	//virtual D3D12_CPU_DESCRIPTOR_HANDLE GetShaderResourceView(const D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc = nullptr) const override;

	//Get the UAV for a resource or subresource
	//virtual D3D12_CPU_DESCRIPTOR_HANDLE GetUnorderedAccessView(const D3D12_UNORDERED_ACCESS_VIEW_DESC* uavDesc = nullptr) const override;

protected:
	IndexBuffer(Device& device, size_t numIndices, DXGI_FORMAT indexFormat);
	IndexBuffer(Device& device, Microsoft::WRL::ComPtr<ID3D12Resource> resource, size_t numIndices, DXGI_FORMAT indexFormat);
	virtual ~IndexBuffer() = default;

	void CreateIndexBufferView();

private:
	size_t m_NumIndicies;
	DXGI_FORMAT m_IndexFormat;
	D3D12_INDEX_BUFFER_VIEW m_IndexBufferView;
};