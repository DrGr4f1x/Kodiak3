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

#include "RadialBlurApp.h"

#include "CommandContext.h"
#include "CommonStates.h"
#include "Filesystem.h"
#include "GraphicsDevice.h"


using namespace Kodiak;
using namespace std;


void RadialBlurApp::Configure()
{
	// Setup file system
	auto& filesystem = Kodiak::Filesystem::GetInstance();

	filesystem.SetDefaultRootDir();
	filesystem.AddSearchPath("Data\\" + GetDefaultShaderPath());
	filesystem.AddSearchPath("Data\\Textures");
	filesystem.AddSearchPath("Data\\Models");
}


void RadialBlurApp::Startup()
{
	m_timerSpeed = 0.125f;

	InitRenderPasses();
	InitRootSigs();
	InitPSOs();
	InitFramebuffers();
	InitConstantBuffers();

	LoadAssets();
}


void RadialBlurApp::Shutdown()
{
	m_model.reset();
	m_gradientTex.reset();

	m_offscreenRenderPass.Destroy();

	m_offscreenFramebuffer.reset();

	m_sceneConstantBuffer.Destroy();

	m_radialBlurRootSig.Destroy();
	m_sceneRootSig.Destroy();
}


bool RadialBlurApp::Update()
{
	bool res = Application::Update();

	if (res)
	{
		UpdateConstantBuffers();
	}

	return res;
}


void RadialBlurApp::Render()
{
	// Offscreen pass [offscreen render target]
	{
		auto& context = GraphicsContext::Begin("Offscreen");

		Color clearColor{ DirectX::Colors::Black };
		context.BeginRenderPass(m_offscreenRenderPass, *m_offscreenFramebuffer, clearColor, 1.0f, 0);

		context.SetViewportAndScissor(0u, 0u, s_offscreenSize, s_offscreenSize);

		context.SetRootSignature(m_sceneRootSig);
		context.SetPipelineState(m_colorPassPSO);

		// Draw the model
		context.SetRootConstantBuffer(0, m_sceneConstantBuffer);
		context.SetSRV(1, 0, *m_gradientTex);

		context.SetVertexBuffer(0, m_model->GetVertexBuffer());
		context.SetIndexBuffer(m_model->GetIndexBuffer());
		context.DrawIndexed((uint32_t)m_model->GetIndexBuffer().GetElementCount());

		context.EndRenderPass();

		context.Finish();
	}

	// Main pass [backbuffer]
	{
		auto& context = GraphicsContext::Begin("Render frame");

		uint32_t curFrame = m_graphicsDevice->GetCurrentBuffer();

		Color clearColor{ DirectX::Colors::Black };
		context.BeginRenderPass(m_defaultRenderPass, *m_defaultFramebuffers[curFrame], clearColor, 1.0f, 0);

		context.SetViewportAndScissor(0u, 0u, m_displayWidth, m_displayHeight);

		context.SetRootSignature(m_sceneRootSig);
		context.SetPipelineState(m_phongPassPSO);

		// Draw the model
		context.SetRootConstantBuffer(0, m_sceneConstantBuffer);
		context.SetSRV(1, 0, *m_gradientTex);

		context.SetVertexBuffer(0, m_model->GetVertexBuffer());
		context.SetIndexBuffer(m_model->GetIndexBuffer());
		context.DrawIndexed((uint32_t)m_model->GetIndexBuffer().GetElementCount());

		if (m_blur)
		{
			context.SetRootSignature(m_radialBlurRootSig);
			context.SetPipelineState(m_radialBlurPSO);
			
			context.SetSRV(0, 0, *m_offscreenFramebuffer->GetColorBuffer(0));
			context.SetConstantBuffer(0, 1, m_radialBlurConstantBuffer);

			context.Draw(3);
		}

		context.EndRenderPass();

		context.Finish();
	}
}


void RadialBlurApp::InitRenderPasses()
{
	auto colorFormat = Format::R8G8B8A8_UNorm;
	auto depthFormat = m_graphicsDevice->GetDepthFormat();
		
	m_offscreenRenderPass.SetColorAttachment(0, colorFormat, ResourceState::Undefined, ResourceState::PixelShaderResource);
	m_offscreenRenderPass.SetDepthAttachment(depthFormat, ResourceState::Undefined, ResourceState::DepthWrite);
	m_offscreenRenderPass.Finalize();
}


void RadialBlurApp::InitRootSigs()
{
	m_sceneRootSig.Reset(2, 1);
	m_sceneRootSig[0].InitAsConstantBuffer(0, ShaderVisibility::Vertex);
	m_sceneRootSig[1].InitAsDescriptorTable(2, ShaderVisibility::Pixel);
	m_sceneRootSig[1].SetTableRange(0, DescriptorType::TextureSRV, 0, 1);
	m_sceneRootSig[1].SetTableRange(1, DescriptorType::CBV, 0, 1);
	m_sceneRootSig.InitStaticSampler(0, CommonStates::SamplerLinearWrap(), ShaderVisibility::Pixel);
	m_sceneRootSig.Finalize("Scene Root Sig", RootSignatureFlags::AllowInputAssemblerInputLayout);

	m_radialBlurRootSig.Reset(1, 1);
	m_radialBlurRootSig[0].InitAsDescriptorTable(2, ShaderVisibility::Pixel);
	m_radialBlurRootSig[0].SetTableRange(0, DescriptorType::TextureSRV, 0, 1);
	m_radialBlurRootSig[0].SetTableRange(1, DescriptorType::CBV, 0, 1);
	m_radialBlurRootSig.InitStaticSampler(0, CommonStates::SamplerLinearClamp(), ShaderVisibility::Pixel);
	m_radialBlurRootSig.Finalize("Radial Blur Root Sig", RootSignatureFlags::None);
}


