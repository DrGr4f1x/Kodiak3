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

#include "OcclusionQueryApp.h"

#include "CommandContext.h"
#include "CommonStates.h"
#include "Filesystem.h"


using namespace Kodiak;
using namespace std;


void OcclusionQueryApp::Configure()
{
	// Setup file system
	auto& filesystem = Kodiak::Filesystem::GetInstance();

	filesystem.SetDefaultRootDir();
	filesystem.AddSearchPath("Data\\" + GetDefaultShaderPath());
	filesystem.AddSearchPath("Data\\Models");

	//g_input.SetCaptureMouse(false);
}


void OcclusionQueryApp::Startup()
{
	using namespace Math;

	m_camera.SetPerspectiveMatrix(
		DirectX::XMConvertToRadians(60.0f),
		(float)m_displayHeight / (float)m_displayWidth,
		0.1f,
		512.0f);
	m_camera.SetPosition(Vector3(0.0f, 0.0f, m_zoom));
	m_camera.Update();

	m_controller.SetSpeedScale(0.01f);
	m_controller.SetCameraMode(CameraMode::ArcBall);
	m_controller.SetOrbitTarget(Vector3(0.0f, 0.0f, 0.0f), Length(m_camera.GetPosition()), 0.25f);

	InitRootSig();
	InitPSOs();
	InitConstantBuffers();

	LoadAssets();
}


void OcclusionQueryApp::Shutdown()
{
	m_rootSignature.Destroy();
}


bool OcclusionQueryApp::Update()
{
	m_controller.Update(m_frameTimer);

	UpdateConstantBuffers();

	return true;
}


void OcclusionQueryApp::Render()
{
	auto& context = GraphicsContext::Begin("Render Frame");

	context.TransitionResource(GetColorBuffer(), ResourceState::RenderTarget);
	context.TransitionResource(GetDepthBuffer(), ResourceState::DepthWrite);
	context.ClearColor(GetColorBuffer());
	context.ClearDepthAndStencil(GetDepthBuffer());

	context.BeginRenderPass(GetBackBuffer());

	context.SetViewportAndScissor(0u, 0u, m_displayWidth, m_displayHeight);
	context.SetRootSignature(m_rootSignature);

	// Occlusion pass
	{

	}

	// Visible pass
	{
		// Teapot
		context.SetPipelineState(m_solidPSO);
		context.SetRootConstantBuffer(0, m_teapotCB);
		context.SetVertexBuffer(0, m_teapotModel->GetVertexBuffer());
		context.SetIndexBuffer(m_teapotModel->GetIndexBuffer());
		context.DrawIndexed((uint32_t)m_teapotModel->GetIndexBuffer().GetElementCount());

		// Sphere
		context.SetRootConstantBuffer(0, m_sphereCB);
		context.SetVertexBuffer(0, m_sphereModel->GetVertexBuffer());
		context.SetIndexBuffer(m_sphereModel->GetIndexBuffer());
		context.DrawIndexed((uint32_t)m_sphereModel->GetIndexBuffer().GetElementCount());

		// Occluder plane
		context.SetPipelineState(m_occluderPSO);
		context.SetRootConstantBuffer(0, m_occluderCB);
		context.SetVertexBuffer(0, m_occluderModel->GetVertexBuffer());
		context.SetIndexBuffer(m_occluderModel->GetIndexBuffer());
		context.DrawIndexed((uint32_t)m_occluderModel->GetIndexBuffer().GetElementCount());
	}

	context.EndRenderPass();
	context.TransitionResource(GetColorBuffer(), ResourceState::Present);

	context.Finish();
}


void OcclusionQueryApp::InitRootSig()
{
	m_rootSignature.Reset(1);
	m_rootSignature[0].InitAsConstantBuffer(0, ShaderVisibility::Vertex);
	m_rootSignature.Finalize("Root Sig", RootSignatureFlags::AllowInputAssemblerInputLayout | RootSignatureFlags::DenyPixelShaderRootAccess);
}


