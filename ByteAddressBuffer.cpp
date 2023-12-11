#include "framework.h"

#include "ByteAddressBuffer.h"
#include "Application.h"

ByteAddressBuffer::ByteAddressBuffer(const std::wstring& name)
	: Buffer(name)
{
	m_SRV = Application::Get().AllocateDescriptors()
}