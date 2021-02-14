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

#include "BloomApp.h"

#include "Graphics\CommandContext.h"
#include "Graphics\CommonStates.h"


using namespace Kodiak;
using namespace Math;
using namespace std;


void BloomApp::Startup()
{
	m_camera.SetPerspectiveMatrix(
		XMConvertToRadians(45.0f),
		(float)m_displayHeight / (float)m_displayWidth,
		0.1f,
		256.0f);
	m_camera.SetPosition(Vector3(0.0f, 0.0f, -10.25f));
	m_camera.SetRotation(Quaternion(XMConvertToRadians(7.5f), XMConvertToRadians(-343.0f), 0.0f));
	m_camera.Update();

	m_timerSpeed = 0.125f;

	m_controller.SetSpeedScale(0.01f);
	m_controller.SetCameraMode(CameraMode::ArcBall);
	m_controller.SetOrbitTarget(Vector3(0.0f, -2.0f, 0.0f), Length(m_camera.GetPosition()), 4.0f);

	InitRootSigs();
	InitPSOs();
	InitFramebuffers();
	InitConstantBuffers();

	LoadAssets();

	InitResourceSets();
}


void BloomApp::Shutdown()
{
	m_ufoModel.reset();
	m_ufoGlowModel.reset();
	m_skyboxModel.reset();
	m_skyboxTex.reset();

	m_offscreenFramebuffer[0].reset();
	m_offscreenFramebuffer[1].reset();

	m_sceneRootSig.Destroy();
	m_blurRootSig.Destroy();
	m_skyboxRootSig.Destroy();
}


bool BloomApp::Update()
{
	m_controller.Update(m_frameTimer, m_mouseMoveHandled);

	UpdateConstantBuffers();

	return true;
}


void BloomApp::UpdateUI()
{
	if (m_uiOverlay->Header("Settings")) 
	{
		m_uiOverlay->CheckBox("Bloom", &m_bloom);
		if (m_uiOverlay->InputFloat("Scale", &m_blurScale, 0.1f, 2))
		{
			UpdateBlurConstants();
		}
	}
}


