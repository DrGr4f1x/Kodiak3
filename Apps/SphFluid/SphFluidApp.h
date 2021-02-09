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

#include "Application.h"
#include "CameraController.h"
#include "Graphics\GpuBuffer.h"
#include "Graphics\PipelineState.h"
#include "Graphics\ResourceSet.h"
#include "Graphics\RootSignature.h"


class SphFluidApp : public Kodiak::Application
{
public:
	SphFluidApp()
		: Application("SphFluid")
		, m_controller(m_camera, Math::Vector3(Math::kYUnitVector))
	{}

	void Configure() final;
	void Startup() final;
	void Shutdown() final;

	bool Update() final;
	void Render() final;

private:
	// Camera controls
	Kodiak::CameraController m_controller;
};