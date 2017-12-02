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

#include "TriangleApp.h"

#include "Color.h"
#include "CommandContext.h"
#include "CommonStates.h"
#include "Filesystem.h"
#include "GraphicsDevice.h"
#include "Shader.h"
#include "SwapChain.h"


using namespace Kodiak;
using namespace std;


void TriangleApp::Configure()
{
	// Setup file system
	auto& filesystem = Kodiak::Filesystem::GetInstance();

	filesystem.SetDefaultRootDir();
	filesystem.AddSearchPath("Data\\" + GetDefaultShaderPath());
}


void TriangleApp::Startup()
{
	// Setup vertices
	vector<Vertex> vertexData =
	{
		{ {  1.0f,  1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
		{ { -1.0f,  1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
		{ {  0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } }
	};
	m_vertexBuffer.Create("Vertex buffer", vertexData.size(), sizeof(Vertex), vertexData.data());

	// Setup indices
	vector<uint32_t> indexData = { 0, 1, 2 };
	m_indexBuffer.Create("Index buffer", indexData.size(), sizeof(uint32_t), indexData.data());

	// Setup constant buffer
	m_constantBuffer.Create("Constant buffer", 1, sizeof(m_vsConstants));

	UpdateConstantBuffer();

	InitRootSig();
	InitPSO();
}


void TriangleApp::Shutdown()
{
	m_vertexBuffer.Destroy();
	m_indexBuffer.Destroy();
	m_constantBuffer.Destroy();
	m_rootSig.Destroy();
}


void TriangleApp::Render()
{
	auto& context = GraphicsContext::Begin("Render frame");

	uint32_t curFrame = m_graphicsDevice->GetCurrentBuffer();
	
	Color clearColor{ DirectX::Colors::CornflowerBlue };
	context.BeginRenderPass(m_renderPass, *m_framebuffers[curFrame], clearColor, 1.0f, 0);

	context.SetViewportAndScissor(0u, 0u, m_displayWidth, m_displayHeight);

	context.SetRootSignature(m_rootSig);
	context.SetPipelineState(m_pso);

	context.SetRootConstantBuffer(0, m_constantBuffer);

	context.SetVertexBuffer(0, m_vertexBuffer);
	context.SetIndexBuffer(m_indexBuffer);

	context.DrawIndexed((uint32_t)m_indexBuffer.GetElementCount());

	context.EndRenderPass();

	context.Finish();
}


void TriangleApp::UpdateConstantBuffer()
{
	// Update matrices
	m_vsConstants.projectionMatrix = Math::Matrix4::MakePerspective(
		DirectX::XMConvertToRadians(60.0f),
		(float)m_displayWidth / (float)m_displayHeight,
		0.1f,
		256.0f);

	m_vsConstants.viewMatrix = Math::AffineTransform::MakeTranslation(Math::Vector3(0.0f, 0.0f, m_zoom));

	m_vsConstants.modelMatrix = Math::Matrix4(Math::kIdentity);

	m_constantBuffer.Update(sizeof(m_vsConstants), &m_vsConstants);
}


void TriangleApp::InitRootSig()
{
	m_rootSig.Reset(1);
	m_rootSig[0].InitAsConstantBuffer(0, ShaderVisibility::Vertex);
	m_rootSig.Finalize("Root sig", RootSignatureFlags::AllowInputAssemblerInputLayout);
}


void TriangleApp::InitPSO()
{
	m_pso.SetRootSignature(m_rootSig);

	// Render state
	m_pso.SetRasterizerState(CommonStates::RasterizerTwoSided());
	m_pso.SetBlendState(CommonStates::BlendDisable());
	m_pso.SetDepthStencilState(CommonStates::DepthStateReadOnlyReversed());

	auto vs = Shader::Load("TriangleVS");
	auto ps = Shader::Load("TrianglePS");

	m_pso.SetVertexShader(vs);
	m_pso.SetPixelShader(ps);

	m_pso.SetRenderPass(m_renderPass);

	m_pso.SetPrimitiveTopology(PrimitiveTopology::TriangleList);

	// Vertex inputs
	VertexStreamDesc vertexStreamDesc{ 0, sizeof(Vertex), InputClassification::PerVertexData };
	VertexElementDesc vertexElements[] =
	{
		{ "Position", 0, Format::R32G32B32_Float, 0, offsetof(Vertex, position), InputClassification::PerVertexData, 0 },
		{ "Color", 0, Format::R32G32B32_Float, 0, offsetof(Vertex, color), InputClassification::PerVertexData, 0 }
	};
	m_pso.SetInputLayout(1, &vertexStreamDesc, _countof(vertexElements), vertexElements);

	m_pso.Finalize();
}