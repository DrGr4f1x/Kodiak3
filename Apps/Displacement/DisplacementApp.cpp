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

#include "DisplacementApp.h"

#include "CommandContext.h"
#include "CommonStates.h"
#include "Filesystem.h"
#include "GraphicsDevice.h"
#include "Input.h"


using namespace Kodiak;
using namespace std;


void DisplacementApp::Configure()
{
	// Setup file system
	auto& filesystem = Kodiak::Filesystem::GetInstance();

	filesystem.SetDefaultRootDir();
	filesystem.AddSearchPath("Data\\" + GetDefaultShaderPath());
	filesystem.AddSearchPath("Data\\Models");
	filesystem.AddSearchPath("Data\\Textures");
}


void DisplacementApp::Startup()
{
	using namespace Math;

	m_camera.SetPerspectiveMatrix(
		DirectX::XMConvertToRadians(45.0f),
		(float)m_displayHeight / (float)m_displayWidth,
		0.1f,
		256.0f);
	m_camera.SetPosition(Vector3(0.830579f, -0.427525f, -0.830579f));

	m_controller.SetSpeedScale(0.0025f);
	m_controller.SetCameraMode(CameraMode::ArcBall);
	m_controller.SetOrbitTarget(Vector3(0.0f, 0.0f, 0.0f), Length(m_camera.GetPosition()), 0.25f);

	InitRootSig();
	InitPSOs();
	InitConstantBuffers();

	UpdateConstantBuffers();

	LoadAssets();

	InitResourceSet();
}


void DisplacementApp::Shutdown()
{
	m_rootSig.Destroy();

	m_texture.reset();
	m_model.reset();
}


bool DisplacementApp::Update()
{
	m_controller.Update(m_frameTimer);

	UpdateConstantBuffers();

	return true;
}


void DisplacementApp::Render()
{
	auto& context = GraphicsContext::Begin("Render frame");

	context.TransitionResource(GetColorBuffer(), ResourceState::RenderTarget);
	context.TransitionResource(GetDepthBuffer(), ResourceState::DepthWrite);
	context.ClearColor(GetColorBuffer());
	context.ClearDepth(GetDepthBuffer());

	context.BeginRenderPass(GetBackBuffer());

	context.SetViewport(0.0f, 0.0f, (float)m_displayWidth, (float)m_displayHeight);

	context.SetRootSignature(m_rootSig);
	
	context.SetResources(m_resources);

	context.SetIndexBuffer(m_model->GetIndexBuffer());
	context.SetVertexBuffer(0, m_model->GetVertexBuffer());

	if (m_split)
	{
		context.SetPipelineState(m_wireframePSO);
		context.SetScissor(0u, 0u, m_displayWidth / 2, m_displayHeight);
		context.DrawIndexed((uint32_t)m_model->GetIndexBuffer().GetElementCount());
	}

	context.SetPipelineState(m_pso);
	context.SetScissor(m_split ? m_displayWidth / 2 : 0u, 0u, m_displayWidth, m_displayHeight);
	context.DrawIndexed((uint32_t)m_model->GetIndexBuffer().GetElementCount());

	context.EndRenderPass();
	context.TransitionResource(GetColorBuffer(), ResourceState::Present);

	context.Finish();
}


void DisplacementApp::InitRootSig()
{
	m_rootSig.Reset(3, 1);
	m_rootSig[0].InitAsDescriptorRange(DescriptorType::CBV, 0, 1, ShaderVisibility::Hull);
	m_rootSig[1].InitAsDescriptorTable(2, ShaderVisibility::Domain);
	m_rootSig[1].SetTableRange(0, DescriptorType::CBV, 0, 1);
	m_rootSig[1].SetTableRange(1, DescriptorType::TextureSRV, 0, 1);
	m_rootSig[2].InitAsDescriptorRange(DescriptorType::TextureSRV, 0, 1, ShaderVisibility::Pixel);
	m_rootSig.InitStaticSampler(0, CommonStates::SamplerLinearWrap());
	m_rootSig.Finalize("Root Sig", RootSignatureFlags::AllowInputAssemblerInputLayout);
}


