//
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author:  David Elder
//

#include "Stdafx.h"

#include "FluidEngine.h"

#include "SmokeSimUtils.h"

#include "Application.h"
#include "Graphics\CommandContext.h"
#include "Graphics\CommonStates.h"


using namespace Kodiak;
using namespace std;


FluidEngine::FluidEngine()
{
	SetFormat(RenderTarget::Velocity0, Format::R16G16B16A16_Float);
	SetFormat(RenderTarget::Velocity1, Format::R16G16B16A16_Float);
	SetFormat(RenderTarget::Pressure, Format::R16_Float);
	SetFormat(RenderTarget::Color0, Format::R16_Float);
	SetFormat(RenderTarget::Color1, Format::R16_Float);
	SetFormat(RenderTarget::Obstacles, Format::R8_UNorm);
	SetFormat(RenderTarget::ObstacleVelocity, Format::R16G16B16A16_Float);
	SetFormat(RenderTarget::TempScalar, Format::R16_Float);
	SetFormat(RenderTarget::TempVector, Format::R16G16B16A16_Float);

	SetName(RenderTarget::Velocity0, "Velocity 0");
	SetName(RenderTarget::Velocity1, "Velocity 1");
	SetName(RenderTarget::Pressure, "Pressure");
	SetName(RenderTarget::Color0, "Color 0");
	SetName(RenderTarget::Color1, "Color 1");
	SetName(RenderTarget::Obstacles, "Obstacles");
	SetName(RenderTarget::ObstacleVelocity, "Obstacle Velocity");
	SetName(RenderTarget::TempScalar, "Temp Scalar");
	SetName(RenderTarget::TempVector, "Temp Vector");
}


void FluidEngine::Initialize(uint32_t width, uint32_t height, uint32_t depth)
{
	m_width = width;
	m_height = height;
	m_depth = depth;

	ComputeFlattened3DTextureDims(m_depth, m_rows, m_cols);

	// Create 3D render targets
	for (uint32_t i = 0; i < 9; ++i)
	{
		ColorBufferPtr target = make_shared<ColorBuffer>();

		RenderTarget rt = RenderTarget(i);
		target->Create3D(GetName(rt), width, height, depth, GetFormat(rt));

		m_renderTargets[i] = target;
	}

	Clear();

	InitRootSig();
	InitPSOs();
	InitFBOs();
	InitConstantBuffers();
	InitResources();

	m_grid.Initialize(width, height, depth);
}


void FluidEngine::Shutdown()
{
	m_rootSig.Destroy();
}


void FluidEngine::Clear()
{
	GraphicsContext& context = GraphicsContext::Begin("Reset Fluid Textures");

	for(size_t i = 0; i < m_renderTargets.size(); ++i)
	{
		context.TransitionResource(*m_renderTargets[i], ResourceState::RenderTarget);
		context.ClearColor(*m_renderTargets[i]);
	}

	context.Finish(true);
}


void FluidEngine::Update(float deltaT, int debugTex)
{
	m_debugTexNumber = debugTex;

	// Set vorticity confinement and decay parameters
	m_confinementScale = m_bUseBFECC ? 0.06f : 0.12f;
	m_decay = m_bUseBFECC ? 0.994f : 0.9995f;

	// Set impulse position / velocity
	m_impulsePosition.SetX(float(m_width) / 10.0f);
	m_impulsePosition.SetY(float(m_height) / 10.0f);
	m_impulsePosition.SetZ(float(m_depth) / 4.0f);
	const float impulseStrength{ 0.8f };
	m_impulseVelocity.SetX(0.5f * impulseStrength);
	m_impulseVelocity.SetY(0.5f * impulseStrength);
	m_impulseVelocity.SetZ(0.8f * impulseStrength);

	UpdateConstantBuffers(2.0f);

	if (true)
	{
		GraphicsContext& context = GraphicsContext::Begin("Fluid Update");
		if (m_bUseBFECC)
		{
			AdvectColorBFECC(context);
		}
		else
		{
			AdvectColor(context);
		}

		AdvectVelocity(context);

		ApplyVorticityConfinement(context);

		ApplyExternalForces(context);

		ComputeVelocityDivergence(context);

		ComputePressure(context);

		ProjectVelocity(context);

		context.Finish();
	}

	// Flip textures
	m_colorTexNumber = 1 - m_colorTexNumber;
}


void FluidEngine::PrepareDebugDraw(GraphicsContext& context)
{
	RenderTarget readSource = (m_colorTexNumber == 0) ? RenderTarget::Color0 : RenderTarget::Color1;

	context.TransitionResource(GetColorBuffer(readSource), ResourceState::PixelShaderResource);
	context.TransitionResource(GetColorBuffer(RenderTarget::Velocity0), ResourceState::PixelShaderResource);
	context.TransitionResource(GetColorBuffer(RenderTarget::Obstacles), ResourceState::PixelShaderResource);
	context.TransitionResource(GetColorBuffer(RenderTarget::ObstacleVelocity), ResourceState::PixelShaderResource);
}


void FluidEngine::DebugDraw(GraphicsContext& context)
{
	ScopedDrawEvent event(context, "Fluid Debug");

	uint32_t screenWidth = GetApplication()->GetWidth();
	uint32_t screenHeight = GetApplication()->GetHeight();
	uint32_t flatWidth = m_width * m_cols;
	uint32_t flatHeight = m_height * m_rows;

	context.SetInvertedViewport(false);
	context.SetViewportAndScissor(screenWidth / 2 - flatWidth / 2, screenHeight / 2 - flatHeight / 2, flatWidth, flatHeight);

	context.SetRootSignature(m_rootSig);
	context.SetPipelineState(m_debugPSO);
	context.SetResources(m_debugResources[m_colorTexNumber]);

	m_grid.DrawSlicesToScreen(context);
	context.SetInvertedViewport(true);
}


ColorBufferPtr FluidEngine::GetRenderTarget(RenderTarget target)
{
	return m_renderTargets[uint32_t(target)];
}


void FluidEngine::SetFormat(RenderTarget target, Format format)
{
	m_renderTargetFormats[uint32_t(target)] = format;
}


