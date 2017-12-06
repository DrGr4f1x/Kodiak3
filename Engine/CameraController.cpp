//
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author:  David Elder
//

#include "Stdafx.h"

#include "CameraController.h"

#include "Camera.h"
#include "Input.h"


using namespace Kodiak;
using namespace Math;


CameraController::CameraController(Camera& camera, Vector3 worldUp) : m_targetCamera(camera)
{
	m_worldUp = Normalize(worldUp);
	m_worldNorth = Normalize(Cross(m_worldUp, Vector3(kXUnitVector)));
	m_worldEast = Cross(m_worldNorth, m_worldUp);

	m_horizontalLookSensitivity = 2.0f;
	m_verticalLookSensitivity = 2.0f;
	m_moveSpeed = 1000.0f;
	m_strafeSpeed = 1000.0f;
	m_mouseSensitivityX = 1.0f;
	m_mouseSensitivityY = 1.0f;

	m_currentPitch = Sin(Dot(camera.GetForwardVec(), m_worldUp));

	Vector3 forward = Normalize(Cross(m_worldUp, camera.GetRightVec()));
	m_currentHeading = ATan2(-Dot(forward, m_worldEast), Dot(forward, m_worldNorth));

	m_speedScale = 1.0f;

	m_fineMovement = false;
	m_fineRotation = false;
	m_momentum = true;

	m_lastYaw = 0.0f;
	m_lastPitch = 0.0f;
	m_lastForward = 0.0f;
	m_lastStrafe = 0.0f;
	m_lastAscent = 0.0f;
}


void CameraController::Update(float deltaTime)
{
	(deltaTime);

	float timeScale = 1.0f;
	timeScale *= m_speedScale;

	if (g_input.IsFirstPressed(DigitalInput::kLThumbClick) || g_input.IsFirstPressed(DigitalInput::kKey_lshift))
	{
		m_fineMovement = !m_fineMovement;
	}

	if (g_input.IsFirstPressed(DigitalInput::kRThumbClick))
	{
		m_fineRotation = !m_fineRotation;
	}

	float speedScale = (m_fineMovement ? 0.1f : 1.0f) * timeScale;
	float panScale = (m_fineRotation ? 0.5f : 1.0f) * timeScale;

	float yaw = g_input.GetTimeCorrectedAnalogInput(AnalogInput::kAnalogRightStickX) * m_horizontalLookSensitivity * panScale;
	float pitch = g_input.GetTimeCorrectedAnalogInput(AnalogInput::kAnalogRightStickY) * m_verticalLookSensitivity * panScale;

	float forward = m_moveSpeed * speedScale * (
		g_input.GetTimeCorrectedAnalogInput(AnalogInput::kAnalogLeftStickY) +
		(g_input.IsPressed(DigitalInput::kKey_w) ? deltaTime : 0.0f) +
		(g_input.IsPressed(DigitalInput::kKey_s) ? -deltaTime : 0.0f));

	float strafe = m_strafeSpeed * speedScale * (
		g_input.GetTimeCorrectedAnalogInput(AnalogInput::kAnalogLeftStickX) +
		(g_input.IsPressed(DigitalInput::kKey_d) ? deltaTime : 0.0f) +
		(g_input.IsPressed(DigitalInput::kKey_a) ? -deltaTime : 0.0f));

	float ascent = m_strafeSpeed * speedScale * (
		g_input.GetTimeCorrectedAnalogInput(AnalogInput::kAnalogRightTrigger) -
		g_input.GetTimeCorrectedAnalogInput(AnalogInput::kAnalogLeftTrigger) +
		(g_input.IsPressed(DigitalInput::kKey_e) ? deltaTime : 0.0f) +
		(g_input.IsPressed(DigitalInput::kKey_q) ? -deltaTime : 0.0f));

	if (m_momentum)
	{
		ApplyMomentum(m_lastYaw, yaw, deltaTime);
		ApplyMomentum(m_lastPitch, pitch, deltaTime);
		ApplyMomentum(m_lastForward, forward, deltaTime);
		ApplyMomentum(m_lastStrafe, strafe, deltaTime);
		ApplyMomentum(m_lastAscent, ascent, deltaTime);
	}

	// don't apply momentum to mouse inputs
	yaw += g_input.GetAnalogInput(AnalogInput::kAnalogMouseX) * m_mouseSensitivityX;
	pitch += g_input.GetAnalogInput(AnalogInput::kAnalogMouseY) * m_mouseSensitivityY;

	m_currentPitch += pitch;
	m_currentPitch = DirectX::XMMin(XM_PIDIV2, m_currentPitch);
	m_currentPitch = DirectX::XMMax(-XM_PIDIV2, m_currentPitch);

	m_currentHeading -= yaw;
	if (m_currentHeading > XM_PI)
	{
		m_currentHeading -= XM_2PI;
	}
	else if (m_currentHeading <= -XM_PI)
	{
		m_currentHeading += XM_2PI;
	}

	Matrix3 orientation = Matrix3(m_worldEast, m_worldUp, -m_worldNorth) * Matrix3::MakeYRotation(m_currentHeading) * Matrix3::MakeXRotation(m_currentPitch);
	Vector3 position = orientation * Vector3(strafe, ascent, -forward) + m_targetCamera.GetPosition();
	m_targetCamera.SetTransform(AffineTransform(orientation, position));
	m_targetCamera.Update();
}


void CameraController::ApplyMomentum(float& oldValue, float& newValue, float deltaTime)
{
	float blendedValue;
	if (Abs(newValue) > Abs(oldValue))
	{
		blendedValue = Lerp(newValue, oldValue, Pow(0.6f, deltaTime * 60.0f));
	}
	else
	{
		blendedValue = Lerp(newValue, oldValue, Pow(0.8f, deltaTime * 60.0f));
	}
	oldValue = blendedValue;
	newValue = blendedValue;
}