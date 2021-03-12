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
#include "Graphics\PipelineState.h"
#include "Graphics\ResourceSet.h"
#include "Graphics\RootSignature.h"


class VolumeRenderer
{
public:
	void Initialize(uint32_t width, uint32_t height, uint32_t depth);
	void Shutdown();

private:
	void InitRootSig();
	void InitPSOs();

private:
	uint32_t m_width{ 0 };
	uint32_t m_height{ 0 };
	uint32_t m_depth{ 0 };

	struct RayVertex
	{
		float position[3];
	};

	struct RayCastConstants
	{
		Math::Matrix4 worldProjectionMatrix;
		Math::Matrix4 projectionWorldMatrix;
		Math::Vector3 gridDim;
		Math::Vector3 invGridDim;
		Math::Vector3 eyeOnGrid;
		float rtDim[2];
		float invRtDim[2];
		float zNear;
		float zFar;
		float maxGridDim;
		float gridScaleFactor;
		float edgeThreshold;
	};

	Kodiak::RootSignature m_rootSig;
	Kodiak::GraphicsPSO m_compRayDataBackPSO;
	Kodiak::GraphicsPSO m_compRayDataFrontPSO;
	Kodiak::GraphicsPSO m_quadDownsamplePSO;
	Kodiak::GraphicsPSO m_quadRaycastPSO;
	Kodiak::GraphicsPSO m_quadEdgeDetectPSO;
	Kodiak::GraphicsPSO m_quadRaycastCopyPSO;
};