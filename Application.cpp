#include "framework.h"
#include "Application.h"
#include "Game.h"
#include "Window.h"
#include "CommandQueue.h"
#include "DescriptorAllocator.h"

constexpr wchar_t WINDOW_CLASS_NAME[] = L"DX12RenderWindowClass";

using WindowPtr = std::shared_ptr<Window>;
using WindowMap = std::map<HWND, WindowPtr>;
using WindowNameMap = std::map<std::wstring, WindowPtr>;

static Application* gs_pSingleton = nullptr;
static WindowMap gs_Windows;
static WindowNameMap gs_WindowByName;

uint64_t Application::ms_FrameCount = 0;

static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

//Wrapper struct to allow shared pointers for the window class (the constructor and destructor for the Window class are protected, and not accessible by std::make_shared
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
    //
    //    m_Adapter = GetAdapter(false);
    //    if (m_Adapter)
    //        m_Device = CreateDevice(m_Adapter);
    //
    //    if (m_Device)
    //    {
    //        m_DirectCommandQueue = std::make_shared<CommandQueue>(m_Device, D3D12_COMMAND_LIST_TYPE_DIRECT);
    //        m_ComputeCommandQueue = std::make_shared<CommandQueue>(m_Device, D3D12_COMMAND_LIST_TYPE_COMPUTE);
    //        m_CopyCommandQueue = std::make_shared<CommandQueue>(m_Device, D3D12_COMMAND_LIST_TYPE_COPY);
    //
    //        m_TearingSupported = CheckTearingSupport();
    //    }
    //
}

void Application::Initialize()
{
#if defined(_DEBUG)
    //Always enable the debug layer before doing anything DX12 related
    Microsoft::WRL::ComPtr<ID3D12Debug> debugInterface;
    ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)));
    debugInterface->EnableDebugLayer();
    //Enable the following if you want full validation (will slow down rendering a lot though)
    //debugInterface->SetEnableGPUBasedValidation(TRUE);
    //debugInterface->SetEnableSynchronizedCommandQueueValidation(TRUE);
#endif
    auto dxgiAdapter = GetAdapter(false);
    if (!dxgiAdapter)
    {
        //if no supporting DX12 adapters exist, fall back to WARP
        dxgiAdapter = GetAdapter(true);
    }

    if (dxgiAdapter)
    {
        m_Device = CreateDevice(dxgiAdapter);
    }
    else
    {
        throw std::exception("DXGI adapter enumeration failed");
    }

    m_DirectCommandQueue = std::make_shared<CommandQueue>(D3D12_COMMAND_LIST_TYPE_DIRECT);
    m_ComputeCommandQueue = std::make_shared<CommandQueue>(D3D12_COMMAND_LIST_TYPE_COMPUTE);
    m_CopyCommandQueue = std::make_shared<CommandQueue>(D3D12_COMMAND_LIST_TYPE_COPY);

    m_TearingSupported = CheckTearingSupport();

    //Create Descriptor Allocators
    for (int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
    {
        m_DescriptorAllocators[i] = std::make_unique<DescriptorAllocator>(static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(i));
    }

    //Initialize frame counter
    ms_FrameCount = 0;
}

void Application::Create(HINSTANCE hInstance)
{
    if (!gs_pSingleton)
    {
        gs_pSingleton = new Application(hInstance);
        gs_pSingleton->Initialize();
    }
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
    Microsoft::WRL::ComPtr<IDXGIFactory4> dxgiFactory;    //Create factory, which is used to query available adapters (video card, interfaces with the hardware)
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
    Microsoft::WRL::ComPtr<IDXGIFactory4> factory4;
    if (SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&factory4))))
    {
        Microsoft::WRL::ComPtr<IDXGIFactory5> factory5;
        if (SUCCEEDED(factory4.As(&factory5)))
        {
            factory5->CheckFeatureSupport(
                DXGI_FEATURE_PRESENT_ALLOW_TEARING,
                &allowTearing, sizeof(allowTearing));
        }
    }
    return allowTearing == TRUE;
}

bool Application::IsTearingSupported() const
{
    return m_TearingSupported;
}

