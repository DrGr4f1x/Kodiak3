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
#include "Grid.h"


class FluidEngine
{
public:
	enum class RenderTarget
	{
		Velocity0,
		Velocity1,
		Pressure,
		Color0,
		Color1,
		Obstacles,
		ObstacleVelocity,
		TempScalar,
		TempVector
	};

	FluidEngine();

	void Initialize(uint32_t width, uint32_t height, uint32_t depth);
	void Shutdown();

	void Clear();

	void Update(float deltaT);

	Kodiak::ColorBufferPtr GetRenderTarget(RenderTarget target);

private:
	void SetFormat(RenderTarget target, Kodiak::Format format);
	Kodiak::Format GetFormat(RenderTarget target) const;
	void SetName(RenderTarget target, const std::string& name);
	const std::string& GetName(RenderTarget target) const;
	Kodiak::ColorBuffer& GetColorBuffer(RenderTarget target);
	uint32_t GetSlot(RenderTarget target) const;

	void InitRootSig();
	void InitPSOs();
	void InitFBOs();
	void InitConstantBuffers();
	void InitResources();

	void UpdateConstantBuffers(float deltaT);

	void AdvectColorBFECC(Kodiak::GraphicsContext& context);
	void AdvectColor(Kodiak::GraphicsContext& context);

private:
	uint32_t m_width{ 0 };
	uint32_t m_height{ 0 };
	uint32_t m_depth{ 0 };

	// 3D color buffers
	std::array<Kodiak::Format, 9> m_renderTargetFormats;
	std::array<std::string, 9> m_renderTargetNames;
	std::array<Kodiak::ColorBufferPtr, 9> m_renderTargets;

	struct FluidConstants
	{
		Math::Vector3 texDim;
		Math::Vector3 invTexDim;
		int texNumber;
		Math::Vector4 obstVelocity;
		float modulate;
		float size;
		Math::Vector3 center;
		Math::Vector4 splatColor;
		float epsilon;
		float timestep;
		float forward;
		Math::Vector3 halfVolumeDim;
		Math::Vector3 boxLBDCorner;
		Math::Vector3 boxRTUCorner;
	};

	Kodiak::RootSignature m_rootSig;
	Kodiak::GraphicsPSO m_advectColorPSO;
	Kodiak::GraphicsPSO m_advectColorForwardPSO;
	Kodiak::GraphicsPSO m_advectColorBackPSO;
	Kodiak::GraphicsPSO m_advectColorBFECCPSO;
	Kodiak::GraphicsPSO m_advectVelocityPSO;

	Kodiak::FrameBuffer m_colorFBO[2];
	Kodiak::FrameBuffer m_tempVectorFBO;
	Kodiak::FrameBuffer m_tempScalarFBO;

	FluidConstants m_advectColorConstants;
	Kodiak::ConstantBuffer m_advectColorConstantBuffer;
	Kodiak::ResourceSet m_advectColorResources[2];

	FluidConstants m_advectColorForwardConstants;
	Kodiak::ConstantBuffer m_advectColorForwardConstantBuffer;
	Kodiak::ResourceSet m_advectColorForwardResources[2];

	FluidConstants m_advectColorBackConstants;
	Kodiak::ConstantBuffer m_advectColorBackConstantBuffer;
	Kodiak::ResourceSet m_advectColorBackResources;

	FluidConstants m_advectColorBFECCConstants;
	Kodiak::ConstantBuffer m_advectColorBFECCConstantBuffer;
	Kodiak::ResourceSet m_advectColorBFECCResources[2];

	// Simulation parameters
	bool m_bUseBFECC{ true };
	float m_confinementScale{ 0.06f };
	float m_decay{ 0.994f };
	uint32_t m_colorTexNumber{ 0 };

	// Utilities
	Grid m_grid;
};