#include "framework.h"
#include "LearningAttempt2.h"

#pragma comment (lib, "d3d12")
#pragma comment (lib, "dxgi")
#pragma comment (lib, "d3dcompiler")

#if defined(min)
#undef min
#endif

#if defined(max)
#undef max
#endif

#if defined(CreateWindow)
#undef CreateWindow
#endif

#define MAX_LOADSTRING 100

using namespace Microsoft::WRL;

void EnableDebugLayer();                        //Activate debug layer

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
const uint8_t g_NumFrames = 3;                  // The number of swap chain back buffers (CANNOT be less than 2)
bool g_UseWarp = false;                         //Use WARP adapter (Software rasterizer, for old GPUs)

uint32_t g_ClientWidth = 1280;                  //Window width
uint32_t g_ClientHeight = 720;                  //Window height

bool g_IsInitialized = false;                   //Set to true once DX12 objects are initialized (to prevent early window messages)

//Global DirectX Variables
HWND g_hWnd;                                    //Global Window Handle

RECT g_WindowRect;                              //Global Window Rectangle (for fullscreen state, stores the previous window dimensions b4 going fullscreen)

//Okay actually the D3D12 objects
ComPtr<ID3D12Device2> g_Device{};               //The D3D12 Device Interface pointer (which is connected to the d3d12 adapter)
ComPtr<ID3D12CommandQueue> g_CommandQueue{};    //The D3D12 Command Queue Interface pointer (to execute command lists in the queue)
ComPtr<IDXGISwapChain4> g_SwapChain{};          //The Swap chain interface pointer (To swap buffer execution chains, presents the rendered image to the window)
ComPtr<ID3D12Resource> g_BackBuffers[g_NumFrames] = {}; //The Backbuffer resources interface pointer array (Holds the pointers to the backbuffer resources)
ComPtr<ID3D12GraphicsCommandList> g_CommandList{};  //The command list interface pointer (GPU commands go here first, then it places commands to execute in the queue, uses a single thread per list)
ComPtr<ID3D12CommandAllocator> g_CommandAllocators[g_NumFrames] = {}; //The backing memory for recording the GPU commands into a command list (Can't be reused w/out the queue finishing, at least one per render frame that is "in flight" so at least one per back buffer)
ComPtr<ID3D12DescriptorHeap> g_RTVDescriptorHeap{}; //The back buffer textures of the swap chain. Describes location of texture resource in GPU mem, dimensions of texture, and format. Clears back buffers of the render target and render geometry to the screen)
UINT g_RTVDescriptorSize{};                     //The RTVs are stored in a descriptor heap (an array of descriptors or views, or better put, a resource that resides in GPU memory)
                                                //The size of a single RTV is vendor specific, so to correctly offset the index, its' size needs to be queried during init.
UINT g_CurrentBackBufferIndex{};                //The index of the current back buffer might not be sequential

//Syncronization objects
ComPtr<ID3D12Fence> g_Fence{};                 //Used to store the fence object for syncing the command queue
uint64_t g_FenceValue = 0;                      //Next fence value to signal the command queue next command
uint64_t g_FrameFenceValues[g_NumFrames] = {};  //For each rendered frame that could be "in-flight" in the command Queue, this var holds the fence value used for that particular indexed frame
HANDLE g_FenceEvent{};                          //a handle to an OS event object that will be used to receive the notification that the fence has reached a value (so it know when to stop delaying)

//Some variables to control the swap Chain's present method
bool g_VSync = true;                            //By default, enable VSYNC, can be toggled with the V key
bool g_TearingSupoorted = false;                //by default tearing not supported
bool g_Fullscreen = false;                      //By default, use windowed mode, can be toggled with the Alt+Enter or F11 keys

