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

#include "CommandContext.h"
#include "CommonStates.h"
#include "GraphicsDevice.h"
#include "Filesystem.h"
#include "Input.h"
#include "Shader.h"


using namespace Kodiak;
using namespace std;


void StencilBufferApp::Configure()
{
	// Setup file system
	auto& filesystem = Kodiak::Filesystem::GetInstance();

	filesystem.SetDefaultRootDir();
	filesystem.AddSearchPath("Data\\" + GetDefaultShaderPath());
	filesystem.AddSearchPath("Data\\Models");

	//g_input.SetCaptureMouse(false);
}


void StencilBufferApp::Startup()
{
	using namespace Math;

	m_camera.SetPerspectiveMatrix(
		DirectX::XMConvertToRadians(60.0f),
		(float)m_displayHeight / (float)m_displayWidth,
		0.1f, 
		512.0f);
	m_camera.SetPosition(Vector3(-4.838f, 3.23f, -7.05f));
	m_camera.Update();

	m_controller.SetSpeedScale(0.01f);
	m_controller.SetCameraMode(CameraMode::ArcBall);
	m_controller.SetOrbitTarget(Math::Vector3(0.0f, 3.5f, 0.0f), Math::Length(m_camera.GetPosition()), 0.25f);

	InitRootSig();
	InitPSOs();
	InitConstantBuffer();

	LoadAssets();
}


void StencilBufferApp::Shutdown()
{
	m_model.reset();

	m_rootSig.Destroy();
	m_constantBuffer.Destroy();
}


bool StencilBufferApp::Update()
{
	m_controller.Update(m_frameTimer);

	UpdateConstantBuffer();

	return true;
}


void StencilBufferApp::Render()
{
	auto& context = GraphicsContext::Begin("Render frame");

	uint32_t curFrame = m_graphicsDevice->GetCurrentBuffer();

	Color clearColor{ DirectX::Colors::Black };
	context.BeginRenderPass(m_renderPass, *m_framebuffers[curFrame], clearColor, 1.0f, 0);

	context.SetViewportAndScissor(0u, 0u, m_displayWidth, m_displayHeight);

	context.SetRootSignature(m_rootSig);

	context.SetRootConstantBuffer(0, m_constantBuffer);

	context.SetVertexBuffer(0, m_model->GetVertexBuffer());
	context.SetIndexBuffer(m_model->GetIndexBuffer());

	context.SetStencilRef(1);

	// Draw model toon-shaded to setup stencil mask
	{
		context.SetPipelineState(m_toonPSO);
		context.DrawIndexed((uint32_t)m_model->GetIndexBuffer().GetElementCount());
	}

	// Draw stenciled outline
	{
		context.SetPipelineState(m_outlinePSO);
		context.DrawIndexed((uint32_t)m_model->GetIndexBuffer().GetElementCount());
	}

	context.EndRenderPass();

	context.Finish();
}


void StencilBufferApp::InitRootSig()
{
	m_rootSig.Reset(1);
	m_rootSig[0].InitAsConstantBuffer(0, ShaderVisibility::Vertex);
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

		m_toonPSO.SetRenderPass(m_renderPass);

		m_toonPSO.SetPrimitiveTopology(PrimitiveTopology::TriangleList);

		// Vertex inputs
		VertexStreamDesc vertexStreamDesc{ 0, 9 * sizeof(float), InputClassification::PerVertexData };
		VertexElementDesc vertexElements[] =
		{
			{ "POSITION", 0, Format::R32G32B32_Float, 0, 0, InputClassification::PerVertexData, 0 },
			{ "COLOR", 0, Format::R32G32B32_Float, 0, 3 * sizeof(float), InputClassification::PerVertexData, 0 },
			{ "NORMAL", 0, Format::R32G32B32_Float, 0, 6 * sizeof(float), InputClassification::PerVertexData, 0 }
		};
		m_toonPSO.SetInputLayout(1, &vertexStreamDesc, _countof(vertexElements), vertexElements);

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

		m_outlinePSO.SetRenderPass(m_renderPass);

		m_outlinePSO.SetPrimitiveTopology(PrimitiveTopology::TriangleList);

		// Vertex inputs
		VertexStreamDesc vertexStreamDesc{ 0, 9 * sizeof(float), InputClassification::PerVertexData };
		VertexElementDesc vertexElements[] =
		{
			{ "POSITION", 0, Format::R32G32B32_Float, 0, 0, InputClassification::PerVertexData, 0 },
			{ "COLOR", 0, Format::R32G32B32_Float, 0, 3 * sizeof(float), InputClassification::PerVertexData, 0 },
			{ "NORMAL", 0, Format::R32G32B32_Float, 0, 6 * sizeof(float), InputClassification::PerVertexData, 0 }
		};
		m_outlinePSO.SetInputLayout(1, &vertexStreamDesc, _countof(vertexElements), vertexElements);

		m_outlinePSO.Finalize();
	}
}


void StencilBufferApp::InitConstantBuffer()
{
	m_constantBuffer.Create("Constant Buffer", 1, sizeof(Constants));

	UpdateConstantBuffer();
}


void StencilBufferApp::UpdateConstantBuffer()
{
	using namespace Math;

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