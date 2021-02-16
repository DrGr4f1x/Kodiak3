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
#include "Graphics\PipelineState.h"
#include "Graphics\RootSignature.h"


class Voxelizer
{
public:
	void Initialize(Kodiak::ColorBufferPtr obstacleTex3D, Kodiak::ColorBufferPtr obstacleVelocityTex3D);

	bool IsInitialized() const { return m_initialized; }

private:
	void InitRootSigs();
	void InitPSOs();

private:
	bool m_initialized{ false };

	// 3D volume dimensions
	uint32_t m_width{ 0 };
	uint32_t m_height{ 0 };
	uint32_t m_depth{ 0 };

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
	Kodiak::DepthBufferPtr m_depthBuffer;

	Kodiak::RootSignature m_voxelizeRootSig;
	Kodiak::RootSignature m_resolveRootSig;
	Kodiak::RootSignature m_genVelocityRootSig;

	Kodiak::GraphicsPSO m_voxelizePSO;
	Kodiak::GraphicsPSO m_resolvePSO;
	Kodiak::GraphicsPSO m_genVelocityPSO;
};