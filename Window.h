#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <wrl.h>
#include "DirectX-Headers/include/directx/d3d12.h"
#include <dxgi1_5.h>

#include "Events.h"
#include "HighResolutionClock.h"
#include "RenderTarget.h"
#include "Texture.h"
//#include "GUI.h"

#include <memory>

class Game;
class Texture;

class Window : public std::enable_shared_from_this<Window>
{
public:
	//Number of swapchain back buffers
	static const UINT bufferCount = 3;

	//Get a handle to this windows instance, or nullptr if it's not a valid window
	HWND GetWindowHandle() const;

	//initialize the window
	void Initialize();

	//Destroy this window
	void Destroy();

	const std::wstring& GetWindowName() const;

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

	//Get the render target of the window. This method should be called every frame since the color attachment point changes depending on the window's current back buffer
	const RenderTarget& GetRenderTarget() const;

	//Present the swapchain's back buffer to the screen, returns the current index after presenting
	UINT Present(const Texture& texture = Texture());

protected:
	//The window proc needs to call protected methods of this class
	friend LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

	//Only the application can create a window
	friend class Application;

	//The game class needs to register itself with a window
	friend class Game;

	Window() = delete;
	Window(HWND hWnd, const std::wstring& windowName, int clientWidth, int clientHeight, bool vSync);
	virtual ~Window();

	//Register a game with this window. this allows the window to callback functions in the game class
	void RegisterCallbacks(std::shared_ptr<Game> pGame);

	//Update and Draw can only be called by the application
	virtual void OnUpdate(UpdateEventArgs& e);
	virtual void OnRender(RenderEventArgs& e);

	//A keyboard key was pressed
	virtual void OnKeyPressed(KeyEventArgs& e);
	//Keyboard key was released
	virtual void OnKeyReleased(KeyEventArgs& e);

	//Mouse was moved
	virtual void OnMouseMoved(MouseMotionEventArgs& e);
	//Mouse button was pressed
	virtual void OnMouseButtonPressed(MouseButtonEventArgs& e);
	//Mouse button was released
	virtual void OnMouseButtonReleased(MouseButtonEventArgs& e);
	//Mouse wheel was moved
	virtual void OnMouseWheel(MouseWheelEventArgs& e);

	//The window was resized
	virtual void OnResize(ResizeEventArgs& e);

	//Create the swapchain
	Microsoft::WRL::ComPtr<IDXGISwapChain4> CreateSwapChain();

	//Update the render target views for the swapchain back buffers
	void UpdateRenderTargetViews();

private:
	//Windows should not be copied
	Window(const Window& copy) = delete;
	Window& operator=(const Window& other) = delete;

	HWND m_hWnd;

	std::wstring m_windowName;
	bool m_VSync;
	bool m_Fullscreen;
	int m_ClientWidth;
	int m_ClientHeight;

	//
	//High resolution clock goes here, render and update clock
	//
	
	HighResolutionClock m_UpdateClock;
	HighResolutionClock m_RenderClock;
	//uint64_t m_FrameCounter;

	UINT64 m_FenceValues[bufferCount];
	uint64_t m_FrameValues[bufferCount];

	//Associated game
	std::weak_ptr<Game> m_pGame;

	Microsoft::WRL::ComPtr<IDXGISwapChain4> m_SwapChain;
	Texture m_BackBufferTextures[bufferCount];
	//Marked mutable to allow modification in a const function
	mutable RenderTarget m_RenderTarget;

	UINT m_CurrentBackBufferIndex;
	//Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_RTVDescriptorHeap; //The back buffer textures of the swap chain. Describes location of texture resource in GPU mem, dimensions of texture, and format. Clears back buffers of the render target and render geometry to the screen)
	//Microsoft::WRL::ComPtr<ID3D12Resource> m_BackBuffers[bufferCount] = {};

	//UINT m_RTVDescriptorSize;
	//UINT m_CurrentBackBufferIndex;

	RECT m_WindowRect;

	bool m_IsTearingSupported;

	int m_PreviousMouseX;
	int m_PreviousMouseY;

	//GUI m_GUI;
};