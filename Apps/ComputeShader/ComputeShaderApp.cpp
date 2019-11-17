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

#include "ComputeShaderApp.h"

#include "CommonStates.h"
#include "CommandContext.h"
#include "Input.h"


using namespace Kodiak;
using namespace std;


void ComputeShaderApp::Startup()
{
	// Setup vertices for a single uv-mapped quad made from two triangles
	vector<Vertex> vertexData =
	{
		{ {  1.0f,  1.0f,  0.0f }, { 1.0f, 0.0f } },
		{ { -1.0f,  1.0f,  0.0f }, { 0.0f, 0.0f } },
		{ { -1.0f, -1.0f,  0.0f }, { 0.0f, 1.0f } },
		{ {  1.0f, -1.0f,  0.0f }, { 1.0f, 1.0f } }
	};
	m_vertexBuffer.Create("Vertex buffer", vertexData.size(), sizeof(Vertex), vertexData.data());

	vector<uint32_t> indexData = { 0,1,2, 2,3,0 };
	m_indexBuffer.Create("Index buffer", indexData.size(), sizeof(uint32_t), indexData.data());

	m_camera.SetPerspectiveMatrix(DirectX::XMConvertToRadians(60.0f),
		(float)m_displayHeight / (float)(m_displayWidth / 2),
		0.1f,
		256.0f);
	m_camera.SetPosition(Math::Vector3(0.0f, 0.0f, 2.0f));
	m_camera.Update();

	InitRootSigs();
	InitPSOs();
	InitConstantBuffer();

	LoadAssets();

	m_computeScratch.Create("Compute Scratch", m_texture->GetWidth(), m_texture->GetHeight(), 1, Format::R8G8B8A8_UNorm);

	InitResourceSets();
}


void ComputeShaderApp::Shutdown()
{
	m_rootSig.Destroy();
	m_computeRootSig.Destroy();

	m_texture.reset();
}


bool ComputeShaderApp::Update()
{
	if (g_input.IsFirstPressed(DigitalInput::kKey_add))
	{
		++m_curComputeTechnique;
	}
	else if (g_input.IsFirstPressed(DigitalInput::kKey_subtract))
	{
		--m_curComputeTechnique;
	}

	if (m_curComputeTechnique < 0)
	{
		m_curComputeTechnique = 2;
	}

	if (m_curComputeTechnique > 2)
	{
		m_curComputeTechnique = 0;
	}

	return true;
}


void ComputeShaderApp::Render()
{
	auto& context = GraphicsContext::Begin("Scene");

	// Compute
	{
		auto& computeContext = context.GetComputeContext();

		computeContext.TransitionResource(m_computeScratch, ResourceState::UnorderedAccess);
		computeContext.TransitionResource(*m_texture, ResourceState::NonPixelShaderResource);

		computeContext.SetRootSignature(m_computeRootSig);
		if (m_curComputeTechnique == 0)
		{
			computeContext.SetPipelineState(m_embossPSO);
		}
		else if (m_curComputeTechnique == 1)
		{
			computeContext.SetPipelineState(m_edgeDetectPSO);
		}
		else
		{
			computeContext.SetPipelineState(m_sharpenPSO);
		}

		computeContext.SetResources(m_computeResources);

		computeContext.Dispatch2D(m_computeScratch.GetWidth(), m_computeScratch.GetHeight(), 16, 16);

		computeContext.TransitionResource(m_computeScratch, ResourceState::PixelShaderResource);
	}

	// Graphics

	context.TransitionResource(*m_texture, ResourceState::PixelShaderResource, true);
	context.TransitionResource(GetColorBuffer(), ResourceState::RenderTarget);
	context.TransitionResource(GetDepthBuffer(), ResourceState::DepthWrite);
	context.ClearColor(GetColorBuffer());
	context.ClearDepth(GetDepthBuffer());

	context.BeginRenderPass(GetBackBuffer());

	context.SetViewportAndScissor(0u, 0u, m_displayWidth / 2, m_displayHeight);

	context.SetRootSignature(m_rootSig);
	context.SetPipelineState(m_pso);

	context.SetResources(m_gfxLeftResources);

	context.SetVertexBuffer(0, m_vertexBuffer);
	context.SetIndexBuffer(m_indexBuffer);

	context.DrawIndexed((uint32_t)m_indexBuffer.GetElementCount());

	context.SetViewportAndScissor(m_displayWidth / 2, 0u, m_displayWidth / 2, m_displayHeight);

	context.SetResources(m_gfxRightResources);

	context.DrawIndexed((uint32_t)m_indexBuffer.GetElementCount());

	context.EndRenderPass();

	context.Finish();
}


