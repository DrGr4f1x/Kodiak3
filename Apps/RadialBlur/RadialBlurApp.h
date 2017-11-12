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
#include "PipelineState.h"
#include "RenderPass.h"
#include "RootSignature.h"

class RadialBlurApp : public Kodiak::Application
{
public:
	RadialBlurApp() : Application("Radial Blur") {}

	void Configure() final;
	void Startup() final;
	void Shutdown() final;

	void Render() final;

private:
	void InitRenderPasses();
	void InitRootSigs();
	void InitPSOs();

private:
	// Vertex layout used in this example
	struct Vertex
	{
		float position[3];
		float uv[2];
		float color[3];
		float normal[3];
	};

	Kodiak::RenderPass		m_renderPass;
	Kodiak::RenderPass		m_offscreenRenderPass;

	Kodiak::RootSignature	m_radialBlurRootSig;
	Kodiak::RootSignature	m_sceneRootSig;

	Kodiak::GraphicsPSO		m_radialBlurPSO;
	Kodiak::GraphicsPSO		m_colorPassPSO;
	Kodiak::GraphicsPSO		m_phongPassPSO;
	Kodiak::GraphicsPSO		m_offscreenDisplayPSO;
};