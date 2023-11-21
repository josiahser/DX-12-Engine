#include "framework.h"
#include "Application.h"
#include "Game.h"
#include "Window.h"
#include "CommandQueue.h"

constexpr wchar_t WINDOW_CLASS_NAME[] = L"DX12RenderWindowClass";

using WindowPtr = std::shared_ptr<Window>;
using WindowMap = std::map<HWND, WindowPtr>;
using WindowNameMap = std::map<std::wstring, WindowPtr>;

static Application* gs_pSingleton = nullptr;
static WindowMap gs_Windows{};
static WindowNameMap gs_WindowByName{};

static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

//Wrapper struct to allow shared pointers for the window class
struct MakeWindow : public Window
{
	MakeWindow(HWND hWnd, const std::wstring& windowName, int clientWidth, int clientHeight, bool vSync)
		: Window(hWnd, windowName, clientWidth, clientHeight, vSync)
	{}
};

Application::Application(HINSTANCE hInstance)
	: m_hInstance(hInstance),
	m_TearingSupported(false)
{
	// Per monitor V2 DPI awareness context, added in Windows 10
	SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

#if defined(_DEBUG)
	//Always enable the debug layer before doing anything DX12 related
	Microsoft::WRL::ComPtr<ID3D12Debug> debugInterface{};
	ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)));
	debugInterface->EnableDebugLayer();
#endif

    WNDCLASSEXW wndClass = { 0 };

    wndClass.cbSize = sizeof(WNDCLASSEX);
    wndClass.style = CS_HREDRAW | CS_VREDRAW;
    wndClass.lpfnWndProc = &WndProc;
    wndClass.hInstance = m_hInstance;
    wndClass.hIcon = ::LoadIcon(m_hInstance, nullptr);
    wndClass.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
    wndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wndClass.lpszMenuName = NULL;
    wndClass.lpszClassName = WINDOW_CLASS_NAME;
    wndClass.hIconSm = ::LoadIcon(m_hInstance, nullptr);

    if (!RegisterClassExW(&wndClass))
    {
        MessageBoxA(NULL, "unable to register the window class", "Error", MB_OK | MB_ICONERROR);
    }

    m_Adapter = GetAdapter(false);
    if (m_Adapter)
        m_Device = CreateDevice(m_Adapter);

    if (m_Device)
    {
        m_DirectCommandQueue = std::make_shared<CommandQueue>(m_Device, D3D12_COMMAND_LIST_TYPE_DIRECT);
        m_ComputeCommandQueue = std::make_shared<CommandQueue>(m_Device, D3D12_COMMAND_LIST_TYPE_COMPUTE);
        m_CopyCommandQueue = std::make_shared<CommandQueue>(m_Device, D3D12_COMMAND_LIST_TYPE_COPY);

        m_TearingSupported = CheckTearingSupport();
    }
}

void Application::Create(HINSTANCE hInstance)
{
    if (!gs_pSingleton)
        gs_pSingleton = new Application(hInstance);
}

Application& Application::Get()
{
    assert(gs_pSingleton);
    return *gs_pSingleton;
}

void Application::Destroy()
{
    if (gs_pSingleton)
    {
        assert(gs_Windows.empty() && gs_WindowByName.empty() && "All windows should be destroyed before destroying the application instance");

        delete gs_pSingleton;
        gs_pSingleton = nullptr;
    }
}

Application::~Application()
{
    Flush();
}

