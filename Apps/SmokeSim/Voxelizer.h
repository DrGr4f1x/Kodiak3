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

#include "Graphics\ColorBuffer.h"
#include "Graphics\DepthBuffer.h"
#include "Graphics\Framebuffer.h"
#include "Graphics\GpuBuffer.h"
#include "Graphics\Model.h"
#include "Graphics\PipelineState.h"
#include "Graphics\ResourceSet.h"
#include "Graphics\RootSignature.h"


class Voxelizer
{
public:
	void Initialize(Kodiak::ColorBufferPtr obstacleTex3D, Kodiak::ColorBufferPtr obstacleVelocityTex3D);
	
	void SetGridToWorldMatrix(const Math::Matrix4& gridToWorldMatrix);
	void AddModel(Kodiak::ModelPtr model);

	bool IsInitialized() const { return m_initialized; }

	void Update(float deltaT);
	void Render(Kodiak::GraphicsContext& context);

private:
	void InitFrameBuffers();
	void InitRootSigs();
	void InitPSOs();
	void InitSliceVertices();
	void InitResources();

	void StencilClipScene(Kodiak::GraphicsContext& context);
	void DrawSlices(Kodiak::GraphicsContext& context);

private:
	bool m_initialized{ false };

	// 3D volume dimensions
	uint32_t m_width{ 0 };
	uint32_t m_height{ 0 };
	uint32_t m_depth{ 0 };

	// Grid to world matrix
	Math::Matrix4 m_gridToWorldMatrix{ Math::kIdentity };
	Math::Matrix4 m_worldToGridMatrix{ Math::kIdentity };

	// Rows, columns of flattened 3D volume
	uint32_t m_rows{ 0 };
	uint32_t m_cols{ 0 };

	// Vertices
	struct SliceVertex
	{
		float position[3];
		float texcoord[3];
	};

	// Rendering resources
	Kodiak::ColorBufferPtr m_obstacleTex3D;
	Kodiak::ColorBufferPtr m_obstacleVelocityTex3D;
	Kodiak::ColorBufferPtr m_debugColorBuffer;
	Kodiak::DepthBufferPtr m_depthBuffer;

	Kodiak::FrameBuffer m_voxelizeFBO;
	Kodiak::FrameBuffer m_resolveFBO;
	Kodiak::FrameBuffer m_genVelocityFBO;

	Kodiak::RootSignature m_voxelizeRootSig;
	Kodiak::RootSignature m_resolveRootSig;
	Kodiak::RootSignature m_genVelocityRootSig;

	Kodiak::GraphicsPSO m_voxelizePSO;
	Kodiak::GraphicsPSO m_resolvePSO;
	Kodiak::GraphicsPSO m_genVelocityPSO;

	Kodiak::VertexBuffer m_sliceVertexBuffer;
	Kodiak::ResourceSet m_resolveResources;

	// Scene objects
	struct VoxelizeConstants
	{
		Math::Matrix4 modelViewProjectionMatrix;
	};

	struct Object
	{
		// Gen velocity pass data
		
		Kodiak::ModelPtr		model;
	};
	std::vector<Object> m_sceneObjects;
};