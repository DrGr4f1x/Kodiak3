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
#include "Graphics\GpuBuffer.h"
#include "Graphics\Model.h"
#include "Graphics\PipelineState.h"
#include "Graphics\ResourceSet.h"
#include "Graphics\RootSignature.h"
#include "Texture.h"


class TerrainTessellationApp : public Kodiak::Application
{
public:
	TerrainTessellationApp()
		: Application("Terrain Tessellation")
		, m_controller(m_camera, Math::Vector3(Math::kYUnitVector))
	{}

	void Configure() final;
	void Startup() final;
	void Shutdown() final;

	bool Update() final;
	void UpdateUI() final;
	void Render() final;

private:
	void InitRootSigs();
	void InitPSOs();
	void InitConstantBuffers();
	void InitResourceSets();
	void InitTerrain();

	void LoadAssets();

	void UpdateConstantBuffers();

private:
	struct Vertex
	{
		float position[3];
		float normal[3];
		float uv[2];		
	};

	struct SkyConstants
	{
		Math::Matrix4 modelViewProjectionMatrix;
	};

	struct TerrainConstants
	{
		Math::Matrix4 projectionMatrix;
		Math::Matrix4 modelViewMatrix;
		Math::Vector4 lightPos;
		Math::BoundingPlane frustumPlanes[6];
		float viewportDim[2];
		float displacementFactor;
		float tessellationFactor;
		float tessellatedEdgeSize;
	};

	Kodiak::RootSignature	m_skyRootSig;
	Kodiak::RootSignature	m_terrainRootSig;

	Kodiak::GraphicsPSO		m_skyPSO;
	Kodiak::GraphicsPSO		m_terrainPSO;

	SkyConstants			m_skyConstants;
	Kodiak::ConstantBuffer	m_skyConstantBuffer;

	TerrainConstants		m_terrainConstants;
	Kodiak::ConstantBuffer	m_terrainConstantBuffer;

	Kodiak::IndexBuffer		m_terrainIndices;
	Kodiak::VertexBuffer	m_terrainVertices;

	Kodiak::ModelPtr		m_skyModel;
	Kodiak::TexturePtr		m_skyTexture;
	Kodiak::TexturePtr		m_terrainTextureArray;
	Kodiak::TexturePtr		m_terrainHeightMap;

	Kodiak::ResourceSet		m_skyResources;
	Kodiak::ResourceSet		m_terrainResources;

	Kodiak::CameraController m_controller;
};