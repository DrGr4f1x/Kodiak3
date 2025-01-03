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

#include "Graphics\CommandContext.h"
#include "Graphics\CommonStates.h"
#include "Graphics\UIOverlay.h"


using namespace Kodiak;
using namespace Math;
using namespace std;


void OcclusionQueryApp::Startup()
{
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
	InitQueryHeap();

	LoadAssets();

	InitResourceSets();
}


void OcclusionQueryApp::Shutdown()
{
	m_rootSig.Destroy();
}


bool OcclusionQueryApp::Update()
{
	m_controller.Update(m_frameTimer, m_mouseMoveHandled);

	UpdateConstantBuffers();

	return true;
}


void OcclusionQueryApp::UpdateUI()
{
	if (m_uiOverlay->Header("Occlusion query results")) 
	{
		m_uiOverlay->Text("Teapot: %d samples passed", m_passedSamples[0]);
		m_uiOverlay->Text("Sphere: %d samples passed", m_passedSamples[1]);
	}
}


void OcclusionQueryApp::Render()
{
	auto& context = GraphicsContext::Begin("Scene");

	context.TransitionResource(GetColorBuffer(), ResourceState::RenderTarget);
	context.TransitionResource(GetDepthBuffer(), ResourceState::DepthWrite);
	context.ClearColor(GetColorBuffer());
	context.ClearDepthAndStencil(GetDepthBuffer());

	auto curFrame = GetCurrentFrame();

	// Occlusion pass

	context.ResetOcclusionQueries(m_queryHeap, 2 * curFrame, 2);
	context.BeginRenderPass(GetBackBuffer());

	context.SetViewportAndScissor(0u, 0u, m_displayWidth, m_displayHeight);
	context.SetRootSignature(m_rootSig);

	{
		context.SetPipelineState(m_simplePSO);

		// Occluder plane
		context.SetResources(m_occluderResources);
		m_occluderModel->Render(context);

		// Teapot
		context.BeginOcclusionQuery(m_queryHeap, 2 * curFrame);

		context.SetResources(m_teapotResources);
		m_teapotModel->Render(context);

		context.EndOcclusionQuery(m_queryHeap, 2 * curFrame);

		// Sphere
		context.BeginOcclusionQuery(m_queryHeap, 2 * curFrame + 1);

		context.SetResources(m_sphereResources);
		m_sphereModel->Render(context);

		context.EndOcclusionQuery(m_queryHeap, 2 * curFrame + 1);
	}
	context.EndRenderPass();

	// Copy query results to buffer
	context.ResolveOcclusionQueries(m_queryHeap, 2 * curFrame, 2, m_readbackBuffer, 2 * curFrame * sizeof(uint64_t));

	context.ClearColor(GetColorBuffer());
	context.ClearDepthAndStencil(GetDepthBuffer());

	// Visible pass

	context.BeginRenderPass(GetBackBuffer());

	context.SetViewportAndScissor(0u, 0u, m_displayWidth, m_displayHeight);
	context.SetRootSignature(m_rootSig);
	{
		// Teapot
		context.SetPipelineState(m_solidPSO);
		context.SetResources(m_teapotResources);
		m_teapotModel->Render(context);

		// Sphere
		context.SetResources(m_sphereResources);
		m_sphereModel->Render(context);

		// Occluder plane
		context.SetPipelineState(m_occluderPSO);
		context.SetResources(m_occluderResources);
		m_occluderModel->Render(context);
	}

	RenderUI(context);

	context.EndRenderPass();
	context.TransitionResource(GetColorBuffer(), ResourceState::Present);

	context.Finish();
}


void OcclusionQueryApp::InitRootSig()
{
	m_rootSig.Reset(1);
	m_rootSig[0].InitAsDescriptorRange(DescriptorType::CBV, 0, 1, ShaderVisibility::Vertex);
	m_rootSig.Finalize("Root Sig", RootSignatureFlags::AllowInputAssemblerInputLayout | RootSignatureFlags::DenyPixelShaderRootAccess);
}


