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

#include "Stdafx.h"

#include "Camera.h"

#include <cmath>


using namespace Math;


void BaseCamera::SetLookDirection(Vector3 forward, Vector3 up)
{
	// Given, but ensure normalization
	Scalar forwardLenSq = LengthSquare(forward);
	forward = Select(forward * RecipSqrt(forwardLenSq), -Vector3(kZUnitVector), forwardLenSq < Scalar(0.000001f));

	// Deduce a valid, orthogonal right vector
	Vector3 right = Cross(forward, up);
	Scalar rightLenSq = LengthSquare(right);
	right = Select(right * RecipSqrt(rightLenSq), Quaternion(Vector3(kYUnitVector), -XM_PIDIV2) * forward, rightLenSq < Scalar(0.000001f));

	// Compute actual up vector
	up = Cross(right, forward);

	// Finish constructing basis
	m_basis = Matrix3(right, up, -forward);
	m_cameraToWorld.SetRotation(Quaternion(m_basis));
}


void BaseCamera::Update()
{
	m_previousViewProjMatrix = m_viewProjMatrix;

	m_viewMatrix = Matrix4(~m_cameraToWorld);
	m_viewProjMatrix = m_projMatrix * m_viewMatrix;
	m_reprojectMatrix = m_previousViewProjMatrix * Invert(GetViewProjMatrix());

	m_frustumVS = Frustum(m_projMatrix);
	m_frustumWS = m_cameraToWorld * m_frustumVS;
}


void Camera::UpdateProjMatrix()
{
	float Y = 1.0f / std::tanf(m_verticalFOV * 0.5f);
	float X = Y * m_aspectRatio;

	float Q1, Q2;

	// ReverseZ puts far plane at Z=0 and near plane at Z=1.  This is never a bad idea, and it's
	// actually a great idea with F32 depth buffers to redistribute precision more evenly across
	// the entire range.  It requires clearing Z to 0.0f and using a GREATER variant depth test.
	// Some care must also be done to properly reconstruct linear W in a pixel shader from hyperbolic Z.
	if (m_reverseZ)
	{
		Q1 = m_nearClip / (m_farClip - m_nearClip);
		Q2 = Q1 * m_farClip;
	}
	else
	{
		Q1 = m_farClip / (m_nearClip - m_farClip);
		Q2 = Q1 * m_nearClip;
	}

	SetProjMatrix(Matrix4(
		Vector4(X, 0.0f, 0.0f, 0.0f),
		Vector4(0.0f, Y, 0.0f, 0.0f),
		Vector4(0.0f, 0.0f, Q1, -1.0f),
		Vector4(0.0f, 0.0f, Q2, 0.0f)
	));
}