// Forward declarations of functions included in this code module:
void                                MyRegisterClass(HINSTANCE hInstance, const wchar_t* className);
HWND                                InitInstance(const wchar_t* windowClassName, HINSTANCE hInstance, LPCWSTR title, uint32_t width, uint32_t height);
LRESULT CALLBACK                    WndProc(HWND, UINT, WPARAM, LPARAM);
void                                ParseCommandLineArguments();
ComPtr<IDXGIAdapter4>               GetAdapter(bool useWarp); //Query for a compatible adapter
ComPtr<ID3D12Device2>               CreateDevice(ComPtr<IDXGIAdapter4> adapter);
ComPtr<ID3D12CommandQueue>          CreateCommandQueue(ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type);
bool                                CheckTearingSupport();
ComPtr<IDXGISwapChain4>             CreateSwapChain(HWND hWnd, ComPtr<ID3D12CommandQueue> commandQueue, uint32_t width, uint32_t height, uint32_t bufferCount);
ComPtr<ID3D12DescriptorHeap>        CreateDescriptorHeap(ComPtr<ID3D12Device2> device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors);
void                                UpdateRenderTargetViews(ComPtr<ID3D12Device2> device, ComPtr<IDXGISwapChain4> swapChain, ComPtr<ID3D12DescriptorHeap> descriptorHeap);
ComPtr<ID3D12CommandAllocator>      CreateCommandAllocator(ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type);
ComPtr<ID3D12GraphicsCommandList>   CreateCommandList(ComPtr<ID3D12Device2> device, ComPtr<ID3D12CommandAllocator> commandAllocator, D3D12_COMMAND_LIST_TYPE type);
ComPtr<ID3D12Fence>                 CreateFence(ComPtr<ID3D12Device2> device);
HANDLE                              CreateEventHandle();
uint64_t                            Signal(ComPtr<ID3D12CommandQueue> commandQueue, ComPtr<ID3D12Fence> fence, uint64_t& fenceValue);
void                                WaitForFenceValue(ComPtr<ID3D12Fence> fence, uint64_t fenceValue, HANDLE fenceEvent, std::chrono::milliseconds duration = std::chrono::milliseconds::max());
void                                Flush(ComPtr<ID3D12CommandQueue> commandQueue, ComPtr<ID3D12Fence> fence, uint64_t& fenceValue, HANDLE fenceEvent);
void                                Update();
void                                Render();
void                                Resize(uint32_t width, uint32_t height);
void                                SetFullScreen(bool fullscreen);

//Main entry point for the application
int CALLBACK wWinMain(HINSTANCE hInstance,
                      HINSTANCE hPrevInstance,
                      PWSTR     lpCmdLine,
                      int       nCmdShow)
{
    //If Windows 10 and above, per Monitor V2 DPI awareness context available
    //This allows the client area of the window to achieve 100% scaling
    //While still allowing non-client window content to be rendered in a DPI sensitive fashion
    SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    //// Initialize global strings
    //LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    //LoadStringW(hInstance, IDC_LEARNINGATTEMPT2, szWindowClass, MAX_LOADSTRING);
    //MyRegisterClass(hInstance);

    //// Perform application initialization:
    //if (!InitInstance (hInstance, g_ClientWidth, g_ClientHeight))
    //{
    //    return FALSE;
    //}

    const wchar_t* windowClassName = L"DX12WindowClass";
    ParseCommandLineArguments();

    EnableDebugLayer();

    g_TearingSupoorted = CheckTearingSupport();

    MyRegisterClass(hInstance, windowClassName);
    g_hWnd = InitInstance(windowClassName, hInst, L"Learning DirectX 12", g_ClientWidth, g_ClientHeight);

    //Initialize the global window rect variable
    ::GetWindowRect(g_hWnd, &g_WindowRect);

    //Create the DX12 objects
    //Start with the adapter
    ComPtr<IDXGIAdapter4> dxgiAdapter4 = GetAdapter(g_UseWarp);

    //Use that adapter to create a device
    g_Device = CreateDevice(dxgiAdapter4);

    //Use that device to create a direct command queue
    g_CommandQueue = CreateCommandQueue(g_Device, D3D12_COMMAND_LIST_TYPE_DIRECT);

    //Use the global window handle, window size and that new command queue to create a swap chain
    g_SwapChain = CreateSwapChain(g_hWnd, g_CommandQueue, g_ClientWidth, g_ClientHeight, g_NumFrames);

    //Make sure you query the swap chain for the current back buffer index and assign it to the global variable
    g_CurrentBackBufferIndex = g_SwapChain->GetCurrentBackBufferIndex();

    //Use the Device to create a Render Target View descriptor heap for the amount of resources in g_NumFrames
    g_RTVDescriptorHeap = CreateDescriptorHeap(g_Device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, g_NumFrames);

    //Get the size of each descriptor in the heap for incrementing using the device
    g_RTVDescriptorSize = g_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    //Fill the descriptor heap with render target views
    UpdateRenderTargetViews(g_Device, g_SwapChain, g_RTVDescriptorHeap);

    //Create the command list and command alloctors for the list, a command allocator(memory allocated for command list) for each in-flight render frames (# of back-buffers)
    for (int i = 0; i < g_NumFrames; ++i)
        g_CommandAllocators[i] = CreateCommandAllocator(g_Device, D3D12_COMMAND_LIST_TYPE_DIRECT);
    g_CommandList = CreateCommandList(g_Device, g_CommandAllocators[g_CurrentBackBufferIndex], D3D12_COMMAND_LIST_TYPE_DIRECT);

    //Create the fence and fence event object, using the device
    g_Fence = CreateFence(g_Device);
    g_FenceEvent = CreateEventHandle();

    g_IsInitialized = true;

    ::ShowWindow(g_hWnd, SW_SHOW);

    MSG msg = {};

    //HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_LEARNINGATTEMPT2));

    // Main message loop:
    while (msg.message != WM_QUIT)
    {
        if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }
    }
    //Make sure the command queue has finished all commands before closing
    Flush(g_CommandQueue, g_Fence, g_FenceValue, g_FenceEvent);

    ::CloseHandle(g_FenceEvent);

    return 0;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
