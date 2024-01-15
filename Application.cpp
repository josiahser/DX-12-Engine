#include "ApplicationHeaders.h"

#include "Application.h"

#include "Window.h"

static Application* gs_pSingelton = nullptr;
constexpr wchar_t WINDOW_CLASS_NAME[] = L"RenderWindowClass";

static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

//Set the name of a std::thread
//for debugging
const DWORD MS_VC_EXCEPTION = 0x406D1388;

//Set the name of a running thread
#pragma pack(push, 8)
typedef struct tagTHREADNAME_INFO
{
    DWORD dwType; //Must be 0x1000
    LPCSTR szName; //POinter to name (in user address space)
    DWORD dwThreadID; //Thread ID (-1=caller thread)
    DWORD dwFlags; //Reserved for future use, must be zero
} THREADNAME_INFO;
#pragma pack(pop)

inline void SetThreadName(std::thread& thread, const char* threadName)
{
    THREADNAME_INFO info;
    info.dwType = 0x1000;
    info.szName = threadName;
    info.dwThreadID = ::GetThreadId(reinterpret_cast<HANDLE>(thread.native_handle()));
    info.dwFlags = 0;

    __try
    {
        ::RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {

    }
}

constexpr int MAX_CONSOLE_LINES = 500;

using WindowMap = std::map<HWND, std::weak_ptr<Window>>;
using WindowMapByName = std::map<std::wstring, std::weak_ptr<Window>>;
static WindowMap gs_WindowMap;
static WindowMapByName gs_WindowMapByName;

static std::mutex gs_WindowHandlesMutex;

//Wrapper struct to allow shared pointers for the window class (the constructor and destructor for the Window class are protected, and not accessible by std::make_shared
struct MakeWindow : public Window
{
	MakeWindow(HWND hWnd, const std::wstring& windowName, int clientWidth, int clientHeight)
		: Window(hWnd, windowName, clientWidth, clientHeight)
	{}
};

//Create a console window (consoles are not automatically created for Windows subsystem)
//static void CreateConsole()
//{
//    //Allocate a console
//    if (AllocConsole())
//    {
//        HANDLE lStdHandle = GetStdHandle(STD_OUTPUT_HANDLE);
//
//        //Increase screen buffer to allow more lines of text than the default
//        CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
//        GetConsoleScreenBufferInfo(lStdHandle, &consoleInfo);
//        consoleInfo.dwSize.Y = MAX_CONSOLE_LINES;
//        SetConsoleScreenBufferSize(lStdHandle, consoleInfo.dwSize);
//        SetConsoleCursorPosition(lStdHandle, { 0, 0 });
//
//        //Redirect unbuffered STDOUT to the console
//        int hConHandle = _open_osfhandle((intptr_t)lStdHandle, _O_TEXT);
//        FILE* fp = _fdopen(hConHandle, "w");
//        freopen_s(&fp, "CONOUT$", "w", stdout);
//        setvbuf(stdout, nullptr, _IONBF, 0);
//
//        //Redirect unbuffered STDIN to the console
//        lStdHandle = GetStdHandle(STD_INPUT_HANDLE);
//        hConHandle = _open_osfhandle((intptr_t)lStdHandle, _O_TEXT);
//        fp = _fdopen(hConHandle, "w");
//        freopen_s(&fp, "CONOUT", "w", stderr);
//        setvbuf(stderr, nullptr, _IONBF, 0);
//
//        //Clear the error state for each of the C++ standard stream objects
//        std::wcout.clear();
//        std::cout.clear();
//        std::wcerr.clear();
//        std::cerr.clear();
//        std::wcin.clear();
//        std::cin.clear();
//    }
//}

Application::Application(HINSTANCE hInstance)
    : m_hInstance(hInstance)
    , m_bIsRunning(false)
    , m_RequestQuit(false)
{
    // Per monitor V2 DPI awareness context, added in Windows 10
    SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

//#if defined(_DEBUG)
//    //Create a console window for std::cout
//    CreateConsole();
//#endif

    //Can uncomment after adding speedlog and Gainput

    //// Init spdlog.
    //spdlog::init_thread_pool(8192, 1);
    //auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    //auto rotating_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
    //    "logs/log.txt", 1024 * 1024 * 5, 3,
    //    true);  // Max size: 5MB, Max files: 3, Rotate on open: true
    //auto msvc_sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();

    //std::vector<spdlog::sink_ptr> sinks{ stdout_sink, rotating_sink, msvc_sink };
    //m_Logger = std::make_shared<spdlog::async_logger>("GameFramework", sinks.begin(), sinks.end(),
    //    spdlog::thread_pool(), spdlog::async_overflow_policy::block);
    //spdlog::register_logger(m_Logger);
    //spdlog::set_default_logger(m_Logger);

    //// Init gainput.
    //m_KeyboardDevice = m_InputManager.CreateDevice<gainput::InputDeviceKeyboard>();
    //m_MouseDevice = m_InputManager.CreateDevice<gainput::InputDeviceMouse>();
    //for (unsigned i = 0; i < gainput::MaxPadCount; ++i)
    //{
    //    m_GamepadDevice[i] = m_InputManager.CreateDevice<gainput::InputDevicePad>(i);
    //}

    //// This will prevent normalization of mouse coordinates.
    //m_InputManager.SetDisplaySize(1, 1);

    //Initializes the COM library for use by the calling thread, sets the threads concurrency model, and creates a new apartment for the thread if one is required
    //Must be called at least once for each thread that uses the COM library
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr))
    {
        _com_error err(hr);
        throw new std::exception("COM library error");
    }

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
}

