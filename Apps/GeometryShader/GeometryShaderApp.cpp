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

#include "GeometryShaderApp.h"

#include "Graphics\CommandContext.h"
#include "Graphics\CommonStates.h"
#include "Graphics\GraphicsFeatures.h"


using namespace Kodiak;
using namespace std;


void GeometryShaderApp::Configure()
{
	Application::Configure();

	// Specify required graphics features 
	g_requiredFeatures.geometryShader = true;
}


void GeometryShaderApp::Startup()
{
	using namespace Math;

	m_camera.SetPerspectiveMatrix(
		XMConvertToRadians(60.0f),
		(float)m_displayHeight / (float)m_displayWidth,
		0.001f,
		256.0f);
	m_camera.SetPosition(Math::Vector3(-3.38f, 0.0f, -7.25f));

	m_controller.SetSpeedScale(0.01f);
	m_controller.SetCameraMode(CameraMode::ArcBall);
	m_controller.SetOrbitTarget(Vector3(0.0f, 0.0f, 0.0f), Length(m_camera.GetPosition()), 4.0f);

	InitRootSigs();
	InitPSOs();
	InitConstantBuffer();

	LoadAssets();

	InitResourceSets();
}


void GeometryShaderApp::Shutdown()
{
	m_meshRootSig.Destroy();
	m_geomRootSig.Destroy();
}


bool GeometryShaderApp::Update()
{
	m_controller.Update(m_frameTimer, m_mouseMoveHandled);

	UpdateConstantBuffer();

	return true;
}


void GeometryShaderApp::UpdateUI()
{
	if (m_uiOverlay->Header("Settings")) 
	{
		m_uiOverlay->CheckBox("Display normals", &m_showNormals);
	}
}


void GeometryShaderApp::Render()
{
	auto& context = GraphicsContext::Begin("Scene");

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

	if(m_showNormals)
	{
		context.SetRootSignature(m_geomRootSig);
		context.SetPipelineState(m_geomPSO);
		
		context.SetResources(m_geomResources);

		context.DrawIndexed((uint32_t)m_model->GetIndexBuffer().GetElementCount());
	}

	RenderUI(context);

	context.EndRenderPass();
	context.TransitionResource(GetColorBuffer(), ResourceState::Present);

	context.Finish();
}


void GeometryShaderApp::InitRootSigs()
{
	m_meshRootSig.Reset(1);
	m_meshRootSig[0].InitAsDescriptorRange(DescriptorType::CBV, 0, 1, ShaderVisibility::Vertex);
	m_meshRootSig.Finalize("Mesh Root Sig", RootSignatureFlags::AllowInputAssemblerInputLayout);

	m_geomRootSig.Reset(1);
	m_geomRootSig[0].InitAsDescriptorRange(DescriptorType::CBV, 0, 1, ShaderVisibility::Geometry);
	m_geomRootSig.Finalize("Geom Root Sig", RootSignatureFlags::AllowInputAssemblerInputLayout);
}


void GeometryShaderApp::InitPSOs()
{
	m_meshPSO.SetRootSignature(m_meshRootSig);
	m_meshPSO.SetRenderTargetFormat(GetColorFormat(), GetDepthFormat());

	m_meshPSO.SetBlendState(CommonStates::BlendDisable());
	m_meshPSO.SetRasterizerState(CommonStates::RasterizerDefaultCW());
	m_meshPSO.SetDepthStencilState(CommonStates::DepthStateReadWriteReversed());

	m_meshPSO.SetPrimitiveTopology(PrimitiveTopology::TriangleList);

	m_meshPSO.SetVertexShader("MeshVS");
	m_meshPSO.SetPixelShader("MeshPS");

	VertexStreamDesc vertexStream{ 0, sizeof(Vertex), InputClassification::PerVertexData };
	vector<VertexElementDesc> vertexElements =
	{
		{ "POSITION", 0, Format::R32G32B32_Float, 0, offsetof(Vertex, position), InputClassification::PerVertexData, 0 },
		{ "NORMAL", 0, Format::R32G32B32_Float, 0, offsetof(Vertex, normal), InputClassification::PerVertexData, 0 },
		{ "COLOR", 0, Format::R32G32B32_Float, 0, offsetof(Vertex, color), InputClassification::PerVertexData, 0 },
		
	};
	m_meshPSO.SetInputLayout(vertexStream, vertexElements);
	m_meshPSO.Finalize();

	
	m_geomPSO.SetRootSignature(m_geomRootSig);
	m_geomPSO.SetRenderTargetFormat(GetColorFormat(), GetDepthFormat());

	m_geomPSO.SetBlendState(CommonStates::BlendDisable());
	m_geomPSO.SetRasterizerState(CommonStates::RasterizerDefault());
	m_geomPSO.SetDepthStencilState(CommonStates::DepthStateReadWriteReversed());

	m_geomPSO.SetPrimitiveTopology(PrimitiveTopology::TriangleList);

	m_geomPSO.SetVertexShader("BaseVS");
	m_geomPSO.SetGeometryShader("BaseGS");
	m_geomPSO.SetPixelShader("BasePS");

	m_geomPSO.SetInputLayout(vertexStream, vertexElements);
	m_geomPSO.Finalize();
}


void GeometryShaderApp::InitConstantBuffer()
{
	m_constantBuffer.Create("Constant Buffer", 1, sizeof(Constants));

	UpdateConstantBuffer();
}


void GeometryShaderApp::InitResourceSets()
{
	m_meshResources.Init(&m_meshRootSig);
	m_meshResources.SetCBV(0, 0, m_constantBuffer);
	m_meshResources.Finalize();

	m_geomResources.Init(&m_geomRootSig);
	m_geomResources.SetCBV(0, 0, m_constantBuffer);
	m_geomResources.Finalize();
}


void GeometryShaderApp::UpdateConstantBuffer()
{
	m_constants.projectionMatrix = m_camera.GetProjMatrix();
	m_constants.modelMatrix = m_camera.GetViewMatrix();

	m_constantBuffer.Update(sizeof(m_constants), &m_constants);
}


void GeometryShaderApp::LoadAssets()
{
	auto layout = VertexLayout({
		VertexComponent::Position,
		VertexComponent::Normal,
		VertexComponent::Color
	});
	m_model = Model::Load("suzanne.obj", layout, 0.25f);
}