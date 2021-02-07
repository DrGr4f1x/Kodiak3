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
#include "Graphics\QueryHeap.h"
#include "Graphics\ResourceSet.h"
#include "Graphics\RootSignature.h"


class OcclusionQueryApp : public Kodiak::Application
{
public:
	OcclusionQueryApp()
		: Application("Occlusion Query")
		, m_controller(m_camera, Math::Vector3(Math::kYUnitVector))
	{}

	void Startup() final;
	void Shutdown() final;

	bool Update() final;
	void UpdateUI() final;
	void Render() final;

private:
	void InitRootSig();
	void InitPSOs();
	void InitConstantBuffers();
	void InitQueryHeap();
	void InitResourceSets();

	void UpdateConstantBuffers();

	void LoadAssets();

private:
	struct VSConstants
	{
		Math::Matrix4 projectionMatrix{ Math::kIdentity };
		Math::Matrix4 modelViewMatrix{ Math::kIdentity };
		Math::Vector4 lightPos{ 10.0f, 10.0f, 10.0f, 1.0f };
		float visible{ 1.0f };
	};

	VSConstants m_occluderConstants;
	VSConstants m_teapotConstants;
	VSConstants m_sphereConstants;

	Kodiak::ConstantBuffer m_occluderCB;
	Kodiak::ConstantBuffer m_teapotCB;
	Kodiak::ConstantBuffer m_sphereCB;
	
	Kodiak::ResourceSet	m_occluderResources;
	Kodiak::ResourceSet m_teapotResources;
	Kodiak::ResourceSet	m_sphereResources;

	Kodiak::RootSignature m_rootSig;

	Kodiak::GraphicsPSO m_solidPSO;
	Kodiak::GraphicsPSO m_simplePSO;
	Kodiak::GraphicsPSO m_occluderPSO;

	Kodiak::ModelPtr m_occluderModel;
	Kodiak::ModelPtr m_teapotModel;
	Kodiak::ModelPtr m_sphereModel;

	Kodiak::OcclusionQueryHeap m_queryHeap;
	Kodiak::ReadbackBuffer m_readbackBuffer;

	Kodiak::CameraController m_controller;
	float m_zoom{ -35.0f };

	uint32_t m_passedSamples[2]{ 0,0 };
};