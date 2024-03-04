#pragma once

#include "Camera.h"
#include "CameraController.h"
#include "Light.h"

#include "Application.h"

#include "RenderTarget.h"

#include <d3d12.h>

#include <future>
#include <memory>
#include <string>

class CommandList;
class Device;
class GUI;
class PipelineStateObject;
class RenderTarget;
class RootSignature;
class Scene;
class SwapChain;

class EffectPSO;

class Demo
{
public:

	Demo(const std::wstring& name, int width, int height, bool vSync = false);
	virtual ~Demo();

	//Start the game loop and return the error code
	uint32_t Run();

	//Load content for the demo
	void LoadContent();

	//Unload demo content that was loaded in LoadContent()
	void UnloadContent();

protected:
	//Update the game logic
	void OnUpdate(UpdateEventArgs& e);

	void OnResize(ResizeEventArgs& e);

	//Render the stuff
	void OnRender(RenderEventArgs& e);

	//Invoked by the window when a key is pressed while it has focus
	void OnKeyPressed(KeyEventArgs& e);

	//Invoked when said key is released
	void OnKeyReleased(KeyEventArgs& e);

	//Invoked when mouse is moved over the window
	virtual void OnMouseMoved(MouseMotionEventArgs& e);

	//Handle DPI change events
	void OnDPIScaleChanged(DPIScaleEventArgs& e);

	//Render GUI
	void OnGUI(const std::shared_ptr<CommandList>& commandList, const RenderTarget& renderTarget);

	void OnWindowClosed(WindowCloseEventArgs& e);

private:
	//Load assets
	//Executed as an async task, so we can render a loading screen in main thread
	bool LoadScene(const std::wstring& sceneFile);

	//Opens a file dialog and loads a new scene file
	void OpenFile();

	/**
	* This function is called to report the loading progress of the scene. This is useful for updating the loading
	* progress bar.
	*
	* @param progress The loading progress (as a normalized float in the range [0...1].
	*
	* @returns true to continue loading or false to cancel loading.
	*/
	bool LoadingProgress(float loadingProgress);

	//Member variables here below
	//
	//
	// DX12 device
	std::shared_ptr<Device> m_Device;
	std::shared_ptr<SwapChain> m_SwapChain;
	std::shared_ptr<GUI> m_GUI;

	std::shared_ptr<Scene> m_Scene;

	//Some geometry to render
	std::shared_ptr<Scene> m_Cube;
	std::shared_ptr<Scene> m_Sphere;
	std::shared_ptr<Scene> m_Cone;
	std::shared_ptr<Scene> m_Torus;
	std::shared_ptr<Scene> m_Plane;
	std::shared_ptr<Scene> m_Axis;

	/*std::shared_ptr<Texture> m_DefaultTexture;
	std::shared_ptr<Texture> m_DirectXTexture;
	std::shared_ptr<Texture> m_EarthTexture;
	std::shared_ptr<Texture> m_MonaLisaTexture;*/

	//Pipeline state object for rendering the scene
	std::shared_ptr<EffectPSO> m_LightingPSO;
	std::shared_ptr<EffectPSO> m_DecalPSO;
	std::shared_ptr<EffectPSO> m_UnlitPSO;

	//Render target
	RenderTarget m_RenderTarget;

	std::shared_ptr<Window> m_Window; //Render window (from Application)

	D3D12_VIEWPORT m_Viewport;
	D3D12_RECT m_ScissorRect;

	//Root signature
	//std::shared_ptr<RootSignature> m_RootSignature;

	//Pipeline state object
	//std::shared_ptr<PipelineStateObject> m_PipelineState;
	//std::shared_ptr<PipelineStateObject> m_UnlitPipelineState;

	Camera m_Camera;
	CameraController m_CameraController;
	Logger m_Logger;

	int m_Height;
	int m_Width;
	bool m_VSync;

	//Define some lights
	std::vector<PointLight> m_PointLights;
	std::vector<SpotLight> m_SpotLights;
	std::vector<DirectionalLight> m_DirectionalLights;

	//Rotate the lights in a circle
	bool m_AnimateLights;

	bool              m_Fullscreen;
	bool              m_AllowFullscreenToggle;
	bool              m_ShowFileOpenDialog;
	bool              m_CancelLoading;
	bool              m_ShowControls;
	std::atomic_bool  m_IsLoading;
	std::future<bool> m_LoadingTask;
	float             m_LoadingProgress;
	std::string       m_LoadingText;

	float m_FPS;
};