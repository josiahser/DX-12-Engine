#include "framework.h"

#include "IndexBuffer.h"

#include <cassert>

IndexBuffer::IndexBuffer(Device& device, size_t numIndices, DXGI_FORMAT indexFormat)
	: Buffer(device, CD3DX12_RESOURCE_DESC::Buffer(numIndices * (indexFormat == DXGI_FORMAT_R16_UINT ? 2 : 4)))
	, m_NumIndicies(numIndices)
	, m_IndexFormat(indexFormat)
	, m_IndexBufferView {}
{
	assert(indexFormat == DXGI_FORMAT_R16_UINT || indexFormat == DXGI_FORMAT_R32_UINT);
	CreateIndexBufferView();
}

IndexBuffer::IndexBuffer(Device& device, Microsoft::WRL::ComPtr<ID3D12Resource> resource, size_t numIndices, DXGI_FORMAT indexFormat)
	: Buffer(device, resource)
	, m_NumIndicies(numIndices)
	, m_IndexFormat(indexFormat)
	, m_IndexBufferView{}
{
	assert(indexFormat == DXGI_FORMAT_R16_UINT || indexFormat == DXGI_FORMAT_R32_UINT);
	CreateIndexBufferView();
}

void IndexBuffer::CreateIndexBufferView()
{
	UINT bufferSize = m_NumIndicies * (m_IndexFormat == DXGI_FORMAT_R16_UINT ? 2 : 4);

	m_IndexBufferView.BufferLocation = m_d3d12Resource->GetGPUVirtualAddress();
	m_IndexBufferView.SizeInBytes = bufferSize;
	m_IndexBufferView.Format = m_IndexFormat;
}
//
//D3D12_CPU_DESCRIPTOR_HANDLE IndexBuffer::GetShaderResourceView(const D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc) const
//{
//	throw std::exception("IndexBuffer::GetShaderResourceView should not be called");
//}
//
//D3D12_CPU_DESCRIPTOR_HANDLE IndexBuffer::GetUnorderedAccessView(const D3D12_UNORDERED_ACCESS_VIEW_DESC* uavDesc) const
//{
//	throw std::exception("IndexBuffer::GetUAV should not be called");
//}