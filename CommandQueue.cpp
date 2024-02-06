#include "framework.h"
#include "CommandQueue.h"

//#include "Application.h"
#include "CommandList.h"
#include "Device.h"
#include "ResourceStateTracker.h"

//Adapter for std::make_shared
class MakeCommandList : public CommandList
{
public:
	MakeCommandList(Device& device, D3D12_COMMAND_LIST_TYPE type)
		: CommandList(device, type)
	{}

	virtual ~MakeCommandList() {}
};

CommandQueue::CommandQueue(Device& device, D3D12_COMMAND_LIST_TYPE type)
	: m_Device(device)
	, m_CommandListType(type)
	, m_FenceValue(0)
	, m_bProcessInFlightCommandLists(true)
{
	auto d3d12Device = m_Device.GetD3D12Device();

	D3D12_COMMAND_QUEUE_DESC desc = {};
	desc.Type = type;
	desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	desc.NodeMask = 0;

	ThrowIfFailed(d3d12Device->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_CommandQueue)));
	ThrowIfFailed(d3d12Device->CreateFence(m_FenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence)));

	switch (type)
	{
	case D3D12_COMMAND_LIST_TYPE_COPY:
		m_CommandQueue->SetName(L"Copy Command Queue");
		break;
	case D3D12_COMMAND_LIST_TYPE_COMPUTE:
		m_CommandQueue->SetName(L"Compute Command Queue");
		break;
	case D3D12_COMMAND_LIST_TYPE_DIRECT:
		m_CommandQueue->SetName(L"Direct Command Queue");
		break;
	}

	//Set the thread name for easy debugging
	char threadName[256];
	sprintf_s(threadName, "ProcessInFlightCommandLists");
	switch (type)
	{
	case D3D12_COMMAND_LIST_TYPE_DIRECT:
		strcat_s(threadName, "(Direct)");
		break;
	case D3D12_COMMAND_LIST_TYPE_COMPUTE:
		strcat_s(threadName, "(Compute)");
		break;
	case D3D12_COMMAND_LIST_TYPE_COPY:
		strcat_s(threadName, "(Copy)");
		break;
	default:
		break;
	}

	m_ProcessInFlightCommandListsThread = std::thread(&CommandQueue::ProcessInFlightCommandLists, this);
	SetThreadName(m_ProcessInFlightCommandListsThread, threadName);
}

CommandQueue::~CommandQueue()
{
	m_bProcessInFlightCommandLists = false;
	m_ProcessInFlightCommandListsThread.join();
}

uint64_t CommandQueue::Signal()
{
	uint64_t fenceValue = ++m_FenceValue;
	m_CommandQueue->Signal(m_Fence.Get(), fenceValue);

	return fenceValue;
}

bool CommandQueue::IsFenceComplete(uint64_t fenceValue)
{
	return m_Fence->GetCompletedValue() >= fenceValue;
}

void CommandQueue::WaitForFenceValue(uint64_t fenceValue)
{
	if (!IsFenceComplete(fenceValue))
	{
		auto event = ::CreateEvent(NULL, FALSE, FALSE, NULL);
		if (event)
		{
			m_Fence->SetEventOnCompletion(fenceValue, event);
			::WaitForSingleObject(event, DWORD_MAX);

			::CloseHandle(event);
		}
	}
}

void CommandQueue::Flush()
{
	std::unique_lock<std::mutex> lock(m_ProcessInFlightCommandListsThreadMutex);
	m_ProcessInFlightCommandListsThreadCV.wait(lock, [this] { return m_InFlightCommandLists.Empty(); });

	//In case the command queue was signaled directly using CommandQueue::Signal()
	//Then the fence value of the command queue might be higher than the fence value of any of the executed command lists
	WaitForFenceValue(m_FenceValue);
}

//Returns a command list in an executable state (doesn't need to be reset)
std::shared_ptr<CommandList> CommandQueue::GetCommandList()
{
	std::shared_ptr<CommandList> commandList;

	//If there is a command list on queue
	if (!m_AvailableCommandLists.Empty())
	{
		m_AvailableCommandLists.TryPop(commandList);
	}
	else
	{
		//Otherwise create a new command list
		commandList = std::make_shared<MakeCommandList>(m_Device, m_CommandListType);
	}

	return commandList;

	////Checking to see if the allocator queue isn't empty, and whether there's any unused or reusable allocators (not in flight)
	//if (!m_CommandAllocatorQueue.empty() && IsFenceComplete(m_CommandAllocatorQueue.front().fenceValue))
	//{
	//	commandAllocator = m_CommandAllocatorQueue.front().commandAllocator;
	//	m_CommandAllocatorQueue.pop();

	//	ThrowIfFailed(commandAllocator->Reset());
	//}
	//else
	//{
	//	//If there's no allocators on the queue (it's empty) or they're all unusable (in flight) then create one
	//	commandAllocator = CreateCommandAllocator();
	//}
	////Use this valid allocator to create the command list associated with that allocator
	//if (!m_CommandListQueue.empty()) //If the command list queue is not empty (command lists have been made)
	//{
	//	commandList = m_CommandListQueue.front();
	//	m_CommandListQueue.pop();

	//	ThrowIfFailed(commandList->Reset(commandAllocator.Get(), nullptr));
	//}
	//else
	//{
	//	//If the commandlist queue IS empty, and there's none to use, create a command list using the command allocator designated earlier
	//	commandList = CreateCommandList(commandAllocator);
	//}
	////Associate the command allocator with the command list so it can be retrieved when the command list is executed
	//ThrowIfFailed(commandList->SetPrivateDataInterface(__uuidof(ID3D12CommandAllocator), commandAllocator.Get()));

	//return commandList;
}

