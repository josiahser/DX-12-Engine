#include "framework.h"

#include "ConstantBuffer.h"
#include "Application.h"
#include "Device.h"
#include "DirectX-Headers/include/directx/d3dx12.h"

ConstantBuffer::ConstantBuffer(Device& device, Microsoft::WRL::ComPtr<ID3D12Resource> resource)
	: Buffer(device, resource)
{
	m_SizeInBytes = GetD3D12ResourceDesc().Width;
}

ConstantBuffer::~ConstantBuffer()
{}

//ConstantBuffer::ConstantBuffer(const std::wstring& name)
//	: Buffer(name)
//	, m_SizeInBytes(0)
//{
//	m_ConstantBufferView = Application::Get().AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
//}
//
//ConstantBuffer::~ConstantBuffer()
//{}
//
//void ConstantBuffer::CreateViews(size_t numElements, size_t elementSize)
//{
//	m_SizeInBytes = numElements * elementSize;
//
//	D3D12_CONSTANT_BUFFER_VIEW_DESC d3d12ConstantBufferViewDesc;
//	d3d12ConstantBufferViewDesc.BufferLocation = m_d3d12Resource->GetGPUVirtualAddress();
//	d3d12ConstantBufferViewDesc.SizeInBytes = static_cast<UINT>(Math::AlignUp(m_SizeInBytes, 16));
//
//	auto device = Application::Get().GetDevice();
//
//	device->CreateConstantBufferView(&d3d12ConstantBufferViewDesc, m_ConstantBufferView.GetDescriptorHandle());
//}
//
//D3D12_CPU_DESCRIPTOR_HANDLE ConstantBuffer::GetShaderResourceView(const D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc) const
//{
//	throw std::exception("ConstantBuffer::GetShaderResourceView shouldn't be called");
//}
//
//D3D12_CPU_DESCRIPTOR_HANDLE ConstantBuffer::GetUnorderedAccessView(const D3D12_UNORDERED_ACCESS_VIEW_DESC* uavDesc) const
//{
//	throw std::exception("ConstantBuffer::GetUAV shouldn't be called");
//}