Format FluidEngine::GetFormat(RenderTarget target) const
{
	return m_renderTargetFormats[uint32_t(target)];
}


void FluidEngine::SetName(RenderTarget target, const string& name)
{
	m_renderTargetNames[uint32_t(target)] = name;
}


const string& FluidEngine::GetName(RenderTarget target) const
{
	return m_renderTargetNames[uint32_t(target)];
}


ColorBuffer& FluidEngine::GetColorBuffer(RenderTarget target)
{
	return *m_renderTargets[uint32_t(target)];
}


uint32_t FluidEngine::GetSlot(RenderTarget target) const
{
	switch (target)
	{
		case RenderTarget::Velocity0: return 0;
		case RenderTarget::Velocity1: return 1;
		case RenderTarget::Color0:
		case RenderTarget::Color1: return 2;
		case RenderTarget::Obstacles: return 3;
		case RenderTarget::ObstacleVelocity: return 4;
		case RenderTarget::Pressure: return 5;
		case RenderTarget::TempScalar: return 6;
		case RenderTarget::TempVector: return 7;
	}
	// Can't get here
	assert(false);
	return 0;
}


void FluidEngine::InitRootSig()
{
	m_rootSig.Reset(2, 2);
	m_rootSig[0].InitAsConstantBuffer(0, ShaderVisibility::Graphics);
	m_rootSig[1].InitAsDescriptorRange(DescriptorType::TextureSRV, 0, 8, ShaderVisibility::Pixel);
	m_rootSig.InitStaticSampler(0, CommonStates::SamplerPointClamp(), ShaderVisibility::Pixel);
	m_rootSig.InitStaticSampler(1, CommonStates::SamplerLinearClamp(), ShaderVisibility::Pixel);
	m_rootSig.Finalize("Fluid Engine Root Sig", RootSignatureFlags::AllowInputAssemblerInputLayout);
}


