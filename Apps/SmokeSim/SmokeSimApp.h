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

#include "FluidEngine.h"
#include "Voxelizer.h"


class SmokeSimApp : public Kodiak::Application
{
public:
	SmokeSimApp()
		: Application("SmokeSim")
		, m_controller(m_camera, Math::Vector3(Math::kYUnitVector))
	{}

	void Configure() final;
	void Startup() final;
	void Shutdown() final;

	bool Update() final;
	void Render() final;

private:
	void InitRootSigs();
	void UpdateConstantBuffers();

	void SetupScene();

	// Simulation and render
	void RenderScene(Kodiak::GraphicsContext& context);

private:
	struct Vertex
	{
		float position[3];
		float normal[3];
	};

	struct Constants
	{
		Math::Matrix4 projectionMatrix;
		Math::Matrix4 modelMatrix;
		Kodiak::Color color;
	};

	struct Object
	{
		Kodiak::GraphicsPSO		objectPSO;
		Kodiak::ConstantBuffer	constantBuffer;
		Constants				constants;
		Kodiak::ResourceSet		resources;
		Kodiak::ModelPtr		model;
	};

	Kodiak::RootSignature		m_meshRootSig;

	std::array<Object, 4>		m_sceneObjects;

	// Camera controls
	Kodiak::CameraController	m_controller;

	// Fluid sim and rendering
	uint32_t m_gridWidth{ 64 };
	uint32_t m_gridHeight{ 64 };
	uint32_t m_gridDepth{ 64 };
	FluidEngine m_fluidEngine;
	Voxelizer m_voxelizer;
};