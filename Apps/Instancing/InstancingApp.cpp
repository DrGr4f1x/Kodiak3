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

#include "InstancingApp.h"

#include "CommandContext.h"
#include "CommonStates.h"


using namespace Kodiak;
using namespace std;


void InstancingApp::Startup()
{
	using namespace Math;

	m_camera.SetPerspectiveMatrix(
		DirectX::XMConvertToRadians(60.0f),
		(float)m_displayHeight / (float)m_displayWidth,
		0.1f,
		256.0f);
	m_camera.SetPosition(Vector3(kZero));
	m_camera.Update();

	m_controller.SetSpeedScale(0.01f);
	m_controller.SetCameraMode(CameraMode::ArcBall);
	m_controller.SetOrbitTarget(Vector3(0.0f, 0.0f, 0.0f), Length(m_camera.GetPosition()), 0.25f);

	InitRootSigs();
	InitPSOs();
	InitConstantBuffer();
	
	// Load assets first, since instancing data needs to know how many array slices are in
	// the rock texture
	LoadAssets();
	InitInstanceBuffer();

	InitResourceSets();
}


void InstancingApp::Shutdown()
{
	m_starfieldRootSig.Destroy();
	m_modelRootSig.Destroy();
}


bool InstancingApp::Update()
{
	m_controller.Update(m_frameTimer);

	UpdateConstantBuffer();

	return true;
}


void InstancingApp::Render()
{
	auto& context = GraphicsContext::Begin("Scene");

	context.TransitionResource(GetColorBuffer(), ResourceState::RenderTarget);
	context.TransitionResource(GetDepthBuffer(), ResourceState::DepthWrite);
	context.ClearColor(GetColorBuffer());
	context.ClearDepthAndStencil(GetDepthBuffer());

	context.BeginRenderPass(GetBackBuffer());

	context.SetViewportAndScissor(0u, 0u, m_displayWidth, m_displayHeight);

	// Starfield
	{
		context.SetRootSignature(m_starfieldRootSig);
		context.SetPipelineState(m_starfieldPSO);

		context.Draw(4);
	}

	// Planet
	{
		context.SetRootSignature(m_modelRootSig);
		context.SetPipelineState(m_planetPSO);

		context.SetResources(m_planetResources);

		context.SetIndexBuffer(m_planetModel->GetIndexBuffer());
		context.SetVertexBuffer(0, m_planetModel->GetVertexBuffer());

		context.DrawIndexed((uint32_t)m_planetModel->GetIndexBuffer().GetElementCount());
	}

	// Rocks
	{
		context.SetPipelineState(m_rockPSO);

		context.SetResources(m_rockResources);

		context.SetIndexBuffer(m_rockModel->GetIndexBuffer());
		context.SetVertexBuffer(0, m_rockModel->GetVertexBuffer());
		context.SetVertexBuffer(1, m_instanceBuffer);

		context.DrawIndexedInstanced((uint32_t)m_rockModel->GetIndexBuffer().GetElementCount(), m_numInstances, 0, 0, 0);
	}

	context.EndRenderPass();

	context.Finish();
}


void InstancingApp::InitRootSigs()
{
	m_starfieldRootSig.Reset(0);
	m_starfieldRootSig.Finalize(
		"Starfield RootSig",
		RootSignatureFlags::AllowInputAssemblerInputLayout | 
		RootSignatureFlags::DenyPixelShaderRootAccess);

	m_modelRootSig.Reset(2, 1);
	m_modelRootSig[0].InitAsDescriptorRange(DescriptorType::CBV, 0, 1, ShaderVisibility::Vertex);
	m_modelRootSig[1].InitAsDescriptorRange(DescriptorType::TextureSRV, 0, 1, ShaderVisibility::Pixel);
	m_modelRootSig.InitStaticSampler(0, CommonStates::SamplerLinearWrap(), ShaderVisibility::Pixel);
	m_modelRootSig.Finalize("Model RootSig", RootSignatureFlags::AllowInputAssemblerInputLayout);
}


