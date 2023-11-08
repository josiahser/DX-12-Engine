#include "framework.h"
#include "LearningAttempt2.h"

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
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
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
ComPtr<IDXGISwapChain3> g_SwapChain{};          //The Swap chain interface pointer (To swap buffer execution chains, presents the rendered image to the window)
ComPtr<ID3D12Resource> g_BackBuffers[g_NumFrames] = {}; //The Backbuffer resources interface pointer array (Holds the pointers to the backbuffer resources)
ComPtr<ID3D12GraphicsCommandList2> g_CommandList{};  //The command list interface pointer (GPU commands go here first, then it places commands to execute in the queue, uses a single thread per list)
ComPtr<ID3D12CommandAllocator> g_CommandAllocators[g_NumFrames] = {}; //The backing memory for recording the GPU commands into a command list (Can't be reused w/out the queue finishing, at least one per render frame that is "in flight" so at least one per back buffer)
ComPtr<ID3D12DescriptorHeap> g_RTVDescriptorHeap{}; //The back buffer textures of the swap chain. Describes location of texture resource in GPU mem, dimensions of texture, and format. Clears back buffers of the render target and render geometry to the screen)
UINT g_RTVDescriptorSize{};                     //The RTVs are stored in a descriptor heap (an array of descriptors or views, or better put, a resource that resides in GPU memory)
                                                //The size of a single RTV is vendor specific, so to correctly offset the index, its' size needs to be queried during init.
UINT g_CurrentBackBufferIndex{};                //The index of the current back buffer might not be sequential

//Syncronization objects
ComPtr<ID3D12Fence1> g_Fence{};                 //Used to store the fence object for syncing the command queue
uint64_t g_FenceValue = 0;                      //Next fence value to signal the command queue next command
uint64_t g_FrameFenceValues[g_NumFrames] = {};  //For each rendered frame that could be "in-flight" in the command Queue, this var holds the fence value used for that particular indexed frame
HANDLE g_FenceEvent{};                          //a handle to an OS event object that will be used to receive the notification that the fence has reached a value (so it know when to stop delaying)

//Some variables to control the swap Chain's present method
bool g_VSync = true;                            //By default, enable VSYNC, can be toggled with the V key
bool g_TearingSupoorted = false;                //by default tearing not supported
bool g_Fullscreen = false;                      //By default, use windowed mode, can be toggled with the Alt+Enter or F11 keys

// Forward declarations of functions included in this code module:
void                  MyRegisterClass(HINSTANCE hInstance);
HWND                  InitInstance(HINSTANCE, uint32_t, uint32_t);
LRESULT CALLBACK      WndProc(HWND, UINT, WPARAM, LPARAM);
void                  ParseCommandLineArguments();
ComPtr<IDXGIAdapter4> GetAdapter(bool useWarp); //Query for a compatible adapter
ComPtr<ID3D12Device2> CreateDevice(ComPtr<IDXGIAdapter4> adapter);
ComPtr<ID3D12CommandQueue> CreateCommandQueue(ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type);
bool CheckTearingSupport();
ComPtr<IDXGISwapChain4> CreateSwapChain(HWND hWnd, ComPtr<ID3D12CommandQueue> commandQueue, uint32_t width, uint32_t height, uint32_t bufferCount);
ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(ComPtr<ID3D12Device2> device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors);
void UpdateRenderTargetViews(ComPtr<ID3D12Device2> device, ComPtr<IDXGISwapChain4> swapChain, ComPtr<ID3D12DescriptorHeap> descriptorHeap);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_LEARNINGATTEMPT2, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, g_ClientWidth, g_ClientHeight))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_LEARNINGATTEMPT2));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
void MyRegisterClass(HINSTANCE hInstance)
{
    //Registers a window class for creating our render window with
    WNDCLASSEXW wcex = {};

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = &WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LEARNINGATTEMPT2));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = NULL;
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

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
HWND InitInstance(HINSTANCE hInstance, uint32_t width, uint32_t height)
{
   hInst = hInstance; // Store instance handle in our global variable

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
   HWND hWnd = ::CreateWindowExW(NULL, szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
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
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
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

//TODO: Command Allocator

//TODO: Command List

//TODO: Fence

//TODO: Fence event

//TODO: Signal 

//TODO: Wait for fence value

//TODO: Flush

//TODO: Update

//TODO: Render & Present

//TODO: Resize

//TODO: Fullscreen state

//TODO: Fix up the window message procedure (wndProc)

//TODO: Fix up the main entry point (wWinMain)

//TODO: Put everything into classes