void FluidEngine::InitPSOs()
{
	VertexStreamDesc vertexStream{ 0, sizeof(GridVertex), InputClassification::PerVertexData };
	vector<VertexElementDesc> vertexElements =
	{
		{ "POSITION", 0, Format::R32G32B32_Float, 0, offsetof(GridVertex, position), InputClassification::PerVertexData, 0 },
		{ "TEXCOORD", 0, Format::R32G32B32_Float, 0, offsetof(GridVertex, texcoord), InputClassification::PerVertexData, 0 }
	};

	// Advect color
	{
		m_advectColorPSO.SetRootSignature(m_rootSig);
		m_advectColorPSO.SetRenderTargetFormat(GetFormat(RenderTarget::Color0), Format::Unknown);
		m_advectColorPSO.SetBlendState(CommonStates::BlendDisable());
		m_advectColorPSO.SetDepthStencilState(CommonStates::DepthStateDisabled());
		m_advectColorPSO.SetRasterizerState(CommonStates::RasterizerTwoSided());
		m_advectColorPSO.SetPrimitiveTopology(PrimitiveTopology::TriangleList);
		m_advectColorPSO.SetInputLayout(vertexStream, vertexElements);
		m_advectColorPSO.SetVertexShader("FluidGridVS");
		m_advectColorPSO.SetGeometryShader("ArrayGS");
		m_advectColorPSO.SetPixelShader("AdvectPS");
		m_advectColorPSO.Finalize();
	}

	// Advect color forward
	{
		m_advectColorForwardPSO.SetRootSignature(m_rootSig);
		m_advectColorForwardPSO.SetRenderTargetFormat(GetFormat(RenderTarget::TempVector), Format::Unknown);
		m_advectColorForwardPSO.SetBlendState(CommonStates::BlendDisable());
		m_advectColorForwardPSO.SetDepthStencilState(CommonStates::DepthStateDisabled());
		m_advectColorForwardPSO.SetRasterizerState(CommonStates::RasterizerTwoSided());
		m_advectColorForwardPSO.SetPrimitiveTopology(PrimitiveTopology::TriangleList);
		m_advectColorForwardPSO.SetInputLayout(vertexStream, vertexElements);
		m_advectColorForwardPSO.SetVertexShader("FluidGridVS");
		m_advectColorForwardPSO.SetGeometryShader("ArrayGS");
		m_advectColorForwardPSO.SetPixelShader("AdvectPS");
		m_advectColorForwardPSO.Finalize();
	}

	// Advect color back
	{
		m_advectColorBackPSO.SetRootSignature(m_rootSig);
		m_advectColorBackPSO.SetRenderTargetFormat(GetFormat(RenderTarget::TempScalar), Format::Unknown);
		m_advectColorBackPSO.SetBlendState(CommonStates::BlendDisable());
		m_advectColorBackPSO.SetDepthStencilState(CommonStates::DepthStateDisabled());
		m_advectColorBackPSO.SetRasterizerState(CommonStates::RasterizerTwoSided());
		m_advectColorBackPSO.SetPrimitiveTopology(PrimitiveTopology::TriangleList);
		m_advectColorBackPSO.SetInputLayout(vertexStream, vertexElements);
		m_advectColorBackPSO.SetVertexShader("FluidGridVS");
		m_advectColorBackPSO.SetGeometryShader("ArrayGS");
		m_advectColorBackPSO.SetPixelShader("AdvectPS");
		m_advectColorBackPSO.Finalize();
	}

	// Advect color BFECC
	{
		m_advectColorBFECCPSO.SetRootSignature(m_rootSig);
		m_advectColorBFECCPSO.SetRenderTargetFormat(GetFormat(RenderTarget::Color0), Format::Unknown);
		m_advectColorBFECCPSO.SetBlendState(CommonStates::BlendDisable());
		m_advectColorBFECCPSO.SetDepthStencilState(CommonStates::DepthStateDisabled());
		m_advectColorBFECCPSO.SetRasterizerState(CommonStates::RasterizerTwoSided());
		m_advectColorBFECCPSO.SetPrimitiveTopology(PrimitiveTopology::TriangleList);
		m_advectColorBFECCPSO.SetInputLayout(vertexStream, vertexElements);
		m_advectColorBFECCPSO.SetVertexShader("FluidGridVS");
		m_advectColorBFECCPSO.SetGeometryShader("ArrayGS");
		m_advectColorBFECCPSO.SetPixelShader("AdvectBFECCPS");
		m_advectColorBFECCPSO.Finalize();
	}

	// Advect velocity
	{
		m_advectVelocityPSO.SetRootSignature(m_rootSig);
		m_advectVelocityPSO.SetRenderTargetFormat(GetFormat(RenderTarget::Velocity1), Format::Unknown);
		m_advectVelocityPSO.SetBlendState(CommonStates::BlendDisable());
		m_advectVelocityPSO.SetDepthStencilState(CommonStates::DepthStateDisabled());
		m_advectVelocityPSO.SetRasterizerState(CommonStates::RasterizerTwoSided());
		m_advectVelocityPSO.SetPrimitiveTopology(PrimitiveTopology::TriangleList);
		m_advectVelocityPSO.SetInputLayout(vertexStream, vertexElements);
		m_advectVelocityPSO.SetVertexShader("FluidGridVS");
		m_advectVelocityPSO.SetGeometryShader("ArrayGS");
		m_advectVelocityPSO.SetPixelShader("AdvectVelPS");
		m_advectVelocityPSO.Finalize();
	}

	// Vorticity
	{
		m_vorticityPSO.SetRootSignature(m_rootSig);
		m_vorticityPSO.SetRenderTargetFormat(GetFormat(RenderTarget::TempVector), Format::Unknown);
		m_vorticityPSO.SetBlendState(CommonStates::BlendDisable());
		m_vorticityPSO.SetDepthStencilState(CommonStates::DepthStateDisabled());
		m_vorticityPSO.SetRasterizerState(CommonStates::RasterizerTwoSided());
		m_vorticityPSO.SetPrimitiveTopology(PrimitiveTopology::TriangleList);
		m_vorticityPSO.SetInputLayout(vertexStream, vertexElements);
		m_vorticityPSO.SetVertexShader("FluidGridVS");
		m_vorticityPSO.SetGeometryShader("ArrayGS");
		m_vorticityPSO.SetPixelShader("VorticityPS");
		m_vorticityPSO.Finalize();
	}

	// Confinement
	{
		BlendStateDesc blendAdditive{};
		blendAdditive.alphaToCoverageEnable = false;
		blendAdditive.independentBlendEnable = false;
		for (auto& renderTargetBlend : blendAdditive.renderTargetBlend)
		{
			renderTargetBlend.blendEnable = true;
			renderTargetBlend.srcBlend = Blend::One;
			renderTargetBlend.dstBlend = Blend::One;
			renderTargetBlend.blendOp = BlendOp::Add;
			renderTargetBlend.srcBlendAlpha = Blend::One;
			renderTargetBlend.dstBlendAlpha = Blend::One;
			renderTargetBlend.blendOpAlpha = BlendOp::Add;
			renderTargetBlend.writeMask = ColorWrite::All;
		}

		m_confinementPSO.SetRootSignature(m_rootSig);
		m_confinementPSO.SetRenderTargetFormat(GetFormat(RenderTarget::Velocity1), Format::Unknown);
		m_confinementPSO.SetBlendState(blendAdditive);
		m_confinementPSO.SetDepthStencilState(CommonStates::DepthStateDisabled());
		m_confinementPSO.SetRasterizerState(CommonStates::RasterizerTwoSided());
		m_confinementPSO.SetPrimitiveTopology(PrimitiveTopology::TriangleList);
		m_confinementPSO.SetInputLayout(vertexStream, vertexElements);
		m_confinementPSO.SetVertexShader("FluidGridVS");
		m_confinementPSO.SetGeometryShader("ArrayGS");
		m_confinementPSO.SetPixelShader("ConfinementPS");
		m_confinementPSO.Finalize();
	}

	// Gaussian
	{
		BlendStateDesc blendAlpha{};
		blendAlpha.alphaToCoverageEnable = false;
		blendAlpha.independentBlendEnable = false;
		for (auto& renderTargetBlend : blendAlpha.renderTargetBlend)
		{
			renderTargetBlend.blendEnable = true;
			renderTargetBlend.srcBlend = Blend::SrcAlpha;
			renderTargetBlend.dstBlend = Blend::InvSrcAlpha;
			renderTargetBlend.blendOp = BlendOp::Add;
			renderTargetBlend.srcBlendAlpha = Blend::One;
			renderTargetBlend.dstBlendAlpha = Blend::One;
			renderTargetBlend.blendOpAlpha = BlendOp::Add;
			renderTargetBlend.writeMask = ColorWrite::All;
		}

		// Gaussian color
		{
			m_gaussianColorPSO.SetRootSignature(m_rootSig);
			m_gaussianColorPSO.SetRenderTargetFormat(GetFormat(RenderTarget::Color0), Format::Unknown);
			m_gaussianColorPSO.SetBlendState(blendAlpha);
			m_gaussianColorPSO.SetDepthStencilState(CommonStates::DepthStateDisabled());
			m_gaussianColorPSO.SetRasterizerState(CommonStates::RasterizerTwoSided());
			m_gaussianColorPSO.SetPrimitiveTopology(PrimitiveTopology::TriangleList);
			m_gaussianColorPSO.SetInputLayout(vertexStream, vertexElements);
			m_gaussianColorPSO.SetVertexShader("FluidGridVS");
			m_gaussianColorPSO.SetGeometryShader("ArrayGS");
			m_gaussianColorPSO.SetPixelShader("GaussianPS");
			m_gaussianColorPSO.Finalize();
		}

		// Gaussian velocity
		{
			m_gaussianVelocityPSO.SetRootSignature(m_rootSig);
			m_gaussianVelocityPSO.SetRenderTargetFormat(GetFormat(RenderTarget::Velocity0), Format::Unknown);
			m_gaussianVelocityPSO.SetBlendState(blendAlpha);
			m_gaussianVelocityPSO.SetDepthStencilState(CommonStates::DepthStateDisabled());
			m_gaussianVelocityPSO.SetRasterizerState(CommonStates::RasterizerTwoSided());
			m_gaussianVelocityPSO.SetPrimitiveTopology(PrimitiveTopology::TriangleList);
			m_gaussianVelocityPSO.SetInputLayout(vertexStream, vertexElements);
			m_gaussianVelocityPSO.SetVertexShader("FluidGridVS");
			m_gaussianVelocityPSO.SetGeometryShader("ArrayGS");
			m_gaussianVelocityPSO.SetPixelShader("GaussianPS");
			m_gaussianVelocityPSO.Finalize();
		}
	}

	// Divergence
	{
		m_divergencePSO.SetRootSignature(m_rootSig);
		m_divergencePSO.SetRenderTargetFormat(GetFormat(RenderTarget::Velocity1), Format::Unknown);
		m_divergencePSO.SetBlendState(CommonStates::BlendDisable());
		m_divergencePSO.SetDepthStencilState(CommonStates::DepthStateDisabled());
		m_divergencePSO.SetRasterizerState(CommonStates::RasterizerTwoSided());
		m_divergencePSO.SetPrimitiveTopology(PrimitiveTopology::TriangleList);
		m_divergencePSO.SetInputLayout(vertexStream, vertexElements);
		m_divergencePSO.SetVertexShader("FluidGridVS");
		m_divergencePSO.SetGeometryShader("ArrayGS");
		m_divergencePSO.SetPixelShader("DivergencePS");
		m_divergencePSO.Finalize();
	}

	// Pressure
	{
		m_pressurePSO.SetRootSignature(m_rootSig);
		m_pressurePSO.SetRenderTargetFormat(GetFormat(RenderTarget::Pressure), Format::Unknown);
		m_pressurePSO.SetBlendState(CommonStates::BlendDisable());
		m_pressurePSO.SetDepthStencilState(CommonStates::DepthStateDisabled());
		m_pressurePSO.SetRasterizerState(CommonStates::RasterizerTwoSided());
		m_pressurePSO.SetPrimitiveTopology(PrimitiveTopology::TriangleList);
		m_pressurePSO.SetInputLayout(vertexStream, vertexElements);
		m_pressurePSO.SetVertexShader("FluidGridVS");
		m_pressurePSO.SetGeometryShader("ArrayGS");
		m_pressurePSO.SetPixelShader("JacobiPS");
		m_pressurePSO.Finalize();
	}

	// Project velocity
	{
		m_projectVelocityPSO.SetRootSignature(m_rootSig);
		m_projectVelocityPSO.SetRenderTargetFormat(GetFormat(RenderTarget::Velocity0), Format::Unknown);
		m_projectVelocityPSO.SetBlendState(CommonStates::BlendDisable());
		m_projectVelocityPSO.SetDepthStencilState(CommonStates::DepthStateDisabled());
		m_projectVelocityPSO.SetRasterizerState(CommonStates::RasterizerTwoSided());
		m_projectVelocityPSO.SetPrimitiveTopology(PrimitiveTopology::TriangleList);
		m_projectVelocityPSO.SetInputLayout(vertexStream, vertexElements);
		m_projectVelocityPSO.SetVertexShader("FluidGridVS");
		m_projectVelocityPSO.SetGeometryShader("ArrayGS");
		m_projectVelocityPSO.SetPixelShader("ProjectPS");
		m_projectVelocityPSO.Finalize();
	}

	// Debug
	{
		m_debugPSO.SetRootSignature(m_rootSig);
		m_debugPSO.SetRenderTargetFormat(GetApplication()->GetColorFormat(), GetApplication()->GetDepthFormat());
		m_debugPSO.SetBlendState(CommonStates::BlendDisable());
		m_debugPSO.SetDepthStencilState(CommonStates::DepthStateDisabled());
		m_debugPSO.SetRasterizerState(CommonStates::RasterizerTwoSided());
		m_debugPSO.SetPrimitiveTopology(PrimitiveTopology::TriangleList);
		m_debugPSO.SetInputLayout(vertexStream, vertexElements);
		m_debugPSO.SetVertexShader("FluidGridVS");
		m_debugPSO.SetPixelShader("DrawTexturePS");
		m_debugPSO.Finalize();
	}
}


