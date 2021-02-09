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
#include "Graphics\Model.h"
#include "Graphics\PipelineState.h"
#include "Graphics\ResourceSet.h"
#include "Graphics\RootSignature.h"


class SmokeSimApp : public Kodiak::Application
{
public:
	SmokeSimApp()
		: Application("SmokeSim")
		, m_controller(m_camera, Math::Vector3(Math::kYUnitVector))
	{}

	void Configure() final;
	void Startup() final;
	void Shutdown() final;

	bool Update() final;
	void Render() final;

private:
	void InitRootSigs();
	void InitPSOs();
	void InitConstantBuffer();
	void InitResourceSets();

	void UpdateConstantBuffer();

	void LoadAssets();

private:
	struct Vertex
	{
		float position[3];
		float normal[3];
		float uv[2];
	};

	struct Constants
	{
		Math::Matrix4 projectionMatrix;
		Math::Matrix4 modelMatrix;
	};

	Kodiak::RootSignature		m_meshRootSig;
	Kodiak::GraphicsPSO			m_meshPSO;

	Kodiak::ConstantBuffer		m_constantBuffer;
	Constants					m_constants;

	Kodiak::ResourceSet			m_meshResources;

	Kodiak::ModelPtr			m_model;
	Math::Matrix4				m_modelMatrix{ Math::kIdentity };

	// Camera controls
	Kodiak::CameraController	m_controller;
};