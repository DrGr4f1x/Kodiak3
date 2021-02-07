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


class StencilBufferApp : public Kodiak::Application
{
public:
	StencilBufferApp() 
		: Application("Stencil Buffer")
		, m_controller(m_camera, Math::Vector3(Math::kYUnitVector))
	{}

	void Startup() final;
	void Shutdown() final;

	bool Update() final;
	void UpdateUI() final;
	void Render() final;

private:
	void InitRootSig();
	void InitPSOs();
	void InitConstantBuffer();
	void InitResourceSet();

	void UpdateConstantBuffer();

	void LoadAssets();

private:
	struct Constants
	{
		Math::Matrix4		viewProjectionMatrix;
		Math::Matrix4		modelMatrix;
		Math::Vector4		lightPos{ 0.0f, -2.0f, 1.0f, 0.0f };
		float				outlineWidth = 0.05f;
	};

	Kodiak::RootSignature	m_rootSig;
	Kodiak::GraphicsPSO		m_toonPSO;
	Kodiak::GraphicsPSO		m_outlinePSO;

	Kodiak::ConstantBuffer	m_constantBuffer;
	Constants				m_constants;

	Kodiak::ResourceSet		m_resources;

	Kodiak::ModelPtr		m_model;

	Kodiak::CameraController m_controller;
};