#pragma once

#pragma once

#include "RenderTarget.h"
#include <dxgi1_5.h>
#include <wrl/client.h>

#include <memory>

class CommandQueue;
class Device;
class Texture;

class SwapChain
{
public:
	//Number of swapchain back buffers
	static const UINT BufferCount = 3;

	//Check to see if the swap chain is in full screen exclusive mode
	bool IsFullscreen() const
	{
		return m_Fullscreen;
	}

	//Set to full screen (true) or windowed(false)
	void SetFullscreen(bool fullscreen);

	//Toggle fullscreen
	void ToggleFullscreen()
	{
		SetFullscreen(!m_Fullscreen);
	}
	
	void SetVSync(bool vSync)
	{
		m_VSync = vSync;
	}

	bool GetVSync() const
	{
		return m_VSync;
	}

	void ToggleVSync()
	{
		SetVSync(!m_VSync);
	}

	//Is tearing supported?
	bool IsTearingSupported() const
	{
		return m_TearingSupported;
	}

	//Block the current thread until the swapchain has finished presenting
	//Doing this at the beginning of the update loop can improve input latency
	void WaitForSwapChain();

	//Resize the swapchain's backbuffers. called whenever window is resized
	void Resize(uint32_t width, uint32_t height);

	//Get the render target of the window. Should be called every frame since the color attachment point changes depending on the window's current back buffer
	const RenderTarget& GetRenderTarget() const;

	//Present the swapchain's back buffer to the screen
	UINT Present(const std::shared_ptr<Texture>& texture = nullptr);

	//Get the format that is used to create the backbuffer
	DXGI_FORMAT GetRenderTargetFormat() const
	{
		return m_RenderTargetFormat;
	}

	Microsoft::WRL::ComPtr<IDXGISwapChain4> GetDXGISwapChain() const
	{
		return m_dxgiSwapChain;
	}

protected:
	//Swap chains can only be created through the device
	SwapChain(Device& device, HWND hWnd, DXGI_FORMAT renderTargetFormat = DXGI_FORMAT_R10G10B10A2_UNORM);
	virtual ~SwapChain();

	//Update the swapchain's RTVs
	void UpdateRenderTargetViews();

private:
	//Device we use to create the swap chain
	Device& m_Device;

	//Command queue that is used to create the swapchain
	//Command queue will be signaled right after the Present to ensure that the swapchain's back buffers are not in-flight before the next frame is allowed to be rendered
	CommandQueue& m_CommandQueue;
	Microsoft::WRL::ComPtr<IDXGISwapChain4> m_dxgiSwapChain;
	std::shared_ptr<Texture> m_BackBufferTextures[BufferCount];
	mutable RenderTarget m_RenderTarget;

	//The current backbuffer index of the swap chain
	UINT m_CurrentBackBufferIndex;
	UINT64 m_FenceValues[BufferCount]; //The fence values to wait for before leaving the Present method

	//A handle to a waitable object. Used to wait for the swapchain before presenting
	HANDLE m_hFrameLatencyWaitableObject;

	//The window handle that is associated with this swap chain
	HWND m_hWnd;

	//Current width/height of the swapchain
	uint32_t m_Width;
	uint32_t m_Height;

	//The format of the back buffer
	DXGI_FORMAT m_RenderTargetFormat;

	//Should present be synced with vertical refresh rate of the screen. (true to remove screen tearing)
	bool m_VSync;

	//Whether or not tearing is supported
	bool m_TearingSupported;

	//Whether the app is in full screen or windowed mode
	bool m_Fullscreen;

};