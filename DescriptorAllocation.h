#pragma once

//#pragma comment(lib, "d3d12")

#include <d3d12.h>

#include <cstdint>
#include <memory>

class DescriptorAllocatorPage;

class DescriptorAllocation
{
public:
	//Creates a NULL descriptor
	DescriptorAllocation();

	//Used by the Page::Allocate method to construct a valid descriptor allocation
	DescriptorAllocation(D3D12_CPU_DESCRIPTOR_HANDLE descriptor, uint32_t numHandles, uint32_t descriptorSize, std::shared_ptr<DescriptorAllocatorPage> page);

	//Destructor will automatically free the allocation
	~DescriptorAllocation();

	//Copies are not allowed
	DescriptorAllocation(const DescriptorAllocation&) = delete;
	DescriptorAllocation& operator=(const DescriptorAllocation&) = delete;

	//Move is allowed
	DescriptorAllocation (DescriptorAllocation&& allocation) noexcept;
	DescriptorAllocation& operator=(DescriptorAllocation&& other) noexcept;

	//Check if this is a valid descriptor
	bool IsNull() const;
	bool IsValid() const
	{
		return !IsNull();
	}

	//Get a descriptor at a particular offset in the allocation
	D3D12_CPU_DESCRIPTOR_HANDLE GetDescriptorHandle(uint32_t offset = 0) const;
	
	//Get number of (consecutive) handles for this allocation
	uint32_t GetNumHandles() const;

	//Get the heap that his allocation came from (for internal use only)
	std::shared_ptr<DescriptorAllocatorPage> GetDescriptorAllocatorPage() const;

private:
	//Free the descriptor back to the heap it came from
	void Free();

	//The base descriptor
	D3D12_CPU_DESCRIPTOR_HANDLE m_Descriptor;

	//The number of descriptors in this allocation
	uint32_t m_NumHandles;

	//The offset to the next descriptor
	uint32_t m_DescriptorSize;

	//A pointer back to the original page where this allocation came from
	std::shared_ptr<DescriptorAllocatorPage> m_Page;

};