void BloomApp::Render()
{
	auto& context = GraphicsContext::Begin("Frame");

	// Offscreen color pass

	if (m_bloom)
	{
		context.BeginEvent("Offscreen pass");

		context.TransitionResource(*m_offscreenFramebuffer[0]->GetColorBuffer(0), ResourceState::RenderTarget);
		context.TransitionResource(*m_offscreenFramebuffer[0]->GetDepthBuffer(), ResourceState::DepthWrite);
		context.ClearColor(*m_offscreenFramebuffer[0]->GetColorBuffer(0));
		context.ClearDepth(*m_offscreenFramebuffer[0]->GetDepthBuffer());

		// 3D scene (glow pass)
		{
			ScopedDrawEvent event(context, "Glow pass");

			context.BeginRenderPass(*m_offscreenFramebuffer[0]);

			context.SetViewportAndScissor(0u, 0u, 256, 256);

			context.SetRootSignature(m_sceneRootSig);
			context.SetPipelineState(m_colorPassPSO);

			context.SetResources(m_sceneResources);

			// Render model
			{
				const size_t numMeshes = m_ufoGlowModel->GetNumMeshes();
				for (size_t meshIdx = 0; meshIdx < numMeshes; ++meshIdx)
				{
					auto mesh = m_ufoGlowModel->GetMesh(meshIdx);

					context.SetIndexBuffer(mesh->GetIndexBuffer());
					context.SetVertexBuffer(0, mesh->GetVertexBuffer());

					const auto numParts = mesh->GetNumMeshParts();
					for (size_t partIdx = 0; partIdx < numParts; ++partIdx)
					{
						const auto& meshPart = mesh->GetMeshPart(partIdx);

						context.DrawIndexed(meshPart.indexCount, meshPart.indexBase, meshPart.vertexBase);
					}
				}
			}

			context.EndRenderPass();
		}
		
		// Vertical blur pass
		{
			ScopedDrawEvent event(context, "Vertical blur");

			context.TransitionResource(*m_offscreenFramebuffer[0]->GetColorBuffer(0), ResourceState::PixelShaderResource);
			context.TransitionResource(*m_offscreenFramebuffer[1]->GetColorBuffer(0), ResourceState::RenderTarget);
			context.ClearColor(*m_offscreenFramebuffer[1]->GetColorBuffer(0));

			context.BeginRenderPass(*m_offscreenFramebuffer[1]);

			context.SetRootSignature(m_blurRootSig);
			context.SetPipelineState(m_blurVertPSO);

			context.SetResources(m_blurVertResources);
			context.Draw(3);

			context.EndRenderPass();
		}

		context.EndEvent();
	}
	else
	{
		context.TransitionResource(*m_offscreenFramebuffer[1]->GetColorBuffer(0), ResourceState::RenderTarget);
		context.ClearColor(*m_offscreenFramebuffer[1]->GetColorBuffer(0));
	}

	// Backbuffer color pass

	context.BeginEvent("Color pass");

	context.TransitionResource(GetColorBuffer(), ResourceState::RenderTarget);
	context.TransitionResource(*m_offscreenFramebuffer[1]->GetColorBuffer(0), ResourceState::PixelShaderResource);
	context.TransitionResource(GetDepthBuffer(), ResourceState::DepthWrite);
	context.ClearColor(GetColorBuffer());
	context.ClearDepth(GetDepthBuffer());

	context.BeginRenderPass(GetBackBuffer());

	context.SetViewportAndScissor(0u, 0u, m_displayWidth, m_displayHeight);
	   
	// Skybox
	{
		ScopedDrawEvent event(context, "Skybox");

		context.SetRootSignature(m_skyboxRootSig);
		context.SetPipelineState(m_skyboxPSO);

		context.SetResources(m_skyboxResources);

		// Render model
		{
			const size_t numMeshes = m_skyboxModel->GetNumMeshes();
			for (size_t meshIdx = 0; meshIdx < numMeshes; ++meshIdx)
			{
				auto mesh = m_skyboxModel->GetMesh(meshIdx);

				context.SetIndexBuffer(mesh->GetIndexBuffer());
				context.SetVertexBuffer(0, mesh->GetVertexBuffer());

				const auto numParts = mesh->GetNumMeshParts();
				for (size_t partIdx = 0; partIdx < numParts; ++partIdx)
				{
					const auto& meshPart = mesh->GetMeshPart(partIdx);

					context.DrawIndexed(meshPart.indexCount, meshPart.indexBase, meshPart.vertexBase);
				}
			}
		}
	}

	// 3D scene (phong pass)
	{
		ScopedDrawEvent event(context, "Phong pass");

		context.SetRootSignature(m_sceneRootSig);
		context.SetPipelineState(m_phongPassPSO);

		context.SetResources(m_sceneResources);

		// Render model
		{
			const size_t numMeshes = m_ufoModel->GetNumMeshes();
			for (size_t meshIdx = 0; meshIdx < numMeshes; ++meshIdx)
			{
				auto mesh = m_ufoModel->GetMesh(meshIdx);

				context.SetIndexBuffer(mesh->GetIndexBuffer());
				context.SetVertexBuffer(0, mesh->GetVertexBuffer());

				const auto numParts = mesh->GetNumMeshParts();
				for (size_t partIdx = 0; partIdx < numParts; ++partIdx)
				{
					const auto& meshPart = mesh->GetMeshPart(partIdx);

					context.DrawIndexed(meshPart.indexCount, meshPart.indexBase, meshPart.vertexBase);
				}
			}
		}
	}

	// Horizontal blur pass
	{
		ScopedDrawEvent event(context, "Horizontal blur");

		context.SetRootSignature(m_blurRootSig);
		context.SetPipelineState(m_blurHorizPSO);

		context.SetResources(m_blurHorizResources);
		context.Draw(3);
	}

	RenderUI(context);

	context.EndRenderPass();
	context.TransitionResource(GetColorBuffer(), ResourceState::Present);

	context.Finish();
}

void BloomApp::InitRootSigs()
{
	m_sceneRootSig.Reset(1);
	m_sceneRootSig[0].InitAsDescriptorRange(DescriptorType::CBV, 0, 1, ShaderVisibility::Vertex);
	m_sceneRootSig.Finalize("Color Pass Root Sig", RootSignatureFlags::AllowInputAssemblerInputLayout | RootSignatureFlags::DenyPixelShaderRootAccess);

	m_blurRootSig.Reset(1, 1);
	m_blurRootSig[0].InitAsDescriptorTable(2, ShaderVisibility::Pixel);
	m_blurRootSig[0].SetTableRange(0, DescriptorType::TextureSRV, 0, 1);
	m_blurRootSig[0].SetTableRange(1, DescriptorType::CBV, 0, 1);
	m_blurRootSig.InitStaticSampler(0, CommonStates::SamplerLinearClamp(), ShaderVisibility::Pixel);
	m_blurRootSig.Finalize("Blur Root Sig", RootSignatureFlags::AllowInputAssemblerInputLayout | RootSignatureFlags::DenyVertexShaderRootAccess);

	m_skyboxRootSig.Reset(2, 1);
	m_skyboxRootSig[0].InitAsDescriptorRange(DescriptorType::CBV, 0, 1, ShaderVisibility::Vertex);
	m_skyboxRootSig[1].InitAsDescriptorRange(DescriptorType::TextureSRV, 0, 1, ShaderVisibility::Pixel);
	m_skyboxRootSig.InitStaticSampler(0, CommonStates::SamplerLinearClamp(), ShaderVisibility::Pixel);
	m_skyboxRootSig.Finalize("Skybox RootSig", RootSignatureFlags::AllowInputAssemblerInputLayout);
}


