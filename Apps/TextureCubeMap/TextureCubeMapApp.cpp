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

#include "TextureCubeMapApp.h"

#include "CommandContext.h"
#include "CommonStates.h"
#include "Filesystem.h"
#include "GraphicsDevice.h"


using namespace Kodiak;
using namespace std;


void TextureCubeMapApp::Configure()
{
	// Setup file system
	auto& filesystem = Kodiak::Filesystem::GetInstance();

	filesystem.SetDefaultRootDir();
	filesystem.AddSearchPath("Data\\" + GetDefaultShaderPath());
	filesystem.AddSearchPath("Data\\Models");
	filesystem.AddSearchPath("Data\\Textures");
}


void TextureCubeMapApp::Startup()
{
	InitRootSigs();
	InitPSOs();
	InitConstantBuffers();

	m_camera.SetPerspectiveMatrix(
		XMConvertToRadians(60.0f),
		(float)m_displayHeight / (float)m_displayWidth,
		0.001f,
		256.0f);
	m_camera.SetPosition(Math::Vector3(0.0f, 0.0f, 4.0f));

	m_camera.Update();

	m_controller.SetSpeedScale(0.01f);
	m_controller.RefreshFromCamera();
	m_controller.SetCameraMode(CameraMode::ArcBall);

	UpdateConstantBuffers();

	LoadAssets();
}


void TextureCubeMapApp::Shutdown()
{
	m_skyboxRootSig.Destroy();
	m_modelRootSig.Destroy();

	m_skyboxTex.reset();
	m_skyboxModel.reset();
	m_models.clear();
}


bool TextureCubeMapApp::Update()
{
	m_controller.Update(m_frameTimer);

	UpdateConstantBuffers();

	return true;
}


void TextureCubeMapApp::Render()
{
	auto& context = GraphicsContext::Begin("Render frame");

	uint32_t curFrame = m_graphicsDevice->GetCurrentBuffer();

	Color clearColor{ DirectX::Colors::Black };
	context.BeginRenderPass(m_renderPass, *m_framebuffers[curFrame], clearColor, 1.0f, 0);

	context.SetViewportAndScissor(0u, 0u, m_displayWidth, m_displayHeight);

	// Skybox
	{
		context.SetRootSignature(m_skyboxRootSig);
		context.SetPipelineState(m_skyboxPSO);

		context.SetRootConstantBuffer(0, m_vsSkyboxConstantBuffer);
		context.SetSRV(1, 0, *m_skyboxTex);

		context.SetIndexBuffer(m_skyboxModel->GetIndexBuffer());
		context.SetVertexBuffer(0, m_skyboxModel->GetVertexBuffer());

		context.DrawIndexed((uint32_t)m_skyboxModel->GetIndexBuffer().GetElementCount());
	}

	// Model
	{
		auto model = m_models[0];

		context.SetRootSignature(m_modelRootSig);
		context.SetPipelineState(m_modelPSO);

		context.SetRootConstantBuffer(0, m_vsModelConstantBuffer);
		context.SetConstantBuffer(1, 0, m_psConstantBuffer);
		context.SetSRV(1, 1, *m_skyboxTex);

		context.SetIndexBuffer(model->GetIndexBuffer());
		context.SetVertexBuffer(0, model->GetVertexBuffer());

		context.DrawIndexed((uint32_t)model->GetIndexBuffer().GetElementCount());
	}

	context.EndRenderPass();

	context.Finish();
}


void TextureCubeMapApp::InitRootSigs()
{
	m_skyboxRootSig.Reset(2, 1);
	m_skyboxRootSig[0].InitAsConstantBuffer(0, ShaderVisibility::Vertex);
	m_skyboxRootSig[1].InitAsDescriptorRange(DescriptorType::TextureSRV, 0, 1, ShaderVisibility::Pixel);
	m_skyboxRootSig.InitStaticSampler(0, CommonStates::SamplerLinearClamp(), ShaderVisibility::Pixel);
	m_skyboxRootSig.Finalize("Skybox Root Sig", RootSignatureFlags::AllowInputAssemblerInputLayout);

	m_modelRootSig.Reset(2, 1);
	m_modelRootSig[0].InitAsConstantBuffer(0, ShaderVisibility::Vertex);
	m_modelRootSig[1].InitAsDescriptorTable(2, ShaderVisibility::Pixel);
	m_modelRootSig[1].SetTableRange(0, DescriptorType::CBV, 0, 1);
	m_modelRootSig[1].SetTableRange(1, DescriptorType::TextureSRV, 0, 1);
	m_modelRootSig.InitStaticSampler(0, CommonStates::SamplerLinearClamp(), ShaderVisibility::Pixel);
	m_modelRootSig.Finalize("Model Root Sig", RootSignatureFlags::AllowInputAssemblerInputLayout);
}


