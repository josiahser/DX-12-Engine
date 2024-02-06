#pragma once

#include "DescriptorAllocation.h"
#include "Resource.h"
//#include "TextureUsage.h"
#include "d3dx12.h"

#include <mutex>
#include <unordered_map>

//class Texture : public Resource
//{
//public:
//    explicit Texture(TextureUsage textureUsage = TextureUsage::Albedo, const std::wstring& name = L"");
//    explicit Texture(const D3D12_RESOURCE_DESC& resourceDesc, const D3D12_CLEAR_VALUE* clearValue = nullptr, TextureUsage textureUsage = TextureUsage::Albedo, const std::wstring& name = L"");
//    explicit Texture(Microsoft::WRL::ComPtr<ID3D12Resource> resource, TextureUsage textureUsage = TextureUsage::Albedo, const std::wstring& name = L"");
//
//    Texture(const Texture& copy);
//    Texture(Texture&& copy);
//
//    Texture& operator=(const Texture& other);
//    Texture& operator=(Texture&& other);
//
//    virtual ~Texture();
//
//    TextureUsage GetTextureUsage() const
//    {
//        return m_TextureUsage;
//    }
//
//    void SetTextureUsage(TextureUsage textureUsage)
//    {
//        m_TextureUsage = textureUsage;
//    }
//
//    //Resize the texture
//    void Resize(uint32_t width, uint32_t height, uint32_t depthOrArraySize = 1);
//
//    //Create SRV and UAVs for the resource
//    virtual void CreateViews();
//
//    //Get the SRV for a resource
//    //@param dxgiFormat the required format of the resource.
//    //When accessing a depth-stencil buffer as a shader resource view, the format will be different
//    virtual D3D12_CPU_DESCRIPTOR_HANDLE GetShaderResourceView(const D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc = nullptr) const override;
//
//    //Get the UAV for a resource or subresource
//    virtual D3D12_CPU_DESCRIPTOR_HANDLE GetUnorderedAccessView(const D3D12_UNORDERED_ACCESS_VIEW_DESC* uavDesc = nullptr) const override;
//
//    //Get the RTV for the texture
//    virtual D3D12_CPU_DESCRIPTOR_HANDLE GetRenderTargetView() const;
//
//    //Get the DSV for the texture
//    virtual D3D12_CPU_DESCRIPTOR_HANDLE GetDepthStencilView() const;
//
//    static bool CheckSRVSupport(D3D12_FORMAT_SUPPORT1 formatSupport)
//    {
//        return ((formatSupport & D3D12_FORMAT_SUPPORT1_SHADER_SAMPLE) != 0 || (formatSupport & D3D12_FORMAT_SUPPORT1_SHADER_LOAD) != 0);
//    }
//
//    static bool CheckRTVSupport(D3D12_FORMAT_SUPPORT1 formatSupport)
//    {
//        return ((formatSupport & D3D12_FORMAT_SUPPORT1_RENDER_TARGET) != 0);
//    }
//
//    static bool CheckUAVSupport(D3D12_FORMAT_SUPPORT1 formatSupport)
//    {
//        return ((formatSupport & D3D12_FORMAT_SUPPORT1_TYPED_UNORDERED_ACCESS_VIEW) != 0);
//    }
//
//    static bool CheckDSVSupport(D3D12_FORMAT_SUPPORT1 formatSupport)
//    {
//        return ((formatSupport & D3D12_FORMAT_SUPPORT1_DEPTH_STENCIL) != 0);
//    }
//
//    static bool IsUAVCompatibleFormat(DXGI_FORMAT format);
//    static bool ISSRGBFormat(DXGI_FORMAT format);
//    static bool IsBGRFormat(DXGI_FORMAT format);
//    static bool IsDepthFormat(DXGI_FORMAT format);
//
//    //Return a typeless format from the given format
//    static DXGI_FORMAT GetTypelessFormat(DXGI_FORMAT format);
//    static DXGI_FORMAT GetUAVCompatibleFormat(DXGI_FORMAT format);
//
//protected:
//
//private:
//    DescriptorAllocation CreateShaderResourceView(const D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc) const;
//    DescriptorAllocation CreateUnorderedAccessView(const D3D12_UNORDERED_ACCESS_VIEW_DESC* uavDesc) const;
//
//    mutable std::unordered_map<size_t, DescriptorAllocation> m_ShaderResourceViews;
//    mutable std::unordered_map<size_t, DescriptorAllocation> m_UnorderedAccessViews;
//
//    mutable std::mutex m_ShaderResourceViewsMutex;
//    mutable std::mutex m_UnorderedAccessViewsMutex;
//
//    DescriptorAllocation m_RenderTargetView;
//    DescriptorAllocation m_DepthStencilView;
//
//    TextureUsage m_TextureUsage;
//};

