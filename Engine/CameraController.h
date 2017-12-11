//
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author:  David Elder
//

#pragma once

#include "Camera.h"

namespace Kodiak
{

class CameraController
{
public:
	// Assumes worldUp is not the X basis vector
	CameraController(Math::Camera& camera, Math::Vector3 worldUp);

	void Update(float dt);

	void RefreshFromCamera();

	void SlowMovement(bool enable) { m_fineMovement = enable; }
	void SlowRotation(bool enable) { m_fineRotation = enable; }

	void EnableMomentum(bool enable) { m_momentum = enable; }

	void SetSpeedScale(float scale) { m_speedScale = scale; }

private:
	CameraController& operator=(const CameraController&) { return *this; }

	void ApplyMomentum(float& oldValue, float& newValue, float deltaTime);

	Math::Vector3 m_worldUp;
	Math::Vector3 m_worldNorth;
	Math::Vector3 m_worldEast;
	Math::Camera& m_targetCamera;
	float m_horizontalLookSensitivity;
	float m_verticalLookSensitivity;
	float m_moveSpeed;
	float m_strafeSpeed;
	float m_mouseSensitivityX;
	float m_mouseSensitivityY;

	float m_currentHeading;
	float m_currentPitch;

	float m_speedScale;

	bool m_fineMovement;
	bool m_fineRotation;
	bool m_momentum;

	float m_lastYaw;
	float m_lastPitch;
	float m_lastForward;
	float m_lastStrafe;
	float m_lastAscent;
};

} // namespace Kodiak