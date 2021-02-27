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


void FluidEngine::Update(float deltaT)
{
	// Set vorticity confinement and decay parameters
	m_confinementScale = m_bUseBFECC ? 0.06f : 0.12f;
	m_decay = m_bUseBFECC ? 0.994f : 0.9995f;

	UpdateConstantBuffers(deltaT);

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
		context.Finish();
	}

	// Flip textures
	m_colorTexNumber = 1 - m_colorTexNumber;
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
		m_advectVelocityPSO.SetRenderTargetFormat(GetFormat(RenderTarget::Velocity0), Format::Unknown);
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
}


void FluidEngine::InitConstantBuffers()
{
	m_advectColorConstantBuffer.Create("Advect Color Constant Buffer", 1, sizeof(FluidConstants));
	m_advectColorForwardConstantBuffer.Create("Advect Color Forward Constant Buffer", 1, sizeof(FluidConstants));
	m_advectColorBackConstantBuffer.Create("Advect Color Back Constant Buffer", 1, sizeof(FluidConstants));
	m_advectColorBFECCConstantBuffer.Create("Advect Color BFECC Constant Buffer", 1, sizeof(FluidConstants));
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
}


void FluidEngine::UpdateConstantBuffers(float deltaT)
{
	m_advectColorConstants = {};
	m_advectColorConstants.timestep = deltaT;
	m_advectColorConstants.modulate =m_decay;
	m_advectColorConstants.forward = 1.0f;
	m_advectColorConstantBuffer.Update(sizeof(FluidConstants), &m_advectColorConstants);

	m_advectColorForwardConstants = {};
	m_advectColorForwardConstants.timestep = deltaT;
	m_advectColorForwardConstants.modulate = 1.0f;
	m_advectColorForwardConstants.forward = 1.0f;
	m_advectColorForwardConstantBuffer.Update(sizeof(FluidConstants), &m_advectColorForwardConstants);

	m_advectColorBackConstants = {};
	m_advectColorBackConstants.timestep = deltaT;
	m_advectColorBackConstants.modulate = 1.0f;
	m_advectColorBackConstants.forward = -1.0f;
	m_advectColorBackConstantBuffer.Update(sizeof(FluidConstants), &m_advectColorBackConstants);

	m_advectColorBFECCConstants = {};
	m_advectColorBFECCConstants.halfVolumeDim = Math::Vector3(float(m_width) / 2.0f, float(m_height) / 2.0f, float(m_depth) / 2.0f);
	m_advectColorBFECCConstants.timestep = deltaT;
	m_advectColorBFECCConstants.modulate = m_decay;
	m_advectColorBFECCConstants.forward = 1.0f;
	m_advectColorBFECCConstantBuffer.Update(sizeof(FluidConstants), &m_advectColorBFECCConstants);
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