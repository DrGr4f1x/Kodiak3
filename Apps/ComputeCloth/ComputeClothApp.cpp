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

#include "ComputeClothApp.h"

#include "CommonStates.h"
#include "CommandContext.h"
#include "UIOverlay.h"


using namespace Kodiak;
using namespace Math;
using namespace std;


void ComputeClothApp::Startup()
{
	m_camera.SetPerspectiveMatrix(
		XMConvertToRadians(60.0f),
		(float)m_displayHeight / (float)m_displayWidth,
		0.1f,
		512.0f);
	m_camera.SetPosition(Math::Vector3(-2.0f, -2.0f, -2.0f));

	m_controller.SetSpeedScale(0.01f);
	m_controller.SetCameraMode(CameraMode::ArcBall);
	m_controller.SetOrbitTarget(Vector3(0.0f, 0.0f, 0.0f), Length(m_camera.GetPosition()), 2.0f);

	InitRootSigs();
	InitPSOs();
	InitConstantBuffers();
	InitCloth();

	LoadAssets();

	InitResourceSets();
}


void ComputeClothApp::Shutdown()
{
	m_sphereRootSig.Destroy();
	m_clothRootSig.Destroy();
	m_computeRootSig.Destroy();
}


bool ComputeClothApp::Update()
{
	m_controller.Update(m_frameTimer, m_mouseMoveHandled);

	UpdateConstantBuffers();

	return true;
}


void ComputeClothApp::UpdateUI()
{
	if (m_uiOverlay->Header("Settings")) 
	{
		m_uiOverlay->CheckBox("Simulate wind", &m_simulateWind);
	}
}


void ComputeClothApp::Render()
{
	auto& context = GraphicsContext::Begin("Scene");

	// Cloth simulation
	{
		auto& computeContext = context.GetComputeContext();

		computeContext.TransitionResource(m_clothBuffer[0], ResourceState::NonPixelShaderResource);
		computeContext.TransitionResource(m_clothBuffer[1], ResourceState::NonPixelShaderResource);

		computeContext.SetRootSignature(m_computeRootSig);
		computeContext.SetPipelineState(m_computePSO);

		const uint32_t iterations = 64;
		for (uint32_t j = 0; j < iterations; ++j)
		{
			m_readSet = 1 - m_readSet;
			if (j == iterations - 1)
				computeContext.SetResources(m_computeNormalResources);
			else
				computeContext.SetResources(m_computeResources[m_readSet]);
			computeContext.Dispatch2D(m_gridSize[0], m_gridSize[1]);
		}
	}

	context.TransitionResource(GetColorBuffer(), ResourceState::RenderTarget);
	context.TransitionResource(GetDepthBuffer(), ResourceState::DepthWrite);
	context.TransitionResource(m_clothBuffer[0], ResourceState::VertexBuffer);
	Color clearColor{ DirectX::Colors::LightGray };
	context.ClearColor(GetColorBuffer(), clearColor);
	context.ClearDepth(GetDepthBuffer());

	context.BeginRenderPass(GetBackBuffer());

	context.SetViewportAndScissor(0u, 0u, m_displayWidth, m_displayHeight);

	// Sphere
	context.SetRootSignature(m_sphereRootSig);
	context.SetPipelineState(m_spherePSO);

	context.SetResources(m_sphereResources);

	context.SetIndexBuffer(m_sphereModel->GetIndexBuffer());
	context.SetVertexBuffer(0, m_sphereModel->GetVertexBuffer());

	context.DrawIndexed((uint32_t)m_sphereModel->GetIndexBuffer().GetElementCount());

	// Cloth
	context.SetRootSignature(m_clothRootSig);
	context.SetPipelineState(m_clothPSO);

	context.SetResources(m_clothResources);

	context.SetIndexBuffer(m_clothIndexBuffer);
	context.SetVertexBuffer(0, m_clothBuffer[0]);

	context.DrawIndexed((uint32_t)m_clothIndexBuffer.GetElementCount());

	RenderUI(context);

	context.EndRenderPass();
	context.TransitionResource(GetColorBuffer(), ResourceState::Present);

	context.Finish();
}


