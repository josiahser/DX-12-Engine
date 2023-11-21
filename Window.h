#pragma once

#include "framework.h"

class Game;

class Window
{
public:
	//Number of swapchain back buffers
	static const UINT bufferCount = 3;

	//Get a handle to this windows instance, or nullptr if it's not a valid window
	HWND GetWindowHandle() const;

	//Destroy this window
	void Destroy();

	const std::wstring& GetWindowName();

	int GetClientHeight() const;
	int GetClientWidth() const;

	//Should this window be rendered with V-sync?
	bool IsVSync() const;
	void SetVSync(bool vSync);
	void ToggleVSync();

	//Is this a windowed window or full-screen?
	bool IsFullscreen() const;

	//Set the fullscreen state of the window
	void SetFullscreen(bool fullscreen);
	void ToggleFullscreen();

	//Show this window
	void Show();

	//Hide the window
	void Hide();

	//Return the current back buffer index
	UINT GetCurrentBackBufferIndex() const;

	//Present the swapchain's back buffer to the screen, returns the current index after presenting
	UINT Present();

	//Get the RTV for the current back buffer
	D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentRenderTargetView() const;

	//Get the back buffer resource for the current back buffer
	Microsoft::WRL::ComPtr<ID3D12Resource> GetCurrentBackBuffer() const;

protected:
	//The window proc needs to call protected methods of this class
	friend LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

	//Only the application can create a window
	friend class Application;

	//The game class needs to register itself with a window
	friend class Game;

	Window() = delete;
	Window(HWND hWnd, const std::wstring& windowName, int clientWidth, int clientHeight, bool vSync);
	virtual ~Window();

	//Register a game with this window. this allows the window to callback functions in the game class
	void RegisterCallbacks(std::shared_ptr<Game> pGame);

	//
	//Here's where all the event arguments and keys go
	//

	//Create the swapchain
	Microsoft::WRL::ComPtr<IDXGISwapChain4> CreateSwapChain();

	//Update the render target views for the swapchain back buffers
	void UpdateRenderTargetViews();

private:
	//Windows should not be copied
	Window(const Window& copy) = delete;
	Window& operator=(const Window& other) = delete;

	HWND m_hWnd{};

	std::wstring m_windowName{};
	bool g_VSync{};
	bool g_Fullscreen{};
	int m_ClientWidth{};
	int m_ClientHeight{};

	//
	//High resolution clock goes here, render and update clock
	//
	
	std::chrono::high_resolution_clock m_UpdateClock;
	std::chrono::high_resolution_clock m_RenderClock;
	uint64_t m_FrameCounter{};

	//Associated game
	std::weak_ptr<Game> m_pGame{};

	Microsoft::WRL::ComPtr<IDXGISwapChain4> m_SwapChain{};
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_RTVDescriptorHeap{}; //The back buffer textures of the swap chain. Describes location of texture resource in GPU mem, dimensions of texture, and format. Clears back buffers of the render target and render geometry to the screen)
	Microsoft::WRL::ComPtr<ID3D12Resource> m_BackBuffers[bufferCount] = {};

	UINT m_RTVDescriptorSize{};
	UINT m_CurrentBackBufferIndex{};

	RECT m_WindowRect{};

	bool m_IsTearingSupported{};
};