#pragma once

#include "d3dx12.h"

#include <wrl.h>

#include <cstdint>
#include <memory>
#include <queue>
//#include <functional>

class Device;
class CommandList;
class RootSignature;

class DynamicDescriptorHeap
{
public:
	DynamicDescriptorHeap(Device& device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, uint32_t numDescriptorsPerHeap = 1024);

	virtual ~DynamicDescriptorHeap();

	//Stages a contiguous range of CPU visible descriptors
	//Descriptors are not copied to the GPU visible descriptor heap until
	//The CommitStagedDescriptors function is called
	void StageDescriptors(uint32_t rootParameterIndex, uint32_t offset, uint32_t numDescriptors, const D3D12_CPU_DESCRIPTOR_HANDLE srcDescriptors);

	//Stage an inline CBV descriptor
	void StageInLineCBV(uint32_t rootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation);

	//Stage an inline SRV descriptor
	void StageInLineSRV(uint32_t rootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation);

	//Stage an inline UAV descriptor
	void StageInLineUAV(uint32_t rootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation);

	/*
	* Copy all of the staged descriptors to the GPU visible descriptor heap and
	* Bind the descriptor heap and the descriptor tables to the command list
	* The passed-in function object is used to set the GPU visible descriptors
	* on the command list. Two possible functions are:
	* * Before a Draw : ID3D12GraphicsCommandList::SetGraphicsRootDescriptorTable
	* * Before a Dispatch : ID3D12GraphicsCommandList::SetComputeRootDescriptorTable
	* 
	* Since the DynamicDescriptorHeap can't know which function will be used, it must
	* be passed as an argument to the function
	*/
	//void CommitStagedDescriptors(CommandList& commandList, std::function<void(ID3D12GraphicsCommandList*, UINT, D3D12_GPU_DESCRIPTOR_HANDLE)> setFunc);
	void CommitStagedDescriptorsForDraw(CommandList& commandList);
	void CommitStagedDescriptorsForDispatch(CommandList& commandList);

	/*
	*Copies a single CPU visible descriptor to a GPU visible descriptor heap
	* Useful for ID3D12GraphicsCommandList::ClearUnorderedAccessViewFloat and Uint
	* methods, which require both a CPU and GPU visible descriptors for a UAV resource
	* commandList is the command list required in case the GPU visible descriptor heap
	* needs to be updated on the command list
	* cpuDescriptor is the CPU desscriptor to copy into a GPU visible descriptor heap
	* Returns the GPU visible descriptor
	*/
	D3D12_GPU_DESCRIPTOR_HANDLE CopyDescriptor(CommandList& commandList, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptor);

	//Parse the root signature to determine which root parameters contain
	//descriptor tables and determine the number of descriptors needed for each table
	void ParseRootSignature(const std::shared_ptr<RootSignature>& rootSignature);

	//Reset used descriptors this should only be done if any descriptors that are being
	//referenced by a command list has finished executing on the command queue
	void Reset();
private:
	//Request a descriptor heap if one is available
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> RequestDescriptorHeap();

	//Create a new descriptor heap if no descriptor heap is available
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap();

	//Compute the # of stale descriptors that need to be copied to GPU visible descriptor heap
	uint32_t ComputeStaleDescriptorCount() const;

	//Copy all the staged descriptors to the GPU visible descriptor heap and bind the descriptor heap and the tables to the command list
	//The passed in function object is used to set the GPU visible descriptors on the command list
	//Before a draw, SetGraphicsRootDescriptorTable
	//Before a dispatch, SetComputeRootDescriptorTable
	void CommitDescriptorTables(CommandList& commandList, std::function<void(ID3D12GraphicsCommandList*, UINT, D3D12_GPU_DESCRIPTOR_HANDLE)> setFunc);
	void CommitInlineDescriptors(CommandList& commandList, const D3D12_GPU_VIRTUAL_ADDRESS* bufferLocations, uint32_t& bitMask,
		std::function<void(ID3D12GraphicsCommandList*, UINT, D3D12_GPU_VIRTUAL_ADDRESS)> setFunc);

	/// The max # of descriptor tables per root signature
	/// A 32-bit mask is used to keep track of the root parameter indices
	/// that are descriptor tables
	static const uint32_t MaxDescriptorTables = 32;

	//A structure that represents a descriptor table entry in the root signature
	struct DescriptorTableCache
	{
		DescriptorTableCache()
			: NumDescriptors(0)
			, BaseDescriptor(nullptr)
		{}

		//Reset the table cache
		void Reset()
		{
			NumDescriptors = 0;
			BaseDescriptor = nullptr;
		}

		//The number of descriptors in this descriptor table
		uint32_t NumDescriptors;
		//Pointer to the descriptor in the descriptor handle cache
		D3D12_CPU_DESCRIPTOR_HANDLE* BaseDescriptor;
	};

	Device& m_Device;

	/// Describes the type of descriptors that will be staged
	/// Using this dynamic descriptor heap
	/// (either CBV_SRV_UAV or SAMPLER)
	/// Also determines the type of GPU visible descriptor heap to create
	D3D12_DESCRIPTOR_HEAP_TYPE m_DescriptorHeapType;

	//The number of descriptors to allocate in new GPU visible descriptors heaps
	uint32_t m_NumDescriptorsPerHeap;

	//The increment size of a descriptor
	uint32_t m_DescriptorHandleIncrementSize;

	//Descriptor handle cache
	std::unique_ptr<D3D12_CPU_DESCRIPTOR_HANDLE[]> m_DescriptorHandleCache;

	//Descriptor handle cache per descriptor table
	DescriptorTableCache m_DescriptorTableCache[MaxDescriptorTables];

	//Inline CBV
	D3D12_GPU_VIRTUAL_ADDRESS m_InlineCBV[MaxDescriptorTables];

	//Inline SRV
	D3D12_GPU_VIRTUAL_ADDRESS m_InlineSRV[MaxDescriptorTables];

	//Inline UAV
	D3D12_GPU_VIRTUAL_ADDRESS m_InlineUAV[MaxDescriptorTables];

	// Each bit in the bit mask represents the index
	// in the root signature that contains a descriptor table
	uint32_t m_DescriptorTableBitMask;

	//Each bit set in the bit mask represents a descriptor table
	//in the root signature that has changed since the descriptors were copied
	uint32_t m_StaleDescriptorTableBitMask;
	uint32_t m_StaleCBVBitMask;
	uint32_t m_StaleSRVBitMask;
	uint32_t m_StaleUAVBitMask;

	//Alias for a ID3D12Descriptor Heap queue (GPU visible)
	using DescriptorHeapPool = std::queue< Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> >;

	DescriptorHeapPool m_DescriptorHeapPool; //All the descriptor heaps created by the dynamic class
	DescriptorHeapPool m_AvailableDescriptorHeaps; //Only the heaps that still contain descriptors

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_CurrentDescriptorHeap; //Current heap bound to the command list
	CD3DX12_GPU_DESCRIPTOR_HANDLE m_CurrentGPUDescriptorHandle; //handles within the heap bound to the command list
	CD3DX12_CPU_DESCRIPTOR_HANDLE m_CurrentCPUDescriptorHandle;

	uint32_t m_NumFreeHandles; //# of handles that are still available in the currently bound heap
};