void ComputeClothApp::InitRootSigs()
{
	m_sphereRootSig.Reset(1);
	m_sphereRootSig[0].InitAsDescriptorRange(DescriptorType::CBV, 0, 1, ShaderVisibility::Vertex);
	m_sphereRootSig.Finalize("Sphere Root Sig", RootSignatureFlags::AllowInputAssemblerInputLayout | RootSignatureFlags::DenyPixelShaderRootAccess);

	m_clothRootSig.Reset(2, 1);
	m_clothRootSig[0].InitAsDescriptorRange(DescriptorType::CBV, 0, 1, ShaderVisibility::Vertex);
	m_clothRootSig[1].InitAsDescriptorRange(DescriptorType::TextureSRV, 0, 1, ShaderVisibility::Pixel);
	m_clothRootSig.InitStaticSampler(0, CommonStates::SamplerLinearClamp(), ShaderVisibility::Pixel);
	m_clothRootSig.Finalize("Cloth Root Sig", RootSignatureFlags::AllowInputAssemblerInputLayout);

	m_computeRootSig.Reset(1);
	m_computeRootSig[0].InitAsDescriptorTable(3, ShaderVisibility::Compute);
	m_computeRootSig[0].SetTableRange(0, DescriptorType::StructuredBufferSRV, 0, 1);
	m_computeRootSig[0].SetTableRange(1, DescriptorType::StructuredBufferUAV, 0, 1);
	m_computeRootSig[0].SetTableRange(2, DescriptorType::CBV, 0, 1);
	m_computeRootSig.Finalize("Compute Root Sig");
}


void ComputeClothApp::InitPSOs()
{
	m_spherePSO.SetRootSignature(m_sphereRootSig);
	m_spherePSO.SetBlendState(CommonStates::BlendDisable());
	m_spherePSO.SetDepthStencilState(CommonStates::DepthStateReadWriteReversed());
	m_spherePSO.SetRasterizerState(CommonStates::RasterizerDefault());
	m_spherePSO.SetRenderTargetFormat(GetColorFormat(), GetDepthFormat());
	m_spherePSO.SetPrimitiveTopology(PrimitiveTopology::TriangleList);

	m_clothPSO = m_spherePSO;

	VertexStreamDesc sphereVtxStream{ 0, sizeof(SphereVertex), InputClassification::PerVertexData };
	vector<VertexElementDesc> sphereVtxElements =
	{
		{ "POSITION", 0, Format::R32G32B32_Float, 0, offsetof(SphereVertex, position), InputClassification::PerVertexData, 0 },
		{ "NORMAL", 0, Format::R32G32B32_Float, 0, offsetof(SphereVertex, normal), InputClassification::PerVertexData, 0 },
	};
	m_spherePSO.SetInputLayout(sphereVtxStream, sphereVtxElements);

	m_spherePSO.SetVertexShader("SphereVS");
	m_spherePSO.SetPixelShader("SpherePS");

	m_clothPSO.SetRootSignature(m_clothRootSig);
	m_clothPSO.SetPrimitiveTopology(PrimitiveTopology::TriangleStrip);
	m_clothPSO.SetPrimitiveRestart(IndexBufferStripCutValue::Value_0xFFFFFFFF);

	VertexStreamDesc clothVtxStream{ 0, sizeof(Particle), InputClassification::PerVertexData };
	vector<VertexElementDesc> clothVtxElements =
	{
		{ "POSITION", 0, Format::R32G32B32_Float, 0, offsetof(Particle, pos), InputClassification::PerVertexData, 0 },
		{ "TEXCOORD", 0, Format::R32G32_Float, 0, offsetof(Particle, uv), InputClassification::PerVertexData, 0 },
		{ "NORMAL", 0, Format::R32G32B32_Float, 0, offsetof(Particle, normal), InputClassification::PerVertexData, 0 },

	};
	m_clothPSO.SetInputLayout(clothVtxStream, clothVtxElements);

	m_clothPSO.SetVertexShader("ClothVS");
	m_clothPSO.SetPixelShader("ClothPS");

	m_spherePSO.Finalize();
	m_clothPSO.Finalize();

	m_computePSO.SetRootSignature(m_computeRootSig);
	m_computePSO.SetComputeShader("ClothCS");
	m_computePSO.Finalize();
}


