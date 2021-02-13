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

#include "Graphics\CommandContext.h"
#include "Graphics\CommonStates.h"


using namespace Kodiak;
using namespace Math;
using namespace std;


void TextureCubeMapApp::Startup()
{
	m_camera.SetPerspectiveMatrix(
		XMConvertToRadians(60.0f),
		(float)m_displayHeight / (float)m_displayWidth,
		0.001f,
		256.0f);
	m_camera.SetPosition(Vector3(0.0f, 0.0f, 4.0f));

	m_camera.Update();

	m_controller.SetSpeedScale(0.01f);
	m_controller.RefreshFromCamera();
	m_controller.SetCameraMode(CameraMode::ArcBall);
	m_controller.SetOrbitTarget(Vector3(0.0f, 0.0f, 0.0f), 4.0f, 2.0f);

	InitRootSigs();
	InitPSOs();
	InitConstantBuffers();

	LoadAssets();

	InitResourceSets();
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
	m_controller.Update(m_frameTimer, m_mouseMoveHandled);

	UpdateConstantBuffers();

	return true;
}


void TextureCubeMapApp::UpdateUI()
{
	if (m_uiOverlay->Header("Settings")) 
	{
		m_uiOverlay->SliderFloat("LOD bias", &m_psConstants.lodBias, 0.0f, (float)m_skyboxTex->GetNumMips());
		m_uiOverlay->ComboBox("Object type", &m_curModel, m_modelNames);
		m_uiOverlay->CheckBox("Skybox", &m_displaySkybox);
	}
}


void TextureCubeMapApp::Render()
{
	auto& context = GraphicsContext::Begin("Scene");

	context.TransitionResource(GetColorBuffer(), ResourceState::RenderTarget);
	context.TransitionResource(GetDepthBuffer(), ResourceState::DepthWrite);
	context.ClearColor(GetColorBuffer());
	context.ClearDepth(GetDepthBuffer());

	context.BeginRenderPass(GetBackBuffer());

	context.SetViewportAndScissor(0u, 0u, m_displayWidth, m_displayHeight);

	// Skybox
	if (m_displaySkybox)
	{
		context.SetRootSignature(m_skyboxRootSig);
		context.SetPipelineState(m_skyboxPSO);

		context.SetResources(m_skyboxResources);

		context.SetIndexBuffer(m_skyboxModel->GetIndexBuffer());
		context.SetVertexBuffer(0, m_skyboxModel->GetVertexBuffer());

		context.DrawIndexed((uint32_t)m_skyboxModel->GetIndexBuffer().GetElementCount());
	}

	// Model
	{
		auto model = m_models[m_curModel];

		context.SetRootSignature(m_modelRootSig);
		context.SetPipelineState(m_modelPSO);

		context.SetResources(m_modelResources);

		context.SetIndexBuffer(model->GetIndexBuffer());
		context.SetVertexBuffer(0, model->GetVertexBuffer());

		context.DrawIndexed((uint32_t)model->GetIndexBuffer().GetElementCount());
	}

	RenderUI(context);

	context.EndRenderPass();
	context.TransitionResource(GetColorBuffer(), ResourceState::Present);

	context.Finish();
}


