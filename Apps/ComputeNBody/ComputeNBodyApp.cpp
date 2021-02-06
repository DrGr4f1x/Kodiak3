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

#include "ComputeNBodyApp.h"

#include "Graphics\CommonStates.h"
#include "Graphics\CommandContext.h"
#include "GraphicsFeatures.h"
#include "UIOverlay.h"


using namespace Kodiak;
using namespace Math;
using namespace std;


#define PARTICLES_PER_ATTRACTOR 4 * 1024


void ComputeNBodyApp::Configure()
{
	Application::Configure();

	// Specify required graphics features 
	g_requiredFeatures.geometryShader = true;
}


void ComputeNBodyApp::Startup()
{
	m_camera.SetPerspectiveMatrix(
		XMConvertToRadians(60.0f),
		(float)m_displayHeight / (float)m_displayWidth,
		0.1f,
		512.0f);
	m_camera.SetPosition(Math::Vector3(-8.0f, -8.0f, -8.0f));

	m_controller.SetSpeedScale(0.01f);
	m_controller.SetCameraMode(CameraMode::ArcBall);
	m_controller.SetOrbitTarget(Vector3(0.0f, 0.0f, 0.0f), Length(m_camera.GetPosition()), 2.0f);

	InitRootSigs();
	InitPSOs();
	InitConstantBuffers();
	InitParticles();

	LoadAssets();

	InitResourceSets();
}


void ComputeNBodyApp::Shutdown()
{
	m_rootSig.Destroy();
	m_computeRootSig.Destroy();
}


bool ComputeNBodyApp::Update()
{
	m_controller.Update(m_frameTimer);

	UpdateConstantBuffers();

	return true;
}


void ComputeNBodyApp::Render()
{
	auto& context = GraphicsContext::Begin("Scene");

	// Particle simulation
	{
		auto& computeContext = context.GetComputeContext();

		computeContext.TransitionResource(m_particleBuffer, ResourceState::UnorderedAccess);

		computeContext.SetRootSignature(m_computeRootSig);
		computeContext.SetPipelineState(m_computeCalculatePSO);

		computeContext.SetResources(m_computeResources);

		computeContext.Dispatch1D(6 * PARTICLES_PER_ATTRACTOR, 256);

		computeContext.InsertUAVBarrier(m_particleBuffer);

		computeContext.SetPipelineState(m_computeIntegratePSO);

		computeContext.Dispatch1D(6 * PARTICLES_PER_ATTRACTOR, 256);

		computeContext.TransitionResource(m_particleBuffer, ResourceState::NonPixelShaderResource);
	}

	context.TransitionResource(GetColorBuffer(), ResourceState::RenderTarget);
	context.TransitionResource(GetDepthBuffer(), ResourceState::DepthWrite);
	context.ClearColor(GetColorBuffer());
	context.ClearDepth(GetDepthBuffer());

	context.BeginRenderPass(GetBackBuffer());

	context.SetViewportAndScissor(0u, 0u, m_displayWidth, m_displayHeight);

	// Draw particles
	context.SetRootSignature(m_rootSig);
	context.SetPipelineState(m_PSO);

	context.SetResources(m_graphicsResources);

	context.Draw(6 * PARTICLES_PER_ATTRACTOR);

	RenderUI(context);

	context.EndRenderPass();
	context.TransitionResource(GetColorBuffer(), ResourceState::Present);

	context.Finish();
}


void ComputeNBodyApp::InitRootSigs()
{
	m_rootSig.Reset(3, 1);
	m_rootSig[0].InitAsDescriptorTable(2, ShaderVisibility::Vertex);
	m_rootSig[0].SetTableRange(0, DescriptorType::CBV, 0, 1);
	m_rootSig[0].SetTableRange(1, DescriptorType::StructuredBufferSRV, 0, 1);
	m_rootSig[1].InitAsDescriptorRange(DescriptorType::CBV, 0, 1, ShaderVisibility::Geometry);
	m_rootSig[2].InitAsDescriptorRange(DescriptorType::TextureSRV, 0, 2, ShaderVisibility::Pixel);
	m_rootSig.InitStaticSampler(0, CommonStates::SamplerLinearClamp(), ShaderVisibility::Pixel);
	m_rootSig.Finalize("Root Sig", RootSignatureFlags::AllowInputAssemblerInputLayout);

	m_computeRootSig.Reset(1);
	m_computeRootSig[0].InitAsDescriptorTable(2, ShaderVisibility::Compute);
	m_computeRootSig[0].SetTableRange(0, DescriptorType::CBV, 0, 1);
	m_computeRootSig[0].SetTableRange(1, DescriptorType::StructuredBufferUAV, 0, 1);
	m_computeRootSig.Finalize("Compute Root Sig");
}


