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


using namespace Kodiak;
using namespace std;


void TriangleApp::Startup()
{
	// Setup vertices
	vector<Vertex> vertexData =
	{
		{ { -1.0f, -1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
		{ {  1.0f, -1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
		{ {  0.0f,  1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } }
	};
	m_vertexBuffer.Create("Vertex buffer", vertexData.size(), sizeof(Vertex), vertexData.data());

	// Setup indices
	vector<uint32_t> indexData = { 0, 1, 2 };
	m_indexBuffer.Create("Index buffer", indexData.size(), sizeof(uint32_t), indexData.data());

	// Setup constant buffer
	m_constantBuffer.Create("Constant buffer", 1, sizeof(m_vsConstants));
	m_vsConstants.modelMatrix = Math::Matrix4(Math::kIdentity);

	// Setup camera
	m_camera.SetPerspectiveMatrix(
		DirectX::XMConvertToRadians(60.0f),
		(float)m_displayHeight / (float)m_displayWidth,
		0.1f,
		256.0f);
	m_camera.SetPosition(Math::Vector3(0.0f, 0.0f, -m_zoom));
	m_camera.Update();

	m_controller.SetSpeedScale(0.025f);
	m_controller.RefreshFromCamera();

	UpdateConstantBuffer();

	InitRootSig();
	InitPSO();
	InitResourceSet();
}


void TriangleApp::Shutdown()
{
	m_rootSig.Destroy();
}


bool TriangleApp::Update()
{
	m_controller.Update(m_frameTimer);

	UpdateConstantBuffer();

	return true;
}


void TriangleApp::Render()
{
	auto& context = GraphicsContext::Begin("Render frame");

	context.TransitionResource(GetColorBuffer(), ResourceState::RenderTarget);
	context.TransitionResource(GetDepthBuffer(), ResourceState::DepthWrite);
	Color clearColor{ DirectX::Colors::CornflowerBlue };
	context.ClearColor(GetColorBuffer(), clearColor);
	context.ClearDepth(GetDepthBuffer());

	context.BeginRenderPass(GetBackBuffer());
	
	context.SetViewportAndScissor(0u, 0u, m_displayWidth, m_displayHeight);

	context.SetRootSignature(m_rootSig);
	context.SetPipelineState(m_pso);

	context.SetResources(m_resources);

	context.SetVertexBuffer(0, m_vertexBuffer);
	context.SetIndexBuffer(m_indexBuffer);

	context.DrawIndexed((uint32_t)m_indexBuffer.GetElementCount());

	context.EndRenderPass();

	context.TransitionResource(GetColorBuffer(), ResourceState::Present);

	context.Finish();
}


void TriangleApp::UpdateConstantBuffer()
{
	// Update matrices
	m_vsConstants.viewProjectionMatrix = m_camera.GetViewProjMatrix();
	
	m_constantBuffer.Update(sizeof(m_vsConstants), &m_vsConstants);
}


void TriangleApp::InitRootSig()
{
	m_rootSig.Reset(1);
	m_rootSig[0].InitAsDescriptorRange(DescriptorType::CBV, 0, 1, ShaderVisibility::Vertex);
	m_rootSig.Finalize("Root sig", RootSignatureFlags::AllowInputAssemblerInputLayout);
}


void TriangleApp::InitPSO()
{
	m_pso.SetRootSignature(m_rootSig);

	// Render state
	m_pso.SetRasterizerState(CommonStates::RasterizerTwoSided());
	m_pso.SetBlendState(CommonStates::BlendDisable());
	m_pso.SetDepthStencilState(CommonStates::DepthStateReadOnlyReversed());

	m_pso.SetVertexShader("TriangleVS");
	m_pso.SetPixelShader("TrianglePS");

	m_pso.SetRenderTargetFormat(GetColorFormat(), GetDepthFormat());

	m_pso.SetPrimitiveTopology(PrimitiveTopology::TriangleList);

	// Vertex inputs
	VertexStreamDesc vertexStream = { 0, sizeof(Vertex), InputClassification::PerVertexData };
	vector<VertexElementDesc> vertexElements =
	{
		{ "POSITION", 0, Format::R32G32B32_Float, 0, offsetof(Vertex, position), InputClassification::PerVertexData, 0 },
		{ "COLOR", 0, Format::R32G32B32_Float, 0, offsetof(Vertex, color), InputClassification::PerVertexData, 0 }
	};
	m_pso.SetInputLayout(vertexStream, vertexElements);

	m_pso.Finalize();
}


void TriangleApp::InitResourceSet()
{
	m_resources.Init(&m_rootSig);
	m_resources.SetCBV(0, 0, m_constantBuffer);
	m_resources.Finalize();
}