void FluidEngine::InitFBOs()
{
	m_colorFBO[0].SetColorBuffer(0, m_renderTargets[uint32_t(RenderTarget::Color0)]);
	m_colorFBO[0].Finalize();

	m_colorFBO[1].SetColorBuffer(0, m_renderTargets[uint32_t(RenderTarget::Color1)]);
	m_colorFBO[1].Finalize();

	m_tempVectorFBO.SetColorBuffer(0, m_renderTargets[uint32_t(RenderTarget::TempVector)]);
	m_tempVectorFBO.Finalize();

	m_tempScalarFBO.SetColorBuffer(0, m_renderTargets[uint32_t(RenderTarget::TempScalar)]);
	m_tempScalarFBO.Finalize();

	m_velocityFBO[0].SetColorBuffer(0, m_renderTargets[uint32_t(RenderTarget::Velocity0)]);
	m_velocityFBO[0].Finalize();

	m_velocityFBO[1].SetColorBuffer(0, m_renderTargets[uint32_t(RenderTarget::Velocity1)]);
	m_velocityFBO[1].Finalize();

	m_pressureFBO.SetColorBuffer(0, m_renderTargets[uint32_t(RenderTarget::Pressure)]);
	m_pressureFBO.Finalize();
}


void FluidEngine::InitConstantBuffers()
{
	m_advectColorConstantBuffer.Create("Advect Color Constant Buffer", 1, sizeof(FluidConstants));
	m_advectColorForwardConstantBuffer.Create("Advect Color Forward Constant Buffer", 1, sizeof(FluidConstants));
	m_advectColorBackConstantBuffer.Create("Advect Color Back Constant Buffer", 1, sizeof(FluidConstants));
	m_advectColorBFECCConstantBuffer.Create("Advect Color BFECC Constant Buffer", 1, sizeof(FluidConstants));
	m_advectVelocityConstantBuffer.Create("Advect Velocity Constant Buffer", 1, sizeof(FluidConstants));
	m_vorticityConstantBuffer.Create("Vorticity Constant Buffer", 1, sizeof(FluidConstants));
	m_confinementConstantBuffer.Create("Confinement Constant Buffer", 1, sizeof(FluidConstants));
	m_gaussianColorConstantBuffer.Create("Gaussian Color Constant Buffer", 1, sizeof(FluidConstants));
	m_gaussianVelocityConstantBuffer.Create("Gaussian Velocity Constant Buffer", 1, sizeof(FluidConstants));
	m_divergenceConstantBuffer.Create("Velocity Divergence Constant Buffer", 1, sizeof(FluidConstants));
	m_pressureConstantBuffer.Create("Pressure Constant Buffer", 1, sizeof(FluidConstants));
	m_projectVelocityConstantBuffer.Create("Project Velocity Constant Buffer", 1, sizeof(FluidConstants));
	m_debugConstantBuffer.Create("Debug Constant Buffer", 1, sizeof(FluidConstants));
}


