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

#include "PipelinesApp.h"

#include "Graphics\CommandContext.h"
#include "Graphics\CommonStates.h"
#include "Graphics\GraphicsFeatures.h"


using namespace Kodiak;
using namespace Math;
using namespace std;


void PipelinesApp::Configure()
{
	Application::Configure();

	g_optionalFeatures.fillModeNonSolid = true;
}


void PipelinesApp::Startup()
{
	m_camera.SetPerspectiveMatrix(
		DirectX::XMConvertToRadians(60.0f),
		(float)m_displayHeight / (float)(m_displayWidth / 3),
		0.1f,
		512.0f);
	m_camera.SetPosition(Vector3(-4.838f, 3.23f, -7.05f));
	m_camera.Update();

	m_controller.SetSpeedScale(0.01f);
	m_controller.SetCameraMode(CameraMode::ArcBall);
	m_controller.SetOrbitTarget(Vector3(0.0f, 2.5f, 0.0f), Length(m_camera.GetPosition()), 0.25f);

	InitRootSig();
	InitPSOs();
	InitConstantBuffer();

	LoadAssets();

	InitResourceSet();
}


void PipelinesApp::Shutdown()
{
	m_model.reset();

	m_rootSig.Destroy();
}


bool PipelinesApp::Update()
{
	m_controller.Update(m_frameTimer, m_mouseMoveHandled);

	UpdateConstantBuffer();

	return true;
}


void PipelinesApp::Render()
{
	auto& context = GraphicsContext::Begin("Scene");

	context.TransitionResource(GetColorBuffer(), ResourceState::RenderTarget);
	context.TransitionResource(GetDepthBuffer(), ResourceState::DepthWrite);
	context.ClearColor(GetColorBuffer());
	context.ClearDepthAndStencil(GetDepthBuffer());

	context.BeginRenderPass(GetBackBuffer());

	context.SetScissor(0, 0, m_displayWidth, m_displayHeight);

	context.SetRootSignature(m_rootSig);
	context.SetResources(m_resources);

	// Left : solid color
	{
		context.SetViewport(0.0f, 0.0f, (float)m_displayWidth / 3.0f, (float)m_displayHeight);
		context.SetPipelineState(m_phongPSO);
		m_model->Render(context);
	}

	// Middle : toon shading
	{
		context.SetViewport((float)m_displayWidth / 3.0f, 0.0f, (float)m_displayWidth / 3.0f, (float)m_displayHeight);
		context.SetPipelineState(m_toonPSO);
		m_model->Render(context);
	}

	// Right : wireframe
	if (g_enabledFeatures.fillModeNonSolid)
	{
		context.SetViewport(2.0f * (float)m_displayWidth / 3.0f, 0, (float)m_displayWidth / 3.0f, (float)m_displayHeight);
		context.SetPipelineState(m_wireframePSO);
		m_model->Render(context);
	}

	RenderUI(context);

	context.EndRenderPass();
	context.TransitionResource(GetColorBuffer(), ResourceState::Present);

	context.Finish();
}


void PipelinesApp::InitRootSig()
{
	m_rootSig.Reset(1);
	m_rootSig[0].InitAsDescriptorRange(DescriptorType::CBV, 0, 1, ShaderVisibility::Vertex);
	m_rootSig.Finalize("Root Sig", RootSignatureFlags::AllowInputAssemblerInputLayout);
}


void PipelinesApp::InitPSOs()
{
	m_phongPSO.SetRootSignature(m_rootSig);
	m_phongPSO.SetBlendState(CommonStates::BlendDisable());
	m_phongPSO.SetDepthStencilState(CommonStates::DepthStateReadWriteReversed());
	m_phongPSO.SetRasterizerState(CommonStates::RasterizerDefault());
	m_phongPSO.SetRenderTargetFormat(GetColorFormat(), GetDepthFormat());
	m_phongPSO.SetPrimitiveTopology(PrimitiveTopology::TriangleList);

	{
		// Vertex inputs
		VertexStreamDesc vertexStream{ 0, sizeof(Vertex), InputClassification::PerVertexData };
		vector<VertexElementDesc> vertexElements =
		{
			{ "POSITION", 0, Format::R32G32B32_Float, 0, offsetof(Vertex, position), InputClassification::PerVertexData, 0 },
			{ "NORMAL", 0, Format::R32G32B32_Float, 0, offsetof(Vertex, normal), InputClassification::PerVertexData, 0 },
			{ "TEXCOORD", 0, Format::R32G32_Float, 0, offsetof(Vertex, uv), InputClassification::PerVertexData, 0 },
			{ "COLOR", 0, Format::R32G32B32_Float, 0, offsetof(Vertex, color), InputClassification::PerVertexData, 0 },

		};
		m_phongPSO.SetInputLayout(vertexStream, vertexElements);
	}

	if (g_enabledFeatures.fillModeNonSolid)
	{
		m_wireframePSO = m_phongPSO;
		m_wireframePSO.SetParent(&m_phongPSO);

		m_wireframePSO.SetRasterizerState(CommonStates::RasterizerWireframe());
		m_wireframePSO.SetVertexShader("WireframeVS");
		m_wireframePSO.SetPixelShader("WireframePS");
	}

	m_toonPSO = m_phongPSO;
	m_toonPSO.SetParent(&m_phongPSO);

	m_phongPSO.SetVertexShader("PhongVS");
	m_phongPSO.SetPixelShader("PhongPS");

	m_toonPSO.SetVertexShader("ToonVS");
	m_toonPSO.SetPixelShader("ToonPS");

	m_phongPSO.Finalize();
	m_toonPSO.Finalize();

	if (g_enabledFeatures.fillModeNonSolid)
	{
		m_wireframePSO.Finalize();
	}
}


void PipelinesApp::InitConstantBuffer()
{
	m_constantBuffer.Create("Constant Buffer", 1, sizeof(VSConstants));

	UpdateConstantBuffer();
}


void PipelinesApp::InitResourceSet()
{
	m_resources.Init(&m_rootSig);
	m_resources.SetCBV(0, 0, m_constantBuffer);
	m_resources.Finalize();
}


void PipelinesApp::UpdateConstantBuffer()
{
	m_vsConstants.projectionMatrix = m_camera.GetProjMatrix();
	m_vsConstants.modelMatrix = m_camera.GetViewMatrix();
	m_vsConstants.lightPos = Vector4(0.0f, 2.0f, -1.0f, 0.0f);

	m_constantBuffer.Update(sizeof(m_vsConstants), &m_vsConstants);
}


void PipelinesApp::LoadAssets()
{
	auto layout = VertexLayout(
		{
			VertexComponent::Position,
			VertexComponent::Normal,
			VertexComponent::UV,
			VertexComponent::Color
		});

	m_model = Model::Load("treasure_smooth.dae", layout, 1.0f);
}