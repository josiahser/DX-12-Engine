#pragma once

#include "Buffer.h"
#include "ByteAddressBuffer.h"

class Device;

class StructuredBuffer : public Buffer
{
public:
	//Get the # of elements contained in this buffer
	virtual size_t GetNumElements() const
	{
		return m_NumElements;
	}

	//Get the size in bytes of each element in this buffer
	virtual size_t GetElementSize() const
	{
		return m_ElementSize;
	}

	////Create the views for the buffer resource
	////Used by the command list when setting the buffer contents
	//virtual void CreateViews(size_t numElements, size_t elementSize) override;

	////Get the SRV for a resource
	//virtual D3D12_CPU_DESCRIPTOR_HANDLE GetShaderResourceView(const D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc = nullptr) const
	//{
	//	return m_SRV.GetDescriptorHandle();
	//}

	////Get the UAV for a resource or subresource
	//virtual D3D12_CPU_DESCRIPTOR_HANDLE GetUnorderedAccessView(const D3D12_UNORDERED_ACCESS_VIEW_DESC* uavDesc = nullptr) const override
	//{
	//	//Buffers don't have subresources
	//	return m_UAV.GetDescriptorHandle();
	//}

	std::shared_ptr<ByteAddressBuffer> GetCounterBuffer() const
	{
		return m_CounterBuffer;
	}

protected:

	StructuredBuffer(Device& device, size_t numElements, size_t elementSize);
	StructuredBuffer(Device& device, Microsoft::WRL::ComPtr<ID3D12Resource> resource, size_t numElements, size_t elementSize);

	virtual ~StructuredBuffer() = default;

private:
	size_t m_NumElements;
	size_t m_ElementSize;

	//DescriptorAllocation m_SRV;
	//DescriptorAllocation m_UAV;

	//A buffer to store the internal counter for the structured buffer
	std::shared_ptr<ByteAddressBuffer> m_CounterBuffer;
};