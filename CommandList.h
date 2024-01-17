#pragma once

#include "TextureUsage.h"
#include "VertexTypes.h"
#include <DirectXMath.h>

#include "DirectX-Headers/include/directx/d3d12.h"
#include <wrl.h>

#pragma comment(lib, "d3d12")
#pragma comment(lib, "dxgi")
#pragma comment(lib, "d3dcompiler")
#pragma comment(lib, "DXGUID")

#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <vector>

class Buffer;
class ByteAddressBuffer;
class ConstantBuffer;
class ConstantBufferView;
class Device;
class DynamicDescriptorHeap;
class GenerateMipsPSO;
class IndexBuffer;
class PanoToCubemapPSO;
class PipelineStateObject;
class RenderTarget;
class Resource;
class ResourceStateTracker;
class RootSignature;
class Scene;
class ShaderResourceView;
class StructuredBuffer;
class Texture;
class UnorderedAccessView;
class UploadBuffer;
class VertexBuffer;

class CommandList : public std::enable_shared_from_this<CommandList>
{
public:
    /*CommandList(D3D12_COMMAND_LIST_TYPE type);
    virtual ~CommandList();*/
    
    //Get the type of command list
    D3D12_COMMAND_LIST_TYPE GetCommandListType() const
    {
        return m_d3d12CommandListType;
    }

    //Get the device that was used to create the command list
    Device& GetDevice() const
    {
        return m_Device;
    }

    //Get direct access to the ID3D12GraphicsCommandList2 interface
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2>GetD3D12CommandList() const
    {
        return m_d3d12CommandList;
    }

    //Transition a resource to a particular state. The state before is handled by the resource state tracker
    //subresource is defaulted to all subresources (all subresources transition to the same state)
    //Flush barriers is whether to force flush any barriers, since they need to be flushed before a command that expects the resource to be in a particular state can run
    void TransitionBarrier(const std::shared_ptr<Resource>& resource, D3D12_RESOURCE_STATES stateAfter, UINT subResource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, bool flushBarriers = false);
    void TransitionBarrier(Microsoft::WRL::ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES stateAfter, UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, bool flushBarriers = false);

    //Add a UAV barrier to ensure that any writes to a resource have completed before reading from the resource
    //Resource is the resource to add the barrier for, and flushbarriers is whether to force flush
    void UAVBarrier(const std::shared_ptr<Resource>& resource = nullptr, bool flushBarriers = false);
    void UAVBarrier(Microsoft::WRL::ComPtr<ID3D12Resource> resource, bool flushBarriers = false);

    //Add an aliasing barrier to indicate a transition btwn usages of two different resources that occupy the same space in a heap
    void AliasingBarrier(const std::shared_ptr<Resource>& beforeResource = nullptr, const std::shared_ptr<Resource>& afterResource = nullptr, bool flushBarriers = false);
    void AliasingBarrier(Microsoft::WRL::ComPtr<ID3D12Resource> beforeResource, Microsoft::WRL::ComPtr<ID3D12Resource> afterResource, bool flushBarriers = false);
    
    //Flush any barriers that have been pushed to the command list
    void FlushResourceBarriers();

    void CopyResource(const std::shared_ptr<Resource>& dstRes, const std::shared_ptr<Resource>& srcRes);
    void CopyResource(Microsoft::WRL::ComPtr<ID3D12Resource> dstRes, Microsoft::WRL::ComPtr<ID3D12Resource> srcRes);

    //Resolve a multisampled resource into a non-multisampled resource
    void ResolveSubResource(const std::shared_ptr<Resource>& dstRes, const std::shared_ptr<Resource>& srcRes, uint32_t dstSubresource = 0, uint32_t srcSubresource = 0);

    //Copy the contents to a vertex buffer in GPU memory
    std::shared_ptr<VertexBuffer> CopyVertexBuffer(size_t numVertices, size_t vertexStride, const void* vertexBufferData);

    template<typename T>
    std::shared_ptr<VertexBuffer> CopyVertexBuffer(const std::vector<T>& vertexBufferData)
    {
        return CopyVertexBuffer(vertexBufferData.size(), sizeof(T), vertexBufferData.data());
    }

