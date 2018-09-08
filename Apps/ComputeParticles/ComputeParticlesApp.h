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
#include "ColorBuffer.h"
#include "GpuBuffer.h"
#include "PipelineState.h"
#include "RootSignature.h"
#include "Texture.h"

class ComputeParticlesApp : public Kodiak::Application
{
public:
	ComputeParticlesApp() : Kodiak::Application("Compute Particles") {}

	void Configure() final;
	void Startup() final;
	void Shutdown() final;

	bool Update() final;
	void Render() final;

private:
	void InitRootSigs();
	void InitPSOs();
	void InitConstantBuffers();

	void LoadAssets();

private:
	Kodiak::RootSignature	m_graphicsRootSig;
	Kodiak::RootSignature	m_computeRootSig;

	Kodiak::GraphicsPSO		m_graphicsPSO;
	Kodiak::ComputePSO		m_computePSO;

	Kodiak::TexturePtr		m_colorTexture;
	Kodiak::TexturePtr		m_gradientTexture;
};