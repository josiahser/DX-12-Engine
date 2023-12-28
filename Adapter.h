#pragma once

#include "DirectX-Headers/include/directx/d3d12.h"
#include <dxgi1_6.h>
#include <wrl/client.h>

#include <memory>
#include <vector>

class Adapter;
using AdapterList = std::vector<std::shared_ptr<Adapter>>;

class Adapter
{
public:
	//Get a list of DX12 compatible hardware adapters sorted by GPU preference
	static AdapterList GetAdapters(DXGI_GPU_PREFERENCE gpuPreferencec = DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE);

	//Create a GPU adapter
	//@param gpuPreference The GPU preference, high performance by default
	//@param useWarp if true, use warp adapter
	//Returns a shared pointer to a GPU adapter or nullptr if the adapter couldn't be created
	static std::shared_ptr<Adapter> Create(DXGI_GPU_PREFERENCE gpuPreference = DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, bool useWarp = false);

	//Get the IDXGI Adapter
	Microsoft::WRL::ComPtr<IDXGIAdapter4> GetDXGIAdapter() const
	{
		return m_dxgiAdapter;
	}

	//Get description of the adapter
	const std::wstring GetDescription() const;

protected:
	Adapter(Microsoft::WRL::ComPtr<IDXGIAdapter4> dxgiAdapter);
	virtual ~Adapter() = default;

private:
	Microsoft::WRL::ComPtr<IDXGIAdapter4> m_dxgiAdapter;
	DXGI_ADAPTER_DESC3 m_Desc;
};