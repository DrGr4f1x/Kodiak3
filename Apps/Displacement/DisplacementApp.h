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
#include "ResourceSet.h"
#include "RootSignature.h"
#include "Texture.h"

class DisplacementApp : public Kodiak::Application
{
public:
	DisplacementApp()
		: Application("Displacement")
		, m_controller(m_camera, Math::Vector3(Math::kYUnitVector))
	{}

	void Configure() final;
	void Startup() final;
	void Shutdown() final;

	bool Update() final;
	void UpdateUI() final;
	void Render() final;

private:
	void InitRootSig();
	void InitPSOs();
	void InitConstantBuffers();
	void InitResourceSet();

	void UpdateConstantBuffers();

	void LoadAssets();

private:
	struct Vertex
	{
		float position[3];
		float normal[3];
		float uv[2];
	};

	struct HSConstants
	{
		float tessLevel;
	};

	struct DSConstants
	{
		Math::Matrix4 projectionMatrix;
		Math::Matrix4 modelMatrix;
		Math::Vector4 lightPos;
		float tessAlpha;
		float tessStrength;
	};

	Kodiak::RootSignature	m_rootSig;
	Kodiak::GraphicsPSO		m_pso;
	Kodiak::GraphicsPSO		m_wireframePSO;

	Kodiak::ConstantBuffer	m_hsConstantBuffer;
	HSConstants				m_hsConstants;

	Kodiak::ConstantBuffer	m_dsConstantBuffer;
	DSConstants				m_dsConstants;

	Kodiak::ResourceSet		m_resources;

	Kodiak::TexturePtr		m_texture;
	Kodiak::ModelPtr		m_model;

	Kodiak::CameraController m_controller;

	// App features
	bool m_split{ true };
	bool m_displacement{ true };
};