void DisplacementApp::InitPSOs()
{
	m_pso.SetRootSignature(m_rootSig);

	// Render state
	m_pso.SetRasterizerState(CommonStates::RasterizerDefault());
	m_pso.SetBlendState(CommonStates::BlendDisable());
	m_pso.SetDepthStencilState(CommonStates::DepthStateReadWriteReversed());

	m_pso.SetVertexShader("BaseVS");
	m_pso.SetPixelShader("BasePS");
	m_pso.SetHullShader("DisplacementHS");
	m_pso.SetDomainShader("DisplacementDS");

	m_pso.SetRenderTargetFormat(GetColorFormat(), GetDepthFormat());

	m_pso.SetPrimitiveTopology(PrimitiveTopology::Patch_3_ControlPoint);

	// Vertex inputs
	VertexStreamDesc vertexStream{ 0, sizeof(Vertex), InputClassification::PerVertexData };
	vector<VertexElementDesc> vertexElements =
	{
		{ "POSITION", 0, Format::R32G32B32_Float, 0, offsetof(Vertex, position), InputClassification::PerVertexData, 0 },
		{ "NORMAL", 0, Format::R32G32B32_Float, 0, offsetof(Vertex, normal), InputClassification::PerVertexData, 0 },
		{ "TEXCOORD", 0, Format::R32G32_Float, 0, offsetof(Vertex, uv), InputClassification::PerVertexData, 0 },
	};
	m_pso.SetInputLayout(vertexStream, vertexElements);

	m_wireframePSO = m_pso;
	m_wireframePSO.SetRasterizerState(CommonStates::RasterizerWireframe());

	m_pso.Finalize();
	m_wireframePSO.Finalize();
}


void DisplacementApp::InitConstantBuffers()
{
	m_hsConstants.tessLevel = 64.0f;
	// TODO Fix this (bad resource transitions)
	//m_hsConstantBuffer.Create("HS Constant Buffer", 1, sizeof(HSConstants), &m_hsConstants);
	m_hsConstantBuffer.Create("HS Constant Buffer", 1, sizeof(HSConstants));

	m_dsConstants.lightPos = Math::Vector4(0.0f, 5.0f, 0.0f, 0.0f);
	m_dsConstants.tessAlpha = 1.0f;
	m_dsConstants.tessStrength = 0.1f;
	m_dsConstantBuffer.Create("DS Constant Buffer", 1, sizeof(DSConstants));
}


void DisplacementApp::InitResourceSet()
{
	m_resources.Init(&m_rootSig);
	m_resources.SetCBV(0, 0, m_hsConstantBuffer);
	m_resources.SetCBV(1, 0, m_dsConstantBuffer);
	m_resources.SetSRV(1, 1, *m_texture);
	m_resources.SetSRV(2, 0, *m_texture);
	m_resources.Finalize();
}


void DisplacementApp::UpdateConstantBuffers()
{
	m_hsConstantBuffer.Update(sizeof(m_hsConstants), &m_hsConstants);

	m_dsConstants.lightPos.SetY(5.0f - m_dsConstants.tessStrength);
	m_dsConstants.projectionMatrix = m_camera.GetProjMatrix();
	m_dsConstants.modelMatrix = m_camera.GetViewMatrix();
	m_dsConstantBuffer.Update(sizeof(m_dsConstants), &m_dsConstants);
}


void DisplacementApp::LoadAssets()
{
	m_texture = Texture::Load("stonefloor03_color_bc3_unorm.ktx");

	auto layout = VertexLayout({ VertexComponent::Position, VertexComponent::Normal, VertexComponent::UV });
	m_model = Model::Load("plane.obj", layout, 0.25f);
}