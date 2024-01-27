#pragma once

#pragma once

#include "DescriptorAllocation.h"

#include "d3dx12.h"
#include <dxgi1_6.h>
#include <wrl/client.h>

#include <memory>

class Adapter;
class ByteAddressBuffer;
class CommandQueue;
class CommandList;
class ConstantBuffer;
class ConstantBufferView;
class DescriptorAllocator;
class GUI;
class IndexBuffer;
class PipelineStateObject;
class RenderTarget;
class Resource;
class RootSignature;
class Scene;
class ShaderResourceView;
class StructuredBuffer;
class SwapChain;
class Texture;
class UnorderedAccessView;
class VertexBuffer;

class Device
{
public:
	//Always enable the DX12 Debug layer before doing anything DX12 related
	static void EnableDebugLayer();
	static void ReportLiveObjects();

	//Create a new DX12 device using the provided adapter, if none is specified, the highest performance one is chosen
	static std::shared_ptr<Device> Create(std::shared_ptr<Adapter> adapter = nullptr);

	//Get a description of the adapter that was used to create the device
	std::wstring GetDescription() const;

	//Allocate a number of CPU visible descriptors
	DescriptorAllocation AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors = 1);

	//Gets the size of the handle increment for the given type of descriptor heap
	UINT GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE type) const
	{
		return m_d3d12Device->GetDescriptorHandleIncrementSize(type);
	}

	//Create a swapchain using the provided OS window handle
	std::shared_ptr<SwapChain> CreateSwapChain(HWND hWnd, DXGI_FORMAT backBufferFormat = DXGI_FORMAT_R10G10B10A2_UNORM);

	//Create a GUI object
	std::shared_ptr<GUI> CreateGUI(HWND hWnd, const RenderTarget& renderTarget);

	//Create a ConstantBuffer from a given D3D12Resource
	std::shared_ptr<ConstantBuffer> CreateConstantBuffer(Microsoft::WRL::ComPtr<ID3D12Resource> resource);

	//Create a ByteAddressBuffer resource
	//@param resDesc a description of the resource
	std::shared_ptr<ByteAddressBuffer> CreateByteAddressBuffer(size_t bufferSize);
	std::shared_ptr<ByteAddressBuffer> CreateByteAddressBuffer(Microsoft::WRL::ComPtr<ID3D12Resource> resource);

	//Create a structured buffer resource
	std::shared_ptr<StructuredBuffer> CreateStructuredBuffer(size_t numElements, size_t elementSize);
	std::shared_ptr<StructuredBuffer> CreateStructuredBuffer(Microsoft::WRL::ComPtr<ID3D12Resource> resource, size_t numElements, size_t elementSize);

	//Create a texture resource
	//@param resourceDesc A descrip of the texture to create
	//@param [clearVlue] Optional optimized clear value for texture
	//@param [textureUsage] Optional texture usage flag for a hint how texture will be used
	//Return a pointer to the created texture
	std::shared_ptr<Texture> CreateTexture(const D3D12_RESOURCE_DESC& resourceDesc, const D3D12_CLEAR_VALUE* clearValue = nullptr);
	std::shared_ptr<Texture> CreateTexture(Microsoft::WRL::ComPtr<ID3D12Resource> resource, const D3D12_CLEAR_VALUE* clearValue = nullptr);

	std::shared_ptr<IndexBuffer> CreateIndexBuffer(size_t numIndices, DXGI_FORMAT indexFormat);
	std::shared_ptr<IndexBuffer> CreateIndexBuffer(Microsoft::WRL::ComPtr<ID3D12Resource> resource, size_t numIndices, DXGI_FORMAT indexFormat);

	std::shared_ptr<VertexBuffer> CreateVertexBuffer(size_t numVertices, size_t vertexStride);
	std::shared_ptr<VertexBuffer> CreateVertexBuffer(Microsoft::WRL::ComPtr<ID3D12Resource> resource, size_t numVertices, size_t vertexStride);

	std::shared_ptr<RootSignature> CreateRootSignature(const D3D12_ROOT_SIGNATURE_DESC1& rootSignatureDesc);

	template<class PipelineStateStream>
	std::shared_ptr<PipelineStateObject> CreatePipelineStateObject(PipelineStateStream& pipelineStateStream)
	{
		D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = { sizeof(PipelineStateStream), &pipelineStateStream };

		return DoCreatePipelineStateObject(pipelineStateStreamDesc);
	}

	std::shared_ptr<ConstantBufferView> CreateConstantBufferView(const std::shared_ptr<ConstantBuffer>& constantBuffer, size_t offset = 0);

	std::shared_ptr<ShaderResourceView> CreateShaderResourceView(const std::shared_ptr<Resource>& resource, const D3D12_SHADER_RESOURCE_VIEW_DESC* srv = nullptr);

	std::shared_ptr<UnorderedAccessView> CreateUnorderedAccessView(const std::shared_ptr<Resource>& resource, const std::shared_ptr<Resource>& counterResource = nullptr, const D3D12_UNORDERED_ACCESS_VIEW_DESC* uav = nullptr);

	//Flush all command queues
	void Flush();

	//Release stale descriptors, only should be called with a completed frame counter
	void ReleaseStaleDescriptors();

	//Get the adapter used to create this device
	std::shared_ptr<Adapter> GetAdapter() const
	{
		return m_Adapter;
	}

	//Get a command queue. either as direct, compute, or copy type. By default direct queue is returned
	CommandQueue& GetCommandQueue(D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT);

	Microsoft::WRL::ComPtr<ID3D12Device2> GetD3D12Device() const
	{
		return m_d3d12Device;
	}

	D3D_ROOT_SIGNATURE_VERSION GetHighestRootSignatureVersion() const
	{
		return m_HighestRootSignatureVersion;
	}

	//Check if the requested multisample quality is supported for the given format
	DXGI_SAMPLE_DESC GetMultisampleQualityLevels(DXGI_FORMAT format, UINT numSamples = D3D12_MAX_MULTISAMPLE_SAMPLE_COUNT, D3D12_MULTISAMPLE_QUALITY_LEVEL_FLAGS flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE) const;

protected:
	explicit Device(std::shared_ptr<Adapter> adapter);
	virtual ~Device();

	std::shared_ptr<PipelineStateObject> DoCreatePipelineStateObject(const D3D12_PIPELINE_STATE_STREAM_DESC& pipelineStateStreamDesc);

private:
	Microsoft::WRL::ComPtr<ID3D12Device2> m_d3d12Device;

	//Adapter used to create device
	std::shared_ptr<Adapter> m_Adapter;

	//Default command queue
	std::unique_ptr<CommandQueue> m_DirectCommandQueue;
	std::unique_ptr<CommandQueue> m_ComputeCommandQueue;
	std::unique_ptr<CommandQueue> m_CopyCommandQueue;

	//Descriptor Allocators
	std::unique_ptr<DescriptorAllocator> m_DescriptorAllocators[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

	D3D_ROOT_SIGNATURE_VERSION m_HighestRootSignatureVersion;
};