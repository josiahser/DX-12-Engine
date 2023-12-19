#pragma once
/*
Descriptor Allocator is used to allocate descriptors from a CPU visible descriptor heap.
Useful for staging resource descriptors in CPU memory and later copied to a GPU visible descriptor heap
for use in a shader. Used when loading new resources (like texture), unloaded resources are returned back to the heap for reuse
Uses a *FREE LIST* list of available allocations, linearly searched and allocated using first-fit (can be improved with binary-search)
*/
#include "DescriptorAllocation.h"

#include "DirectX-Headers/include/directx/d3dx12.h"

#include <cstdint>
#include <mutex>
#include <memory>
#include <set>
#include <vector>

class DescriptorAllocatorPage;

class DescriptorAllocator
{
public:
	//Type of descriptors can be CBV_SRV_UAV, SAMPLER, RTV, or DSV
	DescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptorsPerHeap = 256);
	virtual ~DescriptorAllocator();

	/**
	* Allocate a number of contiguous descriptors from a CPU visible descriptor Heap
	* 
	* numDescriptors is the number of contiguous descriptors to allocate
	* cannot be more than the number of descriptors per descriptor heap
	*/
	DescriptorAllocation Allocate(uint32_t numDescriptors = 1);

	/**
	* When the frame has completed, the stale descriptors can be released
	*/
	void ReleaseStaleDescriptors(uint64_t frameNumber);

private:
	//Alias for the vector of ptrs to descriptor allocator pages
	using DescriptorHeapPool = std::vector< std::shared_ptr<DescriptorAllocatorPage> >;

	//Create a new heap with a specific number of descriptors
	std::shared_ptr<DescriptorAllocatorPage> CreateAllocatorPage();

	D3D12_DESCRIPTOR_HEAP_TYPE m_HeapType;
	uint32_t m_NumDescriptorsPerHeap;

	DescriptorHeapPool m_HeapPool;
	//Indices of available heaps in the heap pool
	std::set<size_t> m_AvaialableHeaps;

	std::mutex m_AllocationMutex;
};