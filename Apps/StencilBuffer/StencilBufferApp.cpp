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

#include "StencilBufferApp.h"

#include "Graphics\CommandContext.h"
#include "Graphics\CommonStates.h"


using namespace Kodiak;
using namespace Math;
using namespace std;


void StencilBufferApp::Startup()
{
	m_camera.SetPerspectiveMatrix(
		DirectX::XMConvertToRadians(60.0f),
		(float)m_displayHeight / (float)m_displayWidth,
		0.1f, 
		512.0f);
	m_camera.SetPosition(Vector3(-4.838f, 3.23f, -7.05f));
	m_camera.Update();

	m_controller.SetSpeedScale(0.01f);
	m_controller.SetCameraMode(CameraMode::ArcBall);
	m_controller.SetOrbitTarget(Vector3(0.0f, 3.5f, 0.0f), Length(m_camera.GetPosition()), 0.25f);

	InitRootSig();
	InitPSOs();
	InitConstantBuffer();

	LoadAssets();

	InitResourceSet();
}


void StencilBufferApp::Shutdown()
{
	m_model.reset();

	m_rootSig.Destroy();
}


bool StencilBufferApp::Update()
{
	m_controller.Update(m_frameTimer, m_mouseMoveHandled);

	UpdateConstantBuffer();

	return true;
}


void StencilBufferApp::UpdateUI()
{
	if (m_uiOverlay->Header("Settings")) 
	{
		m_uiOverlay->InputFloat("Outline width", &m_constants.outlineWidth, 0.05f, 2);
	}
}


void StencilBufferApp::Render()
{
	auto& context = GraphicsContext::Begin("Scene");

	context.TransitionResource(GetColorBuffer(), ResourceState::RenderTarget);
	context.TransitionResource(GetDepthBuffer(), ResourceState::DepthWrite);
	context.ClearColor(GetColorBuffer());
	context.ClearDepthAndStencil(GetDepthBuffer());

	context.BeginRenderPass(GetBackBuffer());

	context.SetViewportAndScissor(0u, 0u, m_displayWidth, m_displayHeight);

	context.SetRootSignature(m_rootSig);

	context.SetResources(m_resources);

	context.SetStencilRef(1);

	// Draw model toon-shaded to setup stencil mask
	{
		context.SetPipelineState(m_toonPSO);
		m_model->Render(context);
	}

	// Draw stenciled outline
	{
		context.SetPipelineState(m_outlinePSO);
		m_model->Render(context);
	}

	RenderGrid(context);
	RenderUI(context);

	context.EndRenderPass();
	context.TransitionResource(GetColorBuffer(), ResourceState::Present);

	context.Finish();
}


void StencilBufferApp::InitRootSig()
{
	m_rootSig.Reset(1);
	m_rootSig[0].InitAsDescriptorRange(DescriptorType::CBV, 0, 1, ShaderVisibility::Vertex);
	m_rootSig.Finalize("Root Sig", RootSignatureFlags::AllowInputAssemblerInputLayout);
}


