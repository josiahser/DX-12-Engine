#pragma once

#include "TextureUsage.h"

#include <d3d12.h>
#include <wrl.h>

#include <map>
#include <memory>
#include <mutex>
#include <vector>

class Buffer;
class ByteAddressBuffer;
class ConstantBuffer;
class DynamicDescriptorHeap;
class GenerateMipsPSO;
class IndexBuffer;
class PanoToCubemapPSO;
class RenderTarget;
class Resource;
class ResourceStateTracker;
class StructuredBuffer;
class RootSignature;
class Texture;
class UploadBuffer;
class VertexBuffer;

class CommandList
{
public:
    CommandList(D3D12_COMMAND_LIST_TYPE type);
    virtual ~CommandList();
    
    //Get the type of command list
    D3D12_COMMAND_LIST_TYPE GetCommandListType() const
    {
        return m_d3d12CommandListType;
    }

    //Get direct access to the ID3D12GraphicsCommandList2 interface
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2>GetGraphicsCommandList() const
    {
        return m_d3d12CommandList;
    }

    //Transition a resource to a particular state. The state before is handled by the resource state tracker
    //subresource is defaulted to all subresources (all subresources transition to the same state)
    //Flush barriers is whether to force flush any barriers, since they need to be flushed before a command that expects the resource to be in a particular state can run
    void TransitionBarrier(const Resource& resource, D3D12_RESOURCE_STATES stateAfter, UINT subResource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, bool flushBarriers);

    //Add a UAV barrier to ensure that any writes to a resource have completed before reading from the resource
    //Resource is the resource to add the barrier for, and flushbarriers is whether to force flush
    void UAVBarrier(const Resource& resource, bool flushBarriers = false);

    //Add an aliasing barrier to indicate a transition btwn usages of two different resources that occupy the same space in a heap
    void AliasingBarrier(const Resource& beforeResource, const Resource& afterResource, bool flushBarriers = false);
    
    //Flush any barriers that have been pushed to the command list
    void FlushResourceBarriers();

    void CopyResource(Resource& dstRes, const Resource& srcRes);

    //Resolve a multisampled resource into a non-multisampled resource
    void ResolveSubResource(Resource& dstRes, const Resource& srcRes, uint32_t dstSubresource = 0, uint32_t srcSubresource = 0);

    //Copy the contents to a vertex buffer in GPU memory
    void CopyVertexBuffer(VertexBuffer& vertexBuffer, size_t numVertices, size_t vertexStride, const void* vertexBufferData);
    template<typename T>
    void CopyVertexBuffer(VertexBuffer& vertexBuffer, const std::vector<T>& vertexBufferData)
    {
        CopyVertexBuffer(vertexBuffer, vertexBufferData.size(), sizeof(T), vertexBufferData.data());
    }

    //Copy the contents to an index buffer in GPU memory
    void CopyIndexBuffer(IndexBuffer& indexBuffer, size_t numIndices, DXGI_FORMAT indexFormat, const void* indexBufferData);
    template<typename T>
    void CopyIndexBuffer(IndexBuffer& indexBuffer, const std::vector<T>& indexBufferData)
    {
        assert(sizeof(T) == 2 || sizeof(T) == 4);

        DXGI_FORMAT indexFormat = (sizeof(T) == 2) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
        CopyIndexBuffer(indexBuffer, indexBufferData.size(), indexFormat, indexBufferData.data());
    }

    //Copy the contents to a byte address buffer in GPU memory
    void CopyByteAddressBuffer(ByteAddressBuffer& byteAddressBuffer, size_t bufferSize, const void* bufferData);
    template<typename T>
    void CopyByteAddressBuffer(ByteAddressBuffer& byteAddressBuffer, const T& data)
    {
        CopyByteAddressBuffer(byteAddressBuffer, sizeof(T), &data);
    }

    //Copy the contents to a structured buffer in GPU memory
    void CopyStructuredBuffer(StructuredBuffer& structuredBuffer, size_t numElements, size_t elementSize, const void* bufferData);
    template <typename T>
    void CopyStructuredBuffer(StructuredBuffer& structuredBuffer, const std::vector<T>& bufferData)
    {
        CopyStructuredBuffer(structuredBuffer, bufferData.size(), sizeof(T), bufferData.data());
    }