Microsoft::WRL::ComPtr<IDXGIAdapter4> Application::GetAdapter(bool useWarp)
{
    Microsoft::WRL::ComPtr<IDXGIFactory4> dxgiFactory{};    //Create factory, which is used to query available adapters (video card, interfaces with the hardware)
    UINT createFactoryFlags = 0;
#if defined(_DEBUG)
    createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

    ThrowIfFailed(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory)));

    Microsoft::WRL::ComPtr<IDXGIAdapter1> dxgiAdapter1;
    Microsoft::WRL::ComPtr<IDXGIAdapter4> dxgiAdapter4;

    if (useWarp)                    //If we use a warp adapter,
    {
        ThrowIfFailed(dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&dxgiAdapter1))); //We have to use dxgiadapter1 for the enumwarpadapter method, but our function returns adapter4
        ThrowIfFailed(dxgiAdapter1.As(&dxgiAdapter4));      //Basically a static_cast but for COM objects, so we can return the correct Interface
    }
    else                            //If we're not using a warp adapter, find a compatible adapter hardware
    {
        SIZE_T maxDedicatedVideoMemory = 0; //Initialize VRAM variable
        for (UINT i = 0; dxgiFactory->EnumAdapters1(i, &dxgiAdapter1) != DXGI_ERROR_NOT_FOUND; ++i) //Returns error if adapter index is greater than or equal to the number of available adapters
        {
            DXGI_ADAPTER_DESC1 dxgiAdapterDesc1{};      //Create description for adapter
            dxgiAdapter1->GetDesc1(&dxgiAdapterDesc1);

            //Check to see if the adapter can create a D3D12 device without actually creating it. (largest VRAM is favored)
            if ((dxgiAdapterDesc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 &&       //Checking for this flag is to make sure we don't use WARP (software adapter, software flag)
                SUCCEEDED(D3D12CreateDevice(dxgiAdapter1.Get(),
                    D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)) &&
                dxgiAdapterDesc1.DedicatedVideoMemory > maxDedicatedVideoMemory)
            {
                maxDedicatedVideoMemory = dxgiAdapterDesc1.DedicatedVideoMemory;
                ThrowIfFailed(dxgiAdapter1.As(&dxgiAdapter4));      //Again so we can return an adapter4
            }
        }
    }

    return dxgiAdapter4;
}

Microsoft::WRL::ComPtr<ID3D12Device2> Application::CreateDevice(Microsoft::WRL::ComPtr<IDXGIAdapter4> adapter)
{
    Microsoft::WRL::ComPtr<ID3D12Device2> d3d12Device2{};
    ThrowIfFailed(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&d3d12Device2)));
    //Enable debug messages in debug mode
#if defined(_DEBUG)
    Microsoft::WRL::ComPtr<ID3D12InfoQueue> pInfoQueue{};
    if (SUCCEEDED(d3d12Device2.As(&pInfoQueue)))
    {
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);
        //Suppress whole categories of messages
        //D3D12_MESSAGE_CATEGORY categories[] = {};

        //Suppress messages based on their severity level
        D3D12_MESSAGE_SEVERITY Severities[] = { D3D12_MESSAGE_SEVERITY_INFO };

        //Supress individual messages by their ID
        D3D12_MESSAGE_ID DenyIds[] = {
            D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,   //Occurs when render target is cleared using a clear color that isn't the optimized one
            D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,                         //Occurs when a frame is captured using the graphics debugger
            D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE                        //Same as above
        };

        D3D12_INFO_QUEUE_FILTER NewFilter = {};
        //NewFilter.DenyList.NumCategories = _countof(Categories);
        //NewFilter.DenyList.pCategoryList = Categories;
        NewFilter.DenyList.NumSeverities = _countof(Severities);
        NewFilter.DenyList.pSeverityList = Severities;
        NewFilter.DenyList.NumIDs = _countof(DenyIds);
        NewFilter.DenyList.pIDList = DenyIds;

        ThrowIfFailed(pInfoQueue->PushStorageFilter(&NewFilter));
    }
#endif

    return d3d12Device2;
}

bool Application::CheckTearingSupport()
{
    BOOL allowTearing = FALSE;

    //instead of directly making the dxgi 1.5 factory needed to have access to the ::CheckFeatureSupport method,
    //Create the DXGI 1.4 interface and query for 1.5.
    Microsoft::WRL::ComPtr<IDXGIFactory4> factory4{};
    if (SUCCEEDED(CreateDXGIFactory(IID_PPV_ARGS(&factory4))))
    {
        Microsoft::WRL::ComPtr<IDXGIFactory5> factory5{};
        if (SUCCEEDED(factory4.As(&factory5)))
        {
            if (FAILED(factory5->CheckFeatureSupport(
                DXGI_FEATURE_PRESENT_ALLOW_TEARING,
                &allowTearing, sizeof(allowTearing))))
            {
                allowTearing = FALSE;
            }
        }
    }
    return allowTearing == TRUE;
}

