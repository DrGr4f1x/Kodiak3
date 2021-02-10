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

#include "SmokeSimApp.h"

#include "Graphics\CommandContext.h"
#include "Graphics\CommonStates.h"


using namespace Kodiak;
using namespace DirectX;
using namespace std;


void SmokeSimApp::Configure()
{
	Application::Configure();
}


void SmokeSimApp::Startup()
{
	using namespace Math;

	LoadAssets();
	Vector3 center = m_model->GetBoundingBox().GetCenter();
	
	m_camera.SetPerspectiveMatrix(
		XMConvertToRadians(60.0f),
		(float)m_displayHeight / (float)m_displayWidth,
		0.001f,
		256.0f);
	m_camera.SetPosition(Math::Vector3(-3.38f, 0.0f, -7.25f));
	m_camera.Focus(m_model->GetBoundingBox());

	m_controller.SetSpeedScale(0.01f);
	m_controller.SetCameraMode(CameraMode::ArcBall);
	m_controller.SetOrbitTarget(center, Length(m_camera.GetPosition()), 4.0f);

	InitRootSigs();
	InitPSOs();
	InitConstantBuffer();

	InitResourceSets();
}


void SmokeSimApp::Shutdown()
{
	m_meshRootSig.Destroy();
}


bool SmokeSimApp::Update()
{
	m_controller.Update(m_frameTimer, m_mouseMoveHandled);

	UpdateConstantBuffer();

	return true;
}


void SmokeSimApp::Render()
{
	auto& context = GraphicsContext::Begin("Frame");

	context.TransitionResource(GetColorBuffer(), ResourceState::RenderTarget);
	context.TransitionResource(GetDepthBuffer(), ResourceState::DepthWrite);
	context.ClearColor(GetColorBuffer());
	context.ClearDepth(GetDepthBuffer());

	context.BeginRenderPass(GetBackBuffer());

	context.SetViewportAndScissor(0u, 0u, m_displayWidth, m_displayHeight);

	context.SetRootSignature(m_meshRootSig);
	context.SetPipelineState(m_meshPSO);

	context.SetResources(m_meshResources);

	context.SetIndexBuffer(m_model->GetIndexBuffer());
	context.SetVertexBuffer(0, m_model->GetVertexBuffer());

	context.DrawIndexed((uint32_t)m_model->GetIndexBuffer().GetElementCount());

	context.EndRenderPass();

	context.TransitionResource(GetColorBuffer(), ResourceState::Present);

	context.Finish();
}


void SmokeSimApp::InitRootSigs()
{
	m_meshRootSig.Reset(1);
	m_meshRootSig[0].InitAsDescriptorRange(DescriptorType::CBV, 0, 1, ShaderVisibility::Vertex);
	m_meshRootSig.Finalize("Mesh Root Sig", RootSignatureFlags::AllowInputAssemblerInputLayout);
}

void SmokeSimApp::InitPSOs()
{
	m_meshPSO.SetRootSignature(m_meshRootSig);
	m_meshPSO.SetRenderTargetFormat(GetColorFormat(), GetDepthFormat());

	m_meshPSO.SetBlendState(CommonStates::BlendDisable());
	m_meshPSO.SetRasterizerState(CommonStates::RasterizerDefault());
	m_meshPSO.SetDepthStencilState(CommonStates::DepthStateReadWriteReversed());

	m_meshPSO.SetPrimitiveTopology(PrimitiveTopology::TriangleList);

	m_meshPSO.SetVertexShader("MeshVS");
	m_meshPSO.SetPixelShader("MeshPS");

	VertexStreamDesc vertexStream{ 0, sizeof(Vertex), InputClassification::PerVertexData };
	vector<VertexElementDesc> vertexElements =
	{
		{ "POSITION", 0, Format::R32G32B32_Float, 0, offsetof(Vertex, position), InputClassification::PerVertexData, 0 },
		{ "NORMAL", 0, Format::R32G32B32_Float, 0, offsetof(Vertex, normal), InputClassification::PerVertexData, 0 },
		{ "TEXCOORD", 0, Format::R32G32_Float, 0, offsetof(Vertex, uv), InputClassification::PerVertexData, 0 },

	};
	m_meshPSO.SetInputLayout(vertexStream, vertexElements);
	m_meshPSO.Finalize();
}


void SmokeSimApp::InitConstantBuffer()
{
	m_constantBuffer.Create("Constant Buffer", 1, sizeof(Constants));

	UpdateConstantBuffer();
}


void SmokeSimApp::InitResourceSets()
{
	m_meshResources.Init(&m_meshRootSig);
	m_meshResources.SetCBV(0, 0, m_constantBuffer);
	m_meshResources.Finalize();
}


void SmokeSimApp::UpdateConstantBuffer()
{
	m_constants.projectionMatrix = m_camera.GetProjMatrix();
	Math::Matrix4 viewMatrix = m_camera.GetViewMatrix();
	m_constants.modelMatrix = viewMatrix * m_modelMatrix;

	m_constantBuffer.Update(sizeof(m_constants), &m_constants);
}


void SmokeSimApp::LoadAssets()
{
	auto layout = VertexLayout({
		VertexComponent::Position,
		VertexComponent::Normal,
		VertexComponent::UV
		});
	m_model = Model::Load("Statue\\socha_100kuw.obj", layout, 0.25f);
}