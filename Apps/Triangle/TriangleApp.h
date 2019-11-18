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


class TriangleApp : public Kodiak::Application
{
public:
	TriangleApp() 
		: Application("Triangle")
		, m_controller(m_camera, Math::Vector3(Math::kYUnitVector)) 
	{}

	void Configure() final;
	void Startup() final;
	void Shutdown() final;

	bool Update() final;
	void Render() final;

private:
	void UpdateConstantBuffer();

	void InitRootSig();
	void InitPSO();
	void InitResourceSet();

private:
	// Vertex layout used in this example
	struct Vertex 
	{
		float position[3];
		float color[3];
	};

	// Vertex buffer and attributes
	Kodiak::VertexBuffer m_vertexBuffer;

	// Index buffer
	Kodiak::IndexBuffer m_indexBuffer;

	// Uniform buffer block object
	Kodiak::ConstantBuffer m_constantBuffer;

	struct
	{
		Math::Matrix4 viewProjectionMatrix;
		Math::Matrix4 modelMatrix;
	} m_vsConstants;

	// Camera controls
	float m_zoom{ -2.5f };
	Kodiak::CameraController m_controller;

	Kodiak::RootSignature	m_rootSig;
	Kodiak::GraphicsPSO		m_pso;

	Kodiak::ResourceSet		m_resources;
};