#pragma once

#include "EffectPSO.h"

#include "Light.h"

#include <DirectXMath.h>

#include <memory>
#include <vector>

class CommandList;
class Device;
class Material;
class RootSignature;
class PipelineStateObject;
class ShaderResourceView;
class Texture;

class EffectPSO
{
public:
	//light properties for the pixel shader
	struct LightProperties
	{
		uint32_t NumPointLights;
		uint32_t NumSpotLights;
		uint32_t NumDirectionalLights;
	};

	//Transformation matrices for the vertex shader
	struct alignas(16) Matrices
	{
		DirectX::XMMATRIX ModelMatrix;
		DirectX::XMMATRIX ModelViewMatrix;
		DirectX::XMMATRIX InverseTransposeModelViewMatrix;
		DirectX::XMMATRIX ModelViewProjectionMatrix;
	};

	//An enum for root signature parameters
	enum RootParameters
	{
		// Vertex shader parameter
		MatricesCB,  // ConstantBuffer<Matrices> MatCB : register(b0);

		// Pixel shader parameters
		MaterialCB,         // ConstantBuffer<Material> MaterialCB : register( b0, space1 );
		LightPropertiesCB,  // ConstantBuffer<LightProperties> LightPropertiesCB : register( b1 );

		PointLights,        // StructuredBuffer<PointLight> PointLights : register( t0 );
		SpotLights,         // StructuredBuffer<SpotLight> SpotLights : register( t1 );
		DirectionalLights,  // StructuredBuffer<DirectionalLight> DirectionalLights : register( t2 )

		Textures,  // Texture2D AmbientTexture       : register( t3 );
		// Texture2D EmissiveTexture : register( t4 );
		// Texture2D DiffuseTexture : register( t5 );
		// Texture2D SpecularTexture : register( t6 );
		// Texture2D SpecularPowerTexture : register( t7 );
		// Texture2D NormalTexture : register( t8 );
		// Texture2D BumpTexture : register( t9 );
		// Texture2D OpacityTexture : register( t10 );
		NumRootParameters
	};

	EffectPSO(std::shared_ptr<Device> device, bool enableLighting, bool enableDecal);
	virtual ~EffectPSO();

	const std::vector<PointLight>& GetPointLights() const
	{
		return m_PointLights;
	}

	void SetPointLights(const std::vector<PointLight>& pointLights)
	{
		m_PointLights = pointLights;
		m_DirtyFlags |= DF_PointLights;
	}

	const std::vector<SpotLight>& GetSpotLights() const
	{
		return m_SpotLights;
	}
	void SetSpotLights(const std::vector<SpotLight>& spotLights)
	{
		m_SpotLights = spotLights;
		m_DirtyFlags |= DF_SpotLights;
	}

	const std::vector<DirectionalLight>& GetDirectionalLights() const
	{
		return m_DirectionalLights;
	}
	void SetDirectionalLights(const std::vector<DirectionalLight>& directionalLights)
	{
		m_DirectionalLights = directionalLights;
		m_DirtyFlags |= DF_DirectionalLights;
	}

	const std::shared_ptr<Material>& GetMaterial() const
	{
		return m_Material;
	}
	void SetMaterial(const std::shared_ptr<Material>& material)
	{
		m_Material = material;
		m_DirtyFlags |= DF_Material;
	}

	// Set matrices.
	void XM_CALLCONV SetWorldMatrix(DirectX::FXMMATRIX worldMatrix)
	{
		m_pAlignedMVP->World = worldMatrix;
		m_DirtyFlags |= DF_Matrices;
	}
	DirectX::XMMATRIX GetWorldMatrix() const
	{
		return m_pAlignedMVP->World;
	}

	void XM_CALLCONV SetViewMatrix(DirectX::FXMMATRIX viewMatrix)
	{
		m_pAlignedMVP->View = viewMatrix;
		m_DirtyFlags |= DF_Matrices;
	}
	DirectX::XMMATRIX GetViewMatrix() const
	{
		return m_pAlignedMVP->View;
	}

	void XM_CALLCONV SetProjectionMatrix(DirectX::FXMMATRIX projectionMatrix)
	{
		m_pAlignedMVP->Projection = projectionMatrix;
		m_DirtyFlags |= DF_Matrices;
	}
	DirectX::XMMATRIX GetProjectionMatrix() const
	{
		return m_pAlignedMVP->Projection;
	}

	// Apply this effect to the rendering pipeline.
	void Apply(CommandList& commandList);

private:
	enum DirtyFlags
	{
		DF_None = 0,
		DF_PointLights = (1 << 0),
		DF_SpotLights = (1 << 1),
		DF_DirectionalLights = (1 << 2),
		DF_Material = (1 << 3),
		DF_Matrices = (1 << 4),
		DF_All = DF_PointLights | DF_SpotLights | DF_DirectionalLights | DF_Material | DF_Matrices
	};

	struct alignas(16) MVP
	{
		DirectX::XMMATRIX World;
		DirectX::XMMATRIX View;
		DirectX::XMMATRIX Projection;
	};

	//Helper function to bind a texture to the rendering pipeline
	inline void Bindtexture(CommandList& commandList, uint32_t offset, const std::shared_ptr<Texture>& texture);

	std::shared_ptr<Device> m_Device;
	std::shared_ptr<RootSignature> m_RootSignature;
	std::shared_ptr<PipelineStateObject> m_PipelineStateObject;

	std::vector<PointLight> m_PointLights;
	std::vector<SpotLight> m_SpotLights;
	std::vector<DirectionalLight> m_DirectionalLights;

	//Material to apply during rendering
	std::shared_ptr<Material> m_Material;

	//An SRV used to pad unused texture slots
	std::shared_ptr<ShaderResourceView> m_DefaultSRV;

	//Matrices
	MVP* m_pAlignedMVP;
	//If commandlist changes, all parameteres need to be rebound
	CommandList* m_pPreviousCommandList;

	//Which properties need to be bound
	uint32_t m_DirtyFlags;

	bool m_EnableLighting;
	bool m_EnableDecal;
};