void FluidEngine::InitResources()
{
	for (int i = 0; i < 2; ++i)
	{
		RenderTarget colorSource = (i == 0) ? RenderTarget::Color1 : RenderTarget::Color0;
		m_advectColorResources[i].Init(&m_rootSig);
		m_advectColorResources[i].SetCBV(0, 0, m_advectColorConstantBuffer);
		m_advectColorResources[i].SetSRV(1, GetSlot(RenderTarget::Velocity0), GetColorBuffer(RenderTarget::Velocity0));
		m_advectColorResources[i].SetSRV(1, GetSlot(colorSource), GetColorBuffer(colorSource));
		m_advectColorResources[i].SetSRV(1, GetSlot(RenderTarget::Obstacles), GetColorBuffer(RenderTarget::Obstacles));
		m_advectColorResources[i].Finalize();
	}

	for (int i = 0; i < 2; ++i)
	{
		RenderTarget colorSource = (i == 0) ? RenderTarget::Color1 : RenderTarget::Color0;
		m_advectColorForwardResources[i].Init(&m_rootSig);
		m_advectColorForwardResources[i].SetCBV(0, 0, m_advectColorForwardConstantBuffer);
		m_advectColorForwardResources[i].SetSRV(1, GetSlot(RenderTarget::Velocity0), GetColorBuffer(RenderTarget::Velocity0));
		m_advectColorForwardResources[i].SetSRV(1, GetSlot(colorSource), GetColorBuffer(colorSource));
		m_advectColorForwardResources[i].SetSRV(1, GetSlot(RenderTarget::Obstacles), GetColorBuffer(RenderTarget::Obstacles));
		m_advectColorForwardResources[i].Finalize();
	}

	m_advectColorBackResources.Init(&m_rootSig);
	m_advectColorBackResources.SetCBV(0, 0, m_advectColorBackConstantBuffer);
	m_advectColorBackResources.SetSRV(1, GetSlot(RenderTarget::Velocity0), GetColorBuffer(RenderTarget::Velocity0));
	m_advectColorBackResources.SetSRV(1, GetSlot(RenderTarget::Color0), GetColorBuffer(RenderTarget::TempVector));
	m_advectColorBackResources.SetSRV(1, GetSlot(RenderTarget::Obstacles), GetColorBuffer(RenderTarget::Obstacles));
	m_advectColorBackResources.Finalize();

	for (int i = 0; i < 2; ++i)
	{
		RenderTarget colorSource = (i == 0) ? RenderTarget::Color1 : RenderTarget::Color0;
		m_advectColorBFECCResources[i].Init(&m_rootSig);
		m_advectColorBFECCResources[i].SetCBV(0, 0, m_advectColorBFECCConstantBuffer);
		m_advectColorBFECCResources[i].SetSRV(1, GetSlot(RenderTarget::Velocity0), GetColorBuffer(RenderTarget::Velocity0));
		m_advectColorBFECCResources[i].SetSRV(1, GetSlot(colorSource), GetColorBuffer(colorSource));
		m_advectColorBFECCResources[i].SetSRV(1, GetSlot(RenderTarget::Obstacles), GetColorBuffer(RenderTarget::Obstacles));
		m_advectColorBFECCResources[i].SetSRV(1, GetSlot(RenderTarget::TempScalar), GetColorBuffer(RenderTarget::TempScalar));
		m_advectColorBFECCResources[i].Finalize();
	}

	m_advectVelocityResources.Init(&m_rootSig);
	m_advectVelocityResources.SetCBV(0, 0, m_advectVelocityConstantBuffer);
	m_advectVelocityResources.SetSRV(1, GetSlot(RenderTarget::Velocity0), GetColorBuffer(RenderTarget::Velocity0));
	m_advectVelocityResources.Finalize();

	m_vorticityResources.Init(&m_rootSig);
	m_vorticityResources.SetCBV(0, 0, m_vorticityConstantBuffer);
	m_vorticityResources.SetSRV(1, GetSlot(RenderTarget::Velocity1), GetColorBuffer(RenderTarget::Velocity1));
	m_vorticityResources.Finalize();

	m_confinementResources.Init(&m_rootSig);
	m_confinementResources.SetCBV(0, 0, m_confinementConstantBuffer);
	m_confinementResources.SetSRV(1, GetSlot(RenderTarget::TempVector), GetColorBuffer(RenderTarget::TempVector));
	m_confinementResources.Finalize();

	m_gaussianColorResources.Init(&m_rootSig);
	m_gaussianColorResources.SetCBV(0, 0, m_gaussianColorConstantBuffer);
	m_gaussianColorResources.SetSRV(1, GetSlot(RenderTarget::Obstacles), GetColorBuffer(RenderTarget::Obstacles));
	m_gaussianColorResources.Finalize();

	m_gaussianVelocityResources.Init(&m_rootSig);
	m_gaussianVelocityResources.SetCBV(0, 0, m_gaussianVelocityConstantBuffer);
	m_gaussianVelocityResources.SetSRV(1, GetSlot(RenderTarget::Obstacles), GetColorBuffer(RenderTarget::Obstacles));
	m_gaussianVelocityResources.Finalize();

	m_divergenceResources.Init(&m_rootSig);
	m_divergenceResources.SetCBV(0, 0, m_divergenceConstantBuffer);
	m_divergenceResources.SetSRV(1, GetSlot(RenderTarget::Velocity1), GetColorBuffer(RenderTarget::Velocity1));
	m_divergenceResources.SetSRV(1, GetSlot(RenderTarget::Obstacles), GetColorBuffer(RenderTarget::Obstacles));
	m_divergenceResources.SetSRV(1, GetSlot(RenderTarget::ObstacleVelocity), GetColorBuffer(RenderTarget::ObstacleVelocity));
	m_divergenceResources.Finalize();

	for(int i = 0; i < 2; ++i)
	{
		RenderTarget pressureSource = (i == 0) ? RenderTarget::Pressure : RenderTarget::TempScalar;

		m_pressureResources[i].Init(&m_rootSig);
		m_pressureResources[i].SetCBV(0, 0, m_pressureConstantBuffer);
		m_pressureResources[i].SetSRV(1, GetSlot(RenderTarget::Obstacles), GetColorBuffer(RenderTarget::Obstacles));
		m_pressureResources[i].SetSRV(1, GetSlot(RenderTarget::Pressure), GetColorBuffer(pressureSource));
		m_pressureResources[i].Finalize();
	}

	m_projectVelocityResources.Init(&m_rootSig);
	m_projectVelocityResources.SetCBV(0, 0, m_projectVelocityConstantBuffer);
	m_projectVelocityResources.SetSRV(1, GetSlot(RenderTarget::Pressure), GetColorBuffer(RenderTarget::Pressure));
	m_projectVelocityResources.SetSRV(1, GetSlot(RenderTarget::Velocity1), GetColorBuffer(RenderTarget::Velocity1));
	m_projectVelocityResources.SetSRV(1, GetSlot(RenderTarget::Obstacles), GetColorBuffer(RenderTarget::Obstacles));
	m_projectVelocityResources.SetSRV(1, GetSlot(RenderTarget::ObstacleVelocity), GetColorBuffer(RenderTarget::ObstacleVelocity));
	m_projectVelocityResources.Finalize();

	for (int i = 0; i < 2; ++i)
	{
		RenderTarget colorSource = (i == 0) ? RenderTarget::Color0 : RenderTarget::Color1;

		m_debugResources[i].Init(&m_rootSig);
		m_debugResources[i].SetCBV(0, 0, m_debugConstantBuffer);
		m_debugResources[i].SetSRV(1, GetSlot(colorSource), GetColorBuffer(colorSource));
		m_debugResources[i].SetSRV(1, GetSlot(RenderTarget::Velocity0), GetColorBuffer(RenderTarget::Velocity0));
		m_debugResources[i].SetSRV(1, GetSlot(RenderTarget::Obstacles), GetColorBuffer(RenderTarget::Obstacles));
		m_debugResources[i].SetSRV(1, GetSlot(RenderTarget::ObstacleVelocity), GetColorBuffer(RenderTarget::ObstacleVelocity));
		m_debugResources[i].Finalize();
	}
}


