#pragma once

#include "DescriptorAllocation.h"

#include <cstdint>

class Device;
class PipelineStateObject;
class RootSignature;
//struct used in the panotocubemap_CS compute shader
struct PanoToCubemapCB
{
	//Size of the cubemap face in pixels at the current mipmap level
	uint32_t CubemapSize;
	//First mip level to generate
	uint32_t FirstMip;
	//The number of mips to generate
	uint32_t NumMips;
};

//Again, don't use scoped enums to avoid the explicit cast that is required in order to treat them as root indices in the root signature
namespace PanoToCubemapRS
{
	enum
	{
		PanoToCubemapCB,
		SrcTexture,
		DstMips,
		NumRootParameters
	};
}

class PanoToCubemapPSO
{
public:
	PanoToCubemapPSO(Device& device);

	std::shared_ptr<RootSignature> GetRootSignature() const
	{
		return m_RootSignature;
	}

	std::shared_ptr<PipelineStateObject> GetPipelineState() const
	{
		return m_PipelineState;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE GetDefaultUAV() const
	{
		return m_DefaultUAV.GetDescriptorHandle();
	}

private:
	std::shared_ptr<RootSignature> m_RootSignature;
	std::shared_ptr<PipelineStateObject> m_PipelineState;
	//Default (no resource) UAV's to pad the unused UAV descriptors
	//If generating less than 5 mip map levels, the unused need padding with default UAVs
	DescriptorAllocation m_DefaultUAV;
};