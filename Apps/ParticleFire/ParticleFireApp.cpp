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

#include "ParticleFireApp.h"

#include "Graphics\CommandContext.h"
#include "Graphics\CommonStates.h"


using namespace Kodiak;
using namespace std;


void ParticleFireApp::Startup()
{
	InitRootSigs();
	InitPSOs();
	InitConstantBuffers();

	m_camera.SetPerspectiveMatrix(
		XMConvertToRadians(60.0f),
		(float)m_displayHeight / (float)m_displayWidth,
		0.001f,
		256.0f);
	m_camera.SetPosition(Math::Vector3(-48.48f, -33.9f, -48.48f));

	m_controller.SetSpeedScale(0.01f);
	m_controller.SetCameraMode(CameraMode::ArcBall);
	m_controller.SetOrbitTarget(Math::Vector3(0.0f, 0.0f, 0.0f), Math::Length(m_camera.GetPosition()), 2.0f);

	UpdateConstantBuffers();

	LoadAssets();

	InitResourceSet();
}


void ParticleFireApp::Shutdown()
{
	m_rootSig.Destroy();

	m_modelColorTex.reset();
	m_modelNormalTex.reset();
	m_particleFireTex.reset();
	m_particleSmokeTex.reset();
	m_model.reset();
}


bool ParticleFireApp::Update()
{
	m_controller.Update(m_frameTimer);

	UpdateConstantBuffers();

	return true;
}


void ParticleFireApp::Render()
{
	auto& context = GraphicsContext::Begin("Scene");

	context.TransitionResource(GetColorBuffer(), ResourceState::RenderTarget);
	context.TransitionResource(GetDepthBuffer(), ResourceState::DepthWrite);
	context.ClearColor(GetColorBuffer());
	context.ClearDepth(GetDepthBuffer());

	context.BeginRenderPass(GetBackBuffer());

	context.SetViewportAndScissor(0u, 0u, m_displayWidth, m_displayHeight);

	// Model
	{
		context.SetRootSignature(m_rootSig);
		context.SetPipelineState(m_modelPSO);
		
		context.SetResources(m_resources);

		m_model->Render(context);
	}

	RenderUI(context);

	context.EndRenderPass();
	context.TransitionResource(GetColorBuffer(), ResourceState::Present);

	context.Finish();
}


void ParticleFireApp::InitRootSigs()
{
	m_rootSig.Reset(2, 1);
	m_rootSig[0].InitAsDescriptorRange(DescriptorType::CBV, 0, 1, ShaderVisibility::Vertex);
	m_rootSig[1].InitAsDescriptorTable(2, ShaderVisibility::Pixel);
	m_rootSig[1].SetTableRange(0, DescriptorType::TextureSRV, 0, 1);
	m_rootSig[1].SetTableRange(1, DescriptorType::TextureSRV, 1, 1);
	m_rootSig.InitStaticSampler(0, CommonStates::SamplerLinearClamp(), ShaderVisibility::Pixel);
	m_rootSig.Finalize("Root Sig", RootSignatureFlags::AllowInputAssemblerInputLayout);
}


void ParticleFireApp::InitPSOs()
{
	// Model
	{
		m_modelPSO.SetRootSignature(m_rootSig);
		m_modelPSO.SetRenderTargetFormat(GetColorFormat(), GetDepthFormat());

		m_modelPSO.SetBlendState(CommonStates::BlendDisable());
		m_modelPSO.SetRasterizerState(CommonStates::RasterizerDefault());
		m_modelPSO.SetDepthStencilState(CommonStates::DepthStateReadWriteReversed());

		m_modelPSO.SetPrimitiveTopology(PrimitiveTopology::TriangleList);

		m_modelPSO.SetVertexShader("NormalMapVS");
		m_modelPSO.SetPixelShader("NormalMapPS");

		VertexStreamDesc vertexStream{ 0, sizeof(ModelVertex), InputClassification::PerVertexData };
		vector<VertexElementDesc> vertexElements =
		{
			{ "POSITION", 0, Format::R32G32B32_Float, 0, offsetof(ModelVertex, pos), InputClassification::PerVertexData, 0 },
			{ "TEXCOORD", 0, Format::R32G32_Float, 0, offsetof(ModelVertex, uv), InputClassification::PerVertexData, 0 },
			{ "NORMAL", 0, Format::R32G32B32_Float, 0, offsetof(ModelVertex, normal), InputClassification::PerVertexData, 0 },
			{ "TANGENT", 0, Format::R32G32B32_Float, 0, offsetof(ModelVertex, tangent), InputClassification::PerVertexData, 0},
			{ "BITANGENT", 0, Format::R32G32B32_Float, 0, offsetof(ModelVertex, bitangent), InputClassification::PerVertexData, 0},
		};

		m_modelPSO.SetInputLayout(vertexStream, vertexElements);
		m_modelPSO.Finalize();
	}
}


void ParticleFireApp::InitConstantBuffers()
{
	m_modelVsConstantBuffer.Create("Model Constant Buffer", 1, sizeof(ModelVSConstants));
}


void ParticleFireApp::InitResourceSet()
{
	m_resources.Init(&m_rootSig);
	m_resources.SetCBV(0, 0, m_modelVsConstantBuffer);
	m_resources.SetSRV(1, 0, *m_modelColorTex);
	m_resources.SetSRV(1, 1, *m_modelNormalTex);
	m_resources.Finalize();
}


void ParticleFireApp::UpdateConstantBuffers()
{
	using namespace Math;
	m_modelVsConstants.modelMatrix = Matrix4(kIdentity);
	m_modelVsConstants.normalMatrix = Transpose(Invert(m_modelVsConstants.modelMatrix));
	m_modelVsConstants.viewMatrix = m_camera.GetViewMatrix();
	m_modelVsConstants.projectionMatrix = m_camera.GetProjMatrix();
	m_modelVsConstants.cameraPos = Vector4(m_camera.GetPosition());

	float x = sinf(m_frameTimer * 2.0f * DirectX::XM_PI) * 1.5f;
	float y = 0.0f;
	float z = cosf(m_frameTimer * 2.0f * DirectX::XM_PI) * 1.5f;
	float w = 1.0f;
	m_modelVsConstants.lightPos = Vector4(x, y, z, w);

	m_modelVsConstantBuffer.Update(sizeof(m_modelVsConstants), &m_modelVsConstants);
}


void ParticleFireApp::LoadAssets()
{
	m_modelColorTex = Texture::Load("fireplace_colormap_bc3_unorm.ktx", Format::Unknown, true);
	m_modelNormalTex = Texture::Load("fireplace_normalmap_bc3_unorm.ktx");
	m_particleFireTex = Texture::Load("particle_fire.ktx", Format::Unknown, true);
	m_particleSmokeTex = Texture::Load("particle_smoke.ktx", Format::Unknown, true);

	auto layout = VertexLayout(
	{
		VertexComponent::Position,
		VertexComponent::UV,
		VertexComponent::Normal,
		VertexComponent::Tangent,
		VertexComponent::Bitangent,
	});

	m_model = Model::Load("fireplace.obj", layout, 10.0f, RemoveFlag(ModelLoad::StandardDefault, ModelLoad::FlipUVs));
}