void FluidEngine::UpdateConstantBuffers(float deltaT)
{
	using namespace Math;

	const float width = float(m_width);
	const float height = float(m_height);
	const float depth = float(m_depth);

	m_constants = {};
	m_constants.obstVelocity = Vector4{ kZero };
	m_constants.splatColor = Vector4{ 0.0f, 0.0f, 0.0f, 1.0f };
	m_constants.texDim = Vector3{ width, height, depth };
	m_constants.invTexDim = Vector3{ 1.0f / width, 1.0f / height, 1.0f / depth };
	m_constants.center = Vector3{ kZero };
	m_constants.halfVolumeDim = Vector3{ width / 2.0f, height / 2.0f, depth / 2.0f };
	m_constants.boxLBDCorner = Vector3{ kZero };
	m_constants.boxRTUCorner = Vector3{ kOne };
	m_constants.texNumber = m_debugTexNumber;
	m_constants.timestep = deltaT;

	// Advect color
	m_constants.modulate = m_decay;
	m_constants.forward = 1.0f;
	m_advectColorConstantBuffer.Update(sizeof(FluidConstants), &m_constants);

	// Advect color forward
	m_constants.modulate = 1.0f;
	m_constants.forward = 1.0f;
	m_advectColorForwardConstantBuffer.Update(sizeof(FluidConstants), &m_constants);

	// Advect color back
	m_constants.modulate = 1.0f;
	m_constants.forward = -1.0f;
	m_advectColorBackConstantBuffer.Update(sizeof(FluidConstants), &m_constants);

	// Advect color BFECC
	m_constants.modulate = m_decay;
	m_constants.forward = 1.0f;
	m_advectColorBFECCConstantBuffer.Update(sizeof(FluidConstants), &m_constants);

	// Advect velocity
	m_constants.modulate = 1.0f;
	m_constants.forward = 1.0f;
	m_advectVelocityConstantBuffer.Update(sizeof(FluidConstants), &m_constants);

	// Vorticity
	m_vorticityConstantBuffer.Update(sizeof(FluidConstants), &m_constants);

	// Confinement
	m_constants.epsilon = m_confinementScale;
	m_confinementConstantBuffer.Update(sizeof(FluidConstants), &m_constants);

	// Gaussian color
	m_constants.size = m_impulseSize;
	m_constants.center = m_impulsePosition;
	static float t{ 0.0f };
	t += 0.05f;
	const float density = 1.5f * (((sinf(t + 2.0f * DirectX::XM_PI / 3.0f) * 0.5f + 0.5f)) * m_saturation + (1.0f - m_saturation));
	m_constants.splatColor = Vector4(density, density, density, 1.0f);
	m_gaussianColorConstantBuffer.Update(sizeof(FluidConstants), &m_constants);

	// Gaussian velocity
	m_constants.splatColor = Vector4(m_impulseVelocity, 1.0f);
	m_constants.center = m_impulsePosition + Vector3(g_rng.NextFloat(-2.5f, 2.5f), g_rng.NextFloat(-2.5f, 2.5f), g_rng.NextFloat(-2.5f, 2.5f));
	m_gaussianVelocityConstantBuffer.Update(sizeof(FluidConstants), &m_constants);

	// Divergence
	m_divergenceConstantBuffer.Update(sizeof(FluidConstants), &m_constants);

	// Pressure
	m_pressureConstantBuffer.Update(sizeof(FluidConstants), &m_constants);

	// Project Velocity
	m_constants.modulate = 1.0f;
	m_projectVelocityConstantBuffer.Update(sizeof(FluidConstants), &m_constants);

	// Debug
	m_debugConstantBuffer.Update(sizeof(FluidConstants), &m_constants);
}


