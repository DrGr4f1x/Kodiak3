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

#include "MultisamplingApp.h"

#include "CommandContext.h"
#include "CommonStates.h"
#include "GraphicsDevice.h"


using namespace Kodiak;
using namespace Math;
using namespace std;


void MultisamplingApp::Configure()
{
	Application::Configure();

	OptionalFeatures().sampleRateShading = true;
}


void MultisamplingApp::Startup()
{
	m_camera.SetPerspectiveMatrix(
		DirectX::XMConvertToRadians(60.0f),
		(float)m_displayHeight / (float)m_displayWidth,
		0.1f,
		256.0f);
	m_camera.SetPosition(Vector3(-4.838f, 3.23f, -7.05f));
	m_camera.Update();

	m_controller.SetSpeedScale(0.01f);
	m_controller.SetCameraMode(CameraMode::ArcBall);
	m_controller.SetOrbitTarget(Vector3(0.0f, 3.5f, 0.0f), Length(m_camera.GetPosition()), 0.25f);

	InitRenderTargets();
	InitRootSig();
	InitPSOs();
	InitConstantBuffer();

	LoadAssets();

	InitResources();
}


void MultisamplingApp::Shutdown()
{
	m_rootSig.Destroy();

	m_texture.reset();
	m_model.reset();
}


bool MultisamplingApp::Update()
{
	m_controller.Update(m_frameTimer, m_mouseMoveHandled);

	UpdateConstantBuffer();

	return true;
}


void MultisamplingApp::UpdateUI()
{
	if (EnabledFeatures().sampleRateShading) 
	{
		if (m_uiOverlay->Header("Settings")) 
		{
			m_uiOverlay->CheckBox("Sample rate shading", &m_sampleRateShading);
		}
	}
}


void MultisamplingApp::Render()
{
	auto& context = GraphicsContext::Begin("Scene");

	Color clearColor{ DirectX::Colors::White };
	context.TransitionResource(*m_frameBuffer->GetColorBuffer(0), ResourceState::RenderTarget);
	context.TransitionResource(*m_frameBuffer->GetDepthBuffer(), ResourceState::DepthWrite);
	context.ClearColor(*m_frameBuffer->GetColorBuffer(0), clearColor);
	context.ClearDepth(*m_frameBuffer->GetDepthBuffer());

	context.BeginRenderPass(*m_frameBuffer);

	context.SetViewportAndScissor(0u, 0u, m_displayWidth, m_displayHeight);

	context.SetRootSignature(m_rootSig);
	context.SetPipelineState(m_sampleRateShading ? m_psoMsaaSampleRate : m_psoMsaa);

	context.SetResources(m_resources);

	context.SetIndexBuffer(m_model->GetIndexBuffer());
	context.SetVertexBuffer(0, m_model->GetVertexBuffer());

	context.DrawIndexed((uint32_t)m_model->GetIndexBuffer().GetElementCount());

	context.EndRenderPass();

	context.TransitionResource(*m_frameBuffer->GetColorBuffer(0), ResourceState::ResolveSource);
	context.TransitionResource(GetColorBuffer(), ResourceState::ResolveDest);

	context.Resolve(*m_frameBuffer->GetColorBuffer(0), GetColorBuffer(), GetColorFormat());

	// Render UI after MSAA resolve
	context.TransitionResource(GetColorBuffer(), ResourceState::RenderTarget);
	context.BeginRenderPass(GetBackBuffer());
	RenderUI(context);
	context.EndRenderPass();

	context.TransitionResource(GetColorBuffer(), ResourceState::Present);

	context.Finish();
}


void MultisamplingApp::InitRenderTargets()
{
	m_colorTarget = make_shared<ColorBuffer>(DirectX::Colors::White);
	m_colorTarget->SetMsaaMode(m_numSamples, m_numSamples);
	m_colorTarget->Create("Color target", m_displayWidth, m_displayHeight, 1, GetColorFormat());

	m_depthTarget = make_shared<DepthBuffer>(1.0f, 0);
	m_depthTarget->Create("Depth target", m_displayWidth, m_displayHeight, m_numSamples, GetDepthFormat());

	m_frameBuffer = make_shared<FrameBuffer>();
	m_frameBuffer->SetColorBuffer(0, m_colorTarget);
	m_frameBuffer->SetDepthBuffer(m_depthTarget);
	m_frameBuffer->Finalize();
}


