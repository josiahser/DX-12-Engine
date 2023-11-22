#include "framework.h"
#include "CommandQueue.h"

CommandQueue::CommandQueue(Microsoft::WRL::ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type)
	: m_FenceValue(0), m_CommandListType(type), m_Device(device)
{
	D3D12_COMMAND_QUEUE_DESC desc = {};
	desc.Type = type;
	desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	desc.NodeMask = 0;

	ThrowIfFailed(m_Device->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_CommandQueue)));
	ThrowIfFailed(m_Device->CreateFence(m_FenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence)));

	m_FenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(m_FenceEvent && "Failed to create fence event handle.");
}

CommandQueue::~CommandQueue()
{

}

Microsoft::WRL::ComPtr<ID3D12CommandAllocator>CommandQueue::CreateCommandAllocator()
{
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator{};
	ThrowIfFailed(m_Device->CreateCommandAllocator(m_CommandListType, IID_PPV_ARGS(&commandAllocator)));

	return commandAllocator;
}

Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2>CommandQueue::CreateCommandList(Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator)
{
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList{};
	ThrowIfFailed(m_Device->CreateCommandList(0, m_CommandListType, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList)));

	return commandList;
}

//Returns a command list in an executable state (doesn't need to be reset)
Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2>CommandQueue::GetCommandList()
{
	//Temp variables to store the command allocator and list
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator{};
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList{};

	//Checking to see if the allocator queue isn't empty, and whether there's any unused or reusable allocators (not in flight)
	if (!m_CommandAllocatorQueue.empty() && IsFenceComplete(m_CommandAllocatorQueue.front().fenceValue))
	{
		commandAllocator = m_CommandAllocatorQueue.front().commandAllocator;
		m_CommandAllocatorQueue.pop();

		ThrowIfFailed(commandAllocator->Reset());
	}
	else
	{
		//If there's no allocators on the queue (it's empty) or they're all unusable (in flight) then create one
		commandAllocator = CreateCommandAllocator();
	}
	//Use this valid allocator to create the command list associated with that allocator
	if (!m_CommandListQueue.empty()) //If the command list queue is not empty (command lists have been made)
	{
		commandList = m_CommandListQueue.front();
		m_CommandListQueue.pop();

		ThrowIfFailed(commandList->Reset(commandAllocator.Get(), nullptr));
	}
	else
	{
		//If the commandlist queue IS empty, and there's none to use, create a command list using the command allocator designated earlier
		commandList = CreateCommandList(commandAllocator);
	}
	//Associate the command allocator with the command list so it can be retrieved when the command list is executed
	ThrowIfFailed(commandList->SetPrivateDataInterface(__uuidof(ID3D12CommandAllocator), commandAllocator.Get()));

	return commandList;
}

uint64_t CommandQueue::ExecuteCommandList(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList)
{
	//Close before a command list can be executed (so no new commands can be added while it's executing)
	commandList->Close();

	//Retrieve the associated command allocator to the commandlist
	ID3D12CommandAllocator* commandAllocator{};
	UINT dataSize = sizeof(commandAllocator);
	ThrowIfFailed(commandList->GetPrivateData(__uuidof(ID3D12CommandAllocator), &dataSize, &commandAllocator));

	//Create temp array since execute command lists expects an array of command lists
	ID3D12CommandList* const ppCommandLists[] = {
		commandList.Get()
	};

	m_CommandQueue->ExecuteCommandLists(1, ppCommandLists);
	uint64_t fenceValue = Signal();

	//Put the command allocator and list to the back of their respective queues
	m_CommandAllocatorQueue.emplace(CommandAllocatorEntry{ fenceValue, commandAllocator });
	m_CommandListQueue.push(commandList);

	//The ownership of the command allocator has been transferred to the ComPtr
	//In the command allocator queue. It's safe to release the reference in this temp COM pointer here
	commandAllocator->Release();

	return fenceValue;
}

uint64_t CommandQueue::Signal()
{
	uint64_t fenceValueForSignal = ++m_FenceValue;
	ThrowIfFailed(m_CommandQueue->Signal(m_Fence.Get(), fenceValueForSignal));

	return fenceValueForSignal;
}

void CommandQueue::WaitForFenceEvent(uint64_t fenceValue)
{
	std::chrono::milliseconds duration = std::chrono::milliseconds::max();
	if (m_Fence->GetCompletedValue() < fenceValue)
	{
		ThrowIfFailed(m_Fence->SetEventOnCompletion(fenceValue, m_FenceEvent));
		::WaitForSingleObject(m_FenceEvent, static_cast<DWORD>(duration.count()));
	}
}

bool CommandQueue::IsFenceComplete(uint64_t fenceValue)
{
	return m_Fence->GetCompletedValue() >= fenceValue;
}

void CommandQueue::Flush()
{
	WaitForFenceEvent(Signal());
}

Microsoft::WRL::ComPtr<ID3D12CommandQueue> CommandQueue::GetD3D12CommandQueue() const
{
	return m_CommandQueue;
}