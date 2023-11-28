#pragma once

//#include "framework.h"
//#include "Window.h"
//#include "Game.h"
//#include "Application.h"
#include <d3d12.h>
#include <wrl.h>

#include <cstdint>
#include <queue>

class CommandQueue
{
public:
	CommandQueue(Microsoft::WRL::ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type);
	virtual ~CommandQueue();

	//Get an available command list from the command queue
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> GetCommandList();

	//Execute a commandlist on the queue
	//Returns the fence value to wait for for this command list
	uint64_t ExecuteCommandList(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList);

	uint64_t Signal();
	bool IsFenceComplete(uint64_t fenceValue);
	void WaitForFenceValue(uint64_t fenceValue);
	void Flush();

	Microsoft::WRL::ComPtr<ID3D12CommandQueue> GetD3D12CommandQueue() const;

protected:
	//Used if no command list or command allocator are available
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CreateCommandAllocator();
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> CreateCommandList(Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator);

private:
	//Keep track of command allocators that are "in-flight" (being used currently)
	struct CommandAllocatorEntry
	{
		uint64_t fenceValue{};
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
	};

	using CommandAllocatorQueue = std::queue<CommandAllocatorEntry>;
	using CommandListQueue = std::queue<Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> >;

	D3D12_COMMAND_LIST_TYPE						m_CommandListType;
	Microsoft::WRL::ComPtr<ID3D12Device2>		m_Device;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue>	m_CommandQueue;
	Microsoft::WRL::ComPtr<ID3D12Fence>			m_Fence;
	HANDLE										m_FenceEvent;
	uint64_t									m_FenceValue;

	CommandAllocatorQueue						m_CommandAllocatorQueue;
	CommandListQueue							m_CommandListQueue;
};