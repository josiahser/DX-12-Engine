#include "framework.h"

#include "Adapter.h"
#include "ByteAddressBuffer.h"
#include "CommandList.h"
#include "CommandQueue.h"
#include "ConstantBuffer.h"
#include "ConstantBufferView.h"
#include "DescriptorAllocator.h"
#include "Device.h"
#include "IndexBuffer.h"
#include "PipelineStateObject.h"
#include "ResourceStateTracker.h"
#include "RootSignature.h"
#include "Scene.h"
#include "ShaderResourceView.h"
#include "StructuredBuffer.h"
#include "SwapChain.h"
#include "Texture.h"
#include "UnorderedAccessView.h"
#include "VertexBuffer.h"

#pragma region Class adapters for std::make_shared

class MakeGUI : public GUI
{

};