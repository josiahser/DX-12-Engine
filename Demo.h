#pragma once

#include "Camera.h"
#include "Light.h"

#include "Events.h"
#include "Application.h"

#include "RenderTarget.h"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

class CommandList;
class Device;
class GUI;
class Mesh;
class RootSignature;
class PipelineStateObject;
class Scene;
class SwapChain;
class Texture;

class Window;

class Demo
{
public:

	Demo(const std::wstring& name, int width, int height, bool vSync = false);
	virtual ~Demo();

	//Start the game loop and return the error code
	uint32_t Run();

	//Load content for the demo
	bool LoadContent();

	//Unload demo content that was loaded in LoadContent()
	void UnloadContent();

protected:
	//Update the game logic
	void OnUpdate(UpdateEventArgs& e);

	//Render the stuff
	void OnRender();

	void OnGUI(const std::shared_ptr<CommandList>& commandList, const RenderTarget& renderTarget);

	//Invoked by the window when a key is pressed while it has focus
	void OnKeyPressed(KeyEventArgs& e);

	//Invoked when said key is released
	void OnKeyReleased(KeyEventArgs& e);

	//Invoked when mouse is moved over the window
	void OnMouseMoved(MouseMotionEventArgs& e);

	//Invoked when mouse wheel is scrolled
	void OnMouseWheel(MouseWheelEventArgs& e);

	void OnResize(ResizeEventArgs& e);

private:
	//Member variables here below
	std::shared_ptr<Window> m_Window; //Render window (from Application)
	
	// DX12 device
	std::shared_ptr<Device> m_Device;
	std::shared_ptr<SwapChain> m_SwapChain;
	std::shared_ptr<GUI> m_GUI;

	//Some geometry to render
	std::shared_ptr<Scene> m_Cube;
	std::shared_ptr<Scene> m_Sphere;
	std::shared_ptr<Scene> m_Cone;
	std::shared_ptr<Scene> m_Torus;
	std::shared_ptr<Scene> m_Plane;

	std::shared_ptr<Texture> m_DefaultTexture;
	std::shared_ptr<Texture> m_DirectXTexture;
	std::shared_ptr<Texture> m_EarthTexture;
	std::shared_ptr<Texture> m_MonaLisaTexture;

	//Render target
	RenderTarget m_RenderTarget;

	//Root signature
	std::shared_ptr<RootSignature> m_RootSignature;

	//Pipeline state object
	std::shared_ptr<PipelineStateObject> m_PipelineState;
	std::shared_ptr<PipelineStateObject> m_UnlitPipelineState;

	D3D12_VIEWPORT m_Viewport;
	D3D12_RECT m_ScissorRect;

	Camera m_Camera;
	struct alignas(16) CameraData
	{
		DirectX::XMVECTOR m_InitialCamPos;
		DirectX::XMVECTOR m_InitialCamRot;
	};
	CameraData* m_pAlignedCameraData;

	//Camera controller
	float m_Forward;
	float m_Backward;
	float m_Left;
	float m_Right;
	float m_Up;
	float m_Down;

	float m_Pitch;
	float m_Yaw;

	//Rotate the lights in a circle
	bool m_AnimateLights;
	//Set to true if the shift key is pressed
	bool m_Shift;

	int m_Height;
	int m_Width;
	bool m_VSync;

	//Define some lights
	std::vector<PointLight> m_PointLights;
	std::vector<SpotLight> m_SpotLights;

	//Logger for logging
	Logger m_Logger;
};