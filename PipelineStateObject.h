#pragma once

#include "DirectX-Headers/include/directx/d3d12.h"

#include <wrl/client.h>

class Device;

class PipelineStateObject
{
public:
	Microsoft::WRL::ComPtr<ID3D12PipelineState> GetD3D12PipelineState() const
	{
		return m_d3d12PipelineState;
	}

protected:
	PipelineStateObject(Device& device, const D3D12_PIPELINE_STATE_STREAM_DESC& desc);
	virtual ~PipelineStateObject() = default;

private:
	Device& m_Device;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_d3d12PipelineState;
};