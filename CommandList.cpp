#include "framework.h"
#include "CommandList.h"

#include "Application.h"
#include "CommandQueue.h"
#include "DynamicDescriptorHeap.h"
#include "Resource.h"
#include "ResourceStateTracker.h"
#include "RootSignature.h"

//Static Global member variables
std::map<std::wstring, ID3D12Resource*> Command;