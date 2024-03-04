#include "Demo.h"

#include "Device.h"

#include <shellapi.h>

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nShowCmd)
{
#if defined (_DEBUG)
	//Enable debug layer before doing anything DX12 related
	Device::EnableDebugLayer();
#endif

	WCHAR path[MAX_PATH];

	int argc = 0;
	LPWSTR* argv = ::CommandLineToArgvW(lpCmdLine, &argc);
	if (argv)
	{
		for (int i = 0; i < argc; ++i)
		{
			//-wd specify the workind directory
			if (::wcscmp(argv[i], L"-wd") == 0)
			{
				::wcscpy_s(path, argv[++i]);
				::SetCurrentDirectoryW(path);
			}
		}
		::LocalFree(argv);
	}

	int retCode = 0;

	Application::Create(hInstance);
	{
		auto demo = std::make_unique<Demo>(L"Learning DirectX12", 1280, 720);
		retCode = demo->Run();
	}

	Application::Destroy();

	::atexit(&Device::ReportLiveObjects);

	return retCode;
}