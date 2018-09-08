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

#include "ComputeParticlesApp.h"

#include "CommonStates.h"
#include "CommandContext.h"
#include "Filesystem.h"
#include "GraphicsDevice.h"
#include "Input.h"


using namespace Kodiak;
using namespace Math;
using namespace std;


void ComputeParticlesApp::Configure()
{
	// Setup file system
	auto& filesystem = Kodiak::Filesystem::GetInstance();

	filesystem.SetDefaultRootDir();
	filesystem.AddSearchPath("Data\\" + GetDefaultShaderPath());
	filesystem.AddSearchPath("Data\\Textures");
}


void ComputeParticlesApp::Startup()
{
	InitRootSigs();
	InitPSOs();
	InitConstantBuffers();
	InitParticles();

	LoadAssets();
}


void ComputeParticlesApp::Shutdown()
{
	m_computeRootSig.Destroy();
	m_graphicsRootSig.Destroy();
}


bool ComputeParticlesApp::Update()
{
	if (m_animStart > 0.0f)
	{
		m_animStart -= m_frameTimer * 5.0f;
	}
	else if (m_animStart <= 0.0f)
	{
		m_localTimer += m_frameTimer * 0.04f;
		if (m_localTimer > 1.0f)
		{
			m_localTimer = 0.f;
		}
	}

	UpdateConstantBuffers();

	return true;
}


void ComputeParticlesApp::Render()
{
	auto& context = GraphicsContext::Begin("Render frame");

	// Simulate particles on compute
	{
		auto& computeContext = context.GetComputeContext();

		computeContext.TransitionResource(m_particleBuffer, ResourceState::UnorderedAccess);

		computeContext.SetRootSignature(m_computeRootSig);
		computeContext.SetPipelineState(m_computePSO);

		computeContext.SetUAV(0, 0, m_particleBuffer);
		computeContext.SetConstantBuffer(0, 1, m_csConstantBuffer);

		computeContext.Dispatch1D(m_particleCount, 256);

		computeContext.TransitionResource(m_particleBuffer, ResourceState::NonPixelShaderResource);
	}

	// Render particles
	context.TransitionResource(m_particleBuffer, ResourceState::UnorderedAccess);
	context.TransitionResource(GetColorBuffer(), ResourceState::RenderTarget);
	context.TransitionResource(GetDepthBuffer(), ResourceState::DepthWrite);
	context.ClearColor(GetColorBuffer());
	context.ClearDepth(GetDepthBuffer());

	context.BeginRenderPass(GetBackBuffer());

	context.SetRootSignature(m_graphicsRootSig);
	context.SetPipelineState(m_graphicsPSO);

	context.SetViewportAndScissor(0u, 0u, m_displayWidth, m_displayHeight);

	context.SetSRV(0, 0, m_particleBuffer);
	context.SetConstantBuffer(0, 1, m_vsConstantBuffer);
	context.SetSRV(1, 0, *m_colorTexture);
	context.SetSRV(1, 1, *m_gradientTexture);

	context.Draw(6 * m_particleCount);

	context.EndRenderPass();
	context.TransitionResource(GetColorBuffer(), ResourceState::Present);

	context.Finish();
}


void ComputeParticlesApp::InitRootSigs()
{
	m_computeRootSig.Reset(1);
	m_computeRootSig[0].InitAsDescriptorTable(2, ShaderVisibility::Compute);
	m_computeRootSig[0].SetTableRange(0, DescriptorType::StructuredBufferUAV, 0, 1);
	m_computeRootSig[0].SetTableRange(1, DescriptorType::CBV, 0, 1);
	m_computeRootSig.Finalize("Compute Root Sig");

	m_graphicsRootSig.Reset(2, 1);
	m_graphicsRootSig[0].InitAsDescriptorTable(2, ShaderVisibility::Vertex);
	m_graphicsRootSig[0].SetTableRange(0, DescriptorType::StructuredBufferSRV, 0, 1);
	m_graphicsRootSig[0].SetTableRange(1, DescriptorType::CBV, 0, 1);
	m_graphicsRootSig[1].InitAsDescriptorTable(2, ShaderVisibility::Pixel);
	m_graphicsRootSig[1].SetTableRange(0, DescriptorType::TextureSRV, 0, 1);
	m_graphicsRootSig[1].SetTableRange(1, DescriptorType::TextureSRV, 1, 1);
	m_graphicsRootSig.InitStaticSampler(0, CommonStates::SamplerLinearWrap(), ShaderVisibility::Pixel);
	m_graphicsRootSig.Finalize("Graphics Root Sig");
}


