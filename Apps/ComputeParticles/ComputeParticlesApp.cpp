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

	LoadAssets();
}


void ComputeParticlesApp::Shutdown()
{}


bool ComputeParticlesApp::Update()
{
	return true;
}


void ComputeParticlesApp::Render()
{
	auto& context = GraphicsContext::Begin("Render frame");

	// Simulate particles on compute
	{

	}

	// Render particles
	context.TransitionResource(GetColorBuffer(), ResourceState::RenderTarget);
	context.TransitionResource(GetDepthBuffer(), ResourceState::DepthWrite);
	context.ClearColor(GetColorBuffer());
	context.ClearDepth(GetDepthBuffer());

	context.BeginRenderPass(GetBackBuffer());

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

#if 0
	m_graphicsRootSig.Reset(2);
	m_graphicsRootSig[0].InitAsDescriptorTable(2, ShaderVisibility::Vertex);
	m_graphicsRootSig[0].SetTableRange(0, DescriptorType::BufferSRV, 0, 1);
	m_graphicsRootSig[0].SetTableRange(1, DescriptorType::CBV, 0, 1);
	m_graphicsRootSig[1].InitAsDescriptorRange(DescriptorType::TextureSRV, 0, 2, ShaderVisibility::Pixel);
	m_graphicsRootSig.Finalize("Graphics Root Sig");
#endif
}


void ComputeParticlesApp::InitPSOs()
{
	m_computePSO.SetRootSignature(m_computeRootSig);
	m_computePSO.SetComputeShader("ParticlesCS");
	m_computePSO.Finalize();
}


void ComputeParticlesApp::InitConstantBuffers()
{}


void ComputeParticlesApp::LoadAssets()
{
	m_colorTexture = Texture::Load("particle01_rgba.ktx");
	m_gradientTexture = Texture::Load("particle_gradient_rgba.ktx");
}