void BloomApp::InitPSOs()
{
	m_colorPassPSO.SetRootSignature(m_sceneRootSig);
	m_colorPassPSO.SetRenderTargetFormat(Format::R8G8B8A8_UNorm, GetDepthFormat());
	m_colorPassPSO.SetPrimitiveTopology(PrimitiveTopology::TriangleList);
	m_colorPassPSO.SetBlendState(CommonStates::BlendDisable());
	m_colorPassPSO.SetDepthStencilState(CommonStates::DepthStateReadWriteReversed());
	m_colorPassPSO.SetRasterizerState(CommonStates::RasterizerTwoSided());
	m_colorPassPSO.SetVertexShader("ColorPassVS");
	m_colorPassPSO.SetPixelShader("ColorPassPS");

	m_blurVertPSO = m_colorPassPSO;
	m_blurVertPSO.SetRootSignature(m_blurRootSig);
	m_blurVertPSO.SetRenderTargetFormat(Format::R8G8B8A8_UNorm, Format::Unknown);
	m_blurVertPSO.SetBlendState(CommonStates::BlendDisable());
	m_blurVertPSO.SetDepthStencilState(CommonStates::DepthStateDisabled());
	m_blurVertPSO.SetVertexShader("GaussBlurVS");
	m_blurVertPSO.SetPixelShader("GaussBlurPS");

	m_blurHorizPSO = m_colorPassPSO;
	m_blurHorizPSO.SetRootSignature(m_blurRootSig);
	m_blurHorizPSO.SetRenderTargetFormat(GetColorFormat(), GetDepthFormat());
	m_blurHorizPSO.SetBlendState(CommonStates::BlendAdditive());
	m_blurHorizPSO.SetDepthStencilState(CommonStates::DepthStateDisabled());
	m_blurHorizPSO.SetVertexShader("GaussBlurVS");
	m_blurHorizPSO.SetPixelShader("GaussBlurPS");

	VertexStreamDesc vertexStream = { 0, sizeof(Vertex), InputClassification::PerVertexData };
	vector<VertexElementDesc> vertexElements =
	{
		{ "POSITION", 0, Format::R32G32B32_Float, 0, offsetof(Vertex, position), InputClassification::PerVertexData, 0 },
		{ "TEXCOORD", 0, Format::R32G32_Float, 0, offsetof(Vertex, uv), InputClassification::PerVertexData, 0 },
		{ "COLOR", 0, Format::R32G32B32_Float, 0, offsetof(Vertex, color), InputClassification::PerVertexData, 0 },
		{ "NORMAL", 0, Format::R32G32B32_Float, 0, offsetof(Vertex, normal), InputClassification::PerVertexData, 0 }
	};
	m_colorPassPSO.SetInputLayout(vertexStream, vertexElements);

	m_phongPassPSO = m_colorPassPSO;
	m_phongPassPSO.SetRenderTargetFormat(GetColorFormat(), GetDepthFormat());
	m_phongPassPSO.SetVertexShader("PhongPassVS");
	m_phongPassPSO.SetPixelShader("PhongPassPS");

	m_skyboxPSO = m_colorPassPSO;
	m_skyboxPSO.SetRootSignature(m_skyboxRootSig);
	m_skyboxPSO.SetRenderTargetFormat(GetColorFormat(), GetDepthFormat());
	m_skyboxPSO.SetDepthStencilState(CommonStates::DepthStateDisabled());
	m_skyboxPSO.SetVertexShader("SkyBoxVS");
	m_skyboxPSO.SetPixelShader("SkyBoxPS");

	m_colorPassPSO.Finalize();
	m_phongPassPSO.Finalize();
	m_blurVertPSO.Finalize();
	m_blurHorizPSO.Finalize();
	m_skyboxPSO.Finalize();
}


void BloomApp::InitFramebuffers()
{
	for (int i = 0; i < 2; ++i) 
	{
		// Framebuffer for offscreen pass
		m_offscreenFramebuffer[i] = make_shared<FrameBuffer>();

		auto colorBuffer = make_shared<ColorBuffer>(DirectX::Colors::Black);
		colorBuffer->Create("Offscreen Color Buffer " + to_string(i), 256, 256, 1, Format::R8G8B8A8_UNorm);

		m_offscreenFramebuffer[i]->SetColorBuffer(0, colorBuffer);

		if (i == 0)
		{
			auto depthBuffer = make_shared<DepthBuffer>(1.0f);
			depthBuffer->Create("Offscreen Depth Buffer " + to_string(i), 256, 256, GetDepthFormat());
			m_offscreenFramebuffer[i]->SetDepthBuffer(depthBuffer);
		}
		
		m_offscreenFramebuffer[i]->Finalize();
	}
}