void FluidEngine::AdvectColorBFECC(GraphicsContext& context)
{
	ScopedDrawEvent outer(context, "Advect Color BFECC");

	// Advect forward to get \phi^(n+1)
	{
		ScopedDrawEvent inner(context, "Advect Forward");

		RenderTarget readSource = (m_colorTexNumber == 0) ? RenderTarget::Color1 : RenderTarget::Color0;

		context.TransitionResource(GetColorBuffer(RenderTarget::TempVector), ResourceState::RenderTarget);
		context.TransitionResource(GetColorBuffer(readSource), ResourceState::PixelShaderResource);
		context.TransitionResource(GetColorBuffer(RenderTarget::Obstacles), ResourceState::PixelShaderResource);
		context.TransitionResource(GetColorBuffer(RenderTarget::Velocity0), ResourceState::PixelShaderResource);
		context.ClearColor(GetColorBuffer(RenderTarget::TempVector));

		context.BeginRenderPass(m_tempVectorFBO);

		context.SetViewportAndScissor(0, 0, m_width, m_height);

		context.SetRootSignature(m_rootSig);
		context.SetPipelineState(m_advectColorForwardPSO);
		context.SetResources((m_colorTexNumber == 0) ? m_advectColorForwardResources[0] : m_advectColorForwardResources[1]);

		m_grid.DrawSlices(context);

		context.EndRenderPass();
	}

	// Advect back to get \bar{\phi}
	{
		ScopedDrawEvent inner(context, "Advect Back");

		context.TransitionResource(GetColorBuffer(RenderTarget::TempScalar), ResourceState::RenderTarget);
		context.TransitionResource(GetColorBuffer(RenderTarget::TempVector), ResourceState::PixelShaderResource);
		context.ClearColor(GetColorBuffer(RenderTarget::TempScalar));

		context.BeginRenderPass(m_tempScalarFBO);

		context.SetViewportAndScissor(0, 0, m_width, m_height);

		context.SetRootSignature(m_rootSig);
		context.SetPipelineState(m_advectColorBackPSO);
		context.SetResources(m_advectColorBackResources);

		m_grid.DrawSlices(context);

		context.EndRenderPass();
	}

	// Advect forward with BFECC shader
	{
		ScopedDrawEvent inner(context, "Advect Forward (BFECC)");

		RenderTarget readSource = (m_colorTexNumber == 0) ? RenderTarget::Color1 : RenderTarget::Color0;
		RenderTarget writeDest = (m_colorTexNumber == 0) ? RenderTarget::Color0 : RenderTarget::Color1;

		context.TransitionResource(GetColorBuffer(writeDest), ResourceState::RenderTarget);
		context.TransitionResource(GetColorBuffer(readSource), ResourceState::PixelShaderResource);
		context.TransitionResource(GetColorBuffer(RenderTarget::TempScalar), ResourceState::PixelShaderResource);

		context.BeginRenderPass((m_colorTexNumber == 0) ? m_colorFBO[0] : m_colorFBO[1]);

		context.SetViewportAndScissor(0, 0, m_width, m_height);

		context.SetRootSignature(m_rootSig);
		context.SetPipelineState(m_advectColorBFECCPSO);
		context.SetResources((m_colorTexNumber == 0) ? m_advectColorBFECCResources[0] : m_advectColorBFECCResources[1]);

		m_grid.DrawSlices(context);

		context.EndRenderPass();
	}
}


void FluidEngine::AdvectColor(GraphicsContext& context)
{
	ScopedDrawEvent event(context, "Advect Color");

	RenderTarget target = (m_colorTexNumber == 0) ? RenderTarget::Color0 : RenderTarget::Color1;
	RenderTarget resource = (m_colorTexNumber == 0) ? RenderTarget::Color1 : RenderTarget::Color0;

	context.TransitionResource(GetColorBuffer(target), ResourceState::RenderTarget);
	context.TransitionResource(GetColorBuffer(RenderTarget::Obstacles), ResourceState::PixelShaderResource);
	context.TransitionResource(GetColorBuffer(RenderTarget::Velocity0), ResourceState::PixelShaderResource);
	context.TransitionResource(GetColorBuffer(resource), ResourceState::PixelShaderResource);
	
	context.BeginRenderPass(m_colorFBO[m_colorTexNumber]);

	context.SetViewportAndScissor(0, 0, m_width, m_height);

	context.SetRootSignature(m_rootSig);
	context.SetPipelineState(m_advectColorPSO);
	context.SetResources((m_colorTexNumber == 0) ? m_advectColorResources[0] : m_advectColorResources[1]);

	m_grid.DrawSlices(context);

	context.EndRenderPass();
}


void FluidEngine::AdvectVelocity(GraphicsContext& context)
{
	ScopedDrawEvent event(context, "Advect Velocity");

	context.TransitionResource(GetColorBuffer(RenderTarget::Velocity1), ResourceState::RenderTarget);
	context.TransitionResource(GetColorBuffer(RenderTarget::Velocity0), ResourceState::PixelShaderResource);

	context.BeginRenderPass(m_velocityFBO[1]);

	context.SetViewportAndScissor(0, 0, m_width, m_height);
	context.SetRootSignature(m_rootSig);
	context.SetPipelineState(m_advectVelocityPSO);
	context.SetResources(m_advectVelocityResources);

	m_grid.DrawSlices(context);

	context.EndRenderPass();
}


