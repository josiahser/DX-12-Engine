#pragma once

#include <cstdint>
#include <vector>

#include "Texture.h"

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
	void AttachTexture(AttachmentPoint attachmentPoint, const Texture& texture);
	const Texture& GetTexture(AttachmentPoint attachmentPoint) const;

	//Resize all the textures associated with the render target
	void Resize(uint32_t width, uint32_t height);

	//Get a list of textures attached to the render target
	//Primarily used by the commandlist when binding render target to the Output merger stage
	const std::vector<Texture>& GetTextures() const;

	//Get the Render Target Formats of the textures currently attached to this render target object
	//Needed to configure the Pipeline State Object (PSO)
	D3D12_RT_FORMAT_ARRAY GetRenderTargetFormats() const;

	//Get the format of the attached depth/stencil buffer
	DXGI_FORMAT GetDepthStencilFormat() const;

protected:

private:
	std::vector<Texture> m_Textures;
};