void TextureCubeMapApp::InitRootSigs()
{
	m_skyboxRootSig.Reset(2, 1);
	m_skyboxRootSig[0].InitAsDescriptorRange(DescriptorType::CBV, 0, 1, ShaderVisibility::Vertex);
	m_skyboxRootSig[1].InitAsDescriptorRange(DescriptorType::TextureSRV, 0, 1, ShaderVisibility::Pixel);
	m_skyboxRootSig.InitStaticSampler(0, CommonStates::SamplerLinearClamp(), ShaderVisibility::Pixel);
	m_skyboxRootSig.Finalize("Skybox Root Sig", RootSignatureFlags::AllowInputAssemblerInputLayout);

	m_modelRootSig.Reset(2, 1);
	m_modelRootSig[0].InitAsDescriptorRange(DescriptorType::CBV, 0, 1, ShaderVisibility::Vertex);
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
	m_skyboxPSO.SetRenderTargetFormat(GetColorFormat(), GetDepthFormat());

	m_skyboxPSO.SetBlendState(CommonStates::BlendDisable());
	m_skyboxPSO.SetRasterizerState(CommonStates::RasterizerDefaultCW());
	m_skyboxPSO.SetDepthStencilState(CommonStates::DepthStateDisabled());

	m_skyboxPSO.SetPrimitiveTopology(PrimitiveTopology::TriangleList);

	m_skyboxPSO.SetVertexShader("SkyboxVS");
	m_skyboxPSO.SetPixelShader("SkyboxPS");

	VertexStreamDesc vertexStream{ 0, sizeof(Vertex), InputClassification::PerVertexData };
	vector<VertexElementDesc> vertexElements =
	{
		{ "POSITION", 0, Format::R32G32B32_Float, 0, offsetof(Vertex, position), InputClassification::PerVertexData, 0 },
		{ "NORMAL", 0, Format::R32G32B32_Float, 0, offsetof(Vertex, normal), InputClassification::PerVertexData, 0 },
		{ "TEXCOORD", 0, Format::R32G32_Float, 0, offsetof(Vertex, uv), InputClassification::PerVertexData, 0 },
	};

	m_skyboxPSO.SetInputLayout(vertexStream, vertexElements);
	m_skyboxPSO.Finalize();

	// Model
	m_modelPSO.SetRootSignature(m_modelRootSig);
	m_modelPSO.SetRenderTargetFormat(GetColorFormat(), GetDepthFormat());

	m_modelPSO.SetBlendState(CommonStates::BlendDisable());
	m_modelPSO.SetRasterizerState(CommonStates::RasterizerDefault());
	m_modelPSO.SetDepthStencilState(CommonStates::DepthStateReadWriteReversed());

	m_modelPSO.SetPrimitiveTopology(PrimitiveTopology::TriangleList);

	m_modelPSO.SetVertexShader("ReflectVS");
	m_modelPSO.SetPixelShader("ReflectPS");

	m_modelPSO.SetInputLayout(vertexStream, vertexElements);
	m_modelPSO.Finalize();
}


void TextureCubeMapApp::InitConstantBuffers()
{
	m_vsSkyboxConstantBuffer.Create("VS Constant Buffer", 1, sizeof(VSConstants));
	m_vsModelConstantBuffer.Create("VS Constant Buffer", 1, sizeof(VSConstants));
	m_psConstantBuffer.Create("PS Constant Buffer", 1, sizeof(PSConstants));

	UpdateConstantBuffers();
}


void TextureCubeMapApp::InitResourceSets()
{
	m_modelResources.Init(&m_modelRootSig);
	m_modelResources.SetCBV(0, 0, m_vsModelConstantBuffer);
	m_modelResources.SetCBV(1, 0, m_psConstantBuffer);
	m_modelResources.SetSRV(1, 1, *m_skyboxTex);
	m_modelResources.Finalize();


	m_skyboxResources.Init(&m_skyboxRootSig);
	m_skyboxResources.SetCBV(0, 0, m_vsSkyboxConstantBuffer);
	m_skyboxResources.SetSRV(1, 0, *m_skyboxTex);
	m_skyboxResources.Finalize();
}


void TextureCubeMapApp::UpdateConstantBuffers()
{
	Matrix4 modelMatrix = Matrix4(kIdentity);

	Matrix4 viewMatrix = AffineTransform(m_camera.GetRotation(), Vector3(0.0f, 0.0f, 0.0f));
	
	m_vsSkyboxConstants.viewProjectionMatrix = m_camera.GetProjMatrix() * Invert(viewMatrix);
	m_vsSkyboxConstants.modelMatrix = modelMatrix;
	m_vsSkyboxConstants.eyePos = Vector3(0.0f, 0.0f, 0.0f);
	m_vsSkyboxConstantBuffer.Update(sizeof(m_vsSkyboxConstants), &m_vsSkyboxConstants);

	m_vsModelConstants.viewProjectionMatrix = m_camera.GetViewProjMatrix();
	m_vsModelConstants.modelMatrix = modelMatrix;
	m_vsModelConstants.eyePos = m_camera.GetPosition();
	m_vsModelConstantBuffer.Update(sizeof(m_vsModelConstants), &m_vsModelConstants);

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

	m_modelNames.push_back("Sphere");
	m_modelNames.push_back("Teapot");
	m_modelNames.push_back("Torus knot");
	m_modelNames.push_back("Venus");
}