//Execute command list and return the fence value to wait for for this command list.
uint64_t CommandQueue::ExecuteCommandList(std::shared_ptr<CommandList> commandList)
{
	return ExecuteCommandLists(std::vector<std::shared_ptr<CommandList>>({ commandList }));
	////Close before a command list can be executed (so no new commands can be added while it's executing)
	//commandList->Close();

	////Retrieve the associated command allocator to the commandlist
	//ID3D12CommandAllocator* commandAllocator;
	//UINT dataSize = sizeof(commandAllocator);
	//ThrowIfFailed(commandList->GetPrivateData(__uuidof(ID3D12CommandAllocator), &dataSize, &commandAllocator));

	////Create temp array since execute command lists expects an array of command lists
	//ID3D12CommandList* const ppCommandLists[] = {
	//	commandList.Get()
	//};

	//m_CommandQueue->ExecuteCommandLists(1, ppCommandLists);
	//uint64_t fenceValue = Signal();

	////Put the command allocator and list to the back of their respective queues
	//m_CommandAllocatorQueue.emplace(CommandAllocatorEntry{ fenceValue, commandAllocator });
	//m_CommandListQueue.push(commandList);

	////The ownership of the command allocator has been transferred to the ComPtr
	////In the command allocator queue. It's safe to release the reference in this temp COM pointer here
	//commandAllocator->Release();

	//return fenceValue;
}

uint64_t CommandQueue::ExecuteCommandLists(const std::vector<std::shared_ptr<CommandList>>& commandLists)
{
	ResourceStateTracker::Lock();

	//Command Lists that need to be put back on the command list queue
	std::vector<std::shared_ptr<CommandList>> toBeQueued;
	toBeQueued.reserve(commandLists.size() * 2); //2 times since each command list will have a pending command list

	//Generate mips command lists.
	std::vector<std::shared_ptr<CommandList>> generateMipsCommandLists;
	generateMipsCommandLists.reserve(commandLists.size());

	//Command lists that need to be executed
	std::vector<ID3D12CommandList*> d3d12CommandLists;
	d3d12CommandLists.reserve(commandLists.size() * 2); //2x again for the pending command lists

	for (auto commandList : commandLists)
	{
		auto pendingCommandList = GetCommandList();
		bool hasPendingBarriers = commandList->Close(pendingCommandList);
		pendingCommandList->Close();
		//If there are no pending barriers on the pending command list, there is no reason to execute an empty command list on the command queue
		if (hasPendingBarriers)
		{
			d3d12CommandLists.push_back(pendingCommandList->GetD3D12CommandList().Get());
		}
		d3d12CommandLists.push_back(commandList->GetD3D12CommandList().Get());

		toBeQueued.push_back(pendingCommandList);
		toBeQueued.push_back(commandList);

		auto generateMipsCommandList = commandList->GetGenerateMipsCommandList();
		if (generateMipsCommandList)
		{
			generateMipsCommandList->Close();
			generateMipsCommandLists.push_back(generateMipsCommandList);
		}
	}

	UINT numCommandLists = static_cast<UINT>(d3d12CommandLists.size());
	m_CommandQueue->ExecuteCommandLists(numCommandLists, d3d12CommandLists.data());
	uint64_t fenceValue = Signal();

	ResourceStateTracker::Unlock();

	//Queue command lists for reuse
	for (auto commandList : toBeQueued)
	{
		m_InFlightCommandLists.Push({ fenceValue, commandList });
	}

	//If there are any command lists that generate mips then execute those after the initial resource command lists have finished
	if (generateMipsCommandLists.size() > 0)
	{
		auto& computeQueue = m_Device.GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COMPUTE);
		computeQueue.Wait(*this);
		computeQueue.ExecuteCommandLists(generateMipsCommandLists);
	}
	return fenceValue;
}

void CommandQueue::Wait(const CommandQueue& other)
{
	m_CommandQueue->Wait(other.m_Fence.Get(), other.m_FenceValue);
}

Microsoft::WRL::ComPtr<ID3D12CommandQueue> CommandQueue::GetD3D12CommandQueue() const
{
	return m_CommandQueue;
}

void CommandQueue::ProcessInFlightCommandLists()
{
	std::unique_lock<std::mutex> lock(m_ProcessInFlightCommandListsThreadMutex, std::defer_lock);

	while (m_bProcessInFlightCommandLists)
	{
		CommandListEntry commandListEntry;
		lock.lock();
		while (m_InFlightCommandLists.TryPop(commandListEntry))
		{
			auto fenceValue = std::get<0>(commandListEntry);
			auto commandList = std::get<1>(commandListEntry);

			WaitForFenceValue(fenceValue);

			commandList->Reset();

			m_AvailableCommandLists.Push(commandList);
		}
		lock.unlock();
		m_ProcessInFlightCommandListsThreadCV.notify_one();

		std::this_thread::yield();
	}
}