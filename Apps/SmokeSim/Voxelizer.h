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
	void Shutdown();
	
	void SetGridToWorldMatrix(const Math::Matrix4& gridToWorldMatrix);
	void AddModel(Kodiak::ModelPtr model);

	bool IsInitialized() const { return m_initialized; }

	void Update(float deltaT);
	void Render(Kodiak::GraphicsContext& context);

private:
	void InitFrameBuffers();
	void InitRootSigs();
	void InitPSOs();
	void InitConstantBuffers();
	void InitSliceVertices();
	void InitResources();

	void UpdateConstantBuffers();

	void StencilClipScene(Kodiak::GraphicsContext& context);
	void DrawSlices(Kodiak::GraphicsContext& context);
	void ComputeResolve(Kodiak::GraphicsContext& context);
	void RenderVelocity(Kodiak::GraphicsContext& context);

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

	// Timestep
	float m_deltaT{ 0.0f };

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
	Kodiak::RootSignature m_resolveComputeRootSig;
	Kodiak::RootSignature m_genVelocityRootSig;

	Kodiak::GraphicsPSO m_voxelizePSO;
	Kodiak::GraphicsPSO m_resolvePSO;
	Kodiak::ComputePSO m_resolveComputePSO;
	Kodiak::GraphicsPSO m_genVelocityPSO;

	Kodiak::VertexBuffer m_sliceVertexBuffer;
	Kodiak::ResourceSet m_resolveResources;
	Kodiak::ResourceSet m_resolveComputeResources;

	struct alignas(256) VoxelizeProjectionConstants
	{
		Math::Matrix4* projectionMatrix{ nullptr };
	};
	size_t m_voxelizeDynamicAlignment{ 0 };
	VoxelizeProjectionConstants m_voxelizeProjectionConstants{};
	Kodiak::ConstantBuffer m_voxelizeProjectionConstantBuffer;

	struct GenVelocityGSConstantData
	{
		int sliceIndex;
		float sliceZ;
		float projSpacePixDim[2];
	};

	struct GenVelocityGSConstants
	{
		GenVelocityGSConstantData* data{ nullptr };
	};
	size_t m_genVelocityDynamicAlignment{ 0 };
	GenVelocityGSConstants m_genVelocityGSConstants{};
	Kodiak::ConstantBuffer m_genVelocityGSConstantBuffer;

	Kodiak::ConstantBuffer m_resolveComputeConstantBuffer;

	// Scene objects

	struct VoxelizeModelConstants
	{
		Math::Matrix4 modelViewMatrix{ Math::kIdentity };
	};

	struct GenVelocityVSConstants
	{
		Math::Matrix4 modelViewProjectionMatrix;
		Math::Matrix4 prevModelViewProjectionMatrix;
		float gridDim[3];
		float deltaT;
	};

	struct Object
	{
		// Voxelization pass
		VoxelizeModelConstants voxelizeConstants{};
		Kodiak::ConstantBuffer voxelizeConstantBuffer;
		Kodiak::ResourceSet voxelizeResources;

		// Gen velocity pass data
		GenVelocityVSConstants genVelocityConstants{};
		Kodiak::ConstantBuffer genVelocityConstantBuffer;
		Kodiak::ResourceSet genVelocityResources;

		Kodiak::ModelPtr		model;
	};
	std::vector<Object> m_sceneObjects;
};