void OcclusionQueryApp::InitPSOs()
{
	m_solidPSO.SetRootSignature(m_rootSig);
	m_solidPSO.SetBlendState(CommonStates::BlendDisable());
	m_solidPSO.SetDepthStencilState(CommonStates::DepthStateReadWriteReversed());
	m_solidPSO.SetRasterizerState(CommonStates::RasterizerDefault());
	m_solidPSO.SetRenderTargetFormat(GetColorFormat(), GetDepthFormat());
	m_solidPSO.SetPrimitiveTopology(PrimitiveTopology::TriangleList);
	m_solidPSO.SetVertexShader("MeshVS");
	m_solidPSO.SetPixelShader("MeshPS");

	// Vertex inputs
	VertexStreamDesc vertexStream{ 0, 10 * sizeof(float), InputClassification::PerVertexData };
	vector<VertexElementDesc> vertexElements =
	{
		{ "POSITION", 0, Format::R32G32B32_Float, 0, 0, InputClassification::PerVertexData, 0 },
		{ "NORMAL", 0, Format::R32G32B32_Float, 0, 3 * sizeof(float), InputClassification::PerVertexData, 0 },
		{ "COLOR", 0, Format::R32G32B32A32_Float, 0, 6 * sizeof(float), InputClassification::PerVertexData, 0 }
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


void OcclusionQueryApp::InitQueryHeap()
{
	m_queryHeap.Create(2 * NumSwapChainBuffers);
	m_readbackBuffer.Create("Readback Buffer", 2 * NumSwapChainBuffers, sizeof(uint64_t));
}


void OcclusionQueryApp::InitResourceSets()
{
	m_occluderResources.Init(&m_rootSig);
	m_occluderResources.SetCBV(0, 0, m_occluderCB);
	m_occluderResources.Finalize();


	m_teapotResources.Init(&m_rootSig);
	m_teapotResources.SetCBV(0, 0, m_teapotCB);
	m_teapotResources.Finalize();


	m_sphereResources.Init(&m_rootSig);
	m_sphereResources.SetCBV(0, 0, m_sphereCB);
	m_sphereResources.Finalize();
}


void OcclusionQueryApp::UpdateConstantBuffers()
{
	using namespace DirectX;

	Matrix4 projectionMatrix = m_camera.GetProjMatrix();
	Matrix4 viewMatrix = m_camera.GetViewMatrix();

	Matrix4 rotationMatrix(AffineTransform::MakeYRotation(XMConvertToRadians(-135.0f)));

	m_occluderConstants.projectionMatrix = projectionMatrix;
	m_occluderConstants.modelViewMatrix = viewMatrix * rotationMatrix;
	m_occluderCB.Update(sizeof(m_occluderConstants), &m_occluderConstants);

	uint64_t teapotQueryResult = 1;
	uint64_t sphereQueryResult = 1;

	uint32_t resultFrame = (GetCurrentFrame() + 1) % NumSwapChainBuffers;
	bool getResults = GetFrameNumber() >= NumSwapChainBuffers;

	if (getResults)
	{
		uint64_t* data = (uint64_t*)m_readbackBuffer.Map();

		teapotQueryResult = data[2 * resultFrame];
		sphereQueryResult = data[2 * resultFrame + 1];

		m_passedSamples[0] = (uint32_t)teapotQueryResult;
		m_passedSamples[1] = (uint32_t)sphereQueryResult;

		m_readbackBuffer.Unmap();
	}

	m_teapotConstants.projectionMatrix = projectionMatrix;
	m_teapotConstants.modelViewMatrix = viewMatrix * rotationMatrix * Matrix4(AffineTransform::MakeTranslation(Vector3(0.0f, 0.0f, -10.0f)));
	m_teapotConstants.visible = teapotQueryResult > 0 ? 1.0f : 0.0f;
	m_teapotCB.Update(sizeof(m_teapotConstants), &m_teapotConstants);

	m_sphereConstants.projectionMatrix = projectionMatrix;
	m_sphereConstants.modelViewMatrix = viewMatrix * rotationMatrix * Matrix4(AffineTransform::MakeTranslation(Vector3(0.0f, 0.0f, 10.0f)));
	m_sphereConstants.visible = sphereQueryResult > 0 ? 1.0f : 0.0f;
	m_sphereCB.Update(sizeof(m_sphereConstants), &m_sphereConstants);
}


void OcclusionQueryApp::LoadAssets()
{
	auto layout = VertexLayout<VertexComponent::PositionNormalColor>();

	m_occluderModel = Model::Load("plane_z.3ds", layout, 0.4f);
	m_teapotModel = Model::Load("teapot.3ds", layout, 0.3f);
	m_sphereModel = Model::Load("sphere.3ds", layout, 0.3f);
}