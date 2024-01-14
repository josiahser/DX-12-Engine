#pragma once

#include "Events.h"

#include "HighResolutionTimer.h"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <memory>
#include <string>

//Forward declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

class Window
{
public:
	//Get a handle to this windows instance, or nullptr if it's not a valid window
	HWND GetWindowHandle() const;

	//Get the current (normalized) DPI scaling for this window
	float GetDPIScaling() const;

	//Get the name that was used to create the window
	const std::wstring& GetWindowName() const;

	//Set the window title
	void SetWindowTitle(const std::wstring& windowTitle);

	//Get the current title of the window
	const std::wstring& GetWindowTitle() const;

	//Get the height of the window's client area
	int GetClientHeight() const
	{
		return m_ClientHeight;
	}

	//Get the width of the window's client area
	int GetClientWidth() const
	{
		return m_ClientWidth;
	}

	//Is this a windowed window or full-screen?
	bool IsFullscreen() const;

	//Set the fullscreen state of the window
	void SetFullscreen(bool fullscreen);
	void ToggleFullscreen();

	//Show this window
	void Show();

	//Hide the window
	void Hide();

	/**
	* Invoked when the game should be updated.
	*/
	UpdateEvent Update;

	/**
	 * The DPI scaling of the window has changed.
	 */
	DPIScaleEvent DPIScaleChanged;

	/**
	 * Window close event is fired when the window is about to be closed.
	 */
	WindowCloseEvent Close;

	/**
	 * Invoked when the window is resized.
	 */
	ResizeEvent Resize;

	/**
	 * Invoked when the window is minimized.
	 */
	ResizeEvent Minimized;

	/**
	 * Invoked when the window is maximized.
	 */
	ResizeEvent Maximized;

	/**
	 * Invoked when the window is restored.
	 */
	ResizeEvent Restored;

	/**
	 * Invoked when a keyboard key is pressed while the window has focus.
	 */
	KeyboardEvent KeyPressed;

	/**
	 * Invoked when a keyboard key is released while the window has focus.
	 */
	KeyboardEvent KeyReleased;

	/**
	 * Invoked when the window gains keyboard focus.
	 */
	Event KeyboardFocus;

	/**
	 * Invoked when the window loses keyboard focus.
	 */
	Event KeyboardBlur;

	/**
	 * Invoked when the mouse is moved over the window.
	 */
	MouseMotionEvent MouseMoved;

	/**
	 * Invoked when the mouse enters the client area.
	 */
	MouseMotionEvent MouseEnter;

	/**
	 * Invoked when the mouse button is pressed over the window.
	 */
	MouseButtonEvent MouseButtonPressed;

	/**
	 * Invoked when the mouse button is released over the window.
	 */
	MouseButtonEvent MouseButtonReleased;

	/**
	 * Invoked when the mouse wheel is scrolled over the window.
	 */
	MouseWheelEvent MouseWheel;

	/**
	 * Invoked when the mouse cursor leaves the client area.
	 */
	Event MouseLeave;

	/**
	 * Invoked when the window gains mouse focus.
	 */
	Event MouseFocus;

	/**
	 * Invoked when the window looses mouse focus.
	 */
	Event MouseBlur;


protected:
	//The window proc needs to call protected methods of this class
	friend LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

	//Only the application can create a window
	friend class Application;

	Window(HWND hWnd, const std::wstring& windowName, int clientWidth, int clientHeight);
	virtual ~Window();

	// Update game
	virtual void OnUpdate(UpdateEventArgs& e);

	// The DPI scaling of the window has changed.
	virtual void OnDPIScaleChanged(DPIScaleEventArgs& e);

	// Window was closed.
	virtual void OnClose(WindowCloseEventArgs& e);

	// Window was resized.
	virtual void OnResize(ResizeEventArgs& e);

	// Window was minimized.
	virtual void OnMinimized(ResizeEventArgs& e);

	// Window was maximized.
	virtual void OnMaximized(ResizeEventArgs& e);

	// Window was restored.
	virtual void OnRestored(ResizeEventArgs& e);

	// A keyboard key was pressed.
	virtual void OnKeyPressed(KeyEventArgs& e);
	// A keyboard key was released
	virtual void OnKeyReleased(KeyEventArgs& e);
	// Window gained keyboard focus
	virtual void OnKeyboardFocus(EventArgs& e);
	// Window lost keyboard focus
	virtual void OnKeyboardBlur(EventArgs& e);

	// The mouse was moved
	virtual void OnMouseMoved(MouseMotionEventArgs& e);
	// A button on the mouse was pressed
	virtual void OnMouseButtonPressed(MouseButtonEventArgs& e);
	// A button on the mouse was released
	virtual void OnMouseButtonReleased(MouseButtonEventArgs& e);
	// The mouse wheel was moved.
	virtual void OnMouseWheel(MouseWheelEventArgs& e);

	// The mouse entered the client area.
	virtual void OnMouseEnter(MouseMotionEventArgs& e);
	// The mouse left the client are of the window.
	virtual void OnMouseLeave(EventArgs& e);
	// The application window has received mouse focus
	virtual void OnMouseFocus(EventArgs& e);
	// The application window has lost mouse focus
	virtual void OnMouseBlur(EventArgs& e);


private:
	HWND m_hWnd;

	std::wstring m_Name;
	std::wstring m_Title;

	uint32_t m_ClientWidth;
	uint32_t m_ClientHeight;

	int32_t m_PreviousMouseX;
	int32_t m_PreviousMouseY;
	
	//Get the current dpi scaling of the window
	float m_DPIScaling;

	//Get the current fullscreen state of the window
	bool m_IsFullscreen;

	//True if window is minimized
	bool m_IsMinimized;

	//True if window is maxed
	bool m_IsMaximized;

	//This is true when the mouse is inside the window's client rect
	bool m_bInClientRect;
	RECT m_WindowRect;

	//This is set to true when the window receives keyboard focus
	bool m_bHasKeyboardFocus;

	HighResolutionTimer m_Timer;
};