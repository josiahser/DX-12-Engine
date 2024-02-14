#include "EffectPSO.h"

#include "CommandList.h"
#include "Device.h"
#include "Helpers.h"
#include "Material.h"
#include "PipelineStateObject.h"
#include "RootSignature.h"
#include "VertexTypes.h"

#include <d3dcompiler.h>
#include "d3dx12.h"
#include <wrl/client.h>

using namespace Microsoft::WRL;

EffectPSO::EffectPSO(std::shared_ptr<Device> device, bool enableLighting, bool enableDecal)
	: m_Device(device)
	, m_DirtyFlags(DF_All)
	, m_pPreviousCommandList(nullptr)
	, m_EnableLighting(enableLighting)
	, m_EnableDecal(enableDecal)
{
	m_pAlignedMVP = (MVP*)_aligned_malloc(sizeof(MVP), 16);

	//Setup the root signature
	//Load the vertex shader
	ComPtr<ID3DBlob> vertexShaderBlob;
	ThrowIfFailed(D3DReadFileToBlob(L"VertexShader.cso", &vertexShaderBlob));

	//Load the pixel shader
	ComPtr<ID3DBlob> pixelShaderBlob;
	if (enableLighting)
	{
		if (enableDecal)
		{
			ThrowIfFailed(D3DReadFileToBlob(L"Decal_PS.cso", &pixelShaderBlob));
		}
		else
		{
			ThrowIfFailed(D3DReadFileToBlob(L"Lighting_PS.cso", &pixelShaderBlob));
		}
	}
	else
	{
		ThrowIfFailed(D3DReadFileToBlob(L"Unlit_PS.cso", &pixelShaderBlob));
	}

	//Create a root signature
	//Allow input layout and deny unnecessary access
	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | 
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

	//Descriptor  range for the texture
	CD3DX12_DESCRIPTOR_RANGE1 descriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 8, 3);

	//clang-format off
	CD3DX12_ROOT_PARAMETER1 rootParameters[RootParameters::NumRootParameters];
	rootParameters[RootParameters::MatricesCB].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_VERTEX);
	rootParameters[RootParameters::MaterialCB].InitAsConstantBufferView(0, 1, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameters[RootParameters::LightPropertiesCB].InitAsConstants(sizeof(LightProperties) / 4, 1, 0, D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameters[RootParameters::PointLights].InitAsShaderResourceView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameters[RootParameters::SpotLights].InitAsShaderResourceView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameters[RootParameters::DirectionalLights].InitAsShaderResourceView(2, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameters[RootParameters::Textures].InitAsDescriptorTable(1, &descriptorRange, D3D12_SHADER_VISIBILITY_PIXEL);

	CD3DX12_STATIC_SAMPLER_DESC anisotropicSampler(0, D3D12_FILTER_ANISOTROPIC);

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init_1_1(RootParameters::NumRootParameters, rootParameters, 1, &anisotropicSampler, rootSignatureFlags);
	//clang-format on

	m_RootSignature = m_Device->CreateRootSignature(rootSignatureDesc.Desc_1_1);

	//Setup the pipeline state
	struct PipelineStateStream
	{
		CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE        pRootSignature;
		CD3DX12_PIPELINE_STATE_STREAM_VS                    VS;
		CD3DX12_PIPELINE_STATE_STREAM_PS                    PS;
		CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER            RasterizerState;
		CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT          InputLayout;
		CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY    PrimitiveTopologyType;
		CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT  DSVFormat;
		CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
		CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_DESC           SampleDesc;
	} pipelineStateStream;

	//Create a color buffer with sRGB for gamma correction
	DXGI_FORMAT backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	DXGI_FORMAT depthBufferFormat = DXGI_FORMAT_D32_FLOAT;

	//Check the best multisample quality level that can be used for the given back buffer format
	DXGI_SAMPLE_DESC sampleDesc = m_Device->GetMultisampleQualityLevels(backBufferFormat);

	D3D12_RT_FORMAT_ARRAY rtvFormats = {};
	rtvFormats.NumRenderTargets = 1;
	rtvFormats.RTFormats[0] = backBufferFormat;

	CD3DX12_RASTERIZER_DESC rasterizerState(D3D12_DEFAULT);
	if (m_EnableDecal)
	{
		//Disable backface culling on decal geometry
		rasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	}

	pipelineStateStream.pRootSignature = m_RootSignature->GetRootSignature().Get();
	pipelineStateStream.VS = CD3DX12_SHADER_BYTECODE(vertexShaderBlob.Get());
	pipelineStateStream.PS = CD3DX12_SHADER_BYTECODE(pixelShaderBlob.Get());
	pipelineStateStream.RasterizerState = rasterizerState;
	pipelineStateStream.InputLayout = VertexPositionNormalTangentBitangentTexture::InputLayout;
	pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pipelineStateStream.DSVFormat = depthBufferFormat;
	pipelineStateStream.RTVFormats = rtvFormats;
	pipelineStateStream.SampleDesc = sampleDesc;

	m_PipelineStateObject = m_Device->CreatePipelineStateObject(pipelineStateStream);

	//Create an SRV that can be used to pad unused texture slots
	D3D12_SHADER_RESOURCE_VIEW_DESC defaultSRV;
	defaultSRV.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	defaultSRV.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	defaultSRV.Texture2D.MostDetailedMip = 0;
	defaultSRV.Texture2D.MipLevels = 1;
	defaultSRV.Texture2D.PlaneSlice = 0;
	defaultSRV.Texture2D.ResourceMinLODClamp = 0;
	defaultSRV.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	m_DefaultSRV = m_Device->CreateShaderResourceView(nullptr, &defaultSRV);
}

EffectPSO::~EffectPSO()
{
	_aligned_free(m_pAlignedMVP);
}

inline void EffectPSO::Bindtexture(CommandList& commandList, uint32_t offset, const std::shared_ptr<Texture>& texture)
{
	if (texture)
	{
		commandList.SetShaderResourceView(RootParameters::Textures, offset, texture, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}
	else
	{
		commandList.SetShaderResourceView(RootParameters::Textures, offset, m_DefaultSRV, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}
}

void EffectPSO::Apply(CommandList& commandList)
{
	commandList.SetPipelineState(m_PipelineStateObject);
	commandList.SetGraphicsRootSignature(m_RootSignature);

	if (m_DirtyFlags & DF_Matrices)
	{
		Matrices m;
		m.ModelMatrix = m_pAlignedMVP->World;
		m.ModelViewMatrix = m_pAlignedMVP->World * m_pAlignedMVP->View;
		m.ModelViewProjectionMatrix = m.ModelViewMatrix * m_pAlignedMVP->Projection;
		m.InverseTransposeModelViewMatrix = XMMatrixTranspose(XMMatrixInverse(nullptr, m.ModelViewMatrix));

		commandList.SetGraphicsDynamicConstantBuffer(RootParameters::MatricesCB, m);
	}

	if (m_DirtyFlags & DF_Material)
	{
		if (m_Material)
		{
			const auto& materialProps = m_Material->GetMaterialProperties();

			commandList.SetGraphicsDynamicConstantBuffer(RootParameters::MaterialCB, materialProps);

			using TextureType = Material::TextureType;

			Bindtexture(commandList, 0, m_Material->GetTexture(TextureType::Ambient));
			Bindtexture(commandList, 1, m_Material->GetTexture(TextureType::Emissive));
			Bindtexture(commandList, 2, m_Material->GetTexture(TextureType::Diffuse));
			Bindtexture(commandList, 3, m_Material->GetTexture(TextureType::Specular));
			Bindtexture(commandList, 4, m_Material->GetTexture(TextureType::SpecularPower));
			Bindtexture(commandList, 5, m_Material->GetTexture(TextureType::Normal));
			Bindtexture(commandList, 6, m_Material->GetTexture(TextureType::Bump));
			Bindtexture(commandList, 7, m_Material->GetTexture(TextureType::Opacity));
		}
	}

	if (m_DirtyFlags & DF_PointLights)
	{
		commandList.SetGraphicsDynamicStructuredBuffer(RootParameters::PointLights, m_PointLights);
	}

	if (m_DirtyFlags & DF_SpotLights)
	{
		commandList.SetGraphicsDynamicStructuredBuffer(RootParameters::SpotLights, m_SpotLights);
	}

	if (m_DirtyFlags & DF_DirectionalLights)
	{
		commandList.SetGraphicsDynamicStructuredBuffer(RootParameters::DirectionalLights, m_DirectionalLights);
	}

	if (m_DirtyFlags & (DF_PointLights | DF_SpotLights | DF_DirectionalLights))
	{
		LightProperties lightProps;
		lightProps.NumPointLights = static_cast<uint32_t>(m_PointLights.size());
		lightProps.NumSpotLights = static_cast<uint32_t>(m_SpotLights.size());
		lightProps.NumDirectionalLights = static_cast<uint32_t>(m_DirectionalLights.size());

		commandList.SetGraphics32BitConstants(RootParameters::LightPropertiesCB, lightProps);
	}

	// Clear the dirty flags to avoid setting any states the next time the effect is applied.
	m_DirtyFlags = DF_None;
}