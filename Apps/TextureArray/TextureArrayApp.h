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
#include "Graphics\PipelineState.h"
#include "Graphics\ResourceSet.h"
#include "Graphics\RootSignature.h"
#include "Graphics\Texture.h"

class TextureArrayApp : public Kodiak::Application
{
public:
	TextureArrayApp() 
		: Application("Texture Array")
		, m_controller(m_camera, Math::Vector3(Math::kYUnitVector)) 
	{}

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
	};

	struct InstanceData 
	{
		// Model matrix
		Math::Matrix4 modelMatrix;
		// Texture array index
		// Vec4 due to padding
		Math::Vector4 arrayIndex;
	};

	struct Constants
	{
		// Global matrix
		Math::Matrix4 viewProjectionMatrix;
		
		// Separate data for each instance
		InstanceData* instance{ nullptr };
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
	uint32_t				m_layerCount{ 0 };

	// Camera controls
	float m_zoom{ -15.0f };
	Math::Vector3 m_rotation{ -15.0f, 35.0f, 0.0f };
	Kodiak::CameraController m_controller;
};