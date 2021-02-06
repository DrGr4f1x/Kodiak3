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
#include "Graphics\ColorBuffer.h"
#include "GpuBuffer.h"
#include "PipelineState.h"
#include "ResourceSet.h"
#include "RootSignature.h"
#include "Texture.h"


class ComputeNBodyApp : public Kodiak::Application
{
public:
	ComputeNBodyApp()
		: Application("Compute N-Body")
		, m_controller(m_camera, Math::Vector3(Math::kYUnitVector))
	{}

	void Configure() final;
	void Startup() final;
	void Shutdown() final;

	bool Update() final;
	void Render() final;

private:
	void InitRootSigs();
	void InitPSOs();
	void InitConstantBuffers();
	void InitResourceSets();
	void InitParticles();

	void LoadAssets();

	void UpdateConstantBuffers();

private:
	struct GraphicsConstants 
	{
		Math::Matrix4 projectionMatrix;
		Math::Matrix4 modelViewMatrix;
		Math::Matrix4 invViewMatrix;
		float screenDim[2];
	};

	struct ComputeConstants
	{
		float deltaT;
		float destX;
		float destY;
		int particleCount;
	};

	struct Particle
	{
		Math::Vector4 pos;
		Math::Vector4 vel;
	};

	Kodiak::RootSignature	m_rootSig;
	Kodiak::RootSignature	m_computeRootSig;

	Kodiak::GraphicsPSO		m_PSO;
	Kodiak::ComputePSO		m_computeCalculatePSO;
	Kodiak::ComputePSO		m_computeIntegratePSO;

	GraphicsConstants		m_graphicsConstants;
	ComputeConstants		m_computeConstants;

	Kodiak::ConstantBuffer	m_graphicsConstantBuffer;
	Kodiak::ConstantBuffer	m_computeConstantBuffer;

	Kodiak::StructuredBuffer m_particleBuffer;

	Kodiak::ResourceSet		m_graphicsResources;
	Kodiak::ResourceSet		m_computeResources;

	Kodiak::TexturePtr		m_gradientTexture;
	Kodiak::TexturePtr		m_colorTexture;

	Kodiak::CameraController m_controller;
};