void MyRegisterClass(HINSTANCE hInstance, const wchar_t* className)
{
    //Registers a window class for creating our render window with
    WNDCLASSEXW wcex = {};

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = &WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = ::LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LEARNINGATTEMPT2));
    wcex.hCursor        = ::LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = NULL;
    wcex.lpszClassName  = className;
    wcex.hIconSm        = ::LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    static ATOM atom = ::RegisterClassExW(&wcex);
    assert(atom > 0);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
HWND InitInstance(const wchar_t* windowClassName ,HINSTANCE hInstance, LPCWSTR title, uint32_t width, uint32_t height)
{
   //hInst = hInstance; // Store instance handle in our global variable

   int screenWidth = ::GetSystemMetrics(SM_CXSCREEN);
   int screenHeight = ::GetSystemMetrics(SM_CYSCREEN);

   RECT windowRect = { 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };
   ::AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

   int windowWidth = windowRect.right - windowRect.left;
   int windowHeight = windowRect.bottom - windowRect.top;

   //Center window within screen, clamp to 0, 0 for the top left corner
   int windowX = std::max<int>(0, (screenWidth - windowWidth) / 2);
   int windowY = std::max<int>(0, (screenHeight - windowHeight) / 2);
   
   //Create window handle
   HWND hWnd = ::CreateWindowExW(NULL, windowClassName, title, WS_OVERLAPPEDWINDOW,
      windowX, windowY, windowWidth, windowHeight, nullptr, nullptr, hInstance, nullptr);

   assert(hWnd && "Failed to create window");

   return hWnd;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (g_IsInitialized)
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
            Update();
            Render();
            break;
        case WM_SYSKEYDOWN:
        case WM_KEYDOWN:
        {
            bool alt = (::GetAsyncKeyState(VK_MENU) & 0x8000) != 0;

            switch (wParam)
            {
            case 'V':
                g_VSync = !g_VSync;
                break;
            case VK_ESCAPE:
                ::PostQuitMessage(0);
                break;
            case VK_RETURN:
                if (alt)
                {
            case VK_F11:
                SetFullScreen(!g_Fullscreen);
                }
                break;
            }
        }
        break;
        //The default window procedure will play a sys notification when alt+enter combo is pressed
        //Unless we handle this message below
        case WM_SYSCHAR:
            break;
        case WM_SIZE:
        {
            RECT clientRect = {};
            ::GetClientRect(g_hWnd, &clientRect);

            int width = clientRect.right - clientRect.left;
            int height = clientRect.bottom - clientRect.top;

            Resize(width, height);
        }
        break;
        case WM_DESTROY:
            PostQuitMessage(0);
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

void ParseCommandLineArguments()
{
    int argc;
    wchar_t** argv = ::CommandLineToArgvW(::GetCommandLineW(), &argc);

    for (size_t i = 0; i < argc; ++i)
    {
        if (::wcscmp(argv[i], L"-w") == 0 || ::wcscmp(argv[i], L"--width") == 0)
        {
            g_ClientWidth = ::wcstol(argv[++i], nullptr, 10);
        }
        if (::wcscmp(argv[i], L"-h") == 0 || ::wcscmp(argv[i], L"--height") == 0)
        {
            g_ClientHeight = ::wcstol(argv[++i], nullptr, 10);
        }
        if (::wcscmp(argv[i], L"-warp") == 0 || ::wcscmp(argv[i], L"--warp") == 0)
        {
            g_UseWarp = true;
        }
    }
    //Free Memory Allocated by CommandLineToArgvW
    ::LocalFree(argv);
}

void EnableDebugLayer()                         //Activate debug layer
{
#if defined(_DEBUG)                             //Allows all errors while creating d3d12 objects to be caught by debug layer
    ComPtr<ID3D12Debug> debugInterface{};
    ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)));
    debugInterface->EnableDebugLayer();
