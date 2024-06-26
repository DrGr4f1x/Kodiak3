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
#include "Graphics\Texture.h"

class Texture3dApp : public Kodiak::Application
{
public:
	Texture3dApp() 
		: Application("Texture 3D")
		, m_controller(m_camera, Math::Vector3(Math::kYUnitVector))
	{
		m_constants.depth = 0.0f;
	}

	void Startup() final;
	void Shutdown() final;

	bool Update() final;
	void Render() final;

private:
	void InitRootSig();
	void InitPSO();
	void InitConstantBuffer();
	void InitTexture();
	void InitResourceSet();

	void UpdateConstantBuffer();

private:
	struct Vertex
	{
		float position[3];
		float normal[3];
		float uv[2];
	};

	struct Constants
	{
		Math::Matrix4 viewProjectionMatrix;
		Math::Matrix4 modelMatrix;
		Math::Vector4 viewPos;
		float depth;
	};

	Kodiak::RootSignature		m_rootSig;
	Kodiak::GraphicsPSO			m_pso;

	Constants					m_constants;
	Kodiak::ConstantBuffer		m_constantBuffer;

	Kodiak::ResourceSet			m_resources;

	Kodiak::VertexBuffer		m_vertexBuffer;
	Kodiak::IndexBuffer			m_indexBuffer;

	Kodiak::TexturePtr			m_texture;

	float m_zoom{ -2.5f };
	Kodiak::CameraController	m_controller;
};