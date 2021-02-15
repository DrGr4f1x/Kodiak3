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
	Vector3 center = m_planeModel->GetBoundingBox().GetCenter();
	
	m_camera.SetPerspectiveMatrix(
		XMConvertToRadians(60.0f),
		(float)m_displayHeight / (float)m_displayWidth,
		0.001f,
		256.0f);
	m_camera.SetPosition(Math::Vector3(-3.38f, 2.0f, -7.25f));
	m_camera.Focus(m_planeModel->GetBoundingBox());

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
	

	context.SetResources(m_meshResources);
	
	// Plane
	{
		context.SetPipelineState(m_planePSO);
		m_planeModel->Render(context);
	}

	// Cylinder
	{
		context.SetPipelineState(m_cylinderPSO);
		m_boxModel->Render(context);
	}

	// Grid and UI
	RenderGrid(context);

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
	{
		m_planePSO.SetRootSignature(m_meshRootSig);
		m_planePSO.SetRenderTargetFormat(GetColorFormat(), GetDepthFormat());

		m_planePSO.SetBlendState(CommonStates::BlendDisable());
		m_planePSO.SetRasterizerState(CommonStates::RasterizerDefault());
		m_planePSO.SetDepthStencilState(CommonStates::DepthStateReadWriteReversed());

		m_planePSO.SetPrimitiveTopology(PrimitiveTopology::TriangleList);

		m_planePSO.SetVertexShader("MeshVS");
		m_planePSO.SetPixelShader("MeshPS");

		VertexStreamDesc vertexStream{ 0, sizeof(Vertex), InputClassification::PerVertexData };
		vector<VertexElementDesc> vertexElements =
		{
			{ "POSITION", 0, Format::R32G32B32_Float, 0, offsetof(Vertex, position), InputClassification::PerVertexData, 0 },
			{ "NORMAL", 0, Format::R32G32B32_Float, 0, offsetof(Vertex, normal), InputClassification::PerVertexData, 0 },
			{ "TEXCOORD", 0, Format::R32G32_Float, 0, offsetof(Vertex, uv), InputClassification::PerVertexData, 0 },

		};
		m_planePSO.SetInputLayout(vertexStream, vertexElements);
		m_planePSO.Finalize();
	}

	{
		m_cylinderPSO.SetRootSignature(m_meshRootSig);
		m_cylinderPSO.SetRenderTargetFormat(GetColorFormat(), GetDepthFormat());

		m_cylinderPSO.SetBlendState(CommonStates::BlendDisable());
		m_cylinderPSO.SetRasterizerState(CommonStates::RasterizerDefault());
		m_cylinderPSO.SetDepthStencilState(CommonStates::DepthStateReadWriteReversed());

		m_cylinderPSO.SetPrimitiveTopology(PrimitiveTopology::TriangleStrip);
		m_cylinderPSO.SetPrimitiveRestart(IndexBufferStripCutValue::Value_0xFFFF);

		m_cylinderPSO.SetVertexShader("MeshVS");
		m_cylinderPSO.SetPixelShader("MeshPS");

		VertexStreamDesc vertexStream{ 0, sizeof(Vertex), InputClassification::PerVertexData };
		vector<VertexElementDesc> vertexElements =
		{
			{ "POSITION", 0, Format::R32G32B32_Float, 0, offsetof(Vertex, position), InputClassification::PerVertexData, 0 },
			{ "NORMAL", 0, Format::R32G32B32_Float, 0, offsetof(Vertex, normal), InputClassification::PerVertexData, 0 },
			{ "TEXCOORD", 0, Format::R32G32_Float, 0, offsetof(Vertex, uv), InputClassification::PerVertexData, 0 },

		};
		m_cylinderPSO.SetInputLayout(vertexStream, vertexElements);
		m_cylinderPSO.Finalize();
	}
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
	//m_model = Model::Load("Statue\\socha_100kuw.obj", layout, 0.25f);
	m_planeModel = Model::MakePlane(layout, 10.0f, 10.0f);
	m_cylinderModel = Model::MakeCylinder(layout, 5.0f, 1.0f, 32);
	m_sphereModel = Model::MakeSphere(layout, 5.0f, 16, 16);
	m_boxModel = Model::MakeBox(layout, 2.0f, 4.0f, 6.0f);
}