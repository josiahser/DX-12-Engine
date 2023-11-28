#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Shlwapi.h>

#include "Application.h"
#include "Demo.h"

#include <dxgidebug.h>

void ReportLiveObjects()
{
	IDXGIDebug1* dxgiDebug{};
	DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug));

	dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_IGNORE_INTERNAL);
	dxgiDebug->Release();
}

int CALLBACK wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
	int retCode = 0;

	Application::Create(hInstance);
	{
		std::shared_ptr<Demo> demo = std::make_shared<Demo>(L"Learning DirectX12", 1280, 720);
		retCode = Application::Get().Run(demo);
	}
	Application::Destroy();

	return retCode;
}

//TODO: Add some helper classes

	//Upload buffer (linear allocator that creates resources in an Upload Heap)
	//Used for dynamic buffer data

	//DescriptorAllocator (Used to allocate a number of CPU visible descriptors, for RTV and DSV or other views)

	//DynamicDescriptorHeap ensures that all of the GPU visible descriptors are copied to a single GPU visible heap before draw is executed on GPU

	//ResourceStateTracker to track the before state of a resource across multiple threads, used when transitioning resources

	//CommandList to bring it all together and simplify using it, abstracts away a lot of the DX12 specific code

//TODO: Organize it all together

//TODO: Add more/make shaders more complex