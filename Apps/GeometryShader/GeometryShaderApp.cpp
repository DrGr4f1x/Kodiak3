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

#include "CommandContext.h"
#include "CommonStates.h"
#include "Filesystem.h"
#include "GraphicsDevice.h"


using namespace Kodiak;
using namespace std;


void GeometryShaderApp::Configure()
{
	// Setup file system
	auto& filesystem = Kodiak::Filesystem::GetInstance();

	filesystem.SetDefaultRootDir();
	filesystem.AddSearchPath("Data\\" + GetDefaultShaderPath());
	filesystem.AddSearchPath("Data\\Models");
}


void GeometryShaderApp::Startup()
{
	InitRootSigs();
	InitPSOs();
	InitConstantBuffer();

	m_camera.SetPerspectiveMatrix(
		XMConvertToRadians(60.0f),
		(float)m_displayHeight / (float)m_displayWidth,
		0.001f,
		256.0f);
	m_camera.SetPosition(Math::Vector3(0.0f, 0.0f, 8.0f));

	m_camera.Update();

	m_controller.SetSpeedScale(0.01f);
	m_controller.RefreshFromCamera();

	UpdateConstantBuffer();

	LoadAssets();
}


void GeometryShaderApp::Shutdown()
{
	m_model.reset();

	m_constantBuffer.Destroy();

	m_meshRootSig.Destroy();
	m_geomRootSig.Destroy();
}


bool GeometryShaderApp::Update()
{
	m_controller.Update(m_frameTimer);

	UpdateConstantBuffer();

	return true;
}


void GeometryShaderApp::Render()
{
	auto& context = GraphicsContext::Begin("Render frame");

	uint32_t curFrame = m_graphicsDevice->GetCurrentBuffer();

	Color clearColor{ DirectX::Colors::Black };
	context.BeginRenderPass(m_renderPass, *m_framebuffers[curFrame], clearColor, 1.0f, 0);

	context.SetViewportAndScissor(0u, 0u, m_displayWidth, m_displayHeight);

	context.SetRootSignature(m_meshRootSig);
	context.SetPipelineState(m_meshPSO);

	context.SetRootConstantBuffer(0, m_constantBuffer);

	context.SetIndexBuffer(m_model->GetIndexBuffer());
	context.SetVertexBuffer(0, m_model->GetVertexBuffer());

	context.DrawIndexed((uint32_t)m_model->GetIndexBuffer().GetElementCount());

	{
		context.SetRootSignature(m_geomRootSig);
		context.SetPipelineState(m_geomPSO);
		
		context.SetRootConstantBuffer(0, m_constantBuffer);
		
		context.DrawIndexed((uint32_t)m_model->GetIndexBuffer().GetElementCount());
	}

	context.EndRenderPass();

	context.Finish();
}


void GeometryShaderApp::InitRootSigs()
{
	m_meshRootSig.Reset(1);
	m_meshRootSig[0].InitAsConstantBuffer(0, ShaderVisibility::Vertex);
	m_meshRootSig.Finalize("Mesh Root Sig", RootSignatureFlags::AllowInputAssemblerInputLayout);

	m_geomRootSig.Reset(1);
	m_geomRootSig[0].InitAsConstantBuffer(0, ShaderVisibility::Geometry);
	m_geomRootSig.Finalize("Geom Root Sig", RootSignatureFlags::AllowInputAssemblerInputLayout);
}


void GeometryShaderApp::InitPSOs()
{
	m_meshPSO.SetRootSignature(m_meshRootSig);
	m_meshPSO.SetRenderPass(m_renderPass);

	m_meshPSO.SetBlendState(CommonStates::BlendDisable());
	m_meshPSO.SetRasterizerState(CommonStates::RasterizerDefaultCW());
	m_meshPSO.SetDepthStencilState(CommonStates::DepthStateReadWriteReversed());

	m_meshPSO.SetPrimitiveTopology(PrimitiveTopology::TriangleList);

	m_meshPSO.SetVertexShader("MeshVS");
	m_meshPSO.SetPixelShader("MeshPS");

	VertexStreamDesc vertexStreamDesc{ 0, sizeof(Vertex), InputClassification::PerVertexData };
	VertexElementDesc vertexElements[] =
	{
		{ "POSITION", 0, Format::R32G32B32_Float, 0, offsetof(Vertex, position), InputClassification::PerVertexData, 0 },
		{ "NORMAL", 0, Format::R32G32B32_Float, 0, offsetof(Vertex, normal), InputClassification::PerVertexData, 0 },
		{ "COLOR", 0, Format::R32G32B32_Float, 0, offsetof(Vertex, color), InputClassification::PerVertexData, 0 },
		
	};
	m_meshPSO.SetInputLayout(1, &vertexStreamDesc, _countof(vertexElements), vertexElements);
	m_meshPSO.Finalize();

	
	m_geomPSO.SetRootSignature(m_geomRootSig);
	m_geomPSO.SetRenderPass(m_renderPass);

	m_geomPSO.SetBlendState(CommonStates::BlendDisable());
	m_geomPSO.SetRasterizerState(CommonStates::RasterizerDefault());
	m_geomPSO.SetDepthStencilState(CommonStates::DepthStateReadWriteReversed());

	m_geomPSO.SetPrimitiveTopology(PrimitiveTopology::TriangleList);

	m_geomPSO.SetVertexShader("BaseVS");
	m_geomPSO.SetGeometryShader("BaseGS");
	m_geomPSO.SetPixelShader("BasePS");

	m_geomPSO.SetInputLayout(1, &vertexStreamDesc, _countof(vertexElements), vertexElements);
	m_geomPSO.Finalize();
}


void GeometryShaderApp::InitConstantBuffer()
{
	m_constantBuffer.Create("Constant Buffer", 1, sizeof(Constants));
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