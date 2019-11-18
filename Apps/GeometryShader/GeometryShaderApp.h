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


class GeometryShaderApp : public Kodiak::Application
{
public:
	GeometryShaderApp()
		: Application("Geometry Shader")
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
	void InitConstantBuffer();
	void InitResourceSets();

	void UpdateConstantBuffer();

	void LoadAssets();

private:
	struct Vertex
	{
		float position[3];
		float normal[3];
		float color[3];
	};

	struct Constants
	{
		Math::Matrix4 projectionMatrix;
		Math::Matrix4 modelMatrix;
	};

	Kodiak::RootSignature		m_meshRootSig;
	Kodiak::RootSignature		m_geomRootSig;

	Kodiak::GraphicsPSO			m_meshPSO;
	Kodiak::GraphicsPSO			m_geomPSO;

	Kodiak::ConstantBuffer		m_constantBuffer;
	Constants					m_constants;

	Kodiak::ResourceSet			m_meshResources;
	Kodiak::ResourceSet			m_geomResources;

	Kodiak::ModelPtr			m_model;

	Kodiak::CameraController	m_controller;

	bool m_showNormals{ true };
};