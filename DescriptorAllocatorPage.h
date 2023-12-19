#pragma once
/**
* A wrapper class for a ID3D12DescriptorHeap
* That implements a free list allocator
* to manage the descriptors in the heap
* (descriptors in the ID3D12Heap are 
* allocated space on a page, like a register)
*/
#include "DescriptorAllocation.h"

#include <d3d12.h>
#include "DirectX-Headers/include/directx/d3dx12.h"
#include <wrl.h>

#include <map>
#include <memory>
#include <mutex>
#include <queue>

class DescriptorAllocatorPage : public std::enable_shared_from_this<DescriptorAllocatorPage>
{
public:
	DescriptorAllocatorPage(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors);

	D3D12_DESCRIPTOR_HEAP_TYPE GetHeapType() const;

	//Check to see if the descriptor page has a contiguous block of descriptors large enough
	bool HasSpace(uint32_t numDescriptors) const;

	//Get number of available handles in the heap
	uint32_t NumFreeHandles() const;

	//Allocate a number of descriptors from this descriptor heap.
	//if it cannot be allocated, then a null descriptor is returned
	DescriptorAllocation Allocate(uint32_t numDescriptors);

	//Return a descriptor back to the heap
	//The frameNumber stale descriptors are not freed directly,
	//but put on a stale allocations queue, which are returned
	//to the heap using the ReleaseStaleAllocations method
	void Free(DescriptorAllocation&& descriptorHandle, uint64_t frameNumber);

	//Returns the stale descriptors back to the descriptor heap
	void ReleaseStaleDescriptors(uint64_t frameNumber);

protected:
	//Compute the offset of the descriptor handle from the start of the heap
	uint32_t ComputeOffset(D3D12_CPU_DESCRIPTOR_HANDLE handle);

	//Adds a new block to the free list
	void AddNewBlock(uint32_t offset, uint32_t numDescriptors);

	//Free a block of descriptors
	//This will also merge free blocks
	//in the free list to form larger blocks that can be reused
	void FreeBlock(uint32_t offset, uint32_t numDescriptors);

private:
	//The offset (in descriptors) within the descriptor heap
	//Aliases for readability
	using OffsetType = uint32_t;
	//The number of descriptors that are available
	using SizeType = uint32_t;

	struct FreeBlockInfo;
	//A map that lists the free blocks by the offset within the descriptor heap
	using FreeListByOffset = std::map<OffsetType, FreeBlockInfo>;

	//A map that lists the free blocks by size
	//Needs to be multimap since multiple blocks can have the same size
	using FreeListBySize = std::multimap<SizeType, FreeListByOffset::iterator>;

	struct FreeBlockInfo
	{
		FreeBlockInfo(SizeType size)
			: Size(size)
		{}

		SizeType Size;
		FreeListBySize::iterator FreeListBySizeIt;
	};

	struct StaleDescriptorInfo
	{
		StaleDescriptorInfo(OffsetType offset, SizeType size, uint64_t frame)
			: Offset(offset)
			, Size(size)
			, FrameNumber(frame)
		{}

		//The offset within the descriptor heap
		OffsetType Offset;
		//The number of descriptors
		SizeType Size;
		//The frame number that the descriptor was freed
		uint64_t FrameNumber;
	};

	//Stale descriptors are queued for release until the frame they were freed in has completed
	using StaleDescriptorQueue = std::queue<StaleDescriptorInfo>;

	//Necessary data structures to track the state of the free list
	FreeListByOffset m_FreeListByOffset;
	FreeListBySize m_FreeListBySize;
	StaleDescriptorQueue m_StaleDescriptors;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_d3d12DescriptorHeap;
	D3D12_DESCRIPTOR_HEAP_TYPE m_HeapType;
	CD3DX12_CPU_DESCRIPTOR_HANDLE m_BaseDescriptor;
	uint32_t m_DescriptorHandleIncrementSize;
	uint32_t m_NumDescsriptorsInHeap;
	uint32_t m_NumFreeHandles;

	std::mutex m_AllocationMutex;

};