void OcclusionQueryApp::InitPSOs()
{
	m_solidPSO.SetRootSignature(m_rootSignature);
	m_solidPSO.SetBlendState(CommonStates::BlendDisable());
	m_solidPSO.SetDepthStencilState(CommonStates::DepthStateReadWriteReversed());
	m_solidPSO.SetRasterizerState(CommonStates::RasterizerDefaultCW());
	m_solidPSO.SetRenderTargetFormat(GetColorFormat(), GetDepthFormat());
	m_solidPSO.SetPrimitiveTopology(PrimitiveTopology::TriangleList);
	m_solidPSO.SetVertexShader("MeshVS");
	m_solidPSO.SetPixelShader("MeshPS");

	// Vertex inputs
	VertexStreamDesc vertexStream{ 0, 9 * sizeof(float), InputClassification::PerVertexData };
	vector<VertexElementDesc> vertexElements =
	{
		{ "POSITION", 0, Format::R32G32B32_Float, 0, 0, InputClassification::PerVertexData, 0 },
		{ "NORMAL", 0, Format::R32G32B32_Float, 0, 3 * sizeof(float), InputClassification::PerVertexData, 0 },
		{ "COLOR", 0, Format::R32G32B32_Float, 0, 6 * sizeof(float), InputClassification::PerVertexData, 0 }
	};

	m_solidPSO.SetInputLayout(vertexStream, vertexElements);

	m_simplePSO = m_solidPSO;
	m_simplePSO.SetRasterizerState(CommonStates::RasterizerTwoSided());
	m_simplePSO.SetVertexShader("SimpleVS");
	m_simplePSO.SetPixelShader("SimplePS");

	m_occluderPSO = m_simplePSO;
	BlendStateDesc blendDesc;
	blendDesc.renderTargetBlend[0].blendEnable = true;
	blendDesc.renderTargetBlend[0].blendOp = BlendOp::Add;
	blendDesc.renderTargetBlend[0].srcBlend = Blend::SrcColor;
	blendDesc.renderTargetBlend[0].dstBlend = Blend::InvSrcColor;
	m_occluderPSO.SetBlendState(blendDesc);
	m_occluderPSO.SetVertexShader("OccluderVS");
	m_occluderPSO.SetPixelShader("OccluderPS");

	m_solidPSO.Finalize();
	m_simplePSO.Finalize();
	m_occluderPSO.Finalize();
}


void OcclusionQueryApp::InitConstantBuffers()
{
	m_occluderCB.Create("Occluder Constant Buffer", 1, sizeof(VSConstants));
	m_teapotCB.Create("Teapot Constant Buffer", 1, sizeof(VSConstants));
	m_sphereCB.Create("Sphere Constant Buffer", 1, sizeof(VSConstants));

	UpdateConstantBuffers();
}


void OcclusionQueryApp::UpdateConstantBuffers()
{
	using namespace Math;
	using namespace DirectX;

	Matrix4 projectionMatrix = m_camera.GetProjMatrix();
	Matrix4 viewMatrix = m_camera.GetViewMatrix();

	Matrix4 rotationMatrix(AffineTransform::MakeYRotation(XMConvertToRadians(-135.0f)));

	m_occluderConstants.projectionMatrix = projectionMatrix;
	m_occluderConstants.modelViewMatrix = viewMatrix * rotationMatrix;
	m_occluderCB.Update(sizeof(m_occluderConstants), &m_occluderConstants);

	m_teapotConstants.projectionMatrix = projectionMatrix;
	m_teapotConstants.modelViewMatrix = viewMatrix * rotationMatrix * Matrix4(AffineTransform::MakeTranslation(Vector3(0.0f, 0.0f, -10.0f)));
	m_teapotConstants.visible = 1.0f;
	m_teapotCB.Update(sizeof(m_teapotConstants), &m_teapotConstants);

	m_sphereConstants.projectionMatrix = projectionMatrix;
	m_sphereConstants.modelViewMatrix = viewMatrix * rotationMatrix * Matrix4(AffineTransform::MakeTranslation(Vector3(0.0f, 0.0f, 10.0f)));
	m_sphereConstants.visible = 1.0f;
	m_sphereCB.Update(sizeof(m_sphereConstants), &m_sphereConstants);
}


void OcclusionQueryApp::LoadAssets()
{
	auto layout = VertexLayout(
	{
		VertexComponent::Position,
		VertexComponent::Normal,
		VertexComponent::Color
	});

	m_occluderModel = Model::Load("plane_z.3ds", layout, 0.4f);
	m_teapotModel = Model::Load("teapot.3ds", layout, 0.3f);
	m_sphereModel = Model::Load("sphere.3ds", layout, 0.3f);
}