#endif
}

//Create factory and query available adapters until it finds one with highest VRAM and DX12 compatible (or use WARP)
ComPtr<IDXGIAdapter4> GetAdapter(bool useWarp)
{
    ComPtr<IDXGIFactory4> dxgiFactory{};    //Create factory, which is used to query available adapters (video card, interfaces with the hardware)
    UINT createFactoryFlags = 0;
#if defined(_DEBUG)
    createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

    ThrowIfFailed(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory)));

    ComPtr<IDXGIAdapter1> dxgiAdapter1;
    ComPtr<IDXGIAdapter4> dxgiAdapter4;

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

//Create device using the adapter that was previously queried using factories
ComPtr<ID3D12Device2> CreateDevice(ComPtr<IDXGIAdapter4> adapter)
{
    ComPtr<ID3D12Device2> d3d12Device2{};
    ThrowIfFailed(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&d3d12Device2)));
    //Enable debug messages in debug mode
#if defined(_DEBUG)
    ComPtr<ID3D12InfoQueue> pInfoQueue{};
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

//Create a command queue of specific type (direct) using the device created earlier
ComPtr<ID3D12CommandQueue> CreateCommandQueue(ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type)
{
    ComPtr<ID3D12CommandQueue> d3d12CommandQueue{};

    D3D12_COMMAND_QUEUE_DESC desc = {};
    desc.Type = type;
    desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;             
    desc.NodeMask = 0;                                      //Used for node structure if using multiple GPUs

    ThrowIfFailed(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&d3d12CommandQueue)));

    return d3d12CommandQueue;
}

