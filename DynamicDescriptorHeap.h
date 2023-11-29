#pragma once

#include "DirectX-Headers/include/directx/d3dx12.h"

#include <wrl.h>

#include <cstdint>
#include <memory>
#include <queue>
#include <functional>


class CommandList;
class RootSignature;

class DynamicDescriptorHeap
{
public:
	DynamicDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, uint32_t numDescriptorsPerHeap = 1024);

	virtual ~DynamicDescriptorHeap();

	//Stages a contiguous range of CPU visible descriptors
	//Descriptors are not copied to the GPU visible descriptor heap until
	//The CommitStagedDescriptors function is called
	void StageDescriptors(uint32_t rootParameterIndex, uint32_t offset, uint32_t numDescriptors, const D3D12_CPU_DESCRIPTOR_HANDLE srcDescriptors);

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
	void CommitStagedDescriptors(CommandList& commandList, std::function<void(ID3D12GraphicsCommandList*, UINT, D3D12_GPU_DESCRIPTOR_HANDLE)> setFunc);
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


};