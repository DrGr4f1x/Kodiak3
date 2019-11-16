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
#include "GpuBuffer.h"
#include "Model.h"
#include "PipelineState.h"
#include "ResourceSet.h"
#include "RootSignature.h"
#include "Texture.h"

class RadialBlurApp : public Kodiak::Application
{
public:
	RadialBlurApp() : Application("Radial Blur") {}

	void Startup() final;
	void Shutdown() final;

	bool Update() final;
	void Render() final;

private:
	void InitRootSigs();
	void InitPSOs();
	void InitFramebuffers();
	void InitConstantBuffers();
	void InitResourceSets();

	void LoadAssets();

	void UpdateConstantBuffers();

private:
	// Render pipeline resources
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
		Math::Matrix4 modelMat;
		float gradientPos{ 0.0f };
	};

	struct RadialBlurConstants
	{
		float radialBlurScale = { 0.35f };
		float radialBlurStrength = { 0.75f };
		float radialOrigin[2] = { 0.5f, 0.5f };
	};

	static const uint32_t	s_offscreenSize{ 512 };

	Kodiak::FrameBufferPtr	m_offscreenFramebuffer;

	Kodiak::RootSignature	m_radialBlurRootSig;
	Kodiak::RootSignature	m_sceneRootSig;

	Kodiak::GraphicsPSO		m_radialBlurPSO;
	Kodiak::GraphicsPSO		m_colorPassPSO;
	Kodiak::GraphicsPSO		m_phongPassPSO;

	// Constant buffers
	SceneConstants			m_sceneConstants;
	Kodiak::ConstantBuffer	m_sceneConstantBuffer;

	RadialBlurConstants		m_radialBlurConstants;
	Kodiak::ConstantBuffer	m_radialBlurConstantBuffer;

	// Resource sets
	Kodiak::ResourceSet		m_sceneResources;
	Kodiak::ResourceSet		m_blurResources;

	// Assets
	Kodiak::ModelPtr		m_model;
	Kodiak::TexturePtr		m_gradientTex;

	// Camera controls
	float m_zoom{ -10.0f };
	Math::Vector3 m_cameraPos{ Math::kZero };

	// Features
	bool m_blur{ true };
	Math::Vector3 m_rotation{ -16.25f, -28.75f, 0.0f };
};