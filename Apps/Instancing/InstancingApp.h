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
#include "Model.h"
#include "PipelineState.h"
#include "RootSignature.h"
#include "Texture.h"


class InstancingApp : public Kodiak::Application
{
public:
	InstancingApp()
		: Kodiak::Application("Instancing")
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
	void InitConstantBuffer();
	void InitInstanceBuffer();

	void UpdateConstantBuffer();

	void LoadAssets();

private:
	struct VSConstants
	{
		Math::Matrix4 projectionMatrix{ Math::kIdentity };
		Math::Matrix4 modelViewMatrix{ Math::kIdentity };
		Math::Vector4 lightPos{ 0.0f, -5.0f, 0.0f, 1.0f };
		float localSpeed{ 0.0f };
		float globalSpeed{ 0.0f };
	};

	VSConstants m_planetConstants;
	
	Kodiak::ConstantBuffer m_planetConstantBuffer;

	Kodiak::VertexBuffer m_instanceBuffer;

	Kodiak::RootSignature m_starfieldRootSig;
	Kodiak::RootSignature m_modelRootSig;

	Kodiak::GraphicsPSO m_starfieldPSO;
	Kodiak::GraphicsPSO m_rockPSO;
	Kodiak::GraphicsPSO m_planetPSO;

	Kodiak::TexturePtr m_rockTexture;
	Kodiak::TexturePtr m_planetTexture;

	Kodiak::ModelPtr m_rockModel;
	Kodiak::ModelPtr m_planetModel;

	Kodiak::CameraController m_controller;
	float m_zoom{ -18.5 };
	float m_rotationSpeed{ 0.25f };

	const uint32_t m_numInstances{ 8192 };
};