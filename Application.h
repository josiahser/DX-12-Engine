#pragma once

#include "Events.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

//Undefine windows macro
#ifdef CreateWindow
	#undef CreateWindow
#endif

#include <cstdint>
#include <limits>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <type_traits>

class Window;

using WndProcEvent = Delegater<LRESULT(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)>;

class Application
{
public:
	//Create a single application with the application instance handle
	static Application& Create(HINSTANCE hInstance);

	//Destroy the application instance and all windows created by it
	static void Destroy();

	//Get the single Application
	static Application& Get();

	//Create logger

	//Get keyboard device ID

	//Get mouse device ID

	//Get gamepad device ID

	//Get an input device template

	//Create input map

	////Check if v-sync off is supported
	//bool IsTearingSupported() const;
	////Check if the requested multisample quality is supported for the given format
	//DXGI_SAMPLE_DESC GetMultisampleQualityLevels(DXGI_FORMAT format, UINT numSamples, D3D12_MULTISAMPLE_QUALITY_LEVEL_FLAGS flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE) const;
	////Create a new render window instance, width and height in pixels. If window with that name exists, that window will be returned
	//std::shared_ptr<Window> CreateRenderWindow(const std::wstring& windowClassName, int width, int height, bool vSync = true);
	////Destroy window given the window name
	//void DestroyWindow(const std::wstring& windowName);
	////Destroy the window given the window reference
	//void DestroyWindow(std::shared_ptr<Window> window);
	////Find window by window name
	//std::shared_ptr<Window> GetWindowByName(const std::wstring& windowName);

	//Run the application loop and message pump
	//return error code if error occurs
	int32_t Run();

	//Inform the input manager of changes to the size of the display
	//Needed for gainput to normalize mouse inputs
	//Only use it on a single window's Resize event
	void SetDisplaySize(int width, int height);

	//Process input events, should be called once per frame before updating game logic
	void ProcessInput();

	//Request to quit the application and close all windows
	void Stop();

	//Register directoryChange Listener

	//Create a render window
	std::shared_ptr<Window> CreateWindow(const std::wstring& windowName, int clientWidth, int clientHeight);

	//Get window by name
	std::shared_ptr<Window> GetWindowByName(const std::wstring& windowName) const;

	//Invoked when a message is sent to a window
	WndProcEvent WndProcHandler;

	//invoked when a file is modified on disk

	//Application is exiting
	Event Exit;

protected:
	friend LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	
	Application(HINSTANCE hInstance);
	virtual ~Application();

	//A file modification was detected

	//Windows message handler
	virtual LRESULT OnWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	//Application is going to close
	virtual void OnExit(EventArgs& e);

private:
	Application(const Application&) = delete;
	Application(Application&&) = delete;
	Application& operator=(Application&) = delete;
	Application& operator=(Application&&) = delete;

	//Directory change listener thread entry point function

	//Handle to app instance
	HINSTANCE m_hInstance;

	//Logger m_Logger;

	//Gainput input manager and devices

	//Set to true while app is running
	std::atomic_bool m_bIsRunning;
	//Should the app quit?
	std::atomic_bool m_RequestQuit;

	//Directory change listener

	//Thread to run directory change listener

	//Flag to terminate directory change thread
};