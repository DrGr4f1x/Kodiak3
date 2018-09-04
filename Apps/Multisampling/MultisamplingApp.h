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
#include "Model.h"
#include "PipelineState.h"
#include "RootSignature.h"
#include "Texture.h"

class MultisamplingApp : public Kodiak::Application 
{
public:
	MultisamplingApp()
		: Kodiak::Application("Multisampling")
		, m_controller(m_camera, Math::Vector3(Math::kYUnitVector))
	{}

	void Configure() final;
	void Startup() final;
	void Shutdown() final;

	bool Update() final;
	void Render() final;

private:
	void InitRenderTargets();
	void InitRootSig();
	void InitPSO();
	void InitConstantBuffer();

	void LoadAssets();

	void UpdateConstantBuffer();

private:
	// Vertex layout for this example
	struct Vertex
	{
		float position[3];
		float normal[3];
		float uv[2];
		float color[3];
	};

	struct Constants
	{
		Math::Matrix4 viewProjectionMatrix;
		Math::Matrix4 modelMatrix;
		Math::Vector4 lightPos;
	};

	Kodiak::ColorBufferPtr		m_colorTarget;
	Kodiak::DepthBufferPtr		m_depthTarget;
	const uint32_t				m_numSamples{ 8 };

	Kodiak::FrameBufferPtr		m_frameBuffer;

	Constants					m_constants;
	Kodiak::ConstantBuffer		m_constantBuffer;

	Kodiak::RootSignature		m_rootSig;
	Kodiak::GraphicsPSO			m_pso;

	Kodiak::TexturePtr			m_texture;
	Kodiak::ModelPtr			m_model;

	Kodiak::CameraController	m_controller;
};