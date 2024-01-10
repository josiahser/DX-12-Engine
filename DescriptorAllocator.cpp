#include "framework.h"

#include "DescriptorAllocator.h"
#include "DescriptorAllocatorPage.h"

//Adapter for make_shared
struct MakeAllocatorPage : public DescriptorAllocatorPage
{
public:
	MakeAllocatorPage(Device& device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors)
		: DescriptorAllocatorPage(device, type, numDescriptors)
	{}

	virtual ~MakeAllocatorPage() {}
};

DescriptorAllocator::DescriptorAllocator(Device& device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptorsPerHeap)
	: m_Device(device)
	, m_HeapType(type)
	, m_NumDescriptorsPerHeap(numDescriptorsPerHeap)
{
}

DescriptorAllocator::~DescriptorAllocator()
{}

std::shared_ptr<DescriptorAllocatorPage> DescriptorAllocator::CreateAllocatorPage()
{
	std::shared_ptr<DescriptorAllocatorPage> newPage = std::make_shared<MakeAllocatorPage>(m_Device, m_HeapType, m_NumDescriptorsPerHeap);

	m_HeapPool.emplace_back(newPage);
	m_AvaialableHeaps.insert(m_HeapPool.size() - 1);

	return newPage;
}

DescriptorAllocation DescriptorAllocator::Allocate(uint32_t numDescriptors)
{
	std::lock_guard<std::mutex> lock(m_AllocationMutex);

	DescriptorAllocation allocation;

	auto iter = m_AvaialableHeaps.begin();
	while(iter != m_AvaialableHeaps.end())
	{
		auto allocatorPage = m_HeapPool[*iter];

		allocation = allocatorPage->Allocate(numDescriptors);

		if (allocatorPage->NumFreeHandles() == 0)
		{
			iter = m_AvaialableHeaps.erase(iter);
		}
		else
			++iter;

		//If a valid allocation has been found
		if (!allocation.IsNull())
			break;
	}

	//If no available heap could satisfy the requested number of descriptors, create a new one
	if (allocation.IsNull())
	{
		m_NumDescriptorsPerHeap = std::max(m_NumDescriptorsPerHeap, numDescriptors);
		auto newPage = CreateAllocatorPage();

		allocation = newPage->Allocate(numDescriptors);
	}

	return allocation;
}

void DescriptorAllocator::ReleaseStaleDescriptors()
{
	std::lock_guard<std::mutex> lock(m_AllocationMutex);

	for (size_t i = 0; i < m_HeapPool.size(); ++i)
	{
		auto page = m_HeapPool[i];

		page->ReleaseStaleDescriptors();

		if (page->NumFreeHandles() > 0)
		{
			m_AvaialableHeaps.insert(i);
		}
	}
}