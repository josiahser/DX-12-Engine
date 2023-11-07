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