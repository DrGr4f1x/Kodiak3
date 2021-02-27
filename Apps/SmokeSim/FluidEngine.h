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

	void Update(float deltaT, int debugTex);

	void PrepareDebugDraw(Kodiak::GraphicsContext& context);
	void DebugDraw(Kodiak::GraphicsContext& context);

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
	void AdvectVelocity(Kodiak::GraphicsContext& context);
	void ApplyVorticityConfinement(Kodiak::GraphicsContext& context);
	void ApplyExternalForces(Kodiak::GraphicsContext& context);
	void ComputeVelocityDivergence(Kodiak::GraphicsContext& context);
	void ComputePressure(Kodiak::GraphicsContext& context);
	void ProjectVelocity(Kodiak::GraphicsContext& context);

private:
	uint32_t m_width{ 0 };
	uint32_t m_height{ 0 };
	uint32_t m_depth{ 0 };
	uint32_t m_rows{ 0 };
	uint32_t m_cols{ 0 };

	// 3D color buffers
	std::array<Kodiak::Format, 9> m_renderTargetFormats;
	std::array<std::string, 9> m_renderTargetNames;
	std::array<Kodiak::ColorBufferPtr, 9> m_renderTargets;

	struct FluidConstants
	{
		Math::Vector4 obstVelocity;
		Math::Vector4 splatColor;
		Math::Vector3 texDim;
		Math::Vector3 invTexDim;
		Math::Vector3 center;
		Math::Vector3 halfVolumeDim;
		Math::Vector3 boxLBDCorner;
		Math::Vector3 boxRTUCorner;
		int texNumber;
		float modulate;
		float size;
		float epsilon;
		float timestep;
		float forward;
	};
	FluidConstants m_constants;

	Kodiak::RootSignature m_rootSig;
	Kodiak::GraphicsPSO m_advectColorPSO;
	Kodiak::GraphicsPSO m_advectColorForwardPSO;
	Kodiak::GraphicsPSO m_advectColorBackPSO;
	Kodiak::GraphicsPSO m_advectColorBFECCPSO;
	Kodiak::GraphicsPSO m_advectVelocityPSO;
	Kodiak::GraphicsPSO m_vorticityPSO;
	Kodiak::GraphicsPSO m_confinementPSO;
	Kodiak::GraphicsPSO m_gaussianColorPSO;
	Kodiak::GraphicsPSO m_gaussianVelocityPSO;
	Kodiak::GraphicsPSO m_divergencePSO;
	Kodiak::GraphicsPSO m_pressurePSO;
	Kodiak::GraphicsPSO m_projectVelocityPSO;
	Kodiak::GraphicsPSO m_debugPSO;

	Kodiak::FrameBuffer m_colorFBO[2];
	Kodiak::FrameBuffer m_tempVectorFBO;
	Kodiak::FrameBuffer m_tempScalarFBO;
	Kodiak::FrameBuffer m_velocityFBO[2];
	Kodiak::FrameBuffer m_pressureFBO;

	Kodiak::ConstantBuffer m_advectColorConstantBuffer;
	Kodiak::ResourceSet m_advectColorResources[2];

	Kodiak::ConstantBuffer m_advectColorForwardConstantBuffer;
	Kodiak::ResourceSet m_advectColorForwardResources[2];

	Kodiak::ConstantBuffer m_advectColorBackConstantBuffer;
	Kodiak::ResourceSet m_advectColorBackResources;

	Kodiak::ConstantBuffer m_advectColorBFECCConstantBuffer;
	Kodiak::ResourceSet m_advectColorBFECCResources[2];

	Kodiak::ConstantBuffer m_advectVelocityConstantBuffer;
	Kodiak::ResourceSet m_advectVelocityResources;

	Kodiak::ConstantBuffer m_vorticityConstantBuffer;
	Kodiak::ResourceSet m_vorticityResources;

	Kodiak::ConstantBuffer m_confinementConstantBuffer;
	Kodiak::ResourceSet m_confinementResources;

	Kodiak::ConstantBuffer m_gaussianColorConstantBuffer;
	Kodiak::ResourceSet m_gaussianColorResources;

	Kodiak::ConstantBuffer m_gaussianVelocityConstantBuffer;
	Kodiak::ResourceSet m_gaussianVelocityResources;

	Kodiak::ConstantBuffer m_divergenceConstantBuffer;
	Kodiak::ResourceSet m_divergenceResources;

	Kodiak::ConstantBuffer m_pressureConstantBuffer;
	Kodiak::ResourceSet m_pressureResources[2];

	Kodiak::ConstantBuffer m_projectVelocityConstantBuffer;
	Kodiak::ResourceSet m_projectVelocityResources;

	Kodiak::ConstantBuffer m_debugConstantBuffer;
	Kodiak::ResourceSet m_debugResources[2];

	// Simulation parameters
	bool m_bUseBFECC{ true };
	float m_confinementScale{ 0.06f };
	float m_decay{ 0.994f };
	uint32_t m_colorTexNumber{ 0 };
	int m_debugTexNumber{ 0 };
	float m_impulseSize{ 0.15f };
	Math::Vector3 m_impulsePosition{ Math::kZero };
	Math::Vector3 m_impulseVelocity{ Math::kZero };
	float m_saturation{ 0.78f };
	uint32_t m_iterations{ 10 };

	// Utilities
	Grid m_grid;
};