    //Copy the contents to an index buffer in GPU memory
    std::shared_ptr<IndexBuffer> CopyIndexBuffer(size_t numIndices, DXGI_FORMAT indexFormat, const void* indexBufferData);

    template<typename T>
    std::shared_ptr<IndexBuffer> CopyIndexBuffer(const std::vector<T>& indexBufferData)
    {
        assert(sizeof(T) == 2 || sizeof(T) == 4);

        DXGI_FORMAT indexFormat = (sizeof(T) == 2) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
        return CopyIndexBuffer(indexBufferData.size(), indexFormat, indexBufferData.data());
    }

    //Copy the contents to a constant buffer in GPU memory
    std::shared_ptr<ConstantBuffer> CopyConstantBuffer(size_t bufferSize, const void* bufferData);

    template<typename T>
    std::shared_ptr<ConstantBuffer> CopyConstantBuffer(const T& data)
    {
        return CopyConstantBuffer(sizeof(T), &data);
    }

    //Copy the contents to a byte address buffer in GPU memory
    std::shared_ptr<ByteAddressBuffer> CopyByteAddressBuffer(size_t bufferSize, const void* bufferData);

    template<typename T>
    void CopyByteAddressBuffer(const T& data)
    {
        CopyByteAddressBuffer(sizeof(T), &data);
    }

    //Copy the contents to a structured buffer in GPU memory
    std::shared_ptr<StructuredBuffer> CopyStructuredBuffer(size_t numElements, size_t elementSize, const void* bufferData);

    template <typename T>
    std::shared_ptr<StructuredBuffer> CopyStructuredBuffer(const std::vector<T>& bufferData)
    {
        CopyStructuredBuffer(bufferData.size(), sizeof(T), bufferData.data());
    }

