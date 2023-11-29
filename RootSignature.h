#pragma once

#include "DirectX-Headers/include/directx/d3dx12.h"

#include <wrl.h>
#include <vector>

class RootSignature
{
public:
	RootSignature();
	RootSignature(const D3D12_ROOT_SIGNATURE_DESC1& rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION rootSignatureVersion);

	virtual ~RootSignature();

	void Destroy();

	Microsoft::WRL::ComPtr<ID3D12RootSignature> GetRootSignature() const
	{
		return m_RootSignature;
	}

	void SetRootSignatureDesc(const D3D12_ROOT_SIGNATURE_DESC1& rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION rootSignatureVersion);

	const D3D12_ROOT_SIGNATURE_DESC1& GetRootSignatureDesc() const
	{
		return m_RootSignatureDesc;
	}

	uint32_t GetDescriptorTableBitMask(D3D12_DESCRIPTOR_HEAP_TYPE descriptorHeapType) const;
	uint32_t GetNumDescriptors(uint32_t rootIndex) const;

private:
	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_RootSignature;
	D3D12_ROOT_SIGNATURE_DESC1 m_RootSignatureDesc;

	//Need to know the # of descriptors per table, a max of 32 tables are supoorted
	//(since a 32-bit mask is used to represent the descriptor tables in the root signature)
	uint32_t m_NumDescriptorsPerTable[32];

	//A bit mask that represents the root parameter indices that are tables for Samplers
	uint32_t m_SamplerTableBitMask;

	//A bit mask that represents the root parameter indices that are CBV, UAV, SRV tables
	uint32_t m_DescriptorTableBitMask;
};