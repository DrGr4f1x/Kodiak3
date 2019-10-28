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

class TextureCubeMapApp : public Kodiak::Application
{
public:
	TextureCubeMapApp()
		: Application("Texture CubeMap")
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

	void UpdateConstantBuffers();

	void LoadAssets();

private:
	struct Vertex
	{
		float position[3];
		float normal[3];
		float uv[2];
	};

	struct VSConstants
	{
		Math::Matrix4 viewProjectionMatrix;
		Math::Matrix4 modelMatrix;
		Math::Vector3 eyePos;
	};

	struct PSConstants
	{
		float lodBias;
	};
	Kodiak::RootSignature		m_modelRootSig;
	Kodiak::RootSignature		m_skyboxRootSig;

	Kodiak::GraphicsPSO			m_modelPSO;
	Kodiak::GraphicsPSO			m_skyboxPSO;

	VSConstants					m_vsSkyboxConstants;
	VSConstants					m_vsModelConstants;
	PSConstants					m_psConstants;
	Kodiak::ConstantBuffer		m_vsSkyboxConstantBuffer;
	Kodiak::ConstantBuffer		m_vsModelConstantBuffer;
	Kodiak::ConstantBuffer		m_psConstantBuffer;

	Kodiak::ResourceSet			m_modelResources;
	Kodiak::ResourceSet			m_skyboxResources;

	Kodiak::TexturePtr			m_skyboxTex;
	Kodiak::ModelPtr			m_skyboxModel;
	std::vector<Kodiak::ModelPtr> m_models;

	Kodiak::CameraController	m_controller;
};
