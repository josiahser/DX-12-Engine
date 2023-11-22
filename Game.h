#pragma once

#include "Events.h"
#include "framework.h"

class Window;

class Game : public std::enable_shared_from_this<Game>
{
public:
	Game(const std::wstring& name, int width, int height, bool vSync);
	virtual ~Game();

	int GetClientWidth() const
	{
		return m_Width;
	}

	int GetClientHeight() const
	{
		return m_Height;
	}

	//Initialize the DX runtime
	virtual bool Initialize();

	//Load required content
	virtual bool LoadContent() = 0;

	//Unload specific content that was loaded in LoadContent
	virtual void UnloadContent() = 0;

	//Destroy any resource that are used by the game
	virtual void Destroy();

protected:
	friend class Window;

	//Update the game logic (When the WM_PAINT message is recieved by the window)
	virtual void OnUpdate(UpdateEventArgs& e);

	//Render stuff (immediately after the OnUpdate function)
	virtual void OnRender(RenderEventArgs& e);

	//invoked by the registered window whn a key is pressed while the window has focus
	virtual void OnKeyPressed(KeyEventArgs& e);

	//When key is released
	virtual void OnKeyRelease(KeyEventArgs& e);

	//When mouse is moved over the window
	virtual void OnMouseMoved(MouseMotionEventArgs& e);

	//When mouse button is pressed over the window
	virtual void OnMouseButtonPressed(MouseButtonEventArgs& e);

	//When mouse button is released over the window
	virtual void OnMouseButtonReleased(MouseButtonEventArgs& e);

	//When mouse wheel is scrolled over window
	virtual void OnMouseWheel(MouseWheelEventArgs& e);

	//When the window is resized (Don't have to worry about swap chain buffers, as window class handles that. resize any resources allocated specifically for derived class
	virtual void OnResize(ResizeEventArgs& e);

	//When the window is destroyed (default is to call the unload content method
	virtual void OnWindowDestroy();

	std::shared_ptr<Window> m_pWindow{};

private:
	std::wstring m_Name{};
	int m_Width{};
	int m_Height{};
	bool m_vSync{};
};