void ComputeParticlesApp::InitPSOs()
{
	m_computePSO.SetRootSignature(m_computeRootSig);
	m_computePSO.SetComputeShader("ParticleCS");
	m_computePSO.Finalize();

	m_graphicsPSO.SetRootSignature(m_graphicsRootSig);

	BlendStateDesc blendDesc{};
	blendDesc.renderTargetBlend[0].blendEnable = TRUE;
	blendDesc.renderTargetBlend[0].blendOp = BlendOp::Add;
	blendDesc.renderTargetBlend[0].srcBlend = Blend::One;
	blendDesc.renderTargetBlend[0].dstBlend = Blend::One;
	blendDesc.renderTargetBlend[0].blendOpAlpha = BlendOp::Add;
	blendDesc.renderTargetBlend[0].srcBlendAlpha = Blend::One;
	blendDesc.renderTargetBlend[0].dstBlendAlpha = Blend::One;
	blendDesc.renderTargetBlend[0].writeMask = ColorWrite::All;

	m_graphicsPSO.SetBlendState(blendDesc);
	m_graphicsPSO.SetDepthStencilState(CommonStates::DepthStateDisabled());
	m_graphicsPSO.SetRasterizerState(CommonStates::RasterizerDefault());

	m_graphicsPSO.SetRenderTargetFormat(GetColorFormat(), GetDepthFormat());

	m_graphicsPSO.SetPrimitiveTopology(PrimitiveTopology::TriangleList);

	m_graphicsPSO.SetVertexShader("ParticleVS");
	m_graphicsPSO.SetPixelShader("ParticlePS");

	m_graphicsPSO.Finalize();
}


void ComputeParticlesApp::InitConstantBuffers()
{
	m_csConstantBuffer.Create("CS Constant Buffer", 1, sizeof(CSConstants));
	m_vsConstantBuffer.Create("VS Constant Buffer", 1, sizeof(VSConstants));

	UpdateConstantBuffers();
}


void ComputeParticlesApp::InitParticles()
{
	vector<Particle> particles(m_particleCount);
	RandomNumberGenerator rng;

	for (auto& particle : particles)
	{
		particle.pos[0] = rng.NextFloat(-1.0f, 1.0f);
		particle.pos[1] = rng.NextFloat(-1.0f, 1.0f);
		particle.vel[0] = 0.0f;
		particle.vel[1] = 0.0f;
		particle.gradientPos = Vector4(0.5f * particle.pos[0], 0.0f, 0.0f, 0.0f);
	}

	m_particleBuffer.Create("Particle SB", m_particleCount, sizeof(Particle), particles.data());
}


void ComputeParticlesApp::UpdateConstantBuffers()
{
	m_csConstants.deltaT = m_frameTimer * 2.5f;
	m_csConstants.destX = sinf(DirectX::XMConvertToRadians(m_localTimer * 360.0f)) * 0.75f;;
	m_csConstants.destY = 0.0f;
	m_csConstants.particleCount = m_particleCount;

	m_csConstantBuffer.Update(sizeof(CSConstants), &m_csConstants);

	m_vsConstants.invTargetSize[0] = 1.0f / GetColorBuffer().GetWidth();
	m_vsConstants.invTargetSize[1] = 1.0f / GetColorBuffer().GetHeight();
	m_vsConstants.pointSize = 8;

	m_vsConstantBuffer.Update(sizeof(VSConstants), &m_vsConstants);
}


void ComputeParticlesApp::LoadAssets()
{
	m_colorTexture = Texture::Load("particle01_rgba.ktx");
	m_gradientTexture = Texture::Load("particle_gradient_rgba.ktx");
}