    //Set the current primitive topology for the rendering pipeline
    void SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY primitiveTopology);

    //Load texture by a filename
    std::shared_ptr<Texture> LoadTextureFromFile(const std::wstring& fileName, bool sRGB = false);

    //Load a scene file
    //@param fileName the path to the scene file definition
    //@param [LoadingProgress] an optional callback function that can be used to report loading progress
    std::shared_ptr<Scene> LoadSceneFromFile(const std::wstring& fileName, const std::function<bool(float)>& loadingProgress = std::function<bool(float)>());

    //Load a scene from a string
    //@param sceneString is the scene file definition as a string
    //@param format is the format of the scene file, such as NFF
    std::shared_ptr<Scene> LoadSceneFromString(const std::string& sceneString, const std::string& format);

    //Create a cube
    //@param size is the Size of one side of the cube
    //@param reverseWinding whether to reverse the winding order of the triangles (for skyboxes)
    std::shared_ptr<Scene> CreateCube(float size = 1.0f, bool reverseWinding = false);

    //Create a sphere
    //@param radius is the Radius of the sphere
    //@param reverseWinding is whether to reverse the winding order of the triangles (for skydomes)
    std::shared_ptr<Scene> CreateSphere(float radius = 0.5f, uint32_t tessellation = 16, bool reverseWinding = false);

    //Create a cylinder
    //@param radius is the radius of the primary axis of the cylinder
    //@param height is the height of the cylinder
    //@param tessellation How smooth the cylinder will be
    //@param reverseWinding is whether to reverse the winding order of the triangles
    std::shared_ptr<Scene> CreateCylinder(float radius = 0.5f, float height = 1.0f, uint32_t tessellation = 32, bool reverseWinding = false);

    //Create a cone
    std::shared_ptr<Scene> CreateCone(float radius = 0.5f, float height = 1.0f, uint32_t tessellation = 32, bool reverseWinding = false);

    //Create a torus
    std::shared_ptr<Scene> CreateTorus(float radius = 0.5f, float thickness = 0.333f, uint32_t tessellation = 32, bool reverseWinding = false);

    //Create a plane
    std::shared_ptr<Scene> CreatePlane(float width = 1.0f, float height = 1.0f, bool reverseWinding = false);

    //Clear a texture
    void ClearTexture(const std::shared_ptr<Texture>& texture, const float clearColor[4]);

    //Clear depth/stencil texture
    void ClearDepthStencilTexture(const std::shared_ptr<Texture>& texture, D3D12_CLEAR_FLAGS clearFlags, float depth = 1.0f, uint8_t stencil = 0);

    //Generate mips for the texture.
    //The first subresource is used to generate the mip chain
    //Mips are automatically generated for textures loaded from files
    void GenerateMips(const std::shared_ptr<Texture>& texture);

    //Generate a cubemap texture from a panoramic (equirectangular) texture
    void PanoToCubemap(const std::shared_ptr<Texture>& cubemap, const std::shared_ptr<Texture>& pano);

    //Copy subresource data to a texture
    void CopyTextureSubresource(const std::shared_ptr<Texture>& texture, uint32_t firstSubresource, uint32_t numSubresources, D3D12_SUBRESOURCE_DATA* subresourceData);

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
    void SetVertexBuffers(uint32_t startSlot, const std::vector<std::shared_ptr<VertexBuffer>>& vertexBufferViews);
    void SetVertexBuffer(uint32_t slot, const std::shared_ptr<VertexBuffer>& vertexBufferView);

    //Set dynamic vertex buffer data to the rendering pipeline
    void SetDynamicVertexBuffer(uint32_t slot, size_t numVertices, size_t vertexSize, const void* vertexBufferData);
    template<typename T>
    void SetDynamicVertexBuffer(uint32_t slot, const std::vector<T>& vertexBufferData)
    {
        SetDynamicVertexBuffer(slot, vertexBufferData.size(), sizeof(T), vertexBufferData.data());
    }

    //Bind the index buffer to the rendering pipeline
    void SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer);

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
    void SetPipelineState(const std::shared_ptr<PipelineStateObject>& pipelineState);

    //Set the current root signature on the command list
    void SetGraphicsRootSignature(const std::shared_ptr<RootSignature>& rootSignature);
    void SetComputeRootSignature(const std::shared_ptr<RootSignature>& rootSignature);

    //Set an inline CBV (only Constant Buffers can be used with inline CBV's
    void SetConstantBufferView(uint32_t rootParameterIndex, const std::shared_ptr<ConstantBuffer>& buffer,
        D3D12_RESOURCE_STATES stateAfter = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
        size_t bufferOffset = 0);

    //Set an inline SRV (only buffer resources can be used with inline SRV's)
    void SetShaderResourceView(uint32_t rootParameterIndex, const std::shared_ptr<Buffer>& buffer,
        D3D12_RESOURCE_STATES stateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
        size_t bufferOffset = 0);

    //Set an inline UAV (only buffer resources can be used with inline UAV's
    void SetUnorderedAccessView(uint32_t rootParameterIndex, const std::shared_ptr<Buffer>& buffer, D3D12_RESOURCE_STATES stateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        size_t bufferOffset = 0);

    //Set  the CBV on the rendering pipeline
    void SetConstantBufferView(uint32_t rootParameterIndex, uint32_t descriptorOffset, const std::shared_ptr<ConstantBufferView>& cbv,
        D3D12_RESOURCE_STATES stateAfter = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

    ////Set the SRV on the graphics pipeline
    void SetShaderResourceView(uint32_t rootParameterIndex,
        uint32_t descriptorOffset,
        const std::shared_ptr<ShaderResourceView>& srv,
        D3D12_RESOURCE_STATES stateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
        UINT firstSubresource = 0,
        UINT numSubresources = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

    ////Set an SRV on the graphics pipeline using the default SRV for the texture
    void SetShaderResourceView(int32_t rootParameterIndex, uint32_t descriptorOffset, const std::shared_ptr<Texture>& texture,
        D3D12_RESOURCE_STATES stateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
        UINT firstSubresource = 0,
        UINT numSubresources = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

    ////Set the UAV on the graphics pipeline
    void SetUnorderedAccessView(uint32_t rootParameterIndex,
        uint32_t descriptorOffset,
        const std::shared_ptr<UnorderedAccessView>& uav,
        D3D12_RESOURCE_STATES stateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        UINT firstSubresource = 0,
        UINT numSubresources = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

    ////Set the UAV on the graphics pipeline using a specific mip of the texture
    void SetUnorderedAccessView(uint32_t rootParameterIndex, uint32_t descriptorOffset, const std::shared_ptr<Texture>& texture, UINT mip,
        D3D12_RESOURCE_STATES stateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS, UINT firstSubresource = 0,
        UINT numSubresources = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

    //Set the render targets for the graphics rendering pipeline
    void SetRenderTarget(const RenderTarget& renderTarget);

    //Draw geometry
    void Draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t startVertex = 0, uint32_t startInstance = 0);
    void DrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1, uint32_t startIndex = 0, int32_t baseVertex = 0, uint32_t startInstance = 0);

    //Dispatch a compute shader
    void Dispatch(uint32_t numGroupsX, uint32_t numGroupsY = 1, uint32_t numGroupsZ = 1);

    /// <summary>
    /// The following methods are only meant to be used by internal classes
    /// </summary>
protected:
    friend class CommandQueue;
    friend class DynamicDescriptorHeap;
    friend class std::default_delete<CommandList>;

    CommandList(Device& device, D3D12_COMMAND_LIST_TYPE type);
    virtual ~CommandList();

    //Close the command list, used by the command queue
    //The param is the command list that is used to execute pending resource barriers(if any) for this command list
    //returns true if there's any pending resource barriers that need to be processed
    bool Close(const std::shared_ptr<CommandList>& pendingCommandList);

    //Just close the command list, useful for pending command lists
    void Close();

    //Reset the command list, should only be called by the CommandQueue before the list is returned from GetCommandList
    void Reset();

    //Release tracked objects, useful if the swap chain needs to be resized
    void ReleaseTrackedObjects();

    //Set the currently bound descriptor heap
    //Should only be called by the DynamicDescriptorHeap class
    void SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, ID3D12DescriptorHeap* heap);

    std::shared_ptr<CommandList> GetGenerateMipsCommandList() const
    {
        return m_ComputeCommandList;
    }

private:
    //For procedural mesh generation
    using VertexCollection = std::vector<VertexPositionNormalTangentBitangentTexture>;
    using IndexCollection = std::vector<uint16_t>;

    //Create a scene that contains a single node with a single mesh
    std::shared_ptr<Scene> CreateScene(const VertexCollection& vertices, const IndexCollection& indices);

    //Helper function for flipping winding of geometric primitives for LH vs RH coords
    inline void ReverseWinding(IndexCollection& indices, VertexCollection& vertices);
    //Helper function for inverting normals for "inside" vs "outside" viewing
    inline void InvertNormals(VertexCollection& vertices);
    //Helper function to compute a point on a unit circle aligned to the x,z plane and centered at the origin
    inline DirectX::XMVECTOR GetCircleVector(size_t i, size_t tessellation) noexcept;
    //Helper function to compute a tangent vector at a point on a unit sphere aligned to the x,z plane
    inline DirectX::XMVECTOR GetCircleTangent(size_t i, size_t tessellation) noexcept;
    //Helper creates a triangle fan to close the end of a cylinder/cone
    void CreateCylinderCap(VertexCollection& vertices, IndexCollection& indices, size_t tessellation, float height, float radius, bool isTop);

    //Add a resource to a list of tracked resources (ensures lifetime while command list is in-flight on a command queue
    void TrackResource(Microsoft::WRL::ComPtr<ID3D12Object> object);
    void TrackResource(const std::shared_ptr<Resource>& res);

    //Generate mips for UAV compatible textures
    void GenerateMips_UAV(const std::shared_ptr<Texture>& texture, bool isSRGB);
    ////Generate mips for BGR textures
    //void GenerateMips_BGR(Texture& texture);
    ////Generate mips for sRGB textures
    //void GenerateMips_sRGB(Texture& texture);

    //Copy the contents of a CPU buffer to a GPU buffer (possibly replacing the previous buffer contents)
    Microsoft::WRL::ComPtr<ID3D12Resource> CopyBuffer(size_t bufferSize, const void* bufferData, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);

    //Binds the current descriptor heaps to the command list
    void BindDescriptorHeaps();

    Device& m_Device;
    D3D12_COMMAND_LIST_TYPE m_d3d12CommandListType;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> m_d3d12CommandList;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_d3d12CommandAllocator;

    //For copy queues, it may be necessary to generate mips while loading textures
    //Mips can't be generated on copy queues but must be generated on compute or direct queues
    //In this case, a Compute command list is generated and executed after the copy queue is finished uploading the first subresource
    std::shared_ptr<CommandList> m_ComputeCommandList;

    //Keep track of the currently bound root signatures to minimize root signature changes
    ID3D12RootSignature* m_RootSignature;

    //Keep track of the currently bound pipeline state object to minimize PSO changes
    ID3D12PipelineState* m_PipelineState;

    //Resource Created in an upload heap. useful for drawing dynamic geometry
    //or for uploading constant buffer data that changes every draw call
    std::unique_ptr<UploadBuffer> m_UploadBuffer;

    //Resource state tracker used by the command list to track(per list) the current state of a resource
    //Also tracks the global state of a resource in order to minimize resource state transitions
    std::unique_ptr<ResourceStateTracker> m_ResourceStateTracker;

    //Dynamic descriptor heap allows for descriptors to be staged before being committed to the command list.
    //Dynamic descriptors need to be committed before a Draw or Dispatch
    std::unique_ptr<DynamicDescriptorHeap> m_DynamicDescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

    //Keep track of the currently bound descriptor heaps. Only change descriptor heaps if they are different than the currently bound descriptor heaps
    ID3D12DescriptorHeap* m_DescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

    //Pipeline state object for Mipmap generation
    std::unique_ptr<GenerateMipsPSO> m_GenerateMipsPSO;
    //Pipeline state object for converting panorama(equirectangular) to cubemaps
    std::unique_ptr<PanoToCubemapPSO> m_PanoToCubemapPSO;

    //Objects that are being tracked by a command list that is "in-flight" on the command queue and cannot be deleted
    //To ensure objects are not deleted until the command list is finished executing, a reference to the object is stored.
    //The referenced objects are released when the command list is reset
    using TrackedObjects = std::vector <Microsoft::WRL::ComPtr<ID3D12Object> >;

    TrackedObjects m_TrackedObjects;

    //Keep track of loaded textures to avoid loading the same texture multiple times
    static std::map<std::wstring, ID3D12Resource*> ms_TextureCache;
    static std::mutex ms_TextureCacheMutex;
};

//Definition for inline function
inline DirectX::XMVECTOR CommandList::GetCircleVector(size_t i, size_t tessellation) noexcept
{
    float angle = float(i) * DirectX::XM_2PI / float(tessellation);
    float dx, dz;

    DirectX::XMScalarSinCos(&dx, &dz, angle);

    DirectX::XMVECTORF32 v = { {{dx, 0, dz, 0}} };
    return v;
}

inline DirectX::XMVECTOR CommandList::GetCircleTangent(size_t i, size_t tessellation) noexcept
{
    float angle = (float(i) * DirectX::XM_2PI / float(tessellation)) + DirectX::XM_PIDIV2;
    float dx, dz;

    DirectX::XMScalarSinCos(&dx, &dz, angle);

    DirectX::XMVECTORF32 v = { {{dx, 0, dz, 0}} };
    return v;
}

inline void CommandList::ReverseWinding(IndexCollection& indices, VertexCollection& vertices)
{
    assert((indices.size() % 3) == 0);
    for (auto it = indices.begin(); it != indices.end(); it += 3)
    {
        std::swap(*it, *(it + 2));
    }

    for (auto it = vertices.begin(); it != vertices.end(); ++it)
    {
        it->TexCoord.x = (1.f - it->TexCoord.x);
    }
}

inline void CommandList::InvertNormals(VertexCollection& vertices)
{
    for (auto it = vertices.begin(); it != vertices.end(); ++it)
    {
        it->Normal.x = -it->Normal.x;
        it->Normal.y = -it->Normal.y;
        it->Normal.z = -it->Normal.z;
    }
}