//Check to see if screen tearing is supported (for variable refresh rates)
bool CheckTearingSupport()
{
    BOOL allowTearing = FALSE;

    //instead of directly making the dxgi 1.5 factory needed to have access to the ::CheckFeatureSupport method,
    //Create the DXGI 1.4 interface and query for 1.5.
    ComPtr<IDXGIFactory4> factory4{};
    if (SUCCEEDED(CreateDXGIFactory(IID_PPV_ARGS(&factory4))))
    {
        ComPtr<IDXGIFactory5> factory5{};
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

//Create the swap chain used to present buffers and flip front buffers with back buffers. Associated with a specific Window handle (hWnd)
ComPtr<IDXGISwapChain4> CreateSwapChain(HWND hWnd, ComPtr<ID3D12CommandQueue> commandQueue, uint32_t width, uint32_t height, uint32_t bufferCount)
{
    ComPtr<IDXGISwapChain4> dxgiSwapChain4{};
    ComPtr<IDXGIFactory4> dxgiFactory4{};
    UINT createFactoryFlags = 0;
#if defined(_DEBUG)
    createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

    ThrowIfFailed(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory4)));

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
    swapChainDesc.Width = width;
    swapChainDesc.Height = height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.Stereo = FALSE;
    swapChainDesc.SampleDesc = { 1, 0 };
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = bufferCount;
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    // It is recommended to always allow tearing if it's supported
    swapChainDesc.Flags = CheckTearingSupport() ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

    ComPtr<IDXGISwapChain1> swapChain1{};
    ThrowIfFailed(dxgiFactory4->CreateSwapChainForHwnd(commandQueue.Get(), hWnd, &swapChainDesc, nullptr, nullptr, &swapChain1));

    //Disable the alt+enter fullscreen toggle feature, it'll be handled manually
    ThrowIfFailed(dxgiFactory4->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER));
    
    ThrowIfFailed(swapChain1.As(&dxgiSwapChain4));

    return dxgiSwapChain4;
}

//Create the descriptor heap that holds Render Target Views (in this case, it stores the RTV)
ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(ComPtr<ID3D12Device2> device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors)
{
    ComPtr<ID3D12DescriptorHeap> descriptorHeap{};

    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.NumDescriptors = numDescriptors;
    desc.Type = type;  // Several different types, including one for RTV || One for Samplers || one for constant buffers, shader resource, or unordered access views || one for depth stencil views 
    
    ThrowIfFailed(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptorHeap)));

    return descriptorHeap;
}

//Update/Create the Render Target Views (RTV), describes a resource that can be attached to a bind slot of the output merger stage. Describes a resource that receives the final color from the pixel shader
void UpdateRenderTargetViews(ComPtr<ID3D12Device2> device, ComPtr<IDXGISwapChain4> swapChain, ComPtr<ID3D12DescriptorHeap> descriptorHeap)
{
    auto rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(descriptorHeap->GetCPUDescriptorHandleForHeapStart());

    for (int i = 0; i < g_NumFrames; ++i)
    {
        ComPtr<ID3D12Resource> backBuffer;
        ThrowIfFailed(swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer)));

        device->CreateRenderTargetView(backBuffer.Get(), nullptr, rtvHandle);

        g_BackBuffers[i] = backBuffer;

        rtvHandle.Offset(rtvDescriptorSize);
    }
}

//Create the command allocator, which is the backing memory used by a command list. associated with specific type of command list and can only be accessed indirectly through said list
ComPtr<ID3D12CommandAllocator> CreateCommandAllocator(ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type)
{
    ComPtr<ID3D12CommandAllocator> commandAllocator{};
    ThrowIfFailed(device->CreateCommandAllocator(type, IID_PPV_ARGS(&commandAllocator)));

    return commandAllocator;
}

//Create a command list used to record commands that are executed on the GPU. The commands are not executed until the command list is sent to the command queue.
ComPtr<ID3D12GraphicsCommandList> CreateCommandList(ComPtr<ID3D12Device2> device, ComPtr<ID3D12CommandAllocator> commandAllocator, D3D12_COMMAND_LIST_TYPE type)
{
    ComPtr<ID3D12GraphicsCommandList> commandList;
    ThrowIfFailed(device->CreateCommandList(0, type, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList)));

    ThrowIfFailed(commandList->Close());

    return commandList;
}

//Create a fence, an interface for a GPU/CPU synch object. Stores a 64bit uint initialized to 0 and updated using fence.signal for CPU and commandqueue.signal for GPU
ComPtr<ID3D12Fence> CreateFence(ComPtr<ID3D12Device2> device)
{
    ComPtr<ID3D12Fence> fence{};
    ThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));

    return fence;
}

//Create an OS event handle, used to block the CPU thread until the fence has been signaled a specific value
HANDLE CreateEventHandle()
{
    HANDLE fenceEvent{};

    fenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
    assert(fenceEvent && "Failed to create fence event.");

    return fenceEvent;
}

