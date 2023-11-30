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

class CommandList
{
public:
    CommandList();
    virtual ~CommandList();

    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>GetGraphicsCommandList();

    void SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap);
};