void StencilBufferApp::InitPSOs()
{
	// Toon PSO
	{
		m_toonPSO.SetRootSignature(m_rootSig);

		// Render state
		m_toonPSO.SetRasterizerState(CommonStates::RasterizerTwoSided());
		m_toonPSO.SetBlendState(CommonStates::BlendDisable());

		DepthStencilStateDesc dsState = CommonStates::DepthStateReadWriteReversed();
		dsState.stencilEnable = true;
		dsState.backFace.stencilFunc = ComparisonFunc::Always;
		dsState.backFace.stencilDepthFailOp = StencilOp::Replace;
		dsState.backFace.stencilFailOp = StencilOp::Replace;
		dsState.backFace.stencilPassOp = StencilOp::Replace;
		dsState.frontFace = dsState.backFace;
		m_toonPSO.SetDepthStencilState(dsState);

		m_toonPSO.SetVertexShader("ToonVS");
		m_toonPSO.SetPixelShader("ToonPS");

		m_toonPSO.SetRenderTargetFormat(GetColorFormat(), GetDepthFormat());

		m_toonPSO.SetPrimitiveTopology(PrimitiveTopology::TriangleList);

		// Vertex inputs
		VertexStreamDesc vertexStream{ 0, 9 * sizeof(float), InputClassification::PerVertexData };
		vector<VertexElementDesc> vertexElements =
		{
			{ "POSITION", 0, Format::R32G32B32_Float, 0, 0, InputClassification::PerVertexData, 0 },
			{ "COLOR", 0, Format::R32G32B32_Float, 0, 3 * sizeof(float), InputClassification::PerVertexData, 0 },
			{ "NORMAL", 0, Format::R32G32B32_Float, 0, 6 * sizeof(float), InputClassification::PerVertexData, 0 }
		};
		m_toonPSO.SetInputLayout(vertexStream, vertexElements);

		m_toonPSO.Finalize();
	}

	// Outline PSO
	{
		m_outlinePSO.SetRootSignature(m_rootSig);

		// Render state
		m_outlinePSO.SetRasterizerState(CommonStates::RasterizerDefault());
		m_outlinePSO.SetBlendState(CommonStates::BlendDisable());

		DepthStencilStateDesc dsState = CommonStates::DepthStateReadWriteReversed();
		dsState.depthEnable = false;
		dsState.stencilEnable = true;
		dsState.backFace.stencilFunc = ComparisonFunc::NotEqual;
		dsState.backFace.stencilDepthFailOp = StencilOp::Keep;
		dsState.backFace.stencilFailOp = StencilOp::Keep;
		dsState.backFace.stencilPassOp = StencilOp::Replace;
		dsState.frontFace = dsState.backFace;
		m_outlinePSO.SetDepthStencilState(dsState);

		m_outlinePSO.SetVertexShader("OutlineVS");
		m_outlinePSO.SetPixelShader("OutlinePS");

		m_outlinePSO.SetRenderTargetFormat(GetColorFormat(), GetDepthFormat());

		m_outlinePSO.SetPrimitiveTopology(PrimitiveTopology::TriangleList);

		// Vertex inputs
		VertexStreamDesc vertexStream{ 0, 9 * sizeof(float), InputClassification::PerVertexData };
		vector<VertexElementDesc> vertexElements =
		{
			{ "POSITION", 0, Format::R32G32B32_Float, 0, 0, InputClassification::PerVertexData, 0 },
			{ "COLOR", 0, Format::R32G32B32_Float, 0, 3 * sizeof(float), InputClassification::PerVertexData, 0 },
			{ "NORMAL", 0, Format::R32G32B32_Float, 0, 6 * sizeof(float), InputClassification::PerVertexData, 0 }
		};
		m_outlinePSO.SetInputLayout(vertexStream, vertexElements);

		m_outlinePSO.Finalize();
	}
}


void StencilBufferApp::InitConstantBuffer()
{
	m_constantBuffer.Create("Constant Buffer", 1, sizeof(Constants));

	UpdateConstantBuffer();
}


void StencilBufferApp::InitResourceSet()
{
	m_resources.Init(&m_rootSig);
	m_resources.SetCBV(0, 0, m_constantBuffer);
	m_resources.Finalize();
}


void StencilBufferApp::UpdateConstantBuffer()
{
	m_constants.viewProjectionMatrix = m_camera.GetViewProjMatrix();
	m_constants.modelMatrix = Matrix4(kIdentity);
	m_constants.lightPos = Vector4(m_camera.GetPosition(), 1.0f);

	m_constantBuffer.Update(sizeof(m_constants), &m_constants);
}


void StencilBufferApp::LoadAssets()
{
	auto layout = VertexLayout(
	{
		VertexComponent::Position,
		VertexComponent::Color,
		VertexComponent::Normal
	});

	m_model = Model::Load("venus.fbx", layout, 0.3f);
}