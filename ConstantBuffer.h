#pragma once

#include "Buffer.h"

#include <d3d12.h>
#include <wrl/client.h>

class ConstantBuffer : public Buffer
{
public:
	////Inherited from buffer
	//virtual void CreateViews(size_t numElements, size_t elementSize) override;

	size_t GetSizeInBytes() const
	{
		return m_SizeInBytes;
	}

	//D3D12_CPU_DESCRIPTOR_HANDLE GetConstantBufferView() const
	//{
	//	return m_ConstantBufferView.GetDescriptorHandle();
	//}

	////Get the SRV for a resource
	//virtual D3D12_CPU_DESCRIPTOR_HANDLE GetShaderResourceView(const D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc = nullptr) const override;

	////Get the UAV for a resource or subresource
	//virtual D3D12_CPU_DESCRIPTOR_HANDLE GetUnorderedAccessView(const D3D12_UNORDERED_ACCESS_VIEW_DESC* uavDesc = nullptr) const override;

protected:
	ConstantBuffer(Device& device, Microsoft::WRL::ComPtr<ID3D12Resource> resource);
	virtual ~ConstantBuffer();

private:
	size_t m_SizeInBytes;
	//DescriptorAllocation m_ConstantBufferView
};