//Signal (set) the fence from the GPU with the value that the CPU needs to wait for, it is only signaled once the GPU command queue has reached that point during execution, not immediately.
uint64_t Signal(ComPtr<ID3D12CommandQueue> commandQueue, ComPtr<ID3D12Fence> fence, uint64_t& fenceValue)
{
    uint64_t fenceValueForSignal = ++fenceValue;
    ThrowIfFailed(commandQueue->Signal(fence.Get(), fenceValueForSignal));

    return fenceValueForSignal;
}

//If CPU needs to stall to wait for the GPU to finish executing commands, tell it to wait until the fence is signaled with a specific value.
void WaitForFenceValue(ComPtr<ID3D12Fence> fence, uint64_t fenceValue, HANDLE fenceEvent, std::chrono::milliseconds duration)
{
    if (fence->GetCompletedValue() < fenceValue)
    {
        ThrowIfFailed(fence->SetEventOnCompletion(fenceValue, fenceEvent));
        ::WaitForSingleObject(fenceEvent, static_cast<DWORD>(duration.count()));
    }
}

//Used if you need to wait until all previously executed commands have finished executing on the GPU before the CPU can continue (uses the Signal function and WaitForFenceValue function)
void Flush(ComPtr<ID3D12CommandQueue> commandQueue, ComPtr<ID3D12Fence> fence, uint64_t& fenceValue, HANDLE fenceEvent)
{
    uint64_t fenceValueForSignal = Signal(commandQueue, fence, fenceValue);
    WaitForFenceValue(fence, fenceValueForSignal, fenceEvent);
}

//Updates the display, for now, just the Framerate per second into the debug output in VS. Example of a gameloop updating every frame.
void Update()
{
    static uint64_t frameCounter = 0;
    static double elapsedSeconds = 0.0;
    static std::chrono::high_resolution_clock clock;
    static auto t0 = clock.now();

    frameCounter++;
    auto t1 = clock.now();
    auto deltaTime = t1 - t0;
    t0 = t1;

    elapsedSeconds += deltaTime.count() * 1e-9;
    if (elapsedSeconds > 1.0)
    {
        char buffer[500] = {};
        auto fps = frameCounter / elapsedSeconds;
        sprintf_s(buffer, 500, "FPS: %f\n", fps);
        OutputDebugStringA(buffer);

        frameCounter = 0;
        elapsedSeconds = 0.0;
    }
}

