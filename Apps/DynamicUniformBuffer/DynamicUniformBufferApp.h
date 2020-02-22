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


class DynamicUniformBufferApp : public Kodiak::Application
{
public:
	DynamicUniformBufferApp()
		: Application("Dynamic Uniform Buffer")
		, m_controller(m_camera, Math::Vector3(Math::kYUnitVector))
	{}

	void Startup() final;
	void Shutdown() final;

	bool Update() final;
	void Render() final;

private:
	void InitRootSig();
	void InitPSO();
	void InitConstantBuffers();
	void InitBox();
	void InitResourceSet();

	void UpdateConstantBuffers();

private:
	static const uint32_t m_numCubesSide{ 5 };
	static const uint32_t m_numCubes{ m_numCubesSide * m_numCubesSide * m_numCubesSide };

	struct VSConstants
	{
		Math::Matrix4 projectionMatrix;
		Math::Matrix4 viewMatrix;
	};
	VSConstants m_vsConstants;
	Kodiak::ConstantBuffer m_vsConstantBuffer;

	struct alignas(256) VSModelConstants
	{
		Math::Matrix4* modelMatrix{ nullptr };
	};
	size_t m_dynamicAlignment{ 0 };
	VSModelConstants m_vsModelConstants;
	Kodiak::ConstantBuffer m_vsModelConstantBuffer;

	Kodiak::RootSignature m_rootSignature;
	Kodiak::GraphicsPSO m_pso;

	Kodiak::VertexBuffer m_vertexBuffer;
	Kodiak::IndexBuffer m_indexBuffer;

	Kodiak::ResourceSet m_resources;

	Kodiak::CameraController m_controller;

	float m_animationTimer{ 0.0f };
	Math::Vector3 m_rotations[m_numCubes];
	Math::Vector3 m_rotationSpeeds[m_numCubes];
};