void ComputeNBodyApp::InitPSOs()
{
	m_PSO.SetRootSignature(m_rootSig);
	m_PSO.SetDepthStencilState(CommonStates::DepthStateDisabled());

	BlendStateDesc desc{};
	desc.alphaToCoverageEnable = false;
	desc.independentBlendEnable = false;
	desc.renderTargetBlend[0].blendEnable = true;
	desc.renderTargetBlend[0].srcBlend = Blend::One;
	desc.renderTargetBlend[0].dstBlend = Blend::One;
	desc.renderTargetBlend[0].blendOp = BlendOp::Add;
	desc.renderTargetBlend[0].srcBlendAlpha = Blend::SrcAlpha;
	desc.renderTargetBlend[0].dstBlendAlpha = Blend::DstAlpha;
	desc.renderTargetBlend[0].blendOpAlpha = BlendOp::Add;
	desc.renderTargetBlend[0].writeMask = ColorWrite::All;
	m_PSO.SetBlendState(desc);

	m_PSO.SetRasterizerState(CommonStates::RasterizerTwoSided());
	m_PSO.SetRenderTargetFormat(GetColorFormat(), GetDepthFormat());
	m_PSO.SetPrimitiveTopology(PrimitiveTopology::PointList);
	m_PSO.SetVertexShader("ParticleVS");
	m_PSO.SetGeometryShader("ParticleGS");
	m_PSO.SetPixelShader("ParticlePS");
	m_PSO.Finalize();

	m_computeCalculatePSO.SetRootSignature(m_computeRootSig);
	m_computeCalculatePSO.SetComputeShader("ParticleCalculateCS");
	m_computeCalculatePSO.Finalize();

	m_computeIntegratePSO.SetRootSignature(m_computeRootSig);
	m_computeIntegratePSO.SetComputeShader("ParticleIntegrateCS");
	m_computeIntegratePSO.Finalize();
}


void ComputeNBodyApp::InitConstantBuffers()
{
	m_graphicsConstantBuffer.Create("Constant Buffer", 1, sizeof(GraphicsConstants));
	m_computeConstantBuffer.Create("Compute Constant Buffer", 1, sizeof(ComputeConstants));
}


void ComputeNBodyApp::InitResourceSets()
{
	m_graphicsResources.Init(&m_rootSig);
	m_graphicsResources.SetCBV(0, 0, m_graphicsConstantBuffer);
	m_graphicsResources.SetSRV(0, 1, m_particleBuffer);
	m_graphicsResources.SetCBV(1, 0, m_graphicsConstantBuffer);
	m_graphicsResources.SetSRV(2, 0, *m_colorTexture);
	m_graphicsResources.SetSRV(2, 1, *m_gradientTexture);
	m_graphicsResources.Finalize();

	m_computeResources.Init(&m_computeRootSig);
	m_computeResources.SetCBV(0, 0, m_computeConstantBuffer);
	m_computeResources.SetUAV(0, 1, m_particleBuffer);
	m_computeResources.Finalize();
}


void ComputeNBodyApp::InitParticles()
{
	vector<Vector3> attractors = 
	{
		Vector3(5.0f, 0.0f, 0.0f),
		Vector3(-5.0f, 0.0f, 0.0f),
		Vector3(0.0f, 0.0f, 5.0f),
		Vector3(0.0f, 0.0f, -5.0f),
		Vector3(0.0f, 4.0f, 0.0f),
		Vector3(0.0f, -8.0f, 0.0f),
	};

	uint32_t numParticles = static_cast<uint32_t>(attractors.size()) * PARTICLES_PER_ATTRACTOR;

	vector<Particle> particles(numParticles);

	for (uint32_t i = 0; i < static_cast<uint32_t>(attractors.size()); i++)
	{
		for (uint32_t j = 0; j < PARTICLES_PER_ATTRACTOR; j++)
		{
			Particle &particle = particles[i * PARTICLES_PER_ATTRACTOR + j];

			// First particle in group as heavy center of gravity
			if (j == 0)
			{
				particle.pos = Vector4(1.5f * attractors[i], 90000.0f);
				particle.vel = Vector4(0.0f);
			}
			else
			{
				// Position					
				Vector3 position(attractors[i] + 0.75f * Vector3(g_rng.Normal(), g_rng.Normal(), g_rng.Normal()));
				float len = Length(Normalize(position - attractors[i]));
				position.SetY(position.GetY() * (2.0f - (len * len)));

				// Velocity
				Vector3 angular = (((i % 2) == 0) ? 1.0f : -1.0f) * Vector3(0.5f, 1.5f, 0.5f);
				Vector3 velocity = Cross((position - attractors[i]), angular) + Vector3(g_rng.Normal(), g_rng.Normal(), g_rng.Normal() * 0.025f);

				float mass = (g_rng.Normal() * 0.5f + 0.5f) * 75.0f;
				particle.pos = Vector4(position, mass);
				particle.vel = Vector4(velocity, 0.0f);
			}

			// Color gradient offset
			particle.vel.SetW((float)i * 1.0f / static_cast<uint32_t>(attractors.size()));
		}
	}

	m_particleBuffer.Create("Particle UAV", particles.size(), sizeof(Particle), false, particles.data());
}


void ComputeNBodyApp::LoadAssets()
{
	m_gradientTexture = Texture::Load("particle_gradient_rgba.ktx");
	m_colorTexture = Texture::Load("particle01_rgba.ktx");
}


void ComputeNBodyApp::UpdateConstantBuffers()
{
	m_graphicsConstants.projectionMatrix = m_camera.GetProjMatrix();
	m_graphicsConstants.modelViewMatrix = m_camera.GetViewMatrix();
	m_graphicsConstants.invViewMatrix = Invert(m_camera.GetViewMatrix());
	m_graphicsConstants.screenDim[0] = (float)m_displayWidth;
	m_graphicsConstants.screenDim[1] = (float)m_displayHeight;

	m_graphicsConstantBuffer.Update(sizeof(GraphicsConstants), &m_graphicsConstants);

	m_computeConstants.deltaT = m_frameTimer * 0.05f;
	m_computeConstants.destX = sinf(XMConvertToRadians(m_timer * 360.0f)) * 0.75f;
	m_computeConstants.destY = 0.0f;
	m_computeConstants.particleCount = 6 * PARTICLES_PER_ATTRACTOR;

	m_computeConstantBuffer.Update(sizeof(ComputeConstants), &m_computeConstants);
}
