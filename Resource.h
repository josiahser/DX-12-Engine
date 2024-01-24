#pragma once

#include <d3d12.h>
#include <memory>
#include <wrl.h>

#include <string>

class Device;

class Resource
{
public:
	//Get the device used to create this resource
	Device& GetDevice() const
	{
		return m_Device;
	}

	////Constructors and operator overloads
	//explicit Resource(const std::wstring& name = L"");
	//explicit Resource(const D3D12_RESOURCE_DESC& resourceDesc, const D3D12_CLEAR_VALUE* clearValue = nullptr, const std::wstring& name = L"");
	//explicit Resource(Microsoft::WRL::ComPtr<ID3D12Resource> resource, const std::wstring& name = L"");
	//
	//Resource(const Resource& copy);
	//Resource(Resource&& copy) noexcept;

	//Resource& operator=(const Resource& other);
	//Resource& operator=(Resource&& other) noexcept;

	//virtual ~Resource();

	//Get access to the underlying D3D12 resource
	Microsoft::WRL::ComPtr<ID3D12Resource> GetD3D12Resource() const
	{
		return m_d3d12Resource;
	}

	D3D12_RESOURCE_DESC GetD3D12ResourceDesc() const
	{
		D3D12_RESOURCE_DESC resDesc = {};
		if (m_d3d12Resource)
		{
			resDesc = m_d3d12Resource->GetDesc();
		}

		return resDesc;
	}

	////Replace the D3D12 resource, only called by command list
	//virtual void SetD3D12Resource(Microsoft::WRL::ComPtr<ID3D12Resource> d3d12Resource, const D3D12_CLEAR_VALUE* clearValue = nullptr);

	////Get the SRV for a resource, @param srvDesc is the description of SRV to return, default is nullptr
	//virtual D3D12_CPU_DESCRIPTOR_HANDLE GetShaderResourceView(const D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc = nullptr) const = 0;

	////Get the UAV for a resource or subresource @param uavDesc is the description of the UAV to return
	//virtual D3D12_CPU_DESCRIPTOR_HANDLE GetUnorderedAccessView(const D3D12_UNORDERED_ACCESS_VIEW_DESC* uavDesc = nullptr) const = 0;

	//Set the name of the resource. Useful for debugging. Name of resource will persist if the underlying D3D12 resource is replaced with SetD3d12Resource
	void SetName(const std::wstring& name);
	const std::wstring& GetName() const
	{
		return m_ResourceName;
	}

	//Release the underlying resource, most useful for swap chain resizing
	//virtual void Reset();

	//Check if the resource format supports a specific feature
	bool CheckFormatSupport(D3D12_FORMAT_SUPPORT1 formatSupport) const;
	bool CheckFormatSupport(D3D12_FORMAT_SUPPORT2 formatSupport) const;

protected:
	//The underlying D3D12 resource
	//Resource creation should go through the device
	Resource(Device& device, const D3D12_RESOURCE_DESC& resourceDesc, const D3D12_CLEAR_VALUE* clearValue = nullptr);
	Resource(Device& device, Microsoft::WRL::ComPtr<ID3D12Resource> resource, const D3D12_CLEAR_VALUE* clearValue = nullptr);

	virtual ~Resource() = default;

	//Device used to create this resource
	Device& m_Device;

	Microsoft::WRL::ComPtr<ID3D12Resource> m_d3d12Resource;
	D3D12_FEATURE_DATA_FORMAT_SUPPORT m_FormatSupport;
	std::unique_ptr<D3D12_CLEAR_VALUE> m_d3d12ClearValue;
	std::wstring m_ResourceName;

private:
	//Check the format support and populate the m_FormatSupport structure
	void CheckFeatureSupport();
};