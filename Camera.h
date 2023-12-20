#pragma once

#include <DirectXMath.h>

//When performing transformations on the camera, it is sometimes useful to express which space this transformation should be applied
enum class Space
{
	Local,
	World,
};

class Camera
{
public:

	Camera();
	virtual ~Camera();

	void XM_CALLCONV set_LookAt(DirectX::FXMVECTOR eye, DirectX::FXMVECTOR target, DirectX::FXMVECTOR up);
	DirectX::XMMATRIX get_ViewMatrix() const;
	DirectX::XMMATRIX get_InverseViewMatrix() const;

	///
	/// Set the camera to a perspective projection matrix
	/// @param fovy the vertical field of view in degrees
	/// @param aspect the aspect ratio of the screen
	/// @param zNear the distance to the near clipping plane
	/// @param zFar the distance to the far clipping plane
	///
	
	void set_Projection(float fovy, float aspect, float zNear, float zFar);
	DirectX::XMMATRIX get_ProjectionMatrix() const;
	DirectX::XMMATRIX get_InverseProjectionMatrix() const;

	//Set the field of view in degrees
	void set_FoV(float fovy);

	//Get the FoV in degrees
	float get_FoV() const;

	//Set the camera's position in world space
	void XM_CALLCONV set_Translation(DirectX::FXMVECTOR translation);
	DirectX::XMVECTOR get_Translation() const;

	//set the camera's rotation in world-space
	//@param rotation the rotation quaternion
	void XM_CALLCONV set_Rotation(DirectX::FXMVECTOR rotation);

	//Query the camera's rotation, returns the camera's rotation quaternion
	DirectX::XMVECTOR get_Rotation() const;

	void XM_CALLCONV Translate(DirectX::FXMVECTOR translation, Space space = Space::Local);
	void Rotate(DirectX::FXMVECTOR quaternion);

protected:
	virtual void UpdateViewMatrix() const;
	virtual void UpdateInverseViewMatrix() const;
	virtual void UpdateProjectionMatrix() const;
	virtual void UpdateInverseProjectionMatrix() const;

	//This data must be aligned otherwise the SSE intrinsics fail and throw exceptions
	__declspec(align(16)) struct AlignedData
	{
		//World-space position of the camera
		DirectX::XMVECTOR m_Translation;
		//World-Space rotation of the camera, a QUATERNION
		DirectX::XMVECTOR m_Rotation;

		DirectX::XMMATRIX m_ViewMatrix, m_InverseViewMatrix;
		DirectX::XMMATRIX m_ProjectionMatrix, m_InverseProjectionMatrix;
	};
	AlignedData* pData;

	//projection parameters
	float m_vFoV; //Vertical FoV
	float m_AspectRatio; //Aspect Ratio
	float m_zNear; //Near clip distance
	float m_zFar; //Far clip distance

	//True if the view matrix needs to be updated
	mutable bool m_ViewDirty, m_InverseViewDirty;
	//True if the projection matrix needs to be updated
	mutable bool m_ProjectionDirty, m_InverseProjectionDirty;

private:

};