//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Developed by Minigraph
//
// Author:  James Stanard 
//

#pragma once

#include "VectorMath.h"
#include "Math/Frustum.h"

namespace Math
{

class BaseCamera
{
public:

	// Call this function once per frame and after you've changed any state.  This
	// regenerates all matrices.  Calling it more or less than once per frame will break
	// temporal effects and cause unpredictable results.
	void Update();

	// Public functions for controlling where the camera is and its orientation
	void SetEyeAtUp(Vector3 eye, Vector3 at, Vector3 up);
	void SetLookDirection(Vector3 forward, Vector3 up);
	void SetRotation(Quaternion basisRotation);
	void SetPosition(Vector3 worldPos);
	void SetTransform(const AffineTransform& xform);
	void SetTransform(const OrthogonalTransform& xform);

	const Quaternion GetRotation() const { return m_cameraToWorld.GetRotation(); }
	const Vector3 GetRightVec() const { return m_basis.GetX(); }
	const Vector3 GetUpVec() const { return m_basis.GetY(); }
	const Vector3 GetForwardVec() const { return -m_basis.GetZ(); }
	const Vector3 GetPosition() const { return m_cameraToWorld.GetTranslation(); }

	// Accessors for reading the various matrices and frusta
	const Matrix4& GetViewMatrix() const { return m_viewMatrix; }
	const Matrix4& GetProjMatrix() const { return m_projMatrix; }
	const Matrix4& GetViewProjMatrix() const { return m_viewProjMatrix; }
	const Matrix4& GetReprojectionMatrix() const { return m_reprojectMatrix; }
	const Frustum& GetViewSpaceFrustum() const { return m_frustumVS; }
	const Frustum& GetWorldSpaceFrustum() const { return m_frustumWS; }

protected:
	BaseCamera() : m_cameraToWorld(kIdentity), m_basis(kIdentity) {}

	void SetProjMatrix(const Matrix4& ProjMat) { m_projMatrix = ProjMat; }

	OrthogonalTransform m_cameraToWorld;

	// Redundant data cached for faster lookups.
	Matrix3 m_basis;

	// Transforms homogeneous coordinates from world space to view space.  In this case, view space is defined as +X is
	// to the right, +Y is up, and -Z is forward.  This has to match what the projection matrix expects, but you might
	// also need to know what the convention is if you work in view space in a shader.
	Matrix4 m_viewMatrix;		// i.e. "World-to-View" matrix

								// The projection matrix transforms view space to clip space.  Once division by W has occurred, the final coordinates
								// can be transformed by the viewport matrix to screen space.  The projection matrix is determined by the screen aspect 
								// and camera field of view.  A projection matrix can also be orthographic.  In that case, field of view would be defined
								// in linear units, not angles.
	Matrix4 m_projMatrix;		// i.e. "View-to-Projection" matrix

								// A concatenation of the view and projection matrices.
	Matrix4 m_viewProjMatrix;	// i.e.  "World-To-Projection" matrix.

								// The view-projection matrix from the previous frame
	Matrix4 m_previousViewProjMatrix;

	// Projects a clip-space coordinate to the previous frame (useful for temporal effects).
	Matrix4 m_reprojectMatrix;

	Frustum m_frustumVS;		// View-space view frustum
	Frustum m_frustumWS;		// World-space view frustum
};


class Camera : public BaseCamera
{
public:
	Camera();

	// Controls the view-to-projection matrix
	void SetPerspectiveMatrix(float verticalFovRadians, float aspectHeightOverWidth, float nearZClip, float farZClip);
	void SetFOV(float verticalFovInRadians) { m_verticalFOV = verticalFovInRadians; UpdateProjMatrix(); }
	void SetAspectRatio(float heightOverWidth) { m_aspectRatio = heightOverWidth; UpdateProjMatrix(); }
	void SetZRange(float nearZ, float farZ) { m_nearClip = nearZ; m_farClip = farZ; UpdateProjMatrix(); }
	void ReverseZ(bool enable) { m_reverseZ = enable; UpdateProjMatrix(); }

	float GetFOV() const { return m_verticalFOV; }
	float GetNearClip() const { return m_nearClip; }
	float GetFarClip() const { return m_farClip; }
	float GetClearDepth() const { return m_reverseZ ? 0.0f : 1.0f; }

private:

	void UpdateProjMatrix();

	float m_verticalFOV;			// Field of view angle in radians
	float m_aspectRatio;
	float m_nearClip;
	float m_farClip;
	bool m_reverseZ;				// Invert near and far clip distances so that Z=0 is the far plane
};


inline void BaseCamera::SetEyeAtUp(Vector3 eye, Vector3 at, Vector3 up)
{
	SetLookDirection(at - eye, up);
	SetPosition(eye);
}


inline void BaseCamera::SetPosition(Vector3 worldPos)
{
	m_cameraToWorld.SetTranslation(worldPos);
}


inline void BaseCamera::SetTransform(const AffineTransform& xform)
{
	// By using these functions, we rederive an orthogonal transform.
	SetLookDirection(-xform.GetZ(), xform.GetY());
	SetPosition(xform.GetTranslation());
}


inline void BaseCamera::SetRotation(Quaternion basisRotation)
{
	m_cameraToWorld.SetRotation(Normalize(basisRotation));
	m_basis = Matrix3(m_cameraToWorld.GetRotation());
}


inline Camera::Camera() : m_reverseZ(true)
{
	SetPerspectiveMatrix(XM_PIDIV4, 9.0f / 16.0f, 1.0f, 1000.0f);
}


inline void Camera::SetPerspectiveMatrix(float verticalFovRadians, float aspectHeightOverWidth, float nearZClip, float farZClip)
{
	m_verticalFOV = verticalFovRadians;
	m_aspectRatio = aspectHeightOverWidth;
	m_nearClip = nearZClip;
	m_farClip = farZClip;

	UpdateProjMatrix();

	m_previousViewProjMatrix = m_viewProjMatrix;
}

} // namespace Math