DXGI_SAMPLE_DESC Application::GetMultisampleQualityLevels(DXGI_FORMAT format, UINT numSamples, D3D12_MULTISAMPLE_QUALITY_LEVEL_FLAGS flags) const
{
    DXGI_SAMPLE_DESC sampleDesc = { 1, 0 };

    D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS qualityLevels;
    qualityLevels.Format = format;
    qualityLevels.SampleCount = 1;
    qualityLevels.Flags = flags;
    qualityLevels.NumQualityLevels = 0;

    while (qualityLevels.SampleCount <= numSamples && SUCCEEDED(m_Device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &qualityLevels, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS))) && qualityLevels.NumQualityLevels > 0)
    {
        sampleDesc.Count = qualityLevels.SampleCount;
        sampleDesc.Quality = qualityLevels.NumQualityLevels - 1;

        qualityLevels.SampleCount *= 2;
    }

    return sampleDesc;
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
    pWindow->Initialize();

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
    std::shared_ptr<Window> window;
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
    std::shared_ptr<CommandQueue> commandQueue;
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

DescriptorAllocation Application::AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors)
{
    return m_DescriptorAllocators[type]->Allocate(numDescriptors);
}

void Application::ReleaseStaleDescriptors(uint64_t finishedFrame)
{
    for (int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
    {
        m_DescriptorAllocators[i]->ReleaseStaleDescriptors(finishedFrame);
    }
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

//Convert message ID into a mousebutton ID
MouseButtonEventArgs::MouseButton DecodeMouseButton(UINT messageID)
{
    MouseButtonEventArgs::MouseButton mouseButton = MouseButtonEventArgs::None;
    switch (messageID)
    {
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_LBUTTONDBLCLK:
    {
        mouseButton = MouseButtonEventArgs::Left;
    }
    break;
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_RBUTTONDBLCLK:
    {
        mouseButton = MouseButtonEventArgs::Right;
    }
    break;
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_MBUTTONDBLCLK:
    {
        mouseButton = MouseButtonEventArgs::Middle;
    }
    break;
    }

    return mouseButton;
}

static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
    {
        return true;
    }

    WindowPtr pWindow;
    {
        WindowMap::iterator iter = gs_Windows.find(hWnd);
        if (iter != gs_Windows.end())
            pWindow = iter->second;
    }

    if (pWindow)
    {
        switch (message)
        {
            //case WM_COMMAND:
            //    {
            //        int wmId = LOWORD(wParam);
            //        // Parse the menu selections:
            //        switch (wmId)
            //        {
            //        case IDM_EXIT:
            //            DestroyWindow(hWnd);
            //            break;
            //        default:
            //            return DefWindowProc(hWnd, message, wParam, lParam);
            //        }
            //    }
            //    break;
        case WM_PAINT:
        {
            ++Application::ms_FrameCount;
            //Delta time will be filled by the Window
            UpdateEventArgs updateEventArgs(0.0f, 0.0f, Application::ms_FrameCount);
            pWindow->OnUpdate(updateEventArgs);
            RenderEventArgs renderEventArgs(0.0f, 0.0f, Application::ms_FrameCount);
            pWindow->OnRender(renderEventArgs);
        }
        break;
        case WM_SYSKEYDOWN:
        case WM_KEYDOWN:
        {
            MSG charMsg;
            //Get the unicode charater (UTF-16)
            unsigned int c = 0;
            
            //For printable characters, the next message will be WM_CHAR
            //this message contains the character code we need to send the key pressed event
            if (PeekMessage(&charMsg, hWnd, 0, 0, PM_NOREMOVE) && charMsg.message == WM_CHAR)
            {
                GetMessage(&charMsg, hWnd, 0, 0);
                c = static_cast<unsigned int>(charMsg.wParam);

                /*if (charMsg.wParam > 0 && charMsg.wParam < 0x10000)
                    ImGui::GetIO().AddInputCharacter((unsigned short)charMsg.wParam);*/
            }
            bool shift = (GetAsyncKeyState(VK_SHIFT) & 0x800) != 0;
            bool control = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
            bool alt = (GetAsyncKeyState(VK_MENU) & 0x8000) != 0;
            KeyCode::Key key = (KeyCode::Key)wParam;
            unsigned int scanCode = (lParam & 0x00FF0000) >> 16;
            KeyEventArgs keyEventArgs(key, c, KeyEventArgs::Pressed, shift, control, alt);
            pWindow->OnKeyPressed(keyEventArgs);
        }
        break;
        case WM_SYSKEYUP:
        case WM_KEYUP:
        {
            bool shift = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
            bool control = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
            bool alt = (GetAsyncKeyState(VK_MENU) & 0x8000) != 0;
            KeyCode::Key key = (KeyCode::Key)wParam;
            unsigned int c = 0;
            unsigned int scanCode = (lParam & 0x00FF0000) >> 16;

            //Determine which key was released by converting key code and scan code to a character
            unsigned char keyboardState[256];
            GetKeyboardState(keyboardState);
            wchar_t translatedCharacters[4];
            if (int result = ToUnicodeEx(static_cast<UINT>(wParam), scanCode, keyboardState, translatedCharacters, 4, 0, NULL) > 0)
                c = translatedCharacters[0];

            KeyEventArgs keyEventArgs(key, c, KeyEventArgs::Released, shift, control, alt);
            pWindow->OnKeyReleased(keyEventArgs);
        }
        break;
        //The default window procedure will play a sys notification when alt+enter combo is pressed
        //Unless we handle this message below
        case WM_SYSCHAR:
            break;
        case WM_MOUSEMOVE:
        {
            bool lButton = (wParam & MK_LBUTTON) != 0;
            bool rButton = (wParam & MK_RBUTTON) != 0;
            bool mButton = (wParam & MK_MBUTTON) != 0;
            bool shift = (wParam & MK_SHIFT) != 0;
            bool control = (wParam & MK_CONTROL) != 0;

            int x = ((int)(short)LOWORD(lParam));
            int y = ((int)(short)HIWORD(lParam));

            MouseMotionEventArgs mouseMotionEventArgs(lButton, mButton, rButton, control, shift, x, y);
            pWindow->OnMouseMoved(mouseMotionEventArgs);
        }
        break;
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN:
        {
            bool lButton = (wParam & MK_LBUTTON) != 0;
            bool rButton = (wParam & MK_RBUTTON) != 0;
            bool mButton = (wParam & MK_MBUTTON) != 0;
            bool shift = (wParam & MK_SHIFT) != 0;
            bool control = (wParam & MK_CONTROL) != 0;

            int x = ((int)(short)LOWORD(lParam));
            int y = ((int)(short)HIWORD(lParam));

            MouseButtonEventArgs mouseButtonEventArgs(DecodeMouseButton(message), MouseButtonEventArgs::Pressed, lButton, mButton, rButton, control, shift, x, y);
            pWindow->OnMouseButtonPressed(mouseButtonEventArgs);
        }
        break;
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP:
        {
            bool lButton = (wParam & MK_LBUTTON) != 0;
            bool rButton = (wParam & MK_RBUTTON) != 0;
            bool mButton = (wParam & MK_MBUTTON) != 0;
            bool shift = (wParam & MK_SHIFT) != 0;
            bool control = (wParam & MK_CONTROL) != 0;

            int x = ((int)(short)LOWORD(lParam));
            int y = ((int)(short)HIWORD(lParam));

            MouseButtonEventArgs mouseButtonEventArgs(DecodeMouseButton(message), MouseButtonEventArgs::Released, lButton, mButton, rButton, control, shift, x, y);
            pWindow->OnMouseButtonReleased(mouseButtonEventArgs);
        }
        break;
        case WM_MOUSEWHEEL:
        {
            // The distance the mouse wheel is rotated.
            // A positive value indicates the wheel was rotated to the right.
            // A negative value indicates the wheel was rotated to the left.
            float zDelta = ((int)(short)HIWORD(wParam)) / (float)WHEEL_DELTA;
            short keyStates = (short)LOWORD(wParam);

            bool lButton = (keyStates & MK_LBUTTON) != 0;
            bool rButton = (keyStates & MK_RBUTTON) != 0;
            bool mButton = (keyStates & MK_MBUTTON) != 0;
            bool shift = (keyStates & MK_SHIFT) != 0;
            bool control = (keyStates & MK_CONTROL) != 0;

            int x = ((int)(short)LOWORD(lParam));
            int y = ((int)(short)HIWORD(lParam));

            // Convert the screen coordinates to client coordinates.
            POINT clientToScreenPoint{};
            clientToScreenPoint.x = x;
            clientToScreenPoint.y = y;
            ScreenToClient(hWnd, &clientToScreenPoint);

            MouseWheelEventArgs mouseWheelEventArgs(zDelta, lButton, mButton, rButton, control, shift, (int)clientToScreenPoint.x, (int)clientToScreenPoint.y);
            pWindow->OnMouseWheel(mouseWheelEventArgs);
        }
        break;
        case WM_SIZE:
        {
            int width = ((int)(short)LOWORD(lParam));
            int height = ((int)(short)HIWORD(lParam));

            ResizeEventArgs resizeEventArgs(width, height);
            pWindow->OnResize(resizeEventArgs);
        }
        break;
        case WM_DESTROY:
        {
            //If a window is being destroyed, remove it from the window maps
            RemoveWindow(hWnd);
            if (gs_Windows.empty()) //If there are no more windows, quit the app
                PostQuitMessage(0);
        }
        break;
        default:
            return ::DefWindowProcW(hWnd, message, wParam, lParam);
        }
    }
    else
    {
        return ::DefWindowProcW(hWnd, message, wParam, lParam);
    }
    return 0;
}