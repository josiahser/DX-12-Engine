#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <Windows.h>
#include <shellapi.h>

#if defined(CreateWindow)
#undef CreateWindow
#endif

#include <wrl/client.h>
using namespace Microsoft::WRL;

//D3D12 Extension lib
#include "DirectX-Headers/include/directx/d3dx12.h"
#include <DirectXMath.h>
#include "DirectXTex-main/DirectXTex/DirectXTex.h"

//DirectX 12 headers
#include <d3dcompiler.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>

using namespace DirectX;

#pragma comment(lib, "d3d12")
#pragma comment(lib, "dxgi")
#pragma comment(lib, "d3dcompiler")
// 
// C RunTime Header Files
//#include <stdlib.h>
//#include <malloc.h>
//#include <memory.h>
//#include <tchar.h>

#if defined(min)
#undef min
#endif

#if defined(max)
#undef max
#endif

//STL Headers
#include <algorithm>
#include <atomic>
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <new>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

namespace fs = std::filesystem;

#include "Helpers.h"