bool Application::IsTearingSupported() const
{
    return m_TearingSupported;
}

std::shared_ptr<Window> Application::CreateRenderWindow(const std::wstring& windowName, int clientWidth, int clientHeight, bool vSync)
{
    //First, check if a window with the given name already exists
    WindowNameMap::iterator windowIter = gs_WindowByName.find(windowName);
    if (windowIter != gs_WindowByName.end())
        return windowIter->second;

    RECT windowRect = { 0, 0, clientWidth, clientHeight };
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    HWND hWnd = CreateWindowW(WINDOW_CLASS_NAME, windowName.c_str(), WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, windowRect.right - windowRect.left, 
        windowRect.bottom - windowRect.top, nullptr, nullptr, m_hInstance, nullptr);

    if (!hWnd)
    {
        MessageBoxA(NULL, "Could not create the render window.", "Error", MB_OK | MB_ICONERROR);
        return nullptr;
    }

    WindowPtr pWindow = std::make_shared<MakeWindow>(hWnd, windowName, clientWidth, clientHeight, vSync);

    gs_Windows.insert(WindowMap::value_type(hWnd, pWindow));
    gs_WindowByName.insert(WindowNameMap::value_type(windowName, pWindow));

    return pWindow;
}

void Application::DestroyWindow(std::shared_ptr<Window> window)
{
    if (window)
        window->Destroy();
}

void Application::DestroyWindow(const std::wstring& windowName)
{
    WindowPtr pWindow = GetWindowByName(windowName);
    if (pWindow)
        DestroyWindow(pWindow);
}

std::shared_ptr<Window> Application::GetWindowByName(const std::wstring& windowName)
{
    std::shared_ptr<Window> window{};
    WindowNameMap::iterator iter = gs_WindowByName.find(windowName);
    if (iter != gs_WindowByName.end())
        window = iter->second;
    return window;
}

int Application::Run(std::shared_ptr<Game> pGame)
{
    if (!pGame->Initialize()) return 1;
    if (!pGame->LoadContent()) return 2;

    MSG msg = { 0 };
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    Flush();

    pGame->UnloadContent();
    pGame->Destroy();

    return static_cast<int>(msg.wParam);
}

void Application::Quit(int exitCode)
{
    PostQuitMessage(exitCode);
}

Microsoft::WRL::ComPtr<ID3D12Device2> Application::GetDevice() const
{
    return m_Device;
}

std::shared_ptr<CommandQueue> Application::GetCommandQueue(D3D12_COMMAND_LIST_TYPE type) const
{
    std::shared_ptr<CommandQueue> commandQueue{};
    switch (type)
    {
    case D3D12_COMMAND_LIST_TYPE_DIRECT:
        commandQueue = m_DirectCommandQueue;
        break;
    case D3D12_COMMAND_LIST_TYPE_COMPUTE:
        commandQueue = m_ComputeCommandQueue;
        break;
    case D3D12_COMMAND_LIST_TYPE_COPY:
        commandQueue = m_CopyCommandQueue;
        break;
    default:
        assert(false && "Invalid command queue type.");
    }

    return commandQueue;
}

void Application::Flush()
{
    m_DirectCommandQueue->Flush();
    m_ComputeCommandQueue->Flush();
    m_CopyCommandQueue->Flush();
}

Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> Application::CreateDescriptorHeap(UINT numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE type)
{
    D3D12_DESCRIPTOR_HEAP_DESC desc{};
    desc.Type = type;
    desc.NumDescriptors = numDescriptors;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    desc.NodeMask = 0;

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap;
    ThrowIfFailed(m_Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptorHeap)));

    return descriptorHeap;
}

UINT Application::GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE type) const
{
    return m_Device->GetDescriptorHandleIncrementSize(type);
}

//Remove window from our window list
static void RemoveWindow(HWND hWnd)
{
    WindowMap::iterator windowIter = gs_Windows.find(hWnd);
    if (windowIter != gs_Windows.end())
    {
        WindowPtr pWindow = windowIter->second;
        gs_WindowByName.erase(pWindow->GetWindowName());
        gs_Windows.erase(windowIter);
    }
}