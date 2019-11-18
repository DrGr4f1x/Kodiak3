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
#include "ColorBuffer.h"
#include "GpuBuffer.h"
#include "Model.h"
#include "PipelineState.h"
#include "ResourceSet.h"
#include "RootSignature.h"
#include "Texture.h"

class ComputeClothApp : public Kodiak::Application
{
public:
	ComputeClothApp() 
		: Application("Compute Cloth") 
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
	void InitConstantBuffers();
	void InitCloth();
	void InitResourceSets();

	void LoadAssets();

	void UpdateConstantBuffers();

private:
	struct SphereVertex
	{
		float position[3];
		float normal[3];
	};

	struct VSConstants
	{
		Math::Matrix4 projectionMatrix;
		Math::Matrix4 modelViewMatrix;
		Math::Vector4 lightPos;
	};

	struct CSConstants
	{
		float deltaT;
		float particleMass;
		float springStiffness;
		float damping;
		float restDistH;
		float restDistV;
		float restDistD;
		float sphereRadius;
		Math::Vector4 spherePos;
		Math::Vector4 gravity;
		int32_t particleCount[2];
		uint32_t calculateNormals;
	};

	struct Particle
	{
		Math::Vector4 pos;
		Math::Vector4 vel;
		Math::Vector4 uv;
		Math::Vector4 normal;
		Math::Vector4 pinned;
	};

	Kodiak::RootSignature	m_sphereRootSig;
	Kodiak::RootSignature	m_clothRootSig;
	Kodiak::RootSignature	m_computeRootSig;

	Kodiak::GraphicsPSO		m_spherePSO;
	Kodiak::GraphicsPSO		m_clothPSO;
	Kodiak::ComputePSO		m_computePSO;

	VSConstants				m_vsConstants;
	Kodiak::ConstantBuffer	m_vsConstantBuffer;

	CSConstants				m_csConstants;
	Kodiak::ConstantBuffer	m_csConstantBuffer;
	Kodiak::ConstantBuffer	m_csNormalConstantBuffer;

	Kodiak::StructuredBuffer m_clothBuffer[2];
	Kodiak::IndexBuffer		m_clothIndexBuffer;

	Kodiak::ModelPtr		m_sphereModel;
	Kodiak::TexturePtr		m_texture;

	Kodiak::ResourceSet		m_sphereResources;
	Kodiak::ResourceSet		m_clothResources;
	Kodiak::ResourceSet		m_computeResources[2];
	Kodiak::ResourceSet		m_computeNormalResources;

	Kodiak::CameraController m_controller;

	// Cloth dimensions
	const float m_sphereRadius{ 0.5f };
	const uint32_t m_gridSize[2]{ 64, 64 };
	const float m_size[2]{ 2.5f, 2.5f };
	bool m_simulateWind{ true };
	bool m_pinnedCloth{ false };
	uint32_t m_readSet{ 1 };
};