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
#include "RenderPass.h"
#include "RootSignature.h"
#include "Texture.h"

class ParticleFireApp : public Kodiak::Application
{
public:
	ParticleFireApp()
		: Kodiak::Application("Particle Fire")
		, m_controller(m_camera, Math::Vector3(Math::kYUnitVector))
	{
		m_timerSpeed = 2.0f;
	}

	void Configure() final;
	void Startup() final;
	void Shutdown() final;

	bool Update() final;
	void Render() final;

private:
	void InitRootSigs();
	void InitPSOs();
	void InitConstantBuffers();

	void UpdateConstantBuffers();

	void LoadAssets();

private:
	struct ModelVertex
	{
		float pos[3];
		float uv[2];
		float normal[3];
		float tangent[3];
		float bitangent[3];
	};

	struct ModelVSConstants
	{
		Math::Matrix4 projectionMatrix;
		Math::Matrix4 viewMatrix;
		Math::Matrix4 modelMatrix;
		Math::Matrix4 normalMatrix;
		Math::Vector4 lightPos;
		Math::Vector4 cameraPos;
	};

	Kodiak::RootSignature		m_rootSig;

	Kodiak::GraphicsPSO			m_modelPSO;
	Kodiak::GraphicsPSO			m_particlePSO;

	ModelVSConstants			m_modelVsConstants;
	Kodiak::ConstantBuffer		m_modelVsConstantBuffer;

	Kodiak::TexturePtr			m_modelColorTex;
	Kodiak::TexturePtr			m_modelNormalTex;
	Kodiak::TexturePtr			m_particleFireTex;
	Kodiak::TexturePtr			m_particleSmokeTex;
	Kodiak::ModelPtr			m_model;

	Kodiak::CameraController	m_controller;
	float						m_zoom{ -75.0f };
};