void RadialBlurApp::InitPSOs()
{
	m_radialBlurPSO.SetRootSignature(m_radialBlurRootSig);
	m_radialBlurPSO.SetRenderPass(m_defaultRenderPass);
	m_radialBlurPSO.SetPrimitiveTopology(PrimitiveTopology::TriangleList);
	m_radialBlurPSO.SetBlendState(CommonStates::BlendAdditive());
	m_radialBlurPSO.SetDepthStencilState(CommonStates::DepthStateReadWriteReversed());
	m_radialBlurPSO.SetRasterizerState(CommonStates::RasterizerTwoSided());
	m_radialBlurPSO.SetVertexShader("RadialBlurVS");
	m_radialBlurPSO.SetPixelShader("RadialBlurPS");

	m_offscreenDisplayPSO = m_radialBlurPSO;
	m_offscreenDisplayPSO.SetBlendState(CommonStates::BlendDisable());

	m_phongPassPSO = m_offscreenDisplayPSO;
	m_phongPassPSO.SetRootSignature(m_sceneRootSig);
	m_phongPassPSO.SetVertexShader("PhongPassVS");
	m_phongPassPSO.SetPixelShader("PhongPassPS");

	VertexStreamDesc vertexStreamDesc{ 0, sizeof(Vertex), InputClassification::PerVertexData };
	VertexElementDesc vertexElements[] =
	{
		{ "POSITION", 0, Format::R32G32B32_Float, 0, offsetof(Vertex, position), InputClassification::PerVertexData, 0 },
		{ "TEXCOORD", 0, Format::R32G32_Float, 0, offsetof(Vertex, uv), InputClassification::PerVertexData, 0 },
		{ "COLOR", 0, Format::R32G32B32_Float, 0, offsetof(Vertex, color), InputClassification::PerVertexData, 0 },
		{ "NORMAL", 0, Format::R32G32B32_Float, 0, offsetof(Vertex, normal), InputClassification::PerVertexData, 0 }
	};
	m_phongPassPSO.SetInputLayout(1, &vertexStreamDesc, _countof(vertexElements), vertexElements);

	m_colorPassPSO = m_phongPassPSO;
	m_colorPassPSO.SetRenderPass(m_offscreenRenderPass);
	m_colorPassPSO.SetVertexShader("ColorPassVS");
	m_colorPassPSO.SetPixelShader("ColorPassPS");

	m_radialBlurPSO.Finalize();
	m_offscreenDisplayPSO.Finalize();
	m_phongPassPSO.Finalize();
	m_colorPassPSO.Finalize();
}


void RadialBlurApp::InitFramebuffers()
{
	// Framebuffer for offscreen pass
	{
		auto colorBuffer = make_shared<ColorBuffer>(DirectX::Colors::Black);
		colorBuffer->Create("Offscreen Color Buffer", s_offscreenSize, s_offscreenSize, 1, Format::R8G8B8A8_UNorm);

		auto depthBuffer = make_shared<DepthBuffer>(1.0f);
		depthBuffer->Create("Offscreen Depth Buffer", s_offscreenSize, s_offscreenSize, m_graphicsDevice->GetDepthFormat());

		m_offscreenFramebuffer = make_shared<FrameBuffer>();
		m_offscreenFramebuffer->SetColorBuffer(0, colorBuffer);
		m_offscreenFramebuffer->SetDepthBuffer(depthBuffer);
		m_offscreenFramebuffer->Finalize(m_offscreenRenderPass);
	}
}


void RadialBlurApp::InitConstantBuffers()
{
	m_sceneConstantBuffer.Create("Scene Constant Buffer", 1, sizeof(SceneConstants));

	m_radialBlurConstantBuffer.Create("Radial Blur Constant Buffer", 1, sizeof(RadialBlurConstants));
	m_radialBlurConstantBuffer.Update(sizeof(m_radialBlurConstants), &m_radialBlurConstants);
}


void RadialBlurApp::LoadAssets()
{
	auto layout = VertexLayout(
	{ 
		VertexComponent::Position,
		VertexComponent::UV,
		VertexComponent::Color,
		VertexComponent::Normal
	});
	m_model = Model::Load("glowsphere.dae", layout, 0.05f);
	m_gradientTex = Texture::Load("particle_gradient_rgba.ktx");
}


void RadialBlurApp::UpdateConstantBuffers()
{
	using namespace Math;

	m_sceneConstants.projectionMat = Matrix4::MakePerspective(
		DirectX::XMConvertToRadians(45.0f),
		(float)m_displayWidth / (float)m_displayHeight,
		0.1f,
		256.0f);

	Matrix4 viewMatrix = AffineTransform::MakeTranslation(Vector3(0.0f, 0.0f, m_zoom));

	auto rotation = AffineTransform::MakeXRotation(DirectX::XMConvertToRadians(m_rotation.GetX()));
	rotation = rotation * AffineTransform::MakeYRotation(DirectX::XMConvertToRadians(m_rotation.GetY()));
	rotation = rotation * AffineTransform::MakeYRotation(DirectX::XMConvertToRadians(m_timer * 360.0f));
	rotation = rotation * AffineTransform::MakeZRotation(DirectX::XMConvertToRadians(m_rotation.GetZ()));

	m_sceneConstants.modelMat = Matrix4(kIdentity);
	m_sceneConstants.modelMat = Matrix4(AffineTransform::MakeTranslation(m_cameraPos)) * viewMatrix * rotation;

	if (!m_paused)
	{
		m_sceneConstants.gradientPos += m_frameTimer * 0.1f;
	}

	m_sceneConstantBuffer.Update(sizeof(m_sceneConstants), &m_sceneConstants);
}