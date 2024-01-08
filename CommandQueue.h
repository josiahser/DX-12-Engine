#pragma once

//#include "framework.h"
//#include "Window.h"
//#include "Game.h"
//#include "Application.h"
#include "DirectX-Headers/include/directx/d3d12.h"
#include <wrl.h>

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <queue>

#include "ThreadSafeQueue.h"

class CommandList;
class Device;

class CommandQueue
{
public:
	//Get an available command list from the command queue
	std::shared_ptr<CommandList> GetCommandList();

	//Execute a commandlist on the queue
	//Returns the fence value to wait for for this command list
	uint64_t ExecuteCommandList(std::shared_ptr<CommandList> commandList);
	uint64_t ExecuteCommandLists(const std::vector<std::shared_ptr<CommandList>>& commandLists);

	uint64_t Signal();
	bool IsFenceComplete(uint64_t fenceValue);
	void WaitForFenceValue(uint64_t fenceValue);
	void Flush();

	//Wait for another command queue to finish
	void Wait(const CommandQueue& other);

	Microsoft::WRL::ComPtr<ID3D12CommandQueue> GetD3D12CommandQueue() const;

protected:
	friend class std::default_delete<CommandQueue>;

	CommandQueue(Device& device, D3D12_COMMAND_LIST_TYPE type);
	virtual ~CommandQueue();
	//Used if no command list or command allocator are available
	/*Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CreateCommandAllocator();
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> CreateCommandList(Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator);*/

private:
	//Free any command lists that are finished processing on the command queue
	void ProcessInFlightCommandLists();
	
	//Keep track of command allocators that are "in-flight" (being used currently) (first member is the fence value to wait for, second is a shared pointer to the inflight command list.
	using CommandListEntry = std::tuple<uint64_t, std::shared_ptr<CommandList>>;

	Device&										m_Device;
	D3D12_COMMAND_LIST_TYPE						m_CommandListType;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue>	m_CommandQueue;
	Microsoft::WRL::ComPtr<ID3D12Fence>			m_Fence;
	std::atomic_uint64_t						m_FenceValue;

	ThreadSafeQueue<CommandListEntry>			m_InFlightCommandLists;
	ThreadSafeQueue<std::shared_ptr<CommandList>>m_AvailableCommandLists;

	//A thread to process in-flight command lists
	std::thread m_ProcessInFlightCommandListsThread;
	std::atomic_bool m_bProcessInFlightCommandLists;
	std::mutex m_ProcessInFlightCommandListsThreadMutex;
	std::condition_variable m_ProcessInFlightCommandListsThreadCV;
};