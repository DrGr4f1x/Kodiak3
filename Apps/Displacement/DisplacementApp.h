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
#include "GpuBuffer.h"
#include "Model.h"
#include "PipelineState.h"
#include "RootSignature.h"

class DisplacementApp : public Kodiak::Application
{
public:
	DisplacementApp()
		: Kodiak::Application("Displacement")
		, m_controller(m_camera, Math::Vector3(Math::kYUnitVector))
	{}

	void Configure() final;
	void Startup() final;
	void Shutdown() final;

private:
	void InitRootSig();
	void InitPSOs();

private:
	struct Vertex
	{
		float position[3];
		float normal[3];
		float uv[2];
	};

	Kodiak::RootSignature	m_rootSig;
	Kodiak::GraphicsPSO		m_pso;

	Kodiak::CameraController m_controller;
};