class Device;

class Texture : public Resource
{
public:
	//Resize the texture
	void Resize(uint32_t width, uint32_t height, uint32_t depthOrArraySize = 1);

	//Get the RTV for the texture
	D3D12_CPU_DESCRIPTOR_HANDLE GetRenderTargetView() const;

	//Get the DSV for the texture
	D3D12_CPU_DESCRIPTOR_HANDLE GetDepthStencilView() const;

	//Get the SRV for the texture
	D3D12_CPU_DESCRIPTOR_HANDLE GetShaderResourceView() const;

	//Get the UAV for the texture at a specific mip level
	//Only supported for 1D and 2D textures
	D3D12_CPU_DESCRIPTOR_HANDLE GetUnorderedAccessView(uint32_t mip) const;

	bool CheckSRVSupport() const
	{
		return CheckFormatSupport(D3D12_FORMAT_SUPPORT1_SHADER_SAMPLE);
	}

	bool CheckRTVSupport() const
	{
		return CheckFormatSupport(D3D12_FORMAT_SUPPORT1_RENDER_TARGET);
	}

	bool CheckUAVSupport() const
	{
		return CheckFormatSupport(D3D12_FORMAT_SUPPORT1_TYPED_UNORDERED_ACCESS_VIEW) && CheckFormatSupport(D3D12_FORMAT_SUPPORT2_UAV_TYPED_LOAD) && CheckFormatSupport(D3D12_FORMAT_SUPPORT2_UAV_TYPED_STORE);
	}

	bool CheckDSVSupport() const
	{
		return CheckFormatSupport(D3D12_FORMAT_SUPPORT1_DEPTH_STENCIL);
	}

	//Check to see if the image format has an alpha channel
	bool HasAlpha() const;

	//Check to see the number of bits per pixel
	size_t BitsPerPixel() const;

	static bool IsUAVCompatibleFormat(DXGI_FORMAT format);
	static bool IsSRGBFormat(DXGI_FORMAT format);
	static bool IsBGRFormat(DXGI_FORMAT format);
	static bool IsDepthFormat(DXGI_FORMAT format);

	//Return a typeless format from the given format
	static DXGI_FORMAT GetTypelessFormat(DXGI_FORMAT format);
	
	//Return an sRGB format in the same format family
	static DXGI_FORMAT GetSRGBFormat(DXGI_FORMAT format);
	static DXGI_FORMAT GetUAVCompatibleFormat(DXGI_FORMAT format);

protected:
	Texture(Device& device, const D3D12_RESOURCE_DESC& resourceDesc, const D3D12_CLEAR_VALUE* clearValue = nullptr);
	Texture(Device& device, Microsoft::WRL::ComPtr<ID3D12Resource> resource, const D3D12_CLEAR_VALUE* clearValue = nullptr);
	virtual ~Texture();

	//Create SRV and UAVs for the resource
	void CreateViews();

private:
	DescriptorAllocation m_RenderTargetView;
	DescriptorAllocation m_DepthStencilView;
	DescriptorAllocation m_ShaderResourceView;
	DescriptorAllocation m_UnorderedAccessView;
};