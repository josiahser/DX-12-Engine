#pragma once

#include "d3dx12.h"

#include <wrl.h>
#include <vector>

class Device;

class RootSignature
{
public:
	Microsoft::WRL::ComPtr<ID3D12RootSignature> GetRootSignature() const
	{
		return m_RootSignature;
	}

	const D3D12_ROOT_SIGNATURE_DESC1& GetRootSignatureDesc() const
	{
		return m_RootSignatureDesc;
	}

	uint32_t GetDescriptorTableBitMask(D3D12_DESCRIPTOR_HEAP_TYPE descriptorHeapType) const;
	uint32_t GetNumDescriptors(uint32_t rootIndex) const;

protected:
	friend class std::default_delete<RootSignature>;

	RootSignature(Device& device, const D3D12_ROOT_SIGNATURE_DESC1& rootSignatureDesc);

	virtual ~RootSignature();

private:
	void Destroy();

	void SetRootSignatureDesc(const D3D12_ROOT_SIGNATURE_DESC1& rootSignatureDesc);

	Device& m_Device;
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