Application::~Application()
{
    gs_WindowMap.clear();
    gs_WindowMapByName.clear();
}

Application& Application::Create(HINSTANCE hInstance)
{
    if (!gs_pSingelton)
    {
        gs_pSingelton = new Application(hInstance);
    }

    return *gs_pSingelton;
}

Application& Application::Get()
{
    assert(gs_pSingelton != nullptr);
    return *gs_pSingelton;
}

void Application::Destroy()
{
    if (gs_pSingelton)
    {
        delete gs_pSingelton;
        gs_pSingelton = nullptr;
    }
}

//Create logger

//Get  Gainput's

int32_t Application::Run()
{
    assert(!m_bIsRunning);

    m_bIsRunning = true;

    MSG msg = {};
    while (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) && msg.message != WM_QUIT)
    {
        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);

        if (m_RequestQuit)
        {
            ::PostQuitMessage(0);
            m_RequestQuit = false;
        }
    }

    m_bIsRunning = false;

    return static_cast<int32_t>(msg.wParam);
}

//Set display size

//process input

void Application::Stop()
{
    //When called from another thread other than the main thread,
    //the WM_QUIT message goes to that thread, to circumvent this
    //We set a boolean flag to indicate the user has requested to quit the app
    m_RequestQuit = true;
}

std::shared_ptr<Window> Application::CreateWindow(const std::wstring& windowName, int clientWidth, int clientHeight)
{
    int screenWidth = ::GetSystemMetrics(SM_CXSCREEN);
    int  screenHeight = ::GetSystemMetrics(SM_CYSCREEN);

    RECT windowRect = { 0, 0, static_cast<LONG>(clientWidth), static_cast<LONG>(clientHeight) };
    
    ::AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    uint32_t width = windowRect.right - windowRect.left;
    uint32_t height = windowRect.bottom - windowRect.top;

    int windowX = std::max<int>(0, (screenWidth - (int)width) / 2);
    int windowY = std::max<int>(0, (screenHeight - (int)height) / 2);

    HWND hWnd = ::CreateWindowExW(NULL, WINDOW_CLASS_NAME, windowName.c_str(), WS_OVERLAPPEDWINDOW,
        windowX, windowY, width, height, NULL, NULL, m_hInstance, NULL);

    if (!hWnd)
    {
        MessageBoxA(NULL, "Could not create the render window.", "Error", MB_OK | MB_ICONERROR);
        return nullptr;
    }

    auto pWindow = std::make_shared<MakeWindow>(hWnd, windowName, clientWidth, clientHeight);

    gs_WindowMap.insert(WindowMap::value_type(hWnd, pWindow));
    gs_WindowMapByName.insert(WindowMapByName::value_type(windowName, pWindow));

    return pWindow;
}

std::shared_ptr<Window> Application::GetWindowByName(const std::wstring& windowName) const
{
    auto iter = gs_WindowMapByName.find(windowName);
    return (iter != gs_WindowMapByName.end()) ? iter->second.lock() : nullptr;
}

//Register change listener and file change listeners

LRESULT Application::OnWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    auto res = WndProcHandler(hWnd, msg, wParam, lParam);
    return res ? *res : 0;
}

void Application::OnExit(EventArgs& e)
{
    //Invoke the exit event
    Exit(e);
}

//Convert message ID into a mousebutton ID
static MouseButton DecodeMouseButton(UINT messageID)
{
    MouseButton mouseButton = MouseButton::None;
    switch (messageID)
    {
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_LBUTTONDBLCLK:
    {
        mouseButton = MouseButton::Left;
    }
    break;
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_RBUTTONDBLCLK:
    {
        mouseButton = MouseButton::Right;
    }
    break;
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_MBUTTONDBLCLK:
    {
        mouseButton = MouseButton::Middle;
    }
    break;
    }

    return mouseButton;
}

static ButtonState DecodeButtonState(UINT messageID)
{
    ButtonState buttonState = ButtonState::Pressed;

    switch (messageID)
    {
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
    case WM_XBUTTONUP:
        buttonState = ButtonState::Released;
        break;
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_XBUTTONDOWN:
        buttonState = ButtonState::Pressed;
        break;
    }

    return buttonState;
}

//Convert wParam of the WM_SIZE events to a windowstate
static WindowState DecodeWindowState(WPARAM wParam)
{
    WindowState windowState = WindowState::Restored;

    switch (wParam)
    {
    case SIZE_RESTORED:
        windowState = WindowState::Restored;
        break;
    case SIZE_MINIMIZED:
        windowState = WindowState::Minimized;
        break;
    case SIZE_MAXIMIZED:
        windowState = WindowState::Maximized;
        break;
    default:
        break;
    }

    return windowState;
}

