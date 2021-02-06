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
#include "ResourceSet.h"
#include "RootSignature.h"
#include "Texture.h"

class BloomApp : public Kodiak::Application
{
public:
	BloomApp() 
		: Application("Bloom") 
		, m_controller(m_camera, Math::Vector3(Math::kYUnitVector))
	{}

	void Startup() final;
	void Shutdown() final;

	bool Update() final;
	void UpdateUI() final;
	void Render() final;

private:
	void InitRootSigs();
	void InitPSOs();
	void InitFramebuffers();
	void InitConstantBuffers();
	void InitResourceSets();

	void LoadAssets();

	void UpdateConstantBuffers();
	void UpdateBlurConstants();

private:
	struct Vertex
	{
		float position[3];
		float uv[2];
		float color[3];
		float normal[3];
	};

	struct SceneConstants
	{
		Math::Matrix4 projectionMat;
		Math::Matrix4 viewMat;
		Math::Matrix4 modelMat;
	};

	struct BlurConstants
	{
		float blurScale;
		float blurStrength;
		int blurDirection;
	};

	Kodiak::FrameBufferPtr	m_offscreenFramebuffer[2];

	Kodiak::RootSignature	m_sceneRootSig;
	Kodiak::RootSignature	m_blurRootSig;
	Kodiak::RootSignature	m_skyboxRootSig;

	Kodiak::GraphicsPSO		m_colorPassPSO;
	Kodiak::GraphicsPSO		m_phongPassPSO;
	Kodiak::GraphicsPSO		m_blurVertPSO;
	Kodiak::GraphicsPSO		m_blurHorizPSO;
	Kodiak::GraphicsPSO		m_skyboxPSO;

	// Constant buffers
	SceneConstants			m_sceneConstants;
	Kodiak::ConstantBuffer	m_sceneConstantBuffer;

	SceneConstants			m_skyboxConstants;
	Kodiak::ConstantBuffer	m_skyboxConstantBuffer;

	BlurConstants			m_blurHorizConstants;
	Kodiak::ConstantBuffer	m_blurHorizConstantBuffer;
	BlurConstants			m_blurVertConstants;
	Kodiak::ConstantBuffer	m_blurVertConstantBuffer;

	// Resource sets
	Kodiak::ResourceSet		m_sceneResources;
	Kodiak::ResourceSet		m_skyboxResources;
	Kodiak::ResourceSet		m_blurHorizResources;
	Kodiak::ResourceSet		m_blurVertResources;

	// Assets
	Kodiak::ModelPtr		m_ufoModel;
	Kodiak::ModelPtr		m_ufoGlowModel;
	Kodiak::ModelPtr		m_skyboxModel;
	Kodiak::TexturePtr		m_skyboxTex;

	Kodiak::CameraController	m_controller;

	bool m_bloom{ true };
	float m_blurScale{ 1.0f };
};