void ComputeClothApp::InitConstantBuffers()
{
	m_vsConstants.lightPos = Vector4(2.0f, 4.0f, 2.0f, 1.0f);
	m_vsConstantBuffer.Create("VS Constant Buffer", 1, sizeof(VSConstants));

	float dx = m_size[0] / (m_gridSize[0] - 1);
	float dy = m_size[1] / (m_gridSize[1] - 1);

	m_csConstants.deltaT = 0.0f;
	m_csConstants.particleMass = 0.1f;
	m_csConstants.springStiffness = 2000.0f;
	m_csConstants.damping = 0.25;
	m_csConstants.restDistH = dx;
	m_csConstants.restDistV = dy;
	m_csConstants.restDistD = sqrtf(dx * dx + dy * dy);
	m_csConstants.sphereRadius = m_sphereRadius;
	m_csConstants.spherePos = m_pinnedCloth ? Vector4(0.0f, 0.0f, -10.0f, 0.0f) : Vector4(0.0f);
	m_csConstants.gravity = Vector4(0.0f, -9.8f, 0.0f, 0.0f);
	m_csConstants.particleCount[0] = m_gridSize[0];
	m_csConstants.particleCount[1] = m_gridSize[1];
	m_csConstants.calculateNormals = 0;

	m_csConstantBuffer.Create("CS Constant Buffer", 1, sizeof(CSConstants), &m_csConstants);
	m_csConstants.calculateNormals = 1;
	m_csNormalConstantBuffer.Create("CS Constant Buffer", 1, sizeof(CSConstants), &m_csConstants);
}


void ComputeClothApp::InitCloth()
{
	const uint32_t numParticles = m_gridSize[0] * m_gridSize[1];
	vector<Particle> particles(m_gridSize[0] * m_gridSize[1]);

	float dx = m_size[0] / (m_gridSize[0] - 1);
	float dy = m_size[1] / (m_gridSize[1] - 1);
	float du = 1.0f / (m_gridSize[0] - 1);
	float dv = 1.0f / (m_gridSize[1] - 1);

	if (m_pinnedCloth)
	{
		Matrix4 transMat = AffineTransform::MakeTranslation(Vector3(-m_size[0] / 2.0f, -m_size[1] / 2.0f, 0.0f));
		for (uint32_t i = 0; i < m_gridSize[1]; i++)
		{
			for (uint32_t j = 0; j < m_gridSize[0]; j++)
			{
				auto& particle = particles[i + j * m_gridSize[1]];
				particle.pos = transMat * Vector4(dx * j, dy * i, 0.0f, 1.0f);
				particle.vel = Vector4(0.0f);
				particle.uv = Vector4(du * j, dv * i, 0.0f, 0.0f);
				particle.normal = Vector4(0.0f, 1.0f, 0.0f, 0.0f);
				float p = (i == 0) && ((j == 0) || (j == m_gridSize[0] / 3) || (j == m_gridSize[0] - m_gridSize[0] / 3) || (j == m_gridSize[0] - 1));
				particle.pinned = Vector4(p, 0.0f, 0.0f, 0.0f);
			}
		}
	}
	else
	{
		Matrix4 transMat = AffineTransform::MakeTranslation(Vector3(-m_size[0] / 2.0f, 2.0f, -m_size[1] / 2.0f));
		for (uint32_t i = 0; i < m_gridSize[1]; i++) 
		{
			for (uint32_t j = 0; j < m_gridSize[0]; j++) 
			{
				auto& particle = particles[i + j * m_gridSize[1]];
				particle.pos = transMat * Vector4(dx * j, 0.0f, dy * i, 1.0f);
				particle.vel = Vector4(0.0f);
				particle.uv = Vector4(1.0f - du * i, dv * j, 0.0f, 0.0f);
				particle.normal = Vector4(0.0f, 1.0f, 0.0f, 0.0f);
				particle.pinned = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
			}
		}
	}

	m_clothBuffer[0].CreateWithFlags("Cloth Buffer 0", numParticles, sizeof(Particle), ResourceType::VertexBuffer, particles.data());
	m_clothBuffer[1].CreateWithFlags("Cloth Buffer 1", numParticles, sizeof(Particle), ResourceType::VertexBuffer, particles.data());

	// Indices
	vector<uint32_t> indices;
	for (uint32_t y = 0; y < m_gridSize[1] - 1; y++) 
	{
		for (uint32_t x = 0; x < m_gridSize[0]; x++) 
		{
			indices.push_back((y + 1) *  m_gridSize[0] + x);
			indices.push_back((y)*  m_gridSize[0] + x);
		}
		// Primitive restart (signaled by special value 0xFFFFFFFF)
		indices.push_back(0xFFFFFFFF);
	}

	m_clothIndexBuffer.Create("Cloth Index Buffer", indices.size(), sizeof(uint32_t), false, indices.data());
}