void FluidEngine::ApplyVorticityConfinement(GraphicsContext& context)
{
	ScopedDrawEvent outer(context, "Apply Vorticity and Confinement");

	// Vorticity
	{
		ScopedDrawEvent inner(context, "Vorticity");

		context.TransitionResource(GetColorBuffer(RenderTarget::TempVector), ResourceState::RenderTarget);
		context.TransitionResource(GetColorBuffer(RenderTarget::Velocity1), ResourceState::PixelShaderResource);
		context.ClearColor(GetColorBuffer(RenderTarget::TempVector));

		context.BeginRenderPass(m_tempVectorFBO);

		context.SetViewportAndScissor(0, 0, m_width, m_height);
		context.SetRootSignature(m_rootSig);
		context.SetPipelineState(m_vorticityPSO);
		context.SetResources(m_vorticityResources);

		m_grid.DrawSlices(context);

		context.EndRenderPass();
	}

	// Confinement
	{
		ScopedDrawEvent inner(context, "Confinement");

		context.TransitionResource(GetColorBuffer(RenderTarget::Velocity1), ResourceState::RenderTarget);
		context.TransitionResource(GetColorBuffer(RenderTarget::TempVector), ResourceState::PixelShaderResource);

		context.BeginRenderPass(m_velocityFBO[1]);

		context.SetViewportAndScissor(0, 0, m_width, m_height);
		context.SetRootSignature(m_rootSig);
		context.SetPipelineState(m_confinementPSO);
		context.SetResources(m_confinementResources);

		m_grid.DrawSlices(context);

		context.EndRenderPass();
	}
}


void FluidEngine::ApplyExternalForces(GraphicsContext& context)
{
	ScopedDrawEvent outer(context, "Apply External Forces");

	// Gaussian color
	{
		ScopedDrawEvent inner(context, "Color Impulse Gaussian");

		RenderTarget target = (m_colorTexNumber == 0) ? RenderTarget::Color0 : RenderTarget::Color1;

		context.TransitionResource(GetColorBuffer(target), ResourceState::RenderTarget);

		context.BeginRenderPass(m_colorFBO[m_colorTexNumber]);

		context.SetViewportAndScissor(0, 0, m_width, m_height);
		context.SetRootSignature(m_rootSig);
		context.SetPipelineState(m_gaussianColorPSO);
		context.SetResources(m_gaussianColorResources);

		m_grid.DrawSlices(context);

		context.EndRenderPass();
	}

	// Gaussian velocity
	{
		ScopedDrawEvent inner(context, "Velocity Impulse Gaussian");

		context.TransitionResource(GetColorBuffer(RenderTarget::Velocity1), ResourceState::RenderTarget);

		context.BeginRenderPass(m_velocityFBO[1]);

		context.SetViewportAndScissor(0, 0, m_width, m_height);
		context.SetRootSignature(m_rootSig);
		context.SetPipelineState(m_gaussianVelocityPSO);
		context.SetResources(m_gaussianVelocityResources);

		m_grid.DrawSlices(context);

		context.EndRenderPass();
	}
}


void FluidEngine::ComputeVelocityDivergence(GraphicsContext& context)
{
	ScopedDrawEvent event(context, "Velocity Divergence");

	context.TransitionResource(GetColorBuffer(RenderTarget::TempVector), ResourceState::RenderTarget);
	context.TransitionResource(GetColorBuffer(RenderTarget::ObstacleVelocity), ResourceState::PixelShaderResource);
	context.TransitionResource(GetColorBuffer(RenderTarget::Velocity1), ResourceState::PixelShaderResource);
	context.ClearColor(GetColorBuffer(RenderTarget::TempVector));

	context.BeginRenderPass(m_tempVectorFBO);

	context.SetViewportAndScissor(0, 0, m_width, m_height);
	context.SetRootSignature(m_rootSig);
	context.SetPipelineState(m_divergencePSO);
	context.SetResources(m_divergenceResources);

	m_grid.DrawSlices(context);

	context.EndRenderPass();
}


void FluidEngine::ComputePressure(GraphicsContext& context)
{
	ScopedDrawEvent outer(context, "Compute Pressure");

	context.TransitionResource(GetColorBuffer(RenderTarget::TempScalar), ResourceState::RenderTarget);
	context.ClearColor(GetColorBuffer(RenderTarget::TempScalar));

	for (uint32_t i = 0; i < m_iterations; i += 2)
	{
		{
			ScopedDrawEvent inner(context, format("Iteration {}", i));

			if (i > 0)
			{
				context.TransitionResource(GetColorBuffer(RenderTarget::TempScalar), ResourceState::RenderTarget);
			}

			context.TransitionResource(GetColorBuffer(RenderTarget::Pressure), ResourceState::PixelShaderResource);

			context.BeginRenderPass(m_tempScalarFBO);

			context.SetViewportAndScissor(0, 0, m_width, m_height);
			context.SetRootSignature(m_rootSig);
			context.SetPipelineState(m_pressurePSO);
			context.SetResources(m_pressureResources[i % 2]);

			m_grid.DrawSlices(context);

			context.EndRenderPass();
		}

		{
			ScopedDrawEvent inner(context, format("Iteration {}", i + 1));

			context.TransitionResource(GetColorBuffer(RenderTarget::Pressure), ResourceState::RenderTarget);
			context.TransitionResource(GetColorBuffer(RenderTarget::TempScalar), ResourceState::PixelShaderResource);

			context.BeginRenderPass(m_pressureFBO);

			context.SetViewportAndScissor(0, 0, m_width, m_height);
			context.SetRootSignature(m_rootSig);
			context.SetPipelineState(m_pressurePSO);
			context.SetResources(m_pressureResources[(i+1) % 2]);

			m_grid.DrawSlices(context);

			context.EndRenderPass();
		}
	}
}


void FluidEngine::ProjectVelocity(GraphicsContext& context)
{
	ScopedDrawEvent event(context, "Project Velocity");

	context.TransitionResource(GetColorBuffer(RenderTarget::Velocity0), ResourceState::RenderTarget);
	context.TransitionResource(GetColorBuffer(RenderTarget::Pressure), ResourceState::PixelShaderResource);

	context.BeginRenderPass(m_velocityFBO[0]);

	context.SetViewportAndScissor(0, 0, m_width, m_height);
	context.SetRootSignature(m_rootSig);
	context.SetPipelineState(m_projectVelocityPSO);
	context.SetResources(m_projectVelocityResources);

	m_grid.DrawSlices(context);

	context.EndRenderPass();
}