    //Set the current primitive topology for the rendering pipeline
    void SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY primitiveTopology);

    //Load texture by a filename
    void LoadTextureFromFile(Texture& texture, const std::wstring& fileName, TextureUsage textureUsage = TextureUsage::Albedo);

    //Clear a texture
    void ClearTexture(const Texture& texture, const float clearColor[4]);

    //Clear depth/stencil texture
    void ClearDepthStencilTexture(const Texture& texture, D3D12_CLEAR_FLAGS clearFlags, float depth = 1.0f, uint8_t stencil = 0);

    //Generate mips for the texture.
    //The first subresource is used to generate the mip chain
    //Mips are automatically generated for textures loaded from files
    void GenerateMips(Texture& texture);

    //Generate a cubemap texture from a panoramic (equirectangular) texture
    void PanoToCubemap(Texture& cubemap, const Texture& pano);

    //Copy subresource data to a texture
    void CopyTextureSubresource(Texture& texture, uint32_t firstSubresource, uint32_t numSubresources, D3D12_SUBRESOURCE_DATA* subresourceData);

    //Set a dynamic constant buffer data to an inline descriptor in the root signature
    void SetGraphicsDynamicConstantBuffer(uint32_t rootParameterIndex, size_t sizeInBytes, const void* bufferData);
    template<typename T>
    void SetGraphicsDynamicConstantBuffer(uint32_t rootParameterIndex, const T& data)
    {
        SetGraphicsDynamicConstantBuffer(rootParameterIndex, sizeof(T), &data);
    }

    //Set a set of 32-bit constants on the graphics pipeline
    void SetGraphics32BitConstants(uint32_t rootParameterIndex, uint32_t numConstants, const void* constants);
    template<typename T>
    void SetGraphics32BitConstants(uint32_t rootParameterIndex, const T& constants)
    {
        static_assert(sizeof(T) % sizeof(uint32_t) == 0, "Size of type must be a multiple of 4 bytes");
        SetGraphics32BitConstants(rootParameterIndex, sizeof(T) / sizeof(uint32_t), &constants);
    }

    //Set a set of 32-bit constants on the compute pipeline
    void SetCompute32BitConstants(uint32_t rootParameterIndex, uint32_t numConstants, const void* constants);
    template<typename T>
    void SetCompute32BitConstants(uint32_t rootParameterIndex, const T& constants)
    {
        static_assert(sizeof(T) % sizeof(uint32_t) == 0, "Size of type must be a multiple of 4 bytes");
        SetCompute32BitConstants(rootParameterIndex, sizeof(T) / sizeof(uint32_t), &constants);
    }

    //Set the vertex buffer to the rendering pipeline
    void SetVertexBuffer(uint32_t slot, const VertexBuffer& vertexBuffer);

    //Set dynamic vertex buffer data to the rendering pipeline
    void SetDynamicVertexBuffer(uint32_t slot, size_t numVertices, size_t vertexSize, const void* vertexBufferData);
    template<typename T>
    void SetDynamicVertexBuffer(uint32_t slot, const std::vector<T>& vertexBufferData)
    {
        SetDynamicVertexBuffer(slot, vertexBufferData.size(), sizeof(T), vertexBufferData.data());
    }

    //Bind the index buffer to the rendering pipeline
    void SetIndexBuffer(const IndexBuffer& indexBuffer);

    //Bind the dynamic index buffer data to the rendering pipeline
    void SetDynamicIndexBuffer(size_t numIndicies, DXGI_FORMAT indexFormat, const void* indexBufferData);
    template<typename T>
    void SetDynamicIndexBuffer(const std::vector<T>& indexBufferData)
    {
        static_assert(sizeof(T) == 2 || sizeof(T) == 4);

        DXGI_FORMAT indexFormat = (sizeof(T) == 2) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
        SetDynamicIndexBuffer(indexBufferData.size(), indexFormat, indexBufferData.data());
    }

    //Set dynamic structured buffer contents
    void SetGraphicsDynamicStructuredBuffer(uint32_t slot, size_t numElements, size_t elementSize, const void* bufferData);
    template<typename T>
    void SetGraphicsDynamicStructuredBuffer(uint32_t slot, const std::vector<T>& bufferData)
    {
        SetGraphicsDynamicStructuredBuffer(slot, bufferData.size(), sizeof(T), bufferData.data());
    }

    //Set viewports
    void SetViewport(const D3D12_VIEWPORT& viewport);
    void SetViewports(const std::vector<D3D12_VIEWPORT>& viewports);

    //Set scissor rects
    void SetScissorRect(const D3D12_RECT& scissorRect);
    void SetScissorRects(const std::vector<D3D12_RECT>& scissorRects);

    //Set the pipeline state object on the command list
    void SetPipelineState(Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState);

    //Set the current root signature on the command list
    void SetGraphicsRootSignature(const RootSignature& rootSignature);
    void SetComputeRootSignature(const RootSignature& rootSignature);

    //Set the SRV on the graphics pipeline
    void SetShaderResourceView(uint32_t rootParameterIndex,
        uint32_t descriptorOffset,
        const Resource& resource,
        D3D12_RESOURCE_STATES stateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
        UINT firstSubresource = 0,
        UINT numSubresources = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
        const D3D12_SHADER_RESOURCE_VIEW_DESC* srv = nullptr);

    //Set the UAV on the graphics pipeline
    void SetUnorderedAccessView(uint32_t rootParameterIndex,
        uint32_t descriptorOffset,
        const Resource& resource,
        D3D12_RESOURCE_STATES stateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        UINT firstSubresource = 0,
        UINT numSubresources = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
        const D3D12_UNORDERED_ACCESS_VIEW_DESC* uav = nullptr);

    //Set the render targets for the graphics rendering pipeline
    void SetRenderTarget(const RenderTarget& renderTarget);

    //Draw geometry
    void Draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t startVertex = 0, uint32_t startInstance = 0);
    void DrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1, uint32_t startIndex = 0, uint32_t baseVertex = 0, uint32_t startInstance = 0);

    //Dispatch a compute shader
    void Dispatch(uint32_t numGroupsX, uint32_t numGroupsY = 1, uint32_t numGroupsZ = 1);

    void SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap);

    void TrackResource(const Resource& resource);

private:
    D3D12_COMMAND_LIST_TYPE m_d3d12CommandListType;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> m_d3d12CommandList;
    ResourceStateTracker m_ResourceStateTracker;
    std::unique_ptr<UploadBuffer> m_UploadBuffer;
    std::unique_ptr<DynamicDescriptorHeap> m_DynamicDescriptorHeap;
};