void ComputeClothApp::InitResourceSets()
{
	m_sphereResources.Init(&m_sphereRootSig);
	m_sphereResources.SetCBV(0, 0, m_vsConstantBuffer);
	m_sphereResources.Finalize();

	m_clothResources.Init(&m_clothRootSig);
	m_clothResources.SetCBV(0, 0, m_vsConstantBuffer);
	m_clothResources.SetSRV(1, 0, *m_texture);
	m_clothResources.Finalize();

	m_computeResources[0].Init(&m_computeRootSig);
	m_computeResources[0].SetSRV(0, 0, m_clothBuffer[0]);
	m_computeResources[0].SetUAV(0, 1, m_clothBuffer[1]);
	m_computeResources[0].SetCBV(0, 2, m_csConstantBuffer);
	m_computeResources[0].Finalize();

	m_computeResources[1].Init(&m_computeRootSig);
	m_computeResources[1].SetSRV(0, 0, m_clothBuffer[1]);
	m_computeResources[1].SetUAV(0, 1, m_clothBuffer[0]);
	m_computeResources[1].SetCBV(0, 2, m_csConstantBuffer);
	m_computeResources[1].Finalize();

	m_computeNormalResources.Init(&m_computeRootSig);
	m_computeNormalResources.SetSRV(0, 0, m_clothBuffer[1]);
	m_computeNormalResources.SetUAV(0, 1, m_clothBuffer[0]);
	m_computeNormalResources.SetCBV(0, 2, m_csNormalConstantBuffer);
	m_computeNormalResources.Finalize();
}


void ComputeClothApp::LoadAssets()
{
	auto layout = VertexLayout(
	{
		VertexComponent::Position,
		VertexComponent::Normal
	});
	m_sphereModel = Model::Load("geosphere.obj", layout, m_sphereRadius * 0.05f);

#if DX12
	m_texture = Texture::Load("XII_BLACK_1kx1k.jpg");
#else
	m_texture = Texture::Load("vulkan_cloth_rgba.ktx");
#endif
}


void ComputeClothApp::UpdateConstantBuffers()
{
	m_vsConstants.projectionMatrix = m_camera.GetProjMatrix();
	m_vsConstants.modelViewMatrix = m_camera.GetViewMatrix();
	m_vsConstantBuffer.Update(sizeof(VSConstants), &m_vsConstants);

	m_csConstants.deltaT = m_frameTimer / 64.0f;
	if (m_simulateWind)
	{
		float scale = (1.0f + g_rng.NextFloat(5.0f)) - (1.0f + g_rng.NextFloat(5.0f));
		m_csConstants.gravity.SetX(cosf(XMConvertToRadians(m_timer * 360.0f)) * scale);
		scale = (1.0f + g_rng.NextFloat(5.0f)) - (1.0f + g_rng.NextFloat(5.0f));
		m_csConstants.gravity.SetZ(sinf(XMConvertToRadians(m_timer * 360.0f)) * scale);
	}
	else
	{
		m_csConstants.gravity.SetX(0.0f);
		m_csConstants.gravity.SetZ(0.0f);
	}
	
	m_csConstants.calculateNormals = 0;
	m_csConstantBuffer.Update(sizeof(CSConstants), &m_csConstants);
	m_csConstants.calculateNormals = 1;
	m_csNormalConstantBuffer.Update(sizeof(CSConstants), &m_csConstants);
}