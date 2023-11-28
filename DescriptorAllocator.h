/*
Descriptor Allocator is used to allocate descriptors from a CPU visible descriptor heap.
Useful for staging resource descriptors in CPU memory and later copied to a GPU visible descriptor heap
for use in a shader. Used when loading new resources (like texture), unloaded resources are returned back to the heap for reuse
Uses a *FREE LIST* list of available allocations, linearly searched and allocated using first-fit (can be improved with binary-search)
*/

#pragma once

#include "framework.h"

class DescriptorAllocator
{
public:

private:

};