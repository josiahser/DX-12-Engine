#pragma once

#include "Game.h"
#include "Window.h"
#include "framework.h"

class Demo : public Game
{
public:
	using super = Game;

	Demo(const std::wstring& name, int width, int height, bool vSync = false);

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

	//Invoked when mouse wheel is scrolled
	virtual void OnMouseWheel(MouseWheelEventArgs& e) override;

	virtual void OnResize(ResizeEventArgs& e) override;

private:
	//Helper functions
	//Transition a resource
	void TransitionResource(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
		Microsoft::WRL::ComPtr<ID3D12Resource> resource,
		D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState);

	//Clear a render target view
	void ClearRTV(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
		D3D12_CPU_DESCRIPTOR_HANDLE rtv, FLOAT* clearColor);

	//Clear the depth of a depth-stencil view
	void ClearDepth(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
		D3D12_CPU_DESCRIPTOR_HANDLE dsv, FLOAT depth = 1.0f);

	//Create a GPU buffer
	void UpdateBufferResource(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
		ID3D12Resource** pDestinationResource, ID3D12Resource** pIntermediateResource,
		size_t numElements, size_t elementSize, const void* bufferData,
		D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);

	//Resize the depth buffer to match the size of the client area
	void ResizeDepthBuffer(int width, int height);

	//Member variables here below

	uint64_t m_FenceValues[Window::bufferCount] = {};

	//Vertex buffer for the cube
	Microsoft::WRL::ComPtr<ID3D12Resource> m_VertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;

	//Index buffer for the cube
	Microsoft::WRL::ComPtr<ID3D12Resource> m_IndexBuffer;
	D3D12_INDEX_BUFFER_VIEW m_IndexBufferView{};

	//Depth Buffer
	Microsoft::WRL::ComPtr<ID3D12Resource> m_DepthBuffer;
	//Descriptor Heap for depth buffer
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DSVHeap;

	//Root signature
	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_RootSignature;
	
	//Pipeline state object
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_PipelineState;

	D3D12_VIEWPORT m_Viewport{};
	D3D12_RECT m_ScissorRect{};

	float m_FoV{};

	DirectX::XMMATRIX m_ModelMatrix{};
	DirectX::XMMATRIX m_ViewMatrix{};
	DirectX::XMMATRIX m_ProjectionMatrix{};

	bool m_ContentLoaded{};
};