void InstancingApp::InitPSOs()
{
	// Starfield
	{
		m_starfieldPSO.SetRootSignature(m_starfieldRootSig);
		m_starfieldPSO.SetBlendState(CommonStates::BlendDisable());
		m_starfieldPSO.SetRasterizerState(CommonStates::RasterizerDefault());
		m_starfieldPSO.SetDepthStencilState(CommonStates::DepthStateDisabled());
		m_starfieldPSO.SetPrimitiveTopology(PrimitiveTopology::TriangleList);

		m_starfieldPSO.SetRenderTargetFormat(GetColorFormat(), GetDepthFormat());

		m_starfieldPSO.SetVertexShader("StarfieldVS");
		m_starfieldPSO.SetPixelShader("StarfieldPS");

		m_starfieldPSO.Finalize();
	}

	// Instanced Rock
	{
		m_rockPSO.SetRootSignature(m_modelRootSig);
		m_rockPSO.SetBlendState(CommonStates::BlendDisable());
		m_rockPSO.SetRasterizerState(CommonStates::RasterizerDefaultCW());
		m_rockPSO.SetDepthStencilState(CommonStates::DepthStateReadWriteReversed());
		m_rockPSO.SetPrimitiveTopology(PrimitiveTopology::TriangleList);

		m_rockPSO.SetRenderTargetFormat(GetColorFormat(), GetDepthFormat());

		m_rockPSO.SetVertexShader("StarfieldVS");
		m_rockPSO.SetPixelShader("StarfieldPS");

		// Vertex inputs
		vector<VertexStreamDesc> vertexStreams = {
			{ 0, 11 * sizeof(float), InputClassification::PerVertexData },
			{ 1, 8 * sizeof(float), InputClassification::PerInstanceData }
		};
		vector<VertexElementDesc> vertexElements =
		{
			{ "POSITION", 0, Format::R32G32B32_Float, 0, 0, InputClassification::PerVertexData, 0 },
			{ "NORMAL", 0, Format::R32G32B32_Float, 0, 3 * sizeof(float), InputClassification::PerVertexData, 0 },
			{ "TEXCOORD", 0, Format::R32G32_Float, 0, 6 * sizeof(float), InputClassification::PerVertexData, 0 },
			{ "COLOR", 0, Format::R32G32B32_Float, 0, 8 * sizeof(float), InputClassification::PerVertexData, 0 },

			{ "INSTANCED_POSITION", 0, Format::R32G32B32_Float, 1, 0, InputClassification::PerInstanceData, 1 },
			{ "INSTANCED_ROTATION", 0, Format::R32G32B32_Float, 1, 3 * sizeof(float), InputClassification::PerInstanceData, 1 },
			{ "INSTANCED_SCALE", 0, Format::R32_Float, 1, 6 * sizeof(float), InputClassification::PerInstanceData, 1 },
			{ "INSTANCED_TEX_INDEX", 0, Format::R32_UInt, 1, 7 * sizeof(float), InputClassification::PerInstanceData, 1 }
		};

		m_rockPSO.SetInputLayout(vertexStreams, vertexElements);

		m_rockPSO.SetVertexShader("InstancingVS");
		m_rockPSO.SetPixelShader("InstancingPS");

		m_rockPSO.Finalize();
	}

	// Planet
	{
		m_planetPSO.SetRootSignature(m_modelRootSig);
		m_planetPSO.SetBlendState(CommonStates::BlendDisable());
		m_planetPSO.SetRasterizerState(CommonStates::RasterizerDefaultCW());
		m_planetPSO.SetDepthStencilState(CommonStates::DepthStateReadWriteReversed());
		m_planetPSO.SetPrimitiveTopology(PrimitiveTopology::TriangleList);

		m_planetPSO.SetRenderTargetFormat(GetColorFormat(), GetDepthFormat());

		m_planetPSO.SetVertexShader("StarfieldVS");
		m_planetPSO.SetPixelShader("StarfieldPS");

		// Vertex inputs
		VertexStreamDesc vertexStream{ 0, 11 * sizeof(float), InputClassification::PerVertexData };
		vector<VertexElementDesc> vertexElements =
		{
			{ "POSITION", 0, Format::R32G32B32_Float, 0, 0, InputClassification::PerVertexData, 0 },
			{ "NORMAL", 0, Format::R32G32B32_Float, 0, 3 * sizeof(float), InputClassification::PerVertexData, 0 },
			{ "TEXCOORD", 0, Format::R32G32_Float, 0, 6 * sizeof(float), InputClassification::PerVertexData, 0 }, 
			{ "COLOR", 0, Format::R32G32B32_Float, 0, 8 * sizeof(float), InputClassification::PerVertexData, 0 }
		};

		m_planetPSO.SetInputLayout(vertexStream, vertexElements);

		m_planetPSO.SetVertexShader("PlanetVS");
		m_planetPSO.SetPixelShader("PlanetPS");

		m_planetPSO.Finalize();
	}
}


void InstancingApp::InitConstantBuffer()
{
	m_planetConstantBuffer.Create("Constant Buffer", 1, sizeof(VSConstants));
	
	UpdateConstantBuffer();
}


