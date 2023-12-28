#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <shellapi.h>
#include <Shlwapi.h>

#include "Application.h"
#include "Demo.h"

#pragma comment(lib, "dxgi")
#pragma comment(lib, "DXGUID")

#include <dxgidebug.h>

void ReportLiveObjects()
{
	IDXGIDebug1* dxgiDebug;
	DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug));

	dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_IGNORE_INTERNAL);
	dxgiDebug->Release();
}

int CALLBACK wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
	int retCode = 0;

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
		std::shared_ptr<Demo> demo = std::make_shared<Demo>(L"Learning DirectX12", 1280, 720);
		retCode = Application::Get().Run(demo);
	}
	Application::Destroy();

	atexit(&ReportLiveObjects);

	return retCode;
}