static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    //Allow for external handling of window messages
    if (Application::Get().OnWndProc(hWnd, message, wParam, lParam))
    {
        return 1;
    }

    std::shared_ptr<Window> pWindow;
    {
        auto iter = gs_WindowMap.find(hWnd);
        if (iter != gs_WindowMap.end())
            pWindow = iter->second.lock();
    }

    if (pWindow)
    {
        switch (message)
        {
        case WM_DPICHANGED:
        {
            float dpiScaling = HIWORD(wParam) / 96.0f;
            DPIScaleEventArgs dpiScaleEventArgs(dpiScaling);
            pWindow->OnDPIScaleChanged(dpiScaleEventArgs);
        }
        break;
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
            //Delta and total time will be filled in by the window
            UpdateEventArgs updateEventArgs(0.0, 0.0);
            pWindow->OnUpdate(updateEventArgs);
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
                //GetMessage(&charMsg, hWnd, 0, 0);
                c = static_cast<unsigned int>(charMsg.wParam);

                /*if (charMsg.wParam > 0 && charMsg.wParam < 0x10000)
                    ImGui::GetIO().AddInputCharacter((unsigned short)charMsg.wParam);*/
            }
            bool shift = (GetAsyncKeyState(VK_SHIFT) & 0x800) != 0;
            bool control = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
            bool alt = (GetAsyncKeyState(VK_MENU) & 0x8000) != 0;

            KeyCode key = (KeyCode)wParam;
            KeyEventArgs keyEventArgs(key, c, KeyState::Pressed, control, shift, alt);
            pWindow->OnKeyPressed(keyEventArgs);
        }
        break;
        case WM_SYSKEYUP:
        case WM_KEYUP:
        {
            bool shift = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
            bool control = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
            bool alt = (GetAsyncKeyState(VK_MENU) & 0x8000) != 0;

            KeyCode key = (KeyCode)wParam;
            unsigned int c = 0;
            unsigned int scanCode = (lParam & 0x00FF0000) >> 16;

            //Determine which key was released by converting key code and scan code to a character
            unsigned char keyboardState[256];
            GetKeyboardState(keyboardState);
            wchar_t translatedCharacters[4];
            if (int result = ToUnicodeEx((UINT)wParam, scanCode, keyboardState, translatedCharacters, 4, 0, NULL) > 0)
                c = translatedCharacters[0];

            KeyEventArgs keyEventArgs(key, c, KeyState::Released, control, shift, alt);
            pWindow->OnKeyReleased(keyEventArgs);
        }
        break;
        //The default window procedure will play a sys notification when alt+enter combo is pressed
        //Unless we handle this message below
        case WM_SYSCHAR:
            break;
        case WM_KILLFOCUS:
        {
            //Window lost keyboard focuse
            EventArgs eventArgs;
            pWindow->OnKeyboardBlur(eventArgs);
        }
        break;
        case WM_SETFOCUS:
        {
            EventArgs eventArgs;
            pWindow->OnKeyboardFocus(eventArgs);
        }
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

            MouseButtonEventArgs mouseButtonEventArgs(DecodeMouseButton(message), ButtonState::Pressed, lButton, mButton, rButton, control, shift, x, y);
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

            MouseButtonEventArgs mouseButtonEventArgs(DecodeMouseButton(message), ButtonState::Released, lButton, mButton, rButton, control, shift, x, y);
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
            ::ScreenToClient(hWnd, &clientToScreenPoint);

            MouseWheelEventArgs mouseWheelEventArgs(zDelta, lButton, mButton, rButton, control, shift, (int)clientToScreenPoint.x, (int)clientToScreenPoint.y);
            pWindow->OnMouseWheel(mouseWheelEventArgs);
        }
        break;
        case WM_SIZE:
        {
            WindowState windowState = DecodeWindowState(wParam);

            int width = ((int)(short)LOWORD(lParam));
            int height = ((int)(short)HIWORD(lParam));

            ResizeEventArgs resizeEventArgs(width, height, windowState);
            pWindow->OnResize(resizeEventArgs);
        }
        break;
        case WM_CLOSE:
        {
            WindowCloseEventArgs windowCloseEventArgs;
            pWindow->Close(windowCloseEventArgs);

            //Check to see if the user canceled the close event
            if (windowCloseEventArgs.ConfirmClose)
            {
                //Destroy window(hwnd);
                //or you can hide the window, windows will be destroyed when the app quits
                pWindow->Hide();
            }
        }
        break;
        case WM_DESTROY:
        {
            std::lock_guard<std::mutex> lock(gs_WindowHandlesMutex);
            WindowMap::iterator iter = gs_WindowMap.find(hWnd);
            if (iter != gs_WindowMap.end())
                gs_WindowMap.erase(iter);
        }
        break;
        default:
            return ::DefWindowProcW(hWnd, message, wParam, lParam);
        }
    }
    else
    {
        switch (message)
        {
        case WM_CREATE:
            break;
        default:
            return ::DefWindowProcW(hWnd, message, wParam, lParam);
        }
    }
    return 0;
}