void TextureCubeMapApp::InitPSOs()
{
	// Skybox
	m_skyboxPSO.SetRootSignature(m_skyboxRootSig);
	m_skyboxPSO.SetRenderPass(m_renderPass);

	m_skyboxPSO.SetBlendState(CommonStates::BlendDisable());
	m_skyboxPSO.SetRasterizerState(CommonStates::RasterizerDefault());
	m_skyboxPSO.SetDepthStencilState(CommonStates::DepthStateDisabled());

	m_skyboxPSO.SetPrimitiveTopology(PrimitiveTopology::TriangleList);

	m_skyboxPSO.SetVertexShader("SkyboxVS");
	m_skyboxPSO.SetPixelShader("SkyboxPS");

	VertexStreamDesc vertexStreamDesc{ 0, sizeof(Vertex), InputClassification::PerVertexData };
	VertexElementDesc vertexElements[] =
	{
		{ "POSITION", 0, Format::R32G32B32_Float, 0, offsetof(Vertex, position), InputClassification::PerVertexData, 0 },
		{ "NORMAL", 0, Format::R32G32B32_Float, 0, offsetof(Vertex, normal), InputClassification::PerVertexData, 0 },
		{ "TEXCOORD", 0, Format::R32G32_Float, 0, offsetof(Vertex, uv), InputClassification::PerVertexData, 0 },
	};

	m_skyboxPSO.SetInputLayout(1, &vertexStreamDesc, _countof(vertexElements), vertexElements);
	m_skyboxPSO.Finalize();

	// Model
	m_modelPSO.SetRootSignature(m_modelRootSig);
	m_modelPSO.SetRenderPass(m_renderPass);

	m_modelPSO.SetBlendState(CommonStates::BlendDisable());
	m_modelPSO.SetRasterizerState(CommonStates::RasterizerDefaultCW());
	m_modelPSO.SetDepthStencilState(CommonStates::DepthStateReadWriteReversed());

	m_modelPSO.SetPrimitiveTopology(PrimitiveTopology::TriangleList);

	m_modelPSO.SetVertexShader("ReflectVS");
	m_modelPSO.SetPixelShader("ReflectPS");

	m_modelPSO.SetInputLayout(1, &vertexStreamDesc, _countof(vertexElements), vertexElements);
	m_modelPSO.Finalize();
}


void TextureCubeMapApp::InitConstantBuffers()
{
	m_vsSkyboxConstantBuffer.Create("VS Constant Buffer", 1, sizeof(VSConstants));
	m_vsModelConstantBuffer.Create("VS Constant Buffer", 1, sizeof(VSConstants));
	m_psConstantBuffer.Create("PS Constant Buffer", 1, sizeof(PSConstants));
}


void TextureCubeMapApp::UpdateConstantBuffers()
{
	using namespace Math;

	Matrix4 modelMatrix = Matrix4(kIdentity);

	Matrix4 viewMatrix = AffineTransform(m_camera.GetRotation(), Vector3(0.0f, 0.0f, 0.0f));
	
	m_vsSkyboxConstants.viewProjectionMatrix = m_camera.GetProjMatrix() * viewMatrix;
	m_vsSkyboxConstants.modelMatrix = modelMatrix;
	m_vsSkyboxConstantBuffer.Update(sizeof(m_vsSkyboxConstants), &m_vsSkyboxConstants);

	m_vsModelConstants.viewProjectionMatrix = m_camera.GetViewProjMatrix();
	m_vsModelConstants.modelMatrix = modelMatrix;
	m_vsModelConstantBuffer.Update(sizeof(m_vsModelConstants), &m_vsModelConstants);

	Matrix4 modelViewMatrix = m_camera.GetViewMatrix() * modelMatrix;
	m_psConstants.invModelViewMatrix = Invert(modelViewMatrix);
	m_psConstants.lodBias = 0.0f;
	m_psConstantBuffer.Update(sizeof(m_psConstants), &m_psConstants);
}


void TextureCubeMapApp::LoadAssets()
{
	m_skyboxTex = Texture::Load("cubemap_yokohama_bc3_unorm.ktx");

	auto layout = VertexLayout(
	{
		VertexComponent::Position,
		VertexComponent::Normal,
		VertexComponent::UV
	});

	m_skyboxModel = Model::Load("cube.obj", layout, 0.05f);

	auto model = Model::Load("sphere.obj", layout, 0.05f);
	m_models.push_back(model);

	model = Model::Load("teapot.dae", layout, 0.05f);
	m_models.push_back(model);

	model = Model::Load("torusknot.obj", layout, 0.05f);
	m_models.push_back(model);

	model = Model::Load("venus.fbx", layout, 0.15f);
	m_models.push_back(model);
}