void InstancingApp::InitInstanceBuffer()
{
	struct InstanceData
	{
		float pos[3];
		float rot[3];
		float scale;
		uint32_t index;
	};

	const int32_t numLayers = (int32_t)m_rockTexture->GetArraySize();

	vector<InstanceData> instanceData;
	instanceData.resize(m_numInstances);

	Math::RandomNumberGenerator rng;

	for (uint32_t i = 0; i < m_numInstances / 2; ++i)
	{
		float ring0[] = { 7.0f, 11.0f };
		float ring1[] = { 14.0f, 18.0f };

		float rho = sqrtf((powf(ring0[1], 2.0f) - pow(ring0[0], 2.0f)) * rng.NextFloat() + powf(ring0[0], 2.0f));
		float theta = DirectX::XM_2PI * rng.NextFloat();

		instanceData[i].pos[0] = rho * cosf(theta);
		instanceData[i].pos[1] = rng.NextFloat(-0.25f, 0.25f);
		instanceData[i].pos[2] = rho * sinf(theta);
		instanceData[i].rot[0] = rng.NextFloat(0, DirectX::XM_PI);
		instanceData[i].rot[1] = rng.NextFloat(0, DirectX::XM_PI);
		instanceData[i].rot[2] = rng.NextFloat(0, DirectX::XM_PI);
		instanceData[i].scale = 0.75f * (1.5f + rng.NextFloat() - rng.NextFloat());
		instanceData[i].index = rng.NextInt(0, numLayers - 1);

		rho = sqrtf((powf(ring1[1], 2.0f) - pow(ring1[0], 2.0f)) * rng.NextFloat() + powf(ring1[0], 2.0f));
		theta = DirectX::XM_2PI * rng.NextFloat();

		uint32_t i2 = i + m_numInstances / 2;
		instanceData[i2].pos[0] = rho * cosf(theta);
		instanceData[i2].pos[1] = rng.NextFloat(-0.25f, 0.25f);
		instanceData[i2].pos[2] = rho * sinf(theta);
		instanceData[i2].rot[0] = rng.NextFloat(0, DirectX::XM_PI);
		instanceData[i2].rot[1] = rng.NextFloat(0, DirectX::XM_PI);
		instanceData[i2].rot[2] = rng.NextFloat(0, DirectX::XM_PI);
		instanceData[i2].scale = 0.75f * (1.5f + rng.NextFloat() - rng.NextFloat());
		instanceData[i2].index = rng.NextInt(0, numLayers - 1);
	}

	m_instanceBuffer.Create("Per-Instance Buffer", m_numInstances, sizeof(InstanceData), instanceData.data());
}


void InstancingApp::InitResourceSets()
{
	m_rockResources.Init(&m_modelRootSig);
	m_rockResources.SetCBV(0, 0, m_planetConstantBuffer);
	m_rockResources.SetSRV(1, 0, *m_rockTexture);
	m_rockResources.Finalize();


	m_planetResources.Init(&m_modelRootSig);
	m_planetResources.SetCBV(0, 0, m_planetConstantBuffer);
	m_planetResources.SetSRV(1, 0, *m_planetTexture);
	m_planetResources.Finalize();
}


void InstancingApp::UpdateConstantBuffer()
{
	using namespace Math;

	m_planetConstants.projectionMatrix = m_camera.GetProjMatrix();
	
	//Matrix4 viewMatrix{ AffineTransform(Vector3(5.5f, -1.85f, 0.0f) + Vector3(0.0f, 0.0f, m_zoom)) };
	
	//viewMatrix = viewMatrix * Matrix4(AffineTransform::MakeXRotation(XMConvertToRadians(-17.2f)));
	//viewMatrix = viewMatrix * Matrix4(AffineTransform::MakeYRotation(XMConvertToRadians(-4.7f)));
	//viewMatrix = viewMatrix * Matrix4(AffineTransform::MakeZRotation(XMConvertToRadians(0.0f)));

	Quaternion rotX{ Vector3(kXUnitVector), XMConvertToRadians(17.2f) };
	Quaternion rotY{ Vector3(kYUnitVector), XMConvertToRadians(4.7f) };
	Quaternion rotZ{ Vector3(kZUnitVector), XMConvertToRadians(0.0f) };

	Quaternion rotTotal = rotX * rotY * rotZ;

	Matrix4 viewMatrix{ AffineTransform(rotTotal, Vector3(5.5f, 1.85f, 0.0f) + Vector3(0.0f, 0.0f, m_zoom)) };

	m_planetConstants.modelViewMatrix = viewMatrix;
	m_planetConstants.localSpeed += m_frameTimer * 0.35f;
	m_planetConstants.globalSpeed += m_frameTimer * 0.01f;

	m_planetConstantBuffer.Update(sizeof(m_planetConstants), &m_planetConstants);
}


void InstancingApp::LoadAssets()
{
	m_rockTexture = Texture::Load("texturearray_rocks_bc3_unorm.ktx", Format::Unknown, true);
	m_planetTexture = Texture::Load("lavaplanet_bc3_unorm.ktx", Format::Unknown, true);

	auto layout = VertexLayout(
	{
		VertexComponent::Position,
		VertexComponent::Normal,
		VertexComponent::UV,
		VertexComponent::Color
	});

	m_rockModel = Model::Load("rock01.dae", layout, 0.1f);
	m_planetModel = Model::Load("sphere.obj", layout, 0.2f);
}