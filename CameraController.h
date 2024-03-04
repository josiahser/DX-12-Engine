#pragma once

#include "Application.h"

#include <memory>

namespace gainput
{
    class InputMap;
}

class Camera;

class CameraController
{
public:
	enum Actions
	{
        LMB,    // Is the left-mouse button pressed?
        RMB,    // Is the right-mouse button pressed?
        MoveX,  // Move Left/right.
        MoveY,  // Move Forward/backward.
        MoveZ,  // Move Up/down.
        ZoomIn, // Zoom camera towards focal point.
        ZoomOut,// Zoom camera away from focal point.
        Pitch,  // Look up/down
        Yaw,    // Look left/right.
        Boost,  // Move/look faster
	};

    CameraController(Camera& camera);

    //Reset view to default settings
    void ResetView();

    //Update camera based on mouse, keyboard events
    //CameraController assumes that the InputManager is updated correctly in the main loop
    void Update(UpdateEventArgs& e);

    //Whether pitch should be inverted
    void SetInverseY(bool inverseY)
    {
        m_InverseY = inverseY;
    }

    bool IsInverseY() const
    {
        return m_InverseY;
    }

private:
    Camera& m_Camera;
    //input
    std::shared_ptr<gainput::InputMap> m_KMInput;

    //Gamepad input
    std::shared_ptr<gainput::InputMap> m_PadInput;

    Logger m_Logger;

    //store previous values to apply smoothing
    float m_X;
    float m_Y;
    float m_Z;
    float m_Zoom;

    //Limit rotation to pitch and yaw
    float m_Pitch;
    float m_Yaw;
    //for smoothing
    float m_PreviousPitch;
    float m_PreviousYaw;

    bool m_InverseY;
};