void MultisamplingApp::InitRootSig()
{
	m_rootSig.Reset(2, 1);
	m_rootSig[0].InitAsDescriptorRange(DescriptorType::CBV, 0, 1, ShaderVisibility::Vertex);
	m_rootSig[1].InitAsDescriptorTable(1, ShaderVisibility::Pixel);
	m_rootSig[1].SetTableRange(0, DescriptorType::TextureSRV, 0, 1);
	m_rootSig.InitStaticSampler(0, CommonStates::SamplerLinearClamp(), ShaderVisibility::Pixel);
	m_rootSig.Finalize("Root Sig", RootSignatureFlags::AllowInputAssemblerInputLayout);
}


void MultisamplingApp::InitPSOs()
{
	m_psoMsaa.SetRootSignature(m_rootSig);

	// Render state
	m_psoMsaa.SetRasterizerState(CommonStates::RasterizerDefaultCW());
	m_psoMsaa.SetBlendState(CommonStates::BlendDisable());
	m_psoMsaa.SetDepthStencilState(CommonStates::DepthStateReadWriteReversed());

	m_psoMsaa.SetVertexShader("MeshVS");
	m_psoMsaa.SetPixelShader("MeshPS");

	m_psoMsaa.SetRenderTargetFormat(GetColorFormat(), GetDepthFormat(), m_numSamples, false);

	m_psoMsaa.SetPrimitiveTopology(PrimitiveTopology::TriangleList);

	// Vertex inputs
	VertexStreamDesc vertexStream{ 0, sizeof(Vertex), InputClassification::PerVertexData };
	vector<VertexElementDesc> vertexElements =
	{
		{ "POSITION", 0, Format::R32G32B32_Float, 0, offsetof(Vertex, position), InputClassification::PerVertexData, 0 },
		{ "NORMAL", 0, Format::R32G32B32_Float, 0, offsetof(Vertex, normal), InputClassification::PerVertexData, 0 },
		{ "TEXCOORD", 0, Format::R32G32_Float, 0, offsetof(Vertex, uv), InputClassification::PerVertexData, 0 },
		{ "COLOR", 0, Format::R32G32B32_Float, 0, offsetof(Vertex, color), InputClassification::PerVertexData, 0}
	};
	m_psoMsaa.SetInputLayout(vertexStream, vertexElements);

	m_psoMsaaSampleRate = m_psoMsaa;
	if (EnabledFeatures().sampleRateShading)
	{
		m_psoMsaaSampleRate.SetRenderTargetFormat(GetColorFormat(), GetDepthFormat(), m_numSamples, true);
	}

	m_psoMsaa.Finalize();
	m_psoMsaaSampleRate.Finalize();
}


void MultisamplingApp::InitConstantBuffer()
{
	m_constantBuffer.Create("Constant Buffer", 1, sizeof(Constants));

	UpdateConstantBuffer();
}


void MultisamplingApp::InitResources()
{
	m_resources.Init(&m_rootSig);
	m_resources.SetCBV(0, 0, m_constantBuffer);
	m_resources.SetSRV(1, 0,* m_texture);
	m_resources.Finalize();
}


void MultisamplingApp::LoadAssets()
{
	auto layout = VertexLayout({
		VertexComponent::Position,
		VertexComponent::Normal,
		VertexComponent::UV,
		VertexComponent::Color
		});
	m_model = Model::Load("voyager.dae", layout, 1.0f);

	m_texture = Texture::Load("voyager_bc3_unorm.ktx");
}


void MultisamplingApp::UpdateConstantBuffer()
{
	m_constants.viewProjectionMatrix = m_camera.GetProjMatrix();
	m_constants.modelMatrix = m_camera.GetViewMatrix();
	m_constants.lightPos = Vector4(5.0f, 5.0f, 5.0f, 1.0f);

	m_constantBuffer.Update(sizeof(Constants), &m_constants);
}