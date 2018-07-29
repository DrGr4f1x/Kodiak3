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
#include "Filesystem.h"
#include "GraphicsDevice.h"
#include "SwapChain.h"


using namespace Kodiak;
using namespace std;


void MultisamplingApp::Configure()
{
	// Setup file system
	auto& filesystem = Kodiak::Filesystem::GetInstance();

	filesystem.SetDefaultRootDir();
	filesystem.AddSearchPath("Data\\" + GetDefaultShaderPath());
	filesystem.AddSearchPath("Data\\Models");
	filesystem.AddSearchPath("Data\\Textures");
}


void MultisamplingApp::Startup()
{
	using namespace Math;

	m_camera.SetPerspectiveMatrix(
		DirectX::XMConvertToRadians(60.0f),
		(float)m_displayHeight / (float)m_displayWidth,
		0.1f,
		256.0f);
	m_camera.SetPosition(Vector3(-4.838f, 3.23f, -7.05f));
	m_camera.Update();

	m_controller.SetSpeedScale(0.01f);
	m_controller.SetCameraMode(CameraMode::ArcBall);
	m_controller.SetOrbitTarget(Math::Vector3(0.0f, 3.5f, 0.0f), Math::Length(m_camera.GetPosition()), 0.25f);

	InitRenderTargets();
	InitRootSig();
	InitPSO();
	InitConstantBuffer();

	LoadAssets();
}


void MultisamplingApp::Shutdown()
{
	m_rootSig.Destroy();

	m_texture.reset();
	m_model.reset();
}


bool MultisamplingApp::Update()
{
	m_controller.Update(m_frameTimer);

	UpdateConstantBuffer();

	return true;
}


void MultisamplingApp::Render()
{
	auto& context = GraphicsContext::Begin("Render frame");

	uint32_t curFrame = m_graphicsDevice->GetCurrentBuffer();

	Color clearColor{ DirectX::Colors::White };
	context.BeginRenderPass(m_renderPass, *m_frameBuffer[curFrame], clearColor, 1.0f, 0);

	context.SetViewportAndScissor(0u, 0u, m_displayWidth, m_displayHeight);

	context.SetRootSignature(m_rootSig);
	context.SetPipelineState(m_pso);

	context.SetRootConstantBuffer(0, m_constantBuffer);

	context.SetIndexBuffer(m_model->GetIndexBuffer());
	context.SetVertexBuffer(0, m_model->GetVertexBuffer());

	context.SetRootConstantBuffer(0, m_constantBuffer);
	context.SetSRV(1, 0, *m_texture);

	context.DrawIndexed((uint32_t)m_model->GetIndexBuffer().GetElementCount());

	context.EndRenderPass();

	context.Finish();
}


void MultisamplingApp::InitRenderTargets()
{
	auto swapChain = m_graphicsDevice->GetSwapChain();
	auto colorFormat = swapChain->GetColorFormat();
	auto depthFormat = m_graphicsDevice->GetDepthFormat();

	m_renderPass.SetColorAttachment(0, colorFormat, ResourceState::RenderTarget, ResourceState::CopySource);
	m_renderPass.SetResolveAttachment(0, colorFormat, ResourceState::CopyDest, ResourceState::Present);
	m_renderPass.SetDepthAttachment(depthFormat, ResourceState::Undefined, ResourceState::DepthWrite);
	m_renderPass.Finalize();

	m_colorTarget = make_shared<ColorBuffer>();
	m_colorTarget->SetMsaaMode(m_numSamples, m_numSamples);
	m_colorTarget->Create("Color target", m_displayWidth, m_displayHeight, 1, colorFormat);

	m_depthTarget = make_shared<DepthBuffer>();
	m_depthTarget->Create("Depth target", m_displayWidth, m_displayHeight, m_numSamples, depthFormat);

	for (int i = 0; i < 3; ++i)
	{
		m_frameBuffer[i] = make_shared<FrameBuffer>();
		m_frameBuffer[i]->SetColorBuffer(0, m_colorTarget);
		m_frameBuffer[i]->SetResolveBuffer(0, m_defaultFramebuffers[i]->GetColorBuffer(0));
		m_frameBuffer[i]->SetDepthBuffer(m_depthTarget);
		m_frameBuffer[i]->Finalize(m_renderPass);
	}
}


void MultisamplingApp::InitRootSig()
{
	m_rootSig.Reset(2, 1);
	m_rootSig[0].InitAsConstantBuffer(0, ShaderVisibility::Vertex);
	m_rootSig[1].InitAsDescriptorTable(1, ShaderVisibility::Pixel);
	m_rootSig[1].SetTableRange(0, DescriptorType::TextureSRV, 0, 1);
	m_rootSig.InitStaticSampler(0, CommonStates::SamplerLinearClamp(), ShaderVisibility::Pixel);
	m_rootSig.Finalize("Root Sig", RootSignatureFlags::AllowInputAssemblerInputLayout);
}


void MultisamplingApp::InitPSO()
{
	m_pso.SetRootSignature(m_rootSig);

	// Render state
	m_pso.SetRasterizerState(CommonStates::RasterizerDefaultCW());
	m_pso.SetBlendState(CommonStates::BlendDisable());
	m_pso.SetDepthStencilState(CommonStates::DepthStateReadWriteReversed());

	m_pso.SetVertexShader("MeshVS");
	m_pso.SetPixelShader("MeshPS");

	m_pso.SetRenderPass(m_defaultRenderPass);

	m_pso.SetPrimitiveTopology(PrimitiveTopology::TriangleList);

	// Vertex inputs
	VertexStreamDesc vertexStreamDesc{ 0, sizeof(Vertex), InputClassification::PerVertexData };
	VertexElementDesc vertexElements[] =
	{
		{ "POSITION", 0, Format::R32G32B32_Float, 0, offsetof(Vertex, position), InputClassification::PerVertexData, 0 },
		{ "NORMAL", 0, Format::R32G32B32_Float, 0, offsetof(Vertex, normal), InputClassification::PerVertexData, 0 },
		{ "TEXCOORD", 0, Format::R32G32_Float, 0, offsetof(Vertex, uv), InputClassification::PerVertexData, 0 },
		{ "COLOR", 0, Format::R32G32B32_Float, 0, offsetof(Vertex, color), InputClassification::PerVertexData, 0}
	};
	m_pso.SetInputLayout(1, &vertexStreamDesc, _countof(vertexElements), vertexElements);

	m_pso.Finalize();
}


void MultisamplingApp::InitConstantBuffer()
{
	m_constantBuffer.Create("Constant Buffer", 1, sizeof(Constants));

	UpdateConstantBuffer();
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
	using namespace Math;

	m_constants.viewProjectionMatrix = m_camera.GetProjMatrix();
	m_constants.modelMatrix = m_camera.GetViewMatrix();
	m_constants.lightPos = Vector4(5.0f, 5.0f, 5.0f, 1.0f);

	m_constantBuffer.Update(sizeof(Constants), &m_constants);
}