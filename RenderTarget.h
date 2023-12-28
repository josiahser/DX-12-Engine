#pragma once

#include <DirectXMath.h>
#include "DirectX-Headers/include/directx/d3d12.h"

#include <cstdint>
#include <memory>
#include <vector>

class Texture;
//Don't use scoped enums in order to avoid the explicit cast required to use them as array indices
enum AttachmentPoint
{
	Color0,
	Color1,
	Color2,
	Color3,
	Color4,
	Color5,
	Color6,
	Color7,
	DepthStencil,
	NumAttachmentPoints
};

class RenderTarget
{
public:
	//Create an empty render target
	RenderTarget();

	RenderTarget(const RenderTarget& copy) = default;
	RenderTarget(RenderTarget&& copy) = default;

	RenderTarget& operator=(const RenderTarget& other) = default;
	RenderTarget& operator=(RenderTarget&& other) = default;

	//Attach a texture to the render target
	//The texture will be copied into the texture array
	void AttachTexture(AttachmentPoint attachmentPoint, std::shared_ptr<Texture> texture);
	std::shared_ptr<Texture> GetTexture(AttachmentPoint attachmentPoint) const;

	//Resize all the textures associated with the render target
	void Resize(DirectX::XMUINT2 size);
	void Resize(uint32_t width, uint32_t height);
	DirectX::XMUINT2 GetSize() const;
	uint32_t GetWidth() const;
	uint32_t GetHeight() const;
	
	//Get a viewport for this render target. The scale and bias parameters can be used to specific a splitscreen
	//(the bias parameter is normalized in the range [0...1]) by default, a fullscreen viewport is returned
	D3D12_VIEWPORT GetViewport(DirectX::XMFLOAT2 scale = { 1.0f, 1.0f }, DirectX::XMFLOAT2 bias = { 0.0f, 0.0f }, float minDepth = 0.0f, float maxDepth = 1.0f) const;

	//Get a list of textures attached to the render target
	//Primarily used by the commandlist when binding render target to the Output merger stage
	const std::vector<std::shared_ptr<Texture>>& GetTextures() const;

	//Get the Render Target Formats of the textures currently attached to this render target object
	//Needed to configure the Pipeline State Object (PSO)
	D3D12_RT_FORMAT_ARRAY GetRenderTargetFormats() const;

	//Get the format of the attached depth/stencil buffer
	DXGI_FORMAT GetDepthStencilFormat() const;

	//Get the sample description of the render target
	DXGI_SAMPLE_DESC GetSampleDesc() const;

	//Reset all textures
	void Reset()
	{
		m_Textures = RenderTargetList(AttachmentPoint::NumAttachmentPoints);
	}

protected:

private:
	using RenderTargetList = std::vector<std::shared_ptr<Texture>>;
	RenderTargetList m_Textures;
	DirectX::XMUINT2 m_Size;
};