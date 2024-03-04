#pragma once

#include "Resource.h"

class Buffer : public Resource
{
public:
protected:
	Buffer(Device& device, const D3D12_RESOURCE_DESC& resDesc);
	Buffer(Device& device, Microsoft::WRL::ComPtr<ID3D12Resource> resource);

	//Buffer(const std::wstring& name = L"");
	//Buffer(const D3D12_RESOURCE_DESC& resDesc, size_t numElements, size_t elementSize, const std::wstring& name = L"");

	////Create the views for the buffer resource, used by command list when setting the buffer contents
	//virtual void CreateViews(size_t numElements, size_t elementSize) = 0;
};