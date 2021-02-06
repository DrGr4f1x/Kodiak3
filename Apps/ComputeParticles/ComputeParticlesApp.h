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
#include "Graphics\ColorBuffer.h"
#include "GpuBuffer.h"
#include "PipelineState.h"
#include "ResourceSet.h"
#include "RootSignature.h"
#include "Texture.h"

class ComputeParticlesApp : public Kodiak::Application
{
public:
	ComputeParticlesApp() : Application("Compute Particles") {}

	void Startup() final;
	void Shutdown() final;

	bool Update() final;
	void UpdateUI() final;
	void Render() final;

private:
	void InitRootSigs();
	void InitPSOs();
	void InitConstantBuffers();
	void InitParticles();
	void InitResourceSets();

	void UpdateConstantBuffers();

	void LoadAssets();

private:
	struct Particle
	{
		float pos[2];
		float vel[2];
		Math::Vector4 gradientPos;
	};
#if DX12
	const uint32_t m_particleCount{ 256 * 1024 };
#elif VK
	const uint32_t m_particleCount{ 256 * 1024 };
#endif

	Kodiak::StructuredBuffer m_particleBuffer;

	struct CSConstants
	{
		float deltaT;
		float destX;
		float destY;
		int particleCount;
	};

	CSConstants m_csConstants;
	Kodiak::ConstantBuffer m_csConstantBuffer;

	struct VSConstants
	{
		float invTargetSize[2];
		float pointSize;
	};

	VSConstants m_vsConstants;
	Kodiak::ConstantBuffer m_vsConstantBuffer;

	Kodiak::RootSignature	m_graphicsRootSig;
	Kodiak::RootSignature	m_computeRootSig;

	Kodiak::GraphicsPSO		m_graphicsPSO;
	Kodiak::ComputePSO		m_computePSO;

	Kodiak::TexturePtr		m_colorTexture;
	Kodiak::TexturePtr		m_gradientTexture;

	Kodiak::ResourceSet		m_computeResources;
	Kodiak::ResourceSet		m_gfxResources;

	bool m_animate{ true };
	float m_animStart{ 20.0f };
	float m_localTimer{ 0.0f };
};