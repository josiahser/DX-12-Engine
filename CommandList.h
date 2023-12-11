#pragma once

#include <d3d12.h>
#include <wrl.h>

#include <map>
#include <memory>
#include <mutex>
#include <vector>

enum class TextureUsage
{
    Albedo,
    Diffuse = Albedo,       // Treat Diffuse and Albedo textures the same.
    Heightmap,
    Depth = Heightmap,      // Treat height and depth textures the same.
    Normalmap,
    RenderTarget,           // Texture is used as a render target.
};

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

    D3D12_COMMAND_LIST_TYPE GetCommandListType() const
    {
        return m_d3d12CommandListType;
    }

    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2>GetGraphicsCommandList() const
    {
        return m_d3d12CommandList;
    }

    void SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap);

private:
    D3D12_COMMAND_LIST_TYPE m_d3d12CommandListType;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> m_d3d12CommandList;
};