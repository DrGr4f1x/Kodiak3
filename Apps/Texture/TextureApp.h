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
#include "PipelineState.h"
#include "ResourceSet.h"
#include "RootSignature.h"
#include "Texture.h"

class TextureApp : public Kodiak::Application
{
public:
	TextureApp() 
		: Application("Texture")
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
	// Vertex layout for this example
	struct Vertex
	{
		float position[3];
		float uv[2];
		float normal[3];
	};

	struct Constants
	{
		Math::Matrix4 viewProjectionMatrix;
		Math::Matrix4 modelMatrix;
		Math::Vector3 viewPos;
		float lodBias = 0.0f;
	};

	Kodiak::VertexBuffer	m_vertexBuffer;
	Kodiak::IndexBuffer		m_indexBuffer;

	Constants				m_constants;
	Kodiak::ConstantBuffer	m_constantBuffer;

	Kodiak::ResourceSet		m_resources;

	Kodiak::RootSignature	m_rootSig;
	Kodiak::GraphicsPSO		m_pso;

	// Assets
	Kodiak::TexturePtr		m_texture;

	// Camera controls
	float m_zoom{ -2.5f };
	Math::Vector3 m_cameraPos{ Math::kZero };
	Kodiak::CameraController m_controller;
};