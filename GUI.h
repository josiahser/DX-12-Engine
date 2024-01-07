#pragma once

#include "ImGUI/imgui.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <memory>

class CommandList;
class Device;
class PipelineStateObject;
class RenderTarget;
class RootSignature;
class ShaderResourceView;
class Texture;

class GUI
{
public:
	//Window message handler, needs to be called by the application to allow ImGUI to handle input messages
	LRESULT WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	//Begin a new ImGUI frame. Do this before calling any ImGui functions that modifies ImGui's render context
	void NewFrame();

	//Render ImGUI to the given render target
	void Render(const std::shared_ptr<CommandList>& commandList, const RenderTarget& renderTarget);

	//Destroy ImGUI context
	void Destroy();

	//Set the font scaling for ImGui(Should be called when the window's DPI scaling changes)
	void SetScaling(float scale);

protected:
	GUI(Device& device, HWND hWnd, const RenderTarget& renderTarget);
	virtual ~GUI();

private:
	Device& m_Device;
	HWND m_hWnd;
	ImGuiContext* m_pImGuiCtx;
	std::shared_ptr<Texture> m_FontTexture;
	std::shared_ptr<ShaderResourceView> m_FontSRV;
	std::shared_ptr<RootSignature> m_RootSignature;
	std::shared_ptr<PipelineStateObject> m_PipelineState;
};