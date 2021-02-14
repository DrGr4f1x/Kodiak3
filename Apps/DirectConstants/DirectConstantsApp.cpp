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

#include "DirectConstantsApp.h"

#include "Graphics\CommandContext.h"
#include "Graphics\CommonStates.h"


using namespace Kodiak;
using namespace std;


void DirectConstantsApp::Configure()
{
	Application::Configure();

	m_timerSpeed = 0.125f;
}


void DirectConstantsApp::Startup()
{
	using namespace Math;

	m_camera.SetPerspectiveMatrix(
		XMConvertToRadians(60.0f),
		(float)m_displayHeight / (float)m_displayWidth,
		0.001f,
		256.0f);
	m_camera.SetPosition(Math::Vector3(18.65f, -14.4f, -18.65f));
	m_camera.SetZoomRotation(-30.0f, -32.5f, 45.0f, 0.0f);

	m_controller.SetSpeedScale(0.01f);
	m_controller.SetCameraMode(CameraMode::ArcBall);
	m_controller.SetOrbitTarget(Vector3(0.0f, 0.0f, 0.0f), Length(m_camera.GetPosition()), 4.0f);

	InitRootSig();
	InitPSO();
	InitConstantBuffer();

	LoadAssets();

	InitResourceSet();
}


void DirectConstantsApp::Shutdown()
{
	m_model.reset();
	m_rootSig.Destroy();
}


bool DirectConstantsApp::Update()
{
	m_controller.Update(m_frameTimer);

	UpdateConstantBuffer();

	constexpr float r = 7.5f;
	const float sin_t = sinf(DirectX::XMConvertToRadians(m_timer * 360.0f));
	const float cos_t = cosf(DirectX::XMConvertToRadians(m_timer * 360.0f));
	constexpr float y = 4.0f;

	m_lightConstants.lightPositions[0] = Math::Vector4(r * 1.1f * sin_t, y, r * 1.1f * cos_t, 1.0f);
	m_lightConstants.lightPositions[1] = Math::Vector4(-r * sin_t, y, -r * cos_t, 1.0f);
	m_lightConstants.lightPositions[2] = Math::Vector4(r * 0.85f * sin_t, y, -sin_t * 2.5f, 1.5f);
	m_lightConstants.lightPositions[3] = Math::Vector4(0.0f, y, r * 1.25f * cos_t, 1.5f);
	m_lightConstants.lightPositions[4] = Math::Vector4(r * 2.25f * cos_t, y, 0.0f, 1.25f);
	m_lightConstants.lightPositions[5] = Math::Vector4(r * 2.5f * cos_t, y, r * 2.5f * sin_t, 1.25f);

	return true;
}


void DirectConstantsApp::Render()
{
	auto& context = GraphicsContext::Begin("Scene");

	context.TransitionResource(GetColorBuffer(), ResourceState::RenderTarget);
	context.TransitionResource(GetDepthBuffer(), ResourceState::DepthWrite);
	context.ClearColor(GetColorBuffer());
	context.ClearDepth(GetDepthBuffer());

	context.BeginRenderPass(GetBackBuffer());

	context.SetViewportAndScissor(0u, 0u, m_displayWidth, m_displayHeight);

	context.SetRootSignature(m_rootSig);
	context.SetPipelineState(m_PSO);

	context.SetResources(m_resources);
	context.SetConstantArray(1, 24, &m_lightConstants);

	m_model->Render(context);

	RenderUI(context);

	context.EndRenderPass();
	context.TransitionResource(GetColorBuffer(), ResourceState::Present);

	context.Finish();
}


void DirectConstantsApp::InitRootSig()
{
	m_rootSig.Reset(2);
	m_rootSig[0].InitAsDescriptorRange(DescriptorType::CBV, 0, 1, ShaderVisibility::Vertex);
	m_rootSig[1].InitAsConstants(1, 24, ShaderVisibility::Vertex);
	m_rootSig.Finalize("Root Sig", RootSignatureFlags::AllowInputAssemblerInputLayout);
}


void DirectConstantsApp::InitPSO()
{
	m_PSO.SetRootSignature(m_rootSig);
	m_PSO.SetRenderTargetFormat(GetColorFormat(), GetDepthFormat());

	m_PSO.SetBlendState(CommonStates::BlendDisable());
	m_PSO.SetRasterizerState(CommonStates::RasterizerDefault());
	m_PSO.SetDepthStencilState(CommonStates::DepthStateReadWriteReversed());

	m_PSO.SetPrimitiveTopology(PrimitiveTopology::TriangleList);

	m_PSO.SetVertexShader("LightsVS");
	m_PSO.SetPixelShader("LightsPS");

	VertexStreamDesc vertexStream{ 0, sizeof(Vertex), InputClassification::PerVertexData };
	vector<VertexElementDesc> vertexElements =
	{
		{ "POSITION", 0, Format::R32G32B32_Float, 0, offsetof(Vertex, position), InputClassification::PerVertexData, 0 },
		{ "NORMAL", 0, Format::R32G32B32_Float, 0, offsetof(Vertex, normal), InputClassification::PerVertexData, 0 },
		{ "COLOR", 0, Format::R32G32B32_Float, 0, offsetof(Vertex, color), InputClassification::PerVertexData, 0 },

	};
	m_PSO.SetInputLayout(vertexStream, vertexElements);
	m_PSO.Finalize();
}


void DirectConstantsApp::InitConstantBuffer()
{
	m_constantBuffer.Create("Constant Buffer", 1, sizeof(Constants));

	UpdateConstantBuffer();
}


void DirectConstantsApp::InitResourceSet()
{
	m_resources.Init(&m_rootSig);
	m_resources.SetCBV(0, 0, m_constantBuffer);
	m_resources.Finalize();
}


void DirectConstantsApp::UpdateConstantBuffer()
{
	m_constants.projectionMatrix = m_camera.GetProjMatrix();
	m_constants.modelMatrix = m_camera.GetViewMatrix();

	m_constantBuffer.Update(sizeof(m_constants), &m_constants);
}


void DirectConstantsApp::LoadAssets()
{
	auto layout = VertexLayout({
		VertexComponent::Position,
		VertexComponent::Normal,
		VertexComponent::Color
		});
	m_model = Model::Load("samplescene.dae", layout, 0.35f);
}