void ComputeShaderApp::InitRootSigs()
{
	m_rootSig.Reset(2, 1);
	m_rootSig[0].InitAsDescriptorRange(DescriptorType::CBV, 0, 1, ShaderVisibility::Vertex);
	m_rootSig[1].InitAsDescriptorRange(DescriptorType::TextureSRV, 0, 1, ShaderVisibility::Pixel);
	m_rootSig.InitStaticSampler(0, CommonStates::SamplerLinearClamp(), ShaderVisibility::Pixel);
	m_rootSig.Finalize("Root Sig", RootSignatureFlags::AllowInputAssemblerInputLayout);

	m_computeRootSig.Reset(1);
	m_computeRootSig[0].InitAsDescriptorTable(2, ShaderVisibility::Compute);
	m_computeRootSig[0].SetTableRange(0, DescriptorType::TextureSRV, 0, 1);
	m_computeRootSig[0].SetTableRange(1, DescriptorType::TextureUAV, 0, 1);
	m_computeRootSig.Finalize("Compute Root Sig");
}


void ComputeShaderApp::InitPSOs()
{
	m_pso.SetRootSignature(m_rootSig);

	// Render state
	m_pso.SetRasterizerState(CommonStates::RasterizerTwoSided());
	m_pso.SetBlendState(CommonStates::BlendDisable());
	m_pso.SetDepthStencilState(CommonStates::DepthStateReadOnlyReversed());

	m_pso.SetVertexShader("TextureVS");
	m_pso.SetPixelShader("TexturePS");

	m_pso.SetRenderTargetFormat(GetColorFormat(), GetDepthFormat());

	m_pso.SetPrimitiveTopology(PrimitiveTopology::TriangleList);

	// Vertex inputs
	VertexStreamDesc vertexStream{ 0, sizeof(Vertex), InputClassification::PerVertexData };
	vector<VertexElementDesc> vertexElements =
	{
		{ "POSITION", 0, Format::R32G32B32_Float, 0, offsetof(Vertex, position), InputClassification::PerVertexData, 0 },
		{ "TEXCOORD", 0, Format::R32G32_Float, 0, offsetof(Vertex, uv), InputClassification::PerVertexData, 0 }
	};
	m_pso.SetInputLayout(vertexStream, vertexElements);

	m_pso.Finalize();


	m_edgeDetectPSO.SetRootSignature(m_computeRootSig);
	m_edgeDetectPSO.SetComputeShader("EdgeDetectCS");
	m_edgeDetectPSO.Finalize();


	m_embossPSO.SetRootSignature(m_computeRootSig);
	m_embossPSO.SetComputeShader("EmbossCS");
	m_embossPSO.Finalize();


	m_sharpenPSO.SetRootSignature(m_computeRootSig);
	m_sharpenPSO.SetComputeShader("SharpenCS");
	m_sharpenPSO.Finalize();
}


void ComputeShaderApp::InitConstantBuffer()
{
	m_constantBuffer.Create("Constant Buffer", 1, sizeof(Constants));

	m_constants.viewProjectionMatrix = m_camera.GetViewProjMatrix();
	m_constants.modelMatrix = Math::Matrix4(Math::kIdentity);
	
	m_constantBuffer.Update(sizeof(Constants), &m_constants);
}


void ComputeShaderApp::InitResourceSets()
{
	m_computeResources.Init(&m_computeRootSig);
	m_computeResources.SetSRV(0, 0, *m_texture);
	m_computeResources.SetUAV(0, 1, m_computeScratch);
	m_computeResources.Finalize();


	m_gfxLeftResources.Init(&m_rootSig);
	m_gfxLeftResources.SetCBV(0, 0, m_constantBuffer);
	m_gfxLeftResources.SetSRV(1, 0, *m_texture);
	m_gfxLeftResources.Finalize();


	m_gfxRightResources.Init(&m_rootSig);
	m_gfxRightResources.SetCBV(0, 0, m_constantBuffer);
	m_gfxRightResources.SetSRV(1, 0, m_computeScratch);
	m_gfxRightResources.Finalize();
}


void ComputeShaderApp::LoadAssets()
{
	m_texture = Texture::Load("het_kanonschot_rgba8.ktx");
}