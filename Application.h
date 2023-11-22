#pragma once

#include "framework.h"

class Window;
class Game;
class CommandQueue;

class Application
{
public:
	//Create a single application with the application instance handle
	static void Create(HINSTANCE hInstance);

	//Destroy the application instance and all windows created by it
	static void Destroy();

	//Get the single Application
	static Application& Get();

	//Check if v-sync off is supported
	bool IsTearingSupported() const;

	//Create a new render window instance, width and height in pixels. If window with that name exists, that window will be returned
	std::shared_ptr<Window> CreateRenderWindow(const std::wstring& windowClassName, int width, int height, bool vSync = true);

	//Destroy window given the window name
	void DestroyWindow(const std::wstring& windowName);

	//Destroy the window given the window reference
	void DestroyWindow(std::shared_ptr<Window> window);

	//Find window by window name
	std::shared_ptr<Window> GetWindowByName(const std::wstring& windowName);

	//Run the application loop and message pump
	//return error code if error occurs
	int Run(std::shared_ptr<Game> pGame);

	//Request to quit the application and close all windows
	//@param exitCode is the error code to return to the invoking process
	void Quit(int exitCode = 0);

	//Get the Direct3D Device
	Microsoft::WRL::ComPtr<ID3D12Device2> GetDevice() const;

	//Get a specific type of command queue
	std::shared_ptr<CommandQueue> GetCommandQueue(D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT) const;

	//Flush all commandqueues
	void Flush();

	//Create the descriptor heap and its increment size
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(UINT numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE type);
	UINT GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE type) const;

protected:
	Application(HINSTANCE hInstance);

	virtual ~Application();

	Microsoft::WRL::ComPtr<ID3D12Device2> CreateDevice(Microsoft::WRL::ComPtr<IDXGIAdapter4> adapter);
	Microsoft::WRL::ComPtr<IDXGIAdapter4> GetAdapter(bool useWarp);

	bool CheckTearingSupport();

private:
	Application(const Application& copy) = delete;
	Application& operator=(const Application& other) = delete;

	HINSTANCE m_hInstance{};
	Microsoft::WRL::ComPtr<IDXGIAdapter4> m_Adapter{};
	Microsoft::WRL::ComPtr<ID3D12Device2> m_Device{};
	std::shared_ptr<CommandQueue> m_DirectCommandQueue{};
	std::shared_ptr<CommandQueue> m_CopyCommandQueue{};
	std::shared_ptr<CommandQueue> m_ComputeCommandQueue{};

	bool m_TearingSupported{};

};