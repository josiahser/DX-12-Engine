#include "framework.h"

#include "PanoToCubemapPSO.h"

//Compiled
//"PanoToCubemap_CS.h"

#include "Application.h"

#include "DirectX-Headers/include/directx/d3dx12.h"

PanoToCubemapPSO::PanoToCubemapPSO()
{
	auto device = Application::Get().GetDevice();

	D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
	featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
}