//Consists of clearing the backbuffer and presenting the rendered frame. Resources must be in the correct state, and transitioned using a resource barrier and putting it in the command list
//Resource barriers include Transition, Aliasing, and UAV
void Render()
{
    //Pointers to the command allocator and back buffer according to the backbuffer index
    auto& commandAllocator = g_CommandAllocators[g_CurrentBackBufferIndex];
    auto& backBuffer = g_BackBuffers[g_CurrentBackBufferIndex];

    //Both the allocator and list are reset
    commandAllocator->Reset();
    g_CommandList->Reset(commandAllocator.Get(), nullptr);

    //The first operation usually performed is a clear
    //but first, transitioning the render target to the RENDER_TARGET state
    {
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(backBuffer.Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

        g_CommandList->ResourceBarrier(1, &barrier);
        //If there are more than one resource barrier to insert into the command list, store all barriers in a list and execute them all at the same time

        //Now that the backbuffer is in the render target state, we can clear the back buffer
        FLOAT clearColor[] = { 0.4f, 0.6f, 0.9f, 1.0f };
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtv(g_RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), g_CurrentBackBufferIndex, g_RTVDescriptorSize);

        g_CommandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
    }

    //And then we present the rendered image to the screen. Again, the back buffer resource must be transitioned back to the PRESENT state
    {
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(backBuffer.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

        g_CommandList->ResourceBarrier(1, &barrier);

        //Now we execute the command list that contains the barrier on the command queue (has to be closed in order to execute list)
        ThrowIfFailed(g_CommandList->Close());

        ID3D12CommandList* const commandLists[] = { g_CommandList.Get() };
        g_CommandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

        UINT syncInterval = g_VSync ? 1 : 0;
        UINT presentFlags = g_TearingSupoorted && !g_VSync ? DXGI_PRESENT_ALLOW_TEARING : 0;
        ThrowIfFailed(g_SwapChain->Present(syncInterval, presentFlags));

        g_FrameFenceValues[g_CurrentBackBufferIndex] = Signal(g_CommandQueue, g_Fence, g_FenceValue);

        g_CurrentBackBufferIndex = g_SwapChain->GetCurrentBackBufferIndex();

        WaitForFenceValue(g_Fence, g_FrameFenceValues[g_CurrentBackBufferIndex], g_FenceEvent);
    }
}

//Resize the first time the window is created, and when resizing in windowed mode. Resizes the swap chain buffers if the client area of the window changes
void Resize(uint32_t width, uint32_t height)
{
    if (g_ClientWidth != width || g_ClientHeight != height)
    {
        //Don't allow 0 size swap chain back buffers
        g_ClientWidth = std::max(1u, width);
        g_ClientHeight = std::max(1u, height);

        //Flush the GPU queue to make sure the swap chain's back buffer
        //are not being referenced by an in-flight command list
        Flush(g_CommandQueue, g_Fence, g_FenceValue, g_FenceEvent);

        for (int i = 0; i < g_NumFrames; ++i)
        {
            //Any references to the back buffers must be released
            //before the swap chain can be resized
            g_BackBuffers[i].Reset();
            g_FrameFenceValues[i] = g_FrameFenceValues[g_CurrentBackBufferIndex];
        }
        //Make sure the same color format and flags are used to recreate the swap chain buffers here
        //We're basically reseting the buffers and swap chain but now with the resized width and height
        DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
        ThrowIfFailed(g_SwapChain->GetDesc(&swapChainDesc));
        ThrowIfFailed(g_SwapChain->ResizeBuffers(g_NumFrames, g_ClientWidth, g_ClientHeight, swapChainDesc.BufferDesc.Format, swapChainDesc.Flags));

        g_CurrentBackBufferIndex = g_SwapChain->GetCurrentBackBufferIndex();

        UpdateRenderTargetViews(g_Device, g_SwapChain, g_RTVDescriptorHeap);
    }
}

//We'll be using Full screen borderless window for simplicity

//Set the screen to fullscreen Borderless window
void SetFullScreen(bool fullscreen)
{
    if (g_Fullscreen != fullscreen)
    {
        g_Fullscreen = fullscreen;

        if (g_Fullscreen) //Switching to fullscreen here
        {
            //Store the current window dimensions so they can be restored easily
            ::GetWindowRect(g_hWnd, &g_WindowRect);

            //Set the window style to a borderless window so the client area fills the entire screen
            UINT windowStyle = WS_OVERLAPPEDWINDOW & ~(WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);

            ::SetWindowLongW(g_hWnd, GWL_STYLE, windowStyle);

            //Query the name of the nearest display device for the window
            //This is to se tthe fullscreen dimensions when using a multi-monitor setup
            HMONITOR hMonitor = ::MonitorFromWindow(g_hWnd, MONITOR_DEFAULTTONEAREST);
            MONITORINFOEX monitorInfo = {};
            monitorInfo.cbSize = sizeof(MONITORINFOEX);
            ::GetMonitorInfo(hMonitor, &monitorInfo);

            ::SetWindowPos(g_hWnd, HWND_TOP, monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.top,
                monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
                monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
                SWP_FRAMECHANGED | SWP_NOACTIVATE);

            ::ShowWindow(g_hWnd, SW_MAXIMIZE);
        }
        else
        {
            //Restore all the window decorators
            ::SetWindowLong(g_hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);

            ::SetWindowPos(g_hWnd, HWND_NOTOPMOST,
                g_WindowRect.left, g_WindowRect.top,
                g_WindowRect.right - g_WindowRect.left,
                g_WindowRect.bottom - g_WindowRect.top,
                SWP_FRAMECHANGED | SWP_NOACTIVATE);

            ::ShowWindow(g_hWnd, SW_NORMAL);
        }
    }
}


//TODO: Put everything into classes