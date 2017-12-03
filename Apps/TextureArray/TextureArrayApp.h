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
#include "GpuBuffer.h"
#include "PipelineState.h"
#include "RootSignature.h"
#include "Texture.h"

class TextureArrayApp : public Kodiak::Application
{
public:
	TextureArrayApp() : Kodiak::Application("Texture Array") {}

	void Configure() final;
	void Startup() final;
	void Shutdown() final;

	bool Update() final;
	void Render() final;

private:
	void InitRootSig();
	void InitPSO();
	void InitConstantBuffer();

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
		// Global matrices
		Math::Matrix4 projectionMatrix;
		Math::Matrix4 viewMatrix;
		
		// Separate data for each instance
		InstanceData* instance{ nullptr };
	};

	Kodiak::VertexBuffer	m_vertexBuffer;
	Kodiak::IndexBuffer		m_indexBuffer;

	Constants				m_constants;
	Kodiak::ConstantBuffer	m_constantBuffer;

	Kodiak::RootSignature	m_rootSig;
	Kodiak::GraphicsPSO		m_pso;

	// Assets
	Kodiak::TexturePtr		m_texture;
	uint32_t				m_layerCount{ 0 };

	// Camera controls
	float m_zoom{ -15.0f };
	Math::Vector3 m_rotation{ -15.0f, 35.0f, 0.0f };
};