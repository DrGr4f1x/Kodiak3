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
#include "Graphics\Model.h"
#include "Graphics\PipelineState.h"
#include "Graphics\ResourceSet.h"
#include "RootSignature.h"


class DirectConstantsApp : public Kodiak::Application
{
public:
	DirectConstantsApp() 
		: Application("Direct Constants")
		, m_controller(m_camera, Math::Vector3(Math::kYUnitVector)) 
	{}

	void Configure() final;
	void Startup() final;
	void Shutdown() final;

	bool Update() final;
	void Render() final;

private:
	void InitRootSig();
	void InitPSO();
	void InitConstantBuffer();
	void InitResourceSet();

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

	struct LightConstants
	{
		Math::Vector4 lightPositions[6];
	};

	Kodiak::RootSignature		m_rootSig;
	Kodiak::GraphicsPSO			m_PSO;

	Kodiak::ConstantBuffer		m_constantBuffer;
	Constants					m_constants;

	LightConstants				m_lightConstants;

	Kodiak::ResourceSet			m_resources;

	Kodiak::ModelPtr			m_model;

	Kodiak::CameraController	m_controller;
};