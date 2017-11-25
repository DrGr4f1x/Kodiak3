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
#include "DepthBuffer.h"
#include "Framebuffer.h"
#include "GpuBuffer.h"
#include "Model.h"
#include "PipelineState.h"
#include "RenderPass.h"
#include "RootSignature.h"
#include "Texture.h"

class RadialBlurApp : public Kodiak::Application
{
public:
	RadialBlurApp() : Application("Radial Blur") {}

	void Configure() final;
	void Startup() final;
	void Shutdown() final;

	bool Update() final;
	void Render() final;

private:
	void InitRenderPasses();
	void InitRootSigs();
	void InitPSOs();
	void InitFramebuffers();
	void InitConstantBuffers();

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

	Kodiak::RenderPass		m_renderPass;
	Kodiak::RenderPass		m_offscreenRenderPass;

	Kodiak::DepthBufferPtr	m_depthBuffer;
	Kodiak::FrameBufferPtr	m_offscreenFramebuffer;
	std::vector<Kodiak::FrameBufferPtr> m_framebuffers;

	Kodiak::RootSignature	m_radialBlurRootSig;
	Kodiak::RootSignature	m_sceneRootSig;

	Kodiak::GraphicsPSO		m_radialBlurPSO;
	Kodiak::GraphicsPSO		m_colorPassPSO;
	Kodiak::GraphicsPSO		m_phongPassPSO;
	Kodiak::GraphicsPSO		m_offscreenDisplayPSO;

	// Constant buffers
	SceneConstants			m_sceneConstants;
	Kodiak::ConstantBuffer	m_sceneConstantBuffer;

	// Assets
	Kodiak::ModelPtr		m_model;
	Kodiak::TexturePtr		m_gradientTex;

	// Camera controls
	float m_zoom{ -2.5f };
	Math::Vector3 m_cameraPos{ Math::kZero };
};