#include "framework.h"

#include "DescriptorAllocator.h"
#include "DescriptorAllocation.h"

DescriptorAllocator::DescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptorsPerHeap)
	: m_HeapType(type)
	, m_NumDescriptorsPerHeap(numDescriptorsPerHeap)
{
}

std::shared_ptr<DescriptorAllocatorPage> DescriptorAllocator::CreateAllocatorPage()
{
	auto newPage = std::make_shared<DescriptorAllocatorPage>(m_HeapType, m_NumDescriptorsPerHeap);

	m_HeapPool.emplace_back(newPage);
	m_AvaialableHeaps.insert(m_HeapPool.size() - 1);

	return newPage;
}

DescriptorAllocation DescriptorAllocator::Allocate(uint32_t numDescriptors)
{
	std::lock_guard<std::mutex> lock(m_AllocationMutex);

	DescriptorAllocation allocation;

	for (auto iter = m_AvaialableHeaps.begin(); iter != m_AvaialableHeaps.end(); ++iter)
	{
		auto allocatorPage = m_HeapPool[*iter];

		allocation = allocatorPage->Allocate(numDescriptors);

		if (allocatorPage->NumFreeHandles() == 0)
		{
			iter = m_AvaialableHeaps.erase(iter);
		}

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

void DescriptorAllocator::ReleaseStaleDescriptors(uint64_t frameNumber)
{
	std::lock_guard<std::mutex> lock(m_AllocationMutex);

	for (size_t i = 0; i < m_HeapPool.size(); ++i)
	{
		auto page = m_HeapPool[i];

		page->ReleaseStaleDescriptors(frameNumber);

		if (page->NumFreeHandles() > 0)
		{
			m_AvaialableHeaps.insert(i);
		}
	}
}