void BloomApp::InitConstantBuffers()
{
	m_sceneConstantBuffer.Create("Scene Constant Buffer", 1, sizeof(SceneConstants));

	m_skyboxConstantBuffer.Create("Skybox Constant Buffer", 1, sizeof(SceneConstants));

	m_blurHorizConstants.blurScale = m_blurScale;
	m_blurHorizConstants.blurStrength = 1.5f;
	m_blurHorizConstants.blurDirection = 1;

	m_blurVertConstants.blurScale = m_blurScale;
	m_blurVertConstants.blurStrength = 1.5f;
	m_blurVertConstants.blurDirection = 0;

	m_blurHorizConstantBuffer.Create("Blur Horiz Constant Buffer", 1, sizeof(BlurConstants));
	m_blurHorizConstantBuffer.Update(sizeof(BlurConstants), &m_blurHorizConstants);

	m_blurVertConstantBuffer.Create("Blur Vert Constant Buffer", 1, sizeof(BlurConstants));
	m_blurVertConstantBuffer.Update(sizeof(BlurConstants), &m_blurVertConstants);

	UpdateConstantBuffers();
}


void BloomApp::InitResourceSets()
{
	m_sceneResources.Init(&m_sceneRootSig);
	m_sceneResources.SetCBV(0, 0, m_sceneConstantBuffer);
	m_sceneResources.Finalize();

	m_skyboxResources.Init(&m_skyboxRootSig);
	m_skyboxResources.SetCBV(0, 0, m_skyboxConstantBuffer);
	m_skyboxResources.SetSRV(1, 0, *m_skyboxTex);
	m_skyboxResources.Finalize();

	m_blurHorizResources.Init(&m_blurRootSig);
	m_blurHorizResources.SetSRV(0, 0, *m_offscreenFramebuffer[1]->GetColorBuffer(0));
	m_blurHorizResources.SetCBV(0, 1, m_blurHorizConstantBuffer);
	m_blurHorizResources.Finalize();

	m_blurVertResources.Init(&m_blurRootSig);
	m_blurVertResources.SetSRV(0, 0, *m_offscreenFramebuffer[0]->GetColorBuffer(0));
	m_blurVertResources.SetCBV(0, 1, m_blurVertConstantBuffer);
	m_blurVertResources.Finalize();
}


void BloomApp::LoadAssets()
{
	auto layout = VertexLayout(
	{
		VertexComponent::Position,
		VertexComponent::UV,
		VertexComponent::Color,
		VertexComponent::Normal
	});
	m_ufoModel = Model::Load("retroufo.dae", layout, 0.05f);
	m_ufoGlowModel = Model::Load("retroufo_glow.dae", layout, 0.05f);
	m_skyboxModel = Model::Load("cube.obj", layout, 1.0f);
	m_skyboxTex = Texture::Load("cubemap_space.ktx");
}


void BloomApp::UpdateConstantBuffers()
{
	m_sceneConstants.projectionMat = m_camera.GetProjMatrix();
	m_sceneConstants.viewMat = m_camera.GetViewMatrix();
	m_sceneConstants.modelMat = Matrix4(kIdentity);

	AffineTransform modelTrans{ kIdentity };
	modelTrans = AffineTransform::MakeTranslation(Vector3(sinf(XMConvertToRadians(m_timer * 360.0f)) * 0.25f, -2.0f, cosf(XMConvertToRadians(m_timer * 360.0f)) * 0.25f));
	modelTrans = modelTrans * AffineTransform::MakeXRotation(-sinf(XMConvertToRadians(m_timer * 360.0f)) * 0.15f);
	modelTrans = modelTrans * AffineTransform::MakeYRotation(XMConvertToRadians(m_timer * 360.0f));
	m_sceneConstants.modelMat = modelTrans;

	m_skyboxConstants.projectionMat = Matrix4::MakePerspective(XMConvertToRadians(45.0f), (float)GetWidth() / (float)GetHeight(), 0.1f, 256.0f);
	m_skyboxConstants.viewMat = Matrix4(Matrix3(m_sceneConstants.viewMat));
	m_skyboxConstants.modelMat = Matrix4(kIdentity);

	m_sceneConstantBuffer.Update(sizeof(SceneConstants), &m_sceneConstants);
	m_skyboxConstantBuffer.Update(sizeof(SceneConstants), &m_skyboxConstants);
}


void BloomApp::UpdateBlurConstants()
{
	m_blurHorizConstants.blurScale = m_blurScale;
	m_blurVertConstants.blurScale = m_blurScale;

	m_blurHorizConstantBuffer.Update(sizeof(BlurConstants), &m_blurHorizConstants);
	m_blurVertConstantBuffer.Update(sizeof(BlurConstants), &m_blurVertConstants);
}