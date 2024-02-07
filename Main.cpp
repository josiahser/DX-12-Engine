#define WIN32_LEAN_AND_MEAN
#include <Shlwapi.h>
#include <Windows.h>
#include <dxgi1_3.h>
#include <dxgidebug.h>
#include <shellapi.h>
#include <wrl/client.h>

#include "Helpers.h"

#include "Application.h"

#include "Demo.h"

void ReportLiveObjects()
{
	IDXGIDebug1* dxgiDebug;
	::DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug));

	dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_IGNORE_INTERNAL);
	dxgiDebug->Release();
}

int CALLBACK wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nShowCmd)
{
	int retCode = 0;

#if defined (_DEBUG)
	//Enable debug layer before doing anything DX12 related
	Microsoft::WRL::ComPtr<ID3D12Debug> debugInterface;
	ThrowIfFailed(::D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)));
	debugInterface->EnableDebugLayer();
#endif

	WCHAR path[MAX_PATH];

	int argc = 0;
	LPWSTR* argv = CommandLineToArgvW(lpCmdLine, &argc);
	if (argv)
	{
		for (int i = 0; i < argc; ++i)
		{
			//-wd specify the workind directory
			if (wcscmp(argv[i], L"-wd") == 0)
			{
				wcscpy_s(path, argv[++i]);
				SetCurrentDirectoryW(path);
			}
		}
		LocalFree(argv);
	}

	Application::Create(hInstance);
	{
		std::unique_ptr<Demo> demo = std::make_unique<Demo>(L"Learning DirectX12", 1280, 720);
		retCode = demo->Run();
	}

	Application::Destroy();

	atexit(&ReportLiveObjects);

	return retCode;
}