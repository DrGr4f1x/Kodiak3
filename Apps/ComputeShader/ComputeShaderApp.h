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
#include "ResourceSet.h"
#include "RootSignature.h"
#include "Texture.h"

class ComputeShaderApp : public Kodiak::Application
{
public:
	ComputeShaderApp() : Application("Compute Shader") {}

	void Startup() final;
	void Shutdown() final;

	bool Update() final;
	void Render() final;

private:
	void InitRootSigs();
	void InitPSOs();
	void InitConstantBuffer();
	void InitResourceSets();

	void LoadAssets();

private:
	// Vertex layout for this example
	struct Vertex
	{
		float position[3];
		float uv[2];
	};

	struct Constants
	{
		Math::Matrix4 viewProjectionMatrix;
		Math::Matrix4 modelMatrix;
	};

	Kodiak::VertexBuffer	m_vertexBuffer;
	Kodiak::IndexBuffer		m_indexBuffer;

	Constants				m_constants;
	Kodiak::ConstantBuffer	m_constantBuffer;

	Kodiak::RootSignature	m_rootSig;
	Kodiak::RootSignature	m_computeRootSig;

	Kodiak::GraphicsPSO		m_pso;
	Kodiak::ComputePSO		m_edgeDetectPSO;
	Kodiak::ComputePSO		m_embossPSO;
	Kodiak::ComputePSO		m_sharpenPSO;

	Kodiak::ResourceSet		m_computeResources;
	Kodiak::ResourceSet		m_gfxLeftResources;
	Kodiak::ResourceSet		m_gfxRightResources;

	Kodiak::ColorBuffer		m_computeScratch;

	Kodiak::TexturePtr		m_texture;

	int32_t				m_curComputeTechnique{ 0 };
};