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

	//Load content for the demo
	virtual bool LoadContent() override;

	//Unload demo content that was loaded in LoadContent()
	virtual void UnloadContent() override;

protected:
	//Update the game logic
	virtual void OnUpdate(UpdateEventArgs& e) override;

	//Render the stuff
	virtual void OnRender(RenderEventArgs& e) override;

	//Invoked by the window when a key is pressed while it has focus
	virtual void OnKeyPressed(KeyEventArgs& e) override;

	//Invoked when said key is released
	virtual void OnKeyReleased(KeyEventArgs& e);

	//Invoked when mouse is moved over the window
	virtual void OnMouseMoved(MouseMotionEventArgs& e);

	//Invoked when mouse wheel is scrolled
	virtual void OnMouseWheel(MouseWheelEventArgs& e) override;

	virtual void OnResize(ResizeEventArgs& e) override;

private:
	//Member variables here below
	//Some geometry to render
	std::unique_ptr<Mesh> m_CubeMesh;
	std::unique_ptr<Mesh> m_SphereMesh;
	std::unique_ptr<Mesh> m_ConeMesh;
	std::unique_ptr<Mesh> m_TorusMesh;
	std::unique_ptr<Mesh> m_PlaneMesh;

	Texture m_DefaultTexture;
	Texture m_DirectXTexture;
	Texture m_EarthTexture;
	Texture m_MonaLisaTexture;

	//Render target
	RenderTarget m_RenderTarget;

	//Root signature
	RootSignature m_RootSignature;

	//Pipeline state object
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_PipelineState;

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

	//Define some lights
	std::vector<PointLight> m_PointLights;
	std::vector<SpotLight> m_SpotLights;
};